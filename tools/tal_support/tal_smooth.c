/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski
 *
 * $Id: tal_smooth.c 1213 2005-08-27 20:51:21Z rjfrank $
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

double	*gtab;
int	verbose = 0;

void build_gauss(double *gauss,long int *radius,long int cutoff);

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
	fprintf(stderr,"         -v verbose mode\n");
	fprintf(stderr,"         -m(masktemp) filename template for a mask volume\n");
	fprintf(stderr,"         -d(back) background value output outside of the mask default:0.0\n");
	fprintf(stderr,"         -r(dx[:dy:dz]) radius of averaging filter default:7:7:7\n");
	fprintf(stderr," output =  mean(pixels within radius)\n");
	tal_exit(1);
}

double s_mean8(unsigned char *data,long int dx,long int dy,long int dz,
	long int x,long int y,long int z,long int *radius)
{
	long int i,j,k,ind;
	double mean = 0.0;
	double count = 0.0;

	long int ddy = (dx);
	long int ddz = (dx*dy);
	long int stx = MAX(x-radius[0],0);
	long int enx = MIN(x+radius[0],dx-1);
	long int sty = MAX(y-radius[1],0);
	long int eny = MIN(y+radius[1],dy-1);
	long int stz = MAX(z-radius[2],0);
	long int enz = MIN(z+radius[2],dz-1);

	unsigned char *ptr1,*ptr2,*ptr3;
	long int inc1 = (2*radius[0]) + 1;
	long int inc2 = inc1 * ((2*radius[1]) + 1);

	ptr1 = data + (stz*ddz);
	for(k=stz;k<=enz;k++) {
		ptr2 = ptr1 + (sty*ddy);
		for(j=sty;j<=eny;j++) {
			ptr3 = ptr2 + (stx);
			ind = (k-z+radius[2])*inc2;
			ind += (j-y+radius[1])*inc1;
			ind += (stx-x+radius[0]) + 1;
			for(i=stx;i<=enx;i++) {
				count += gtab[ind];
				mean = mean + gtab[ind]*(double)(*ptr3++);
				ind++;
			}
			ptr2 += ddy;
		}
		ptr1 += ddz;
	}
	return(mean/count);
}

double s_mean16(unsigned short *data,long int dx,long int dy,long int dz,
	long int x,long int y,long int z,long int *radius)
{
	long int i,j,k,ind;
	double mean = 0.0;
	double count = 0.0;

	long int ddy = (dx);
	long int ddz = (dx*dy);
	long int stx = MAX(x-radius[0],0);
	long int enx = MIN(x+radius[0],dx-1);
	long int sty = MAX(y-radius[1],0);
	long int eny = MIN(y+radius[1],dy-1);
	long int stz = MAX(z-radius[2],0);
	long int enz = MIN(z+radius[2],dz-1);

	unsigned short *ptr1,*ptr2,*ptr3;
	long int inc1 = (2*radius[0]) + 1;
	long int inc2 = inc1 * ((2*radius[1]) + 1);

	ptr1 = data + (stz*ddz);
	for(k=stz;k<=enz;k++) {
		ptr2 = ptr1 + (sty*ddy);
		for(j=sty;j<=eny;j++) {
			ptr3 = ptr2 + (stx);
			ind = (k-z+radius[2])*inc2;
			ind += (j-y+radius[1])*inc1;
			ind += (stx-x+radius[0]) + 1;
			for(i=stx;i<=enx;i++) {
				count += gtab[ind];
				mean = mean + gtab[ind]*(double)(*ptr3++);
				ind++;
			}
			ptr2 += ddy;
		}
		ptr1 += ddz;
	}
	return(mean/count);
}

double s_meanf(float *data,long int dx,long int dy,long int dz,
	long int x,long int y,long int z,long int *radius)
{
	long int i,j,k,ind;
	double mean = 0.0;
	double count = 0.0;

	long int ddy = (dx);
	long int ddz = (dx*dy);
	long int stx = MAX(x-radius[0],0);
	long int enx = MIN(x+radius[0],dx-1);
	long int sty = MAX(y-radius[1],0);
	long int eny = MIN(y+radius[1],dy-1);
	long int stz = MAX(z-radius[2],0);
	long int enz = MIN(z+radius[2],dz-1);

	float 	*ptr1,*ptr2,*ptr3;
	long int inc1 = (2*radius[0]) + 1;
	long int inc2 = inc1 * ((2*radius[1]) + 1);

	ptr1 = data + (stz*ddz);
	for(k=stz;k<=enz;k++) {
		ptr2 = ptr1 + (sty*ddy);
		for(j=sty;j<=eny;j++) {
			ptr3 = ptr2 + (stx);
			ind = (k-z+radius[2])*inc2;
			ind += (j-y+radius[1])*inc1;
			ind += (stx-x+radius[0]) + 1;
			for(i=stx;i<=enx;i++) {
				count += gtab[ind];
				mean = mean + gtab[ind]*(double)(*ptr3++);
				ind++;
			}
			ptr2 += ddy;
		}
		ptr1 += ddz;
	}
	return(mean/count);
}

void build_gauss(double *gauss,long int *radius,long int cutoff)
{
	long int 	i,j,k,m;
	double		r,s;

/* compute sigma for FWHM (=0.5) at cutoff */
	s = -(double)(cutoff*cutoff)/(2*log(0.5));

	if (verbose) fprintf(stderr,"Sigma: %f\n\n",sqrt(s));
	if (verbose) fprintf(stdout,"%ld %ld %ld\n",radius[0]*2+1,radius[1]*2+1,
		radius[2]*2+1);
	m = 1;
	gauss[0] = 0;
	for(k=-radius[2];k<=radius[2];k++) {
	for(j=-radius[1];j<=radius[1];j++) {
	for(i=-radius[0];i<=radius[0];i++) {
		r = (k*k) + (j*j) + (i*i);
		gauss[m] = 255*exp(-(r/(2*s))); /*scaled by 255 for int work*/
		gauss[0] += gauss[m];
		if (verbose) fprintf(stdout,"%.4f ",gauss[m]/255.0);
		m++;
	}
	if (verbose) fprintf(stdout,"\n");
	}
	if (verbose) fprintf(stdout,"\n");
	}
	if (verbose) fprintf(stderr,"Sum=%.4f\n\n",gauss[0]/255.0);
	return;
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
	long int	radius[3] = {7,7,7};
	char		*masktemp = 0L;
	unsigned char	*maskvol;
	double		backval = 0.0;

	char		intemp[256],outtemp[256];
	char		tstr[256],*tp;
	long int	i,err,j;
	long int	icount,z,y,x;
	unsigned char	*data;
	unsigned char	*bptr;
	unsigned short	*sptr;
	float 		*fptr;
	unsigned char	*oimage;

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
			case 'd':
				backval = atof(&(argv[i][2]));
				break;
			case 'm':
				masktemp = &(argv[i][2]);
				break;
			case 'r':
				radius[0] = atoi(&(argv[i][2]));
				radius[1] = atoi(&(argv[i][2]));
				radius[2] = atoi(&(argv[i][2]));
				tp = strchr(&(argv[i][2]),':');
				if (tp == 0L) break;
				radius[1] = atoi(tp+1);
				tp = strchr(tp+1,':');
				if (tp == 0L) break;
				radius[2] = atoi(tp+1);
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
	oimage = malloc(dz*dx*dy);
	if (oimage == 0L) ex_err("Unable to allocate image memory.");

	maskvol = malloc(dx*dy);
	if (maskvol == 0L) ex_err("Unable to allocate image memory.");

	icount = 0;
	for(i=istart;i<=iend;i=i+istep) icount++;

	data = malloc(dz*dx*dy*icount);
	if (data == 0L) ex_err("Unable to allocate image memory.");

	j = (2*radius[0] + 1);
	j *= (2*radius[1] + 1);
	j *= (2*radius[2] + 1);
	gtab = (double *)malloc((j+1)*sizeof(double));
	if (gtab == 0L) ex_err("Unable to allocate gaussian table memory.");
	build_gauss(gtab,radius,radius[0]);

/* read the images */
	fprintf(stderr,"Reading %ld byte dataset for smoothing at radius:%ld,%ld,%ld\n",
		(dz*dx*dy*icount),radius[0],radius[1],radius[2]);

	j = 0;
	for(i=istart;i<=iend;i=i+istep) {
		name_changer(intemp,i,&err,tstr);
		fprintf(stderr,"Reading the file:%s\n",tstr);
		bin_io(tstr,'r',&(data[j]),dx,dy,dz,1,swab,0L);
		j = j + (dx*dy*dz);
	}
/* write it out */
	z = 0;
	if (dz == 1) {
		bptr = (unsigned char *)oimage;
	} else if (dz == 2) {
		sptr = (unsigned short *)oimage;
	} else if (dz == 4) {
		fptr = (float *)oimage;
	}
	for(i=istart;i<=iend;i=i+istep) {

/* initialize the output image */
		if (dz == 1) {
			for(x=0;x<(dx*dy);x++) bptr[x] = backval;
		} else if (dz == 2) {
			for(x=0;x<(dx*dy);x++) sptr[x] = backval;
		} else if (dz == 4) {
			for(x=0;x<(dx*dy);x++) fptr[x] = backval;
		}
		for(x=0;x<(dx*dy);x++) maskvol[x] = 1;

/* read mask file */
		if (masktemp != 0L) {
			name_changer(masktemp,i,&err,tstr);
			fprintf(stderr,"Reading mask file:%s\n",tstr);
			bin_io(tstr,'r',maskvol,dx,dy,1,1,swab,0L);
		}

		for(x=0;x<(dx);x++) {
			for(y=0;y<(dy);y++) {
			if (maskvol[x+(y*dx)] != 0) {
				if (dz == 1) {
					bptr[x+(y*dx)] =
					  s_mean8((unsigned char *)data,dx,dy,
					  	icount,x,y,z,radius);
				} else if (dz == 2) {
					sptr[x+(y*dx)] = 
					  s_mean16((unsigned short *)data,dx,dy,
					  	icount,x,y,z,radius);
				} else if (dz == 4) {
					fptr[x+(y*dx)] = 
					  	s_meanf((float *)data,dx,dy,
					  	icount,x,y,z,radius);
				}
			}
			}
		}		
		name_changer(outtemp,i,&err,tstr);
		fprintf(stderr,"Writing sampled file:%s\n",tstr);
		bin_io(tstr,'w',oimage,dx,dy,dz,1,swab,0L);
		z++;
	}
	
	free(data);
	free(oimage);
	free(maskvol);
	free(gtab);

	tal_exit(0);
	exit(0);
}
