* Header file for fifo */

#ifndef __FIFO_H
#define __FIFO_H

#include "spinlock.h"
#include "sem.h"
#define MYFIFO_BUFSIZ 4096

struct fifo
{
	
	unsigned long data[MYFIFO_BUFSIZ];
	struct sem mutex;
	struct sem empty;
	struct sem filled;

	// Helpful indices
	int in;
	int out;
	int count;
	

} fifo;


// Initializes the fifo
void fifo_init(struct fifo *f);


// Enqueue data word d into the fifo, blocking until fifo has room to accept it
void fifo_wr(struct fifo *f, unsigned long d);


// Dequeue next data word from fifo and return it
unsigned long fifo_rd(struct fifo *f);

#endif
