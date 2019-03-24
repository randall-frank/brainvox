/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski
 *
 * $Id: tal_CDA.c 1213 2005-08-27 20:51:21Z rjfrank $
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

void	filter_raw_data(double *x,double *y,double *z,double *v,long int *num,
	long int why);

double gamma2(double *data,long int num,double *t,long int flag);
double gamma1(double *data,long int num,double *t);
void	printG1G2(double g1,double g2,double t1,double t2,
	long int n1,long int n2,FILE *o);
long int sigG2(double g2,long int n);

void ex_err(char *s)
{
	fprintf(stderr,"Fatal error:%s\n",s);
	tal_exit(1);
}

void cmd_err(char *s)
{
	fprintf(stderr,"(%s:%s) Usage: %s [options] inputtemp outputfile\n",__DATE__,TAL_VERSION,s);
	fprintf(stderr,"Options: -x(dx) image dx default:256\n");
	fprintf(stderr,"         -y(dy) image dy default:256\n");
	fprintf(stderr,"         -z(dz) image depth in bytes default:2\n");
	fprintf(stderr,"         -s(ddx) step size in x default:1\n");
	fprintf(stderr,"         -t(ddy) step size in y default:1\n");
	fprintf(stderr,"         -u(ddz) step size in z default:1\n");
	fprintf(stderr,"         -v analyze full volume default:talairach space only\n");
	fprintf(stderr,"         -g iterate the computation of g2/g1 yielding one datam per pass\n");
	fprintf(stderr,"         -verbose report g2/g1 on each pass when using -g option\n");
	fprintf(stderr,"         -o(offset) data offset (p+offset) default:-256\n");
	fprintf(stderr,"         -d(stddev) sd's for significance level default:2.575\n");
	fprintf(stderr,"         -ties(max_ties) maximum fraction of ties default:.25\n");
	fprintf(stderr,"         -n(num) number of patients summed to make volume default:1\n");
	fprintf(stderr,"         -f(start) first image number default:1\n");
	fprintf(stderr,"         -l(end) last image number default:15\n");
	fprintf(stderr,"         -i(step) image number step default:1\n");
	fprintf(stderr,"         -m(masktemp) volume template name default:none\n");
	fprintf(stderr,"         -b swap bytes with 16bit images default:no\n");
	fprintf(stderr,"         -r(dx[:dy:dz]) radius of max/min check default:2:2:2\n");
	fprintf(stderr,"         -vcheck(tempname) (min) (max) dump an 8bit visual check volume\n         scaling the pet values from min to max. default:none\n");
	fprintf(stderr,"         -thres(V) report all local max/mins with abs values > V\n");
	fprintf(stderr,"         -tail- use only values less than zero default:all values\n");
	fprintf(stderr,"         -tail+ use only values greater than zero default:all values\n");
	fprintf(stderr,"Note: if a tailX options is used, the mean value is assumed to be zero.\n");
	tal_exit(1);
}

long int s_maxmin8(unsigned char *data,long int dx,long int dy,long int dz,
	long int x,long int y,long int z,long int *radius,double *pnt,
	double off,long int *steps,unsigned char *mask)	
{
	long int i,j,k,val,cent;
	long int mean = 0;
	long int count = 0;
	long int ss = 0;
	long int out = 0;
	long int mn = 1000000;
	long int mx = -100000;
	long int dox = 0;
	long int doy = 0;
	long int doz = 0;
	long int smn,smx;

	long int ddy = (dx);
	long int ddz = (dx*dy);
	long int stx = MAX(x-(radius[0]*steps[0]),0);
	long int enx = MIN(x+(radius[0]*steps[0]),dx-1);
	long int sty = MAX(y-(radius[1]*steps[1]),0);
	long int eny = MIN(y+(radius[1]*steps[1]),dy-1);
	long int stz = MAX(z-(radius[2]*steps[2]),0);
	long int enz = MIN(z+(radius[2]*steps[2]),dz-1);

	unsigned char *ptr1,*ptr2,*ptr3;
	unsigned char	*mptr1,*mptr2,*mptr3;

	ptr1 = data + (stz*ddz);
	mptr1 = mask + (stz*ddz);
	for(k=stz;k<=enz;k+=steps[2]) {
		ptr2 = ptr1 + (sty*ddy);
		mptr2 = mptr1 + (sty*ddy);
		for(j=sty;j<=eny;j+=steps[1]) {
			ptr3 = ptr2 + (stx);
			mptr3 = mptr2 + (stx);
			for(i=stx;i<=enx;i+=steps[0]) {
				count++;
				val = (*ptr3) + off;
				if (*mptr3 == 0) return(out);
				if ((i != x) || (j != y) || (k != z)) {
					if (val < mn) {
						mn = val;
						smn = 1;
					} else if (val == mn) {
						smn++;
					}
					if (val > mx) {
						mx = val;
						smx = 1;
					} else if (val == mx) {
						smx++;
					}
				} else {
					cent = val;
				}
				dox += ((i-x)*val);
				doy += ((j-y)*val);
				doz += ((k-z)*val);
				ss += (val*val);
				mean += (val);
				ptr3+=steps[0];
				mptr3+=steps[0];
			}
			ptr2 += (steps[1]*ddy);
			mptr2 += (steps[1]*ddy);
		}
		ptr1 += (ddz*steps[2]);
		mptr1 += (ddz*steps[2]);
	}

	if ((mn == mx) && (mn == cent)) return(out);  /* FLAT case */
/* if point is strictly a max/min or is a tie with less than steps[3] */
	if ((mn > cent) || ((mn == cent) && (smn < steps[3]))) out = -1;
	if ((mx < cent) || ((mx == cent) && (smx < steps[3]))) out = 1;

	pnt[5] = x;
	pnt[6] = y;
	pnt[7] = z;
	pnt[0] = pnt[5] + (double)(dox)/(double)(mean);
	pnt[1] = pnt[6] + (double)(doy)/(double)(mean);
	pnt[2] = pnt[7] + (double)(doz)/(double)(mean);
	pnt[3] = (double)(mean)/(double)(count);
	pnt[4] = ((double)(ss)/(double)(count)) - (pnt[3]*pnt[3]);
	pnt[8] = (double)(cent);

/* var = ((1/n)*sum(x*x)) - mean*mean */

	return(out);
}

long int s_maxmin16(unsigned short *data,long int dx,long int dy,long int dz,
	long int x,long int y,long int z,long int *radius,double *pnt,
	double off,long int *steps,unsigned char *mask)	
{
	long int i,j,k,val,cent;
	long int mean = 0;
	long int count = 0;
	long int ss = 0;
	long int out = 0;
	long int mn = 1000000;
	long int mx = -100000;
	long int dox = 0;
	long int doy = 0;
	long int doz = 0;
	long int smn,smx;

	long int ddy = (dx);
	long int ddz = (dx*dy);
	long int stx = MAX(x-(radius[0]*steps[0]),0);
	long int enx = MIN(x+(radius[0]*steps[0]),dx-1);
	long int sty = MAX(y-(radius[1]*steps[1]),0);
	long int eny = MIN(y+(radius[1]*steps[1]),dy-1);
	long int stz = MAX(z-(radius[2]*steps[2]),0);
	long int enz = MIN(z+(radius[2]*steps[2]),dz-1);

	unsigned short *ptr1,*ptr2,*ptr3;
	unsigned char	*mptr1,*mptr2,*mptr3;

	ptr1 = data + (stz*ddz);
	mptr1 = mask + (stz*ddz);
	for(k=stz;k<=enz;k+=steps[2]) {
		ptr2 = ptr1 + (sty*ddy);
		mptr2 = mptr1 + (sty*ddy);
		for(j=sty;j<=eny;j+=steps[1]) {
			ptr3 = ptr2 + (stx);
			mptr3 = mptr2 + (stx);
			for(i=stx;i<=enx;i+=steps[0]) {
				count++;
				val = (*ptr3) + off;
				if (*mptr3 == 0) return(out);
				if ((i != x) || (j != y) || (k != z)) {
					if (val < mn) {
						mn = val;
						smn = 1;
					} else if (val == mn) {
						smn++;
					}
					if (val > mx) {
						mx = val;
						smx = 1;
					} else if (val == mx) {
						smx++;
					}
				} else {
					cent = val;
				}
				dox += ((i-x)*val);
				doy += ((j-y)*val);
				doz += ((k-z)*val);
				ss += (val*val);
				mean += (val);
				ptr3+=steps[0];
				mptr3+=steps[0];
			}
			ptr2 += (steps[1]*ddy);
			mptr2 += (steps[1]*ddy);
		}
		ptr1 += (ddz*steps[2]);
		mptr1 += (ddz*steps[2]);
	}

	if ((mn == mx) && (mn == cent)) return(out);  /* FLAT case */
/* if point is strictly a max/min or is a tie with less than steps[3] */
	if ((mn > cent) || ((mn == cent) && (smn < steps[3]))) out = -1;
	if ((mx < cent) || ((mx == cent) && (smx < steps[3]))) out = 1;

	pnt[5] = x;
	pnt[6] = y;
	pnt[7] = z;
	pnt[0] = pnt[5] + (double)(dox)/(double)(mean);
	pnt[1] = pnt[6] + (double)(doy)/(double)(mean);
	pnt[2] = pnt[7] + (double)(doz)/(double)(mean);
	pnt[3] = (double)(mean)/(double)(count);
	pnt[4] = ((double)(ss)/(double)(count)) - (pnt[3]*pnt[3]);
	pnt[8] = (double)(cent);

/* var = ((1/n)*sum(x*x)) - mean*mean */

	return(out);
}

long int s_maxminf(float *data,long int dx,long int dy,long int dz,
	long int x,long int y,long int z,long int *radius,double *pnt,
	double off,long int *steps,unsigned char *mask)	
{
	long int i,j,k;
	long int count = 0;
	long int out = 0;
	double	val,cent;
	double 	mn = 10000000;
	double 	mx = -10000000;
	double	ss = 0;
	double	mean = 0;
	double	dox = 0;
	double	doy = 0;
	double	doz = 0;
	long int smn,smx;

	long int ddy = (dx);
	long int ddz = (dx*dy);
	long int stx = MAX(x-(radius[0]*steps[0]),0);
	long int enx = MIN(x+(radius[0]*steps[0]),dx-1);
	long int sty = MAX(y-(radius[1]*steps[1]),0);
	long int eny = MIN(y+(radius[1]*steps[1]),dy-1);
	long int stz = MAX(z-(radius[2]*steps[2]),0);
	long int enz = MIN(z+(radius[2]*steps[2]),dz-1);

	float 		*ptr1,*ptr2,*ptr3;
	unsigned char	*mptr1,*mptr2,*mptr3;

	ptr1 = data + (stz*ddz);
	mptr1 = mask + (stz*ddz);
	for(k=stz;k<=enz;k+=steps[2]) {
		ptr2 = ptr1 + (sty*ddy);
		mptr2 = mptr1 + (sty*ddy);
		for(j=sty;j<=eny;j+=steps[1]) {
			ptr3 = ptr2 + (stx);
			mptr3 = mptr2 + (stx);
			for(i=stx;i<=enx;i+=steps[0]) {
				count++;
				val = (*ptr3) + off;
				if (*mptr3 == 0) return(out);
				if ((i != x) || (j != y) || (k != z)) {
					if (val < mn) {
						mn = val;
						smn = 1;
					} else if (val == mn) {
						smn++;
					}
					if (val > mx) {
						mx = val;
						smx = 1;
					} else if (val == mx) {
						smx++;
					}
				} else {
					cent = val;
				}
				dox += ((i-x)*val);
				doy += ((j-y)*val);
				doz += ((k-z)*val);
				ss += (val*val);
				mean += (val);
				ptr3+=steps[0];
				mptr3+=steps[0];
			}
			ptr2 += (steps[1]*ddy);
			mptr2 += (steps[1]*ddy);
		}
		ptr1 += (ddz*steps[2]);
		mptr1 += (ddz*steps[2]);
	}

	if ((mn == mx) && (mn == cent)) return(out);  /* FLAT case */
/* if point is strictly a max/min or is a tie with less than steps[3] */
	if ((mn > cent) || ((mn == cent) && (smn < steps[3]))) out = -1;
	if ((mx < cent) || ((mx == cent) && (smx < steps[3]))) out = 1;

	pnt[5] = x;
	pnt[6] = y;
	pnt[7] = z;
	pnt[0] = pnt[5] + (double)(dox)/(double)(mean);
	pnt[1] = pnt[6] + (double)(doy)/(double)(mean);
	pnt[2] = pnt[7] + (double)(doz)/(double)(mean);
	pnt[3] = (double)(mean)/(double)(count);
	pnt[4] = ((double)(ss)/(double)(count)) - (pnt[3]*pnt[3]);
	pnt[8] = (double)(cent);

/* var = ((1/n)*sum(x*x)) - mean*mean */

	return(out);
}


double gamma1(double *data,long int num,double *t)
{
	double	g1,sum,sv,n,mean;
	long int	i;

	n = num;

	sum = 0.0;
	for(i=0;i<num;i++) sum += data[i];
	mean = sum/n;

	sum = 0;
	sv = 0;
	for(i=0;i<num;i++) {
		sv += pow(data[i]-mean,2.0);
		sum += pow(data[i] - mean,3.0);
	}
	sv = sv/n;

	g1 = sum/(n*sv*sqrt(sv));
	*t = g1/sqrt(6.0*n*(n-1.0)/((n-2.0)*(n+1.0)*(n+3.0)));

	return(g1);
}

double gamma2(double *data,long int num,double *t,long int flag)
{
	double	g2,sum,sv,n,mean;
	long int	i;

	n = num;

	sum = 0.0;
	for(i=0;i<num;i++) sum += data[i];
	mean = sum/n;

	if (flag != 0) mean = 0.0;

	sum = 0;
	sv = 0;
	for(i=0;i<num;i++) {
		sv += pow(data[i]-mean,2.0);
		sum += pow(data[i] - mean,4.0);
	}
	sv = sv/n;

	g2 = sum/(n*sv*sv);
	*t = g2/sqrt(24.0*n*(n-1.0)*(n-1.0)/((n-3.0)*(n-2.0)*(n+3.0)*(n+5.0)));

	return(g2);
}

int main(int argc,char **argv)
{
	long int	iterateg2 = 0;
	long int	dx = 256;
	long int	dy = 256;
	long int	dz = 2;
	long int	istart = 1;
	long int	iend = 15;
	long int	istep = 1;
	long int	swab = 0;
	long int	radius[3] = {2,2,2};
	long int	xstep = 1;
	long int	ystep = 1;
	long int	zstep = 1;
	long int	full_vol = 0;
	long int	verbose = 0;
	long int	ftype = 0;
	double		offset = -256.0;
	double		sig_dev = 2.575;
	double		max_ties = 0.25;
	double		num_pats = 1.0;
	double		vthres = -999;
	double		checkmin = -100;
	double		checkmax = 100;
	char		*masktemp = 0L;
	char		*checktemp = 0L;
	char		intemp[256];
	char		tstr[256],*tp;
	long int	i,err,j,k;
	long int	icount,z,y,x,xs,xe,ys,ye;
	double		ss,sum,val;
	unsigned char	*data;
	unsigned char	*mask=0;
	unsigned char	*checkv=0;
	double		pnt[9];  /* x,y,z,mean,var,rx,ry,rz,v */
	double		max;
/* gamma2 stuff */
	double		*xarr,*yarr,*zarr,*varr,g2,g1,t1,t2;
	long int	pcount,porig;
	long int	pmax,steps[4];

	FILE		*ofp;

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
			case 's':
				xstep = atoi(&(argv[i][2]));
				break;
			case 't':
				if (strstr(&(argv[i][1]),"thres") != 0) {
					vthres = atof(&(argv[i][6]));
				} else if (strstr(&(argv[i][1]),"tail+") != 0) {
					ftype = 1;
				} else if (strstr(&(argv[i][1]),"tail-") != 0) {
					ftype = -1;
				} else if (strstr(&(argv[i][1]),"ties") !=0) {
					max_ties = atof(&(argv[i][5]));					
				} else {
					ystep = atoi(&(argv[i][2]));
				}
				break;
			case 'u':
				zstep = atoi(&(argv[i][2]));
				break;
			case 'o':
				offset = atof(&(argv[i][2]));
				break;
			case 'd':
				sig_dev = atof(&(argv[i][2]));
				break;
			case 'n':
				num_pats = atof(&(argv[i][2]));
				break;
			case 'g':
				iterateg2 = 1;
				break;
			case 'm':
				masktemp = &(argv[i][2]);
				break;
			case 'v':
				if (argv[i][2] == 'e') {
					verbose = 1;
				} else if (argv[i][2] == 'c') {
					checktemp = &(argv[i][7]);
					if (i < (argc-1)) {
						i++;
						checkmin = atof(argv[i]);
					} else {
						cmd_err(argv[0]);
					}
					if (i < (argc-1)) {
						i++;
						checkmax = atof(argv[i]);
					} else {
						cmd_err(argv[0]);
					}
				} else {
					full_vol = 1;
				}
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
	if (strcmp(argv[i+1],"-") == 0L) {
		ofp = stdout;
	} else {
		ofp = fopen(argv[i+1],"w");
		if (ofp == 0L) {
			printf("Unable to open output file:%s\n",argv[i+1]);
			tal_exit(1);
		}
	}
/* get the image memory */
	icount = 0;
	for(i=istart;i<=iend;i=i+istep) icount++;

	data = malloc(dz*dx*dy*icount);
	if (data == 0L) ex_err("Unable to allocate image memory.");
	mask = malloc(dx*dy*icount);
	if (mask == 0L) ex_err("Unable to allocate mask memory.");
/* get memory for a check slice if needed */
	if (checktemp) {
		checkv = malloc(dx*dy);
		if (checkv == 0L) ex_err("Unable to allocate mask memory.");
	}

/* read the images */
	printf("Reading %ld byte dataset for max/min at radius:%ld,%ld,%ld\n",
		(dz*dx*dy*icount),radius[0],radius[1],radius[2]);
	printf("Using data offset of %f  (v=v+%f)\n",offset,offset);
	j = 0;
	k = 0;
	for(i=istart;i<=iend;i=i+istep) {
		name_changer(intemp,i,&err,tstr);
		printf("Reading the file:%s\n",tstr);
		bin_io(tstr,'r',&(data[j]),dx,dy,dz,1,swab,0L);
		if (masktemp) {
/* read the mask */
			name_changer(masktemp,i,&err,tstr);
			printf("Reading the mask file:%s\n",tstr);
			bin_io(tstr,'r',&(mask[k]),dx,dy,1,1,swab,0L);
		} else {
/* no mask used */
			for(err=0;err<(dx*dy);err++) mask[k+err] = 1;
		}
		j = j + (dx*dy*dz);
		k = k + (dx*dy);
	}
/* get the arrays */
	pmax = 10000;
	pcount = 0;
	xarr = (double *)malloc(sizeof(double)*pmax);
	yarr = (double *)malloc(sizeof(double)*pmax);
	zarr = (double *)malloc(sizeof(double)*pmax);
	varr = (double *)malloc(sizeof(double)*pmax);
/* work from x=127+69,128-69 (58,196)  y=128+70,128-104 (198,24) */
	xs=radius[0];
	xe=(dx-radius[0]);
	ys=radius[1];
	ye=(dy-radius[1]);
	if (full_vol == 0) {
		xs=58+radius[0];
		xe=197-radius[0];
		ys=24+radius[1];
		ye=198-radius[1];
	}
	if (full_vol == 0) {
		fprintf(stderr,"Restricting analysis to talairach space.\n");
	} else {
		fprintf(stderr,"Analysis of full volume in progress.\n");
	}
	steps[0] = xstep;
	steps[1] = ystep;
	steps[2] = zstep;
/* number of points in volume */
	steps[3] = ((radius[0]*2)+1)*((radius[1]*2)+1)*((radius[2]*2)+1);
/* times percentage */
	steps[3] = steps[3] * max_ties;
/* write it out */  
	z = 0;
	for(i=istart;i<=iend;i=i+(istep*zstep)) {
	   if ((z >= radius[2]) && (z < (icount-radius[2]))) {
		fprintf(stderr,"Working on plane %ld of %ld\n",z,icount);

/* clear the check volume if needed */
		if (checkv) {
			for(y=0;y<dy;y++) {
			for(x=0;x<dx;x++) {
				if (dz == 1) {
					val = ((unsigned char *)data)
						[(z*dx*dy)+(y*dx)+x];
				} else if (dz == 2) {
					val = ((unsigned short *)data)
						[(z*dx*dy)+(y*dx)+x];
				} else if (dz == 4) {
					val = ((float *)data)
						[(z*dx*dy)+(y*dx)+x];
				} else {
					val = 0;
				}
				val = (val - checkmin)/(checkmax-checkmin);
				val = val * 127.0;
				if (val < 0) val = 0;
				if (val > 127.0) val = 127;
				checkv[(y*dx)+x] = val;
			}
			}
		}

/* scan the slice */
		for(y=ys;y<ye;y+=ystep) {
			for(x=xs;x<xe;x+=xstep) {
/* for later normalization purposes compute n,sum,ss */
				if (dz == 1) {
				    j=s_maxmin8((unsigned char *)data,dx,dy,
					  icount,x,y,z,radius,pnt,offset,
					steps,mask);
				} else if (dz == 2) {
				    j=s_maxmin16((unsigned short *)data,dx,dy,
					  icount,x,y,z,radius,pnt,offset,
					steps,mask);
				} else if (dz == 4) {
				    j=s_maxminf((float *)data,dx,dy,
					  icount,x,y,z,radius,pnt,offset,
					steps,mask);
				}
				if ((j == 1) || (j == -1)) {  /* found max */
								/* or min */
/* tag the vcheck image */
	if (checkv) {
		val = pnt[8] / num_pats;
		val = (val - (-10.0)) / (10.0 - (-10.0));
		val = val * 127;
		if (val < 0) val = 0;
		if (val > 127.0) val = 127;
		if (ftype == 0) {
			checkv[(y*dx)+x] = val+128;
		} else if ((ftype > 0) && (pnt[8] >= 0.0)) { 
			checkv[(y*dx)+x] = val+128;
		} else if ((ftype < 0) && (pnt[8] <= 0.0)) {
			checkv[(y*dx)+x] = val+128;
		}
	}
/* add it to the list */
	xarr[pcount] = pnt[0];
	yarr[pcount] = pnt[1];
	zarr[pcount] = pnt[2];
	varr[pcount] = pnt[8] / num_pats;
	pcount++;
	if (pcount >= pmax) {
/* add 5000 at a time */
		pmax += 5000;
		xarr = (double *)realloc(xarr,sizeof(double)*pmax);
		yarr = (double *)realloc(yarr,sizeof(double)*pmax);
		zarr = (double *)realloc(zarr,sizeof(double)*pmax);
		varr = (double *)realloc(varr,sizeof(double)*pmax);
	}
				}
			}
		}		
	    }
/* write the check volume if needed */
	    if (checkv) {
		name_changer(checktemp,i,&err,tstr);
		printf("Writing the check file:%s\n",tstr);
		bin_io(tstr,'w',checkv,dx,dy,1,1,swab,0L);
	    }
	    z+=zstep;
	}

/* compute gamma1,gamma2 */
	porig = pcount;
	g1 = gamma1(varr,pcount,&t1);
	g2 = gamma2(varr,pcount,&t2,ftype);

/* filter the data to look at a specific tail, or all data */
/* datafilter: 0=all  1=plus  -1=negative */
	filter_raw_data(xarr,yarr,zarr,varr,&pcount,ftype);

/* compute mean and sd of the detected mins/maxes */
	ss = 0.0;
	sum = 0.0;
	for(i=0;i<pcount;i++) {
		sum += varr[i];
		ss += (varr[i]*varr[i]);
	}
	if (ftype != 0) {
		sum = 0.0;   /* assume a mean of zero */
	}
/* sum becomes mean, ss becomes sd */
	sum = sum/((double)pcount);
	ss = sqrt( (ss/((double)pcount)) - (sum*sum));
	if (vthres != -999) {
		fprintf(ofp,"Values of magnitude greater than %f\n",vthres);
/* select all significant values */
		j = 0;
		for(i=0;i<pcount;i++) {
			if (fabs(varr[i]) > vthres) {
				xarr[j] = xarr[i];
				yarr[j] = yarr[i];
				zarr[j] = zarr[i];
				varr[j] = varr[i];
				j++;
			}
		}
		pcount = j;
/* sort the values */
		for(i=0;i<pcount-1;i++) {
			for(j=0;j<pcount-1;j++) {
				if (fabs(varr[j+1])>fabs(varr[j])) {
					g2 = xarr[j];
					xarr[j] = xarr[j+1];
					xarr[j+1] = g2;
					g2 = yarr[j];
					yarr[j] = yarr[j+1];
					yarr[j+1] = g2;
					g2 = zarr[j];
					zarr[j] = zarr[j+1];
					zarr[j+1] = g2;
					g2 = varr[j];
					varr[j] = varr[j+1];
					varr[j+1] = g2;
				}
			}
		}
/* output values */
		for(i=0;i<pcount;i++) {
	        fprintf(ofp,"%f %f %f %f %f\n",xarr[i],yarr[i],zarr[i],
                        varr[i],((varr[i]-sum)/ss));
                fprintf(ofp,"Tal:%f %f %f %f\n",128.0-xarr[i],
                        yarr[i]-128.0,zarr[i]-44.0,((varr[i]-sum)/ss));
		}
		tal_exit(0);
	}
/* apply the gamma2 stuff */
	printf("Detected %ld local max/mins \n",pcount);
	fprintf(ofp,"Detected %ld local max/mins \n",pcount);
if (iterateg2 == 0) {
/* the standard way: 1) check g2 2) report all that pass g1 */

/* compute gamma 2 until gamma2 is smaller than # of deviations*sd estimate */
/* # of deviations (significance level) is given by user */
/* sd estimate is sqrt(24/n) */
	g2 = gamma2(varr,pcount,&t2,ftype);
	if (sigG2(g2,pcount)) {
		printG1G2(g1,g2,t1,t2,porig,pcount,ofp);
/* select all significant values */
		j = 0;
		for(i=0;i<pcount;i++) {
			if (fabs((varr[i]-sum)/ss) > sig_dev) {
				xarr[j] = xarr[i];
				yarr[j] = yarr[i];
				zarr[j] = zarr[i];
				varr[j] = varr[i];
				j++;
			}
		}
		pcount = j;
/* sort the values */
		for(i=0;i<pcount-1;i++) {
			for(j=0;j<pcount-1;j++) {
				if (fabs((varr[j+1]-sum)/ss) > 
					fabs((varr[j]-sum)/ss)) {
					g2 = xarr[j];
					xarr[j] = xarr[j+1];
					xarr[j+1] = g2;
					g2 = yarr[j];
					yarr[j] = yarr[j+1];
					yarr[j+1] = g2;
					g2 = zarr[j];
					zarr[j] = zarr[j+1];
					zarr[j+1] = g2;
					g2 = varr[j];
					varr[j] = varr[j+1];
					varr[j+1] = g2;
				}
			}
		}
/* list all significant values */
		for(i=0;i<pcount;i++) {
	        fprintf(ofp,"%f %f %f %f %f\n",xarr[i],yarr[i],zarr[i],
                        varr[i],((varr[i]-sum)/ss));
                fprintf(ofp,"Tal:%f %f %f %f\n",128.0-xarr[i],
                        yarr[i]-128.0,zarr[i]-44.0,((varr[i]-sum)/ss));
		}
	} else {
		printG1G2(g1,g2,t1,t2,porig,pcount,ofp);
	}
} else {
/* compute gamma2 until gamma2 is smaller than # of deviations*sd estimate */
/* # of deviations (significance level) is given by user */
/* sd estimate is sqrt(24/n) */
	g2 = gamma2(varr,pcount,&t2,ftype);
	if (sigG2(g2,pcount)) {
		printG1G2(g1,g2,t1,t2,porig,pcount,ofp);
		while (sigG2(g2,pcount)) {
/* select the most significant value */
			j = -1;
			max = -999999;
			for(i=0;i<pcount;i++) {
				if (fabs((varr[i]-sum)/ss) > max) {
					max = fabs((varr[i]-sum)/ss);
					j = i;
				}
			}
			if (j == -1) break;
/* if the g1 is no longer significant, break out */
			if (max < sig_dev) break;
/* print out the value */
	        fprintf(ofp,"%f %f %f %f %f\n",xarr[j],yarr[j],zarr[j],
                        varr[j],((varr[j]-sum)/ss));
                fprintf(ofp,"Tal:%f %f %f %f\n",128.0-xarr[j],
                        yarr[j]-128.0,zarr[j]-44.0,((varr[j]-sum)/ss));
/* compress the dataset  by one */
			pcount = pcount - 1;
			for(i=j;i<pcount;i++) {
				xarr[i] = xarr[i+1];
				yarr[i] = yarr[i+1];
				zarr[i] = zarr[i+1];
				varr[i] = varr[i+1];
			}
/* recompute mean and sd of the remaining detected mins/maxes */
			ss = 0.0;
			sum = 0.0;
			for(i=0;i<pcount;i++) {
				sum += varr[i];
				ss += (varr[i]*varr[i]);
			}
/* sum becomes mean, ss becomes sd */
			sum = sum/((double)pcount);
			ss = sqrt( (ss/((double)pcount)) - (sum*sum));
/* recompute the gamma values (g1 only if the dataset is complete) */
			if (ftype == 0) g1 = gamma1(varr,pcount,&t1);
			g2 = gamma2(varr,pcount,&t2,ftype);
			if (verbose) {
				printG1G2(g1,g2,t1,t2,porig,pcount,ofp);
			}
		}
	} else {
		printG1G2(g1,g2,t1,t2,porig,pcount,ofp);
	}
}
	if (ofp != stdout) fclose(ofp);
/* done */	
	free(data);
	free(mask);
	if (checkv) free(checkv);
	tal_exit(0);
	exit(0);
}

void	filter_raw_data(double *x,double *y,double *z,double *v,long int *num,
	long int why)
{
	long int 	i,j;

	if (why == 0) return;

/* remove points, packing the array as we go */
	j = 0;
	for(i=0;i<(*num);i++) {
		if (why > 0) {
			if (v[i] >= 0.0) {
				x[j] = x[i];
				y[j] = y[i];
				z[j] = z[i];
				v[j] = v[i];
				j++;
			}
		} else {
			if (v[i] <= 0.0) {
				x[j] = x[i];
				y[j] = y[i];
				z[j] = z[i];
				v[j] = v[i];
				j++;
			}
		}
	}
	*num = j;

	return;
}

void interp(long int n,long int n1,long int n2,double v1,double v2,double *o);
void	Kprob(long int n,double *u1,double *u5,double *l5,double *l1);
void	Sprob(long int	n,double *p1,double *p5);

void	printG1G2(double g1,double g2,double t1,double t2,
		long int n1,long int n2,FILE *ofp)
{
	double	u1,u5,l1,l5;
	double	p1,p5;

	Kprob(n2,&u1,&u5,&l5,&l1);
	Sprob(n1,&p1,&p5);

	if (g1 > p1) {
		fprintf(ofp,"Gamma1 is %f (sig at 0.01 (%f))\n",
			g1,p1);	
	} else if (g1 > p5) {
		fprintf(ofp,"Gamma1 is %f (sig at 0.05 (%f))\n",
			g1,p5);
	} else {
		fprintf(ofp,"Gamma1 is %f (not significant (%f))\n",
			g1,p5);
	}
	if (n1 > 500) fprintf(ofp,"Gamma1 t score=%f df=%ld\n",t1,n1-1);

	if (g2 > u1) {
		fprintf(ofp,"Gamma2 is %f (leptokurtosis sig at 0.01(+) (%f))\n",
			g2,u1);
	} else if (g2 > u5) {
		fprintf(ofp,"Gamma2 is %f (leptokurtosis sig at 0.05(+) (%f))\n",
			g2,u5);
	} else if (g2 < l1) {
		fprintf(ofp,"Gamma2 is %f (platykurtosis sig at 0.01(-) (%f))\n",
			g2,l1);
	} else if (g2 < l5) {
		fprintf(ofp,"Gamma2 is %f (platykurtosis sig at 0.05(-) (%f))\n",
			g2,l5);
	} else {
		fprintf(ofp,"Gamma2 is %f (not significant (%f,%f))\n",
			g2,u5,l5);
	}
	if (n2 > 2000) fprintf(ofp,"Gamma2 t score=%f df=%ld\n",t2,n2-1);

	return;
}

/*
	g2 high leptokurtosis - print points.
	g2 low  platykurtosis - no points printed.
*/

long int sigG2(double g2,long int n)
{
	double	u1,u5,l1,l5;

	Kprob(n,&u1,&u5,&l5,&l1);

	if (g2 > u5) return(1);

	return(0);
}


void	Kprob(long int n,double *u1,double *u5,double *l5,double *l1)
{
	long int	nv[27] = { 50,75,100,125,150,200,250,300,
					350,400,450,500,550,600,650,
					700,750,800,850,900,950,1000,
					1200,1400,1600,1800,2000};
	double		up1[27] = {4.88,4.59,4.39,4.24,4.13,3.98,3.87,
				3.79,3.72,3.67,3.63,3.60,3.57,3.54,
				3.52,3.50,3.48,3.46,3.45,3.43,3.42,3.41,
				3.37,3.34,3.32,3.30,3.28};
	double		up5[27] = {3.99,3.87,3.77,3.71,3.65,3.57,3.52,3.47,
				3.44,3.41,3.39,3.37,3.35,3.34,3.33,
				3.31,3.30,3.29,3.28,3.28,3.27,3.26,
				3.24,3.22,3.21,3.20,3.18};
	double		lp5[27] = {2.15,2.27,2.35,2.40,2.45,2.51,2.55,
				2.59,2.62,2.64,2.66,2.67,2.69,2.70,
				2.71,2.72,2.73,2.74,2.74,2.75,2.76,
				2.76,2.78,2.80,2.81,2.82,2.83};
	double		lp1[27] = {1.95,2.08,2.18,2.24,2.29,2.37,2.42,2.46,
				2.50,2.52,2.55,2.57,2.58,2.60,2.61,
				2.62,2.64,2.65,2.66,2.66,2.67,2.68,
				2.71,2.72,2.74,2.76,2.77};

	long int	i,j;

	j = 0;
	for(i=0;i<26;i++) {
		if ((n>= nv[i]) && (n <= nv[i+1])) j = i;
	}

	if (n < nv[0]) j = 0;
	if (n > nv[26]) j = 25;

	interp(n,nv[j],nv[j+1],up5[j],up5[j+1],u5);
	interp(n,nv[j],nv[j+1],up1[j],up1[j+1],u1);
	interp(n,nv[j],nv[j+1],lp5[j],lp5[j+1],l5);
	interp(n,nv[j],nv[j+1],lp1[j],lp1[j+1],l1);

	return;
}

void	Sprob(long int	n,double *p1,double *p5)
{
	long int	nv[21] = {25,30,35,40,45,50,60,70,80,90,100,
				125,150,175,200,250,300,350,400,450,500};

	double		pp5[21] = {0.711,0.661,0.621,0.587,0.558,0.533,0.492,
				0.459,0.432,0.409,0.389,
				0.350,0.321,0.298,0.280,0.251,0.230,
				0.213,0.200,0.188,0.179};
	double		pp1[21] = {1.061,0.982,0.921,0.869,0.825,0.787,0.723,
				0.673,0.631,0.596,0.567,
				0.508,0.464,0.430,0.403,0.360,0.329,
				0.305,0.285,0.269,0.255};
	long int	i,j;

	j = 0;
	for(i=0;i<20;i++) {
		if ((n>= nv[i]) && (n <= nv[i+1])) j = i;
	}

	if (n < nv[0]) j = 0;
	if (n > nv[20]) j = 19;

	interp(n,nv[j],nv[j+1],pp5[j],pp5[j+1],p5);
	interp(n,nv[j],nv[j+1],pp1[j],pp1[j+1],p1);

	return;
}

void interp(long int n,long int n1,long int n2,double v1,double v2,double *o)
{
	double	d;
	
	d = ((double)(n - n1))/((double)(n2 - n1));
	
	*o = d*(v2-v1) + v1;

	return;
}

