#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include "bv_shmem.h"

void bv_sginap_(unsigned long *ticks);

#define MAX_MSGS	20

/* memory mapped portion */
typedef struct {
	time_t 		timestamp;
	long int	length;
	long int	msg[IPC_SHM_SIZE];
} msg_struct;

typedef struct {
	long int	top;
	long int	quitflag;
	long int	lockval;
	msg_struct	msgs[MAX_MSGS];
} ipc_struct;


/* shared memory structure */
static ipc_struct *msg_box;

/* local datum */
static long int	  bottom;

#ifndef OLD_IPC_ROUTINES

void INIT_IPC(long int *first,void *ipc_addr)
{
init_ipc_(first,ipc_addr);
}
void init_ipc_(long int *first,void *ipc_addr)
{
	long int	i,j;

/* get the shared memory pointer */
	msg_box = ipc_addr;
	if (*first) {
/* clear out all the msg boxes */
		msg_box->lockval = 0;
		msg_box->quitflag = 0;
		for(i=0;i<MAX_MSGS;i++) {
			msg_box->msgs[i].timestamp = 0;
			msg_box->msgs[i].length = 0;
		}
		msg_box->top = 0;
	} 
/* no initial messages */
	bottom = msg_box->top;

	return;
}

void SEND_IPC(long int *size,long int *msg)
{
send_ipc_(size,msg);
}
void send_ipc_(long int *size,long int *msg)
{
	long int	i,j;

/* no longer needed */
	if (msg[0] == IPC_OK) return;
	if (msg_box->quitflag != 0) return;

/* special case (quitting) */
	if (msg[0] == IPC_QUIT) {
		msg_box->quitflag = 1;
		return;
	}
/* wait for the lock to clear */
	while (msg_box->lockval != 0) {
		unsigned long	nap = 2;
		msg_box->lockval = 0;
		if (msg_box->quitflag != 0) return;
		bv_sginap_(&nap);
	}
/* grab the lock */
	msg_box->lockval = 1;
/* store the msg */
	j = msg_box->top;
	msg_box->msgs[j].timestamp = time(0);
	msg_box->msgs[j].length = *size;
	for(i=0;i<*size;i++) {
		msg_box->msgs[j].msg[i] = msg[i];
	}
/* bump it along */
	msg_box->top++;
	if (msg_box->top >= MAX_MSGS) msg_box->top = 0;
/* release the lock */
	msg_box->lockval = 0;

	return;
}

void CHECK_IPC(long int *size,long int *msg)
{
check_ipc_(size,msg);
}
void check_ipc_(long int *size,long int *msg)
{
	long int 	i,j;

/* special case (quitting) */
	if (msg_box->quitflag != 0) {
		*size = 1;
		msg[0] = IPC_QUIT;
		return;
	}
/* block if someone is writing */
	while (msg_box->lockval != 0) {
		unsigned long nap = 2;
		bv_sginap_(&nap);
/* special case (quitting) */
		if (msg_box->quitflag != 0) {
			*size = 1;
			msg[0] = IPC_QUIT;
			msg_box->lockval = 0;
			return;
		}
	}
/* get the top value */
	if (msg_box->top == bottom) {
		*size = 1;
		msg[0] = IPC_OK;
		return;
	}
/* get the message */
	j = msg_box->msgs[bottom].length;
	if (j > *size) j = *size;
	for(i=0;i<j;i++) {
		msg[i] =  msg_box->msgs[bottom].msg[i];
	}
	*size = j;
/* bump it along */
	bottom++;
	if (bottom >= MAX_MSGS) bottom = 0;

	return;
}

#endif
