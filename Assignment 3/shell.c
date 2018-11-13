#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <time.h>
#define BUF 4096

char **args(FILE *fin);
int parse(char **arguments);
int getNum(char **arguments);
int run(char **arguments);
int redirect(char *args, int fd, int flags, mode_t mode);

int main(int argc, char **argv)
{
	FILE *fin;
	int hakner;

	if (argc > 1)
	{
		if ((fin = fopen(argv[1], "r")) == NULL)
		{
			fprintf(stderr, "Error opening file %s: %s\n", argv[1], strerror(errno));
			return -1;
		}
	} else if (argc == 1)
		fin = stdin;

	do
	{
		printf("$ ");
		char **arg = args(fin);
		hakner = parse(arg);
		free(arg);
		printf("\n");
	} while (hakner);

	return 0;
}

char **args(FILE *fin)
{
	size_t bufsiz = BUF;
	char *line = malloc(bufsiz);
	getline(&line, &bufsiz, fin);
	if (line[strlen(line) - 1] == '\n')
		line[strlen(line) - 1] = '\0';
	char **arguments = malloc(BUF);		// Array to hold the tokens
	char *token = strtok(line, " ");
	int i = 0;
	while (token != NULL)
	{
		arguments[i] = token;
		token = strtok(NULL, " ");
		i++;
	}
	arguments[i] = NULL;

	return arguments;
}

int parse(char **arguments)
{
	if (arguments[0][0] == '#')
		return 1;

	if (!strcmp(arguments[0], "cd"))
	{
        int numArg = getNum(arguments);

		if (numArg > 2)
		{
			fprintf(stderr, "Error: Too many arguments.\nExpected format: cd [path_to_directory]\n");
			return 1;			
		} else
		{
			if(chdir(arguments[1]) == -1)
			{
				fprintf(stderr, "Error changing to directory %s: %s\n", arguments[1], strerror(errno));
				return 1;
			}
			fprintf(stdout, "Changed directory to %s\n", arguments[1]);
			return 1;	
		}			
		
	}
	else if (!strcmp(arguments[0], "exit"))
	{
		int numArg = getNum(arguments);

		if (numArg > 2)
			{
				fprintf(stderr, "Too many arguments.\nExpected format: exit [exit_value]\n");
				return 1;
			}
			else
			{
				if (arguments[1] == NULL)
					exit(0);
				else
					exit(atoi(arguments[1]));
			}
	} else
		return run(arguments);
}

int getNum(char **arguments)
{
	int size = 0;
	int i = 0;
	while(arguments[i] != NULL)
	{
		size++;
		i++;
	}
	return size;
}

int run(char **arguments)
{
	int status;
	struct rusage ru;
    struct timeval start;
    struct timeval end;
    int j = 0;

    gettimeofday(&start, NULL);
	int pid = fork();
	switch(pid)
	{
		case 0:
			// Redirection Handling 
			while (arguments[j] != NULL)
			{
				if (strstr(arguments[j], "<"))
				{
					if (redirect(arguments[j] + 1, 0, O_RDONLY, 0666))
						exit(1);
					arguments[j] = NULL;
				} else if (strstr(arguments[j], ">"))
				{
					if (redirect(arguments[j] + 1, 1, O_RDWR|O_TRUNC|O_CREAT, 0666))
						exit(1);
					arguments[j] = NULL;
				} else if (strstr(arguments[j], "2>"))
				{
					if (redirect(arguments[j] + 2, 2, O_RDWR|O_TRUNC|O_CREAT, 0666))
						exit(1);
					arguments[j] = NULL;
				} else if (strstr(arguments[j], ">>"))
				{
					if (redirect(arguments[j] + 2, 1, O_RDWR|O_APPEND|O_CREAT, 0666))
						exit(1);
					arguments[j] = NULL;
				} else if (strstr(arguments[j], "2>>"))
				{
					if (redirect(arguments[j] + 3, 2, O_RDWR|O_APPEND|O_CREAT, 0666))
						exit(1);
					arguments[j] = NULL;
				}
				j++;
			}

			if (execvp(arguments[0], arguments) == -1)
				fprintf(stderr, "Error executing %s: %s\n", arguments[0], strerror(errno));

			exit(0);

		case -1:
			fprintf(stderr, "Error in creating child process: %s\n", strerror(errno));
			break;

		default:
			if (wait3(&status, 0, &ru) != -1)
			{
				gettimeofday(&end, NULL);
				fprintf(stderr, "Run Times (Microseconds):\nReal Time:%d \nUser Time:%d \nSystem Time:%d \n", (end.tv_usec - start.tv_usec), ru.ru_utime.tv_usec, ru.ru_stime.tv_usec);
			} else
				fprintf(stderr, "Error obtaining information about child process: %s", strerror(errno));

			break;
	}

	return 1;
}

int redirect(char *args, int fd, int flags, mode_t mode)
{
	int fd_new;

	if ((fd_new = open(args, flags, mode)) < 0)
	{
		fprintf(stderr, "Error opening %s: %s\n", args, strerror(errno));
		return 1;
	}

	if ((dup2(fd_new, fd) < 0))
	{
		return 1;
	}

	if (close(fd_new) < 0)
	{
		fprintf(stderr, "Error closing fd = %d: %s\n", fd_new, strerror(errno));
		return 1;
	}

	return 0;
}
