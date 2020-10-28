#include <stdio.h>
#include <stdbool.h>

bool isvowel(char c)
{
	switch (c) 
	{
		case 'a': case 'e': case 'i': case 'o': case 'u':
		case 'A': case 'E': case 'I': case 'O': case 'U':
			return true;
		default:
			return false;
	}
}

int main()
{
	char buf[128];
	while(fgets(buf, sizeof(buf), stdin))
	{
		char out[128];
		int j = 0;
		for(int i = 0; i < sizeof(buf) && buf[i]; i++)
		{
			char c = buf[i];
			if(!isvowel(c))
				out[j++] = c;
		}
		out[j] = 0;
		printf("%s", out);
	}
	return 0;
}