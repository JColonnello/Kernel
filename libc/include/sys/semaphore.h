#pragma once
#include <stddef.h>
#define lock(x) sem_wait((x))
#define unlock(x) sem_release((x))

#define BLOCKED_MAX 16

typedef struct {int _id;} Semaphore;
typedef struct 
{
	int id;
	int count;
	int blockedCount;
	int blocked[BLOCKED_MAX];
} SemaphoreStatus;

#undef BLOCKED_MAX

void sem_wait(Semaphore);
void sem_release(Semaphore);
Semaphore sem_create(long unsigned initialCount);
int sem_getId(Semaphore);
Semaphore sem_open(int id);
void sem_close(Semaphore handler);
int sem_list(SemaphoreStatus *out, size_t n);
