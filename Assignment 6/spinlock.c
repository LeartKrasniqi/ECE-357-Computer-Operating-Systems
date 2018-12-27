/* Declaring functions in spinlock header file */

#include "spinlock.h"

void spin_lock(volatile char *l)
{
	while (tas(l))
		;
}

void spin_unlock(volatile char *l)
{
	*l = 0;
}
