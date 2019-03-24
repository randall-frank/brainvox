/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski, M.D.
 *
 * $Id: tal_shm_interface.c 1250 2005-09-16 15:51:42Z dforeman $
 *                University of Iowa, Department of Neurology,
 *                Human Neuroanatomy and Neuroimaging Lab
 *                University of Iowa, Image Analysis Facility
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All modifications to the source code must be clearly marked as
 *    such.  Binary redistributions based on modified source code
 *    must be clearly marked as modified versions in the documentation
 *    and/or other materials provided with the distribution.
 * 4. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgment:
 *      This product includes software developed by Randall Frank, Thomas
 *      Grabowski and other unpaid contributors.
 * 5. The names of Randall Frank or Thomas Grabowski may not be used to
 *    endorse or promote products derived from this software without 
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY RANDALL FRANK AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL RANDALL FRANK OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "tal_shm_interface.h"

#define BASE_SHM_KEY (0x661304)

/* tal_shmem references are of the form:
	:XX_%%%
	
	XX = shared memory block number (0=first)
	%%% = filename template (for slice indexing)
*/

#ifdef SHMEM_SUPPORT

#include <ctype.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>

/* lower level commands ------------------------- */

/* returns the volume size in bytes, or -1 */
int tal_shmblk_exists(int volnum,int *shmid)
{
	int flags;

	if ((volnum < 0) || (volnum >= MAX_SHM_BLOCKS)) return(-1);

	flags = (SHM_R | SHM_W);
	*shmid = shmget(volnum+BASE_SHM_KEY, 10000, flags);
	if (*shmid != -1) {
		struct shmid_ds buf;
		int status;
		status = shmctl(*shmid, IPC_STAT, &buf);
		if (status == 0) return(buf.shm_segsz);
	} 
	return(-1);
}

/* create a new shared memory block  -1=error  */
int tal_shmblk_create(int volnum,int size)
{
	int	shmid;
        int 	flags,key;
/* valid volnum? */
	if ((volnum < 0) || (volnum >= MAX_SHM_BLOCKS)) return(-1);
/* does it exist already? */
	if (tal_shmblk_exists(volnum,&shmid) != -1) return(-1);
/* create user RW group RW*/
	key = volnum+BASE_SHM_KEY;
        flags = (IPC_CREAT | (SHM_R | SHM_W | 0040 | 0020)); 
/* ok, we will try to make one */
        shmid = shmget(key, size, flags);
	if (shmid == -1) return(-1);
	return(0);
}

/* delete a shared memory block  -1=error */
int tal_shmblk_delete(int volnum)
{
	int	shmid;
	struct shmid_ds buf;
/* valid volnum? */
        if ((volnum < 0) || (volnum >= MAX_SHM_BLOCKS)) return(-1);
/* does it exist */
	if (tal_shmblk_exists(volnum,&shmid) == -1) return(-1);
/* ok, we will try to delete it */
	if (shmctl(shmid, IPC_RMID, &buf) == 0) return(0);
	return(-1);
}

/* attach to a shared memory block  -1=error */
void *tal_shmblk_open(int volnum)
{
	int	shmid;
/* valid volnum? */
        if ((volnum < 0) || (volnum >= MAX_SHM_BLOCKS)) return((void *)-1);
/* does it exist */
	if (tal_shmblk_exists(volnum,&shmid) == -1) return((void *)-1);
/* attach to the segment */
	return((void *)shmat(shmid, 0, 0));
}

/* detach from a shared memory block  -1=error */
int tal_shmblk_close(void *address)
{
/* detach from the segment */
	return(shmdt((void *)address));
}

/* upper level commands ------------------------- */

/* check to see if the template name is a shmblk */
int tal_shmblk_is_file(char *file)
{
	if ((file[0] == ':') && (file[3] == '_') &&
		(isdigit(file[1])) && (isdigit(file[2]))) {
		if (strlen(file) == 7) return(1);
	}
	return(0);
}

void tal_shmblk_cleanup()
{
	return;
}

int tal_shmblk_IO(char *file,char type,long int dx,long int dy,long int dz,
	unsigned char *data)
{
	long int	volnum,slice,status;
	char		tmp[256];
	int 		isize,shmid,i;
	char 		*addr;

#ifdef DEBUG
	printf("in tal_shmblk_IO:%s,%c,%ld%ld,%ld\n",file,type,dx,dy,dz);
#endif
	
/* valid file */
	if (!tal_shmblk_is_file(file)) return(1);

/* extract the slice number */
	tmp[0] = file[4];
	tmp[1] = file[5];
	tmp[2] = file[6];
	tmp[3] = '\0';
	slice = atoi(tmp);

/* extract the volume number */
	tmp[0] = file[1];
	tmp[1] = file[2];
	tmp[2] = '\0';
	volnum = atoi(tmp);

#ifdef DEBUG
	printf("slice= %ld volnum=%ld\n",slice,volnum);
#endif
	if (slice < 1) return(1);
	slice = slice - 1;    /* slices start at 1 */
	if ((volnum < 0) || (volnum >= MAX_SHM_BLOCKS)) return(1);

/* check the image sizes */
	isize = tal_shmblk_exists(volnum,&shmid);
	if (isize == -1) {
		fprintf(stderr,"Requested shared volume does not exist\n");
		return(1);
	}
	if (isize < dx*dy*dz*(slice+1)) {
		fprintf(stderr,"Request exceeds the shared volume size\n");
		return(1);
	}

/* attach to the volume */
	addr = (char *)tal_shmblk_open(volnum);
	if ((int)addr == -1) return(1);

/* service the request */
	status = 0;
	switch(type) {
		case 'r':
			memcpy(data,addr+dx*dy*dz*slice,dx*dy*dz);
			break;
		case 'w':
			memcpy(addr+dx*dy*dz*slice,data,dx*dy*dz);
			break;
	}

/* detach the volume */
	i = tal_shmblk_close(addr);
	if (i == -1) status = 1;

#ifdef DEBUG
	printf("status=%ld\n",status);
#endif
	return(status);
}

#else
int tal_shmblk_is_file(char *file) {
        return(0);
}
int tal_shmblk_IO(char *file,char type,long int dx,long int dy,long int dz,
        unsigned char *data) {
        return(1);
}
void tal_shmblk_cleanup(void) {
}
#endif
