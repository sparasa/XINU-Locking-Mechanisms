/* sdelete.c - sdelete */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sem.h>
#include <stdio.h>
#include <lock.h>

/*------------------------------------------------------------------------
 *  * sdelete  --  delete a semaphore by releasing its table entry
 *   *------------------------------------------------------------------------
 *    */
int ldelete(int lockdes)
{
        STATWORD ps;
        int     pid;
        struct  lentry  *lptr;

        disable(ps);
        if (isbadsem(lockdes) || locker[lockdes].lstate==SFREE) {
                restore(ps);
                return(SYSERR);
	}
        lptr = &locker[lockdes];
        lptr->lstate = LFREE;
        if (nonempty(lptr->lqhead)) {
                while( (pid=getfirst(lptr->lqhead)) != EMPTY)
                  {
                    	proctab[pid].pwaitret = DELETED;
                    	ready(pid,RESCHNO);
                  }
                resched();
        }
	pid=0;
	while(pid<NPROC){
		if(locker[lockdes].proc[pid]==1){
			locker[lockdes].proc[pid]=-1;
		}
		pid++;
	}
        restore(ps);
        return(OK);
}
