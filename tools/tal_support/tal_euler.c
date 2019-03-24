/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski
 *
 * $Id: tal_euler.c 1213 2005-08-27 20:51:21Z rjfrank $
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

typedef struct {
	long int	dx;
	long int	dy;
	long int	dz;
	long int	icount;
	unsigned short	*mask;
	unsigned char	*data;
	unsigned short	*sdata;
	float 		*fdata;
} c_info;

void ex_err(char *s);
void cmd_err(char *s);
long int get_local(c_info *info,long int x,long int y, long int z,
	double *val,long int *vcheck);

/* val is the value of the data at the voxel, vcheck is true if the
	pixel is inside the mask volume */
long int get_local(c_info *info,long int x,long int y, long int z,
	double *val,long int *vcheck)
{
	long int	i,j,k,l,count;
	double		*v;
	long int	*vc;

	count = 0;
	vc = vcheck;
	v = val;
	for(k=z;k<=(z+1);k++) {
	for(j=y;j<=(y+1);j++) {
	for(i=x;i<=(x+1);i++) {
		if ((i < 0) || (i >= info->dx) ||
			(j < 0) || (j >= info->dy) ||
			(k < 0) || (k >= info->icount)) {
			return(0);
		} else {
			l = i + (j*(info->dx)) + (k*(info->dx)*(info->dy));
			if (info->mask[l] != 0) {
				switch(info->dz) {
					case 1:
						*v = info->data[l];
						break;
					case 2:
						*v = info->sdata[l];
						break;
					case 4:
						*v = info->fdata[l];
						break;
				}
				*vc = 1;
			} else {
				*v = 0;
				*vc = 0;
				count++;
			}
		}
		v++;
		vc++;
	}
	}
	}
/* if all points are illegal, then nothing is returned. */
	if (count == 8) return(0);
/* valid point */
	return(1);
}

void ex_err(char *s)
{
	fprintf(stderr,"Fatal error:%s\n",s);
	tal_exit(1);
}

void cmd_err(char *s)
{
	fprintf(stderr,"(%s:%s) Usage: %s [options] inputtemp tstart tend tstep\n",__DATE__,TAL_VERSION,s);
	fprintf(stderr,"Options: -x(dx) image dx default:256\n");
	fprintf(stderr,"         -y(dy) image dy default:256\n");
	fprintf(stderr,"         -z(dz) image depth in bytes default:2\n");
	fprintf(stderr,"         -o(offset) data offset (p+offset) default:-256\n");
	fprintf(stderr,"         -f(start) first image number default:1\n");
	fprintf(stderr,"         -l(end) last image number default:119\n");
	fprintf(stderr,"         -i(step) image number step default:1\n");
	fprintf(stderr,"         -m(masktemp) template for a mask volume\n");
	fprintf(stderr,"         -b swap bytes with 16bit images default:no\n");
	fprintf(stderr,"Note: Mask volumes must be 8Bit volumes.\n");
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
	double		offset = -256.0;
	double		thres,thres2,thres3;

	char		intemp[256],*mtemp;
	char		tstr[256];
	long int	i,err,j,k;
	long int	icount;
	unsigned char	*data,*tp;
	unsigned short	*mask;

	long int 	x,y,z;
	c_info  	c_data;
	double		val[8],t;
	long int	vcheck[8];
	double		e,f,p,c;
	double 		*euler0,*euler1,*euler2,*euler3;
	long int	bval[8];

	mtemp = 0L;

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
			case 'o':
				offset = atof(&(argv[i][2]));
				break;
			case 'b':
				swab = 1;
				break;
			case 'm':
				mtemp = &(argv[i][2]);
				break;
			default:	
				cmd_err(argv[0]);
				break;
		}
		i++;
	}
	if ((argc-i) != 4) cmd_err(argv[0]);
	strcpy(intemp,argv[i+0]);
	thres = atof(argv[i+1]);
	thres2 = atof(argv[i+2]);
	thres3 = atof(argv[i+3]);

/* count the number of images */
	icount = 0;
	for(i=istart;i<=iend;i=i+istep) icount++;
/* get the image memory */
	data = malloc(dz*dx*dy*icount);
	if (data == 0L) ex_err("Unable to allocate image memory.");
/* mask volume */
	mask = (unsigned short *)malloc(dx*dy*icount*sizeof(unsigned short));
	if (mask == 0L) ex_err("Unable to allocate image memory.");
/* temp mask I/O image */
	tp = malloc(dx*dy);
	if (tp == 0L) ex_err("Unable to allocate image memory."); 

/* read the images */
	fprintf(stderr,"Reading %ld byte dataset.\n",
		(dz*dx*dy*icount));
	j = 0;
	k = 0;
	for(i=istart;i<=iend;i=i+istep) {
/* read image */
		name_changer(intemp,i,&err,tstr);
		fprintf(stderr,"Reading the file:%s\n",tstr);
		bin_io(tstr,'r',&(data[j]),dx,dy,dz,1,swab,0L);
/* read mask (if any) */
		if (mtemp == 0) {
			for(err=0;err<dx*dy;err++) mask[k+err] = 1;
		} else {
/* read the slice and transform it into a short boolean */
			name_changer(mtemp,i,&err,tstr);
			fprintf(stderr,"Reading the mask file:%s\n",tstr);
			bin_io(tstr,'r',tp,dx,dy,1,1,swab,0L);
			for(err=0;err<dx*dy;err++) {
				if (tp[err] == 0) {
					mask[k+err] = 0;
				} else {
					mask[k+err] = 1;
				}
			}
		}
		j = j + (dx*dy*dz);
		k = k + (dx*dy);
	}
/* no longer need temp mask holder */
	free(tp);

/* begin the task at hand... */
	c_data.data = data;
	c_data.sdata = (unsigned short *)data;
	c_data.fdata = (float *)data;
	c_data.mask = mask;
	c_data.dx = dx;
	c_data.dy = dy;
	c_data.dz = dz;
	c_data.icount = icount;

	j = ((thres2-thres)/thres3)+1;
	euler0 = (double *)malloc(sizeof(double)*j);
	if (euler0 == 0L) ex_err("Unable to allocate image memory."); 
	euler1 = (double *)malloc(sizeof(double)*j);
	if (euler1 == 0L) ex_err("Unable to allocate image memory."); 
	euler2 = (double *)malloc(sizeof(double)*j);
	if (euler2 == 0L) ex_err("Unable to allocate image memory."); 
	euler3 = (double *)malloc(sizeof(double)*j);
	if (euler3 == 0L) ex_err("Unable to allocate image memory."); 

	for(i=0;i<j;i++) {
		euler0[i] = 0;
		euler1[i] = 0;
		euler2[i] = 0;
		euler3[i] = 0;
	}
	fprintf(stderr,"Computing Euler characteristic from %.3f to %.3f by %.3f...\n",thres,thres2,thres3);

	for(z=0;z<icount;z++) {
	for(y=0;y<dy;y++) {
	for(x=0;x<dx;x++) {
	if (get_local(&c_data,x,y,z,val,vcheck)) {
		i = 0;
		for(t=thres;t<=thres2;t+=thres3) {
			p = 0;
			e = 0;
			f = 0;
			c = 0;
			for(j=0;j<8;j++) {
				if (vcheck[j] == 0) {
					bval[j] = 0;
				} else if (t >= 0) {
					if ((val[j]+offset) > t) {
						bval[j] = 1;
/* verticies */
						p += 1;
					} else {
						bval[j] = 0;
					}
				} else {
					if ((val[j]+offset) < t) {
						bval[j] = 1;
/* verticies */
						p += 1;
					} else {
						bval[j] = 0;
					}
				}
			}
/* X edges */
			if (bval[0] && bval[1]) e += 1;
			if (bval[2] && bval[3]) e += 1;
			if (bval[6] && bval[7]) e += 1;
			if (bval[4] && bval[5]) e += 1;
/* Y edges */
			if (bval[0] && bval[2]) e += 1;
			if (bval[1] && bval[3]) e += 1;
			if (bval[4] && bval[6]) e += 1;
			if (bval[5] && bval[7]) e += 1;
/* Z edges */
			if (bval[0] && bval[4]) e += 1;
			if (bval[2] && bval[6]) e += 1;
			if (bval[3] && bval[7]) e += 1;
			if (bval[1] && bval[5]) e += 1;
/* faces */
			if (bval[0] && bval[1] && bval[2] && bval[3]) f += 1;
			if (bval[4] && bval[5] && bval[6] && bval[7]) f += 1;
			if (bval[0] && bval[4] && bval[6] && bval[2]) f += 1;
			if (bval[1] && bval[5] && bval[7] && bval[3]) f += 1;
			if (bval[2] && bval[6] && bval[7] && bval[3]) f += 1;
			if (bval[0] && bval[4] && bval[5] && bval[1]) f += 1;
/* cube */
			if (p == 8) c = 1;

/* add it to the euler vectors */
			euler0[i] += ((p/8.0)-(e/4.0)+(f/2.0)-c);
			euler1[i] += ((e/4.0)-2.0*(f/2.0)+3.0*c);
			euler2[i] += ((f/2.0)-3.0*c);
			euler3[i] += (c);
			i++;
		}
	}
	}
	}
	}

/* report the results */
	i = 0;
	for(t=thres;t<=thres2;t+=thres3) {
		fprintf(stdout,"%f\t%f\t%f\t%f\t%f\n",t,
			euler0[i],euler1[i],euler2[i],euler3[i]);
		i++;
	}

/* done */	
	free(mask);
	free(data);
	free(euler0);
	free(euler1);
	free(euler2);
	free(euler3);
	tal_exit(0);
	exit(0);
}
