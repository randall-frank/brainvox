/*
 * Copyright (c), 1990-2005 Randall Frank and Thomas Grabowski
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

#ifndef __CVIO_PRIV_H__
#define __CVIO_PRIV_H__

#include "cvio_lib.h"

/* configuration parameters */
#define MAX_STREAMS     500
#define MAX_FILENAME    1024
#define MAX_BLOCKERS    20

/* filename extensions */
#define EXT_TIM ".tim"
#define EXT_DAT ".dat"
#ifdef WIN32
#define EXT_TIM_GZ ".tgz"
#define EXT_DAT_GZ ".dgz"
#else
#define EXT_TIM_GZ ".tim.gz"
#define EXT_DAT_GZ ".dat.gz"
#endif

/* current stream status */
#define STATUS_UNUSED   0
#define STATUS_FILE     1
#define STATUS_SHMEM    2
#define STATUS_SOCKET   3

/* some extra defines */
#define PERMS              0333
#define SHM_BASE           0x92676

/* port scanning/allocation */
#define CVIO_PORT_BASE  0x4136
#define CVIO_PORT_NONE  -1

/* attribute limits */
#define MAX_AUX_ATTRIBS    50
#define MAX_ATTRIB_LEN     150

/* header flags */
#define HDR_FLAGS_HAS_WRITER    1

/* use "SOCKET" because of WIN32 issues */
#ifndef WIN32
#define SOCKET	int
#endif

/* internal structure definitions */
typedef struct {
        char    filename[MAX_FILENAME];
        key_t   semkey;
        char    date[32];
        CVIO_DT_FLOAT  spacing[CVIO_MAX_DATA_DIMS];
        CVIO_DT_ULONG  flags;
        CVIO_DT_USHORT type;
        CVIO_DT_USHORT ndim;
        CVIO_DT_USHORT dims[CVIO_MAX_DATA_DIMS];
        CVIO_DT_USHORT compression;
        CVIO_DT_ULONG  nsamp;
        CVIO_DT_TIME   first_time;
        CVIO_DT_TIME   last_time;
        CVIO_DT_USHORT bigendian;
        CVIO_DT_ULONG  timestampbase;
	int32_t writers;
	int32_t deleted;
        int32_t max_samples; /* used for shmem, other notional only */
        char    auxattribs[MAX_AUX_ATTRIBS][MAX_ATTRIB_LEN];
} header;

typedef struct {
        CVIO_DT_USHORT time_rule;
        CVIO_DT_ULONG current_sample;
        CVIO_DT_TIME current_time;
} position;

struct sbuffer;

/* one of these is defined for each opened I/O stream */
typedef struct {
        int32_t         status;
        int32_t         flags;
        int32_t         access;
        header          hdr;
        position        pos;
        FILE            *files[3]; /* for files */
        int             shm_id;    /* shmem:shared memory id */
        void            *base;     /* shmem:pointer to attachment loc */
        void            *data;     /* shmem:pointer to data */
        CVIO_DT_TIME    *time;     /* shmem:pointer to time values */
        pid_t           *bpids;    /* shmem:pointer to blocked pids */
        int             blocklock;

/* socket items */
	pthread_t	worker;   /* interface thread: socket create/writers*/
	pthread_mutex_t	s_mutex;  /* interface bwtn ithread & writer */
	SOCKET		s_listen; /* socket to accept connectons on */
	int		port;
	int		n_readers;
	SOCKET		*s_readers; /* array of connected sockets */
	int		n_wait;     /* number of cxns we are waiting for */
	int		f_wait;     /* timeout if still waiting for cxns */
	struct sbuffer	*sbuf;      /* sample buffer */
} cvio_stream;

/* shared memory layout:
        header {sizeof(header)}
        timestamps {header.nsamp*sizeof(CVIO_DT_TIME)}
        rawdata {header.nsamps*sizeof(one sample)}
*/

/* 
 filename formats:

 "basename"    -> basename + basename.(see TIM & DAT extensions above)
 ":shmem:name" -> shared memory block named "string"
 ":sock:hostname:name[:port=p][:wait=n][timeout=f] -> socket, writer waits for "n" readers (if on shutdown, there are still not enough reader for timeout secs, exit

*/

/* prototypes */
void cvio_sock_shutdown(cvio_stream *s);
int cvio_sock_startup(cvio_stream *s,char *host,char *name,int port,int wait,
                      float f_wait);
void *cvio_sock_writeproc(void *inarg);
void *cvio_sock_readproc(void *inarg);
int parse_sock(const char *in,char *host,char *name,int *port,int *wait,
               float *waittime);
int cvio_sock_connect(cvio_stream *s, char *host,char *name,int port);
int sock_add_samples(cvio_stream *s,CVIO_DT_TIME *times,void *samps,
   int32_t num);
int sock_read_samples(cvio_stream *s,CVIO_DT_TIME *times,void *samps,
   int32_t *num, uint32_t timeout);

void byteswap(void *buffer,int32_t len,int32_t swapsize);
int is_bigendian(void);

int cvio_sock_samp_size(cvio_stream *s, int time);
int cvio_sock_find(const char *name,header *h,int what);


#endif
