#include <stdio.h>

void main(int argc, char *args[])
{
	char buf[128];

	if(argc == 0)
		while(fgets(buf, sizeof(buf), stdin))
			printf("%s", buf);
	else
	{
		for(int i = 0; i < argc; i++)
		{
			FILE *file = fopen(args[i], "r");
			if(file == NULL)
				fprintf(stderr, "File not found: %s", args[i]);
			int count;
			while((count = fread(buf, 1, sizeof(buf), file)) != 0)
				fwrite(buf, 1, count, stdout);
			fclose(file);
		}
	}
}