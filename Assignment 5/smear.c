/*
Leart Krasniqi
ECE357: Operating Systems
Prof. Hakner
PS 5 -- Problem 3
*/
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
	if (argc < 4)
	{
		fprintf(stderr, "Error: Too few arguments.  Syntax is:\n./smear TARGET REPLACEMENT file1 {file2...}\n");
		exit(1);
	}

	if (strlen(argv[1]) != strlen(argv[2]))
	{
		fprintf(stderr, "Error: TARGET size must match REPLACEMENT size\n");
		exit(1);
	}

	int target_size = strlen(argv[1]);

	for (int i = 3; i < argc; i++)
	{
		int fdin;
		char *membuffer; 
		char *buf;
		char *temp;
		struct stat statbuf;
		off_t fsize;

		if ((fdin = open(argv[i], O_RDWR)) < 0)
		{
			fprintf(stderr, "Error opening %s: %s\n", argv[i], strerror(errno));
			return -1;
		}

		if (fstat(fdin, &statbuf))
		{
			fprintf(stderr, "Error obtaining fstat info for %s: %s\n", argv[i], strerror(errno));
			return -1;
		}

		fsize = statbuf.st_size;

		if ((membuffer = mmap(NULL, fsize, PROT_READ | PROT_WRITE, MAP_SHARED, fdin, 0)) == MAP_FAILED)
		{
			fprintf(stderr, "Error trying to map %s into memory: %s\n", argv[i], strerror(errno));
			return -1;
		}

		if ((temp = malloc(sizeof(char) * strlen(membuffer))) == NULL)
		{
			fprintf(stderr, "Error creating temp array for copying: %s\n", strerror(errno));
			return -1;
		}

		while ((buf = strstr(membuffer, argv[1])) != NULL)
		{
			strncpy(temp, membuffer, buf - membuffer);
			temp[buf - membuffer] = '\0';
			strcat(temp, argv[2]);
        	strcat(temp, buf + target_size);
        	strcpy(membuffer, temp);
        	buf++; 	
		}
		
		free(temp);

		if (munmap(membuffer, fsize) < 0)
		{
			fprintf(stderr, "Error deleting mapping for %s: %s\n", argv[i], strerror(errno));
			return -1;
		}
		
		close(fdin);
	}

	return 0;
}
