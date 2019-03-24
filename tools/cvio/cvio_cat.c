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

/* copy contents of shared memory segment/file to stdout */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cvio_lib.h"

#define BLOCK_SIZE	1000

void info(char *app)
{
        fprintf(stderr,"(%s) Usage: %s [options] srcfile\n",__DATE__,app);
        fprintf(stderr,"Options:\n");
        fprintf(stderr,"        -v verbose\n");

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
        int32_t 	verbose = 0;
	char	*srcfile;
	uint32_t	src_s;
	int32_t	err,iSize,iStep;
	int32_t	datatype,ndims,dims[3];
	uint32_t	nsamp;
	uint32_t	c_s;
	CVIO_DT_TIME	c_t;
	CVIO_DT_TIME	*tbuffer;
	uint8_t		*dbuffer;
	int32_t	i,j,k;

	/* parse the options */
        i = 1;
        while((i<argc) && (argv[i][0] == '-')) {
                switch(argv[i][1]) {
                        case 'v' : verbose = 1; break;
                        default: info(argv[0]); break;
                }
                i++;
	}

	if (argc-i != 1) info(argv[0]);

	/* get the filenames */
	srcfile = argv[i];

	if (cvio_init()) exit(1);

	/* open the source file */
	err = cvio_open(srcfile,CVIO_ACCESS_READ,&src_s);
	if (err) exit_cvio_error(err);

	/* get its format and size */
	err = cvio_datatype(src_s,&datatype,&ndims,dims);
	if (verbose == 1)	{
		printf("datatype: %d\n",datatype);	/* diagnostic */
		printf("ndims: %d\n",ndims);
		printf("dims: %d %d %d\n",dims[0],dims[1],dims[2]);
	}
	if (err) exit_cvio_error(err);

	err = cvio_tell(src_s,&c_s,&c_t,&nsamp);
	if (verbose == 1)	{
		printf("c_s: %d\n",c_s);
		printf("c_t: %u\n",c_t);
		printf("nsamp: %d\n",nsamp);
	}
	if (err) exit_cvio_error(err);

	/* calculate the buffer memory */
	iSize = (datatype & CVIO_TYP_SIZE_MASK)*dims[0]*dims[1]*dims[2]/8;

	/* and allocate it */
	tbuffer = (CVIO_DT_TIME *)malloc(
		BLOCK_SIZE*(iSize+sizeof(CVIO_DT_TIME)));
	dbuffer = (uint8_t*)(tbuffer + BLOCK_SIZE);
	if (!tbuffer) {
		fprintf(stderr,"Unable to allocate buffer memory.\n");
		cvio_cleanup(0);
		exit(1);
	}

	/* copy iStep samples at a time */
	iStep = BLOCK_SIZE;
	while (1) {
                int32_t rnum;

		rnum = iStep;
		err = cvio_read_next_samples_blocking(src_s,tbuffer,dbuffer,
                                 &rnum,0,0);
		if (err) {
                        if (err==CVIO_ERR_EOF) err = 0;
                	break;
   		}

		k = 0;
		for(i=0;i<rnum;i++) {
			printf("%d",tbuffer[i]);
			if (datatype == CVIO_TYP_STRING) {
				printf("\t%s",((char *)dbuffer)+k);
			} else {
			    for(j=0;j<dims[0]*dims[1]*dims[2];j++) {
				switch(datatype) {
					case CVIO_TYP_BYTE:
						printf("\t%d",
						    ((CVIO_DT_BYTE *)dbuffer)[j+k]);
						break;
					case CVIO_TYP_SHORT:
						printf("\t%d",
						    ((CVIO_DT_SHORT *)dbuffer)[j+k]);
						break;
					case CVIO_TYP_USHORT:
						printf("\t%u",
						    ((CVIO_DT_USHORT *)dbuffer)[j+k]);
						break;
					case CVIO_TYP_FLOAT:
						printf("\t%f",
						    ((CVIO_DT_FLOAT *)dbuffer)[j+k]);
						break;
					case CVIO_TYP_BOOL:
						printf("\t%d",
						    ((CVIO_DT_BYTE *)dbuffer)[j+k]);
						break;
					case CVIO_TYP_IMPULSE:
						printf("\t1");
						break;
					default:
						break;
				}
			    }
			}
			printf("\n");
			k += dims[0]*dims[1]*dims[2];
		}

		nsamp -= rnum;
	}
	
	/* free the buffer */
	free(tbuffer);

	/* report any errors */
	if (err) exit_cvio_error(err);

	/* cleanup and exit */
	err = cvio_close(src_s);

	cvio_cleanup(0);

	exit(0);
}
