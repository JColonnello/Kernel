#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "test_util.h"

#define MAX_BLOCKS 128
#define MAX_MEMORY (1<<25) //Should be around 80% of memory managed by the MM

typedef struct MM_rq{
	void *address;
	uint32_t size;
}mm_rq;

void test_mm(){
	mm_rq mm_rqs[MAX_BLOCKS];

	while (1){
		uint8_t rq = 0;
		uint32_t total = 0;

		// Request as many blocks as we can
		while(rq < MAX_BLOCKS && total < MAX_MEMORY){
			mm_rqs[rq].size = GetUniform(MAX_MEMORY - total - 1) + 1;
			mm_rqs[rq].address = malloc(mm_rqs[rq].size); // TODO: Port this call as required
			if(mm_rqs[rq].address == NULL)
			{
				printf("Null\n");
				break;
			}
			total += mm_rqs[rq].size;
			rq++;
		}

		printf("Set\n");
		// Set
		uint32_t i;
		for (i = 0; i < rq; i++)
			if (mm_rqs[i].address != NULL)
				memset(mm_rqs[i].address, i, mm_rqs[i].size); // TODO: Port this call as required

		printf("Check\n");
		// Check
		for (i = 0; i < rq; i++)
			if (mm_rqs[i].address != NULL)
				if(!memcheck(mm_rqs[i].address, i, mm_rqs[i].size))
					printf("ERROR!\n"); // TODO: Port this call as required

		printf("Free\n");
		// Free
		for (i = 0; i < rq; i++)
			if (mm_rqs[i].address != NULL)
				free(mm_rqs[i].address);  // TODO: Port this call as required
	} 
}

int main(){
	test_mm();
	return 0;
}
