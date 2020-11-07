#include <stdint.h>
#include <stdio.h>
#include <sys/process.h>
#include <sys/semaphore.h>
#include <stdlib.h>
#include <string.h>

uint64_t my_create_process(char * name, char *args[]){
	return execve("userland/test_sync.bin", args, NULL);
}

Semaphore my_sem_open(int id){
	return sem_open(id);
}

uint64_t my_sem_wait(Semaphore sem){
	sem_wait(sem);
	return 0;
}

uint64_t my_sem_post(Semaphore sem){
	sem_release(sem);
	return 0;
}

uint64_t my_sem_close(Semaphore sem){
	sem_close(sem);
	return 0;
}

#define TOTAL_PAIR_PROCESSES 2

int64_t *global = (void*)0xFFFFFFFFC0008000;  //shared memory

void slowInc(int64_t *p, int64_t inc){
	int64_t aux = *p;
	aux += inc;
	yield();
	*p = aux;
}

void inc(int sem_id, int64_t value, uint64_t N) {
	uint64_t i;

	Semaphore sem;
	if (sem_id > 0 && (sem = my_sem_open(sem_id)) < 0) {
		printf("ERROR OPENING SEM %d\n", sem_id);
		return;
	}

	for (i = 0; i < N; i++){
		if (sem)
			my_sem_wait(sem);
		slowInc(global, value);
		if (sem)
			my_sem_post(sem);
	}

	if (sem)
		my_sem_close(sem);

	printf("Final value: %d\n", *global);
}

char idstr[5];
char *plusargs[] = {idstr, "1", "1000000", NULL};
char *minusargs[] = {idstr, "-1", "1000000", NULL};

void test_sync(){
	*global = 0;

	Semaphore sem= sem_create(1);
	int id = sem_getId(sem);
	printf("Semaphore: %d\n", id);
	sprintf(idstr, "%d", id);
	processPriority(getpid(), 5);

	printf("CREATING PROCESSES...(WITH SEM)\n");
	int pids[TOTAL_PAIR_PROCESSES*2];
	for(int i = 0, count = 0; i < TOTAL_PAIR_PROCESSES; i++){
		pids[count++] = my_create_process("inc", plusargs);
		pids[count++] = my_create_process("inc", minusargs);
	}

	for(int i = 0; i < TOTAL_PAIR_PROCESSES * 2; i++)
	{
		while(ispidrun(pids[i]))
			yield();
	}
	my_sem_close(sem);
}

void test_no_sync(){
	*global = 0;
	minusargs[0] = plusargs[0] = "-1";

	printf("CREATING PROCESSES...(WITHOUT SEM)\n");

	int pids[TOTAL_PAIR_PROCESSES*2];
	for(int i = 0, count = 0; i < TOTAL_PAIR_PROCESSES; i++){
		pids[count++] = my_create_process("inc", plusargs);
		pids[count++] = my_create_process("inc", minusargs);
	}
	for(int i = 0; i < TOTAL_PAIR_PROCESSES * 2; i++)
		while(ispidrun(pids[i]))
			yield();
}

int main(int argc, char *args[]){
	if(argc >= 3)
	{
		int sem_id;
		int64_t value;
		uint64_t N;

		sscanf(args[0], "%d", &sem_id);
		sscanf(args[1], "%ld", &value);
		sscanf(args[2], "%lu", &N);
		inc(sem_id, value, N);
		return 0;
	}
	else if(argc == 1 && strcmp(args[0], "nosync") == 0)
	{
		fprintf(stderr, "No sync\n");
		test_no_sync();
	}
	else
		test_sync();
	return 0;
}
