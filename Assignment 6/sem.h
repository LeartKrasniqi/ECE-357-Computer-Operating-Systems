/* Header File for Semaphore */

#ifndef __SEM_H
#define __SEM_H
#define N_PROC 64
#include "spinlock.h"

int vpn;	// Virtual Process Number

struct sem
{
	int count;
	char lock;
	int index;
	int waiting[N_PROC];
	int pid[N_PROC];
	sigset_t sigmask;

} sem;

// Initializes semaphore with initial count
void sem_init(struct sem *s, int c);

// Attempt to perform 'P' operation.  If this operation
// would block, return 0.
int sem_try(struct sem *s);

// Perform the 'P' operation
void sem_wait(struct sem *s);

// Perform the 'V' operation.
void sem_inc(struct sem *s);

#endif
