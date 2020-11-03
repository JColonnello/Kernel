#pragma once
#include <stddef.h>
#define lock(x) sem_wait((x))
#define unlock(x) sem_release((x))

typedef struct Semaphore Semaphore;
typedef struct 
{
	int id;
	int count;
} SemaphoreStatus;

void sem_wait(Semaphore*);
void sem_release(Semaphore*);
Semaphore *sem_create(long unsigned initialCount);
int sem_getId(Semaphore*);
Semaphore *sem_open(int id);
void sem_close(Semaphore *handler);
Semaphore *sem_dup(Semaphore *);
int sem_list(SemaphoreStatus *out, size_t n);
