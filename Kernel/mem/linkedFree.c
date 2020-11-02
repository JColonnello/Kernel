#include <lib.h>
#include "naiveConsole.h"

#ifdef VIRT_MEM_LINKED
extern size_t reservedMemCount;

typedef struct MallocNode MallocNode;
struct MallocNode
{
	size_t size;
	bool used;
	MallocNode *next;
	__attribute__((aligned(16))) char block[];
};

MallocNode *firstNode = NULL;
void *kmalloc(size_t size)
{
	MallocNode **lastNode = &firstNode;
	size = (size + 15) & (size_t)~0xF;
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

#endif