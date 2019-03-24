/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski
 *
 * $Id: tal_remask.c 1213 2005-08-27 20:51:21Z rjfrank $
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
#include "autotrace.h"

#define MIN(a,b) (((a)<(b)?(a):(b)))
#define MAX(a,b) (((a)>(b)?(a):(b)))

/* callback functions */
void    vl_puts(char *in)
{
        fprintf(stderr,"%s\n",in);
}


void ex_err(char *s)
{
	fprintf(stderr,"Fatal error:%s\n",s);
	tal_exit(1);
}

void cmd_err(char *s)
{
	fprintf(stderr,"(%s:%s) Usage: %s [options] inputtemp outputtemp\n",__DATE__,TAL_VERSION,s);
	fprintf(stderr,"Options: -x(dx) image dx default:256\n");
	fprintf(stderr,"         -y(dy) image dy default:256\n");
	fprintf(stderr,"         -z(dz) image depth in bytes default:1\n");
	fprintf(stderr,"         -f(start) first image number default:1\n");
	fprintf(stderr,"         -l(end) last image number default:119\n");
	fprintf(stderr,"         -i(step) image number step default:1\n");
	fprintf(stderr,"         -b swap bytes with 16bit images default:no\n");
	fprintf(stderr,"         -t(threshold) above this is part of object default:1.0\n");
	fprintf(stderr," output =  8bit mask around objects above threshold\n");
	tal_exit(1);
}

int main(int argc,char **argv)
{
	long int	dx = 256;
	long int	dy = 256;
	long int	dz = 1;
	long int	istart = 1;
	long int	iend = 119;
	long int	istep = 1;
	long int	swab = 0;
	double		thres = 1.0;

	char		intemp[256],outtemp[256];
	char		tstr[256];
	long int	i,err,j,k;
	double		v;
	long int	icount;
	unsigned char	*data;
	void 		*timg;
	Dvol		volume;
	Point3D		seeds[10];

	double		n,ss,sum;

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
			case 'z':
				dz = atoi(&(argv[i][2]));
				break;
			case 't':
				thres = atof(&(argv[i][2]));
				break;
			case 'b':
				swab = 1;
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

/* get the image memory */
	timg = (void *)malloc(dz*dx*dy);
	if (timg == 0L) ex_err("Unable to allocate image memory.");

	icount = 0;
	for(i=istart;i<=iend;i=i+istep) icount++;

	data = malloc(dx*dy*icount);
	if (data == 0L) ex_err("Unable to allocate image memory.");

/* read the images */
	fprintf(stderr,"Reading %ld byte dataset for remasking\n",
		(dx*dy*icount));

	j = 0;
	for(i=istart;i<=iend;i=i+istep) {
		name_changer(intemp,i,&err,tstr);
		fprintf(stderr,"Reading the file:%s\n",tstr);
		bin_io(tstr,'r',timg,dx,dy,dz,1,swab,0L);
		sum = 0.0;
		ss = 0.0;
		n = 0.0;
		for(k=0;k<(dx*dy);k++) {
			switch (dz) {
				case 1:
					v = ((unsigned char *)timg)[k];
					break;
				case 2:
					v = ((unsigned short *)timg)[k];
					break;
				case 4:
					v = ((float *)timg)[k];
					break;
			}
			if (v > thres) {
				sum += v;
				ss += (v*v);
				n += 1;
				data[j+k] = 127;
			} else {
				data[j+k] = 0;
			}
		}
		if (n < 2) n = 2;
		sum = sum / n;
		ss = (ss - (n*sum*sum))/(n-1.0);
/*		printf("%ld\t%lf\t%lf\t%lf\n",i,n,sum,sqrt(ss)); */
		j = j + (dx*dy);
	}

/* setup the volume */
	volume.data = data;
	volume.d[0] = dx;
	volume.d[1] = dy;
	volume.d[2] = icount;
	for(i=0;i<3;i++) {
		volume.start[i] = 0;
		volume.end[i] = volume.d[i] - 1;
	}

/* setup the seed... */
	seeds[0].x = 1;
	seeds[0].y = 1;
	seeds[0].z = 1;
	seeds[0].col = 1;

/* do the work */
	fprintf(stderr,"Finding the volume mask...\n");
	volume_autotrace(&volume,0,10,seeds,1,&i);
	fprintf(stderr,"Found %ld external voxels...\n",i);

/* remap into a mask */	
	for(i=0;i<(dx*dy*icount);i++) {
		if (data[i] >= 128) {
			data[i] = 0;
		} else {
			data[i] = 255;
		}
	}

/* output */
	j = 0;
	for(i=istart;i<=iend;i=i+istep) {

		name_changer(outtemp,i,&err,tstr);
		fprintf(stderr,"Writing sampled file:%s\n",tstr);
		bin_io(tstr,'w',&(data[j]),dx,dy,1,1,swab,0L);

		j += (dx*dy);
	}

/* done */	
	free(data);
	free(timg);

	tal_exit(0);

	exit(0);
}
