/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski
 *
 * $Id: tal_median.c 1213 2005-08-27 20:51:21Z rjfrank $
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
#include "stats_inc.h"

int	verbose = 0;

void ex_err(char *s)
{
	fprintf(stderr,"Fatal error:%s\n",s);
	tal_exit(1);
}

void cmd_err(char *s)
{
	fprintf(stderr,"(%s:%s) Usage: %s [options] intemp outtemp\n",__DATE__,TAL_VERSION,s);
	fprintf(stderr,"Options: -x(dx) image dx default:256\n");
	fprintf(stderr,"         -y(dy) image dy default:256\n");
	fprintf(stderr,"         -z(dz) image depth in bytes default:1\n");
	fprintf(stderr,"         -f(start) first image number default:1\n");
	fprintf(stderr,"         -l(end) last image number default:119\n");
	fprintf(stderr,"         -i(step) image number step default:1\n");
	fprintf(stderr,"         -b swap bytes with 16bit images default:no\n");
	fprintf(stderr,"         -m(masktemp) template for a mask volume\n");
	fprintf(stderr,"         -v verbose mode\n");
	fprintf(stderr,"         -r(x:y:z) median box size default:(3:3:3)\n");
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
	long int	rect[3] = {3,3,3};
	char		*masktemp=0L;

	long int	lrect[3],rrect[3];
	char		intemp[256],outtemp[256];
	char		tstr[256];
	long int	i,err,j,k,x,y;
	long int	icount,n;
	float		v;
	float		*list;
	long int	st[3],en[3],d[3];

	unsigned char	*data;
	unsigned char	*slice;
	unsigned char	*mask;

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
				get_xyz(rect,&(argv[i][2]));
				break;
			case 'm':
				masktemp = &(argv[i][2]);
				break;
			case 'b':
				swab = 1;
				break;
			case 'v':
				verbose = 1;
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
	icount = 0;
	for(i=istart;i<=iend;i=i+istep) icount++;

	data = malloc(dx*dy*icount*dz);
	if (data == 0L) ex_err("Unable to allocate image memory.");

	if (masktemp) {
		mask = malloc(dx*dy*icount);
		if (mask == 0L) ex_err("Unable to allocate image memory.");
	} else {
		mask = 0L;
	}

/* for one buffereed slice */
	slice = malloc(dz*dx*dy);
	if (slice == 0L) ex_err("Unable to allocate image memory.");

/* median list */
	i = rect[0]*rect[1]*rect[2];
	list = (float *)malloc(i*sizeof(float));
	if (list == 0L) ex_err("Unable to allocate list memory.");

/* read the images */
	j = 0;
	for(i=istart;i<=iend;i=i+istep) {
		name_changer(intemp,i,&err,tstr);
		fprintf(stderr,"Reading the image file:%s\n",tstr);
		bin_io(tstr,'r',&(data[j*dz]),dx,dy,dz,1,swab,0L);
		if (masktemp) {
			name_changer(masktemp,i,&err,tstr);
			fprintf(stderr,"Reading the mask file:%s\n",tstr);
			bin_io(tstr,'r',&(mask[j]),dx,dy,1,1,swab,0L);
		}
		j = j + (dx*dy);
	}
/* compute the median slice by slice */
	fprintf(stderr,"Computing %ldx%ldx%ld median...\n",
		rect[0],rect[1],rect[2]);
/* compute r/l halves */
	lrect[0] = rect[0]/2;
	lrect[1] = rect[1]/2;
	lrect[2] = rect[2]/2;
	rrect[0] = rect[0] - lrect[0]-1;
	rrect[1] = rect[1] - lrect[1]-1;
	rrect[2] = rect[2] - lrect[2]-1;
/* here we go... */
	j = 0;
	for(i=istart;i<=iend;i=i+istep) {
/* copy the slice first (for voxels which do not change due to masking) */
		memcpy(slice,data+(j*dx*dy*dz),dx*dy*dz);
/* for all the voxels in the slice */
		for(y=0;y<dy;y++) {
		for(x=0;x<dx;x++) {
/* collect the data */
			n = 0;
/* is it valid */
			if (masktemp) {
				k = x+(y*dx)+(j*dx*dy);
				if (mask[k] == 0) continue;
			}
/* start points */
			st[0]=x-lrect[0];
			st[1]=y-lrect[1];
			st[2]=j-lrect[2];
			if (st[0] < 0) st[0] = 0;
			if (st[1] < 0) st[1] = 0;
			if (st[2] < 0) st[2] = 0;
/* end points */
			en[0]=x+rrect[0];
			en[1]=y+rrect[1];
			en[2]=j+rrect[2];
			if (en[0] >= dx ) en[0] = dx-1;
			if (en[1] >= dy) en[1] = dy-1;
			if (en[2] >= icount) en[2] = icount-1;
/* get the rect */
			for(d[2]=st[2];d[2] <= en[2];d[2] += 1) {
			for(d[1]=st[1];d[1] <= en[1];d[1] += 1) {
			for(d[0]=st[0];d[0] <= en[0];d[0] += 1) {
				k = d[0]+(d[1]*dx)+(d[2]*dx*dy);
/* is this voxel outside the mask? */
				if (masktemp) {
					if (mask[k] == 0) continue;
				}
/* get and store */
				switch (dz) {
					case 1:
					 	v=((unsigned char *)data)[k];
						break;
					case 2:
						v=((unsigned short *)data)[k];
						break;
					case 4:
						v=((float *)data)[k];
						break;
					default:
						v = 0;
						break;
				}
				list[n++] = v;
/* next point */
			}
			}
			}
/* compute the median (if needed) */
			if (n > 0) {
				v = median(list,n);
				if (verbose) fprintf(stderr,".");
/* store it off */
				switch (dz) {
				    case 1:
					((unsigned char *)slice)[x+(dx*y)] = v;
					break;
				    case 2:
					((unsigned short *)slice)[x+(dx*y)] = v;
					break;
				    case 4:
					((float *)slice)[x+(dx*y)] = v;
					break;
				}
			}
		}
		}
/* write slice to disk */
		name_changer(outtemp,i,&err,tstr);
		fprintf(stderr,"Writing the image file:%s\n",tstr);
		bin_io(tstr,'w',slice,dx,dy,dz,1,swab,0L);
		j = j + 1;
	}

/* return the memory */	
	free(data);
	free(slice);
	free(list);
	if (mask != 0L) free(mask);

	tal_exit(0);

	exit(0);
}
