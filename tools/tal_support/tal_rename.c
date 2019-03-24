/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski
 *
 * $Id: tal_rename.c 1213 2005-08-27 20:51:21Z rjfrank $
 *                University of Iowa, Department of Neurology,
 *                Human Neuroanatomy and Neuroimaging Lab
 *                University of Iowa, Image Analysis Facility
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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "proto.h"

int	verbose = 0;

void ex_err(char *s)
{
	fprintf(stderr,"Fatal error:%s\n",s);
	tal_exit(1);
}

void cmd_err(char *s)
{
	fprintf(stderr,"(%s:%s) Usage: %s [options] intemp outtemp\n",__DATE__,TAL_VERSION,s);
	fprintf(stderr,"Options: -f(istart) first input file number default:1\n");
	fprintf(stderr,"         -l(iend) last input file number default:124\n");
	fprintf(stderr,"         -i(istep) input file number increment default:1\n");
	fprintf(stderr,"         -F(ostart) first output file number default:(istart)\n");
	fprintf(stderr,"         -I(ostep) output file number increment default:(istep)\n");
	fprintf(stderr,"         -v verbose mode\n");
	fprintf(stderr,"         -t test mode (print the commands only)\n");
	fprintf(stderr,"         -c(cmd) unix command to issue default:mv\n");
	tal_exit(1);
}

#define UNKNOWN	-987654

int main(int argc,char **argv)
{
	long int	istart = 1;
	long int	iend = 124;
	long int	istep = 1;
	long int	testmode = 0;
	long int	ostart = UNKNOWN;
	long int	ostep = UNKNOWN;
	char		cmd[256];

	char		intemp[256],outtemp[256];
	char		tstr[256],syscmd[1024];
	long int	i,err;

/* default command */
	strcpy(cmd,"mv");

	i = 1;
	while ((i < argc) && (argv[i][0] == '-')) {
		if (argv[i][1] == '\0') break;
		switch (argv[i][1]) {
			case 'f':
				istart = atoi(&(argv[i][2]));
				break;
			case 'l':
				iend = atoi(&(argv[i][2]));
				break;
			case 'i':
				istep = atoi(&(argv[i][2]));
				break;
			case 'F':
				ostart = atoi(&(argv[i][2]));
				break;
			case 'I':
				ostep = atoi(&(argv[i][2]));
				break;
			case 'c':
				strcpy(cmd,&(argv[i][2]));
				break;
			case 'v':
				verbose = 1;
				break;
			case 't':
				testmode = 1;
				break;
			default:	
				cmd_err(argv[0]);
				break;
		}
		i++;
	}
	if ((argc-i) != 2) cmd_err(argv[0]);
	strcpy(intemp,argv[i]);
	strcpy(outtemp,argv[i+1]);

/* if unspecified, set it */
	if (ostart == UNKNOWN) ostart = istart;
	if (ostep == UNKNOWN) ostep = istep;

/* for each file number */
	for(i=istart;i<=iend;i=i+istep) {
/* build command "{cmd} file1 file2" */
		strcpy(syscmd,cmd);
		strcat(syscmd," ");

		name_changer(intemp,i,&err,tstr);
		strcat(syscmd,tstr);
		strcat(syscmd," ");

		name_changer(outtemp,ostart,&err,tstr);
		strcat(syscmd,tstr);

		if (verbose) fprintf(stderr,"%s\n",syscmd);
		if (testmode) {
			fprintf(stdout,"%s\n",syscmd);
		} else {
			system(syscmd);
		}

		ostart += ostep;
	}

/* done */
	tal_exit(0);

	exit(0);
}
