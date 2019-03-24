/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski
 *
 * $Id: tal_centroid.c 1213 2005-08-27 20:51:21Z rjfrank $
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

void ex_err(char *s)
{
	fprintf(stderr,"Fatal error:%s\n",s);
	tal_exit(1);
}

void cmd_err(char *s)
{
	fprintf(stderr,"(%s:%s) Usage: %s [options] inputtemp masktemp\n",__DATE__,TAL_VERSION,s);
	fprintf(stderr,"Options: -x(dx) image dx default:256\n");
	fprintf(stderr,"         -y(dy) image dy default:256\n");
	fprintf(stderr,"         -z(dz) image depth in bytes default:2\n");
	fprintf(stderr,"         -f(start) first image number default:1\n");
	fprintf(stderr,"         -l(end) last image number default:15\n");
	fprintf(stderr,"         -i(step) image number step default:1\n");
	fprintf(stderr,"         -b swap bytes with 16bit images default:no\n");
	fprintf(stderr,"         -o(offset) additive constant default:0\n");
	fprintf(stderr,"         -e(frac) frac of mean for edge default:0.75\n");
	fprintf(stderr,"         -v(value) value for edge default:use frac of mean\n");
	fprintf(stderr," output = centroid(p + offset)\n");
	fprintf(stderr,"Note:Mask must be an 8bit volume.\n");
	tal_exit(1);
}

int main(int argc,char **argv)
{
	long int	dx = 256;
	long int	dy = 256;
	long int	dz = 2;
	long int	istart = 1;
	long int	iend = 15;
	long int	istep = 1;
	long int	swab = 0;
	long int	offset = 0;
	double		edge = 0.75;
	double		sumx,sumy;
	char		intemp[256],masktemp[256];
	char		tstr[256];
	long int	i,err,j,nx,ny,k;
	long int	npixels;
	double		sum,min,max;
	double		thres = 0.0;
	unsigned char 	*data,*mdata;
	unsigned char	*bptr,*mbptr;
	unsigned short	*sptr;
	float 		*fptr;

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
				offset = atoi(&(argv[i][2]));
				break;
			case 'v':
				thres = atof(&(argv[i][2]));
				edge = -1.0;
				break;
			case 'e':
				edge = atof(&(argv[i][2]));
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
	strcpy(masktemp,argv[i+1]);
/* get the image memory */
	data = malloc(dz*dx*dy);
	if (data == 0L) ex_err("Unable to allocate image memory.");
	mdata = malloc(dx*dy);
	if (mdata == 0L) ex_err("Unable to allocate image memory.");
/* read the images */
	npixels = 0;
	sum = 0;
	min = 9999999;
	max = 0;
	for(i=istart;i<=iend;i=i+istep) {
		name_changer(masktemp,i,&err,tstr);
		fprintf(stderr,"Reading the mask file:%s\n",tstr);
		bin_io(tstr,'r',mdata,dx,dy,1,0,swab,0L);
		if (strcmp(masktemp,"-") == 0L) {
			for(j=0;j<dx*dy;j++) mdata[j] = 255;
		}
		mbptr = mdata;
		name_changer(intemp,i,&err,tstr);
		fprintf(stderr,"Reading the image file:%s\n",tstr);
		bin_io(tstr,'r',data,dx,dy,dz,0,swab,0L);
/* compute the sum of the pixels and increment the count */
		if (dz == 1) {
			bptr = (unsigned char *)data;
			for(j=0;j<(dx*dy);j++) {
			    if ((*mbptr) != 0) {
				k = (*bptr) + offset;
				if (min > (k)) min = (k);
				if (max < (k)) max = (k);
				sum += (k);
				npixels++;
			    }
			    bptr++;	
			    mbptr++;	
			}
		} else if (dz == 2) {
			sptr = (unsigned short *)data;
			for(j=0;j<(dx*dy);j++) {
			    if ((*mbptr) != 0) {
				k = (*sptr) + offset;
				if (min > (k)) min = (k);
				if (max < (k)) max = (k);
				sum += (k);
				npixels++;
			    }
			    sptr++;	
			    mbptr++;	
			}
		} else if (dz == 4) {
			fptr = (float *)data;
			for(j=0;j<(dx*dy);j++) {
			    if ((*mbptr) != 0) {
				k = (*fptr) + offset;
				if (min > (k)) min = (k);
				if (max < (k)) max = (k);
				sum += (k);
				npixels++;
			    }
			    fptr++;	
			    mbptr++;	
			}
		}
	}
	free(mdata);
	printf("Volume mean = %f, min = %.3f, max = %.3f\n",
		(sum/(double)(npixels)),min,max);
	if (edge >= 0) thres = (sum/(double)(npixels))*edge;
	printf("Operation : Centroid at level %.3f of (%s + %ld)\n",
		thres,intemp,offset);
/* initialize accumulators */
	sumx = 0.0;
	sumy = 0.0;
	nx = 0;
	ny = 0;
/* resample for output */
	for(i=istart;i<=iend;i=i+istep) {
/* read the file */
		name_changer(intemp,i,&err,tstr);
		bin_io(tstr,'r',data,dx,dy,dz,0,swab,0L);
/* apply transformation */
		if (dz == 1) {
			bptr = (unsigned char *)data;
			for(j=0;j<(dy);j++) {
				k = 0;
				while (((bptr[(j*dx)+k]+offset) < thres) 
					&& (k < (dx))) k++;
				if (k < dx) {
					sumx += k;
					nx++;
				}
				k = dx - 1;
				while (((bptr[(j*dx)+k]+offset) < thres) 
					&& (k >= (0))) k--;
				if (k >= 0) {
					sumx += k;
					nx++;
				}
			}
			for(k=0;k<(dx);k++) {
				j = 0;
				while (((bptr[(j*dx)+k]+offset) < thres) 
					&& (j < (dy))) j++;
				if (j < dy) {
					sumy += j;
					ny++;
				}
				j = dy - 1;
				while (((bptr[(j*dx)+k]+offset) < thres) 
					&& (j >= (0))) j--;
				if (j >= 0) {
					sumy += j;
					ny++;
				}
			}
		} else if (dz == 2) {
			sptr = (unsigned short *)data;
			for(j=0;j<(dy);j++) {
				k = 0;
				while (((sptr[(j*dx)+k]+offset) < thres) 
					&& (k < (dx))) k++;
				if (k < dx) {
					sumx += k;
					nx++;
				}
				k = dx - 1;
				while (((sptr[(j*dx)+k]+offset) < thres) 
					&& (k >= (0))) k--;
				if (k >= 0) {
					sumx += k;
					nx++;
				}
			}
			for(k=0;k<(dx);k++) {
				j = 0;
				while (((sptr[(j*dx)+k]+offset) < thres) 
					&& (j < (dy))) j++;
				if (j < dy) {
					sumy += j;
					ny++;
				}
				j = dy - 1;
				while (((sptr[(j*dx)+k]+offset) < thres) 
					&& (j >= (0))) j--;
				if (j >= 0) {
					sumy += j;
					ny++;
				}
			}
		} else if (dz == 4) {
			fptr = (float *)data;
			for(j=0;j<(dy);j++) {
				k = 0;
				while (((fptr[(j*dx)+k]+offset) < thres) 
					&& (k < (dx))) k++;
				if (k < dx) {
					sumx += k;
					nx++;
				}
				k = dx - 1;
				while (((fptr[(j*dx)+k]+offset) < thres) 
					&& (k >= (0))) k--;
				if (k >= 0) {
					sumx += k;
					nx++;
				}
			}
			for(k=0;k<(dx);k++) {
				j = 0;
				while (((fptr[(j*dx)+k]+offset) < thres) 
					&& (j < (dy))) j++;
				if (j < dy) {
					sumy += j;
					ny++;
				}
				j = dy - 1;
				while (((fptr[(j*dx)+k]+offset) < thres) 
					&& (j >= (0))) j--;
				if (j >= 0) {
					sumy += j;
					ny++;
				}
			}
		}
	}
	free(data);
/* output */
	if (nx == 0) nx = 1;
	if (ny == 0) ny = 1;
	sumx = sumx / (double)(nx);
	sumy = sumy / (double)(ny);
	printf("Centroid: %f,%f\n",sumx,sumy);

	tal_exit(0);
	exit(0);
}
