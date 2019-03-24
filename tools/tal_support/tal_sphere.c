/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski
 *
 * $Id: tal_sphere.c 1213 2005-08-27 20:51:21Z rjfrank $
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
#include "stats_inc.h"
#include "proto.h"

#define MIN(a,b) (((a)<(b)?(a):(b)))
#define MAX(a,b) (((a)>(b)?(a):(b)))

#undef DEBUG 

double	smoothness(double n,double p,double s);
double	fsmooth(double t,double s);
void feval(long int argc,char **argv);
void ex_err(char *s);
void cmd_err(char *s);

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
	fprintf(stderr,"         -f(start) first image number default:1\n");
	fprintf(stderr,"         -l(end) last image number default:119\n");
	fprintf(stderr,"         -i(step) image number step default:1\n");
	fprintf(stderr,"         -m(masktemp) template for a mask volume\n");
	fprintf(stderr,"         -b swap bytes with 16bit images default:no\n");
	fprintf(stderr,"         -p average planar sphericity default:3D sphericity\n");
	fprintf(stderr,"Note: Mask volumes must be 8Bit volumes.\n");
	fprintf(stderr,"Differences from SPM ks31.m computation:\n");
	fprintf(stderr,"1) Volumetric basis or averaged slice based.\n");
	fprintf(stderr,"2) Zero valued derivative values allowed.\n");
	fprintf(stderr,"3) Uses one less change value per 3D scanline than ks31.m.\n");
	tal_exit(1);
}

void feval(long int argc,char **argv)
{
	double	n,p,s,t;
	double	st,en,inc;

	if ((argc != 4) && (argc != 5)) {
	fprintf(stderr,"(%s) Usage: %s S p n\n",__DATE__,argv[0]);
	fprintf(stderr,"or: %s S tstart tend tstep\n",argv[0]);
	fprintf(stderr,"This program computes the threshold value t for a process\n");
	fprintf(stderr,"with sphericity S and size n such that the probability of\n");
	fprintf(stderr,"a false positive is p.\n");
	fprintf(stderr,"Assumptions:\n");
	fprintf(stderr,"Pixel size is small with respect to S.\n");
	fprintf(stderr,"S is small with respect to the size of the process.\n");
	tal_exit(1);
	}
	if (argc == 5) {
		s = atof(argv[1]);
		st = atof(argv[2]);
		en = atof(argv[3]);
		inc = atof(argv[4]);
		if (inc <= 0) inc = 1;
		if (en < st) en = st;
		for(t=st;t<=en;t+=inc) {
			fprintf(stdout,"%f\t%f\n",t,fsmooth(t,s));
		}
	} else {
		n = atof(argv[3]);
		p = atof(argv[2]);
		s = atof(argv[1]);
		t = smoothness(n,p,s);

		fprintf(stdout,"%.5f\tThreshold for:%.3f\t%.3f\t%.3f\n",
			t,s,p,n);
	}
	tal_exit(0);
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
	long int	planar = 0;

	char		intemp[256],*mtemp;
	char		tstr[256];
	long int	i,err,j,k,ix,iy,iz;
	long int	icount;
	unsigned char	*data,*tp;
	unsigned char	*mask;

	double	n,s,ss,d;
	double	gstdev;
	double	sx,sy,sz,ssx,ssy,ssz,np;
	double	vx,vy,vz,ps,nps;

	long int 	x,y,z;

	mtemp = 0L;

	if (strstr(argv[0],"tal_spherep") != 0) feval(argc,argv);

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
				planar = 1;
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
	strcpy(intemp,argv[i+0]);

/* count the number of images */
	icount = 0;
	for(i=istart;i<=iend;i=i+istep) icount++;
/* get the image memory */
	data = malloc(dz*dx*dy*icount);
	if (data == 0L) ex_err("Unable to allocate image memory.");
/* mask volume */
	mask = (unsigned char *)malloc(dx*dy*icount);
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

	ps = 0;
	nps = 0;
/* compute global stddev */
	if (planar == 0) {
		s = 0;
		ss = 0;
		n = 0;
		for(z=0;z<icount;z++) {
		for(y=0;y<dy;y++) {
		for(x=0;x<dx;x++) {
			i = x + (y*dx) + (z*(dx*dy));
			if (mask[i]) {
				switch(dz) {
					case 1:
						d = ((unsigned char *)data)[i];
						break;
					case 2:
						d = ((unsigned short *)data)[i];
						break;
					case 4:
						d = ((float *)data)[i];
						break;
					default:
						d = 0;
						break;
				}
				ss = ss + (d*d);
				s = s + d;
				n = n + 1;
			}
		}
		}
		}
/* if no pixels can be found */
		if (n < 2) {
			fprintf(stderr,"No valid voxels detected\n");
			fprintf(stdout,"%.4f\tError.\n",0.0);
			goto errout;
		}
		s = s / n;
		gstdev = sqrt((ss - n*(s*s))/(n-1));
/* init the vars */
		np = 0;
		sx = 0; ssx = 0;
		sy = 0; ssy = 0;
		sz = 0; ssz = 0;
	}
/* begin (If planar, then all slices.  If non-planar, skip Z slice 0.)*/
	for(z=(1-planar);z<icount;z++) {
/* compute slice stdev in planar case */
		if (planar == 1) {
			s = 0;
			ss = 0;
			n = 0;
			for(y=0;y<dy;y++) {
			for(x=0;x<dx;x++) {
				i = x + (y*dx) + (z*(dx*dy));
				if (mask[i]) {
					switch(dz) {
						case 1:
						d = ((unsigned char *)data)[i];
							break;
						case 2:
						d = ((unsigned short *)data)[i];
							break;
						case 4:
						d = ((float *)data)[i];
							break;
						default:
						d = 0;
							break;
					}
					ss = ss + (d*d);
					s = s + d;
					n = n + 1;
				}
			}
			}
#ifdef DEBUG
	printf("Planar: S = %f SS = %f n = %f\n",s,ss,n);
#endif
			if (n > 1) {
				s = s / n;
				gstdev = sqrt((ss - n*(s*s))/(n-1));
			}
#ifdef DEBUG
	printf("Planar: gstdev = %f\n",gstdev);
#endif
/* and re-init the vars */
			np = 0;
			sx = 0; ssx = 0;
			sy = 0; ssy = 0;
			sz = 0; ssz = 0;
		}
/* compute and add gradients for the plane/or volume */
	for(y=1;y<dy;y++) {
	for(x=1;x<dx;x++) {
/* compute indicies */
		i = x + (y*dx) + (z*(dx*dy));
		ix = i - 1;
		iy = i - dx;
		iz = i - (dx*dy);
		if (planar == 1) iz = i; /* no z if planar */
/* if all pixels valid */
		if (mask[i] && mask[ix] && mask[iy] && mask[iz]) {
			switch(dz) {
				case 1:
					d = ((unsigned char *)data)[i];
					vx = ((unsigned char *)data)[ix];
					vy = ((unsigned char *)data)[iy];
					vz = ((unsigned char *)data)[iz];
					break;
				case 2:
					d = ((unsigned short *)data)[i];
					vx = ((unsigned short *)data)[ix];
					vy = ((unsigned short *)data)[iy];
					vz = ((unsigned short *)data)[iz];
					break;
				case 4:
					d = ((float *)data)[i];
					vx = ((float *)data)[ix];
					vy = ((float *)data)[iy];
					vz = ((float *)data)[iz];
					break;
			}
/* compute gradients */
			vx = (d - vx) / gstdev;
			vy = (d - vy) / gstdev;
			vz = (d - vz) / gstdev;
/* compute sum and ss */
			sx = sx + vx;
			ssx = ssx + (vx*vx);
			sy = sy + vy;
			ssy = ssy + (vy*vy);
			sz = sz + vz;
			ssz = ssz + (vz*vz);
			np = np + 1;
		}
	} /* xpixel */
	} /* ypixel */
/* in the planar case, compute an S for the plane */
		if ((planar == 1) && (np > 1)) {
/* compute stdev for each vector */
			sx = sx / np;
			sy = sy / np;
			ssx = sqrt((ssx - np*(sx*sx))/(np-1));
			ssy = sqrt((ssy - np*(sy*sy))/(np-1));
#ifdef DEBUG
	printf("Planar: sdx = %f sdy = %f n = %f\n",ssx,ssy,np);
#endif
/* compute 2D sphereicity */
			s = sqrt(1.0/(2*((ssx*ssx + ssy*ssy)/2.0)));
#ifdef DEBUG
	printf("Planar: S = %f\n",s);
#endif
/*
			s = s*sqrt(8.0*log(2.0)); 
			sqrt(8*ln(2)) = 2.354820045
*/
			s = s*2.354820045;
/* sum planar S values for average */
			ps += s;
			nps += 1.0;
		}
	} /* next plane */

/* report the results */
	if ((planar == 1) && (nps > 0)) {
/* report the average S across planes */
		fprintf(stdout,"%.4f\tPlanar average S\n",ps/nps);
	} else if ((planar == 0) && (np > 1)) {
/* compute stdev for each vector */
		sx = sx / np;
		sy = sy / np;
		sz = sz / np;
		ssx = sqrt((ssx - np*(sx*sx))/(np-1));
		ssy = sqrt((ssy - np*(sy*sy))/(np-1));
		ssz = sqrt((ssz - np*(sz*sz))/(np-1));
/* compute 3D sphereicity */
		s = sqrt(1.0/(2*((ssx*ssx + ssy*ssy + ssz*ssz)/3.0)));
		s = s*sqrt(8.0*log(2.0));
		fprintf(stdout,"%.4f\tVolumetric S\n",s);
	} else {
		fprintf(stderr,"No valid voxels detected\n");
		fprintf(stdout,"%.4f\tError, no S computed.\n",0.0);
		goto errout;
	}

/* done */	
	if (mask) free(mask);
	if (data) free(data);
	tal_exit(0);

/* error */
errout:
	if (mask) free(mask);
	if (data) free(data);
	tal_exit(1);

	exit(0);
}

/* compute the t score for given N, p and S values */
/*
	f(t,s) = 1/(32*PI*s*s*exp(t*t)*p(g,t))
	p(g,t) = 0.5-0.5*erf(0.5*(2.0**0.5)*t)
*/
double	fsmooth(double t,double s)
{
	double	p,f;

	p = 0.5 - 0.5*erf(0.5*sqrt(2.0)*t);
	if (p == 0.0) return(0.0);
	f = 1.0/(32.0*M_PI*s*s*exp(t*t)*p);

	return(f);
}

double	smoothness(double n,double p,double s)
{
	double	t;
	double	inc;
	double	cur,val;
	long int	count;

/* start at t=inf */	
	t = 150;
	inc = -0.1;
	count = 0;
	cur = fsmooth(t,s);
	val = p/n;
	while (fabs(val - cur) > fabs(val/10000.0)) {
#ifdef DEBUG
		printf("cur,val,inc,s,t:%f %f %f %f %f\n",cur,val,inc,s,t);
#endif
		if ((cur > val) && (inc < 0)) inc = -inc;
		if ((cur < val) && (inc > 0)) inc = -(inc/2);
		t += inc;
		cur = fsmooth(t,s);
		count++;
		if (count > 100000) break;
	}
	return(t);
}
