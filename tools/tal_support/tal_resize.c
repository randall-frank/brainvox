/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski
 *
 * $Id: tal_resize.c 1887 2006-08-20 15:35:24Z rjfrank $
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

void Sample3D_f(float *in,long int id[3],
                float *out,long int od[3],
                long int origin[3],long int size[3],long int filter);

void Sample3D_us(unsigned short *in,long int id[3],
                unsigned short *out,long int od[3],
                long int origin[3],long int size[3],long int filter);

void Sample3D_uc(unsigned char *in,long int id[3],
                unsigned char *out,long int od[3],
                long int origin[3],long int size[3],long int filter);

void pad_image(void *in,long int *idxy,void *out,long int *outdxy,
	long int pad[3][2],long int dz);

int	verbose = 0;

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
	fprintf(stderr,"         -z(dz) image depth in bytes default:2\n");
	fprintf(stderr,"         -f(start) first image number default:1\n");
	fprintf(stderr,"         -l(end) last image number default:15\n");
	fprintf(stderr,"         -i(step) image number step default:1\n");
	fprintf(stderr,"         -b swap bytes with 16bit images default:no\n");
	fprintf(stderr,"         -s Enable smoothing filter default:no\n");
	fprintf(stderr,"         -c(x:y:z) Select corner to copy from default:0:0:0\n");
	fprintf(stderr,"         -r(x:y:z) Select rect to copy default:dx:dy:slices\n");
	fprintf(stderr,"         -o(x:y:z) Select output rect default:-r values\n");
	fprintf(stderr,"         -Px(st:en) Set X axis padding before:after output rect default:0:0\n");
	fprintf(stderr,"         -Py(st:en) Set Y axis padding before:after output rect default:0:0\n");
	fprintf(stderr,"         -Pz(st:en) Set Z axis padding before:after output rect default:0:0\n");
	fprintf(stderr,"         -Pv(v) Set padding value default:0.0\n");
	fprintf(stderr,"         -v verbose mode\n");
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
	long int	filter = 0;
	long int	corner[3] = {-1,-1,-1};
	long int	rect[3] = {-1,-1,-1};
	long int	orect[3] = {-1,-1,-1};
	long int	vsize[3];
	long int	pad[3][2] = {{0,0},{0,0},{0,0}};
	double		padval = 0.0;

	long int	padsize[3];
	char		intemp[256],outtemp[256];
	char		tstr[256];
	long int	i,err,j,k;
	long int	icount;
	unsigned char	*data;
	unsigned char	*oimage;
	unsigned char	*padimage;

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
			case 'b':
				swab = 1;
				break;
			case 'v':
				verbose = 1;
				break;
			case 's':
				filter = 1;
				break;
			case 'c':
				get_xyz(corner,&(argv[i][2]));
				break;
			case 'r':
				get_xyz(rect,&(argv[i][2]));
				break;
			case 'o':
				get_xyz(orect,&(argv[i][2]));
				break;
			case 'P':
				if (strncmp("-Pv",argv[i],3) == 0) {
					padval = atof(&(argv[i][3]));
				} else if (strncmp("-Px",argv[i],3) == 0) {
					get_xy(pad[0],&(argv[i][3]));
				} else if (strncmp("-Py",argv[i],3) == 0) {
					get_xy(pad[1],&(argv[i][3]));
				} else if (strncmp("-Pz",argv[i],3) == 0) {
					get_xy(pad[2],&(argv[i][3]));
				} else {
					cmd_err(argv[0]);
				}
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

/* get the number of slices */
	icount = 0;
	for(i=istart;i<=iend;i=i+istep) icount++;

/* check:rect,corner,orect */
	if (rect[0] == -1) {
		rect[0] = dx; rect[1] = dy; rect[2] = icount;
	}
	if (corner[0] == -1) {
		corner[0] = 0; corner[1] = 0; corner[2] = 0;
	}
	if (orect[0] == -1) {
		orect[0] = rect[0]; orect[1] = rect[1]; orect[2] = rect[2];
	}
	vsize[0] = dx; vsize[1] = dy; vsize[2] = icount;

/* check validity */
	for(i=0;i<3;i++) {
		if (corner[i] < 0) ex_err("Invalid corner value");
		if (rect[i] < 0) ex_err("Invalid rect value");
		if (orect[i] < 0) ex_err("Invalid output rect value");
		if (corner[i]+rect[i] > vsize[i]) {
			ex_err("Invalid corner+rect value");
		}
                if (pad[i][0] < 0) ex_err("Invalid padding value (>0)");
                if (pad[i][1] < 0) ex_err("Invalid padding value (>0)");
	}

/* compute padded slice size */
	padsize[0] = pad[0][0] + orect[0] + pad[0][1];
	padsize[1] = pad[1][0] + orect[1] + pad[1][1];
	padsize[2] = pad[2][0] + orect[2] + pad[2][1];

/* get the volume memory */
	data = malloc(vsize[0]*vsize[1]*vsize[2]*dz);
	if (data == 0L) ex_err("Unable to allocate image memory.");

	oimage = malloc(orect[0]*orect[1]*orect[2]*dz);
	if (oimage == 0L) ex_err("Unable to allocate image memory.");

	padimage = malloc(padsize[0]*padsize[1]*dz);
	if (padimage == 0L) ex_err("Unable to allocate image memory.");

/* read the images */
	fprintf(stderr,"Scaling %ldx%ldx%ld rect at %ld,%ld,%ld in the input volume\n",
		rect[0],rect[1],rect[2],corner[0],corner[1],corner[2]);
	fprintf(stderr,"to %ldx%ldx%ld in the output volume padded by [%ld,%ld],[%ld,%ld][%ld,%ld].\n",
		orect[0],orect[1],orect[2],pad[0][0],pad[0][1],
		pad[1][0],pad[1][1],pad[2][0],pad[2][1]);
	fprintf(stderr,"Reading %ld byte dataset for resampling.\n",
		(dx*dy*icount*dz));
	j = 0;
	for(i=istart;i<=iend;i=i+istep) {
		name_changer(intemp,i,&err,tstr);
		fprintf(stderr,"Reading the file:%s\n",tstr);
		bin_io(tstr,'r',&(data[j]),dx,dy,dz,0,swab,0L);
		j = j + (dx*dy*dz);
	}

/* resample the volume */
	switch(dz) {
		case 1:
			Sample3D_uc(	(unsigned char *)data,vsize,
                			(unsigned char *)oimage,orect,
					corner,rect,filter);
			break;
		case 2:
			Sample3D_us(	(unsigned short *)data,vsize,
                			(unsigned short *)oimage,orect,
					corner,rect,filter);
			break;
		case 4:
			Sample3D_f(	(float *)data,vsize,
                			(float *)oimage,orect,
					corner,rect,filter);
			break;
	}

/* write it out slice by slice */
	k = 1;
/* clear pad slice */
	for(i=0;i<padsize[0]*padsize[1];i++) {
		switch(dz) {
			case 1:
				((unsigned char *)padimage)[i] = padval;
				break;
			case 2:
				((unsigned short *)padimage)[i] = padval;
				break;
			case 4:
				((float *)padimage)[i] = padval;
				break;
		}
	}
/* slice padding */
	for(i=0;i<pad[2][0];i++) {
		name_changer(outtemp,k,&err,tstr);
		fprintf(stderr,"Writing sampled file:%s\n",tstr);
		bin_io(tstr,'w',padimage,padsize[0],padsize[1],dz,0,swab,0L);
		k++;
	}
/* now for the real data */
	j = 0;
	for(i=0;i<orect[2];i++) {
		pad_image(&(oimage[j]),orect,padimage,padsize,pad,dz);
		name_changer(outtemp,k,&err,tstr);
		fprintf(stderr,"Writing sampled file:%s\n",tstr);
		bin_io(tstr,'w',padimage,padsize[0],padsize[1],dz,0,swab,0L);
/*
		bin_io(tstr,'w',&(oimage[j]),orect[0],orect[1],dz,0,swab,0L);
*/
		k++;
		j = j + (orect[0]*orect[1]*dz);
	}
/* clear pad slice */
	for(i=0;i<padsize[0]*padsize[1];i++) {
		switch(dz) {
			case 1:
				((unsigned char *)padimage)[i] = padval;
				break;
			case 2:
				((unsigned short *)padimage)[i] = padval;
				break;
			case 4:
				((float *)padimage)[i] = padval;
				break;
		}
	}
/* slice padding */
	for(i=0;i<pad[2][1];i++) {
		name_changer(outtemp,k,&err,tstr);
		fprintf(stderr,"Writing sampled file:%s\n",tstr);
		bin_io(tstr,'w',padimage,padsize[0],padsize[1],dz,0,swab,0L);
		k++;
	}
	
	free(data);
	free(oimage);
	free(padimage);

	tal_exit(0);

	exit(0);
}

void pad_image(void *in,long int *idxy,void *out,long int *odxy,
	long int pad[3][2],long int dz)
{
	long int	x,y,i,j,k,l;

/* over all input pixels (x,y) find output pixels (i,j) and copy data */
	for(y=0;y<idxy[1];y++) {
		x = 0;
		j = y + pad[1][0];
		i = x + pad[0][0];
		k = j*odxy[0] + i;
		l = y*idxy[0] + x;
		for(x=0;x<idxy[0];x++) {
			switch(dz) {
				case 1:
					((unsigned char *)out)[k] = 
						((unsigned char *)in)[l];
					break;
				case 2:
					((unsigned short *)out)[k] = 
						((unsigned short *)in)[l];
					break;
				case 4:
					((float *)out)[k] = 
						((float *)in)[l];
					break;
			}
			k++;
			l++;
		}
	}
	return;
}
