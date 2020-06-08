#include <stdio.h>
#include <stdlib.h>

int main() {
	printf("Begin\n");
	char *buf = malloc(18000);
	sprintf(buf, "Hola\n");
	printf("%s", buf);
	free(buf);
	return 0;
}