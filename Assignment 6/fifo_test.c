/* Test of the fifo */

#include "spinlock.h"
#include "sem.h"
#include "fifo.h"
//#define N_WRITERS 8
//#define N_ITER 5

int main(int argc, char **argv)
{
	if (argc != 3)
	{
		fprintf(stderr, "Error: Expected input is ./a.out N_WRITERS N_ITER\n");
		return -1;
	}

	int N_WRITERS = atoi(argv[1]);
	int N_ITER = atoi(argv[2]);

	// Files for testing the output
	FILE *w_file = fopen("writers.txt", "w");
	FILE *r_file = fopen("readers.txt", "w");

	struct fifo *f;
	unsigned long data;

	if ((f = mmap(NULL, sizeof(struct fifo), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, 0, 0)) == MAP_FAILED)
	{
			fprintf(stderr, "Error trying to map memory: %s\n", strerror(errno));
			return -1;
	};


	fifo_init(f);
	
	int *pid = malloc(sizeof(int)*N_WRITERS);
	if (pid == NULL)
	{
		fprintf(stderr, "Error allocating memory for pid table: %s\n", strerror(errno));
		return -1;
	}

	// Writers
	for (int i = 0; i < N_WRITERS; i++)
	{
		if (( pid[i] = fork() ) < 0) 
		{
			fprintf (stderr, "Error using fork() with writer: %s\n", strerror(errno));
			return -1;
		} 

		if (pid[i] == 0)
		{
			vpn = i;	// Virtual Process Number (Comes from sem.h)

			// Use PID to identify writer contents in FIFO
			for (int j = 0; j < N_ITER; j++)
			{
				data = j + getpid()*10000;
				fifo_wr(f, data);
				//fprintf(stdout, "VPN %d wrote %lu to fifo\n", i, data);
				fprintf(w_file, "%lu\n", data);
			}
			return 0;
		}
	}

	// Create a reader
	int pid_reader = fork();

	switch (pid_reader)
	{
		case -1:
			fprintf(stderr, "Error using fork() with reader: %s\n", strerror(errno));
			return -1;

		case 0:
			vpn = N_WRITERS;
			for (int i = 0; i < N_WRITERS * N_ITER; i++) 
				fprintf(r_file, "%lu\n", fifo_rd(f));

			return 0;
	}

	// Only parent process should get here
	for (int i = 0; i < N_WRITERS; i++)
	{
		if (waitpid(pid[i], NULL, 0) < 0) 
			{ 
				fprintf (stderr, "Error waiting for child processes: %s\n", strerror(errno));
				return -1;
			}
	}

	fclose(w_file);
	fclose(r_file);

	return 0;
}
