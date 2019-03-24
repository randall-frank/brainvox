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

/*****cvio_impulse*****/

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <cvio_lib.h>

/*
 * TODO:
 */

int32_t 	verbose = 0;
int32_t   max_impulses = 1000;
int32_t	tstext = 0;

static void *alloc_sample_buffer(uint32_t stream,int32_t num);
static void compute_sum(CVIO_DT_TIME t,void *samp, int32_t type,int32_t elements,
		CVIO_DT_TIME *ir_t,char *ir_d,float weight);
static void compute_sample(CVIO_DT_TIME t,CVIO_DT_TIME *tlist,void *samp,
	int32_t type,int32_t elements,int32_t ir_n,CVIO_DT_TIME *ir_t,char *ir_d,
	float *tweight);
static float get_weight_sample(uint32_t stream,void *samp);

void info(char *app)
{
        fprintf(stderr,"(%s) Usage: %s [options] impulseresp impulses timefile output \n",__DATE__,app);
        fprintf(stderr,"Options:\n");
        fprintf(stderr,"        -v verbose\n");
        fprintf(stderr,"        -c specifies the output file is to be compressed.\n");
        fprintf(stderr,"        -w enable weighting by the impulse value.  Default weight=1.0\n");
        fprintf(stderr,"        -W(width) if weighted, pick reject threshold width.  Default 0.0\n");
        fprintf(stderr,"        -C(center) if weighted, pick reject threshold center.  Default 0.0\n");
        fprintf(stderr,"        -m(num) set the maximum number of active impulses.  Default 1000\n");
	fprintf(stderr,"	-d don't create output file (it already exists)\n");
	fprintf(stderr,"	-T timestamp all textual output\n");
        exit(1);
}

void	exit_cvio_error(int32_t err) 
{
	char	tstr[256];
	int32_t	tlen = 256;

	cvio_get_error_message(err,tstr,&tlen);
	cvio_log(1,"cvio_impulse: Fatal error:%s\n",tstr);

	cvio_cleanup(0);
	exit(1);
}

int main(int argc, char *argv[])
{
	int curalloc = 20000;
	int got;
	int32_t count;
	int32_t	i,iSize,done;
	char	*impulse_d,*samp_d,*ev_d,*time_d;
	float *impulse_f;
	CVIO_DT_TIME	*impulse_t,ev_t,time_t;
	int32_t	compress = 0;
	int32_t	use_weights = 0;
	int32_t	dontcreate = 0;
	float	cent = 0.0;
	float	width = 0.0;
	char	*timefile;
	char	*irfile;
	char	*srcfile;
	char	*dstfile;
	uint32_t	ev_s,dst_s,time_s,irf_s;
	int32_t	err;
	int32_t	datatype,ndims,dims[3];
	uint32_t	nsamp,maxsamp,nElements;
	CVIO_DT_TIME    c_t;
	CVIO_DT_TIME	*t_list;
	float		*t_weight;
	/* parse the options */
        i = 1;
        while((i<argc) && (argv[i][0] == '-')) {
                switch(argv[i][1]) {
                        case 'v' : verbose = strlen(argv[i])-1; break;
                        case 'm' : max_impulses = atoi(argv[i]+2); break;
                        case 'C' : cent = atof(argv[i]+2); break;
                        case 'W' : width = atof(argv[i]+2); break;
                        case 'w' : use_weights = 1; break;
                        case 'c' : compress = CVIO_CREATE_GZIP; break;
			case 'd' : dontcreate = 1; break;
			case 'T' : tstext = 1; break;
                        default: info(argv[0]); break;
                }
                i++;
	}

	if (argc-i != 4) info(argv[0]);

	/* get the filenames */
	irfile = argv[i];
	srcfile = argv[i+1];
	timefile = argv[i+2];
	dstfile = argv[i+3];

	/* allocate buffer space */
	t_list = (CVIO_DT_TIME *)malloc(max_impulses*sizeof(CVIO_DT_TIME));
	t_weight = (float *)malloc(max_impulses*sizeof(float));
	if ((!t_list) || (!t_weight)) {
		cvio_log(tstext, "Unable to allocate buffer memory\n");
		exit(1);
	}

	/* init the cvio library */
	if (cvio_init()) exit(1);

	/* clear the event list */
	for(i=0;i<max_impulses;i++) t_list[i] = CVIO_TIME_NEXT;

	/* read the impulse response file */
	err = cvio_open(irfile,CVIO_ACCESS_READ,&irf_s);
	if (err) exit_cvio_error(err);

        /* get its format and size */
        err = cvio_datatype(irf_s,&datatype,&ndims,dims);
        if (err) exit_cvio_error(err);

        /* calculate sample size */
	nElements = dims[0]*dims[1]*dims[2];
        iSize = (datatype & CVIO_TYP_SIZE_MASK)*nElements/8;

	/* get the memory for the impulse response */
	impulse_d = malloc(curalloc*iSize);
	impulse_t = malloc(curalloc*sizeof(CVIO_DT_TIME));
	if (!impulse_d || !impulse_t) {
		cvio_log(tstext,"Out of memory\n");
		exit(1);
	}

	/* read the impulse response */
	got = 0;
	err = CVIO_ERR_OK;
	while (err == CVIO_ERR_OK) {
		count = curalloc - got;
		err = cvio_read_next_samples_blocking(irf_s, impulse_t + got, impulse_d + got * iSize, &count, 0, 0);
		got += count;
		if (curalloc - got == 0) {
			curalloc += 20000;
			impulse_t = realloc(impulse_t, curalloc * sizeof(CVIO_DT_TIME));
			impulse_d = realloc(impulse_d, curalloc * iSize);
			if (!impulse_d || !impulse_t) {
				cvio_log(tstext, "Out of memory");
				exit(1);
			}
		}
	}
	nsamp = got;
	err = cvio_close(irf_s);

	/* zero out the time steps */
	for(i=1;i<nsamp;i++) impulse_t[i] -= impulse_t[0];
	impulse_t[0] = 0;

	if (verbose) cvio_log(tstext,"Read %d sample impulse\n",nsamp);

	impulse_f = (float *)impulse_d;

        /* open the time file */
        err = cvio_open(timefile,CVIO_ACCESS_READ,&time_s);
        if (err) exit_cvio_error(err);
	err = cvio_max_length(time_s,&maxsamp);
        if (err) exit_cvio_error(err);

	/* create the destination file */
	if (!dontcreate) {
		err = cvio_create(dstfile,maxsamp,datatype,ndims,dims,compress);
		if (err) exit_cvio_error(err);
	}

        /* open the destination file */
        err = cvio_open(dstfile,CVIO_ACCESS_APPEND,&dst_s);
        if (err) exit_cvio_error(err);

        /* open the src (event) file */
        err = cvio_open(srcfile,CVIO_ACCESS_READ,&ev_s);
        if (err) exit_cvio_error(err);

	/* create dummy samples */
	samp_d = alloc_sample_buffer(dst_s,1);
	time_d = alloc_sample_buffer(time_s,1);
	ev_d = alloc_sample_buffer(ev_s,1);
	if ((!samp_d) || (!time_d) || (!ev_d)) {
		cvio_log(tstext,"Out of memory\n");
		exit(1);
	}

	/* prep the weighting code */
	t_weight[0] = get_weight_sample(ev_s,NULL);

	/* output a sample for each timestamp in time_s */
	c_t = 0;
	ev_t = 0;
	done = 0;

	while(!done) {
		int32_t count;
		count = 1;
		/* read the next output timestamp */
		err = cvio_read_next_samples_blocking(time_s,&time_t,time_d,&count, 0, 0);
		if (err != CVIO_ERR_OK)
			break;
		if (verbose > 1) {
			cvio_log(tstext,"Read output time:%d\n",time_t);
		}

		/* update current (output) time */
		c_t = time_t;

		/* get any pending events and add to the working list */
		/* read until event stream is past the current time (time_t) */
		while(ev_t <= time_t && !done) {
			int32_t count;
			CVIO_DT_TIME    tmp_t;

			count = 1;
//			err = cvio_read_next_samples_blocking(ev_s,&tmp_t,ev_d,&count,time_t,CVIO_TIMEOUT_STREAM);
			err = cvio_read_next_samples_blocking(ev_s,&tmp_t,ev_d,&count,0, 0);
			if (err == CVIO_ERR_EOF || count == 0)
				break;

			float	w = 1.0;
			/* new most recent impulse */
			ev_t = tmp_t;
			/* insert into the working list */
			if (use_weights) {
				w = get_weight_sample(ev_s,ev_d);
			}
			if (verbose > 1) {
				cvio_log(tstext,"Read event:%d %f\n", ev_t,w);
			}
			if (fabs(w-cent)>width) {
				for(i=0;i<max_impulses;i++) {
					if (t_list[i]==CVIO_TIME_NEXT) {
						t_list[i] = ev_t;
						t_weight[i] = w;
						break;
					}
				} 
				if (i == max_impulses) {
					cvio_log(tstext,"Warning, impulse buffer overrun. Output is corrupt.\n");
				}
			}
		}
		/* compute output for the current timestamp */
		/* compute a sample (samp_d) */
		compute_sample(time_t,t_list,samp_d,datatype,
		               nElements,nsamp,impulse_t,impulse_d,t_weight);
		/* add the sample to the output file */
		err = cvio_add_samples(dst_s,&time_t,samp_d,1);
		if (err != CVIO_ERR_OK) done = 1;
	}

	err = cvio_close(dst_s);
	err = cvio_close(ev_s);
	err = cvio_close(time_s);

	free(impulse_d);
	free(samp_d);
	free(ev_d);
	free(time_d);

	free(t_list);
	free(t_weight);

	cvio_cleanup(0);

	exit(0);
}

static float get_weight_sample(uint32_t stream,void *samp)
{
	static	int32_t	datatype,ndims,dims[3];
	int32_t	err;
	float	w = 0.0;

	if (samp) {
		switch(datatype & CVIO_TYP_SIZE_MASK) {
			case CVIO_TYP_BYTE: {
				CVIO_DT_BYTE    *a = (CVIO_DT_BYTE *)samp;
				w = ((float)a[0])/(float)(CVIO_DT_BYTE_MAX);
				}
				break;
			case CVIO_TYP_USHORT: {
				CVIO_DT_USHORT    *a = (CVIO_DT_USHORT *)samp;
				w = ((float)a[0])/(float)(CVIO_DT_USHORT_MAX);
				}
				break;
			case CVIO_TYP_SHORT: {
				CVIO_DT_SHORT    *a = (CVIO_DT_SHORT *)samp;
				w = ((float)a[0])/(float)(CVIO_DT_SHORT_MAX);
				}
				break;
			case CVIO_TYP_FLOAT: {
				CVIO_DT_FLOAT    *a = (CVIO_DT_FLOAT *)samp;
				w = a[0];
				}
				break;
		}
	} else {
        	err = cvio_datatype(stream,&datatype,&ndims,dims);
	}
	return(w);
}

static void *alloc_sample_buffer(uint32_t stream,int32_t num)
{
	int32_t	datatype,ndims,dims[3];
	int32_t	iSize;
	int32_t	err;

        err = cvio_datatype(stream,&datatype,&ndims,dims);
        if (err) return(NULL);

        iSize = (datatype & CVIO_TYP_SIZE_MASK)*dims[0]*dims[1]*dims[2]/8;

	return(calloc(num,iSize));
}

static void compute_sample(CVIO_DT_TIME t,CVIO_DT_TIME *tlist,void *samp,
	int32_t type,int32_t elements,int32_t ir_n,CVIO_DT_TIME *ir_t,char *ir_d,
	float *tweight)
{
	int32_t	j,i,iSize;

	iSize = elements*(type & CVIO_TYP_SIZE_MASK)/8;

	memset(samp,0,iSize);
	for(i=0;i<max_impulses;i++) {
		/* if this is an active event */
		if (tlist[i] != CVIO_TIME_NEXT) {
			/* if the event time + the impulse length are
			 * shorter than the current time, remove the event */
			if (tlist[i]+ir_t[ir_n-1] < t) {
				if (verbose > 1) {
					cvio_log(tstext,"Removing imp:%d\n",
						tlist[i]);
				}
				tlist[i] = CVIO_TIME_NEXT;
			} else if (tlist[i] <= t) {
				/* interpolate and sum */
				for(j=1;j<ir_n;j++) {
					if (tlist[i]+ir_t[j] >= t) break;
				}
				j = j - 1;
				compute_sum(t-tlist[i],samp,type,elements,
					ir_t+j,ir_d+(iSize*j),tweight[i]);
			}
		}
	}
	return;
}

/* interpolate a sample at (t) between ir_t[0] and ir_t[1] */
static void compute_sum(CVIO_DT_TIME t,void *samp, int32_t type,int32_t elements,
		CVIO_DT_TIME *ir_t,char *ir_d,float wei)
{
	double	w = (double)(t-ir_t[0])/(double)(ir_t[1]-ir_t[0]);
	int32_t	i;
	switch(type & CVIO_TYP_SIZE_MASK) {
		case CVIO_TYP_BYTE: {
				CVIO_DT_BYTE    *a = (CVIO_DT_BYTE *)samp;
				CVIO_DT_BYTE    *b = (CVIO_DT_BYTE *)ir_d;
				CVIO_DT_BYTE    *d = b+elements;
				CVIO_DT_BYTE	c;
				for(i=0;i<elements;i++) {
					c = (*b) + w*((*d)-(*b));
					*a += c*wei;
					a++; b++; d++;
				}
				break;
			}
		case CVIO_TYP_SHORT: {
				CVIO_DT_SHORT    *a = (CVIO_DT_SHORT *)samp;
				CVIO_DT_SHORT    *b = (CVIO_DT_SHORT *)ir_d;
				CVIO_DT_SHORT    *d = b+elements;
				CVIO_DT_SHORT	  c;
				for(i=0;i<elements;i++) {
					c = (*b) + w*((*d)-(*b));
					*a += c*wei;
					a++; b++; d++;
				}
				break;
			}
		case CVIO_TYP_USHORT: {
				CVIO_DT_USHORT    *a = (CVIO_DT_USHORT *)samp;
				CVIO_DT_USHORT    *b = (CVIO_DT_USHORT *)ir_d;
				CVIO_DT_USHORT    *d = b+elements;
				CVIO_DT_USHORT	  c;
				for(i=0;i<elements;i++) {
					c = (*b) + w*((*d)-(*b));
					*a += c*wei;
					a++; b++; d++;
				}
				break;
			}
		case CVIO_TYP_FLOAT: {
				CVIO_DT_FLOAT    *a = (CVIO_DT_FLOAT *)samp;
				CVIO_DT_FLOAT    *b = (CVIO_DT_FLOAT *)ir_d;
				CVIO_DT_FLOAT    *d = b+elements;
				CVIO_DT_FLOAT	 c;
				for(i=0;i<elements;i++) {
					c = (*b) + w*((*d)-(*b));
					*a += c*wei;
					a++; b++; d++;
				}
				break;
			}
		default:
			cvio_log(tstext,"Unsupported data type\n");
			break;
	}
}
