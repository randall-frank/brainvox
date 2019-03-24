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

#ifndef __CVIO_H__
#define __CVIO_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Public (external) interface for the CVIO library */
#define CVIO_FILE_VERSION	"1.0"
#define CVIO_LIBRARY_VERSION	"1.11"

/* data types */

typedef uint8_t            CVIO_DT_BOOL;
typedef float              CVIO_DT_FLOAT;
typedef uint8_t	     	   CVIO_DT_BYTE;
typedef uint16_t           CVIO_DT_USHORT;
typedef int16_t            CVIO_DT_SHORT;
typedef char               CVIO_DT_STRING;
typedef uint32_t           CVIO_DT_ULONG;
typedef uint32_t           CVIO_DT_TIME;

/* maximum values for the data types */
#define CVIO_DT_BYTE_MAX	0xff
#define CVIO_DT_USHORT_MAX	0xffff
#define CVIO_DT_SHORT_MAX	0x7fff
#define CVIO_DT_FLOAT_MAX	3.40282347e+38F

/* Please think twice before changing this number!!! */
#define CVIO_MAX_DATA_DIMS	3

/* "special" time value meaning "next" timestamp */
#define CVIO_TIME_NEXT		0xffffffff 

/* number of ticks in one second of the current timestamp */
#define CVIO_TIME_TICKS_PER_SEC	20000

/* defines for the valid stream data types */
#define CVIO_TYP_BOOL		0x0208
#define CVIO_TYP_IMPULSE	0x0100
#define	CVIO_TYP_STRING		0x0108
#define	CVIO_TYP_BYTE		0x0008
#define	CVIO_TYP_SHORT		0x0010
#define	CVIO_TYP_USHORT		0x0110
#define	CVIO_TYP_FLOAT		0x0020
#define CVIO_TYP_SIZE_MASK 	0x00ff

/* file opening permissions */
#define CVIO_ACCESS_READ   	1
#define CVIO_ACCESS_APPEND 	2
#define CVIO_ACCESS_SEQUENTIAL 	4

/* define the timerules */
#define CVIO_TIMERULE_FLOOR	0
#define CVIO_TIMERULE_CEIL	1
#define CVIO_TIMERULE_ROUND	2
#define CVIO_TIMERULE_INTERP	3

/* cleanup flags */
#define	CVIO_CLEANUP_RMSHMEM	1

/* create flags */
#define	CVIO_CREATE_GZIP	1

/* error codes */
#define	CVIO_ERR_OK		0
#define CVIO_ERR_SHORTBUFFER	1
#define CVIO_ERR_NOMEMORY	2
#define CVIO_ERR_UNKNOWN_ATTR	3
#define CVIO_ERR_INVALIDACCESS  4
#define CVIO_ERR_MAXSTREAMS     5
#define CVIO_ERR_BADSTREAM	6
#define CVIO_ERR_BADFILENAME	7
#define CVIO_ERR_MISSINGREQTAGS	8
#define CVIO_ERR_NOCOMPSHMEM	9
#define CVIO_ERR_NOSHMEM	10
#define CVIO_ERR_SHMEMLOCKED	11
#define CVIO_ERR_BADACCESS	12
#define CVIO_ERR_TIMESEQ	13
#define CVIO_ERR_NOSAMPLE	14
#define CVIO_ERR_EOF		15
#define CVIO_ERR_INVALIDRULE	16
#define CVIO_ERR_BADDATATYPE	17
#define CVIO_ERR_BADDIMENSIONS	18
#define CVIO_ERR_NBLOCKERS	19
#define CVIO_ERR_STREAMCLOSED	20
#define CVIO_ERR_DELETED	21

/* timeout types */
#define CVIO_TIMEOUT_LOCAL	0
#define CVIO_TIMEOUT_STREAM	1

#define cvio_log(ts, ...) do {\
	if (ts)\
		printf("%d:", cvio_current_time());\
	printf(__VA_ARGS__);\
	fflush(stdout);\
	} while (0)

/* prototypes */
int32_t cvio_get_error_message(int32_t errorval, char *buffer, int32_t *bufferlen);
int32_t cvio_init(void);
int32_t cvio_cleanup(int32_t cleanupflag);
int32_t cvio_listshmem(char *buffer, int32_t *bufferlen, int32_t *num_strings);
int32_t cvio_listsockets(char *host, char *names, int32_t *size, int32_t *num);
int32_t cvio_create(const char *filename, int32_t nsamples, int32_t type, int32_t ndims,
	int32_t dims[CVIO_MAX_DATA_DIMS],int32_t flags);
int32_t cvio_delete(const char *filename);
int32_t cvio_open(const char *filename, uint32_t access, uint32_t *stream);
int32_t cvio_max_length(uint32_t stream,uint32_t *len);
int32_t cvio_close(uint32_t stream);
int32_t cvio_add_samples(uint32_t stream, CVIO_DT_TIME timeval[], void *buffer,
	int32_t count);
int32_t cvio_read_samples(uint32_t stream, CVIO_DT_TIME timeval[], void *buffer,
	int32_t count);
int32_t cvio_read_samples_blocking(uint32_t stream, CVIO_DT_TIME timeval[],
    void *buffer, int32_t *count, uint32_t timeout, int ttype);
int32_t cvio_getattribute(uint32_t stream, const char *attr_name, void *buffer,
	int32_t *bufferlen);
int32_t cvio_setattribute(uint32_t stream, const char *attr_name, void *buffer);
int32_t cvio_getattriblist(uint32_t stream, char *buffer, int32_t *bufferlen,
	int32_t *num_attribs);
CVIO_DT_TIME cvio_current_time(void);
int32_t cvio_read_next_samples(uint32_t stream, CVIO_DT_TIME timeval[],void *buffer,
	int32_t count);
int32_t cvio_read_next_samples_blocking(uint32_t stream, CVIO_DT_TIME timeval[],
    void *buffer, int32_t *count, uint32_t timeout, int ttype);
int32_t cvio_tell(uint32_t stream, uint32_t *curr_sample, CVIO_DT_TIME *curr_time,
	uint32_t *n_samples);
int32_t cvio_seek(uint32_t stream, int32_t is_time, uint32_t where);
int32_t cvio_datatype(uint32_t stream, int32_t *type, int32_t *ndims, int32_t dims[3]);
int32_t cvio_sleep(CVIO_DT_FLOAT length);

#ifdef __cplusplus
}
#endif

#endif
