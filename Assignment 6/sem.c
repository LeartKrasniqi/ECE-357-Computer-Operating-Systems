/* Function Implementation for Semaphore */

#include "sem.h"
#include "spinlock.h"


void handler(int signum) {}


void sem_init(struct sem *s, int c)
{

	s->count = c;
	s->index = -1;	//Index of -1 means no waiting processes
	s->lock = 0;

	sigfillset(&s->sigmask);
	sigdelset(&s->sigmask, SIGINT);		// Debugging purposes...
	sigdelset(&s->sigmask, SIGUSR1);
	signal(SIGUSR1, handler);
}

// Attempt to atomically decrement the semaphore (if possible)
int sem_try(struct sem *s)
{
	spin_lock(&s->lock);	
	if (s->count > 0)
	{
		s->count--;
		spin_unlock(&s->lock);	
		return 1;
	}
	else
	{
		spin_unlock(&s->lock);	
		return 0;
	}
}

void sem_wait(struct sem *s)
{
	for(;;)
	{
		spin_lock(&s->lock);	
		if (s->count > 0)
		{
			s->count--;
			spin_unlock(&s->lock);
			break;
		}
		else
		{
			s->waiting[s->index] = vpn;	 // Record which VPN is being blocked
			s->index++;
			spin_unlock(&s->lock);	
			sigsuspend(&s->sigmask);
		}
	}
}


void sem_inc(struct sem *s)
{
	spin_lock(&s->lock);	
	s->count += 1;

	// If we go from 0 to 1, then wake up the waiting VPNs
	if(s->count == 1)
	{
		while (s->index != -1)
		{
				kill(s->pid[s->waiting[s->index]], SIGUSR1);
				s->index--;
		}
	}

	spin_unlock(&s->lock);

}
