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

/*****cvio_sample*****/
/* interpolate values from one cvio file into another */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cvio_lib.h"


/*
 * TODO:
 */

void info(char *app)
{
        fprintf(stderr,"(%s) Usage: %s [options] srcfile dstfile\n",__DATE__,app);
        fprintf(stderr,"Options:\n");
        fprintf(stderr,"        -v verbose\n");
        fprintf(stderr,"        -c dstfile is in compressed form\n");
        fprintf(stderr,"        -A do not copy unknown attributes\n");
        fprintf(stderr,"        -t(timerule) 0=floor, 1=ceil, 2=round, 3=interp (default:round)\n");
	fprintf(stderr,"stdin = timesteps to sample at\n");
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
	int32_t	i,iSize;
	void	*tbuffer;
        int32_t 	verbose = 0;
        int32_t 	compress = 0;
        int32_t 	cp_attribs = 1;
	char	*srcfile;
	char	*dstfile;
	uint32_t	src_s,dst_s;
	int32_t	err;
	int32_t	datatype,ndims,dims[3];
	uint32_t	nsamp;
	uint32_t	c_s;
	CVIO_DT_TIME	c_t;
	CVIO_DT_USHORT	timerule = CVIO_TIMERULE_ROUND;
	char	tstr[256];

	/* parse the options */
        i = 1;
        while((i<argc) && (argv[i][0] == '-')) {
                switch(argv[i][1]) {
                        case 'v' : verbose = 1; break;
                        case 't' : timerule = atoi(argv[i]+2);
                        case 'A' : cp_attribs = 0; break;
                        case 'c' : compress = CVIO_CREATE_GZIP; break;
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
	
	/* set the tilerule */
        err = cvio_setattribute(src_s,"REQ_ATTR_TIMERULE_US",&timerule);
	if (err) exit_cvio_error(err);

	/* get its format and size */
	err = cvio_datatype(src_s,&datatype,&ndims,dims);
	if (err) exit_cvio_error(err);

	err = cvio_tell(src_s,&c_s,&c_t,&nsamp);
	if (err) exit_cvio_error(err);

	/* create the destination file */
	err = cvio_create(dstfile,nsamp,datatype,ndims,dims,compress);
	if (err) exit_cvio_error(err);

	/* open the destination file */
	err = cvio_open(dstfile,CVIO_ACCESS_APPEND,&dst_s);
	if (err) exit_cvio_error(err);

	if (cp_attribs) copy_attribs(dst_s,src_s);

	/* calculate the buffer memory */
	iSize = (datatype & CVIO_TYP_SIZE_MASK)*dims[0]*dims[1]*dims[2]/8;

	/* and allocate it */
	tbuffer = (void *)malloc(iSize);
	if (!tbuffer) {
		fprintf(stderr,"Unable to allocate buffer memory.\n");
		cvio_cleanup(0);
		exit(1);
	}

	/* copy iStep samples at a time */
	while(fgets(tstr,256,stdin)) {

		sscanf(tstr,"%d",&c_t);

		err = cvio_read_samples(src_s,&c_t,tbuffer,1);
		if (err) break;

		err = cvio_add_samples(dst_s,&c_t,tbuffer,1);
		if (err) break;
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
