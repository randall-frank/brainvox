/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski
 *
 * $Id: tal_surface.c 1213 2005-08-27 20:51:21Z rjfrank $
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
	fprintf(stderr,"(%s:%s) Usage: %s [options] inputtemp\n",__DATE__,TAL_VERSION,s);
	fprintf(stderr,"Options: -x(dx) image dx default:256\n");
	fprintf(stderr,"         -y(dy) image dy default:256\n");
	fprintf(stderr,"         -z(dz) image depth in bytes default:1\n");
	fprintf(stderr,"         -f(start) first image number default:1\n");
	fprintf(stderr,"         -l(end) last image number default:124\n");
	fprintf(stderr,"         -i(step) image number step default:1\n");
	fprintf(stderr,"         -p(ipx[:ipy]) interpixel spacing default:1.0[:1.0]\n");
	fprintf(stderr,"         -s(interslice) interslice spacing default:1.0\n");
	fprintf(stderr,"         -m(masktemp) template for a mask volume\n");
	fprintf(stderr,"         -t(thres) binary threshold value default:128\n");
	fprintf(stderr,"         -b swap bytes with 16bit images default:no\n");
	fprintf(stderr,"         -r(radius) radius to smooth to before computingarea default:1 (int)\n");
	fprintf(stderr,"Note: Mask volumes must be 8Bit volumes.\n");
	tal_exit(1);
}

int main(int argc,char **argv)
{
	long int	dx = 256;
	long int	dy = 256;
	long int	dz = 1;
	long int	istart = 1;
	long int	iend = 124;
	long int	istep = 1;
	long int	swab = 0;
	double		thres = 128;
	long int	radius = 1;
	double		ipixel[2] = {1.0,1.0};
	double		islice = 1.0;

	char		intemp[256],*mtemp;
	char		tstr[256];
	long int	i,err,j;
	long int	icount,scount;
	unsigned char	*data;
	unsigned char	*tp;
	unsigned char	*mask;
	double		volume,surfacearea,v;

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
			case 'r':
				radius = atoi(&(argv[i][2]));
				break;
			case 't':
				thres = atof(&(argv[i][2]));
				break;
			case 'p':
				get_d_xy(ipixel,&(argv[i][2]));
				break;
			case 's':
				islice = atof(&(argv[i][2]));
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
	if ((argc-i) != 1) cmd_err(argv[0]);
	strcpy(intemp,argv[i]);

/* count the number of images for the kernal */
	icount = 2 + radius*2;
/* get the image memory */
	data = (unsigned char *)malloc(dx*dy*icount);
	if (data == 0L) ex_err("Unable to allocate image memory.");
/* mask volume */
	mask = (unsigned char *)malloc(dx*dy*icount);
	if (mask == 0L) ex_err("Unable to allocate image memory.");
/* temp image I/O image */
	tp = (void *)malloc(dx*dy*dz);
	if (tp == 0L) ex_err("Unable to allocate image memory."); 
/* clear things up */
	for(i=0;i<dx*dy*icount;i++) {
		mask[i] = 0;
		data[i] = 0;
	}

/* init the vars */
	volume = 0.0;
	surfacearea = 0.0;

/* read the images */
	scount = 0;
	for(i=istart;i<=(iend+(istep*(radius+1)));i=i+istep) {
/* next slice */
		scount += 1;
/* shift volumes (mask and data) to make space for the new data */
		for(j=dx*dy*(icount-1)-1;j>=0;j--) {
			data[j+(dx*dy)] = data[j];
			mask[j+(dx*dy)] = mask[j];
		}
/* check to see if we are in the padding region */
		if (i > iend) {
/* past the end of the images, read in blank data */
			for(j=0;j<dx*dy;j++) {
				data[j] = 0;
				mask[j] = 0;
			}
		} else {
/* read image files for real */
			name_changer(intemp,i,&err,tstr);
			fprintf(stderr,"Reading the file:%s\n",tstr);
			bin_io(tstr,'r',tp,dx,dy,dz,0,swab,0L);
/* convert to 0 or 255 binary */
			for(j=0;j<dx*dy;j++) {
				switch(dz) {
					case 1:
						v = ((unsigned char *)tp)[j];
						break;
					case 2:
						v = ((unsigned short *)tp)[j];
						break;
					case 4:
						v = ((float *)tp)[j];
						break;
				}
				if (v > thres) {
					data[j] = 255;
				} else 	{
					data[j] = 0;
				}
			}
/* read mask (if any) */
			if (mtemp == 0) {
				for(j=0;j<dx*dy;j++) mask[j] = 1;
			} else {
/* read the slice and transform it into a short boolean */
				name_changer(mtemp,i,&err,tstr);
				fprintf(stderr,"Reading mask file:%s\n",tstr);
				bin_io(tstr,'r',mask,dx,dy,1,0,swab,0L);
			}
		}

/* comupte the slice SA */
		if (scount >= radius) {	
			slice_isocontour(data,mask,dx,dy,icount,ipixel,islice,
				&surfacearea,&volume);
#ifdef DEBUG
		printf("SA=%f Vol=%f\n",surfacearea,volume);
#endif
		}
	}
/* report the output to the user */
	fprintf(stdout,"Surfacearea\t%f\n",surfacearea);
	fprintf(stdout,"Volume\t%f\n",volume);

/* no longer need temp mask holder */
	free(tp);

/* done */	
	tal_exit(0);

	exit(0);
}
