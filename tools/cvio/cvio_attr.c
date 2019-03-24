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

/*****cvio_attr*****/
/* allow the user to list, add and remove unknown attributes */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cvio_lib.h"

void info(char *app)
{
        fprintf(stderr,"(%s) Usage: %s file [attrib [value]]\n",
		__DATE__,app);
        fprintf(stderr,"Formats:\n");
        fprintf(stderr,"file : list the attributes and values\n");
        fprintf(stderr,"file attrib : remove the attribute\n");
        fprintf(stderr,"file attrib value : add the attribute with the value \"value\"\n");
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

void list_attribs(char *file)
{
	char	listnames[512],*p;
	int32_t	len,num,i;
	char	value[256];
	uint32_t	my_stream;

       	i = cvio_open(file, CVIO_ACCESS_READ, &my_stream);
	if (i) exit_cvio_error(i);

	len = 512;
	cvio_getattriblist(my_stream, listnames, &len, &num);

	p = listnames;
	for(i=0;i<num;i++) {
		len = 256;
		if (!strstr(p,"REQ_") && strstr(p,"_ST")) {
			if (!cvio_getattribute(my_stream,p,value,&len)) {
				printf("%s\t%s\n",p,value);
			}
		}
		p += (strlen(p) + 1);
	}

	cvio_close(my_stream);

	return;
}

int main(int32_t argc, char *argv[])
{
	uint32_t	my_stream;
	int32_t	check;
	char	*attr = NULL;
	char	*value = NULL;

	if (cvio_init()) exit(1);

	if (argc < 2) info(argv[0]);

	if (argc == 2) {
		list_attribs(argv[1]);
	} else {
		attr = argv[2];
		if (argc == 4) value = argv[3];

        	check = cvio_open(argv[1], CVIO_ACCESS_APPEND, &my_stream);
		if (check) exit_cvio_error(check);

		check = cvio_setattribute(my_stream,attr,value);
		if (check) exit_cvio_error(check);

		cvio_close(my_stream);
	}

	cvio_cleanup(0);

	exit(0);
}        


