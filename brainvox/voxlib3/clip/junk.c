#include <stdio.h>
#include <time.h>
#include <thread.h>

#define NUM_THREADS 2
#define CACHE_LINE 8

long junk[10];
void *work_proc(void *);
long ttt = 1;

void main(int argc, char **argv)
{
long stat, i;
void *ret;
time_t	start,end;

/* set the LWP pool level = to the number of threads */
if (thr_setconcurrency(NUM_THREADS+1))
 perror("Can't set concurrency."), exit(1);

for (i=0;i<NUM_THREADS;i++)
 {
junk[i] = 0;
 /* create the threads will all defaults,  call work_proc
    with i as the argument */
 if (thr_create(NULL, 0, work_proc, (void *) i, THR_BOUND, NULL))
  perror("Can't create thread."), exit(1);
 }

printf("Starting parallel...\n");
fflush(stdout);
start = time(0L);
/* yield this LWP for others that may need it */
thr_yield();

/* try to join the running threads.  This call will block
  untill the thread has called thr_exit() */
while (!thr_join(0,0,&ret)) printf("%ld : %ld\n",(int)ret,junk[(int)ret]);

end = time(0L);

printf("Parallel Done... (%ld)\n",end-start);
fflush(stdout);

/* try linear */
printf("\n\n\nStarting sequential...\n");
fflush(stdout);
start = time(0L);
ttt = 0;
for (i=0;i<NUM_THREADS;i++) {
	junk[i] = 0;
	work_proc((void *)i);
}
end = time(0L);

printf("Sequential Done... (%ld)\n",end-start);
fflush(stdout);

exit(0);
}

void *work_proc(void *arg)
{
 long num = (long)arg;
        long i,j;


        printf("In a thread %ld\n",num);
 	fflush(stdout);

        j = 0;

        for(i=0;i<1024*1024*60;i++) {
                j = j + 1;
                junk[num*CACHE_LINE] = j;
                }

        junk[num] = -1;

  	printf("Out of thread %ld\n",num);
 	fflush(stdout);

 /* exit the thread and return the thread num */
 if (ttt) thr_exit((void *)num);

	return;
}

