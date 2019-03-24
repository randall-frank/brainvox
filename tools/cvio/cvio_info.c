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

/* return header information to standard output */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cvio_lib.h"

void info(char *app)
{
        fprintf(stderr,"(%s) Usage: %s file {file...}\n",__DATE__,app);
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

void list_attribs(uint32_t my_stream)
{
	char	listnames[512],*p;
	int32_t	len,num,i;
	char	value[256];

	len = 512;
	cvio_getattriblist(my_stream, listnames, &len, &num);

	p = listnames;
	for(i=0;i<num;i++) {
		len = 256;
		if (!strstr(p,"REQ_") && strstr(p,"_ST")) {
			if (!cvio_getattribute(my_stream,p,value,&len)) {
				printf("%s:%s\n",p,value);
			}
		}
		p += (strlen(p) + 1);
	}

	return;
}

int main(int32_t argc, char *argv[])
{
	uint32_t my_stream;
	int32_t i, here, there, everywhere, size1=32, size2=32, size3=48; 		int32_t base, size6=4;
	int32_t size4=32, size5=32, jumbo1=16, jumbo2=16, jumbo3=16;
	char storage[1024];
	int16_t yes, no, maybe, so=CVIO_TIMERULE_FLOOR;
	int16_t dims[3];
	float spacing[3];
	int32_t spacesize = sizeof(spacing);
	int32_t check,j;

	if (cvio_init()) exit(1);

	if (argc < 2) info(argv[0]);

	for(i=1;i<argc;i++) {

        	check = cvio_open(argv[i], CVIO_ACCESS_READ, &my_stream);
		if (check)  exit_cvio_error(check);

		printf("FILENAME: %s\n", argv[i]);

		cvio_getattribute(my_stream, "REQ_ATTR_DATE_ST", storage, &size1);
		printf("CREATION_DATE: %s\n", storage);
		cvio_getattribute(my_stream, "REQ_ATTR_NSAMP_UL", &here, &size2);
		printf("DATA_NSAMPLES: %d\n", here);
		cvio_getattribute(my_stream, "REQ_ATTR_TYPE_US", &yes, &jumbo1);
		printf("DATA_TYPE: %hd\n",yes);
		cvio_getattribute(my_stream, "REQ_ATTR_NDIM_US", &no, &jumbo2);
		printf("DATA_NDIMS: %hd\n",no);

		cvio_getattribute(my_stream, "REQ_ATTR_DIMS_US", &dims, &size3);
		printf("DATA_DIMS:");
		for(j=0;j<no;j++) printf(" %hd",dims[j]);
		printf("\n");

		cvio_getattribute(my_stream, "REQ_ATTR_COMP_US", &maybe, &jumbo3);
		printf("DATA_COMPRESSION: %s\n",maybe ? "yes" : "no");

		cvio_getattribute(my_stream, "REQ_ATTR_TIMEIN_TI", &there, &size4);	
		printf("FIRST_TIME: %d\n", there);
		cvio_getattribute(my_stream, "REQ_ATTR_TIMEOUT_TI", &everywhere, &size5);
		printf("LAST_TIME: %d\n", everywhere);

		cvio_getattribute(my_stream, "REQ_ATTR_TIMESTAMPBASE_UL", &base, &size6);
		printf("TIMESTAMPBASE: %d\n", base);

		cvio_getattribute(my_stream, "REQ_ATTR_SPACING_F", spacing, &spacesize);
		printf("SPACING: %f %f %f\n", spacing[0],spacing[1],spacing[2]);

		if (so == CVIO_TIMERULE_ROUND) {
			printf("TIME_RULE: round\n");
		}
		else if (so == CVIO_TIMERULE_FLOOR) {
			printf("TIME_RULE: floor\n");
		}
		else if (so == CVIO_TIMERULE_CEIL) {
			printf("TIME_RULE: ceil\n");
		}

		list_attribs(my_stream);
	
		printf("\n");

		cvio_close(my_stream);

	}

	cvio_cleanup(0);

	exit(0);
}        


