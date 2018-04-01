/* kill.c - kill */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <q.h>
#include <stdio.h>
#include<lock.h>

/*------------------------------------------------------------------------
 * kill  --  kill a process and remove it from the system
 *------------------------------------------------------------------------
 */
SYSCALL kill(int pid)
{
	STATWORD ps;    
	struct	pentry	*pptr;		/* points to proc. table for pid*/
	int	dev;
	int ldes1;
	struct lentry *lptr;

	disable(ps);
	if (isbadpid(pid) || (pptr= &proctab[pid])->pstate==PRFREE) {
		restore(ps);
		return(SYSERR);
	}

	if (--numproc == 0)
		xdone();

	dev = pptr->pdevs[0];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->pdevs[1];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->ppagedev;
	if (! isbaddev(dev) )
		close(dev);
	
	send(pptr->pnxtkin, pid);

	freestk(pptr->pbase, pptr->pstklen);
	switch (pptr->pstate) {

	case PRCURR:	pptr->pstate = PRFREE;	/* suicide */
			resched();

	case PRWAIT:	ldes1=proctab[pid].lockid;
			lptr= &locker[ldes1];
			//kprintf("lock descriptor:%d\n",ldes1);
			//locker[ldes1].procmask[currpid]==0;
			//semaph[pptr->psem].semcnt++;
			//kprintf("entered into kill\n");
			
                	int previous=q[lptr->lqtail].qprev;
                	int maxpriority=0;
			
                	while(previous!=lptr->lqhead){
                        	if(previous==pid){
					int temp=q[previous].qprev;
					int lockid=0;
					for(;lockid<NPROC;lockid++){
						proctab[pid].lockmask[lockid]=0;
					}
					locker[ldes1].procmask[pid]=0;
					proctab[pid].pinh=0;
					proctab[pid].pprio=0;
					dequeue(previous);
					previous=temp;
					continue;
				}
				int currpriority=0;
                        	currpriority=proctab[previous].pprio;
				//kprintf("priority-%d\n",currpriority);
                        	if(currpriority>maxpriority){
                                	maxpriority=currpriority;
                        	}
                        	previous=q[previous].qprev;
                	}
			//kprintf("MAXPRIORITY IS %d\n",maxpriority);	
			int pidt=0;
                	for(pidt=0;pidt<NPROC;pidt++){
                        	if(locker[ldes1].procmask[pidt]==1){
					//printf("ENTERED HERE\n");
                                	recchprio(pidt,maxpriority);
                        	}
                	}
			semaph[pptr->psem].semcnt++;
	case PRREADY:	dequeue(pid);
			pptr->pstate = PRFREE;
			break;

	case PRSLEEP:
	case PRTRECV:	unsleep(pid);
						/* fall through	*/
	default:	pptr->pstate = PRFREE;
	}
	restore(ps);
	return(OK);
}
