#pragma once
#define lock(x) sem_wait((x))
#define unlock(x) sem_release((x))

typedef struct Semaphore Semaphore;

void sem_wait(Semaphore*);
void sem_release(Semaphore*);
Semaphore *sem_create(long unsigned initialCount);
int sem_getId(Semaphore*);
Semaphore *sem_open(int id);
void sem_close(Semaphore *handler);