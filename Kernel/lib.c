#include "interrupts/interrupts.h"
#include "naiveConsole.h"
#include <stdbool.h>
#include <stdint.h>
#include <lib.h>
#include <loader.h>
#include <pid.h>

#define PAGE_SIZE 0x1000

extern const void *PML4_ADDR;
extern const void *PDP_ADDR;
extern const void *PD_ADDR;
extern const void *PT_ADDR;
//Bit field to map 4GB of RAM
uint8_t physReservedPages[((uint64_t)4<<30) / PAGE_SIZE / 8];

size_t currVirtualPage[MAX_PID];

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

	return pos << 12;
}

static void freePhysPage(int pos)
{
	physReservedPages[pos / 8] &= ~(1 << (pos % 8));
}

void libInit()
{
	size_t firstFreePage = (&__endOfKernel - &__startOfUniverse) / PAGE_SIZE + 16 + 1;
	memset(physReservedPages, 0xFF, firstFreePage / 8);
	uint8_t last = 0;
	for(int i = 0; i < firstFreePage % 8; i++)
		last = last << 1 | 1;
	physReservedPages[firstFreePage / 8] = last;
	currVirtualPage[0] = firstFreePage + ((uintptr_t)&__startOfUniverse >> 12);
}

#define PMASK 0xFFFFFFFFFF000

#pragma pack(push)
#pragma pack(1) 
typedef union
{
	uintptr_t addr;
	uintptr_t *table;
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
		.pml4off = 0x1FE,
		.pdpoff = 0x1FE,
		.pdoff = 0x1FE,
		.ptoff = 0x1FE,
		.poff = 0,
	};
	size_t count = 0;
	//ProcessDescriptor cp = currentProcess();
	uintptr_t *pml4 = (uintptr_t*)lb.addr;
	for(; va.pml4off < 512; va.pml4off++)
	{
		uintptr_t pml4e = pml4[va.pml4off];
		if(!pml4e)
		{
			uintptr_t table = reservePhysPage();
			pml4[va.pml4off] = table | 0b11;
		}
		VirtualAddr pdp = lb;
		pdp.ptoff = va.pml4off;
		for(; va.pdpoff < 512; va.pdpoff++)
		{
			uintptr_t pdpe = pdp.table[va.pdpoff];
			if(!pdpe)
			{
				uintptr_t table = reservePhysPage();
				pdp.table[va.pdpoff] = table | 0b11;
			}
			VirtualAddr pd = lb;
			pd.pdoff = va.pml4off, pd.ptoff = va.pdpoff;
			for(; va.pdoff < 512; va.pdoff++)
			{
				uintptr_t pde = pd.table[va.pdoff];
				if(!pde)
				{
					uintptr_t table = reservePhysPage();
					pd.table[va.pdoff] = table | 0b11;
				}
				VirtualAddr pt = lb;
				pt.pdpoff = va.pml4off, pt.pdoff = va.pdpoff, pt.ptoff = va.pdoff;
				for(; va.ptoff < 512; va.ptoff++)
				{
					pt.table[va.ptoff] = physMap[count++] | 0x7;
					if(count >= pageCount)
						return;
				}
			}
		}
	}
}

void *kmap(void **virtual, const void *hint, void **physical, size_t pageCount)
{
	if(virtual == NULL && physical == NULL)
	{
		ProcessDescriptor cp = currentProcess();
		uintptr_t hint = currVirtualPage[cp.pid] << 12;
		uintptr_t phyPages[pageCount];
		for(int i = 0; i < pageCount; i++)
			phyPages[i] = reservePhysPage();
		map(hint, phyPages, pageCount);
		currVirtualPage[cp.pid] += pageCount;
		return (void*)hint;
	}
	return NULL;
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
			size_t dif = node->size - size;
			if(dif > sizeof(MallocNode))
			{
				MallocNode *newNode = (MallocNode*)&node->block[size];
				newNode->size = dif - sizeof(MallocNode);
				newNode->used = false;
				newNode->next = node->next;
				node->next = newNode;
			}
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
	return node->block;
}

void kfree(void *ptr)
{
	MallocNode **lastNode = &firstNode;

	while(*lastNode != NULL)
	{
		MallocNode *node = *lastNode;
		if(node->block == ptr)
		{
			node->used = false;
			return;
		}
		lastNode = &node->next;
	}
	ncPrint("Bad free\n");
}