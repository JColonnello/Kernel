#include "loader.h"
#include <lib.h>
#include <naiveConsole.h>
#include <scheduler.h>

#ifdef VIRT_MEM_BUDDY
extern size_t reservedMemCount;

#define ONE ((uint64_t)1)
#define MIN_BLOCK_EXP 12
#define MAX_BLOCK_EXP 29
typedef unsigned long bitstor;
#define UNIT_BITS (8 * sizeof(bitstor))

#define BUDDY_SIZE (ONE<<(MAX_BLOCK_EXP - MIN_BLOCK_EXP + 1)) / UNIT_BITS
bitstor bitfield[BUDDY_SIZE] = {0};

static bool get(int level, int index)
{
	if(level < 0 || level > (MAX_BLOCK_EXP - MIN_BLOCK_EXP))
		return false;
	if(index < 0 || index >= ONE << level)
		return false;

	size_t pos = index;
	for(int i = 0; i < level; i++)
		pos += ONE << i;
	return bitfield[pos / UNIT_BITS] & ONE << (pos % UNIT_BITS);
}

static void set(int level, int index, bool value)
{
	if(level < 0 || level > (MAX_BLOCK_EXP - MIN_BLOCK_EXP))
		return;
	if(index < 0 || index >= ONE << level)
		return;

	size_t pos = index;
	for(int i = 0; i < level; i++)
		pos += ONE << i;
	
	if(value)
		bitfield[pos / UNIT_BITS] |= ONE << (pos % UNIT_BITS);
	else
		bitfield[pos / UNIT_BITS] &= ~(ONE << (pos % UNIT_BITS));
}

static int roundExpUp(size_t size)
{
	int exp = bsr(size);
	return (size == ONE << exp) ? exp : exp+1;
}

static inline int buddy(int index)
{
	return (index % 2) ? index - 1 : index + 1;
}

void *kmalloc(size_t size)
{
	if(size == 0)
		return NULL;

	int exp = roundExpUp(size);
	if(exp > MAX_BLOCK_EXP)
		return NULL;
	if(exp < MIN_BLOCK_EXP)
		exp = MIN_BLOCK_EXP;
	size = (ONE << exp);
	int level = MAX_BLOCK_EXP - exp;

	Scheduler_Disable();
	int i;
	for (i = 0; i < ONE << level; i++)
	{
		if(!get(level, i))
		{
			int parent = level, j = i;
			while(parent > 0 && !get(parent-1, j/2))
				parent--, j /= 2;
			if(get(parent, buddy(j)) || parent == 0)
				break;
		}

	}
	if(i == ONE << level)
		return NULL;
	
	void *block = (void*)&__endOfKernelStack + size * i;
	kmap(&block, NULL, NULL, size / PAGE_SIZE);

	for(; level >= 0 && !get(level, i); level--, i /= 2)
		set(level, i, true);

	reservedMemCount += size;
	Scheduler_Enable();
	return block;
}

void kfree(void *ptr)
{
	if(ptr < &__endOfKernelStack)
		return;

	uintptr_t rel = (uintptr_t)ptr - (uintptr_t)&__endOfKernelStack;
	int exp = rel == 0 ? MAX_BLOCK_EXP : bsf(rel);
	if(exp < MIN_BLOCK_EXP || exp > MAX_BLOCK_EXP)
		goto error;
	int level = MAX_BLOCK_EXP - exp;
	int i = rel >> exp;
	if(i >= ONE << level)
		goto error;

	Scheduler_Disable();
	if(!get(level, i))
		goto error;
	while(get(level+1, i*2))
		level++, i *= 2;

	size_t size = (ONE << ( MAX_BLOCK_EXP - level ));

	kunmap(ptr, size / PAGE_SIZE);
	
	for(; level >= 0; level--, i /= 2)
	{
		set(level, i, false);
		if(get(level, buddy(i)))
			break;
	}
	reservedMemCount -= size;
	goto exit;

	error:
	ncPrint("Bad free\n");
	exit:
	Scheduler_Enable();
	return;
}

#endif