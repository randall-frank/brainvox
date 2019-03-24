/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski
 *
 * $Id: tal_optimal.c 1213 2005-08-27 20:51:21Z rjfrank $
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
#include "opt_search.h"

void ex_err(char *s);
void cmd_err(char *s);

void ex_err(char *s)
{
	fprintf(stderr,"Fatal error:%s\n",s);
	tal_exit(1);
}

void cmd_err(char *s)
{
	fprintf(stderr,"(%s:%s) Usage: %s [options] inputtemp outputimg\n",__DATE__,TAL_VERSION,s);
	fprintf(stderr,"Options: -x(dx) cost function dx default:256\n");
	fprintf(stderr,"         -y(dy) cost function dy default:256\n");
	fprintf(stderr,"         -f(start) first slice number default:1\n");
	fprintf(stderr,"         -l(end) last slice number default:20\n");
	fprintf(stderr,"         -i(step) slice number step default:1\n");
	fprintf(stderr,"         -b swap bytes with 16bit images default:no\n");
	fprintf(stderr,"         -c(x:y) connectivity rules for each dim default:(1:1)\n");
	fprintf(stderr,"         -w(axis) select wrap-around rules for one axis (0-1) default:none\n");
	fprintf(stderr,"         -v select verbosity level default:quiet\n");
	fprintf(stderr,"Output image depth is 16bits.\n");
	fprintf(stderr,"Input cost function is floating point depth.\n");
	tal_exit(1);
}

int main(int argc,char **argv)
{
	long int	dx = 256;
	long int	dy = 256;
	long int	dz = 4;
	long int	istart = 1;
	long int	iend = 20;
	long int	istep = 1;
	long int	swab = 0;
	long int	wrapa = -1;
	long int	range[3] = {1,1,1};
	long int	verbose = 0;
	long int	wrap[3] = {0,0,0};
	long int	size[3];
	
	char		intemp[256],outtemp[256];
	char		tstr[256];
	long int	i,err,k;
	long int	icount;
	float 		*data;
	long int	*depth;
	short		*sdata;

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
			case 'x':
				dx = atoi(&(argv[i][2]));
				break;
			case 'y':
				dy = atoi(&(argv[i][2]));
				break;
			case 'b':
				swab = 1;
				break;
			case 'w':
				wrapa = atoi(&(argv[i][2]));
				break;
			case 'c':
				get_xy(range,&(argv[i][2]));
				break;
			case 'v':
				verbose = strlen(argv[i])-1;
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

/* buld wrap array */
	if (wrapa == 0) {
		wrap[0] = 1; 
	} else if (wrapa == 1) {
		wrap[1] = 1;
	}

/* count the number of images */
	icount = 0;
	for(i=istart;i<=iend;i=i+istep) icount++;
/* get the image memory (bytes) */
	data = (float *)malloc(dx*dy*icount*sizeof(float));
	if (data == 0L) ex_err("Unable to allocate image memory.");
/* depth image */
	depth = malloc(dx*dy*sizeof(long int));
	if (depth == 0L) ex_err("Unable to allocate image memory."); 
	for(i=0;i<dx*dy;i++) depth[i] = 0;

/* read the volume */
	k = 0;
	for(i=istart;i<=iend;i=i+istep) {
/* read slice */
		name_changer(intemp,i,&err,tstr);
		fprintf(stderr,"Reading the file:%s\n",tstr);
		bin_io(tstr,'r',&(data[k]),dx,dy,dz,0,swab,0L);
		k = k + (dx*dy);
	}

/* do the work */
	size[0] = dx; size[1] = dy; size[2] = icount;
	k = OPT_NONE|OPT_TRUNC|OPT_NORM;
	if (verbose >= 1) k |= OPT_VERB;
	if (verbose >= 2) k |= OPT_VVERB;
	
	i = opt_search(3,size,data,depth,range,wrap,k);
	if (i & OPT_FATAL) {
		fprintf(stderr,"A fatal error occurred during search\n");
		fprintf(stderr,"Search aborted.\n");
		free(data);
		free(depth);
		tal_exit(1);
	}
	if (i & OPT_WARN_TRUNC) {
		fprintf(stderr,"Warning, the accumulator was truncated.\n");
	}
	if (i & OPT_WARN_SHIFT) {
		fprintf(stderr,"Warning, volume shifting was required.\n");
	}

/* convert to 16 bits */
	sdata = (short *)data;
	for(i=0;i<dx*dy;i++) sdata[i] = depth[i];

/* write resultant image */
	fprintf(stderr,"Writing the file:%s\n",outtemp);
	bin_io(outtemp,'w',sdata,dx,dy,2,0,0,0L);

/* done */	
	free(data);
	free(depth);

	tal_exit(0);
	
	exit(0);
}
