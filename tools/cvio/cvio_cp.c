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

/* copy contents of shared memory segment/file to another shared memory segment/file */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include "cvio_lib.h"


/*
 * TODO:
 */

/* 1MB max copy buffer */
#define MAX_BUFFER	1000000

void info(char *app)
{
        fprintf(stderr,"(%s) Usage: %s [options] srcfile dstfile\n",__DATE__,app);
        fprintf(stderr,"Options:\n");
        fprintf(stderr,"        -v verbose\n");
        fprintf(stderr,"        -c dstfile is in compressed form\n");
        fprintf(stderr,"        -a append the srcfile to an existing dstfile\n");
        fprintf(stderr,"        -A do not copy unknown attributes\n");
        fprintf(stderr,"        -d(delay) delay between block copy in sec. -1=stdin gated (default:0)\n");
        fprintf(stderr,"        -b(blocksize) block size in bytes (default: %d)\n", MAX_BUFFER);
        fprintf(stderr,"        -m(maxsize) max number of samples for shmem (default: # src samples)\n");
	fprintf(stderr,"	-s(n) skip n samples\n");
	fprintf(stderr,"	-e(n) perform skip every nth samples\n");
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

void copy_attribs(uint32_t dst_s,uint32_t src_s)
{
	char    listnames[512],*p;
	int32_t   len,num,i;
	char    value[256];

	len = 512;
	cvio_getattriblist(src_s, listnames, &len, &num);
	
	p = listnames;
	for(i=0;i<num;i++) {
		len = 256;
		if (!strstr(p,"REQ_") && strstr(p,"_ST")) {
			if (!cvio_getattribute(src_s,p,value,&len)) {
				cvio_setattribute(dst_s,p,value);
			}
		}
		p += (strlen(p) + 1);
	}
	return;
}

int main(int argc, char *argv[])
{
	int32_t	i,iSize,iStep, nread, nwrote, slop, nleft;
	char	*dbuffer;
	CVIO_DT_TIME *tbuffer;
	int32_t		skip = 0, every = 0, skipping = 0;
        int32_t 	verbose = 0;
        int32_t 	append = 0;
        int32_t 	compress = 0;
        int32_t 	cp_attribs = 1;
        float 	del = 0.0;
        int32_t 	blocksize = MAX_BUFFER;
	char	*srcfile;
	char	*dstfile;
	uint32_t	src_s,dst_s;
	int32_t	err;
	int32_t	datatype,ndims,dims[3];
	int32_t	d_datatype,d_ndims,d_dims[3];
	uint32_t	nsamp;
	uint32_t	maxsize = 0;
	uint32_t	c_s;
	CVIO_DT_TIME	c_t;

	/* parse the options */
        i = 1;
        while((i<argc) && (argv[i][0] == '-')) {
                switch(argv[i][1]) {
                        case 'v' : verbose = 1; break;
                        case 'a' : append = 1; break;
                        case 'A' : cp_attribs = 0; break;
                        case 'c' : compress = CVIO_CREATE_GZIP; break;
                        case 'b' : 
				blocksize = atoi(argv[i]+2);
				break;
                        case 'm' : 
				maxsize = atoi(argv[i]+2);
				break;
                        case 'd' : 
				del = atof(argv[i]+2);
				break;
			case 's':
				skip = atoi(argv[i] + 2);
				break;
			case 'e':
				every = atoi(argv[i] + 2);
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

	/* open the source file */
	err = cvio_open(srcfile,CVIO_ACCESS_READ,&src_s);
	if (err) exit_cvio_error(err);

	/* get its format and size */
	err = cvio_datatype(src_s,&datatype,&ndims,dims);
	if (err) exit_cvio_error(err);

	err = cvio_tell(src_s,&c_s,&c_t,&nsamp);
	if (err) exit_cvio_error(err);

	/* create the destination file */
	if (!append) {
                uint32_t max = 0;
                if (maxsize <= 0) maxsize = nsamp;
                if (maxsize < nsamp) maxsize = nsamp;
                err=cvio_max_length(src_s,&max);
                if (err == CVIO_ERR_OK) if (max > maxsize) maxsize = max;
		err=cvio_create(dstfile,maxsize,datatype,ndims,dims,compress);
		nsamp = max;
		if (err) exit_cvio_error(err);
	}

	/* open the destination file */
	err = cvio_open(dstfile,CVIO_ACCESS_APPEND,&dst_s);
	if (err) exit_cvio_error(err);

	if (cp_attribs) copy_attribs(dst_s,src_s);

	/* get the dst format */
	err = cvio_datatype(dst_s,&d_datatype,&d_ndims,d_dims);
	if (err) exit_cvio_error(err);

	/* only allow identical types */
	if ((d_datatype != datatype) || (d_ndims != ndims) ||
	    (d_dims[0] != dims[0]) || (d_dims[1] != dims[1]) || 
	    (d_dims[2] != dims[2])) {
		fprintf(stderr,"Append restricted to files with identical sample formats.\n");
		cvio_cleanup(0);
		exit(1);
	}

	/* calculate the buffer memory (one minimally) */
	iSize = (datatype & CVIO_TYP_SIZE_MASK)*dims[0]*dims[1]*dims[2]/8;
	iStep = blocksize/iSize;
	if (iStep < 1) iStep = 1;

	/* and allocate it */
	tbuffer = malloc(iSize*iStep+iStep*sizeof(CVIO_DT_TIME));
	dbuffer = ((char *)tbuffer) + iStep*sizeof(CVIO_DT_TIME);
	if (!tbuffer) {
		fprintf(stderr,"Unable to allocate buffer memory.\n");
		cvio_cleanup(0);
		exit(1);
	}

	/* copy iStep samples at a time */
	slop = 0;
	skipping = 0;
	while (1) {
		nread = iStep;
		err = cvio_read_next_samples_blocking(src_s,tbuffer,dbuffer,
                                                      &nread,0,0);
		if (err) {
			if (err == CVIO_ERR_EOF)
				err = 0;
                    	break;
		}

		/* stdin gating.  1 char = 1 block of samples */
		if (del < 0.0) {
			char	tstr[256];
			if (!fgets(tstr,256,stdin))  break;
		}

		if (every || skip) {
			for (i = 0; i < nread; i += nwrote) {
				nleft = nread - i;
				if (skipping) {
					nwrote = skip - slop < nleft?skip - slop:nleft;
					slop += nwrote;
					if (slop == skip) {
						skipping = 0;
						slop = 0;
					}
					continue;
				} else {
					nwrote = every - slop < nleft?every - slop:nleft;
					slop += nwrote;
					if (slop == every) {
						skipping = 1;
						slop = 0;
					}
				}
				err = cvio_add_samples(dst_s,tbuffer + i,dbuffer + i*iSize,nwrote);
				if (err) break;
			}
		} else {
			err = cvio_add_samples(dst_s,tbuffer,dbuffer, nread);
			if (err) break;
		}
		if (err)
			break;
		if (del > 0.0) {
			double	t;
			if (floor(del) > 0.0) sleep(floor(del));
			t = 1000000.0*(del - floor(del));
			usleep(t);
		}
	}
	
	/* free the buffer */
	free(tbuffer);

	/* report any errors */
	if (err) exit_cvio_error(err);

	/* cleanup and exit */
	err = cvio_close(src_s);
	err = cvio_close(dst_s);

	cvio_cleanup(0);

	exit(0);
}
