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
	int index;
	SemaphoreResource *resource;
};

static Pool *resourcePool, *handlerPool;
static Semaphore *lock = NULL;

void sem_init()
{
	resourcePool = Pool_Create(sizeof(SemaphoreResource));
	handlerPool = Pool_Create(sizeof(struct Semaphore));
	lock = sem_create(1);
}

Semaphore *sem_create(long unsigned initialCount)
{
	SemaphoreResource *resource;

	if(lock) lock(lock);
	int id = Pool_Reserve(resourcePool);
	if(lock) unlock(lock);
	resource = Pool_GetRef(resourcePool, id);
	*resource = (SemaphoreResource) {0};
	resource->count = initialCount;
	resource->id = id;
	resource->wait = WaitHandle_Create();

	return sem_open(id);
}

Semaphore *sem_open(int id)
{
	if(lock) lock(lock);
	SemaphoreResource *res = Pool_GetRef(resourcePool, id);
	if(res == NULL)
		return NULL;
	res->refs++;
	int handlerId = Pool_Reserve(handlerPool);
	if(lock) unlock(lock);

	Semaphore *handler = Pool_GetRef(handlerPool, handlerId);
	handler->resource = res;
	handler->index = handlerId;

	return handler;
}

int sem_getId(Semaphore *handler)
{
	return handler != NULL ? handler->resource->id : -1;
}

void sem_close(Semaphore *handler)
{
	lock(lock);
	if(--handler->resource->refs == 0)
	{
		WaitHandle_Dispose(handler->resource->wait);
		Pool_Remove(resourcePool, handler->resource->id);
	}
	Pool_Remove(handlerPool, handler->index);
	unlock(lock);
}

void sem_wait(Semaphore *handler)
{
	waitEvent(handler->resource->count > 0 && !atomic_flag_test_and_set(&handler->resource->lock),
		handler->resource->wait);

	handler->resource->count--;
	
	atomic_flag_clear(&handler->resource->lock);
}

void sem_release(Semaphore *handler)
{
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
	int count = Pool_Count(resourcePool);
	if(out == NULL)
		return count;
	
	lock(lock);
	int indexes[count];
	Pool_ToIndexArray(resourcePool, indexes);
	for(int i = 0; i < count && i < n; i++)
	{
		SemaphoreResource *res = Pool_GetRef(resourcePool, indexes[i]);
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

Semaphore *sem_handlerById(int index)
{
	return Pool_GetRef(handlerPool, index);
}

int sem_getHandlerId(Semaphore *sem)
{
	return sem->index;
}