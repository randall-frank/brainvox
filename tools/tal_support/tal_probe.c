/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski
 *
 * $Id: tal_probe.c 1213 2005-08-27 20:51:21Z rjfrank $
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

#define MIN(a,b) (((a)<(b)?(a):(b)))
#define MAX(a,b) (((a)>(b)?(a):(b)))

void ex_err(char *s)
{
	fprintf(stderr,"Fatal error:%s\n",s);
	tal_exit(1);
}

void cmd_err(char *s)
{
	fprintf(stderr,"(%s:%s) Usage: %s [options] inputtemp\n",__DATE__,TAL_VERSION,s);
	fprintf(stderr,"Options: -x(dx) image dx default:256\n");
	fprintf(stderr,"         -y(dy) image dy default:256\n");
	fprintf(stderr,"         -z(dz) image depth in bytes default:2\n");
	fprintf(stderr,"         -o(offset) data offset (p+offset) default:0\n");
	fprintf(stderr,"         -f(start) first image number default:1\n");
	fprintf(stderr,"         -l(end) last image number default:119\n");
	fprintf(stderr,"         -i(step) image number step default:1\n");
	fprintf(stderr,"         -b swap bytes with 16bit images default:no\n");
	fprintf(stderr,"         -m(masktemp) filename template for a mask volume\n");
	fprintf(stderr,"         -r(radius) radius of analysis default:3\n");
	fprintf(stderr,"         -t compute only total stats for all points\n\n");
	fprintf(stderr,"Input is tal space lines of: xxx yyy zzz score \n");
	fprintf(stderr,"Program will stop at end of input file\n");
	tal_exit(1);
}

int main(int argc,char **argv)
{
	long int	dx = 256;
	long int	dy = 256;
	long int	dz = 2;
	long int	istart = 1;
	long int	iend = 119;
	long int	istep = 1;
	long int	swab = 0;
	double		radius = 3.0;
	double		offset = 0.0;
	long int	total = 0;
	char		*masktemp = 0L;

	char		intemp[256];
	char		tstr[256];
	long int	i,err,j,k,icount,np;
	double		ss,sum;
	unsigned char	*data,*mask;
	short		*sdata;
	float		*fdata;
	double		tal[4],avt[4];
	long int	pnt[3];
	double 		d,min,max;
	long int 	r,x,y,z;

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
			case 'r':
				radius = atof(&(argv[i][2]));
				break;
			case 'o':
				offset = atof(&(argv[i][2]));
				break;
			case 'm':
				masktemp = &(argv[i][2]);
				break;
			case 'b':
				swab = 1;
				break;
			case 't':
				total = 1;
				break;
			default:	
				cmd_err(argv[0]);
				break;
		}
		i++;
	}
	if ((argc-i) != 1) cmd_err(argv[0]);
	strcpy(intemp,argv[i]);
/* get the image memory */
	icount = 0;
	for(i=istart;i<=iend;i=i+istep) icount++;

	data = malloc(dz*dx*dy*icount);
	if (data == 0L) ex_err("Unable to allocate image memory.");
	sdata = (short *)data;
	fdata = (float *)data;

	mask = malloc(dx*dy*icount);
	if (mask == 0L) ex_err("Unable to allocate mask memory.");

/* read the images */
	fprintf(stderr,"Reading %ld byte dataset for max/min at radius:%f\n",
		(dz*dx*dy*icount),radius);
	fprintf(stderr,"Using data offset of %f  (v=v+%f)\n",offset,offset);
	j = 0;
	for(i=istart;i<=iend;i=i+istep) {
		if (masktemp) {
			name_changer(masktemp,i,&err,tstr);
			fprintf(stderr,"Reading the mask:%s\n",tstr);
			bin_io(tstr,'r',&(mask[j]),dx,dy,1,1,swab,0L);
		} else {
			for(k=0;k<(dx*dy);k++) mask[j+k] = 1;
		}
		name_changer(intemp,i,&err,tstr);
		fprintf(stderr,"Reading the file:%s\n",tstr);
		bin_io(tstr,'r',&(data[j*dz]),dx,dy,dz,1,swab,0L);
		j = j + (dx*dy);
	}
/* read lines of xxx yyy zzz score */
	sum = 0.0;
	ss = 0.0;
	min = 1000000;
	max = -1000000;
	k = 0;
	np = 0;
	avt[0] = 0.0;
	avt[1] = 0.0;
	avt[2] = 0.0;
	fprintf(stderr,"Enter tal space data lines:\n");
	while(fgets(tstr,255,stdin) != 0L) {
		j = sscanf(tstr,"%lf %lf %lf",&(tal[0]),&(tal[1]),&(tal[2]));
		if (j == 3) {
			if (!total) {
				sum = 0.0;
				ss = 0.0;
				min = 1000000;
				max = -1000000;
				k = 0;
			}
/* probe radius around tal[0]-tal[3] */
/* convert to volume indicies */
			pnt[0] = 128.0 - tal[0];
			pnt[1] = tal[1] + 128.0;
			pnt[2] = tal[2] + 44.0;
			np++;
			avt[0] += tal[0];
			avt[1] += tal[1];
			avt[2] += tal[2];
/* compute values in a sphere */
			r = radius + 1;
			for(z=pnt[2]-r;z<=pnt[2]+r;z++) {
			for(y=pnt[1]-r;y<=pnt[1]+r;y++) {
			for(x=pnt[0]-r;x<=pnt[0]+r;x++) {

			if ((x>=0)&&(y>=0)&&(z>=0)) {
			if ((x<dx)&&(y<dy)&&(z<icount)) {

				d = (pnt[0] - x)*(pnt[0] - x);
				d += (pnt[1] - y)*(pnt[1] - y);
				d += (pnt[2] - z)*(pnt[2] - z);
				d = sqrt(d);
				if (d <= radius) {
					i=(x)+(y*dx)+(z*dx*dy);
					if (mask[i]) {
						if (dz == 2) {
							d = sdata[i]+offset;
						} else if (dz == 1) {
							d = data[i]+offset;
						} else if (dz == 4) {
							d = fdata[i]+offset;
						}
						if (d < min) min = d;
						if (d > max) max = d;
						sum += d;
						ss += (d*d);
						k++;
					}
				}
			}
			}

			}
			}
			}
		}
		if (!total) {
/* sum becomes mean, ss becomes sd  */
			if (k == 0) {
				sum = 0;
				ss = 0;
			} else {
				sum = sum/((double)k);
				ss = sqrt( (ss/((double)k)) - (sum*sum));
			}
			printf("%.2f %.2f %.2f mean: %.2f sd: %.2f min: %.2f max: %.2f n: %ld\n",
				tal[0],tal[1],tal[2],sum,ss,min,max,k);
		}
	}
	if (total) {
/* sum becomes mean, ss becomes sd  */
		if (k == 0) {
			sum = 0;
			ss = 0;
		} else {
			sum = sum/((double)k);
			ss = sqrt( (ss/((double)k)) - (sum*sum));
		}
		avt[0] = avt[0] / (double)(np);
		avt[1] = avt[1] / (double)(np);
		avt[2] = avt[2] / (double)(np);
		printf("%.2f %.2f %.2f mean: %.2f sd: %.2f min: %.2f max: %.2f n: %ld\n",
			avt[0],avt[1],avt[2],sum,ss,min,max,k);
	}

/* done */	
	free(data);
	free(mask);

	tal_exit(0);

	exit(0);
}
