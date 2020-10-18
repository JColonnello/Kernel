#include <collections/pool.h>
#include <lib.h>
#include <stdint.h>
#include <stddef.h>
#include <syncro/semaphore.h>

#define DEFAULT_CAPACITY 32

struct Pool
{
	size_t elemSize;
	size_t count;
	size_t maxCount;
	size_t size;
	void *data;
	uint8_t *flags;
};

static bool checkIndex(Pool *pool, int index)
{
	if(index < 0 || index >= pool->maxCount)
		return false;

	int block = index / 8;
	int offset = index % 8;
	return pool->flags[block] & ((uint8_t)128 >> offset);

}

static void reallocMem(Pool *pool, int newSize)
{
	//Reserve memory for data and flags
	size_t dataSize = pool->elemSize * newSize;
	size_t flagsSize = newSize / 8;
	size_t oldFlagsSize = pool->size / 8;
	uintptr_t *mem = kmalloc(dataSize + flagsSize);
	//Copy flags to new array (data is copied on realloc)
	memcpy(mem, pool->data, pool->elemSize * pool->size);
	pool->data = mem;
	memcpy(mem + dataSize, pool->flags, oldFlagsSize);
	pool->flags = (uint8_t*)(mem + dataSize);

	//Initialize new flags to 0
	memset(&pool->flags[oldFlagsSize], 0, flagsSize - oldFlagsSize);
	pool->size = newSize;
}

Pool *Pool_Create(size_t elemSize)
{
	//Reserve memory
	Pool *pool = kmalloc(sizeof(struct Pool));
	if(pool == NULL)
		return NULL;
	
	//Set variables
	pool->size = 0;
	pool->elemSize = elemSize;
	pool->count = 0;
	pool->maxCount = 0;
	
	//Reserve memory for data
	pool->data = NULL;
	pool->flags = NULL;
	reallocMem(pool, DEFAULT_CAPACITY);

	return pool;
}

int Pool_Add(Pool *pool, void *data)
{
	int pos = 0;
	//If we have reached the end of the array, expland the Pool
	if(pool->count == pool->size)
		reallocMem(pool, pool->size * 2);
	//If there are holes in the Pool check the flags
	if(pool->count < pool->maxCount)
	{
		//Access the flags by byte and then shift to
		//operate on the next bit
		for(int block = 0; block * 8 < pool->maxCount; block++)
		{
			int chunk = pool->flags[block];
			if(chunk == 255) continue;
			for(int bit = 0; bit < 8; bit++, pos++)
			{
				//If a hole is found set that flag to 1 and leave
				if(chunk <= 127) 
				{
					pool->flags[block] |= (uint8_t)128 >> bit;
					goto out;
				}
			}
		}
		out:
		pool->count++;
	}
	//Otherwise Add the element at the end
	else
	{
		int block = pool->count / 8;
		int offset = pool->count % 8;
		pool->flags[block] |= (uint8_t)128 >> offset;
		pos = pool->count++;
	}
	memcpy(pool->data + pos * pool->elemSize, data, pool->elemSize);
	if(pool->count > pool->maxCount) pool->maxCount = pool->count;
	return pos;
}

bool Pool_Get(Pool *pool, int index, void *dest)
{
	if(!checkIndex(pool, index))
		return false;

	memcpy(dest, pool->data + pool->elemSize * index, pool->elemSize);
	return true;
}

void *Pool_GetRef(Pool *pool, int index)
{
	if(!checkIndex(pool, index))
		return NULL;
	return pool->data + pool->elemSize * index;
}

void Pool_Remove(Pool *pool, int index)
{
	int block = index / 8;
	int offset = index % 8;

	//Set flag to 0
	pool->flags[block] &= ~((uint8_t)128 >> offset);
	pool->count--;
	if(index == pool->maxCount - 1)
		pool->maxCount--;
}

int Pool_Count(Pool *pool)
{
	return pool->count;
}

int Pool_ToIndexArray(Pool *pool, int *array)
{
	int arrayPos = 0;
	int poolPos = 0;
	//Search the flags
	for(int block = 0; block < pool->size / 8; block++)
	{
		uint8_t chunk = pool->flags[block];
		for(int bit = 0; bit < 8; bit++, poolPos++, chunk <<= 1)
		{
			//If the flag is 1 copy index to array
			if(chunk >= 128) 
			{
				array[arrayPos++] = poolPos;
			}
		}
	}
	return pool->count;
}

int Pool_ToArray(Pool *pool, void *array)
{
	size_t arrayPos = 0;
	size_t poolPos = 0;
	//Search the flags
	for(int block = 0; block < pool->size / 8; block++)
	{	
		uint8_t chunk = pool->flags[block];
		/*If all the flags in the chunk are 1 copy the
		* 8 elements to the array
		*/
		if(chunk == 255)
		{
			memcpy(array + pool->elemSize * arrayPos,
			 		pool->data + pool->elemSize * poolPos,
			 		 pool->elemSize * 8);
			arrayPos += 8;
			poolPos += 8;
		}
		//Otherwise check the flags == 1 and copy their elements
		else for(int bit = 0; bit < 8; bit++, poolPos++, chunk <<= 1)
		{
			if(chunk >= 128) 
			{
				memcpy(array + pool->elemSize * arrayPos,
			 		pool->data + pool->elemSize * poolPos,
			 		 pool->elemSize);
				arrayPos++;
			}
		}
	}
	return pool->count;
}

void Pool_Dispose(Pool *pool)
{
	kfree(pool->data);
	kfree(pool);
}

void *Pool_Data(Pool *pool)
{
	return pool->data;
}
