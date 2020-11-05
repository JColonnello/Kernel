#include "common/test.h"
#include <syncro/semaphore.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/process.h>

extern void phylo(int pos, int leftId, int rightId);

static PhyloStatus *shared = (void*)0xFFFFFFFFC0008010;  //shared memory
static int pids[MAX_PHYLO] = {0};
static Semaphore *forks[MAX_PHYLO] = {0};
static int count = 2;

static void setArgs(char *args[static 4], char *buf, int seat, int left, int right)
{
	int pos = 0;
	pos += sprintf((args[0] = &buf[pos]), "%d", seat);
	buf[pos++] = 0;
	pos += sprintf((args[1] = &buf[pos]), "%d", left);
	buf[pos++] = 0;
	pos += sprintf((args[2] = &buf[pos]), "%d", right);
	buf[pos++] = 0;
	args[3] = NULL;
}

static void addPhylo()
{
	if(count == MAX_PHYLO)
		return;

	shared->table[count-1].present = false;
	while(ispidrun(pids[count-1]))
		yield();

	char *args[4];
	char buf[64];
	forks[count] = sem_create(1);

	setArgs(args, buf, count-1, sem_getId(forks[count-1]), sem_getId(forks[count]));
	shared->table[count-1].present = true;
	pids[count-1] = execve("userland/phylo.bin", args, NULL);

	setArgs(args, buf, count, sem_getId(forks[count]), sem_getId(forks[0]));
	shared->table[count].present = true;
	shared->table[count].eating = false;
	pids[count] = execve("userland/phylo.bin", args, NULL);
	
	count++;
	shared->count = count;
}

static void removePhylo()
{
	if(count == 2)
		return;

	count--;
	shared->count = count;

	shared->table[count].present = false;
	shared->table[count-1].present = false;
	while(ispidrun(pids[count]))
		yield();
	while(ispidrun(pids[count-1]))
		yield();

	char *args[4];
	char buf[64];
	sem_close(forks[count]);

	setArgs(args, buf, count-1, sem_getId(forks[count-1]), sem_getId(forks[0]));
	shared->table[count-1].present = true;
	pids[count-1] = execve("userland/phylo.bin", args, NULL);
}

static void loop()
{
	Semaphore *printLock = sem_create(1);
	shared->printLock = sem_getId(printLock);
	Semaphore *waiter = sem_create(1);
	shared->waiter = sem_getId(waiter);

	for(int i = 0; i < count; i++)
		forks[i] = sem_create(1);


	shared->count = count;	
	for(int i = 0; i < count; i++)
	{
		char *args[4];
		char buf[64];

		setArgs(args, buf, i, sem_getId(forks[i]), sem_getId(forks[(i+1)%count]));
		shared->table[i].eating = false;
		shared->table[i].present = true;
		pids[i] = execve("userland/phylo.bin", args, NULL);
	}

	bool running = true;
	while(running)
	{
		char c = getchar();
		switch(c)
		{
			case 'q':
				running = false;
				break;
			case 'a':
				addPhylo();
				break;
			case 'r':
				removePhylo();
				break;
		}
	}

	for(int i = 0; i < count; i++)
	{
		shared->table[i].present = false;
		sem_close(forks[i]);
	}
	sem_close(printLock);
	sem_close(waiter);
}

void main(int argc, char *args[])
{
	if(argc == 3)
	{
		int pos, left, right;
		sscanf(args[0], "%d", &pos);
		sscanf(args[1], "%d", &left);
		sscanf(args[2], "%d", &right);
		phylo(pos, left, right);
	}
	else
		loop();
}