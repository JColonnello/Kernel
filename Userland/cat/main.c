#include <stdio.h>

int main()
{
	char buf[128];
	while(fgets(buf, sizeof(buf), stdin))
		printf("%s", buf);
	return 0;
}