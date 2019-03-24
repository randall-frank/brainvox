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

/*****cvio_mk*****/
/* creates a new stream file consisting of either shared memory or a file */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cvio_lib.h"

void info(char *app)
{
        fprintf(stderr,"(%s) Usage: %s [options] dstfile\n",__DATE__,app);
        fprintf(stderr,"Options:\n");
        fprintf(stderr,"        -v verbose\n");
        fprintf(stderr,"        -z(dz) sample type (0=string,1=byte,2=short,3=unsigned short,4=float,5=bool,6=impulse). default:1\n");
	fprintf(stderr,"	-d(d0:d1:d2) sample dimensions. default:1:1:1\n");
	fprintf(stderr,"	-l(num) maximum number of samples. default:100000\n");
	fprintf(stderr,"	-c store the file in compressed form. default:uncompressed\n");
	fprintf(stderr,"	-n(ndims) select the number of sample dims. default:1\n");
        exit(1);
}

void    exit_cvio_error(int32_t err)
{
        char    tstr[256];
        int32_t   tlen = 256;

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
	int32_t verbose = 0;
	int32_t nsamp = 100000;
	int32_t dims[3] = {1,1,1};
	int32_t ndims = 1;
	int32_t flags = 0;
	int32_t type = CVIO_TYP_BYTE;
	int32_t check,i;

        /* parse the options */
        i = 1;
        while((i<argc) && (argv[i][0] == '-')) {
                switch(argv[i][1]) {
                        case 'v' : verbose = 1; break;
			case 'c' : flags = CVIO_CREATE_GZIP; break;
			case 'z' :
				type = atoi(argv[i]+2);
                                switch(type) {
					case 0:
						type = CVIO_TYP_STRING;
						break;
					case 1:
						type = CVIO_TYP_BYTE;
						break;
					case 2:
						type = CVIO_TYP_SHORT;
						break;
					case 3:
						type = CVIO_TYP_USHORT;
						break;
					case 4:
						type = CVIO_TYP_FLOAT;
						break;
					case 5:
						type = CVIO_TYP_BOOL;
						break;
					case 6:
						type = CVIO_TYP_IMPULSE;
						break;
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
                        default: info(argv[0]); break;
                }
                i++;
        }

	if (argc-i != 1) info(argv[0]);

	if (cvio_init()) exit(1);

	check = cvio_create(argv[i], nsamp, type, ndims, dims, flags);
	if (check) exit_cvio_error(check);

	cvio_cleanup(0);

	exit(0);
}

