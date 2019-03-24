#include "voxel.h"
#include "vl_util.h"
#include <stdio.h>
#include <stdlib.h>

#include "vl_parallel.h"

/* System dependant threads abstraction */

#if (defined(IRIX4) || defined(IRIX5))
#include <sys/types.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <limits.h>

VLINT32 vl_init_threads(VLINT32 ncpus)
{
	return(0);
}
VLINT32 vl_start_thread(p_sproc proc,void *data,VLINT32 n,VLINT32 ncpus)
{
	return(sproc(proc,PR_SALL,data));
}
void vl_exit_thread(VLINT32 n)
{
	return;
}
VLINT32 vl_gather_threads(vset *set,VLINT32 ret,VLINT32 *kill,VLINT32 ncpus,
	void *data)
{
	VLINT32	i;

/* do the wait3, sginap loop stuff while periodically calling the
        callback function */
        i = 0;
        while(waitpid(0,&i,WNOHANG) != -1) {
                if (vl_rcallback(ret,1,set)) *kill = 1;
					 sginap(CLK_TCK/30);
                i = 0;
        }

/* catch all the children */
        i = 0;
        while(wait(&i) != -1);

	return(0);
}

#elif defined(SOLARIS)

#include <thread.h>

VLINT32 vl_init_threads(VLINT32 ncpus)
{
	if (thr_setconcurrency(ncpus+1)) return(-1);

	return(0);
}
VLINT32 vl_start_thread(p_sproc proc,void *data,VLINT32 n,VLINT32 ncpus)
{

	if (thr_create(NULL,0,proc,data,THR_BOUND,NULL)) return(-1);

	return(0);
}
void vl_exit_thread(VLINT32 n)
{
	thr_exit((void *)n);

	return;
}
VLINT32 vl_gather_threads(vset *set,VLINT32 ret,VLINT32 *kill,VLINT32 ncpus,
	void *data)
{
	void 		*rt;
	VLINT32	i,done;
	par_quant 	*par = (par_quant *)data;

/* yield this LWP for others that may need it */
	thr_yield();

/* wait until they are all done */
	done = 0;
	while (!done) {
		done = 1;
		for(i=0;i<ncpus;i++) if (par[i].cbnum != -2) done = 0;
					 if (vl_rcallback(ret,set)) *kill = 1;
		usleep(33333L);  /* (1/30*1000000) microseconds */
	}

/* join the running threads.  Blocks until a thread calls thr_exit() */
	while (!thr_join(0,0,&rt));

	return(0);
}

#elif (defined(WIN32) || defined(__WIN32__))
#include <windows.h>
#include <windowsx.h>

VLINT32 vl_init_threads(VLINT32 ncpus)
{
	return(0);
}

VLINT32 vl_start_thread(p_sproc proc,void *data,VLINT32 n,VLINT32 ncpus)
{
SECURITY_ATTRIBUTES lpsa;	/* address of thread security attrs	*/
DWORD cbStack;	/* initial thread stack size	*/
LPTHREAD_START_ROUTINE lpStartAddr;	/* address of thread function	*/
LPVOID lpvThreadParm;	/* argument for new thread	*/
DWORD fdwCreate;	/* creation flags	*/
DWORD lpIDThread;
par_quant 	*par = (par_quant *)data;


	lpsa.nLength = sizeof( LPSECURITY_ATTRIBUTES );
	lpsa.lpSecurityDescriptor = NULL;   /* use default */
	lpsa.bInheritHandle = FALSE;  /* No need for a app handle */
	cbStack = 0; // use default stack size
	lpStartAddr =  ( LPTHREAD_START_ROUTINE ) proc ;
	lpvThreadParm = data;
	fdwCreate = 0;
	par->hThread = (VLVOID *) CreateThread( &lpsa, cbStack, lpStartAddr, lpvThreadParm, fdwCreate, &lpIDThread);
	// give it a high priority...
	SetThreadPriority( (HANDLE ) par->hThread,  THREAD_PRIORITY_HIGHEST );
	return  ( VLINT32 ) par->hThread;
}
void vl_exit_thread(VLINT32 n)
{
	ExitThread( n );
}
VLINT32 vl_gather_threads(vset *set,VLINT32 ret,VLINT32 *kill,VLINT32 ncpus,
	void *data)
{
	VLINT32	i;
	HANDLE hThreads[MAX_CPUS];
	par_quant 	*par = (par_quant *)data;
	for(i=0;i<ncpus;i++) hThreads[i] = (HANDLE ) par[i].hThread;

/* do the wait3, sginap loop stuff while periodically calling the
		  callback function */

		  while ( WaitForMultipleObjects(ncpus, hThreads, TRUE, 33L) == WAIT_TIMEOUT ){
				 if (vl_rcallback(ret,1,set)) *kill = 1; 
		  }

/* catch all the children */

		  //WaitForMultipleObjects(ncpus, hThreads, FALSE, INFINITE);
	// Close the thread handles
	for(i=0;i<ncpus;i++) CloseHandle( (HANDLE ) par[i].hThread );
	return(0);
}


#else

VLINT32 vl_init_threads(VLINT32 ncpus)
{
	return(0);
}
VLINT32 vl_start_thread(p_sproc proc,void *data,VLINT32 n,VLINT32 ncpus)
{
	proc(data);

	return(0);
}
void vl_exit_thread(VLINT32 n)
{
	return;
}
VLINT32 vl_gather_threads(vset *set,VLINT32 ret,VLINT32 *kill,VLINT32 ncpus,
	void *data)
{
	return(0);
}

#endif
