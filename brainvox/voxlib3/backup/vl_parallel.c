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

long int vl_init_threads(long int ncpus)
{
	return(0);
}
long int vl_start_thread(p_sproc proc,void *data,long int n,long int ncpus)
{
	return(sproc(proc,PR_SALL,data));
}
void vl_exit_thread(long int n)
{
	return;
}
long int vl_gather_threads(vset *set,long int ret,long int *kill,long int ncpus,
	void *data)
{
	int	i;

/* do the wait3, sginap loop stuff while periodically calling the
        callback function */
        i = 0;
        while(waitpid(0,&i,WNOHANG) != -1) {
                if (vl_rcallback(ret,set)) *kill = 1;
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

long int vl_init_threads(long int ncpus)
{
	if (thr_setconcurrency(ncpus+1)) return(-1);

	return(0);
}
long int vl_start_thread(p_sproc proc,void *data,long int n,long int ncpus)
{

	if (thr_create(NULL,0,proc,data,THR_BOUND,NULL)) return(-1);

	return(0);
}
void vl_exit_thread(long int n)
{
	thr_exit((void *)n);

	return;
}
long int vl_gather_threads(vset *set,long int ret,long int *kill,long int ncpus,
	void *data)
{
	void 		*rt;
	long int	i,done;
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

#else

long int vl_init_threads(long int ncpus)
{
	return(0);
}
long int vl_start_thread(p_sproc proc,void *data,long int n,long int ncpus)
{
	proc(data);

	return(0);
}
void vl_exit_thread(long int n)
{
	return;
}
long int vl_gather_threads(vset *set,long int ret,long int *kill,long int ncpus,
	void *data)
{
	return(0);
}

#endif
