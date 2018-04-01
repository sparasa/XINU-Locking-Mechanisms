/* chprio.c - chprio */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>
#include <lock.h>

/*------------------------------------------------------------------------
 * chprio  --  change the scheduling priority of a process
 *------------------------------------------------------------------------
 */
SYSCALL chprio(int pid, int newprio)
{
	STATWORD ps;    
	struct	pentry	*pptr;
	int ldes1;
	struct lentry *lptr;
	int flag=0;
	disable(ps);
	if (isbadpid(pid) || newprio<=0 ||
	    (pptr = &proctab[pid])->pstate == PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	
	

	if(proctab[pid].lockid!=0){
		flag=1;
		 if(proctab[pid].pinh==0){
                 	proctab[pid].pprio=newprio;
                 }
                 else{
                 	proctab[pid].pinh=newprio;
                 }

		int lockid=0;
		int maxpriority=0;
		for(;lockid<NPROC;lockid++){
			if(proctab[pid].lockmask[lockid]==1){
				 lptr= &locker[ldes1];
                		 int previous=q[lptr->lqtail].qprev;

                		 while(previous!=lptr->lqhead){
                        		int currpriority=0;
                       			 currpriority=proctab[previous].pprio;
                       			 if(currpriority>maxpriority){
                               			 maxpriority=currpriority;
                        		}
                       			 previous=q[previous].qprev;
                		}
			}
		}

		 if(proctab[pid].pinh==0 && maxpriority>proctab[pid].pprio){
                 	proctab[pid].pinh=proctab[pid].pprio;
                        proctab[pid].pprio=maxpriority;
                 }
                 else if(proctab[pid].pinh!=0 && maxpriority>proctab[pid].pinh){
                        proctab[pid].pprio=maxpriority;
                 }
                 else if(proctab[pid].pinh!=0 && maxpriority<proctab[pid].pinh){
                       	proctab[pid].pprio=proctab[pid].pinh;
                        proctab[pid].pinh=0;
                 }	
		
		 ldes1=proctab[pid].lockid;
                 lptr= &locker[ldes1];
		 int previous=q[lptr->lqtail].qprev;
                 maxpriority=0;

                 while(previous!=lptr->lqhead){
			int currpriority=0;	
                        currpriority=proctab[previous].pprio;
			if(currpriority>maxpriority){
                        	maxpriority=currpriority;
                        }
                      	previous=q[previous].qprev;
		}
		
		int pidt=0;
                for(pidt=0;pidt<NPROC;pidt++){
                	if(locker[ldes1].procmask[pidt]==1){
                                recchprio(pidt,maxpriority);
                        }
                }	
	}
	else{
		if(proctab[pid].pinh==0){
			proctab[pid].pprio=newprio;
		}
		else{
			proctab[pid].pinh=newprio;
		}

		int lockidt=0;
                for(lockidt=0;lockidt<NPROC;lockidt++){
                        if(proctab[pid].lockmask[lockidt]==1){
				flag=1;
				ldes1=lockidt;
                 		lptr= &locker[ldes1];
                 		int previous=q[lptr->lqtail].qprev;
                 		int maxpriority=0;
				while(previous!=lptr->lqhead){
                        		int currpriority=0;
                                	currpriority=proctab[previous].pprio;
                        		if(currpriority>maxpriority){
                                		maxpriority=currpriority;
                        		}
                        		previous=q[previous].qprev;
                		}
				if(proctab[pid].pinh==0 && maxpriority>proctab[pid].pprio){
                                        proctab[pid].pinh=proctab[pid].pprio;
                                        proctab[pid].pprio=maxpriority;
                                }
                                else if(proctab[pid].pinh!=0 && maxpriority>proctab[pid].pinh){
                                        proctab[pid].pprio=maxpriority;
                                }
                                else if(proctab[pid].pinh!=0 && maxpriority<proctab[pid].pinh){
                                proctab[pid].pprio=proctab[pid].pinh;
                                proctab[pid].pinh=0;
                                }				
                        }
                }	
	}
	if(flag==0){
		pptr->pprio=newprio;
	}
	restore(ps);
	return(newprio);
}
