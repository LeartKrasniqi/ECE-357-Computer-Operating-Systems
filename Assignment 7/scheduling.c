/* Nice value test */

#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <time.h>

int main(int argc, char **argv)
{
	if (argc != 5)
	{
		fprintf(stderr, "Error: Excpected input: ./cputimes PROCESSES NICE_VALUE SLEEP_SECONDS FILE\n");
		return -1;
	}

	int *children = malloc(sizeof(int)*atoi(argv[1]));

	int parent_pid = getpid();

	for (int i = 0; i < atoi(argv[1]); i++)
	{
		if ( (children[i] = fork()) < 0)
		{
			fprintf(stderr, "Error forking: %s\n", strerror(errno));
			return -1;
		}

		if (children[i] == 0)
		{
			// Change nice value of third child
			if (i == 2)
				nice(atoi(argv[2]));

			int blah = 0;
			while(blah != INT_MAX)
				blah++;
		}
	}

	sleep(atoi(argv[3]));
	signal(SIGQUIT, SIG_IGN);
	kill(-parent_pid, SIGQUIT);

	long user_sec = 0;
	long user_usec = 0;
	long sys_sec = 0;
	long sys_usec = 0;

	FILE *f = fopen(argv[4], "w");

	for (int i = 0; i < atoi(argv[1]); i++)
	{
		struct rusage ru;

		if (wait4(children[i], NULL, 0, &ru) < 0) 
			{ 
				fprintf (stderr, "Error waiting for child processes: %s\n", strerror(errno));
				return -1;
			}
			
		if (i == 2)
			fprintf(f, "%ld.%06ld,%ld.%06ld,", ru.ru_utime.tv_sec, ru.ru_utime.tv_usec, ru.ru_stime.tv_sec, ru.ru_stime.tv_usec);

		user_sec += ru.ru_utime.tv_sec;
		user_usec += ru.ru_utime.tv_usec;
		sys_sec += ru.ru_stime.tv_sec;
		sys_usec += ru.ru_stime.tv_usec;
	}

	fprintf(f, "%ld.%06ld,%ld.%06ld\n", user_sec, user_usec, sys_sec, sys_usec);

	fclose(f);

	return 0;

}
