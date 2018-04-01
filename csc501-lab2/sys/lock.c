/* lock.c - lock */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sem.h>
#include <stdio.h>
#include <lock.h>
/*------------------------------------------------------------------------
 * lock  --  make current process wait on a lock
 *------------------------------------------------------------------------
 */
extern long unsigned ctr1000;
int	lock(int ldes1, int type, int priority)
{
	//printf("in lock ldesl-%d",ldes1);
	STATWORD ps;    
	struct	lentry	*lptr;
	struct	pentry	*pptr;

	disable(ps);
	//when sempahore is not created return syserr when tried to access the lock
	if ((lptr= &locker[ldes1])->lstate==LFREE) {
		restore(ps);
		return(SYSERR);
	}

	if(locker[ldes1].proc[currpid]==-1){
		restore(ps);
		return(SYSERR);
	}
	proctab[currpid].loc[ldes1]=type;
	//kprintf("pid-%d lockid-%d",currpid,ldes1);
	//recieved first process then reschedule it
	//kprintf("enetered into lock\n %d",proctab[currpid].loc[ldes1]);
	if(lptr->lstate == LUSED){
		//inserting into masks here.
		proctab[currpid].lockmask[ldes1]=1;
		locker[ldes1].procmask[currpid]=1;
		setmax(ldes1);
		lptr->lstate = type;		//save state in semaphore
		proctab[currpid].loc[ldes1]=type;//save state in proctab
		if(type==READ){
			lptr->readcount++;
		}
		//ready(currpid,RESCHYES);        //schedule the new process
		restore(ps);			//return ok
		return(OK);
	}
	
	//do priority inheritance whenever we are pushing a process to waiting queue.

	//if this is not true then insert directly into semaphore queue.
	if(lptr->lstate == READ && type == READ){
		//proctab[currpid].lstate=LREAD;	this should be done onl if it goes to ready queue.
		int prev = q[lptr->lqtail].qprev;
		if(prev!=lptr->lqhead){
			if(priority>q[prev].qkey || priority==q[prev].qkey){
				//ready(currpid,RESCHYES); not needed
				//inserting into masks here.
				proctab[currpid].lockmask[ldes1]=1;
                		locker[ldes1].procmask[currpid]=1;
				setmax(ldes1);
				lptr->readcount++;
				proctab[currpid].loc[ldes1]=type;
				restore(ps);
				return(OK);
			}
		}
		else{
			//ready(currpid,RESCHYES); not needed
			//inserting into masks.
			proctab[currpid].lockmask[ldes1]=1;
                	locker[ldes1].procmask[currpid]=1;
			setmax(ldes1);
                        lptr->readcount++;
                        proctab[currpid].loc[ldes1]=type;
                        restore(ps);
                        return(OK);
		}
		//increasing priority code here.
		//printf("scheduling reader here %s",proctab[currpid].pname);
		proctab[currpid].lockid=ldes1;
		proctab[currpid].loc[ldes1]=type;
		proctab[currpid].ltime=ctr1000;
		//q[currpid].qkey=priority;
		proctab[currpid].pstate = PRWAIT;
		proctab[currpid].pwaitret=OK;
                insert(currpid,lptr->lqhead,priority);
		setmax(ldes1);
		resched();
		restore(ps);
		return pptr->pwaitret;
	}
	else{
		proctab[currpid].lockid=ldes1;
		proctab[currpid].loc[ldes1]=type;
                proctab[currpid].ltime=ctr1000;
		proctab[currpid].pstate = PRWAIT;
		proctab[currpid].pwaitret=OK;
		//q[currpid].qkey=priority;
		insert(currpid,lptr->lqhead,priority);
		setmax(ldes1);
		resched();	
		restore(ps);
		return pptr->pwaitret;
	}
	restore(ps);
	return(OK);
}

void setmax(int ldes1){
	struct lentry *lptr=&locker[ldes1];
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

	int pid=0;
        for(pid=0;pid<NPROC;pid++){
        	if(locker[ldes1].procmask[pid]==1){
                /*	if(proctab[pid].pinh==0 && maxpriority>proctab[pid].pprio){
                        	proctab[pid].pinh=proctab[pid].pprio;
                                proctab[pid].pprio=maxpriority;
                        }
                        else if(proctab[pid].pinh!=0 && maxpriority>proctab[pid].pinh){
                        	proctab[pid].pprio=maxpriority;
                        }
                        else if(proctab[pid].pinh!=0 && maxpriority<proctab[pid].pinh){
                                proctab[pid].pprio=proctab[pid].pinh;
                                proctab[pid].pinh=0;
                        }*/
                        recchprio(pid,maxpriority);

              	}
      	}
}
