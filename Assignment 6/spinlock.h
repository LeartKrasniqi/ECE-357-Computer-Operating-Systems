/* Spinlock Header File */
#ifndef __SPINLOCK_H
#define __SPINLOCK_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

int tas(volatile char *lock);
void spin_lock(volatile char *lock);
void spin_unlock(volatile char *lock);

#endif
