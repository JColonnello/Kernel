#include <sys/semaphore.h>
#include <stdio.h>
#include <stdbool.h>
#include "test_util.h"
#include <common/test.h>

static PhyloStatus *shared = (void*)0xFFFFFFFFC0008010;  //shared memory

#define WAIT 30000000

static void printTable(Semaphore print)
{
	lock(print);
	for(int i = 0; i < shared->count; i++)
		printf(shared->table[i].eating ? "E" : ".");
	printf("\n");
	unlock(print);
}

void phylo(int pos, int leftId, int rightId)
{
	Semaphore printLock = sem_open(shared->printLock),
			waiter = sem_open(shared->waiter),
			left = sem_open(leftId),
			right = sem_open(rightId);

	while(shared->table[pos].present)
	{
		unsigned wait = WAIT + GetUniform(WAIT);
		for(int i = 0; i < wait; i++);

		//Grab forks
		lock(waiter);
		sem_wait(left);
		sem_wait(right);
		unlock(waiter);

		//Eat
		shared->table[pos].eating = true;
		wait = WAIT + GetUniform(WAIT);
		for(int i = 0; i < wait; i++);
		printTable(printLock);

		//Think
		shared->table[pos].eating = false;

		sem_release(right);
		sem_release(left);
	}

	sem_close(printLock);
	sem_close(waiter);
	sem_close(left);
	sem_close(right);
}