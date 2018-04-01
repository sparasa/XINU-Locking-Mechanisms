#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sem.h>
#include <stdio.h>
#include <lock.h>
extern long unsigned ctr1000;
/*------------------------------------------------------------------------
 *  * releaseall  --  release a process, which curremtly holds the lock.
 *   *------------------------------------------------------------------------
 *    */
int releaseall(int numlocks,int ldes1,...)
{
	unsigned long   *des;             /* points to list of args       */
	int flag=0;

        STATWORD ps;
        register struct lentry  *lptr;
	int retvalue=OK;
        disable(ps);
	des = (unsigned long *)(&ldes1) ; 
	while(numlocks>0){
        	if ((lptr= &locker[*(des)])->lstate==LFREE) {
			flag=1;
			numlocks--;
                	des++;
			continue;
        	}
		
		if(proctab[currpid].loc[*(des)]!=READ && proctab[currpid].loc[*(des)]!=WRITE){
			flag=1;
			//retvalue=SYSERR;
			numlocks--;
                	des++;
			continue;
		}

		if(proctab[currpid].loc[*(des)]==READ){
			lptr->readcount--;	//decrement the reader count
			proctab[currpid].loc[*(des)]=LUSED;	//change the loc type so that we can give errorif its called again.
			proctab[currpid].lockmask[*(des)]=0;
			lptr->procmask[currpid]=0;
			changeprev(currpid,*(des));
			//code priority of releasing process
			if(proctab[currpid].pinh!=0){
				proctab[currpid].pprio=proctab[currpid].pinh;
				proctab[currpid].pinh=0;
			}
			if(lptr->readcount==0){
				int prev=q[lptr->lqtail].qprev;
				if(prev!=lptr->lqhead){
					proctab[prev].lockid=0;
					proctab[prev].lockmask[*(des)]=1;
					lptr->procmask[prev]=1;
					getnext(q[prev].qprev,*(des));
					lptr->lstate=WRITE;
					ready(getlast(lptr->lqtail), RESCHNO);						//reschedule the process here
				}
			}
		}

		if(proctab[currpid].loc[*(des)]==WRITE){
			int fixreader=1;
			int readcount=0;
			int writecount=0;
			int begincomp=0;
			int starttime=0;
			int pprior=0;
			int prev = q[lptr->lqtail].qprev;
			proctab[currpid].loc[*(des)]=LUSED;
			//change the hold status of the present process in proc and lock masks.
			proctab[currpid].lockmask[*(des)]=0;
			lptr->procmask[currpid]=0;
			changeprev(currpid,*(des));
			if(proctab[currpid].pinh!=0){
				proctab[currpid].pprio=proctab[currpid].pinh;
				proctab[currpid].pinh=0;
			}
			if(prev!=lptr->lqhead){
				if(proctab[prev].loc[*(des)]==WRITE){
					pprior=q[prev].qkey;
					writecount++;
					fixreader=0;
					starttime=proctab[prev].ltime;
					begincomp=1;
					prev=q[prev].qprev;
				}
				else{
					readcount++;
					pprior=q[prev].qkey;
					int temp=prev;
					proctab[prev].lockmask[*(des)]=1;
                                        lptr->procmask[prev]=1;
					prev=q[prev].qprev;
					proctab[temp].lockid=0;
					getnext(prev,*(des));
					ready(dequeue(temp), RESCHNO);
					lptr->readcount++;
					lptr->lstate=READ;
					fixreader=1;
					begincomp=0;
				}
			}
			//evaluate remaining process from second here.
			while(prev!=lptr->lqhead){
				if(q[prev].qkey!=pprior && begincomp==1){	//if we didnt find writer than take readers continously.
					break;
				}	
				else{
					if(begincomp==0 && proctab[prev].loc[*(des)]==READ){	//writer not found than insert readers into ready queue.
						readcount++;
						lptr->readcount++;
						lptr->lstate=READ;
						pprior=q[prev].qkey;
						proctab[prev].lockmask[*(des)]=1;
                                        	lptr->procmask[prev]=1;
						proctab[prev].lockid=0;
						prev=q[prev].qprev;
						getnext(prev,*(des));
						ready(getlast(lptr->lqtail), RESCHNO);
						continue;
					}
					else if(begincomp==1 && proctab[prev].loc[*(des)]==READ){	//writer found so compare the reader with first writer
						if(ctr1000-starttime+500<ctr1000-proctab[prev].ltime){	//if satisfies push it into queue
							readcount++;
							lptr->readcount++;
							pprior=q[prev].qkey;
							proctab[prev].lockmask[*(des)]=1;
                                        		lptr->procmask[prev]=1;
							int temp=prev;
							prev=q[prev].qprev;
							proctab[temp].lockid=0;
							getnext(q[lptr->lqtail].qprev,*(des));
							dequeue(temp);
							ready(temp,RESCHNO);
							lptr->lstate=READ;
							continue;
						}
						else{
							break;
						}
					}
					else if(fixreader==1 && begincomp==0 && proctab[prev].loc[*(des)]==WRITE){	//first one is reader and we didn't encounter any writer
						starttime=proctab[prev].ltime;
						begincomp=1;
						pprior=q[prev].qkey;
						prev=q[prev].qprev;
						continue;
					}
					else if(fixreader==1&& begincomp==1 && proctab[prev].loc[*(des)]==WRITE){	//first reader and writer found so continue;
						pprior=q[prev].qkey;
						prev=q[prev].qprev;
						continue;				
					}
					else if(fixreader==0 && proctab[prev].loc[*(des)]==WRITE){	//first write than ignore this write
						 pprior=q[prev].qkey;
						prev=q[prev].qprev;
						continue;
					}
				
				}
				pprior=q[prev].qkey;
				prev=q[prev].qprev;
			}
			if(readcount==0 && writecount>0){
				proctab[q[lptr->lqtail].qprev].lockmask[*(des)]=1;
                                lptr->procmask[q[lptr->lqtail].qprev]=1;
				proctab[q[lptr->lqtail].qprev].lockid=0;
				getnext(q[q[lptr->lqtail].qprev].qprev,*(des));
				ready(getlast(lptr->lqtail), RESCHNO);
				lptr->lstate=WRITE;
			}
			else{
				retvalue=OK;
			}
		}
		numlocks--;
		des++;
	}
	resched();
	restore(ps);
	if(flag==1){
		kprintf("enetered into if");
		return(SYSERR);
	}
	return(retvalue);
}

void getnext(int prev,int ldes1){
	struct lentry *lptr=&locker[ldes1];
	int maxpriority=0;
	while(prev!=lptr->lqhead){
		if(proctab[prev].pprio>maxpriority){
			maxpriority=proctab[prev].pprio;
		}
		prev=q[prev].qprev;
	}
	
	int pid=0;
	for(;pid<NPROC;pid++)
	{
		if(locker[ldes1].procmask[pid]==1){
			if(proctab[pid].pinh==0 && proctab[pid].pprio<maxpriority){
				proctab[pid].pinh=proctab[pid].pprio;
				proctab[pid].pprio=maxpriority;
			}
			else if(proctab[pid].pinh!=0 && proctab[pid].pinh<maxpriority){
				proctab[pid].pprio=maxpriority;
			}
			else if(proctab[pid].pinh!=0 && proctab[pid].pinh>maxpriority){
				proctab[pid].pprio=proctab[pid].pinh;
				proctab[pid].pinh=0;
			}
		}
	}
}

void changeprev(int pid,int ldes1){
	struct lentry *lptr=&locker[ldes1];
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
}
