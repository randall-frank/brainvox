/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski
 *
 * $Id: tal_ortho.c 1213 2005-08-27 20:51:21Z rjfrank $
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

#define DPROD(a,b) ((a)[0]*(b)[0] + (a)[1]*(b)[1]+ (a)[2]*(b)[2])

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
	fprintf(stderr,"         -z(dz) image depth in bytes default:1\n");
	fprintf(stderr,"         -f(start) first image number default:1\n");
	fprintf(stderr,"         -l(end) last image number default:100\n");
	fprintf(stderr,"         -i(step) image number step default:1\n");
	fprintf(stderr,"         -b swap bytes with 16bit images default:no\n");
	fprintf(stderr,"         -p(ipx[:ipy]) interpixel spacing default:1.0[:1.0]\n");
	fprintf(stderr,"         -s(interslice) interslice spacing default:1.0\n");
	fprintf(stderr,"         -r(interslice) output interslice spacing default:1.0\n");
	fprintf(stderr,"         -Dx(x:y:z) Select output X axis direction default:1:0:0\n");
	fprintf(stderr,"         -Dy(x:y:z) Select output Y axis direction default:0:1:0\n");
	fprintf(stderr,"         -Dz(x:y:z) Select output Z axis direction default:0:0:1\n");
	fprintf(stderr,"         -d(x:y:z) Select output volume dimensions default:256:256:100\n");
	fprintf(stderr,"         -v verbose mode\n");
	tal_exit(1);
}

int main(int argc,char **argv)
{
	long int	dx = 256;
	long int	dy = 256;
	long int	dz = 1;
	long int	istart = 1;
	long int	iend = 100;
	long int	istep = 1;
	long int	swab = 0;
	long int	Dx[3] = {1,0,0};
	long int	Dy[3] = {0,1,0};
	long int	Dz[3] = {0,0,1};
	long int	orect[3] = {256,256,100};
	double		ipixel[2] = {1.0,1.0};
	double		islice = 1.0;
	double		oslice = 1.0;

	char		intemp[256],outtemp[256];
	char		tstr[256];
	long int	i,err,j;
	long int	icount,z,y,x;
	unsigned char	*data;
	unsigned char	*oimage;
	unsigned short	*si,*so;
	float		*fi,*fo;
	long int	flip[3],ip[3],p[3];
	double		v;

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
				verbose = strlen(argv[i])-1;
				break;
			case 'r':
				oslice = atof(&(argv[i][2]));
				break;
			case 's':
				islice = atof(&(argv[i][2]));
				break;
			case 'p':
				get_d_xy(ipixel,&(argv[i][2]));
				break;
			case 'D':
				if (strncmp("-Dx",argv[i],3) == 0) {
					get_xyz(Dx,&(argv[i][3]));
				} else if (strncmp("-Dy",argv[i],3) == 0) {
					get_xyz(Dy,&(argv[i][3]));
				} else if (strncmp("-Dz",argv[i],3) == 0) {
					get_xyz(Dz,&(argv[i][3]));
				} else {
					cmd_err(argv[0]);
				}
				break;
			case 'd':
				get_xyz(orect,&(argv[i][2]));
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

/* check the output axis spec */
	if (DPROD(Dx,Dx) != 1) ex_err("X axis is not normal");
	if (DPROD(Dy,Dy) != 1) ex_err("Y axis is not normal");
	if (DPROD(Dz,Dz) != 1) ex_err("Z axis is not normal");

	if (DPROD(Dx,Dy) != 0) ex_err("X and Y axis are not orthogonal");
	if (DPROD(Dx,Dz) != 0) ex_err("X and Z axis are not orthogonal");
	if (DPROD(Dy,Dz) != 0) ex_err("Y and Z axis are not orthogonal");
	
/* find flipped axis */
	flip[0] = 0; flip[1] = 0; flip[2] = 0;
	if ((Dx[0] < 0) || (Dy[0] < 0) || (Dz[0] < 0)) flip[0] = 1;
	if ((Dx[1] < 0) || (Dy[1] < 0) || (Dz[1] < 0)) flip[1] = 1;
	if ((Dx[2] < 0) || (Dy[2] < 0) || (Dz[2] < 0)) flip[2] = 1;

	if (flip[0]) fprintf(stderr,"Flipping the original volume X axis\n");
	if (flip[1]) fprintf(stderr,"Flipping the original volume Y axis\n");
	if (flip[2]) fprintf(stderr,"Flipping the original volume Z axis\n");

/* unflip the axis specs */
	for(i=0;i<3;i++) {
		Dx[i] = abs(Dx[i]);
		Dy[i] = abs(Dy[i]);
		Dz[i] = abs(Dz[i]);
	}

/* get the volume memory */
	data = malloc(dx*dy*icount*dz);
	if (data == 0L) ex_err("Unable to allocate image memory.");
	si = (unsigned short *)data;
	fi = (float *)data;

	oimage = malloc(orect[0]*orect[1]*dz);
	if (oimage == 0L) ex_err("Unable to allocate image memory.");
	so = (unsigned short *)oimage;
	fo = (float *)oimage;

/* read the images */
	fprintf(stderr,"Reading %ld byte dataset for ortho resampling.\n",
		(dx*dy*icount*dz));
	if (flip[2]) {
		j = (icount-1)*dx*dy*dz;
	} else {
		j = 0;
	}
	for(i=istart;i<=iend;i=i+istep) {
		name_changer(intemp,i,&err,tstr);
		fprintf(stderr,"Reading the file:%s\n",tstr);
		bin_io(tstr,'r',&(data[j]),dx,dy,dz,0,swab,0L);
	
		if (flip[0]) xflip(&(data[j]),dx,dy,dz);
		if (flip[1]) yflip(&(data[j]),dx,dy,dz);

		if (flip[2]) {
			j = j - (dx*dy*dz);
		} else {
			j = j + (dx*dy*dz);
		}
	}

/* now for each output slice */
	for(z=0;z<orect[2];z++) {
		j = 0;
/* resample a slice (j=slice index, i=volume index) */
		for(y=0;y<orect[1];y++) {
		for(x=0;x<orect[0];x++) {
/* map output x,y,z to original volume */
/* coords in interpixel spacing in output volume */
			p[0] = x; 
			v = y;
			p[1] = (v*ipixel[1])/ipixel[0];
			v = z;
			p[2] = (v*oslice)/ipixel[0];
/* map to input volume */
			ip[0] = Dx[0]*p[0] + Dy[0]*p[1] + Dz[0]*p[2];
			ip[1] = Dx[1]*p[0] + Dy[1]*p[1] + Dz[1]*p[2];
			ip[2] = Dx[2]*p[0] + Dy[2]*p[1] + Dz[2]*p[2];
/* correct Y */
			v = ip[1];
			ip[1] = (v*ipixel[0])/ipixel[1];
/* correct Z */
			v = ip[2];
			ip[2] = (v*ipixel[0])/islice;
/* crop to box */
			i = ip[0] + (ip[1]*dx) + (ip[2]*dx*dy);
			if (ip[0] < 0) i = -1;
			if (ip[1] < 0) i = -1;
			if (ip[2] < 0) i = -1;
			if (ip[0] >= dx) i = -1;
			if (ip[1] >= dy) i = -1;
			if (ip[2] >= icount) i = -1;
/* copy the pixel (if any) */
			switch(dz) {
				case 1:
					if (i > 0) {
						oimage[j] = data[i];
					} else {
						oimage[j] = 0;
					}
					break;
				case 2:
					if (i > 0) {
						so[j] = si[i];
					} else {
						so[j] = 0;
					}
					break;
				case 4:
					if (i > 0) {
						fo[j] = fi[i];
					} else {
						fo[j] = 0;
					}
					break;
			}
			j++;
		}
		}
/* write the slice */
		name_changer(outtemp,z+1,&err,tstr);
		fprintf(stderr,"Writing sampled file:%s\n",tstr);
		bin_io(tstr,'w',oimage,orect[0],orect[1],dz,0,swab,0L);
	}
	
	free(data);
	free(oimage);

	tal_exit(0);

	exit(0);
}
