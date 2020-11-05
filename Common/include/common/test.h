#pragma once
#include <stdbool.h>

#define MAX_PHYLO 16

typedef struct
{
	int count;
	struct
	{
		bool present;
		bool eating;
	} table[MAX_PHYLO];
	int printLock;
	int waiter;
} PhyloStatus;
