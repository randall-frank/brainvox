/*
 * Copyright (c), 1990-2005 Randall Frank and Thomas Grabowski
 *
 * $Id $
 * $HeadURL $
 *
 *                University of Iowa, Department of Neurology,
 *                Human Neuroanatomy and Neuroimaging Lab
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

/* this is the library of cvio functions */

#include <sys/errno.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include "zlib.h"
#include "cvio_priv.h"
#include <math.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <time.h>
#ifndef WIN32
#include <sys/time.h>
#include <sys/select.h>
#include <unistd.h>
#endif
#ifdef linux
#include <sched.h>
#endif
#include <signal.h>

#ifdef WIN32
#include <windows.h>
/* dummy up shared memory interface for non-compliant platforms */
#define NEED_UNION_SEMUN
#define SHM_R	0400
#define SHM_W	0200
#define IPC_CREAT 00001000
#define IPC_RMID  0
#define IPC_SET   1
#define IPC_STAT  2
#define IPC_INFO  3
typedef int key_t;
struct ipc_perm
{
	key_t  key;
	unsigned short uid;   /* owner euid and egid */
	unsigned short gid;
	unsigned short cuid;  /* creator euid and egid */
	unsigned short cgid;
	unsigned short mode;  /* lower 9 bits of access modes */
	unsigned short seq;   /* sequence number */
};
struct shmid_ds {
	struct    ipc_perm shm_perm;  /* operation perms */
	int  shm_segsz;          /* size of segment (bytes) */
	time_t    shm_atime;          /* last attach time */
	time_t    shm_dtime;          /* last detach time */
	time_t    shm_ctime;          /* last change time */
	unsigned short shm_cpid; /* pid of creator */
	unsigned short shm_lpid; /* pid of last operator */
	short     shm_nattch;         /* no. of current attaches */
};
static int shmdt(void *addr) 
{
	return(-1);
}
static void *shmat(int id,void *addr,int flgs)
{
	return((void *)-1);
}
static int shmget(key_t key, int size, int shmflg)
{
	return(-1);
}
int shmctl(int shmid, int cmd, struct shmid_ds *buf)
{
	return(-1);
}
#else
#include <sys/ipc.h>
#include <sys/shm.h>
#endif

#ifndef SEM_R
#define SEM_R 0400
#endif

#ifndef SEM_A
#define SEM_A 0200
#endif

#ifdef NEED_UNION_SEMUN
union semun
{
    int val;
    struct semid_ds *buf;
    ushort *array;
};
#ifndef USES_UNION_SEMUN
#define USES_UNION_SEMUN
#endif
#endif /* NEED_UNION_SEMUN */

#ifdef USES_UNION_SEMUN
#define DeclareSemctlArg    union semun semun_arg
#define SemctlArrayArg(v)   (semun_arg.array = (v), semun_arg)
#define SemctlStructArg(v)  (semun_arg.buf = (v), semun_arg)
#define SemctlIntArg(v)     (semun_arg.val = (v), semun_arg)
#else
#define DeclareSemctlArg        /* nothing ... must be final decl. in list !*/
#define SemctlArrayArg(v)       (v)
#define SemctlStructArg(v)      (v)
#define SemctlIntArg(v)         (v)
#endif /* USES_UNION_SEMUN */

/* local prototypes */
static void unlock(int semid);
static void lock(int semid);
static int find_shmem_block(const char *name, header *hdr, void **base);      
static cvio_stream *valid_stream(uint32_t stream);                       
static int32_t read_hdr(FILE *fp, header *hdr);                               
static int32_t write_hdr(FILE *fp, header *hdr);                              
static void interp_sample(void *dst,void *src,int32_t len,int32_t typ,double w);

/* global list of all the streams */
static	cvio_stream stream_list[MAX_STREAMS];

/* stuff for blocking reads */
static pthread_t signalthread;
static pthread_mutex_t sigusrlock;
static pthread_cond_t sigusrcond;

static void usrhandler(int unused)
{
	pthread_mutex_lock(&sigusrlock);
	pthread_cond_broadcast(&sigusrcond);
	pthread_mutex_unlock(&sigusrlock);
}

/* thread to wait for sigusr1 and farm it out to appropriate blockers */
static void *usrthread(void *args)
{
	sigset_t set;
	int err;

	sigemptyset(&set);
	signal(SIGUSR1, usrhandler);
	while (1) {
		err = sigsuspend(&set);
	}
	return NULL;
}

/* function to interpolate a "sample" */
static void interp_sample(void *dst,void *src,int32_t len,int32_t typ,double w)
{
	int32_t	i,n;

	n = len/(typ & CVIO_TYP_SIZE_MASK);
	switch(typ) {
		case CVIO_TYP_STRING:
		case CVIO_TYP_BYTE:
			{
				CVIO_DT_BYTE	*a = (CVIO_DT_BYTE *)dst;
				CVIO_DT_BYTE	*b = (CVIO_DT_BYTE *)src;
				for(i=0;i<n;i++) {
					*a += w*((*b)-(*a));
					a++; b++;
				}
			}
			break;
		case CVIO_TYP_SHORT:
			{
				CVIO_DT_SHORT	*a = (CVIO_DT_SHORT *)dst;
				CVIO_DT_SHORT	*b = (CVIO_DT_SHORT *)src;
				for(i=0;i<n;i++) {
					*a += w*((*b)-(*a));
					a++; b++;
				}
			}
			break;
		case CVIO_TYP_USHORT:
			{
				CVIO_DT_USHORT	*a = (CVIO_DT_USHORT *)dst;
				CVIO_DT_USHORT	*b = (CVIO_DT_USHORT *)src;
				for(i=0;i<n;i++) {
					*a += w*((*b)-(*a));
					a++; b++;
				}
			}
			break;
		case CVIO_TYP_FLOAT:
			{
				CVIO_DT_FLOAT	*a = (CVIO_DT_FLOAT *)dst;
				CVIO_DT_FLOAT	*b = (CVIO_DT_FLOAT *)src;
				for(i=0;i<n;i++) {
					*a += w*((*b)-(*a));
					a++; b++;
				}
			}
			break;
		case CVIO_TYP_BOOL:
			/* bool interpolation just extends the
			 * earlier of the two samples, which is
			 * already in *dst
			 */
			break;
		case CVIO_TYP_IMPULSE:
			/* impulses are dataless timestamps.
			 * this should be unreachable.
			 */
			break;
	}
	return;
}

/* function to check if this host is bigendian */
int is_bigendian(void)
{
	short	sh[] = {1};
	char	*by;

	by = (char *)sh;

	return(by[0] ? 0 : 1);
}

/* function to perform general 2, 4 and 8 byte byteswapping */
void  byteswap(void *buffer,int32_t len,int32_t swapsize)
{
	int	num;
	char	*p = (char *)buffer;
	char	t;

	switch(swapsize) {
		case 2:
			num = len/swapsize;
			while(num--) {
				t = p[0]; p[0] = p[1]; p[1] = t;
				p += swapsize;
			}
			break;
		case 4:
			num = len/swapsize;
			while(num--) {
				t = p[0]; p[0] = p[3]; p[3] = t;
				t = p[1]; p[1] = p[2]; p[2] = t;
				p += swapsize;
			}
			break;
		case 8:
			num = len/swapsize;
			while(num--) {
				t = p[0]; p[0] = p[7]; p[7] = t;
				t = p[1]; p[1] = p[6]; p[6] = t;
				t = p[2]; p[2] = p[5]; p[5] = t;
				t = p[3]; p[3] = p[4]; p[4] = t;
				p += swapsize;
			}
			break;
		default:
			break;
	}
	return;
}

/* read an ASCII header structure (by tags) */
static int32_t read_hdr(FILE *fp, header *hdr)
{
	int32_t	count = 0;
	char	tstr[256],*t;
	char	ver[256];
	int32_t	i,n_aux = 0;

	for(i=0;i<MAX_AUX_ATTRIBS;i++) hdr->auxattribs[i][0] = '\0';

	rewind(fp);

/* first line must be signature */
	if (!fgets(tstr,256,fp)) return(1);
	sprintf(ver,"CVIO_VERSION: %s",CVIO_FILE_VERSION);
	if (!strstr(tstr,ver)) return(1);

/* defaults for non-required tags */
	hdr->timestampbase = CVIO_TIME_TICKS_PER_SEC;
	hdr->spacing[0] = 1.0;
	hdr->spacing[1] = 1.0;
	hdr->spacing[2] = 1.0;

/* read all the lines */
	while(fgets(tstr,256,fp)) {
		if ((t = strstr(tstr,"FILENAME:"))) {
			sscanf(t+9,"%s",hdr->filename);
			count++;
		} else if ((t = strstr(tstr,"CREATION_DATE:"))) {
			sscanf(t+14,"%s",hdr->date);
			count++;
		} else if ((t = strstr(tstr,"DATA_TYPE:"))) {
			sscanf(t+10,"%hd",&(hdr->type));
			count++;
		} else if ((t = strstr(tstr,"DATA_NDIMS:"))) {
			sscanf(t+11,"%hd",&(hdr->ndim));
			count++;
		} else if ((t = strstr(tstr,"DATA_DIMS:"))) {
			sscanf(t+10,"%hd %hd %hd",&(hdr->dims[0]),
				&(hdr->dims[1]),&(hdr->dims[2]));
			count++;
		} else if ((t = strstr(tstr,"DATA_COMPRESSION:"))) {
			sscanf(t+17,"%hd",&(hdr->compression));
			count++;
		} else if ((t = strstr(tstr,"DATA_NSAMPLES:"))) {
			sscanf(t+14,"%d",&(hdr->nsamp));
			count++;
		} else if ((t = strstr(tstr,"FIRST_TIME:"))) {
			sscanf(t+11,"%d",&(hdr->first_time));
			count++;
		} else if ((t = strstr(tstr,"LAST_TIME:"))) {
			sscanf(t+10,"%d",&(hdr->last_time));
			count++;
		} else if ((t = strstr(tstr,"BIGENDIAN:"))) {
			sscanf(t+10,"%hd",&(hdr->bigendian));
			count++;
		} else if ((t = strstr(tstr,"TIMESTAMPBASE:"))) {
			sscanf(t+14,"%d",&(hdr->timestampbase));
		} else if ((t = strstr(tstr,"SPACING:"))) {
			sscanf(t+8,"%f %f %f",&(hdr->spacing[0]),
				&(hdr->spacing[1]),&(hdr->spacing[2]));
		} else if (n_aux < MAX_AUX_ATTRIBS) {
/* AUX attrib */
			if ((t = strstr(tstr,"_ST:"))) {
				if (strlen(tstr) < MAX_ATTRIB_LEN) {
					strcpy(hdr->auxattribs[n_aux],tstr);
					t = strchr(hdr->auxattribs[n_aux],'\n');
					if (t) *t = '\0';
					t = strchr(hdr->auxattribs[n_aux],'\r');
					if (t) *t = '\0';
					t = strchr(hdr->auxattribs[n_aux],':');
					*t = '\0';
					n_aux += 1;
				}
			}
		}
	}

/* did we get all the required fields? */
	if (count != 10) return(CVIO_ERR_MISSINGREQTAGS);

	return(CVIO_ERR_OK);
}

/* write the header structure in ASCII */
static int32_t write_hdr(FILE *fp, header *hdr)
{
	int32_t	i;

	rewind(fp);
	ftruncate(fileno(fp), 0);

	fprintf(fp,"CVIO_VERSION: %s\n",CVIO_FILE_VERSION);

	fprintf(fp,"FILENAME:%s\n",hdr->filename);
	fprintf(fp,"CREATION_DATE:%s\n",hdr->date);
	fprintf(fp,"DATA_TYPE:%d\n",hdr->type);
	fprintf(fp,"DATA_NDIMS:%d\n",hdr->ndim);
	fprintf(fp,"DATA_DIMS:%d %d %d\n",hdr->dims[0],	
		hdr->dims[1],hdr->dims[2]);
	fprintf(fp,"DATA_COMPRESSION:%d\n",hdr->compression);
	fprintf(fp,"DATA_NSAMPLES:%d\n",hdr->nsamp);
	fprintf(fp,"FIRST_TIME:%d\n",hdr->first_time);
	fprintf(fp,"LAST_TIME:%d\n",hdr->last_time);
	fprintf(fp,"BIGENDIAN:%d\n",hdr->bigendian);
	fprintf(fp,"TIMESTAMPBASE:%d\n",hdr->timestampbase);
	fprintf(fp,"SPACING:%f %f %f\n",hdr->spacing[0],hdr->spacing[1],
		hdr->spacing[2]);

	i = 0;
	while(hdr->auxattribs[i][0]) {
		fprintf(fp,"%s:",hdr->auxattribs[i]);
		fprintf(fp,"%s\n",hdr->auxattribs[i]+
			strlen(hdr->auxattribs[i])+1);
		i++;
	}

	return(CVIO_ERR_OK);
}

/*****cvio_get_error_message*****/
/* function to convert error code to ASCII message */
int32_t cvio_get_error_message(int32_t errorval, char *buffer, int32_t *bufferlen)
{
	char	*msg;

	switch (errorval) {
		case CVIO_ERR_OK:
			msg = "No error.";
			break;
		case CVIO_ERR_SHORTBUFFER:
			msg = "Insufficient buffer length.";
			break;
		case CVIO_ERR_NOMEMORY:
			msg = "Insufficient memory.";
			break;
		case CVIO_ERR_UNKNOWN_ATTR:
			msg = "Unknown attribute.";
			break;
		case CVIO_ERR_INVALIDACCESS:
			msg = "Unknown access value.";
			break;
		case CVIO_ERR_MAXSTREAMS:
			msg = "Maximum number of streams open.";
			break;
		case CVIO_ERR_BADSTREAM:
			msg = "Invalid stream number.";
			break;
		case CVIO_ERR_BADFILENAME:
			msg = "Invalid or non-existant filename.";
			break;
		case CVIO_ERR_MISSINGREQTAGS:
			msg = "Required header tags were missing.";
			break;
		case CVIO_ERR_NOCOMPSHMEM:
			msg = "Compression no allowed with shared memory.";
			break;
		case CVIO_ERR_NOSHMEM:
			msg = "No free shared memory blocks could be found.";
			break;
		case CVIO_ERR_SHMEMLOCKED:
			msg = "Cannot delete shared memory block, in use?";
			break;
		case CVIO_ERR_BADACCESS:
			msg = "File was not opened to allow this access.";
			break;
		case CVIO_ERR_TIMESEQ:
			msg = "Invalid sequence of time values.";
			break;
		case CVIO_ERR_NOSAMPLE:
			msg = "No sample present at this location.";
			break;
		case CVIO_ERR_EOF:
			msg = "Reached end of file before completion.";
			break;
		case CVIO_ERR_INVALIDRULE:
			msg = "Invalid time rule.";
			break;
		case CVIO_ERR_BADDATATYPE:
			msg = "Invalid data type specified.";
			break;
		case CVIO_ERR_BADDIMENSIONS:
			msg = "Invalid dimensions specified.";
			break;
		case CVIO_ERR_NBLOCKERS:
			msg = "Too many blocking readers.";
			break;
		case CVIO_ERR_STREAMCLOSED:
			msg = "Stream closed.";
			break;
		case CVIO_ERR_DELETED:
			msg = "Stream deleted.";
			break;
		default:
			msg = "Unknown error message value.";
			break;
	}
	
	/* WILL ADD MORE CASE TYPES AS NEEDED */

	/* copy the string (as much as will fit) and null term */
	strncpy(buffer,msg,*bufferlen);
	buffer[*bufferlen] = '\0';

	/* warn if it did not fit */
	if (strlen(msg)+1 > *bufferlen) return(CVIO_ERR_SHORTBUFFER);

 	/* return the buffer length */
	*bufferlen = strlen(buffer);

	return(CVIO_ERR_OK);
}

/*****cvio_init*****/
/* function of cvio_init not yet defined */
int32_t cvio_init(void)
{
	sigset_t set;
	int err;
	int32_t	i;

	sigemptyset(&set);
	sigaddset(&set, SIGUSR1);
	sigprocmask(SIG_BLOCK, &set, NULL);

	err = pthread_mutex_init(&sigusrlock, NULL);
assert(err == 0);
	if (err)
		return CVIO_ERR_INVALIDACCESS;
	err = pthread_cond_init(&sigusrcond, NULL);
assert(err == 0);
	if (err)
		return CVIO_ERR_INVALIDACCESS;

	for(i=0;i<MAX_STREAMS;i++) stream_list[i].status = STATUS_UNUSED;

	err = pthread_create(&signalthread, NULL, usrthread, NULL);
	assert(err == 0);
	return(CVIO_ERR_OK);
}

/*****cvio_cleanup*****/
/* shuts down the library - flushes and closes all streams */
int32_t cvio_cleanup(int32_t cleanupflag)
{	
	int32_t	i;
/*
XXX This can unleash blocked sigusr1s and kill the process... just don't do it?
	sigset_t set;

	sigemptyset(&set);
	sigaddset(&set, SIGUSR1);
	sigprocmask(SIG_UNBLOCK, &set, NULL);
*/
/* close all the open files, destroying shmem blocks if requested */
	for(i=0;i<MAX_STREAMS;i++) {
		if (stream_list[i].status != STATUS_UNUSED) {
			char tstr[MAX_FILENAME];
			int32_t	status;
			strcpy(tstr,stream_list[i].hdr.filename);
			status = stream_list[i].status;
			cvio_close(i);
			if ((cleanupflag & CVIO_CLEANUP_RMSHMEM) && 
			    (status == STATUS_SHMEM)) {
				cvio_delete(tstr);
			}
		}
	}

	return(CVIO_ERR_OK);	
}

/*****cvio_listshmem*****/
/* list name of shared memory streams and return total number of shared memory streams */
int32_t cvio_listshmem(char *buffer, int32_t *bufferlen, int32_t *num_strings)
{	
	int32_t 	shm_id, len, l;
	int32_t 	id;
	int32_t 	retval = CVIO_ERR_OK;
	void	*base;
	header	hdr;
	char	*s;
	
	len = *bufferlen;
	s = buffer;
	*bufferlen = 0;

	*num_strings = 0;
	for (id=0; id < MAX_STREAMS*5; id++) {
		shm_id = shmget(id+SHM_BASE, sizeof(header), (SHM_R | SHM_W));
		if (shm_id != -1) {
			base = shmat(shm_id,0,0);
			memcpy(&hdr, base, sizeof(header));
			shmdt(base);
			l = strlen(hdr.filename)+1;
			if (len - l > 0) {
				memcpy(s,hdr.filename,l);
				*num_strings += 1;
				len -= l;
				*bufferlen += l;
				s += l;
			}
		}
	}

	return(retval);
}

/*****cvio_create*****/
/* creates a new stream file consisting of either shared memory or a file */
int32_t cvio_create(const char *filename, int32_t nsamples, int32_t type, int32_t ndims, 
	int32_t dims[CVIO_MAX_DATA_DIMS], int32_t flags)
{
	struct shmid_ds buf;
        int num_bytes, shm_id, i, blocklock;
        key_t id;
        char *shmem_ptr;
	void *base;
        header file_header,tmphdr;
        position file_position;
        FILE *passing_stream;
        struct tm *tmptr;
        time_t secs_now;
	int semval;
	DeclareSemctlArg;

	if ((type != CVIO_TYP_BYTE) && (type != CVIO_TYP_SHORT) && 
	    (type != CVIO_TYP_USHORT) &&
	    (type != CVIO_TYP_BOOL) && (type != CVIO_TYP_IMPULSE) &&
	    (type != CVIO_TYP_FLOAT) && (type != CVIO_TYP_STRING)) {
		return(CVIO_ERR_BADDATATYPE);
	}
	if ((type == CVIO_TYP_STRING) && (ndims != 1)) {
		return(CVIO_ERR_BADDIMENSIONS);
	}
	if (ndims > CVIO_MAX_DATA_DIMS) return(CVIO_ERR_BADDIMENSIONS);
	if (ndims < 1) return(CVIO_ERR_BADDIMENSIONS);
	for(i=0;i<ndims;i++) {
		if (dims[i] <= 0) return(CVIO_ERR_BADDIMENSIONS);
	}
	if (nsamples < 1) return(CVIO_ERR_BADDIMENSIONS);

        time(&secs_now);
        tmptr = localtime(&secs_now);

	strcpy(file_header.filename,filename);

        /* assign necessary values to the header fields */
        file_header.type = type;
        file_header.ndim = ndims;

	for(i=0;i<CVIO_MAX_DATA_DIMS;i++) file_header.dims[i] = 1;
	for(i=0;i<ndims;i++) file_header.dims[i] = dims[i];

	file_header.spacing[0] = 1.0;
	file_header.spacing[1] = 1.0;
	file_header.spacing[2] = 1.0;
	file_header.flags = 0;
        file_header.nsamp = 0;
        file_header.compression = 0;
        file_header.max_samples = nsamples;
	file_header.deleted = 0;
	if (flags & CVIO_CREATE_GZIP) file_header.compression = 1;
        file_header.first_time = 0;
        file_header.last_time = 0;

        strftime(file_header.date, 32, "%j-%Y", tmptr);

	file_header.bigendian = is_bigendian();
	file_header.timestampbase = CVIO_TIME_TICKS_PER_SEC;

	for(i=0;i<MAX_AUX_ATTRIBS;i++) file_header.auxattribs[i][0] = '\0';

        file_position.current_sample = 0;
        file_position.current_time = 0;
        file_position.time_rule = CVIO_TIMERULE_ROUND;

        /* determine whether want shared memory or file */
        if (strstr(filename, ":shmem") == filename){
		char *p;
		p = strstr(file_header.filename, ":wait=");
		if (p) {
			*p = 0;
			file_header.writers = atoi(p+6);
		} else
			file_header.writers = 1;

		if (file_header.compression) return(CVIO_ERR_NOCOMPSHMEM);

        	/* determine size of buffer required */
        	num_bytes = nsamples*file_header.dims[0]*
		           file_header.dims[1]*file_header.dims[2]*
			((type & CVIO_TYP_SIZE_MASK)/8);

		num_bytes += sizeof(header);
		num_bytes += nsamples*sizeof(CVIO_DT_TIME);
		num_bytes += MAX_BLOCKERS*sizeof(pid_t);
		/* make sure the object does not already exist */
		shm_id = find_shmem_block(file_header.filename, &tmphdr, &base);
		if ( shm_id != -1) {
			shmdt(base);
			return(CVIO_ERR_BADFILENAME);
		}

                /* find an unused shared memory block */
		for (id=0; id < MAX_STREAMS*5; id++) {
			/* create and attach to shared memory */
			shm_id = shmget(id+SHM_BASE, num_bytes, 
			                IPC_CREAT | IPC_EXCL | (SHM_R | SHM_W | 0040 | 0020));
			if (shm_id == -1) {
				if (errno == EEXIST)
					continue;
				if (errno == EINVAL) {
					printf("Apple broken shmem?\n");
					continue;
				}
				printf("Failed, %d, %d, %d\n", id, num_bytes, errno);
				perror("life is pain");
				return(CVIO_ERR_NOSHMEM);
			}
			blocklock = semget(id+SHM_BASE, 1, SEM_R | SEM_A | IPC_CREAT);
			if (blocklock == -1) {
#ifdef DEBUG
				printf("unable to create block lock\n");
#endif
				shmctl(shm_id, IPC_RMID, &buf);
				return(CVIO_ERR_NOSHMEM);
			}
			file_header.semkey = id+SHM_BASE;
			semval = 1;
			semctl(blocklock, 0, SETVAL, SemctlIntArg(semval));
			shmem_ptr = shmat(shm_id, 0, 0);
			memcpy(shmem_ptr, &file_header, sizeof(file_header));
			memset(shmem_ptr + num_bytes- MAX_BLOCKERS*sizeof(pid_t),
			       0, MAX_BLOCKERS*sizeof(pid_t));
			/* detach from shared memory */
			shmdt(shmem_ptr);

			return(CVIO_ERR_OK);
                }
		return(CVIO_ERR_NOSHMEM);

        } else if (strstr(filename, ":sock") == filename){
		if (file_header.compression) return(CVIO_ERR_NOCOMPSHMEM);
                /* create the memory bits */
                if (cvio_sock_find(filename,&file_header,0)) {
			return(CVIO_ERR_BADFILENAME);
                }
        } else {
		char 	tstr[MAX_FILENAME];
		FILE	*fp;
                /* create the three files as desired:header, timestamp, data */
        	if (file_header.compression) {
			strcpy(tstr,filename);
                	fp = fopen(strcat(tstr, EXT_TIM_GZ), "wb");
			if (!fp) return(CVIO_ERR_BADFILENAME);
			fclose(fp);
			strcpy(tstr,filename);
                	fp = fopen(strcat(tstr, EXT_DAT_GZ), "wb");
			if (!fp) return(CVIO_ERR_BADFILENAME);
			fclose(fp);
		} else {
			strcpy(tstr,filename);
                	fp = fopen(strcat(tstr, EXT_TIM), "wb");
			if (!fp) return(CVIO_ERR_BADFILENAME);
			fclose(fp);
			strcpy(tstr,filename);
                	fp = fopen(strcat(tstr, EXT_DAT), "wb");
			if (!fp) return(CVIO_ERR_BADFILENAME);
			fclose(fp);
		}

                /* write header structure to file */
                /* open file */
                passing_stream = fopen(filename, "w");
		if (!passing_stream) return(CVIO_ERR_BADFILENAME);

                /* write header structure to file */
		write_hdr(passing_stream, &file_header);

                /* close file */
                fclose(passing_stream);
        }

        return(CVIO_ERR_OK);
}


/*****cvio_delete*****/
/* removes file/shared memory completely */
int32_t cvio_delete(const char *filename)
{
	int blocklock;
	char tstr[MAX_FILENAME];

	if (strstr(filename,":shmem") == filename) {
		header	hdr;
		void	*base;
		struct shmid_ds buf;
		int	shm_id, sem, n;
		int iSize;
		pid_t *bpids;

		/* find and remove shared memory */
		shm_id = find_shmem_block(filename, &hdr, &base);
		if ( shm_id == -1) return(CVIO_ERR_NOSHMEM);
		blocklock = semget(hdr.semkey, 0, SEM_R | SEM_A);
		lock(blocklock);
		hdr.flags = 0;
		hdr.writers = 0;
		hdr.deleted = 1;
		iSize = (hdr.type & CVIO_TYP_SIZE_MASK) * hdr.dims[0]*hdr.dims[1]*hdr.dims[2]/8;
		iSize += sizeof(CVIO_DT_TIME);
		bpids = (pid_t *)((char *)base + hdr.max_samples * iSize + sizeof(header));
		memcpy(base, &hdr, sizeof(header));
		for (n = 0; n < MAX_BLOCKERS; n++) {
			if (bpids[n] != 0)
				kill(bpids[n], SIGUSR1);
		}
		unlock(blocklock);
		sem = semget(hdr.semkey, 0, SEM_R | SEM_A);
		semctl(sem, 0, IPC_RMID);
		shmdt(base);
		if (shmctl(shm_id, IPC_RMID, &buf) != 0) {
			return(CVIO_ERR_SHMEMLOCKED);
		}
	} else if (strstr(filename,":sock") == filename) {
                /* remove memory structure... */
                cvio_sock_find(filename,NULL,2);
	} else {
		/* remove file */
		remove(filename);
		strcpy(tstr,filename);
		remove(strcat(tstr, EXT_TIM));
		strcpy(tstr,filename);
		remove(strcat(tstr, EXT_DAT));
		strcpy(tstr,filename);
		remove(strcat(tstr, EXT_TIM_GZ));
		strcpy(tstr,filename);
		remove(strcat(tstr, EXT_DAT_GZ));
	}

	return(CVIO_ERR_OK);	
}

/*****lock*****/
/* wait for a semaphore to become 0, then decrement it */
static void lock(int semid)
{
	struct sembuf op;
	op.sem_num = 0;
	op.sem_op = -1;
	op.sem_flg = SEM_UNDO;
	semop(semid, &op, 1);
}

/*****unlock*****/
/* increment a semaphore */
static void unlock(int semid)
{
	struct sembuf op;
	op.sem_num = 0;
	op.sem_op = 1;
	op.sem_flg = SEM_UNDO;
	semop(semid, &op, 1);
}

/*****cvio_open*****/
/* open a stream in a specified file */
int32_t cvio_open(const char *filename, uint32_t in_access, uint32_t *stream)
{
        char *access;
        int32_t i;
        cvio_stream *s;

        /* find an open slot */
        for(i=0; i<MAX_STREAMS; i++) {
                if (stream_list[i].status == STATUS_UNUSED) break;
        }

        if (i == MAX_STREAMS) {
                return(CVIO_ERR_MAXSTREAMS);
        }

        *stream = i;
        s = &(stream_list[*stream]);

        /* determine access permissions for writing to file */
	s->flags = in_access & ~(CVIO_ACCESS_READ|CVIO_ACCESS_APPEND);
	s->access = in_access & (CVIO_ACCESS_READ|CVIO_ACCESS_APPEND);
        if (s->access == CVIO_ACCESS_READ) {
                access = "rb";
        } else if (s->access == CVIO_ACCESS_APPEND) {
                access = "ab";
        } else {
                return(CVIO_ERR_INVALIDACCESS);
        }

        strcpy(s->hdr.filename, filename);
	s->blocklock = -1;

        /* determine whether want shared memory or file */
        if (strstr(filename, ":shmem") == filename) {

		int	shm_id;
		int32_t	iSize,nSamps;
		struct shmid_ds	info;

		/* find the block and read the header */
		shm_id = find_shmem_block(filename,&(s->hdr),&(s->base));
		/* did we get one? */
		if (shm_id == -1) return(CVIO_ERR_NOSHMEM);

		/* get the size of a sample+its timeval */
		iSize = (s->hdr.type & CVIO_TYP_SIZE_MASK)*
			s->hdr.dims[0]*s->hdr.dims[1]*s->hdr.dims[2]/8;
		iSize += sizeof(CVIO_DT_TIME);

		/* get the size of the shared memory block */
		shmctl(shm_id, IPC_STAT, &info);

		/* the number of allowed sample is the shared memory
		   block size - blocker list size - the header divided by the sample size */
		nSamps = (info.shm_segsz- sizeof(pid_t)*MAX_BLOCKERS-sizeof(header))/iSize;

		s->shm_id = shm_id;
		s->hdr.max_samples = nSamps;

		/* cook up pointers to the raw data and to the time blocks */
		s->time = (CVIO_DT_TIME *)((char *)s->base + sizeof(header));
		s->data = (char *)s->base + nSamps*sizeof(CVIO_DT_TIME) 
			+ sizeof(header);
		s->bpids = (pid_t *)((char *)s->base + nSamps*iSize + sizeof(header));

		s->blocklock = semget(s->hdr.semkey, 0, SEM_R | SEM_A);

		/* error or stake writing claim to the shared memory block */
		if (s->access == CVIO_ACCESS_APPEND) {
			if (s->hdr.flags & HDR_FLAGS_HAS_WRITER) {
				/* only one writer */
				shmdt(s->base);
				return(CVIO_ERR_BADACCESS);
			} else {
				/* and we claim to be it!! */
				s->hdr.flags |= HDR_FLAGS_HAS_WRITER;
				memcpy(s->base, &(s->hdr), sizeof(header));
			}
		}

                s->status = STATUS_SHMEM;

        } else if (strstr(filename, ":sock") == filename) {
                int port,wait;
                char host[80],name[80]; 
                float f;

                parse_sock(filename,host,name,&port,&wait,&f);
        	if (s->access == CVIO_ACCESS_READ) {
                	if (cvio_sock_connect(s,host,name,port)) {
				return(CVIO_ERR_BADFILENAME);
                	}
		} else {
			/* find the "create" header */
                        if (cvio_sock_find(filename,&(s->hdr),1)) {
				return(CVIO_ERR_BADFILENAME);
                        }
                        /* bring up a socket (writer) */
                	if (cvio_sock_startup(s,host,name,port,wait,f)) {
				return(CVIO_ERR_BADFILENAME);
                	}
		}
                s->status = STATUS_SOCKET;

        } else {

		char	tstr[MAX_FILENAME];

                /* open uncompressed header file & read the header */
		if (s->access == CVIO_ACCESS_APPEND)
			s->files[0] = fopen(filename, "r+");
		else
			s->files[0] = fopen(filename, "r");
		if (!s->files[0]) {
			return(CVIO_ERR_BADFILENAME);
		}
		if (read_hdr(s->files[0], &(s->hdr))) {
			fclose(s->files[0]);
			return(CVIO_ERR_BADFILENAME);
		}

                /* the maximum number of samples is unknown */
                s->hdr.max_samples = 0;

		if (s->hdr.compression) {

			strcpy(tstr,filename);
                        s->files[1] = (FILE *)gzopen(strcat(tstr, EXT_TIM_GZ), 
				access);
			strcpy(tstr,filename);
                        s->files[2] = (FILE *)gzopen(strcat(tstr, EXT_DAT_GZ), 
				access);

			if ((s->files[1] == NULL) || (s->files[2] == NULL)) {
				if (s->files[0]) fclose(s->files[0]);
				if (s->files[1]) gzclose((gzFile)s->files[1]);
				if (s->files[2]) gzclose((gzFile)s->files[2]);
				return(CVIO_ERR_BADFILENAME);
			}

		} else {

			strcpy(tstr,filename);
                        s->files[1] = fopen(strcat(tstr, EXT_TIM), access);
			strcpy(tstr,filename);
                        s->files[2] = fopen(strcat(tstr, EXT_DAT), access);

			if ((s->files[1] == NULL) || (s->files[2] == NULL)) {
				if (s->files[0]) fclose(s->files[0]);
				if (s->files[1]) fclose(s->files[1]);
				if (s->files[2]) fclose(s->files[2]);
				return(CVIO_ERR_BADFILENAME);
			}
                }

                s->status = STATUS_FILE;
        }

        /* establish current position */
	if (s->access == CVIO_ACCESS_APPEND) {
        	s->pos.current_time = 0;  /* this value is unknown */
        	s->pos.current_sample = s->hdr.nsamp;
		s->pos.time_rule = CVIO_TIMERULE_ROUND;
	} else {
        	s->pos.current_time = 0;
        	s->pos.current_sample = 0;
		s->pos.time_rule = CVIO_TIMERULE_ROUND;
	}

        return(CVIO_ERR_OK);
}

/* look for a shared memory block w/the name given */
static int find_shmem_block(const char *name, header *hdr, void **base)
{
	int	shm_id = -1,id;

/* search all shared memory blocks */
	for (id=0; id < MAX_STREAMS*5; id++) {
		shm_id = shmget(id+SHM_BASE, sizeof(header), 
				(SHM_R | SHM_W));
		if (shm_id != -1) {
/* attach and read the header */
			*base = shmat(shm_id,0,0);
			if (*base == -1)
				continue;
			memcpy(hdr, *base, sizeof(header));
/* is it the right one? */
			if (strcmp(hdr->filename,name) == 0) return(shm_id);
/* detach and look at the next one */
			shmdt(*base);
		}
	}
	return(-1);
}

/* check to see if the stream numebr is valid */
static cvio_stream *valid_stream(uint32_t stream)
{
	if (stream >= MAX_STREAMS) return((cvio_stream *)0L);
	if (stream_list[stream].status != STATUS_UNUSED) {
		return(&(stream_list[stream]));
	}
	return((cvio_stream *)0L);
}

/*****cvio_max_length*****/
/* get the maximum length of the file */
int32_t cvio_max_length(uint32_t stream,uint32_t *len)
{
	cvio_stream 	*s;

	s = valid_stream(stream);
	if (!s) return(CVIO_ERR_BADSTREAM);

	/* maximum length of the shared memory block or number of
	 * samples in the disk file */
	if (s->status == STATUS_SHMEM) {
		*len = s->hdr.max_samples;
	} else {
		*len = s->hdr.nsamp;
                if (s->hdr.max_samples > *len) *len = s->hdr.max_samples;
	}

	return(CVIO_ERR_OK);
}

/*****cvio_close*****/
/* close the specified stream */
int32_t cvio_close(uint32_t stream)
{
	cvio_stream 	*s;
	int n;

	s = valid_stream(stream);
	if (!s) return(CVIO_ERR_BADSTREAM);

	if (s->status == STATUS_SHMEM) {
		if (s->access == CVIO_ACCESS_APPEND) {
			/* release our claim as writer... */
			s->hdr.flags &= ~HDR_FLAGS_HAS_WRITER;
			/* use up one of the wait writers */
			if (s->hdr.writers > 0)
				s->hdr.writers--;
			memcpy(s->base, &(s->hdr), sizeof(header));

			if (s->hdr.writers == 0) {
				/* wake anyone still blocking for us now
					so they get writers = 0 and EOF properly
				 */
				lock(s->blocklock);
				for (n = 0; n < MAX_BLOCKERS; n++)
					if (s->bpids[n] != 0)
						kill(s->bpids[n], SIGUSR1);
				unlock(s->blocklock);
			}
		}

		shmdt(s->base);
	
	} else if (s->status == STATUS_FILE) {
		if (s->hdr.compression == 1) {
			gzclose((gzFile)s->files[1]);
			gzclose((gzFile)s->files[2]);
		} else {
			fclose(s->files[1]);
			fclose(s->files[2]);	
		}
		if (s->access == CVIO_ACCESS_APPEND)
			write_hdr(s->files[0], &(s->hdr));
		fclose(s->files[0]);
	} else if (s->status == STATUS_SOCKET) {
		cvio_sock_shutdown(s);
	}

	/* make the file descriptor as unused */
	s->status = STATUS_UNUSED;

	return(CVIO_ERR_OK);
}

/*****cvio_add_samples*****/
/* write count samples to the data stream */
int32_t cvio_add_samples(uint32_t stream, CVIO_DT_TIME timeval[], void *buffer, int32_t count)
{
	int32_t i,iSize;
	cvio_stream 	*s;

	s = valid_stream(stream);
	if (!s) return(CVIO_ERR_BADSTREAM);
	
	if (s->access != CVIO_ACCESS_APPEND) return(CVIO_ERR_BADACCESS);

	/* if using shared memory, update the header */
	if (s->status == STATUS_SHMEM) {
		memcpy(&(s->hdr), s->base, sizeof(header));
		if (s->pos.current_sample + count > s->hdr.max_samples) {
			return(CVIO_ERR_EOF);
		}
	}

	/* rework any timestamps with the value "NEXT" */
	for(i=0;i<count;i++) {
		if (timeval[i] == CVIO_TIME_NEXT) {
			timeval[i] = s->pos.current_time + 1;
		}
		if (timeval[i] <= s->pos.current_time) return(CVIO_ERR_TIMESEQ);
		s->pos.current_time = timeval[i];
	}

	/* if this is the first add, update the first_time value */
	if (s->hdr.nsamp == 0) s->hdr.first_time = timeval[0];
		
	/* find the size of a single sample in chars */
	iSize = (s->hdr.type & CVIO_TYP_SIZE_MASK)*
			s->hdr.dims[0]*s->hdr.dims[1]*s->hdr.dims[2]/8;

	/* swap if endian mismatch */
	if (s->hdr.bigendian != is_bigendian()) {
		byteswap(buffer,iSize*count,
			(s->hdr.type & CVIO_TYP_SIZE_MASK)/8);
		byteswap(timeval,count*sizeof(CVIO_DT_TIME),
			sizeof(CVIO_DT_TIME));
	}

	/* "write" the samples */
	if (s->status == STATUS_SHMEM) {
	    memcpy(s->time+s->pos.current_sample, timeval, 
		count*sizeof(CVIO_DT_TIME));
	    memcpy(((char *)s->data)+iSize*s->pos.current_sample, buffer, 
		count*iSize);
	} else if (s->status == STATUS_SOCKET) {
            if (sock_add_samples(s,timeval,buffer,count)) {
                return(CVIO_ERR_NOMEMORY);
            }
	} else {
	    if (s->hdr.compression == 1) {
		gzwrite((gzFile)(s->files[1]), timeval, count*sizeof(CVIO_DT_TIME)); 
		gzwrite((gzFile)(s->files[2]), buffer, count*iSize); 
	    } else {
		fwrite( timeval, sizeof(CVIO_DT_TIME), count, s->files[1]);
		fwrite( buffer, iSize, count, s->files[2]);
	    }
	}

	/* undo previous swap */
	if (s->hdr.bigendian != is_bigendian()) {
		byteswap(buffer,iSize*count,
			(s->hdr.type & CVIO_TYP_SIZE_MASK)/8);
		byteswap(timeval,count*sizeof(CVIO_DT_TIME),
			sizeof(CVIO_DT_TIME));
	}

	/* change the current sample number and bump the last time value
	   Even if 0 samples were added, the last_time gets updated
	*/
	s->pos.current_sample += count;
	s->hdr.nsamp += count;
	if (!count)
		s->pos.current_time = timeval[0];

	s->hdr.last_time = s->pos.current_time;

	/* if using shared memory, update the header, wake all blockers */
	if (s->status == STATUS_SHMEM) {
		int n;
		/* update:first_time, last_time, nsamp */

		lock(s->blocklock);
		memcpy(s->base, &(s->hdr), sizeof(header));

		for (n = 0; n < MAX_BLOCKERS; n++)
			if (s->bpids[n] != 0)
				kill(s->bpids[n], SIGUSR1);
		unlock(s->blocklock);
	}

	return(CVIO_ERR_OK);
}

/*****cvio_read_samples*****/
/* read count samples from the data stream */
int32_t cvio_read_samples(uint32_t stream, CVIO_DT_TIME timeval[], 
                          void *buffer, int32_t count)
{
	int32_t i, iSize;
	cvio_stream *s;
	void	*tbuff = NULL;
	
	/* determine whether a valid stream */
	s = valid_stream(stream);
	if (!s) return(CVIO_ERR_BADSTREAM);

	if (s->access != CVIO_ACCESS_READ) return(CVIO_ERR_BADACCESS);

	if (count == 0) return CVIO_ERR_OK;

	/* if using shared memory, update the header */
	if (s->status == STATUS_SHMEM) {
		memcpy(&(s->hdr), s->base, sizeof(header));
	}

	if (s->hdr.deleted)
		return CVIO_ERR_DELETED;

	/* find the size of a single sample in chars */
	iSize = (s->hdr.type & CVIO_TYP_SIZE_MASK)*
		s->hdr.dims[0]*s->hdr.dims[1]*s->hdr.dims[2]/8;

	if (s->hdr.nsamp == 0) return(CVIO_ERR_EOF);

	/* read all the requested samples */
	while(count) {
		/* find the NEXT blocks */
		i = 0;
		while((i < count) && (timeval[i] == CVIO_TIME_NEXT)) i++;

		if (i) {
			/* read i NEXT samples */
			if (s->pos.current_sample+i > s->hdr.nsamp) {
				return(CVIO_ERR_EOF);
			}
			if (s->status == STATUS_SHMEM) {
				memcpy(timeval, s->time+s->pos.current_sample,
					i*sizeof(CVIO_DT_TIME));
				if (buffer) {
				    memcpy(buffer, ((char *)s->data) + 
					(iSize*s->pos.current_sample),i*iSize);
				}
                        } else if (s->status == STATUS_SOCKET) {
                                int thecount = count;
                                /* read count samples */
                                if (sock_read_samples(s,timeval,buffer,
                                                      &thecount,-1)) {
					return(CVIO_ERR_STREAMCLOSED);
                                }
                                /* if no samples, EOF! */
                                if (thecount == 0) return(CVIO_ERR_EOF);
			} else if (s->hdr.compression == 1) {
				gzread((gzFile)(s->files[1]), timeval, 
					i*sizeof(CVIO_DT_TIME));
				if (buffer) {
				    gzread((gzFile)(s->files[2]), buffer, 
					i*iSize);
				} else {
				    gzseek((gzFile)(s->files[2]), i*iSize,
				        SEEK_CUR);
				}
			} else {
				fread(timeval, sizeof(CVIO_DT_TIME), i, 
					s->files[1]);
				if (buffer) {
				    fread(buffer, iSize, i, s->files[2]);
				} else {
				    fseek(s->files[2], i*iSize, SEEK_CUR);
				}
			}
			/* swap if endian mismatch */
			if (s->hdr.bigendian != is_bigendian()) {
				byteswap(timeval,i*sizeof(CVIO_DT_TIME),
					sizeof(CVIO_DT_TIME));
				if (buffer) {
				    byteswap(buffer,iSize*i,
					(s->hdr.type & CVIO_TYP_SIZE_MASK)/8);
				}
			}
		} else {
			int32_t		t,done = 0;
			CVIO_DT_TIME	cur,last;
			double		weight = 1.0;

			/* read 1 random access sample */
			i = 1;
			if (timeval[0] > s->hdr.last_time)
				return(CVIO_ERR_EOF);

			/* rewind the timestamp files */
			if (s->status == STATUS_SHMEM) {
				s->pos.current_sample = 0;
			} else if (s->status == STATUS_SOCKET) {
                                /* random access socket not allowed */
				return(CVIO_ERR_INVALIDACCESS);
			} else if (s->hdr.compression == 1) {
				s->pos.current_sample = 0;
				gzseek((gzFile)s->files[1],0,SEEK_SET);
			} else {
				s->pos.current_sample = 0;
				fseek(s->files[1],0,SEEK_SET);
			}

			/* binary search shmem timestamps */
			if (s->status == STATUS_SHMEM || (s->status == STATUS_FILE && s->hdr.compression == 0)) {
				int32_t high, low;
				high = s->hdr.nsamp;
				low = 0;
				while (!done) {
					t = (high+low)/2;
					if (s->status == STATUS_SHMEM)
						memcpy(&cur, s->time+t, sizeof(CVIO_DT_TIME));
					else {
						fseek(s->files[1],
						      sizeof(CVIO_DT_TIME)*t,SEEK_SET);
						fread(&cur, sizeof(CVIO_DT_TIME), 
						      1, s->files[1]);
					}						
					/* swap if endian mismatch */
					if (s->hdr.bigendian != is_bigendian()) {
						byteswap(&cur,i*sizeof(CVIO_DT_TIME),
							sizeof(CVIO_DT_TIME));
					}
					/* are we done yet? */
					if (timeval[0] == cur) {
						done = 1;
					} else if (low + 1 == high) {
						if (s->status == STATUS_SHMEM)
							memcpy(&last, s->time+low, sizeof(CVIO_DT_TIME));
						else {
							fseek(s->files[1], sizeof(CVIO_DT_TIME)*low, SEEK_SET);
							fread(&cur, sizeof(CVIO_DT_TIME), 1, s->files[1]);
						}
						/* swap if endian mismatch */
						if (s->hdr.bigendian != is_bigendian()) {
							byteswap(&last,i*sizeof(CVIO_DT_TIME),
								sizeof(CVIO_DT_TIME));
						}
						if (s->status == STATUS_SHMEM)
							memcpy(&cur, s->time+high, sizeof(CVIO_DT_TIME));
						else {
							fseek(s->files[1], sizeof(CVIO_DT_TIME)*low, SEEK_SET);
							fread(&cur, sizeof(CVIO_DT_TIME), 1, s->files[1]);
						}
						/* swap if endian mismatch */
						if (s->hdr.bigendian != is_bigendian()) {
							byteswap(&cur,i*sizeof(CVIO_DT_TIME),
								sizeof(CVIO_DT_TIME));
						}

						/* pick cur (1), last (2) or err (3) */
						switch(s->pos.time_rule) {
							case CVIO_TIMERULE_ROUND:
								done = 1;
								if ((timeval[0]-last) < 
									(cur-timeval[0])) {
									done = 2;
								}
								break;
							case CVIO_TIMERULE_CEIL:
								done = 1;
								break;
							case CVIO_TIMERULE_FLOOR:
								done = 2;
								break;
							case CVIO_TIMERULE_INTERP:
								done = 3;
								break;
						}
					} else if (timeval[0] > cur) {
						low = t;
					} else if (timeval[0] < cur) {
						high = t;
					}
				}
			} else {
				/* brute force search the timestamp files */
				for(t=0;t<s->hdr.nsamp;t++) {
					gzread((gzFile)(s->files[1]), &cur,
					       sizeof(CVIO_DT_TIME));
					/* swap if endian mismatch */
					if (s->hdr.bigendian != is_bigendian()) {
						byteswap(&cur,i*sizeof(CVIO_DT_TIME),
							sizeof(CVIO_DT_TIME));
					}
					/* are we done yet? */
					if (timeval[0] == cur) {
						done = 1;
					} else if ((t != 0) && (last < timeval[0]) &&
						(cur > timeval[0])) {
						/* pick cur (1), last (2) or err (3) */
						switch(s->pos.time_rule) {
							case CVIO_TIMERULE_ROUND:
								done = 1;
								if ((timeval[0]-last) < 
									(cur-timeval[0])) {
									done = 2;
								}
								break;
							case CVIO_TIMERULE_CEIL:
								done = 1;
								break;
							case CVIO_TIMERULE_FLOOR:
								done = 2;
								break;
							case CVIO_TIMERULE_INTERP:
								done = 3;
								break;
						}
					}
					if (done) break;
					last = cur;
				}
			}
			/* pick the time val to return */

			switch(done) {
				case 1:
					timeval[0] = cur;
					s->pos.current_sample = t;
					break;
				case 2:
					if (t == s->hdr.nsamp - 1) {
						/* off the end due to last_time != any time in stream */
						timeval[0] = last;
						s->pos.current_sample = t;
					} else {
						timeval[0] = last;
						s->pos.current_sample = t-1;
					}
					break;
				case 3:
					/* interp between last and cur */
					if (!tbuff) {
						tbuff = (void *)malloc(i*iSize);
					}
					weight = (double)(timeval[0] - last) /
						 (cur - last);
					s->pos.current_sample = t-1;
					break;
			}

			/* read the sample and posibly reset the time ptr */
			if (s->status == STATUS_SHMEM) {
				if (buffer) {
				    memcpy(buffer, ((char *)s->data) + 
					(iSize*s->pos.current_sample),i*iSize);
				}
				if (done == 3) {
					memcpy(tbuff, ((char *)s->data) + 
					    (iSize*(s->pos.current_sample+1)),
					    i*iSize);
				}
			} else if (s->hdr.compression == 1) {
				if (buffer) {
				    gzseek((gzFile)s->files[2],
					iSize*s->pos.current_sample,SEEK_SET);
				    gzread((gzFile)(s->files[2]), buffer, 
					i*iSize);
				} else {
				    gzseek((gzFile)s->files[2],
					iSize*(s->pos.current_sample+i),
					SEEK_SET);
				}
				if (done == 2) {
					gzseek((gzFile)s->files[1],
					    t*sizeof(CVIO_DT_TIME),SEEK_SET);
				}
				if (done == 3) {
					gzread((gzFile)(s->files[2]), tbuff, 
						i*iSize);
					gzseek((gzFile)s->files[1],
					    t*sizeof(CVIO_DT_TIME),SEEK_SET);
				}
			} else {
				if (buffer) {
				    fseek(s->files[2],
					iSize*s->pos.current_sample,SEEK_SET);
				    fread(buffer, iSize, i, s->files[2]);
				} else {
				    fseek(s->files[2],
					iSize*(s->pos.current_sample+i),
					SEEK_SET);
				}
				if (done == 2) {
					fseek(s->files[1],
					    t*sizeof(CVIO_DT_TIME),SEEK_SET);
				}
				if (done == 3) {
					fread(tbuff, iSize, i, s->files[2]);
					fseek(s->files[1],
					    t*sizeof(CVIO_DT_TIME),SEEK_SET);
				}
			}
			/* swap if endian mismatch */
			if (s->hdr.bigendian != is_bigendian()) {
				if (buffer) {
				    byteswap(buffer,iSize*i,
					(s->hdr.type & CVIO_TYP_SIZE_MASK)/8);
				}
				if (done == 3) {
					byteswap(tbuff,iSize*i,(s->hdr.type 
						& CVIO_TYP_SIZE_MASK)/8);
				}
			}
			if (done == 3) {
			    if (buffer) {
				interp_sample(buffer,tbuff,iSize*i,
					s->hdr.type, weight);
			    }
			}
		}

		/* change current sample number and bump the last time value */
		s->pos.current_sample += i;
		s->pos.current_time = timeval[i-1];

		/* bump the data pointers */
		timeval += i;
		if (buffer) buffer = ((char*)buffer) + (iSize*i);

		/* we have read (i) more samples */
		count -= i;
	}

	if (tbuff) free(tbuff);

	return(CVIO_ERR_OK);
}

/*****cvio_read_samples_blocking*****/
/* Read up to count samples, return actual number of samples read in count
   if no samples are available, sleep until there are
   if timeout ticks pass before new data becomes available, CVIO_ERR_EOF is returned
*/

int32_t cvio_read_samples_blocking(uint32_t stream, CVIO_DT_TIME timeval[], void *buffer, int32_t *count, uint32_t timeout, int ttype)
{
	struct timeval tv;
	int werr = 0;
	uint32_t avail = 0;
	int32_t err, tcount;
	struct timespec to;
	struct timespec *toptr;
	int n;
	cvio_stream 	*s;

	s = valid_stream(stream);
	if (!s) return(CVIO_ERR_BADSTREAM);

	if (s->hdr.deleted) return CVIO_ERR_DELETED;
	if (*count == 0) return CVIO_ERR_OK;

	if (s->status == STATUS_SOCKET) {
                /* try to read up to *count, no longer than timeout */
                if (sock_read_samples(s,timeval,buffer,count,timeout)) {
			return(CVIO_ERR_STREAMCLOSED);
                }
                /* on timeout, return OK */
                if (*count == 0) return(CVIO_ERR_OK);
		/* swap if endian mismatch */
		if (s->hdr.bigendian != is_bigendian()) {
			byteswap(timeval,(*count)*sizeof(CVIO_DT_TIME),
				sizeof(CVIO_DT_TIME));
			if (buffer) {
			    byteswap(buffer,(*count)*cvio_sock_samp_size(s,0),
				(s->hdr.type & CVIO_TYP_SIZE_MASK)/8);
			}
		}
		s->pos.current_sample += *count;
                return(CVIO_ERR_OK);
	}

	tcount = *count;
	if (timeout && ttype == CVIO_TIMEOUT_LOCAL) {
		int sec;
/* in a posix world...
		clock_gettime(CLOCK_REALTIME, &to); */
		gettimeofday(&tv, NULL);
		to.tv_sec = tv.tv_sec;
		to.tv_nsec = tv.tv_usec * 1000;
		sec = timeout/CVIO_TIME_TICKS_PER_SEC;
		to.tv_sec += sec;
		timeout -= sec * CVIO_TIME_TICKS_PER_SEC;
		to.tv_nsec += timeout * (1000000000/CVIO_TIME_TICKS_PER_SEC);
		toptr = &to;
	} else toptr = NULL;

	if (s->status == STATUS_SHMEM) {
		memcpy(&(s->hdr), s->base, sizeof(header));
	}

	if (timeval[0] == CVIO_TIME_NEXT) {
		avail = s->hdr.nsamp - s->pos.current_sample;
		if (avail < tcount) tcount = avail;
	} else {
		if (timeval[0] > s->hdr.last_time)
			tcount = 0;
		else {
/*			printf("%ld <= %ld, read should succeed\n", timeval[0], s->hdr.last_time); */
			tcount = 1;
		}
	}

        if (tcount == 0 && s->status == STATUS_SHMEM) {
		int safe = 0;
		tcount = *count;
		lock(s->blocklock);
		for (n = 0; n < MAX_BLOCKERS; n++)
			/*
			  theoretically, a pid can be in the list multiple
			  times, once for each thread blocking
			*/
			if (s->bpids[n] == 0) {
				safe = 1;
				s->bpids[n] = getpid();
				break;
			}
		unlock(s->blocklock);
		assert(safe);
		if (!safe)
			return CVIO_ERR_NBLOCKERS;
		/* wait for more data signal (SIGUSR1), resleep if interrupted
		 * by any other signal
		 */

		pthread_mutex_lock(&sigusrlock);
		memcpy(&(s->hdr), s->base, sizeof(header));
		while ((s->hdr.writers > 0) && (werr != ETIMEDOUT)) {
			if ((timeval[0] == CVIO_TIME_NEXT)) {
				avail = s->hdr.nsamp - s->pos.current_sample;
			} else if (s->hdr.last_time >= timeval[0]) {
				printf("%d >= %d, read should be successful\n", s->hdr.last_time, timeval[0]);
				avail = 1;
			} else avail = 0;
			if (avail > 0) {
				break;
			}
			if (timeout && (ttype == CVIO_TIMEOUT_STREAM))
				if (s->hdr.last_time > timeout)
					break;
			assert(pthread_mutex_trylock(&sigusrlock));
			if (toptr == NULL) {
				werr = pthread_cond_wait(&sigusrcond, &sigusrlock);
				assert(werr != EINVAL);
			} else {
				werr = pthread_cond_timedwait(&sigusrcond, &sigusrlock, toptr);
				assert(werr != EINVAL);
			}

			assert(werr != EINVAL);
			memcpy(&(s->hdr), s->base, sizeof(header));
		}
		if (timeval[0] == CVIO_TIME_NEXT) {
			avail = s->hdr.nsamp - s->pos.current_sample;
		} else if (s->hdr.last_time >= timeval[0]) {
			printf("%d >= %d, read should be successful\n", s->hdr.last_time, timeval[0]);
			avail = 1;
		} else avail = 0;

		pthread_mutex_unlock(&sigusrlock);
		lock(s->blocklock);
		for (n = 0; n < MAX_BLOCKERS; n++)
			if (s->bpids[n] == getpid()) {
/*XXX could just remember the position... */
				s->bpids[n] = 0;
				break;
			}
		unlock(s->blocklock);

		if (avail < tcount)
			tcount = avail;
	}
	if (s->hdr.deleted)
		return CVIO_ERR_DELETED;

	/* if count is 0 now, we've hit EOF and we've hit timeout */
	if (tcount == 0) {
		if (s->hdr.writers > 0)
			err = CVIO_ERR_OK;
		else
			err = CVIO_ERR_EOF;
		*count = 0;
	} else {
		err = cvio_read_samples(stream, timeval, buffer, tcount);
		*count = tcount;
		if (err != CVIO_ERR_OK) {
			printf("err == %d\n", err);
			printf("Crashing, #writes on stream was %d\n", s->hdr.writers);
		}
		assert(err == CVIO_ERR_OK);
	}

	return err;
}
/*****cvio_getattribute*****/
/* get attribute to the necessary value */
int32_t cvio_getattribute(uint32_t stream, const char *attr_name, void *buffer, int32_t *bufferlen)
{	
	int32_t check;
	cvio_stream 	*s;

	s = valid_stream(stream);
	if (!s) return(CVIO_ERR_BADSTREAM);

	/* if using shared memory, update the header */
	if (s->status == STATUS_SHMEM) {
		memcpy(&(s->hdr), s->base, sizeof(header));
	}
	
	check = *bufferlen;		
	if (check < sizeof(CVIO_DT_USHORT)) return(CVIO_ERR_SHORTBUFFER);
	*bufferlen = sizeof(CVIO_DT_USHORT);

	if (strcmp(attr_name,"REQ_ATTR_DATE_ST") == 0) {
		if (check < 32) return(CVIO_ERR_SHORTBUFFER);
		strcpy((char *)buffer,s->hdr.date);
		*bufferlen = strlen(s->hdr.date);
	}
	else if (strcmp(attr_name,"REQ_ATTR_FTYPE_US") == 0) {
		*((CVIO_DT_USHORT *)buffer) = 0;
                if (s->status == STATUS_SHMEM) {
			*((CVIO_DT_USHORT *)buffer) = 1;
		}
	}
	else if (strcmp(attr_name,"REQ_ATTR_TYPE_US") == 0) {
		*((CVIO_DT_USHORT *)buffer) = s->hdr.type;
	}
	else if (strcmp(attr_name,"REQ_ATTR_NDIM_US") == 0) {
		*((CVIO_DT_USHORT *)buffer) = s->hdr.ndim;
	}
	else if (strcmp(attr_name,"REQ_ATTR_DIMS_US") == 0) {
	
		if (check < CVIO_MAX_DATA_DIMS*sizeof(CVIO_DT_USHORT)) return(CVIO_ERR_SHORTBUFFER);
		memcpy(buffer,s->hdr.dims,CVIO_MAX_DATA_DIMS*sizeof(CVIO_DT_USHORT));
		*bufferlen = CVIO_MAX_DATA_DIMS*sizeof(CVIO_DT_USHORT);
	}
	else if (strcmp(attr_name,"REQ_ATTR_SPACING_F") == 0) {
		if (check < 3*sizeof(CVIO_DT_FLOAT)) return(CVIO_ERR_SHORTBUFFER);
		memcpy(buffer,s->hdr.spacing,3*sizeof(CVIO_DT_FLOAT));
		*bufferlen = 3*sizeof(CVIO_DT_FLOAT);
	}
	else if (strcmp(attr_name,"REQ_ATTR_COMP_US") == 0) {
		*((CVIO_DT_USHORT *)buffer) = s->hdr.compression;
	}
	else if (strcmp(attr_name,"REQ_ATTR_NSAMP_UL") == 0) {
		if (check < sizeof(CVIO_DT_ULONG)) return(CVIO_ERR_SHORTBUFFER);
		*((CVIO_DT_ULONG *)buffer) = s->hdr.nsamp;
		*bufferlen = sizeof(CVIO_DT_ULONG);
	}
	else if (strcmp(attr_name,"REQ_ATTR_TIMESTAMPBASE_UL") == 0) {
		if (check < sizeof(CVIO_DT_ULONG)) return(CVIO_ERR_SHORTBUFFER);
		*((CVIO_DT_ULONG *)buffer) = s->hdr.timestampbase;
		*bufferlen = sizeof(CVIO_DT_ULONG);
	}
	else if (strcmp(attr_name,"REQ_ATTR_CURRSAMP_UL") == 0) {
		if (check < sizeof(CVIO_DT_ULONG)) return(CVIO_ERR_SHORTBUFFER);
		*((CVIO_DT_ULONG *)buffer) = s->pos.current_sample;
		*bufferlen = sizeof(CVIO_DT_ULONG);
	}
	else if (strcmp(attr_name,"REQ_ATTR_CURRTIME_TI") == 0) {
		if (check < sizeof(CVIO_DT_TIME)) return(CVIO_ERR_SHORTBUFFER);
		*((CVIO_DT_TIME *)buffer) = s->pos.current_time;
		*bufferlen = sizeof(CVIO_DT_TIME);
	}
	else if (strcmp(attr_name,"REQ_ATTR_TIMEIN_TI") == 0) {
		if (check < sizeof(CVIO_DT_TIME)) return(CVIO_ERR_SHORTBUFFER);
		*((CVIO_DT_TIME *)buffer) = s->hdr.first_time;
		*bufferlen = sizeof(CVIO_DT_TIME);
	}
	else if (strcmp(attr_name,"REQ_ATTR_TIMEOUT_TI") == 0) {
		if (check < sizeof(CVIO_DT_TIME)) return(CVIO_ERR_SHORTBUFFER);
		*((CVIO_DT_TIME *)buffer) = s->hdr.last_time;
		*bufferlen = sizeof(CVIO_DT_TIME);
	}
	else if (strcmp(attr_name,"REQ_ATTR_TIMERULE_US") == 0) {
		*((CVIO_DT_USHORT *)buffer) = s->pos.time_rule;
	} 
	else if (strcmp(attr_name,"REQ_ATTR_RWRITES_US") == 0) {
		*((CVIO_DT_USHORT *)buffer) = s->hdr.writers;
	} else {
		int32_t	i;
		i = 0;
		while(s->hdr.auxattribs[i][0]) {
			if (strcmp(attr_name,s->hdr.auxattribs[i]) == 0) {
				int32_t	len;
				char 	*p; 

				len = strlen(s->hdr.auxattribs[i])+1;
				p = s->hdr.auxattribs[i] + len;
				len = strlen(p);

				if (check < len+1) return(CVIO_ERR_SHORTBUFFER);
				strcpy((char *)buffer,p);
				*bufferlen = len;
				return(CVIO_ERR_OK);
			}
			i++;
		}
		return(CVIO_ERR_UNKNOWN_ATTR);
	}

	return(CVIO_ERR_OK);
}  

/* set an "unknown" attribute */
static int32_t cvio_setunknownattribute(cvio_stream *s,const char *attr_name, 
	char *buffer)
{
	int32_t	l1,l2 = 0;
	int32_t	i;
	
	l1 = strlen(attr_name);
	if (buffer) l2 = strlen(buffer);

	if ((attr_name[l1-1] != 'T') || (attr_name[l1-2] != 'S') ||
            (attr_name[l1-3] != '_')) return(0);

	if (l1+l2+2 >= MAX_ATTRIB_LEN) return(0);

/* change or add an aux attribute */
	if (buffer) {
		i = 0;
		while(s->hdr.auxattribs[i][0]) {
			if (strcmp(attr_name,s->hdr.auxattribs[i]) == 0) {
				/* change */
				strcpy(s->hdr.auxattribs[i]+l1+1,buffer);
				return(1);
			}
			i++;
		}
		/* add */
		if (i < (MAX_AUX_ATTRIBS-1)) {
			strcpy(s->hdr.auxattribs[i],attr_name);
			strcpy(s->hdr.auxattribs[i]+l1+1,buffer);
			return(1);
		}
		return(0);
	}
/* remove an aux attribute... */
	i = 0;
	while(s->hdr.auxattribs[i][0]) {
		if (strcmp(attr_name,s->hdr.auxattribs[i]) == 0) {
			s->hdr.auxattribs[i][0] = '\0';
			i++;
			while(s->hdr.auxattribs[i][0]) {
				memcpy(s->hdr.auxattribs[i-1],
					s->hdr.auxattribs[i],MAX_ATTRIB_LEN);
				s->hdr.auxattribs[i][0] = '\0';
				i++;
			}
			return(1);
		}
		i++;
	}
	return(0);
}

/*****cvio_setattribute*****/
/* set attribute to the necessary value */
int32_t cvio_setattribute(uint32_t stream, const char *attr_name, void *buffer)
{
	int32_t iSize;
	cvio_stream *s;

	/* determine whether a valid stream */
	s = valid_stream(stream);
	if (!s) return(CVIO_ERR_BADSTREAM);

	/* if using shared memory, update the header */
	if (s->status == STATUS_SHMEM) {
		memcpy(&(s->hdr), s->base, sizeof(header));
	}
	if (s->access != CVIO_ACCESS_READ) {
		if (cvio_setunknownattribute(s,attr_name,buffer)) goto ok;
		if (strcmp(attr_name,"REQ_ATTR_SPACING_F") == 0) {
			if (!buffer) return(CVIO_ERR_BADACCESS);
			memcpy(s->hdr.spacing,buffer,sizeof(CVIO_DT_FLOAT)*3);
			goto ok;
		}
		return(CVIO_ERR_UNKNOWN_ATTR);
	}

	if (!buffer) return(CVIO_ERR_BADACCESS);

	/* find the size of a single sample in chars */
	iSize = (s->hdr.type & CVIO_TYP_SIZE_MASK)*
		s->hdr.dims[0]*s->hdr.dims[1]*s->hdr.dims[2]/8;
		
	if (s->status == STATUS_SHMEM) {
		if (strcmp(attr_name,"REQ_ATTR_CURRSAMP_UL") == 0) {
			if (*((CVIO_DT_ULONG *)buffer) >= s->hdr.nsamp) {
				return(CVIO_ERR_NOSAMPLE);	
			}
			s->pos.current_sample = *((CVIO_DT_ULONG *)buffer);
			memcpy(&s->pos.current_time, s->time+s->pos.current_sample,
				 sizeof(CVIO_DT_TIME));						
		} else if (strcmp(attr_name,"REQ_ATTR_TIMERULE_US") == 0) {
			if ((*((CVIO_DT_USHORT *)buffer) != CVIO_TIMERULE_FLOOR)
   		   	   && (*((CVIO_DT_USHORT *)buffer) != CVIO_TIMERULE_CEIL)
   		   	   && (*((CVIO_DT_USHORT *)buffer) != CVIO_TIMERULE_INTERP)
			   && (*((CVIO_DT_USHORT *)buffer) != CVIO_TIMERULE_ROUND)) {
				return(CVIO_ERR_INVALIDRULE);
			} else {
				if ((s->hdr.type == CVIO_TYP_STRING) && 
					(*((CVIO_DT_USHORT *)buffer) == 
					CVIO_TIMERULE_INTERP)) {
					return(CVIO_ERR_INVALIDRULE);
				}
				s->pos.time_rule = *((CVIO_DT_USHORT *)buffer);
			} 
		} else {
/* Warning, this will not stick!!! */
			if (!cvio_setunknownattribute(s,attr_name,buffer)) {
				return(CVIO_ERR_UNKNOWN_ATTR);
			}
		}	
	} else if (s->status == STATUS_FILE) {
		if (strcmp(attr_name,"REQ_ATTR_CURRSAMP_UL") == 0) {
			if (*((CVIO_DT_ULONG *)buffer) >= s->hdr.nsamp) {
				return(CVIO_ERR_NOSAMPLE);	
			}
			if (s->hdr.compression == 1) {
				s->pos.current_sample = *((CVIO_DT_ULONG *)buffer);;
				gzseek((gzFile)s->files[2], iSize*(s->pos.current_sample), SEEK_SET);
				gzseek((gzFile)s->files[1], sizeof(CVIO_DT_TIME)*(s->pos.current_sample), SEEK_SET);
				gzread((gzFile)s->files[1], &s->pos.current_time, sizeof(CVIO_DT_TIME));
				gzseek((gzFile)s->files[1], sizeof(CVIO_DT_TIME)*(s->pos.current_sample), SEEK_SET);
			} else {
				s->pos.current_sample = *((CVIO_DT_ULONG *)buffer);;
				fseek(s->files[2], iSize*(s->pos.current_sample), SEEK_SET);
				fseek(s->files[1], sizeof(CVIO_DT_TIME)*(s->pos.current_sample), SEEK_SET);
				fread(&s->pos.current_time, sizeof(CVIO_DT_TIME), 1, s->files[1]);
				fseek(s->files[1], sizeof(CVIO_DT_TIME)*(s->pos.current_sample), SEEK_SET);
			}
		} else if ((strcmp(attr_name,"REQ_ATTR_TIMERULE_US")) == 0) {
			if ((*((CVIO_DT_USHORT *)buffer) != CVIO_TIMERULE_FLOOR)
			   && (*((CVIO_DT_USHORT *)buffer) != CVIO_TIMERULE_CEIL)
			   && (*((CVIO_DT_USHORT *)buffer) != CVIO_TIMERULE_INTERP)
			   && (*((CVIO_DT_USHORT *)buffer) != CVIO_TIMERULE_ROUND)) {
				return(CVIO_ERR_INVALIDRULE);
			} else {
				if ((s->hdr.type == CVIO_TYP_STRING) && 
					(*((CVIO_DT_USHORT *)buffer) == 
					CVIO_TIMERULE_INTERP)) {
					return(CVIO_ERR_INVALIDRULE);
				}
				s->pos.time_rule = *((CVIO_DT_USHORT *)buffer);
			}
		} else {
/* Warning, this will not stick!!! */
			if (!cvio_setunknownattribute(s,attr_name,buffer)) {
				return(CVIO_ERR_UNKNOWN_ATTR);
			}
		}
	}

ok:
	/* if using shared memory, update the header */
	if (s->status == STATUS_SHMEM) {
		memcpy(s->base, &(s->hdr), sizeof(header));
	}
	
	return(CVIO_ERR_OK);
}

/*****cvio_getattriblist*****/
/* get entire list of attributes */
int32_t cvio_getattriblist(uint32_t stream, char *buffer, int32_t *bufferlen, int32_t *num_attribs)
{		 
	char	*list[] = {
		"REQ_ATTR_DATE_ST","REQ_ATTR_TYPE_US","REQ_ATTR_NDIM_US",
		"REQ_ATTR_DIMS_US","REQ_ATTR_COMP_US","REQ_ATTR_NSAMP_UL",
		"REQ_ATTR_CURRSAMP_UL","REQ_ATTR_CURRTIME_TI",
		"REQ_ATTR_TIMEIN_TI","REQ_ATTR_TIMEOUT_TI",
		"REQ_ATTR_TIMERULE_US","REQ_ATTR_TIMESTAMPBASE_UL",
		"REQ_ATTR_SPACING_F", NULL
	};

	cvio_stream 	*s;
	char		*p;
	int32_t		len,num,i;

	s = valid_stream(stream);
	if (!s) return(CVIO_ERR_BADSTREAM);

	*num_attribs = 0;
	num = 0;
	p = buffer;
	i = 0;
	while(list[i]) {
		len = strlen(list[i])+1;
		if (num + len > *bufferlen) {
			*bufferlen = num;
			return(CVIO_ERR_SHORTBUFFER);
		}
		strcpy(p,list[i]);
		*num_attribs += 1;
		p += len;
		num += len;
		i++;
	}
	i = 0;
	while(s->hdr.auxattribs[i][0]) {
		len = strlen(s->hdr.auxattribs[i])+1;
		if (num + len > *bufferlen) {
			*bufferlen = num;
			return(CVIO_ERR_SHORTBUFFER);
		}
		strcpy(p,s->hdr.auxattribs[i]);
		*num_attribs += 1;
		p += len;
		num += len;
		i++;
	}
	*bufferlen = num;
	
	return(CVIO_ERR_OK);
}

/*****cvio_current_time*****/
/* gets the timestamp for now (msec) */
CVIO_DT_TIME cvio_current_time(void)
{
	CVIO_DT_TIME	t;

#ifdef WIN32
	DWORD ticks;
	ticks = GetTickCount();
	t = (float)(ticks)*(CVIO_TIME_TICKS_PER_SEC/1000.);
#else
	struct timeval tv;
	struct timezone tz;

	gettimeofday(&tv, &tz);

	t = tv.tv_sec % (60*60*24);                    /* seconds in a day */
	t *= CVIO_TIME_TICKS_PER_SEC;	               /* in TICKS_PER_SEC */
	t += (tv.tv_usec*
		(CVIO_TIME_TICKS_PER_SEC/1000000.));    /* + number of msec */
#endif

	return(t);
}

/*****cvio_sleep*****/
/* sleeps a process for N-seconds (w/msec accuracy if possible) */
/* if the time is 0.0, it will just yield the CPU timeslice (if
   possible) */
int32_t cvio_sleep(CVIO_DT_FLOAT t)
{
	/* if the system has a known "yield" call, use it */
	if (t == 0.0) {
#ifdef linux
		sched_yield();
		return(CVIO_ERR_OK);
#endif
#ifdef sgi
		sginap(0);
		return(CVIO_ERR_OK);
#endif
		t = 0.001;  /* or fake "yield" w/1msec sleep */
	}

	/* do an actual sleep */
	if (floor(t) > 0.0) sleep(floor(t));
	t = 1000000.0*(t - floor(t)); 
	usleep(t);

	return(CVIO_ERR_OK);
}

/* derived utility functions */

int32_t cvio_read_next_samples(uint32_t stream, CVIO_DT_TIME timeval[],
        void *buffer,int32_t count)
{
	int32_t	i;
	for(i=0;i<count;i++) timeval[i] = CVIO_TIME_NEXT;
	return(cvio_read_samples(stream,timeval,buffer,count));
}

int32_t cvio_read_next_samples_blocking(uint32_t stream, CVIO_DT_TIME timeval[],
        void *buffer, int32_t *count, uint32_t timeout, int ttype)
{
	int32_t i;
	
	for (i = 0; i < *count; i++) timeval[i] = CVIO_TIME_NEXT;
	return cvio_read_samples_blocking(stream, timeval, buffer, count, timeout, ttype);
}


int32_t cvio_tell(uint32_t stream, uint32_t *curr_samp, CVIO_DT_TIME *curr_time,
	uint32_t *n_samples)
{
	cvio_stream *s;

	/* determine whether a valid stream */
	s = valid_stream(stream);
	if (!s) return(CVIO_ERR_BADSTREAM);

	/* if using shared memory, update the header */
	if (s->status == STATUS_SHMEM) {
		memcpy(&(s->hdr), s->base, sizeof(header));
	}

	*curr_samp = s->pos.current_sample;
	*curr_time = s->pos.current_time;
	*n_samples = s->hdr.nsamp;

	return(CVIO_ERR_OK);
}

int32_t cvio_seek(uint32_t stream, int32_t is_time, uint32_t where)
{
	int32_t	err;
	cvio_stream *s;

	/* determine whether a valid stream */
	s = valid_stream(stream);
	if (!s) return(CVIO_ERR_BADSTREAM);

        if (s->status == STATUS_SOCKET) return(CVIO_ERR_INVALIDACCESS);

	if (is_time) {
		err = cvio_setattribute(stream,"REQ_ATTR_CURRTIME_TI", &where);
	} else {
		err = cvio_setattribute(stream,"REQ_ATTR_CURRSAMP_UL", &where);
	}
	return(err);
}

int32_t cvio_datatype(uint32_t stream, int32_t *type, int32_t *ndims, int32_t dims[CVIO_MAX_DATA_DIMS])
{
	cvio_stream *s;

	/* determine whether a valid stream */
	s = valid_stream(stream);
	if (!s) return(CVIO_ERR_BADSTREAM);

	*type = s->hdr.type;
	*ndims = s->hdr.ndim;
	dims[0] = s->hdr.dims[0];
	dims[1] = s->hdr.dims[1];
	dims[2] = s->hdr.dims[2];

	return(CVIO_ERR_OK);
}

#ifdef TEST
int main(int argc, char **argv)
{
        int32_t 	nsam, ndim, dim[3], status1, status2, status3, i, status45;
        uint32_t 	s_id;
	char	buffer[256];

	char	*name = "chris";
        CVIO_DT_TIME msec[256], get_times[25]; 
	char recovered_values[25*10*15*5];
        char test_data[256*10*15*5];

        nsam = 25;
        ndim = 3;
        dim[0] = 10;
        dim[1] = 15;
        dim[2] = 5;

	printf("Time:%d\n",cvio_current_time());

        status1 = cvio_create(name, nsam, CVIO_TYP_BYTE, ndim, dim, 0);
        printf("%d\n", status1);
        printf("i got here16\n");
        status2 = cvio_open(name, CVIO_ACCESS_APPEND, &s_id);
        printf("%d\n", status2);
        printf("i got here18\n");

        for (i=0; i<256*10*15*5; i++) {
                test_data[i] = i*56;
        }
        for (i=0; i<256; i++) {
                msec[i] = i*5+2;  /* time in msec */
	}

	status2 = cvio_add_samples(s_id, msec, test_data, 256);
        printf("%d\n", status2);

	status2 = cvio_add_samples(s_id, msec, test_data, 256);
        printf("(should fail:overlapping timevals:%d\n", status2);

        status3 = cvio_close( s_id );
        printf("close:%d\n", status3);

	for (i=0; i<25; i++) {
		get_times[i] = CVIO_TIME_NEXT;
	}

        status2 = cvio_open(name, CVIO_ACCESS_READ, &s_id);
        printf("Open read:%d\n", status2);

	status45 = cvio_read_samples(s_id, get_times, recovered_values, 25);
	printf("read samples:%d\n", status45);
	
	for(i=0;i<25;i++) {
		printf("%d - %d : %d %d\n",
			msec[i],get_times[i],
			test_data[i*10*15*5],recovered_values[i*10*15*5]);
	}

        status3 = cvio_close( s_id );
        printf("close:%d\n", status3);

	if (cvio_delete(name)) {
		printf("delete error!\n");
	}
        if (status2 = cvio_open(name, CVIO_ACCESS_READ, &s_id)) {
		char	err[256];
		status1 = 255;
		cvio_get_error_message(status2,err,&status1);
		printf("Should Error: %s\n",err);
	}

	name = ":shmem:chris";
        status2 = cvio_open(name, CVIO_ACCESS_READ, &s_id);
        printf("%d\n", status2);
        status1 = cvio_create(name, nsam, CVIO_TYP_BYTE, ndim, dim, 0);
        printf("%d\n", status1);
        status2 = cvio_open(name, CVIO_ACCESS_READ, &s_id);
        printf("%d\n", status2);
        status3 = cvio_close( s_id );
        printf("%d\n", status3);
        status3 = cvio_delete( name );
        printf("%d\n", status3);

	status1 = cvio_create(":shmem:hello",nsam,CVIO_TYP_BYTE,ndim,dim,0);
	status1 = cvio_create(":shmem:bye",nsam,CVIO_TYP_BYTE,ndim,dim,0);
	status1 = cvio_create(":shmem:test",nsam,CVIO_TYP_BYTE,ndim,dim,0);

	status2 = 256;
	status3 = 0;
	status1 = cvio_listshmem(buffer, &status2, &status3);
        printf("%d\n", status1);
	for(i=0;i<status2;i++) {
		if (buffer[i]) {
			printf("%c",buffer[i]);
		} else {
			printf("\n");
		}
	}

	status1 = cvio_delete(":shmem:hello");
	status1 = cvio_delete(":shmem:bye");
	status1 = cvio_delete(":shmem:test");

	printf("Time:%d\n",cvio_current_time());
	sleep(1);
	printf("Time:%d\n",cvio_current_time());

	exit(0);
}
#endif
