#include <syncro/wait.h>
#include <syncro/semaphore.h>
#include <collections/pool.h>
#include <stdatomic.h>

typedef struct
{
	int id;
	int refs;
	_Atomic(int) count;
	WaitHandle *wait;
	atomic_flag lock;
} SemaphoreResource;

struct Semaphore
{
	SemaphoreResource *resource;
};

static Pool *pool;
static Semaphore *lock = NULL;

void sem_init()
{
	pool = Pool_Create(sizeof(SemaphoreResource));
	lock = sem_create(1);
}

Semaphore *sem_create(long unsigned initialCount)
{
	SemaphoreResource _temp = {0}, *resource;

	lock(lock);
	int id = Pool_Add(pool, &_temp);
	unlock(lock);
	resource = Pool_GetRef(pool, id);
	resource->count = initialCount;
	resource->id = id;
	resource->wait = WaitHandle_Create();

	return sem_open(id);
}

Semaphore *sem_open(int id)
{
	lock(lock);
	SemaphoreResource *res = Pool_GetRef(pool, id);
	if(res == NULL)
		return NULL;

	res->refs++;
	unlock(lock);
	Semaphore *handler = kmalloc(sizeof(struct Semaphore));
	handler->resource = res;
	return handler;
}

int sem_getId(Semaphore *handler)
{
	return handler != NULL ? handler->resource->id : -1;
}

void sem_close(Semaphore *handler)
{
	if(handler == NULL)
		return;
	
	lock(lock);
	if(--handler->resource->refs == 0)
	{
		WaitHandle_Dispose(handler->resource->wait);
		Pool_Remove(pool, handler->resource->id);
	}
	kfree(handler);
	unlock(lock);
}

void sem_wait(Semaphore *handler)
{
	if(handler == NULL)
		return;

	waitEvent(handler->resource->count > 0 && !atomic_flag_test_and_set(&handler->resource->lock),
		handler->resource->wait);

	handler->resource->count--;
	
	atomic_flag_clear(&handler->resource->lock);
}

void sem_release(Semaphore *handler)
{
	if(handler == NULL)
		return;
		
	handler->resource->count++;
	releaseOne(handler->resource->wait);
}

Semaphore *sem_dup(Semaphore *sem)
{
	int id = sem->resource->id;
	return sem_open(id);
}

int sem_list(SemaphoreStatus *out, size_t n)
{
	int count = Pool_Count(pool);
	if(out == NULL)
		return count;
	
	lock(lock);
	int indexes[count];
	Pool_ToIndexArray(pool, indexes);
	for(int i = 0; i < count && i < n; i++)
	{
		SemaphoreResource *res = Pool_GetRef(pool, indexes[i]);
		out[i] = (SemaphoreStatus)
		{
			.id = res->id,
			.count = res->count,
		};
		out[i].blockedCount = WaitHandle_blockedList(res->wait, out[i].blocked, sizeof(out[i].blocked)/sizeof(*out[i].blocked));
	}
	unlock(lock);

	return count;
}

void sem_info(const Semaphore *sem, SemaphoreStatus *out)
{
	SemaphoreResource *res = sem->resource;
	*out = (SemaphoreStatus)
	{
		.id = res->id,
		.count = res->count,
	};
	out->blockedCount = WaitHandle_blockedList(res->wait, out->blocked, sizeof(out->blocked)/sizeof(*out->blocked));
}