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

/* copy contents of text file into CVIO file */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cvio_lib.h"

void info(char *app)
{
        fprintf(stderr,"(%s) Usage: %s [options] textfile cviofile\n",__DATE__,app);
        fprintf(stderr,"Options:\n");
        fprintf(stderr,"        -v verbose\n");
        fprintf(stderr,"        -z(dz) sample depth. (1=string,2=byte,3=short,4=float,5=unsigned short) default:1\n");
	fprintf(stderr,"	-d(d0:d1:d2) sample dimensions. default:1:1:1\n");
	fprintf(stderr,"	-l(num) number of samples. default:100000\n");
	fprintf(stderr,"	-c store the file in compressed form. default:uncompressed\n");
	fprintf(stderr,"	-n(ndims) select the number of sample dims. default:1\n");
	fprintf(stderr,"	-t generate sequential timestamps\n");
	fprintf(stderr,"	-o one line per string\n");
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

void get_xyz(int32_t *xyz,char *str)
{
        char    t[256];
        char    *p;

        strcpy(t,str);

        p = t;
        sscanf(p,"%d",&(xyz[0]));

        p = strchr(p,':');
        if (p) {
                p++;
                if (*p == '\0') return;
        } else {
		return;
	}
        sscanf(p,"%d",&(xyz[1]));

        p = strchr(p,':');
        if (p) {
                p++;
                if (*p == '\0') return;
        } else {
		return;
        }
        sscanf(p,"%d",&(xyz[2]));

        return;
}

int main(int argc, char *argv[])
{

	/* declare the necessary variables */
	FILE *fp;
	char *srcfile, *dstfile;
	uint32_t dst_s;
	CVIO_DT_TIME *time_buffer;
	uint8_t *data_buffer;
	int32_t verbose = 0;
	int32_t nsamp = 100000;
	int32_t dims[3] = {1,1,1};
	int32_t ndims = 1;
	int32_t flags = 0;
	int32_t autots = 0;
	int32_t oneline = 0;
	int32_t datatype=CVIO_TYP_STRING;
	int32_t cerr, i, j, k, iSize;
	int ferr;

	/* parse the options */
        i = 1;
        while((i<argc) && (argv[i][0] == '-')) {
                switch(argv[i][1]) {
                        case 'v' : verbose = 1; break;
			case 'c' : flags = CVIO_CREATE_GZIP; break;
			case 'z' :
				if (atoi(argv[i]+2) == 1) {
					datatype = CVIO_TYP_STRING;
				} else if (atoi(argv[i]+2) == 2) {
					datatype = CVIO_TYP_BYTE;
				} else if (atoi(argv[i]+2) == 3) {
					datatype = CVIO_TYP_SHORT;
				} else if (atoi(argv[i]+2) == 4) {
					datatype = CVIO_TYP_FLOAT;
				} else if (atoi(argv[i]+2) == 5) {
					datatype = CVIO_TYP_USHORT;
				}
				break;
			case 'l' :
				nsamp = atoi(argv[i]+2);
				break;
			case 'd' :
				get_xyz(dims,argv[i]+2);
				break;
			case 'n' :
				ndims = atoi(argv[i]+2);
				break;
			case 't' :
				autots = 1;
				break;
			case 'o' :
				oneline = 1;
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

	iSize = (datatype & CVIO_TYP_SIZE_MASK)*dims[0]*dims[1]*dims[2]/8;;

	time_buffer = (CVIO_DT_TIME *)malloc(nsamp*sizeof(CVIO_DT_TIME));
	data_buffer = (uint8_t *)malloc(nsamp*iSize);
	if ((!time_buffer)  || (!data_buffer)) {
		fprintf(stderr,"Unable to allocate buffer memory.\n");
		cvio_cleanup(0);
		exit(1);
	}

	/* open the text file */
	fp = fopen(srcfile, "r");

	k=0;
	for(i=0;i<nsamp;i++) {
		if (autots)  {
			time_buffer[i] = CVIO_TIME_NEXT;
			ferr = 1;
		} else {
			ferr = fscanf(fp, "%u", &time_buffer[i]);
		}
		if ((ferr == 0) || (ferr == EOF)) {
			nsamp = i;
			break;
		}
		if (datatype == CVIO_TYP_STRING) {
			if (oneline) {
				if (fgets(((char *)data_buffer) + k, dims[0], fp))
					ferr = 1;
				else ferr = 0;
			} else
				ferr = fscanf(fp,"%s",((char *)data_buffer)+k);
		} else {
			for(j=0;j<dims[0]*dims[1]*dims[2];j++) {
                                int temp;
				switch(datatype) {
					case CVIO_TYP_BYTE:
						ferr = fscanf(fp, "%d", &temp);
                                                ((CVIO_DT_BYTE *)data_buffer)[j+k] = temp;
						break;
					case CVIO_TYP_SHORT:
						ferr = fscanf(fp, "%hd", &((CVIO_DT_SHORT *)data_buffer)[j+k]);
						break;
					case CVIO_TYP_USHORT:
						ferr = fscanf(fp, "%hu", &((CVIO_DT_USHORT *)data_buffer)[j+k]);
						break;
					case CVIO_TYP_FLOAT:
						ferr = fscanf(fp, "%f", &((CVIO_DT_FLOAT *)data_buffer)[j+k]);
						break;
					default:
						break;
				}
			}
		}
		if (autots && ((ferr == 0) || (ferr == EOF))) {
			nsamp = i;
			break;
		}
		k += dims[0]*dims[1]*dims[2];
	}

	/* create the destination file */
	cerr = cvio_create(dstfile, nsamp, datatype, ndims, dims, flags);
	if (cerr) exit_cvio_error(cerr);

	/* open the destination file */
	cerr = cvio_open(dstfile, CVIO_ACCESS_APPEND, &dst_s);
	if (cerr) exit_cvio_error(cerr);
	
	cerr = cvio_add_samples(dst_s, time_buffer, data_buffer, nsamp);

	/* free the buffer */
	free(time_buffer);
	free(data_buffer);

	/* report any errors */
	if (cerr) exit_cvio_error(cerr);

	/* cleanup and exit */
	fclose(fp);
	cerr = cvio_close(dst_s);
	if (cerr) exit_cvio_error(cerr);
	
	cvio_cleanup(0);

	exit(0);
}
