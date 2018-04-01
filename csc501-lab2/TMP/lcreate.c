#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sem.h>
#include <stdio.h>
#include <lock.h>

LOCAL int newlock();

/*------------------------------------------------------------------------
 * screate  --  create and initialize a semaphore, returning its id
 *------------------------------------------------------------------------
 */
SYSCALL lcreate()
{
	STATWORD ps;    
	int	lockers;

	disable(ps);
	if ( (lockers=newlock())==SYSERR ) {
		restore(ps);
		return(SYSERR);
	}
	//semaph[sem].semcnt = count;
	/* sqhead and sqtail were initialized at system startup */
	restore(ps);
	return(lockers);
}

/*------------------------------------------------------------------------
 * newlock  --  allocate an unused lock and return its index
 *------------------------------------------------------------------------
 */
LOCAL int newlock()
{
	int	lockers;
	int	i;

	for (i=0 ; i<NLOCK ; i++) {
		lockers=nextlock--;
		if (nextlock < 0)
			nextlock = NLOCK-1;
		if (locker[lockers].lstate==LFREE) {
			locker[lockers].lstate = LUSED;
			locker[lockers].readcount=0;
			return(lockers);
		}
	}
	return(SYSERR);
}
