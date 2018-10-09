// Leart Krasniqi
// ECE357: Computer Operating Systems
// Assignment 1, Problem 3

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int readWrite(int fdIn, int fdOut, char *buf, char *inFile, char *outFile, int bufSize);

int main(int argc, char **argv)
{
	int opt, bufSize, numIn, fdOut, fdIn;
	char *outFile = NULL;
	char *inFile = NULL;

	bufSize = 1024;

	while ((opt = getopt(argc, argv, "b:o:")) != -1)
	{
		switch (opt)
		{
			case 'b':
				bufSize = atoi(optarg);
				break;
			case 'o':
				outFile = optarg;
				break;
			default:
				exit(EXIT_FAILURE);
		}
	}

	char *buf = malloc((sizeof(char)) * bufSize);

	// Opening output file
	if (outFile != NULL)
	{
		fdOut = open(outFile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
		if (fdOut < 0)
		{
			fprintf(stderr, "Error writing to %s: %s", outFile, strerror(errno));
			return -1;
		}
	} else
	{
		fdOut = STDOUT_FILENO;		// If no output file given, output to terminal
	}

	numIn = argc - optind;
	if (numIn == 0)
	{	
		fdIn = STDIN_FILENO;
		inFile = "Standard Input";
		readWrite(fdIn, fdOut, buf, inFile, outFile, bufSize);
	}
	else
	{
		for (int i = 0; i < numIn; i++)
		{
			inFile = argv[optind + i];
			if (strcmp(inFile, "-") == 0)
			{	
				inFile = "Standard Input";
				fdIn = STDIN_FILENO;
			} else if ((fdIn = open(inFile, O_RDONLY)) < 0)
			{
				fprintf(stderr, "Error reading from %s: %s", inFile, strerror(errno));
				return -1;
			}
			readWrite(fdIn, fdOut, buf, inFile, outFile, bufSize);
		}
	}

	// close output file
	if (fdOut != STDOUT_FILENO)
	{
		if (close(fdOut) < 0)
		{
			fprintf(stderr, "Error closing output file %s: %s", outFile, strerror(errno));
		}
	}
	free(buf);
	return 0;
}

int readWrite(int fdIn, int fdOut, char *buf, char *inFile, char *outFile, int bufSize)
{
	int bytesRead, bytesWritten;
	while ((bytesRead = read(fdIn, buf, sizeof(char)*bufSize)) != 0)
		{
			if (bytesRead < 0)
			{
				fprintf(stderr, "Error reading from %s: %s", inFile, strerror(errno));
			} else
			{
				bytesWritten = write(fdOut, buf, bytesRead);
				if (bytesWritten < 0)
				{	
					fprintf(stderr, "Error writing to %s: %s\n", outFile, strerror(errno));
					return -1;
				} else if (bytesWritten != bytesRead)
				{
					bytesRead -= bytesWritten;
					bytesWritten = write(fdOut, buf + bytesWritten, bytesRead);  // Try to rewrite missing bytes
					if (bytesWritten < 0)
					{
						fprintf(stderr, "Error writing to %s: %s\n", outFile, strerror(errno));
						return -1;
					}
				}
			}
		}

		// close input file
		if (fdIn != STDIN_FILENO)
		{
			if (close(fdIn) < 0)
			{
				fprintf(stderr, "Error closing input file %s: %s", inFile, strerror(errno));
				return -1;
			}
		} 
}
