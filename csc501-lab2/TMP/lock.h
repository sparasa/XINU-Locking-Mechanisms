#ifndef	NLOCK
#define	NLOCK		50	/* number of locks, if not defined	*/
#endif

#define	LFREE	'\03'		/* this lock is free		*/
#define	READ	'\05'		/* read process currently holds this lock	*/
#define WRITE  '\06'           /* write process currently holding this lock */
#define LUSED   '\04'		/* lock just got created without any process */


struct lentry {
	int lprio;
	int readcount;
	char lstate;
	int lqhead;
	int lqtail;
	int proc[NLOCK];
	int procmask[NLOCK];
};

extern struct lentry locker[NLOCK];
extern int nextlock;
