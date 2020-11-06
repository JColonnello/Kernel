#include "naiveConsole.h"
#include <stdbool.h>
#include <stdint.h>
#include <lib.h>
#include <loader.h>
#include <pid.h>
#include <scheduler.h>

#define LOOPBACK 0x1FE
#define ONE ((uint64_t)1)

//Bit field to map 1GB of RAM
static uint64_t physReservedPages[(ONE<<27) / PAGE_SIZE / 64];
static size_t firstFreePage;
static size_t currVirtualPage = 0;
size_t reservedMemCount = 0;
static size_t reservedPagesCount = 0;

__attribute__((used))
void * memset(void * destination, char c, size_t length)
{
	uint8_t chr = (uint8_t)c;
	char * dst = (char*)destination;

	while(length--)
		dst[length] = chr;

	return destination;
}

__attribute__((used))
void * memcpy(void * restrict destination, const void * restrict source, size_t length)
{
	char * restrict dst = destination;
	const char * restrict src = source;
	while(length--)
        *dst++ = *src++;
	return destination;
}

static uintptr_t reservePhysPage()
{
	const size_t pageFieldSize = sizeof(physReservedPages) / sizeof(*physReservedPages);
	int pos = 0, i;

	Scheduler_Disable();
	for(i = 0; i < pageFieldSize && physReservedPages[i] == -ONE; i++, pos += 64) ;
	if(i == pageFieldSize)
		return -1;

	uint64_t chunk = physReservedPages[i];
	int off;
	for(off = 0; chunk & 1; chunk >>= 1, off++, pos++) ;
	physReservedPages[i] |= ONE << off;

	reservedPagesCount++;
	Scheduler_Enable();
	return pos << 12;
}

static void freePhysPage(size_t idx)
{
	size_t chunk = idx / 64;
	if(chunk >= sizeof(physReservedPages) / sizeof(*physReservedPages))
		return;

	size_t off = idx % 64;
	Scheduler_Disable();
	if(physReservedPages[chunk] & (ONE << (off)))
	{
		reservedPagesCount--;
		physReservedPages[chunk] &= ~(ONE << (off));
	}
	Scheduler_Enable();
}

size_t getReservedPagesCount() { return reservedPagesCount; }
size_t getReservedMemoryCount() { return reservedMemCount; }

void libInit()
{
	firstFreePage = (&__endOfKernel - &__startOfUniverse) / PAGE_SIZE + 16 + 1;
	memset(physReservedPages, 0xFF, firstFreePage / 64 * 8);
	uint64_t last = 0;
	for(int i = 0; i < firstFreePage % 64; i++)
		last = last << 1 | 1;
	physReservedPages[firstFreePage / 64] = last;
	currVirtualPage = firstFreePage + ((uintptr_t)&__startOfUniverse >> 12);
}

#define PMASK 0xFFFFFFFFFF000

#pragma pack(push)
#pragma pack(1) 
typedef union
{
	uintptr_t addr;
	uintptr_t *table;
	void *ptr;
	struct
	{
		unsigned poff : 12;
		unsigned ptoff : 9;
		unsigned pdoff : 9;
		unsigned pdpoff : 9;
		unsigned pml4off : 9;
		unsigned sign : 16;
	};
} VirtualAddr;
#pragma pack(pop)

//Welcome to code hell
//Alert: TLB cache magic inbound
static bool map(uintptr_t virtual, const uintptr_t *physMap, size_t pageCount)
{
	VirtualAddr va = { .addr = virtual };
	//Setup loopback address
	const VirtualAddr lb = 
	{ 
		.sign = -1,
		.pml4off = LOOPBACK,
		.pdpoff = LOOPBACK,
		.pdoff = LOOPBACK,
		.ptoff = LOOPBACK,
		.poff = 0,
	};
	size_t count = 0;
	uintptr_t *pml4 = lb.table;
	for(;;)
	{
		uintptr_t pml4e = pml4[va.pml4off];
		VirtualAddr pdp = lb;
		pdp.ptoff = va.pml4off;
		if(!pml4e)
		{
			uintptr_t table = reservePhysPage();
			pml4[va.pml4off] = table | 0b11;
			memset(pdp.table, 0, PAGE_SIZE);
		}
		for(;;)
		{
			uintptr_t pdpe = pdp.table[va.pdpoff];
			VirtualAddr pd = lb;
			pd.pdoff = va.pml4off, pd.ptoff = va.pdpoff;
			if(!pdpe)
			{
				uintptr_t table = reservePhysPage();
				pdp.table[va.pdpoff] = table | 0b11;
				memset(pd.table, 0, PAGE_SIZE);
			}
			for(;;)
			{
				uintptr_t pde = pd.table[va.pdoff];
				VirtualAddr pt = lb;
				pt.pdpoff = va.pml4off, pt.pdoff = va.pdpoff, pt.ptoff = va.pdoff;
				if(!pde)
				{
					uintptr_t table = reservePhysPage();
					pd.table[va.pdoff] = table | 0b11;
					memset(pt.table, 0, PAGE_SIZE);
				}
				for(;;)
				{
					if(pt.table[va.ptoff])
						ncPrint("Overmap\n");
					uintptr_t page;
					if(physMap != NULL)
						page = physMap[count];
					else
					{
						page = reservePhysPage();
						if(page == (uintptr_t)-1)
						{
							kunmap((void*)virtual, count);
							return false;
						}
					}
					pt.table[va.ptoff] = page | 0x7;
					count++;
					//Invalidate this entry in TLB
					_invlpg(va.ptr);
					if(count >= pageCount)
						return true;

					va.ptoff++;
					if(va.ptoff == 0)
						break;
				}
				va.pdoff++;
				if(va.pdoff == 0)
					break;
			}
			va.pdpoff++;
			if(va.pdpoff == 0)
				break;
		}
		va.pml4off++;
		if(va.pml4off == 0)
			break;
	}
	ncPrint("Virtual overrun\n");
	return false;
}

static void checkAndFree(uintptr_t entry)
{
	if(entry & 0x1 && entry > (firstFreePage << 12))
		freePhysPage(entry>>12);
}

void dropTable()
{
	VirtualAddr va = { .addr = 0 };
	//Setup loopback address
	const VirtualAddr lb = 
	{ 
		.sign = -1,
		.pml4off = LOOPBACK,
		.pdpoff = LOOPBACK,
		.pdoff = LOOPBACK,
		.ptoff = LOOPBACK,
		.poff = 0,
	};
	VirtualAddr limit = { .ptr = (void*)&__startOfUniverse };
	
	uintptr_t *pml4 = lb.table;
	while(va.addr < limit.addr)
	{
		uintptr_t pml4e = pml4[va.pml4off];
		if(va.pml4off == LOOPBACK || !(pml4e & 0x1))
			goto incpml4;
		
		VirtualAddr pdp = lb;
		pdp.ptoff = va.pml4off;
		while(va.addr < limit.addr)
		{
			uintptr_t pdpe = pdp.table[va.pdpoff];
			if(!(pdpe & 0x1))
				goto incpdp;
			
			VirtualAddr pd = lb;
			pd.pdoff = va.pml4off, pd.ptoff = va.pdpoff;
			while(true)
			{
				uintptr_t pde = pd.table[va.pdoff];
				if(!(pde & 0x1))
					goto incpd;
				
				VirtualAddr pt = lb;
				pt.pdpoff = va.pml4off, pt.pdoff = va.pdpoff, pt.ptoff = va.pdoff;
				while(true)
				{
					uintptr_t pte = pt.table[va.ptoff];
					checkAndFree(pte);
					va.ptoff++;
					if(va.ptoff == 0)
						break;
				}
				checkAndFree(pde);
				incpd:
				va.pdoff++;
				if(va.pdoff == 0)
					break;
			}
			checkAndFree(pdpe);
			incpdp:
			va.pdpoff++;
			if(va.addr >> 47)
				va.sign = -1;
			if(va.pdpoff == 0)
				break;
		}
		checkAndFree(pml4e);
		incpml4:
		va.pml4off++;
		if(va.pml4off == 0)
			break;
	}
	uintptr_t pml4Loop = pml4[LOOPBACK];
	checkAndFree(pml4Loop);
	//Bug catcher. Not required
	reloadTLB();
}

void kunmap(void *virt, size_t pageCount)
{
	char *virtual = virt;
	for(int i = 0; i < pageCount; i++)
	{
		VirtualAddr addr = { .ptr = virtual };
		VirtualAddr pt = {
			.sign = -1,
			.pml4off = LOOPBACK,
			.pdpoff = addr.pml4off,
			.pdoff = addr.pdpoff,
			.ptoff = addr.pdoff,
			.poff = 0,
		};
		size_t physPage = pt.table[addr.ptoff] >> 12;
		if(physPage >= firstFreePage)
			freePhysPage(physPage);
		pt.table[addr.ptoff] = 0;
		_invlpg(addr.ptr);
		virtual += 0x1000;
	}
}

void *kmap(void **virtual, const void *hint, void **physical, size_t pageCount)
{
	uintptr_t *phyPages = physical != NULL ? *physical : NULL;

	uintptr_t dest;
	if(virtual == NULL)
	{
		dest = currVirtualPage << 12;
		currVirtualPage += pageCount;
	}
	else
		dest = (uintptr_t)*virtual;

	if(!map(dest, phyPages, pageCount))
		return NULL;

	return (void*)dest;
}

#define PD_ADDR 0x10000

uintptr_t createPML4()
{
	//For PML4 and last PDP
	VirtualAddr tmp = { .addr = 0xFFFFFFFFFFFFE000 };
	VirtualAddr umap = 
	{
		.sign = -1,
		.pml4off = LOOPBACK,
		.pdpoff = tmp.pml4off,
		.pdoff = tmp.pdpoff,
		.ptoff = tmp.pdoff,
	};
	kmap(&tmp.ptr, NULL, NULL, 2);
	memset(tmp.ptr, 0, PAGE_SIZE * 2);
	//Temporal map to populate
	uintptr_t pml4 = umap.table[0x1FE] & PAGE_MASK;
	uintptr_t pdp = umap.table[0x1FF] & PAGE_MASK;
	//Set loopback
	tmp.table[LOOPBACK] = pml4 | 0x3;
	//Reference PDP
	tmp.table[0x1FF] = pdp | 0x3;
	tmp.addr += 0x1000;
	//Reference PD of kernel
	tmp.table[0x1FF] = PD_ADDR | 0x3;
	//Reference kernel last table
	umap.table[0x1FE] = 0;
	umap.table[0x1FF] = 0;
	return pml4;
}

void *kcalloc(size_t nmemb, size_t size)
{
	void *mem = kmalloc(nmemb * size);
	if(mem)
		memset(mem, 0, nmemb * size);
	return mem;
}

bool isKernelAddress(void *address)
{
	return (uintptr_t)address>>39 == LOOPBACK || (void*)address >= &__startOfUniverse;
}
