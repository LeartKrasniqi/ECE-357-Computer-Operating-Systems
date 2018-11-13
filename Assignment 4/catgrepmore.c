#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/signal.h>
#include <setjmp.h>
#define BUFSIZE 4096


int numfile, numbyte;
jmp_buf int_jb;

void handler(int s)
{
	if (s == SIGINT)
	{
		fprintf(stderr, "Number of Files Processed: %d\nNumber of Bytes Processed: %d\n", numfile, numbyte);
		exit(1);
	}
}

void pipe_handler(int s)
{
	longjmp(int_jb, 1);
}


int main(int argc, char **argv)
{
	if (argc < 3)
	{
		fprintf(stderr, "Error: Too few inputs.  Expected format is:\n ./catgrepmore [pattern] [infile1]\n");
		exit(1);
	}


	struct sigaction sa1;
	sa1.sa_handler = handler;
	sa1.sa_flags = 0;
	sigemptyset(&sa1.sa_mask);
	if (sigaction(SIGINT, &sa1, 0) == -1)
	{
		fprintf(stderr, "Error invoking sigaction: %s", strerror(errno));
		return -1;
	}

	struct sigaction sa2;
	sa2.sa_handler = pipe_handler;
	sa2.sa_flags = 0;
	sigemptyset(&sa2.sa_mask);

	if (sigaction(SIGPIPE, &sa2, 0) == -1)
	{
		fprintf(stderr, "Error invoking sigaction: %s", strerror(errno));
		return -1;
	}


	for (int i = 2; i < argc; i++)
	{

		int fdIn;
		if ((fdIn = open(argv[i], O_RDONLY)) < 0)
		{
			fprintf(stderr, "Error opening file %s for reading: %s\n", argv[i], strerror(errno));
			return -1;
		}

		int fdPipe1[2], fdPipe2[2];

		if (pipe(fdPipe1) < 0)
		{
			fprintf(stderr, "Error creating pipe for INFILE <-> grep: %s\n", strerror(errno));
			return -1;
		}

		if (pipe(fdPipe2) < 0)
		{
			fprintf(stderr, "Error creating pipe for grep <-> more: %s\n", strerror(errno));
			return -1;
		}

		int pid1 = fork();

		switch (pid1)
		{
			case -1:
				fprintf(stderr, "Error in creating child process: %s\n", strerror(errno));
				break;

			case 0:
				// child one code
				if (dup2(fdPipe1[0], 0) < 0)
				{
					fprintf(stderr, "Error duping STDIN (grep) and fd # %d :%s\n", fdPipe1[0], strerror(errno));
					return -1;
				}

				if (dup2(fdPipe2[1], 1) < 0)
				{
					fprintf(stderr, "Error duping STDOUT (grep) and fd # %d :%s\n", fdPipe2[1], strerror(errno));
					return -1;
				}


				close(fdIn);		// Closes unneeded reference to input file
				close(fdPipe1[0]);	// Closes extra copy of reference to read side of pipe1
				close(fdPipe1[1]);	// Closes unneeded reference to write side of pipe1
				close(fdPipe2[0]);	// Closes unneeded reference to read side of pipe2
				close(fdPipe2[1]);	// Closes extra copy of reference to write side of pipe2 

				if (execlp("grep", "grep", argv[1], NULL) == -1)
					fprintf(stderr, "Error executing grep command: %s\n", strerror(errno));

				return -1;

		}

		int pid2 = fork();

		switch (pid2)
		{
			case -1: 
				fprintf(stderr, "Error in creating child process: %s\n", strerror(errno));
				break;

			case 0:
				// child two code
				if (dup2(fdPipe2[0], 0) < 0)
				{
					fprintf(stderr, "Error duping STDIN (more) and fd # %d : %s\n", fdPipe2[0], strerror(errno));
					return -1;
				}


				close(fdIn);		// Closes unneeded reference to input file
				close(fdPipe1[0]);	// Closes unneeded reference to pipe1
				close(fdPipe1[1]);	// Closes unneeded reference to pipe1
				close(fdPipe2[0]);	// Closes extra copy of reference to read side of pipe2
				close(fdPipe2[1]);	// Closes unneeded reference to write side of pipe2

				if (execlp("more", "more", NULL) == -1)
					fprintf(stderr, "Error executing more command: %s\n", strerror(errno));

				return -1;
		}

		char *buf[BUFSIZE];
		int r, w;

		while ((r = read(fdIn, buf, BUFSIZE)) != 0) 
		{
			if (r < 0)
			{
				fprintf(stderr, "Error reading from file %s: %s\n", argv[i], strerror(errno));
				return -1;
			} else
			{
				w = write(fdPipe1[1], buf, r);

				while (w < r)
				{
					if (w < 0)
					{
						fprintf(stderr, "Error writing from buffer to pipe: %s\n", strerror(errno));
						return -1;
					}
					w += write(fdPipe1[1], buf + w, r - w);
				}
			}
		}

		close(fdPipe1[0]);	// Closes unneeded reference to read side of pipe1
		close(fdPipe1[1]);	// Closes extra copy of reference to write side of pipe1
		close(fdPipe2[0]);	// Closes unneeded reference to pipe2
		close(fdPipe2[1]);	// Closes unneeded reference to pipe2
		close(fdIn);		// Closes input file

		numfile++;
		numbyte += w; 

		int status1, status2;
		int pidgrep = waitpid(pid1, &status1, 0);
		int pidmore = waitpid(pid2, &status2, 0);

		setjmp(int_jb);
	}

	return 0;

}
