/* Test of spinlock */

#include "spinlock.h"

#define CORES 4
#define ITER 5000000


int main(int argc, char **argv)
{
	int *safe_membuf;
	int *unsafe_membuf;
	int pid[CORES];
	int child = 0;
	char *lock;

	if ((safe_membuf = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, 0, 0)) == MAP_FAILED)
		{
			fprintf(stderr, "Error trying to map memory: %s\n", strerror(errno));
			return -1;
		}

	if ((unsafe_membuf = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, 0, 0)) == MAP_FAILED)
		{
			fprintf(stderr, "Error trying to map memory: %s\n", strerror(errno));
			return -1;
		}

	safe_membuf[0] = 0;
	unsafe_membuf[0] = 0;
	lock = (char *)&safe_membuf[1];


	// Spawns multiple children for testing
	for (int i = 0; i < CORES; i++) 
	{
		if (( pid[i] = fork() ) < 0) 
		{
			fprintf (stderr, "Error using fork(): %s\n", strerror(errno));
			return -1;
		} 
		else if (pid[i] == 0) 
		{
			child = 1;
			break;
		} 
		else 
			continue;
	}

	if (child)
	{
		for (int i = 0; i < ITER; i++) 
			unsafe_membuf[0] += 1;

		for (int i = 0; i < ITER; i++) 
		{
			
			/* BEGIN CRITICAL REGION */

			spin_lock(lock);
			safe_membuf[0] += 1;
			spin_unlock(lock);

			/* END CRITICAL REGION */

		}
		
		return 0;
	}
	else
	{
		for (int i = 0; i < CORES; i++) 
		{
			if (waitpid(pid[i], NULL, 0) < 0) 
			{ 
				fprintf (stderr, "Error waiting for child processes: %s\n", strerror(errno));
				return -1;
			}
		}
	}

	fprintf(stdout, "Desired value: %d\n", CORES * ITER);
	fprintf(stdout, "Safe Region value: %d\n", *safe_membuf);
	fprintf(stdout, "Unsafe Region value: %d\n", *unsafe_membuf );

	return 0;
}
