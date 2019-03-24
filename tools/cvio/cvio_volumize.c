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

/* convert a stream of 2D slices into a stream of 3D volumes
 * optionally, with history and stepping
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include "cvio_lib.h"


void info(char *app)
{
        fprintf(stderr,"(%s) Usage: %s [options] srcfile dstfile\n",__DATE__,app);
        fprintf(stderr,"Options:\n");
        fprintf(stderr,"        -v verbose\n");
        fprintf(stderr,"        -c dstfile is in compressed form\n");
        fprintf(stderr,"        -t(timefile) output a volume of timestamps (default: no)\n");
        fprintf(stderr,"        -z(numslices) the number of slices in the volume (default: 1)\n");
        fprintf(stderr,"        -s(step) output every step volumes (default: 1)\n");
        fprintf(stderr,"        -i(a:b:c:d...) set the slice interleave (default: 0:1:2:...n)\n");
        exit(1);
}

void	exit_cvio_error(int32_t err) 
{
	char	tstr[256];
	int32_t	tlen = 256;

	cvio_get_error_message(err,tstr,&tlen);
	fprintf(stderr,"Fatal error:%s\n",tstr);

	cvio_cleanup(0);
	exit(1);
}

int main(int argc, char *argv[])
{
	int32_t 	nslices = 1;
	char	*interleave = NULL;
	char	*outname = NULL;
	int32_t	stepsize = 1;
        int32_t 	verbose = 0;
        int32_t 	compress = 0;
	char	*srcfile;
	char	*dstfile;
	int	*index = NULL;

	float	*tbuffer;
	char	*sbuffer,*in_sample;
	int32_t	i,iSize,iSizeB,iSizeS,iSizeBS,done,iIdx,iOut;
	uint32_t	src_s,dst_s,dst_t;
	int32_t	err;
	int32_t	datatype,ndims,dims[3];
	int32_t	d_datatype,d_ndims,d_dims[3];
	uint32_t	nsamp;
	CVIO_DT_TIME t,last_t;

	/* parse the options */
        i = 1;
        while((i<argc) && (argv[i][0] == '-')) {
                switch(argv[i][1]) {
                        case 'v' : verbose = 1; break;
                        case 'c' : compress = CVIO_CREATE_GZIP; break;
			case 't' : outname = argv[i]+2; break;
                        case 'i' : interleave = argv[i]+1; break;
                        case 'z' : 
				nslices = atoi(argv[i]+2);
				break;
                        case 's' : 
				stepsize = atof(argv[i]+2);
				break;
                        default: info(argv[0]); break;
                }
                i++;
	}

	if (argc-i != 2) info(argv[0]);

	/* get the filenames */
	srcfile = argv[i];
	dstfile = argv[i+1];

	if (cvio_init()) exit(1);

	/* interleave */
	if (nslices <= 0) nslices = 1;
	if (stepsize <= 0) stepsize = 1;
	index = (int *)malloc(sizeof(int)*nslices);
	if (!index) {
		fprintf(stderr,"Unable to allocate buffer memory.\n");
		cvio_cleanup(0);
		exit(1);
	}
	for(i=0;i<nslices;i++) index[i] = i;

	if (interleave) {
		for(i=0;i<nslices;i++) index[i] = -1;
		i = 0;
		while((i<nslices) && interleave) {
			index[i++] = atoi(interleave+1);
			interleave = strchr(interleave+1,':');
		}
		for(i=0;i<nslices;i++) {
			if ((index[i] < 0) || (index[i] > nslices)) {
				fprintf(stderr,"Invalid interleave list.\n");
				cvio_cleanup(0);
				exit(1);
			}
		}
	}

	/* open the source file */
	err = cvio_open(srcfile,CVIO_ACCESS_READ,&src_s);
	if (err) exit_cvio_error(err);

	/* get its format and size */
	err = cvio_datatype(src_s,&datatype,&ndims,dims);
	if (err) exit_cvio_error(err);

	if (ndims != 2) {
		err = cvio_close(src_s);
		fprintf(stderr,"Input samples must be 2D.\n");
		cvio_cleanup(0);
		exit(1);
	}


	/* open the destination file */
	err = cvio_max_length(src_s,&nsamp);
        if (err) exit_cvio_error(err);

	nsamp = (nsamp / stepsize)+1;

	d_ndims = ndims + 1;
	memcpy(d_dims,dims,sizeof(int32_t)*3);
	d_dims[2] = nslices;
	d_datatype = datatype;

	err = cvio_create(dstfile,nsamp,d_datatype,d_ndims,d_dims,compress);
	if (err) exit_cvio_error(err);

	err = cvio_open(dstfile,CVIO_ACCESS_APPEND,&dst_s);
	if (err) exit_cvio_error(err);

	if (outname) {
		err = cvio_create(outname,nsamp,CVIO_TYP_FLOAT,
				d_ndims,d_dims,compress);
		if (err) exit_cvio_error(err);

		err = cvio_open(outname,CVIO_ACCESS_APPEND,&dst_t);
		if (err) exit_cvio_error(err);
	}

	/* calculate the buffer memory */
	iSizeB = (d_datatype & CVIO_TYP_SIZE_MASK)*d_dims[0]*d_dims[1]*
		d_dims[2]/8;
	iSizeBS = iSizeB/d_dims[2];

	iSize = d_dims[0]*d_dims[1]*d_dims[2];
	iSizeS = iSize/d_dims[2];

	/* and allocate it */
	tbuffer = (float *)calloc(1,iSize*sizeof(float)+iSizeB+
			iSizeBS);
	sbuffer = ((char*)tbuffer)+iSize*sizeof(float);
	in_sample = sbuffer + iSizeBS;
	if (!tbuffer) {
		fprintf(stderr,"Unable to allocate buffer memory.\n");
		cvio_cleanup(0);
		exit(1);
	}

	for(i=0;i<iSize;i++) tbuffer[i] = 0.;

	/* operate while there are input data */
	iIdx = 0;
	iOut = 0;
	done = 0;
	last_t = CVIO_TIME_NEXT;
	while(!done) {

		/* read the next sample */
		t = CVIO_TIME_NEXT;
		err = cvio_read_samples(src_s,&t,in_sample,1);
		if (err == CVIO_ERR_OK) {
			float	delta;

			/* time does not exist before the first sample */
			if (last_t == CVIO_TIME_NEXT) last_t = t;

			/* drop the image in */
			memcpy(sbuffer+(index[iIdx]*iSizeBS),
				in_sample,iSizeBS);

			/* "age" the time values */
			delta = t - last_t;
			delta = delta/(float)(CVIO_TIME_TICKS_PER_SEC);
			for(i=0;i<iSize;i++) tbuffer[i] += delta;

			/* zero the "new" slice */
			for(i=0;i<iSizeS;i++) {
				tbuffer[index[iIdx]*iSizeS+i] = 0.0;
			}

			/* output the volume as needed */
			iOut++;
			if (iOut == stepsize) {
				iOut = 0;
				err = cvio_add_samples(dst_s,&t,sbuffer,1);
				if (err != CVIO_ERR_OK) done = 1;

				/* time volume */
				if (outname) {
				    err = cvio_add_samples(dst_t,&t,tbuffer,1);
				    if (err != CVIO_ERR_OK) done = 1;
				}
			}

			/* next slice */
			iIdx = (iIdx + 1) % nslices;
			last_t = t;
		} else {
			done = 1;
		}
	}

	/* free the buffer */
	free(index);
	free(tbuffer);

	/* report any errors */
	if (err) exit_cvio_error(err);

	/* cleanup and exit */
	err = cvio_close(src_s);
	err = cvio_close(dst_s);
	if (outname) err = cvio_close(dst_t);

	cvio_cleanup(0);

	exit(0);
}
