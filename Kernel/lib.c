#include "interrupts/interrupts.h"
#include "naiveConsole.h"
#include <stdbool.h>
#include <stdint.h>
#include <lib.h>
#include <loader.h>
#include <pid.h>

#define LOOPBACK 0x1FE

//Bit field to map 4GB of RAM
static uint8_t physReservedPages[((uint64_t)4<<30) / PAGE_SIZE / 8];
static size_t firstFreePage;
static size_t currVirtualPage = 0;
static size_t reservedMemCount = 0;
static size_t reservedPagesCount = 0;

void * memset(void * destination, char c, size_t length)
{
	uint8_t chr = (uint8_t)c;
	char * dst = (char*)destination;

	while(length--)
		dst[length] = chr;

	return destination;
}

void * memcpy(void * destination, const void * source, size_t length)
{
	/*
	* memcpy does not support overlapping buffers, so always do it
	* forwards. (Don't change this without adjusting memmove.)
	*
	* For speedy copying, optimize the common case where both pointers
	* and the length are word-aligned, and copy word-at-a-time instead
	* of byte-at-a-time. Otherwise, copy by bytes.
	*
	* The alignment logic below should be portable. We rely on
	* the compiler to be reasonably intelligent about optimizing
	* the divides and modulos out. Fortunately, it is.
	*/
	uint64_t i;

	if ((uint64_t)destination % sizeof(uint32_t) == 0 &&
		(uint64_t)source % sizeof(uint32_t) == 0 &&
		length % sizeof(uint32_t) == 0)
	{
		uint32_t *d = (uint32_t *) destination;
		const uint32_t *s = (const uint32_t *)source;

		for (i = 0; i < length / sizeof(uint32_t); i++)
			d[i] = s[i];
	}
	else
	{
		uint8_t * d = (uint8_t*)destination;
		const uint8_t * s = (const uint8_t*)source;

		for (i = 0; i < length; i++)
			d[i] = s[i];
	}

	return destination;
}

static uintptr_t reservePhysPage()
{
	const size_t pageFieldSize = sizeof(physReservedPages);
	int pos = 0, i;
	for(i = 0; physReservedPages[i] == 0xFF && i < pageFieldSize; i++, pos += 8) ;
	if(i == pageFieldSize)
		return -1;

	uint8_t chunk = physReservedPages[i];
	int off;
	for(off = 0; chunk & 1; chunk >>= 1, off++, pos++) ;
	physReservedPages[i] |= 1 << off;

        reservedPagesCount++;
        return pos << 12;
}

static void freePhysPage(size_t idx)
{
	if(idx / 8 >= sizeof(physReservedPages))
		return;

	if(physReservedPages[idx / 8] & (1 << (idx % 8)))
		reservedPagesCount--;
	physReservedPages[idx / 8] &= ~(1 << (idx % 8));
}

size_t getReservedPagesCount() { return reservedPagesCount; }
size_t getReservedMemoryCount() { return reservedMemCount; }

void libInit()
{
	firstFreePage = (&__endOfKernel - &__startOfUniverse) / PAGE_SIZE + 16 + 1;
	memset(physReservedPages, 0xFF, firstFreePage / 8);
	uint8_t last = 0;
	for(int i = 0; i < firstFreePage % 8; i++)
		last = last << 1 | 1;
	physReservedPages[firstFreePage / 8] = last;
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
static void map(uintptr_t virtual, const uintptr_t *physMap, size_t pageCount)
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
					pt.table[va.ptoff] = physMap[count++] | 0x7;
					if(count >= pageCount)
						return;

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
	VirtualAddr limit = { .ptr = &__startOfUniverse };
	
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
}

void kunmap(void *virtual, size_t pageCount)
{
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
	}
}

void *kmap(void **virtual, const void *hint, void **physical, size_t pageCount)
{
	uintptr_t phyPages[pageCount];
	if(physical == NULL)
	{
		for(int i = 0; i < pageCount; i++)
			phyPages[i] = reservePhysPage();
	}
	else
		for(int i = 0; i < pageCount; i++)
			phyPages[i] = (uintptr_t)*physical + PAGE_SIZE * i;

	uintptr_t dest;
	if(virtual == NULL)
	{
		dest = currVirtualPage << 12;
		currVirtualPage += pageCount;
	}
	else
		dest = (uintptr_t)*virtual;

	map(dest, phyPages, pageCount);

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

typedef struct MallocNode MallocNode;
struct MallocNode
{
	size_t size;
	bool used;
	MallocNode *next;
	char block[];
};

MallocNode *firstNode = NULL;
void *kmalloc(size_t size)
{
	MallocNode **lastNode = &firstNode;

	while(*lastNode != NULL)
	{
		MallocNode *node = *lastNode;
		if(!node->used && node->size >= size)
		{
			node->used = true;
			uintptr_t newNodeStart = size;
			int remain = node->size - newNodeStart;
			if(remain > sizeof(MallocNode))
			{
				MallocNode *newNode = (MallocNode*)&node->block[newNodeStart];
				newNode->size = remain - sizeof(MallocNode);
				newNode->used = false;
				newNode->next = node->next;
				node->next = newNode;
				node->size = newNodeStart;
			}
			reservedMemCount += node->size;
			return node->block;
		}
		//Next node is unused and contiguous
		if(!node->used && node->next == (MallocNode*)&node->block[node->size] && !node->next->used)
		{
			MallocNode *next = node->next;
			node->size += next->size + sizeof(MallocNode);
			node->next = next->next;
			//Avoid address leak
			*next = (MallocNode){ 0 };
			//Try again
			continue;
		}
		lastNode = &node->next;
	}
	//No free block found. We have to reserve new pages
	int pages = (size + sizeof(MallocNode) + PAGE_SIZE - 1) / PAGE_SIZE;
	MallocNode *node = kmap(NULL, NULL, NULL, pages);
	MallocNode *postNode = (void*)&node->block[size];
	postNode->size = pages * PAGE_SIZE - sizeof(MallocNode) * 2 - size;
	postNode->used = false;
	postNode->next = NULL;
	node->size = size;
	node->used = true;
	node->next = postNode;
	*lastNode = node;
	reservedMemCount += node->size;
	return node->block;
}

void kfree(void *ptr)
{
	MallocNode **lastNode = &firstNode;

	if(ptr == NULL)
		return;

	while(*lastNode != NULL)
	{
		MallocNode *node = *lastNode;
		if(node->block == ptr)
		{
			node->used = false;
			reservedMemCount -= node->size;
			return;
		}
		lastNode = &node->next;
	}
	ncPrint("Bad free\n");
}

void *kcalloc(size_t nmemb, size_t size)
{
	void *mem = kmalloc(nmemb * size);
	if(mem)
		memset(mem, 0, nmemb * size);
	return mem;
}