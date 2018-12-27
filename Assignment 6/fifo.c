/* Implementation of fifo functions */

#include "fifo.h"

void fifo_init(struct fifo *f)
{
	f->in = 0;
	f->out = 0;

	sem_init (&f->empty, MYFIFO_BUFSIZ);
	sem_init (&f->filled, 0);
	sem_init (&f->mutex, 1);

}


void fifo_wr(struct fifo *f, unsigned long d)
{
	for (;;)
	{
		sem_wait(&f->empty);		// Get empty sem

		if (sem_try(&f->mutex))
		{
			f->data[f->in] = d;
			f->in = (f->in + 1) % MYFIFO_BUFSIZ;
			sem_inc(&f->mutex);
			sem_inc(&f->filled);
			break;
		}
		else
			sem_inc(&f->empty);
	}
}

unsigned long fifo_rd(struct fifo *f)
{
	unsigned long d;

	for (;;)
	{
		sem_wait(&f->filled);	// Get filled sem

		if (sem_try(&f->mutex))
		{
			d = f->data[f->out];
			f->out = (f->out + 1) % MYFIFO_BUFSIZ;
			sem_inc(&f->mutex);
			sem_inc(&f->empty);
			break;
		}
		else
			sem_inc(&f->filled);
	}

	return d;

}
