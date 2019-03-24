/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski
 *
 * $Id: tal_normalize.c 1213 2005-08-27 20:51:21Z rjfrank $
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
	fprintf(stderr,"(%s:%s) Usage: %s [options] inputtemp masktemp outputtemp\n",__DATE__,TAL_VERSION,s);
	fprintf(stderr,"Options: -x(dx) image dx default:256\n");
	fprintf(stderr,"         -y(dy) image dy default:256\n");
	fprintf(stderr,"         -z(dz) image depth in bytes default:2\n");
	fprintf(stderr,"         -f(start) first image number default:1\n");
	fprintf(stderr,"         -l(end) last image number default:15\n");
	fprintf(stderr,"         -i(step) image number step default:1\n");
	fprintf(stderr,"         -b swap bytes with 16bit images default:no\n");
	fprintf(stderr,"         -n(norm) normalized average value default:50.0\n");
	fprintf(stderr,"         -c(const) additive constant default:256\n");
	fprintf(stderr,"         -o(offset) additive constant default:0\n");
	fprintf(stderr,"         -m values outside mask set to 0 default:no special treatment\n");
	fprintf(stderr," output = (p+offset)*((normal_value)/(volume_mean)) + const\n");
	fprintf(stderr,"Note: Mask volume must be 8bit.\n");
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
	double 		normal = 50;
	long int	cnst = 256;
	long int	offset = 0;
	long int	cut_out = 0;
	char		intemp[256],outtemp[256],masktemp[256];
	char		tstr[256];
	long int	i,err,j;
	long int	npixels;
	double		sum,min,max;
	double		ss,mean;
	unsigned char 	*data,*mdata;
	double		mult;
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
			case 'n':
				normal = atof(&(argv[i][2]));
				break;
			case 'c':
				cnst = atoi(&(argv[i][2]));
				break;
			case 'o':
				offset = atoi(&(argv[i][2]));
				break;
			case 'b':
				swab = 1;
				break;
			case 'm':
				cut_out = 1;
				break;
			default:	
				cmd_err(argv[0]);
				break;
		}
		i++;
	}
	if ((argc-i) != 3) cmd_err(argv[0]);
	strcpy(intemp,argv[i]);
	strcpy(masktemp,argv[i+1]);
	strcpy(outtemp,argv[i+2]);
/* get the image memory */
	data = malloc(dz*dx*dy);
	if (data == 0L) ex_err("Unable to allocate image memory.");
	mdata = malloc(dx*dy);
	if (mdata == 0L) ex_err("Unable to allocate image memory.");
/* read the images */
	npixels = 0;
	sum = 0;
	ss = 0;
	min = 9999999;
	max = 0;
	for(i=istart;i<=iend;i=i+istep) {
		name_changer(masktemp,i,&err,tstr);
		printf("Reading the mask file:%s\n",tstr);
		bin_io(tstr,'r',mdata,dx,dy,1,0,swab,0L);
		if (strcmp(masktemp,"-") == 0L) {
			for(j=0;j<dx*dy;j++) mdata[j] = 255;
		}
		mbptr = mdata;
		name_changer(intemp,i,&err,tstr);
		printf("Reading the image file:%s\n",tstr);
		bin_io(tstr,'r',data,dx,dy,dz,0,swab,0L);
/* compute the sum of the pixels and increment the count */
		if (dz == 1) {
			bptr = (unsigned char *)data;
			for(j=0;j<(dx*dy);j++) {
			    if ((*mbptr) != 0) {
				if (min > (*bptr)) min = (*bptr);
				if (max < (*bptr)) max = (*bptr);
				sum += ((*bptr)+offset);
				ss += (((*bptr)+offset)*((*bptr)+offset));
				npixels++;
			    }
			    bptr++;	
			    mbptr++;	
			}
		} else if (dz == 2) {
			sptr = (unsigned short *)data;
			for(j=0;j<(dx*dy);j++) {
			    if ((*mbptr) != 0) {
				if (min > (*sptr)) min = (*sptr);
				if (max < (*sptr)) max = (*sptr);
				sum += ((*sptr)+offset);
				ss += (((*sptr)+offset)*((*sptr)+offset));
				npixels++;
			    }
			    sptr++;	
			    mbptr++;	
			}
		} else if (dz == 4) {
			fptr = (float *)data;
			for(j=0;j<(dx*dy);j++) {
			    if ((*mbptr) != 0) {
				if (min > (*fptr)) min = (*fptr);
				if (max < (*fptr)) max = (*fptr);
				sum += ((*fptr)+offset);
				ss += (((*fptr)+offset)*((*fptr)+offset));
				npixels++;
			    }
			    fptr++;	
			    mbptr++;	
			}
		}
	}
	mult = normal/(sum/(double)(npixels));
	mean = sum/(double)(npixels);
	ss = sqrt((ss/(double)(npixels)) - (mean*mean));
	printf("Volume mean = %f, min = %.3f, max = %.3f, sd = %f\n",
		mean,min,max,ss);
	printf("Operation (normalized mean %.3f): %s=((%s + %ld) * %f) + %ld\n",
		normal,outtemp,intemp,offset,mult,cnst);
/* resample for output */
	for(i=istart;i<=iend;i=i+istep) {
/* read the file */
		name_changer(intemp,i,&err,tstr);
		bin_io(tstr,'r',data,dx,dy,dz,0,swab,0L);
/* apply transformation */
		if (dz == 1) {
			bptr = (unsigned char *)data;
			for(j=0;j<(dx*dy);j++) {
				*bptr = ((*bptr)+offset)*(mult) + cnst;
				bptr++;
			}
		} else if (dz == 2) {
			sptr = (unsigned short *)data;
			for(j=0;j<(dx*dy);j++) {
				*sptr = ((*sptr)+offset)*(mult) + cnst;
				sptr++;
			}
		} else if (dz == 4) {
			fptr = (float *)data;
			for(j=0;j<(dx*dy);j++) {
				*fptr = ((*fptr)+offset)*(mult) + cnst;
				fptr++;
			}
		}
/* remask the data if necessary */
		if (cut_out) {
			name_changer(masktemp,i,&err,tstr);
			printf("Masking,");
			bin_io(tstr,'r',mdata,dx,dy,1,0,swab,0L);
			if (strcmp(masktemp,"-") == 0L) {
				for(j=0;j<dx*dy;j++) mdata[j] = 255;
			}
			mbptr = mdata;
			if (dz == 1) {
				bptr = (unsigned char *)data;
				for(j=0;j<(dx*dy);j++) {
					if ((*mbptr) == 0) *bptr = 0 + cnst;
					bptr++;
					mbptr++;
				}
			} else if (dz == 2) {
				sptr = (unsigned short *)data;
				for(j=0;j<(dx*dy);j++) {
					if ((*mbptr) == 0) *sptr = 0 + cnst;
					sptr++;
					mbptr++;
				}
			} else if (dz == 4) {
				fptr = (float *)data;
				for(j=0;j<(dx*dy);j++) {
					if ((*mbptr) == 0) *fptr = 0 + cnst;
					fptr++;
					mbptr++;
				}
			}
		}
/* write it out */
		name_changer(outtemp,i,&err,tstr);
		printf("Writing sampled file:%s\n",tstr);
		bin_io(tstr,'w',data,dx,dy,dz,0,swab,0L);
	}
	free(mdata);
	free(data);
	tal_exit(0);
	exit(0);
}
