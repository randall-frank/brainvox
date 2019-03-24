/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski
 *
 * $Id: tal_cutting.c 1213 2005-08-27 20:51:21Z rjfrank $
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
#include "roi_utils.h"

void	fill_roi_(char *roiname,long int cut_out,double value, long int dx,
	long int dy,long int dz,double scale,long int flipit,
	unsigned char *data);

void ex_err(char *s)
{
	fprintf(stderr,"Fatal error:%s\n",s);
	tal_exit(1);
}

void cmd_err(char *s)
{
	fprintf(stderr,"(%s:%s) Usage: %s [options] roitemp inputtemp outputtemp\n",__DATE__,TAL_VERSION,s);
	fprintf(stderr,"Options: -x(dx) image dx default:256\n");
	fprintf(stderr,"         -y(dy) image dy default:256\n");
	fprintf(stderr,"         -z(dz) image depth in bytes default:2\n");
	fprintf(stderr,"         -f(start) first image number default:1\n");
	fprintf(stderr,"         -l(end) last image number default:15\n");
	fprintf(stderr,"         -i(step) image number step default:1\n");
	fprintf(stderr,"         -b swap bytes with 16bit images default:no\n");
	fprintf(stderr,"         -noflip do not flip ROIs over the X axis default:flip them\n");
	fprintf(stderr,"         -v(value) default:0\n");
	fprintf(stderr,"         -s(scale) ROI scaling factor default:0.5\n");
	fprintf(stderr,"         -o paint inside of roi default:paint outside of roi\n");
	fprintf(stderr," output = (pixels outside (-o=inside) ROI are set to (value))\n");
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
	double		scale = 0.5;
	double		value = 0;
	long int	cut_out = 1;
	long int	flipit = 1;
	char		roitemp[256],intemp[256],outtemp[256];
	char		tstr[256];
	long int	i,err;
	unsigned char 	*data;

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
			case 's':
				scale = atof(&(argv[i][2]));
				break;
			case 'v':
				value = atof(&(argv[i][2]));
				break;
			case 'o':
				cut_out = 0;
				break;
			case 'b':
				swab = 1;
				break;
			case 'n':    /* -noflip */
				flipit = 0;
				break;
			default:	
				cmd_err(argv[0]);
				break;
		}
		i++;
	}
	if ((argc-i) != 3) cmd_err(argv[0]);
	strcpy(roitemp,argv[i]);
	strcpy(intemp,argv[i+1]);
	strcpy(outtemp,argv[i+2]);
	if (cut_out) {
	printf("Operation: %s = (area outside %s = %.3f) else = %s\n",
		outtemp,roitemp,value,intemp);
	} else {
	printf("Operation: %s = (area inside %s = %.3f) else = %s\n",
		outtemp,roitemp,value,intemp);
	}

/* setup the roi routines */
	init_roi_utils();
	bin_roi_flip(flipit);
	bin_roi_scale(scale);

/* get the image memory */
	data = malloc(dz*dx*dy);
	if (data == 0L) ex_err("Unable to allocate image memory.");
/* read the images */
	for(i=istart;i<=iend;i=i+istep) {
/* read one */
		name_changer(intemp,i,&err,tstr);
		printf("Reading the file:%s\n",tstr);
		bin_io(tstr,'r',data,dx,dy,dz,0,swab,0L);
/* do it */
		name_changer(roitemp,i,&err,tstr);
		if (err != 0L) strcpy(tstr,roitemp);
		fill_roi_(tstr,cut_out,value,dx,dy,dz,scale,flipit,data);

/* write the output image */
		name_changer(outtemp,i,&err,tstr);
		printf("Writing sampled file:%s\n",tstr);
		bin_io(tstr,'w',data,dx,dy,dz,0,swab,0L);
	}
	free(data);
	tal_exit(0);

	exit(0);
}

#define M_PTS 11000

void	fill_roi_(char *roiname,long int cut_out,double value, long int dx,
	long int dy,long int dz,double scale,long int flipit,
	unsigned char *data)
{
	long int	strips[M_PTS],ptr;
	long int	i,x,y,st,en,j;
	unsigned char	*img;
	unsigned short	*sdata,*simg;
	float		*fdata,*fimg;
	Trace		roi;

/* get the temporary buffer */
        img = malloc(dx*dy*dz);
        if (img == 0L) return;
/* get a copy of the original image */
        for(i=0;i<(dx*dy*dz);i++) img[i] = data[i];
	simg = (unsigned short *)img;
	sdata = (unsigned short *)data;
	fimg = (float *)img;
	fdata = (float *)data;
/* flood the image with its background if needed */
        if (cut_out) {
		if (dz == 1) {
                	for(i=0;i<(dx*dy);i++) data[i] = value;
		} else if (dz == 2) {
                	for(i=0;i<(dx*dy);i++) sdata[i] = value;
		} else if (dz == 4) {
                	for(i=0;i<(dx*dy);i++) fdata[i] = value;
		}
        }
/* read ROI */
	printf("Reading the ROI:%s\n",roiname);
	if (read_roi(roiname,&roi)) {
		free(img);
		return;
	}
/* scale it */
	scale_roi(&roi,scale);
/* flip it? */
	if (flipit) {
		for(i=0;i<roi.num_points;i++) {
			roi.thepnts[i].y = (dy-1) - roi.thepnts[i].y;
		}
	}
/* fill it */
	ptr = M_PTS;
	roi_2_strips(&roi,strips,&ptr);
	if (ptr <= 0) {
	   printf("Warning:overflow encountered in polygon filling.\n");
	   free(img);
	   return;
	}
/* fill the interior of the ROI */
        for(i=0;i<ptr;i=i+3) {
                y = strips[i+2];
/* clip the strip to the current image */
                st = strips[i];
                en = strips[i+1];
                if (st < 0) st = 0;
                if (en >= dx) en = dx -1;
                if ((st <= en) && (y >= 0) && (y < dy)) {
/* fill the current strip */
                        j = st+(y*dx);  /* start of line */
                        if (cut_out) {
				if (dz == 1) {
                                	for(x=st;x<=en;x++) {
                                        	data[j] = img[j];
                                        	j++;
                                	}
				} else if (dz == 2) {
                                	for(x=st;x<=en;x++) {
                               	         	sdata[j] = simg[j];
                               	         	j++;
                                	}
				} else if (dz == 4) {
                                	for(x=st;x<=en;x++) {
                               	         	fdata[j] = fimg[j];
                               	         	j++;
                                	}
				}
                        } else {
				if (dz == 1) {
                                	for(x=st;x<=en;x++) {
                               	         	data[j] = value;
                               	         	j++;
                                	}
				} else if (dz == 2) {
                                	for(x=st;x<=en;x++) {
                               	         	sdata[j] = value;
                               	         	j++;
                                	}
				} else if (dz == 4) {
                                	for(x=st;x<=en;x++) {
                               	         	fdata[j] = value;
                               	         	j++;
                                	}
				}
                        }
                }
        }
	free(img);
	return;
}
