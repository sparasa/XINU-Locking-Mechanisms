/* this function is used to initialize semaphore values */
#include <lock.h>
struct  lentry  locker[NLOCK]; // process table
int linit(){
	struct lentry *lptr;
	int i;
	for (i=0 ; i<NLOCK ; i++) {      /* initialize semaphores */
        	(lptr = &locker[i])->lstate = LFREE;
        	lptr->lqtail = 1 + (lptr->lqhead = newqueue());
        }
	return(0);
}
