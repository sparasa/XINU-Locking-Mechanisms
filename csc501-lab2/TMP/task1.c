#include<conf.h>
#include<kernel.h>
#include<proc.h>
#include<lock.h>
#include<stdio.h>
#include<sem.h>

#define DEFAULT_LOCK_PRIO 20
extern int ctr1000;

LOCAL int reader55(int locks)
{

	int begin=ctr1000;
	lock(locks,WRITE,DEFAULT_LOCK_PRIO);
	int end=ctr1000;
	int time=end-begin;
	releaseall(1,locks);
	printf("lock time- %d\n",time);
}

LOCAL int reader44(int semaphore){
	int begin=ctr1000;

	wait(semaphore);
	int end=ctr1000;
	int time=end-begin;

	signal(semaphore);
	printf("semaphore time- %d\n",time);
}

LOCAL void writer44()
{
	int iterator=0;
	for(;iterator<=20000;iterator++){
		int k=0;
		for(;k<10000;k++)
		{
			
		}
	}
}

LOCAL void writer55()
{
        int iterator=0;
        for(;iterator<=20000;iterator++){
		int k=0;
		for(;k<10000;k++){
		}
        }
}

LOCAL void semaphore_s(int sem){
	int reader1=create(reader44,300,90,"reader3",1,sem);
	int writer1=create(writer44,300,60,"writer3",1,1);
	resume(reader1);
	resume(writer1);
	signal(sem);
}

LOCAL void lock_s(int locks){
	int reader2=create(reader55,400,60,"reader3",1,locks);
	int writer2=create(writer55,400,70,"writer3",1,1);
	resume(reader2);
	resume(writer2);
	releaseall(1,locks);
}

int task1()
{
	int semaphore1=screate(1);
	wait(semaphore1);
	
	int semaphore=lcreate();
	lock(semaphore,READ,DEFAULT_LOCK_PRIO);

	semaphore_s(semaphore1);
	lock_s(semaphore);
}

