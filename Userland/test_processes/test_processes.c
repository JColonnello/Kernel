#include <stdint.h>
#include <stdio.h>
#include "test_util.h"
#include <sys/process.h>
#include <stdlib.h>
#include <string.h>

//TO BE INCLUDED
void endless_loop(int main){
	while(ispidrun(main));
}

uint64_t my_create_process(char * name){
	char pid[10];
	sprintf(pid, "%d", getpid());
	char *args[] = {"endless_loop", pid, NULL};
	return execve("userland/test_processes.bin", args, NULL);
}

uint64_t my_kill(uint64_t pid){
	return kill(pid);
}

uint64_t my_block(uint64_t pid){
	return block(pid);
}

uint64_t my_unblock(uint64_t pid){
	return block(pid);
}

#define MAX_PROCESSES 50 //Should be around 80% of the the processes handled by the kernel

enum State {ERROR, RUNNING, BLOCKED, KILLED};

typedef struct P_rq{
	uint32_t pid;
	enum State state;
}p_rq;

void test_processes(){
	p_rq p_rqs[MAX_PROCESSES];
	uint8_t rq;
	uint8_t alive = 0;
	uint8_t action;

	printf("Main process: %d\n", getpid());
	while (1){

		// Create MAX_PROCESSES processes
		for(rq = 0; rq < MAX_PROCESSES; rq++){
			p_rqs[rq].pid = my_create_process("endless_loop");  // TODO: Port this call as required

			if (p_rqs[rq].pid == -1){                           // TODO: Port this as required
				printf("Error creating process\n");               // TODO: Port this as required
				return;
			}else{
				p_rqs[rq].state = RUNNING;
				alive++;
			}
		}

		// Randomly kills, blocks or unblocks processes until every one has been killed
		while (alive > 0){

			for(rq = 0; rq < MAX_PROCESSES; rq++){
				action = GetUniform(2) % 2; 

				switch(action){
					case 0:
						if (p_rqs[rq].state == RUNNING || p_rqs[rq].state == BLOCKED){
							if (my_kill(p_rqs[rq].pid) == -1){          // TODO: Port this as required
								printf("Error killing process\n");        // TODO: Port this as required
								return;
							}
							p_rqs[rq].state = KILLED; 
							alive--;
						}
						break;

					case 1:
						if (p_rqs[rq].state == RUNNING){
							if(my_block(p_rqs[rq].pid) == -1){          // TODO: Port this as required
								printf("Error blocking process\n");       // TODO: Port this as required
								return;
							}
							p_rqs[rq].state = BLOCKED; 
						}
						break;
				}
			}

			// Randomly unblocks processes
			for(rq = 0; rq < MAX_PROCESSES; rq++)
				if (p_rqs[rq].state == BLOCKED && GetUniform(2) % 2){
					if(my_unblock(p_rqs[rq].pid) == -1){            // TODO: Port this as required
						printf("Error unblocking process\n");         // TODO: Port this as required
						return;
					}
					p_rqs[rq].state = RUNNING; 
				}
		} 
	}
}

int main(int argc, char **args){
	if(argc >= 2 && strcmp(args[0], "endless_loop") == 0)
	{
		int main;
		sscanf(args[1], "%d", &main);
		endless_loop(main);
	}
	else
		test_processes();
	return 0;
}
