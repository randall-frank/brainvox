/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski
 *
 * $Id: tal_rad_sample.c 1213 2005-08-27 20:51:21Z rjfrank $
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

	double	ipixel[2] = {1.0,1.0};
	double	islice = 1.0;
	long int	verb = 0;

#define ODX 720     /* theta -pi to pi */
#define ODY 128     /* rad 0 to 128 */
#define ODZ 360     /* phi -pi/2 to pi/2 */

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

double interp(void *data,double x,double y,double z,long int dx,long int dy,
	long int icount,long int dz,double bval);

void ex_err(char *s)
{
	fprintf(stderr,"Fatal error:%s\n",s);
	tal_exit(1);
}

void cmd_err(char *s)
{
	fprintf(stderr,"(%s:%s) Usage: %s [options] inputtemp outtemp\n",__DATE__,TAL_VERSION,s);
	fprintf(stderr,"Options: -x(dx) image dx default:256\n");
	fprintf(stderr,"         -y(dy) image dy default:256\n");
	fprintf(stderr,"         -z(dz) image depth in bytes default:1\n");
	fprintf(stderr,"         -f(start) first image number default:1\n");
	fprintf(stderr,"         -l(end) last image number default:119\n");
	fprintf(stderr,"         -i(step) image number step default:1\n");
	fprintf(stderr,"         -p(ipx[:ipy]) interpixel spacing default:1.0[:1.0]\n");
	fprintf(stderr,"         -s(interslice) interslice spacing default:1.0\n");
	fprintf(stderr,"         -b swap bytes with 16bit images default:no\n");
	fprintf(stderr,"         -v enable verbose mode default:quiet\n");
	fprintf(stderr,"         -d(val) set the background value default:0.0\n");
	fprintf(stderr,"         -oX:Y:Z Select a new origin for sampling default:128:128:44\n");
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
	double		bval = 0;
	double		origin[3] = {128.0,128.0,44.0};
	char		intemp[256],outtemp[256],tstr[256],*ptr,*ptr2;
	long int	i,err,j,phi,rad,theta,icount,k;
	unsigned char 	*data,*odata;
	unsigned char	*bptr;
	unsigned short	*sptr;
	float 		*fptr;
	double		d;
	double	atheta,aphi,drad,x,y,z;

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
			case 'p':
				get_d_xy(ipixel,&(argv[i][2]));
				break;
			case 's':
				islice = atof(&(argv[i][2]));
				break;
			case 'd':
				bval = atof(&(argv[i][2]));
				break;
			case 'o':
				strcpy(tstr,&(argv[i][2]));
				ptr = strchr(tstr,':');
				if (ptr != 0) {
					*ptr = '\0';
					sscanf(tstr,"%lf",&(origin[0]));
					ptr2 = strchr(ptr+1,':');
					if (ptr2 != 0) {
					     *ptr2 = '\0';
					     sscanf(ptr+1,"%lf",&(origin[1]));
					     sscanf(ptr2+1,"%lf",&(origin[2]));
					}
				}
				break;
			case 'b':
				swab = 1;
				break;
			case 'v':
				verb = 1;
				break;
			default:	
				cmd_err(argv[0]);
				break;
		}
		i++;
	}
	if ((argc-i) < 2) cmd_err(argv[0]);
	strcpy(intemp,argv[i]);
	strcpy(outtemp,argv[i+1]);
	i += 2;
/* get the image memory */
	icount = 0;
	for(i=istart;i<=iend;i=i+istep) icount++;
	data = malloc(dz*dx*dy*icount);
	if (data == 0L) ex_err("Unable to allocate image memory.");
	odata = malloc(dz*ODX*ODY);
	if (odata == 0L) ex_err("Unable to allocate image memory.");
	bptr = (unsigned char *)odata;
	sptr = (unsigned short *)odata;
	fptr = (float *)odata;
/* report */
	if (verb) { 
		fprintf(stderr,"Resampling the volume %s to %s using %.3f,%.3f,%.3f as the origin\n",intemp,outtemp,origin[0],origin[1],origin[2]);
	}
/* read the images */
	k = 0;
	for(i=istart;i<=iend;i=i+istep) {
		name_changer(intemp,i,&err,tstr);
		fprintf(stderr,"Reading the image file:%s\n",tstr);
		bin_io(tstr,'r',&(data[k]),dx,dy,dz,1,swab,0);
		k = k + (dx*dy*dz);
	}
/* resample */
	for(theta=0;theta<ODZ;theta+=1.0) {
/* for phi */
		for(phi=0;phi<ODX;phi++) {
/* for rad */
		for(rad=0;rad<ODY;rad++) {
/* get the value */
/* theta -PI to PI */
			atheta = (((double)(theta)/ODZ)*2*M_PI)-M_PI;
/* phi -PI/2 to PI/2 */
			aphi = (((double)(phi)/ODX)*M_PI)-(M_PI/2);
/* rad 0 to 128 */
			drad = rad;
			z = drad*cos(atheta)*cos(aphi);
			y = drad*sin(atheta)*cos(aphi);
			x = drad*sin(aphi);
			x += origin[0];
			y += origin[1];
			z += origin[2];
/* calc z slice number */
			z = z * (islice/ipixel[0]);
			y = y * (ipixel[1]/ipixel[0]);
/* set the pixel */
			j = phi+(ODX*rad);
			d = interp(data,x,y,z,dx,dy,icount,dz,bval);
			switch(dz) {
				case 1:
					bptr[j] = d;
					break;
				case 2:
					sptr[j] = d;
					break;
				case 4:
					fptr[j] = d;
					break;
			}
		}
		}
		name_changer(outtemp,theta,&err,tstr);
		fprintf(stderr,"Writing the image file:%s\n",tstr);
		bin_io(tstr,'w',odata,ODX,ODY,dz,1,swab,0);
	}
	free(data);
	free(odata);
	tal_exit(0);

	exit(0);
}

/* interp macros */
#define MK_IDX(X,Y,Z)  ( (X) + ((dx)*(Y)) + ((dx*dy)*(Z)) )

#define GET_VAL(D,Z,I,V) if ((Z) == 1) { \
			   (V) = ((unsigned char *)(D))[(I)]; \
			 } else if ((Z) == 2) { \
			   (V) = ((unsigned short *)(D))[(I)]; \
			 } else if ((Z) == 4) { \
			   (V) = ((float *)(D))[(I)]; \
			 }

double interp(void *data,double x,double y,double z,long int dx,long int dy,
	long int dz,long int depth,double bval)
{
	double		values[8];
	long int	j;

/* get the rounded values */
	long int	fx = floor(x);
	long int	fy = floor(y);
	long int	fz = floor(z);

	long int	cx = ceil(x);
	long int	cy = ceil(y);
	long int	cz = ceil(z);

	double		fracx = x-fx;
	double		fracy = y-fy;
	double		fracz = z-fz;

	double		d = bval;

/* clip */
	if (fx < 0) return(d);
	if (fy < 0) return(d);
	if (fz < 0) return(d);

	if (cx >= dx) return(d);
	if (cy >= dy) return(d);
	if (cz >= dz) return(d);

/* get the values */
	j = MK_IDX(fx,fy,fz);
	GET_VAL(data,depth,j,values[0]);
	j = MK_IDX(cx,fy,fz);
	GET_VAL(data,depth,j,values[1]);
	j = MK_IDX(fx,cy,fz);
	GET_VAL(data,depth,j,values[2]);
	j = MK_IDX(cx,cy,fz);
	GET_VAL(data,depth,j,values[3]);
	j = MK_IDX(fx,fy,cz);
	GET_VAL(data,depth,j,values[4]);
	j = MK_IDX(cx,fy,cz);
	GET_VAL(data,depth,j,values[5]);
	j = MK_IDX(fx,cy,cz);
	GET_VAL(data,depth,j,values[6]);
	j = MK_IDX(cx,cy,cz);
	GET_VAL(data,depth,j,values[7]);

/* interpolate */
	values[0] *= (1.0-fracx)*(1.0-fracy)*(1.0-fracz);
	values[1] *= (    fracx)*(1.0-fracy)*(1.0-fracz);
	values[2] *= (1.0-fracx)*(    fracy)*(1.0-fracz);
	values[3] *= (    fracx)*(    fracy)*(1.0-fracz);
	values[4] *= (1.0-fracx)*(1.0-fracy)*(    fracz);
	values[5] *= (    fracx)*(1.0-fracy)*(    fracz);
	values[6] *= (1.0-fracx)*(    fracy)*(    fracz);
	values[7] *= (    fracx)*(    fracy)*(    fracz);

/* return */
	d = values[0]+values[1]+values[2]+values[3]+values[4]+values[5]+
		values[6]+values[7];
	return(d);
}
