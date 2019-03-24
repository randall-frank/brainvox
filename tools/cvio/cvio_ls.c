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

/*****cvio_ls*****/
/* list name of shared memory streams and return total number of shared memory streams */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cvio_lib.h"

#define MAX_FILENAME	1024

void info(char *app)
{
	fprintf(stderr,"(%s) Usage: %s [options]\n",__DATE__,app);
	fprintf(stderr,"Options:\n");
	fprintf(stderr,"        -v            verbose\n");
	fprintf(stderr,"        -h<hostname>  socket host to check\n");
	exit(1);
}

int main(int argc, char *argv[])
{	
    int32_t i,j,verbose = 0;
    char *host = "localhost";

    i = 1;
    while((i<argc) && (argv[i][0] == '-')) {
        switch(argv[i][1]) {
            case 'v' : verbose = 1; break;
            case 'h' : host = &(argv[i][2]); break;
            default: info(argv[0]); break;
        }
        i++;
    }
	
    if (cvio_init()) exit(1);

    for(j=0;j<2;j++) {
        int32_t number_strings=0, size=MAX_FILENAME * 50, check;
        char names[MAX_FILENAME * 50];

        if (j == 0)  {
	    check = cvio_listshmem(names, &size, &number_strings);
	    if (verbose) printf("The names of the shared memory segments are: \n");	
        } else {
	    check = cvio_listsockets(host, names, &size, &number_strings);
	    if (verbose) printf("The names of the sockets are: \n");	
        }

	for(i=0;i<size;i++) {
		if (names[i]) {
			printf("%c",names[i]);
		} else {
			printf("\n");
		}
	}

	if (check !=0) {
	    if (verbose) {
                if (j == 0) {
	            fprintf(stderr,"Problem retrieving shared memory names.\n");
                } else {
	            fprintf(stderr,"Problem retrieving socket names.\n");
                }
	    }
	} else {
	    if (verbose) {
                if (j == 0) {
	        printf("\n");	
	        printf("The total number of shared memory streams is %d.\n", number_strings);
	        printf("\n");
                } else {
	        printf("\n");	
	        printf("The total number of socket streams is %d.\n", number_strings);
	        printf("\n");
                }
	    }
	}
    }

    cvio_cleanup(0);

    exit(0);
}


