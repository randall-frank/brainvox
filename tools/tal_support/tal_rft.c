/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski
 *
 * $Id: tal_rft.c 1214 2005-08-30 00:40:52Z rjfrank $
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

/* This code determines the p maxima value for a small volume based on the
*  user-requested field type.  The user provides the z/t/f/chi value, dfs, resel
*  size for each dimension, and the volume for resel count (euler charactistics) 
*  and corrected p value is returned.  The code is based on the article 
*  by Worsley et. al., "A Unified Statistical Approach
*  for Determining Significant Signals in Images of Cerebral Activation".  
*  (For your reference, here is the url of the paper: 
*  http://euclid.math.mcgill.ca/keith/unified/unified.abstract.html)
*  According to the paper, the unified p value is equal to the sum of the 
*  of the ec * ec density product.  The code for determining
*  ECs was derived from the tal_euler with a twofold modification:
*  1.  The threshold is a single value/ not a range.  2.  The edges of 
*  a volume, if within the mask/ above the threshold, are assumed to be 
*  closed boundaries for the search volume; hence the get_local_info applies
*  from -1 to dx/dy/icount.  The code for the d0 fields  is taken from 
*  stats.c, tal_standardize.  Similarily, the code for the d3 fields is 
*  taken from worsley.c.  The d1 and d2 fields utilize the stats.c functions. 
*  Issues of computing the correct statistical map are not addressed in this
*  program.
*/

/* Modifications on 12-06-00:  1.  The array for calculating multiple euler characteristics 
* based on multiple thresholds has been removed.  2.  tp was freed from memory.  3.  The option 
* for diameter and surface 2/3 correction was implemented. 4.  Both criteria that df1 >=D, or
* (df1 + df2) > D for t and f fields respectively are checked. */

/* Modifications 1-16-01: -r option is changed for resel size as an input.  A threshold range is implemented 
* with a start, step, stop threshold values.  The test statistic threshold has been changed from -v to -t.  
* The field specifier option has been changed from tal_standardize style options to worsley style options.
* Several log transforms were utilized to reduce the occurrances of overflow; this problem has not be fully
* fixed. */ 

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "proto.h"
#include "stats_inc.h"

/* actions */
/* note that unlike the tal_standardize code, the chi 2 z replaces the t2z */ 
#define ACT_C2P 1
#define ACT_T2P 2
#define ACT_Z2P 3
#define ACT_F2P 4

/* params */
#define V_DF1 0
#define V_DF2 1

#define MIN(a,b) (((a)<(b)?(a):(b)))
#define MAX(a,b) (((a)>(b)?(a):(b)))

typedef struct {
	long int	dx;
	long int	dy;
	long int	dz;
	long int	icount;
	unsigned short	*mask;
	unsigned char	*data;
	unsigned short	*sdata;
	float 		*fdata;
} c_info;

typedef  double ( * CumDistFunction )(double v,double *vec);


/* local prototypes */

void ex_err(char *s);
void cmd_err(char *s);

void action(double *val,long int act,double *vec);
double interp_dist(double v,CumDistFunction func,double *vec);
double pzv(double r,double t);
double p1zv(double r,double t);
double p2zv(double r,double t);
double ptv(double r,double t,double n);
double p1tv(double r,double t,double n);
double p2tv(double r,double t,double n);
double pfv(double r,double t,double k,double v);
double p1fv(double r,double t,double k,double v);
double p2fv(double r,double t,double k,double v);
double pxv(double r,double t,double v);
double p1xv(double r,double t,double v);
double p2xv(double r,double t,double v);
long int get_local(c_info *info,long int x,long int y, long int z,
	double *val,long int *vcheck);


/* get p value for z d0 */ 
void action_proc(double *val,long int act,double *vec)
{
	double	p,x;
	
	switch(act) {
	case ACT_Z2P:
	p = fabs(*val);
	x = 1.0 - CDF_normal(p,NULL);
	
#ifdef TABLE_DEBUG
	printf("Z=%f -> p=%f\n",*val,x);
#endif
			*val = x;
			break;
			
			
	case ACT_T2P:
	/* output = 1.0 - CDF(absvalue(t),df) */
	p = fabs(*val);
	x = 1.0 - CDF_t(p,vec);
#ifdef TABLE_DEBUG
	printf("t=%f,%f -> p=%f\n",*val,vec[0],x);
#endif
			*val = x;
			break;
			
	case ACT_F2P:
/* output = 1.0 - CDF(absvalue(f),df1,df2) */
			p = fabs(*val);
			x = 1.0 - CDF_f(p,vec);
#ifdef TABLE_DEBUG
	printf("f=%f,%f,%f -> p=%f\n",*val,vec[0],vec[1],x);
#endif
			*val = x;
			break;
	case ACT_C2P:
	/* output = CDF_chi(absvalue(x),df1) */
			p = fabs(*val);
			x = CDF_chi(p,vec);
#ifdef TABLE_DEBUG
	printf("f=%f,%f,%f -> p=%f\n",*val,vec[0],x);
#endif
		*val = x;
		 break;
			
	}

	return;
}

/* get p value for z d3 */
double pzv(double r,double t)
{
	double	pi,p;

	pi = atan2(0.0,-1.0);

	p = r * pow(4.0*log(2.0),3.0/2.0) * (pow(2.0*pi,-2.0)) * 
		(t*t -1.0)*exp(-0.5*t*t);

	return p;
}

/* get p value for z d1 */
double p1zv(double r, double t)
{
	double pi,p;
	
	pi = atan2(0.0,-1.0);
	
	p = r * pow(4.0*log(2.0),1.0/2.0) * (pow(2.0*pi,-1.0))*
		exp(-0.5*t*t);
		
	return p;
	
}

/* get p value for z d2 */
double p2zv(double r, double t)
{
	double pi,p;
	
	pi = atan2(0.0,-1.0);
	
	p = r * (4.0*log(2.0)) * (pow(2.0*pi,-3.0/2.0)) * 
		(t)*exp(-0.5*t*t);

	return p;
}

	
/* t to p, with v degrees of freedom */	
/* t to p for d3 */
/* note unlike worsley.c n = df and df+1 */
double ptv(double r,double t,double n)
{
	double	pi,p;

	pi = atan2(0.0,-1.0);

	p = r * (pow(4.0*log(2.0),(3.0/2.0)) * pow(2.0*pi,-2.0));
	p = p * (((n-1.0)/n)*t*t-1.0) * pow(1.0+t*t/n,-0.5*(n-1.0));

	return p;
}	

/* t to p for d1 */
double p1tv(double r,double t, double n)
{
	double	pi,p;

	pi = atan2(0.0,-1.0);

	p = r * (pow(4.0*log(2.0),(1.0/2.0)) * pow(2.0*pi,-1.0));
	p = p  * (pow(1.0+t*t/n,-0.5*(n-1.0)));

	return p;
}	
	
/* t to p for d2 */	
double p2tv(double r,double t,double n)
{
	double	pi,p;

	pi = atan2(0.0,-1.0);

	p = r * (4.0*log(2.0) * pow(2.0*pi,-(3.0/2.0)));
	p = p * (exp(gammln((n+1.0)/2.0)-(log(pow(n/2.0,0.5))+gammln(n/2.0))));
	p = p * pow((1.0+(t*t/n)),(-0.5*(n-1.0)))*t;

	return p;
}	 

/* F to p, with k and v degrees of freedom */
/* note that unlike worsley.c, the dfs are entered into the function instead of dfs+1.0*/
/* F to p for d3 */
double pfv(double r,double t,double k,double v)
{
	double	pi,p;

	pi = atan2(0.0,-1.0);
	
	p = r * (pow(4.0*log(2.0),(3.0/2.0)) * pow(2*pi,-(3.0/2.0)));
	p = p * exp(gammln((v+k-3.0)/2.0)+log(pow(2.0,-0.5))-(gammln(v/2.0)+gammln(k/2.0)));
	p = p * pow((k*t)/v,0.5*(k-3.0))*pow(1.0+((k*t)/v),-0.5*(v+k-2.0));
	p = p * ((v-1.0)*(v-2.0)*pow((k*t)/v,2.0) - 
		(2.0*v*k -v -k -1.0)*((k*t)/v) +
		(k-1.0)*(k-2.0));

	return p;
}	

/* F to p for d1 */
double p1fv(double r,double t,double k,double v)
{
	double	pi,p;

	pi = atan2(0.0,-1.0);
	

	p = r * (pow(4.0*log(2.0),(1.0/2.0)) * pow(2*pi,-(1.0/2.0)));
	p = p * exp(gammln((v+k-1.0)/2.0) +log(pow(2.0, 0.5)) - (gammln(v/2.0)+gammln(k/2.0)));
	p = p * pow((k*t)/v,0.5*(k-1.0))*pow(1.0+((k*t)/v),-0.5*(v+k-2.0));
	
	return p;
}
	
/* F to p for d2 */
double p2fv(double r,double t,double k,double v)
{
	double	pi,p;

	pi = atan2(0.0,-1.0);
	

	p = r * (4.0*log(2.0)) * pow(2*pi,-1.0);
	p = p * exp(gammln((v+k-2.0)/2.0)-(gammln(v/2.0) + gammln(k/2.0)));
	p = p * pow((k*t)/v,0.5*(k-2.0))*pow(1.0+((k*t)/v),-0.5*(v+k-2.0));
	p = p * (((v-1.0)*(k*t)/v) - (k-1.0));

	return p;
}	
	
#ifdef WIN32
#define finite _finite
#endif
	
/*chi squared field to p with v degrees of freedom */
/*unlike worsley.c the df is entered into function instead of df+1*/
/*chi squared to p for d3 */
double pxv(double r,double t,double v)
{
	double	pi,p;
	pi = atan2(0.0,-1.0);
	
	if (finite(exp(gammln(v/2.0)))==0) {
	p=0;
	}
	
	else {
	p = r * (pow(4.0*log(2.0),(3.0/2.0)) * pow(2.0*pi,-(3.0/2.0)));
	p = p * pow(t,0.5*(v-3.0))*exp(-0.5*t);
	p = p / ( pow(2.0,0.5*(v-2.0))*exp(gammln(v/2.0)) );
	p = p * (t*t - (2.0*v-1.0)*t + (v-1.0)*(v-2.0));
	}
	
	return p;
}	
	
/*chi squared to p for d1 */
double p1xv(double r,double t,double v)
{
	double	pi,p;

	pi = atan2(0.0,-1.0);
	

	if (finite(exp(gammln(v/2.0)))==0) {
	p=0;
	}

	else {
	p = r * (pow(4.0*log(2.0),(1.0/2.0)) * pow(2.0*pi,-(1.0/2.0)));
	p = p * pow(t,0.5*(v-1.0))*exp(-0.5*t);
	p = p / ( pow(2.0,0.5*(v-2.0))*exp(gammln(v/2.0)) );
	}

	return p;
}	
/*chi squared to p for d3 */
double p2xv(double r,double t,double v)
{
	double	pi,p;

	pi = atan2(0.0,-1.0);
	
	if (finite(exp(gammln(v/2.0)))==0) {
	p=0;
	}

	else {
	p = r * (4.0*log(2.0)) * pow(2.0*pi,-1.0);
	p = p * pow(t,0.5*(v-2.0))*exp(-0.5*t);
	p = p / ( pow(2.0,0.5*(v-2.0))*exp(gammln(v/2.0)) );
	p = p * (t - (v-1.0));
	}

	return p;
}




/* val is the value of the data at the voxel, vcheck is true if the
	pixel is inside the mask volume */
long int get_local(c_info *info,long int x,long int y, long int z,
	double *val,long int *vcheck)
{
	long int	i,j,k,l,count;
	double		*v;
	long int	*vc;
	
	count = 0;
	vc = vcheck;
	v = val;
	
	
	
	for(k=z;k<=(z+1);k++) {
	for(j=y;j<=(y+1);j++) {
	for(i=x;i<=(x+1);i++) {
			if ((i < 0) || (i >= info->dx) ||
			(j < 0) || (j >= info->dy) ||
			(k < 0) || (k >= info->icount)) {
			*v = 0; 
		} else { 
		
		
			l = i + (j*(info->dx)) + (k*(info->dx)*(info->dy));
			if (info->mask[l] != 0) {
				switch(info->dz) {
					case 1:
						*v = info->data[l];
						break;
					case 2:
						*v = info->sdata[l];
						break;
					case 4:
						*v = info->fdata[l];
						break;
				}
				
				*vc = 1;
			} else {
				*v = 0;
				*vc = 0;
				count++;
			}
		} 
		v++;
		vc++;
	}
	}
	}
/* if all points are illegal, then nothing is returned. */
	if (count == 8) return(0); 
/* valid point */
	return(1);
}

void ex_err(char *s)
{
	fprintf(stderr,"Fatal error:%s\n",s);
	tal_exit(1);
}

void cmd_err(char *s)
{
	fprintf(stderr,"(%s:%s) Usage: %s [options] inputtemp threshold\n",__DATE__,TAL_VERSION,s);
	fprintf(stderr,"Options: -x(dx) image dx default:256\n");
	fprintf(stderr,"         -y(dy) image dy default:256\n");
	fprintf(stderr,"         -z(dz) image depth in bytes default:2\n");
	fprintf(stderr,"         -f(start) first image number default:1\n");
	fprintf(stderr,"         -l(end) last image number default:119\n");
	fprintf(stderr,"         -i(step) image number step default:1\n");
	fprintf(stderr,"         -m(masktemp) template for a mask volume\n");
	fprintf(stderr,"         -b swap bytes with 16bit images default:no\n");
	fprintf(stderr,"         -r(:x:y:z) specify x, y, z, resel size in voxels:1.0:1.0:1.0\n");
	fprintf(stderr,"         -t(:start:stop:step) (test statistic threshold):1\n");
	fprintf(stderr,"         -df1(degrees) degrees of freedom 1 default:1\n");
	fprintf(stderr,"         -df2(degrees) degrees of freedom 2 default:1\n");
	fprintf(stderr,"         -Pzv Use gaussian field: default field\n");
	fprintf(stderr,"         -Ptv Use t field with v (df1) degress of freedom\n");
	fprintf(stderr,"         -Pfv Use F field with k (df1) and v (df2) degrees of freedom\n");
	fprintf(stderr,"         -Pxv Use a Chisqu field with v (df1) degrees of freedom\n");
	fprintf(stderr,"	 -c Use 2/3 correction factor for diamaters and surfaces\n");
	fprintf(stderr,"Note: Mask volumes must be 8Bit volumes.\n");
	fprintf(stderr,"Note: The threshold is used for defining which voxels constitute the small volume.\n\n");
	fprintf(stderr,"Note: For t and chi-square statistics, the df1 option should be used to specify the degrees of freedom.\n");
	fprintf(stderr,"For an F statistic, df1 and df2 specify the numerator and denominator degrees of freedom respectively.\n");
	tal_exit(1);
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
	long int	corr = 0;
	long int	dim = 0;
	double		thres, v;

	char		intemp[256],*mtemp;
	char		tstr[256];
	long int	i,err,j,k;
	long int	icount;
	unsigned char	*data,*tp;
	unsigned short	*mask;

	long int 	x,y,z;
	c_info  	c_data;
	double		val[8],t;
	long int	vcheck[8];
	double		e[3],f[3],p,c;
	double 		euler[4] = {0,0,0,0};
	double		resel[3] = {1.0,1.0,1.0};
	double		voxel[3] = {1.0,1.0,1.0};
	long int	bval[8];
	double		ftmp[3] = {1.0,1.0,1.0}; /* threshold value to convert to unified-P value*/
	double		d0, d1, d2, d3; /*EC densities for d dimensions*/
	double		vec[10];
	double 		df1 = 1.0;
	double		df2 = 1.0;
	double		unified_p;
	long int	action = ACT_Z2P; /*EC field type*/

	
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
			case 'b':
				swab = 1;
				break;
			case 'm':
				mtemp = &(argv[i][2]);
				break;
			case 'r':
				get_d_xyz(voxel,&(argv[i][3]));
				break;
			case 'c':
				corr = 1;
				break;
			case 't':
				get_d_xyz(ftmp,&(argv[i][3]));
				break;
			case 'P':
				if (strcmp(argv[i],"-Pxv") == 0) {
					action = ACT_C2P;
				} else if (strcmp(argv[i],"-Ptv") == 0) {
					action = ACT_T2P;
				} else if (strcmp(argv[i],"-Pzv") == 0) {
					action = ACT_Z2P;
				} else if (strcmp(argv[i],"-Pfv") == 0) {
					action = ACT_F2P;
				} else {
					cmd_err(argv[0]);
				}
				break;
			case 'd':
				if (strncmp(argv[i],"-df1",4) == 0) {
					df1 = atof(&(argv[i][4]));
				} else if (strncmp(argv[i],"-df2",4) == 0) {
					df2 = atof(&(argv[i][4]));
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
	strcpy(intemp,argv[i+0]);
	thres = atof(argv[i+1]);
	

/* get the data vector */
			vec[V_DF1] = df1;
			vec[V_DF2] = df2;

/* errors */
	if ((df1 < 1.0) || (df2 < 1.0)) ex_err("Invalid degrees of freedom (<1.0)");


/* first compute the characteristics or the resel count */

/* resel size is inverse of # of voxels in a resel */

	
/* to prevent an infinite loop for the start stop step of the threshold values 
* if ftmp[2] is 0, the value is set to 1 */
	if (ftmp[2] == 0) ftmp[2] = 1.0;
	

	resel[0] = 1/voxel[0];
	resel[1] = 1/voxel[1];
	resel[2] = 1/voxel[2];


/* count the number of images */
	icount = 0;
	for(i=istart;i<=iend;i=i+istep) icount++;
/* get the image memory */
	data = malloc(dz*dx*dy*icount);
	if (data == 0L) ex_err("Unable to allocate image memory.");
/* mask volume */
	mask = (unsigned short *)malloc(dx*dy*icount*sizeof(unsigned short));
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

/* begin the task at hand... */
	c_data.data = data;
	c_data.sdata = (unsigned short *)data;
	c_data.fdata = (float *)data;
	c_data.mask = mask;
	c_data.dx = dx;
	c_data.dy = dy;
	c_data.dz = dz;
	c_data.icount = icount;

	fprintf(stderr,"Computing Euler characteristic with a threshold of %.3f.\n",thres);

	for(z=-1;z<icount;z++) {
	for(y=-1;y<dy;y++) {
	for(x=-1;x<dx;x++) {
	
	if (get_local(&c_data,x,y,z,val,vcheck)) {
		i = 0;
		t=thres;
			p = 0;
			e[0] = 0;
			e[1] = 0;
			e[2] = 0;
			f[0] = 0;
			f[1] = 0;
			f[2] = 0;
			c = 0;
			for(j=0;j<8;j++) {
				if (vcheck[j] == 0) {
					bval[j] = 0;
				} else  {
					if ((val[j]) > t) {
						bval[j] = 1;
/* verticies */
						p += 1;
					} else {
						bval[j] = 0;
					}
				} 
			}
/* X edges */
			if (bval[0] && bval[1]) e[0] += 1;
			if (bval[2] && bval[3]) e[0] += 1;
			if (bval[6] && bval[7]) e[0] += 1;
			if (bval[4] && bval[5]) e[0] += 1;
			
/* Y edges */
			if (bval[0] && bval[2]) e[1] += 1;
			if (bval[1] && bval[3]) e[1] += 1;
			if (bval[4] && bval[6]) e[1] += 1;
			if (bval[5] && bval[7]) e[1] += 1;
			
/* Z edges */
			if (bval[0] && bval[4]) e[2] += 1;
			if (bval[2] && bval[6]) e[2] += 1;
			if (bval[3] && bval[7]) e[2] += 1;
			if (bval[1] && bval[5]) e[2] += 1;
			
/* faces */
/* xy face f[0]*/
			if (bval[0] && bval[1] && bval[2] && bval[3]) f[0] += 1;
			if (bval[4] && bval[5] && bval[6] && bval[7]) f[0] += 1;
/* yz face f[1] */			
			if (bval[0] && bval[4] && bval[6] && bval[2]) f[1] += 1;
			if (bval[1] && bval[5] && bval[7] && bval[3]) f[1] += 1;
/* xz face f[2] */			
			if (bval[2] && bval[6] && bval[7] && bval[3]) f[2] += 1;
			if (bval[0] && bval[4] && bval[5] && bval[1]) f[2] += 1;
			
/* cube */
			if (p == 8) c = 1;
			
			
			
		
			
/* add it to the euler vectors */ 
			
			
		euler[0] += (p/8.0)-((e[0]+e[1]+e[2])/4.0)+((f[0]+f[1]+f[2])/2.0)-c;
		euler[1] += ((e[0]/4.0)-((f[0]+f[2])/2.0)+c)*resel[0];
		euler[1] += ((e[1]/4.0)-((f[0]+f[1])/2.0)+c)*resel[1];
		euler[1] += ((e[2]/4.0)-((f[1]+f[2])/2.0)+c)*resel[2];
		euler[2] += ((f[0]/2)-c)*resel[0]*resel[1];
		euler[2] += ((f[1]/2)-c)*resel[1]*resel[2];
		euler[2] += ((f[2]/2)-c)*resel[0]*resel[2];
		euler[3] += c*resel[0]*resel[1]*resel[2];
		
			
			
		/*	i++; */
			
		
	}
	}
	}
	}


/* determine the maximum D, for later determining if the dfs are greater than the D */
	
	if (euler[3]!=0)
		{dim = 3;}
	else if ((euler[3]==0)&&(euler[2]!=0))
	 	{dim = 2;}
	else if ((euler[3]==0)&&(euler[2]==0)&&(euler[1]!=0))
		{dim = 1;}
	else	
		{dim = 0;}

/* report the results */
/*	i = 0; */
/*	t=thres; */
		fprintf(stdout,"%f\t%f\t%f\t%f\n\n",
			euler[0],euler[1],euler[2],euler[3]);
	
			
	/*	i++; */
	


/* now compute the p value for each dim of the user-specified field */

	v = ftmp[0];

switch(action) {
			case ACT_Z2P:
			
			/*fprintf(stdout,"The z value is %g\n",ftmp);*/
			for (;v<=ftmp[1];v+=ftmp[2]) {
			d0 = v;
			d1 = p1zv(1.0,v); 
			d2 = p2zv(1.0,v);
			d3 = pzv(1.0,v);
			action_proc(&(d0),ACT_Z2P,vec);
			/*fprintf(stdout,"%g\n",d0);*/
			/*fprintf(stdout,"%g\n",d1);*/
			/*fprintf(stdout,"%g\n",d2);*/
			/*fprintf(stdout,"%g\n",d3);*/
			/* now compute the unified P field; include 2/3 correction factor if specified by the user */

			if (corr == 0){
			unified_p = (d0 * euler[0]) + (d1 * euler[1]) + (d2 * euler[2]) + (d3 * euler[3]);
			fprintf(stdout,"Z value: %g\t unified p value:%g\n",v,unified_p);
			}
			else {
			unified_p = (d0 * euler[0]) + (d1 * euler[1])*2/3 + (d2 * euler[2])*2/3 + (d3 * euler[3]);
			fprintf(stdout,"Z value: %g\t unified p value: %g\n",v,unified_p);
	
			}
			}
			break;
			
			
			case ACT_T2P:
			if (df1 < dim){
			ex_err("Invalid degrees of freedom (df1<D), where D is the dimensionality of your search volume");
			}
			else {
			/*fprintf(stdout,"The t value is %g\n",ftmp);*/
			for (;v<=ftmp[1];v+=ftmp[2]) {
			d0 = v;
			d1 = p1tv(1.0,v,vec[V_DF1]); 
			d2 = p2tv(1.0,v,vec[V_DF1]);
			d3 = ptv(1.0,v,vec[V_DF1]);
			action_proc(&(d0),ACT_T2P,vec);
			/*fprintf(stdout,"%g\n",d0);*/
			/*fprintf(stdout,"%g\n",d1);*/
			/*fprintf(stdout,"%g\n",d2);*/
			/*fprintf(stdout,"%g\n",d3);*/
			/* now compute the unified P field; include 2/3 correction factor if specified by the user */

			if (corr == 0){
			unified_p = (d0 * euler[0]) + (d1 * euler[1]) + (d2 * euler[2]) + (d3 * euler[3]);
			fprintf(stdout,"t value: %g\t unified p value: %g\n",v,unified_p);
			}
			else {
			unified_p = (d0 * euler[0]) + (d1 * euler[1])*2/3 + (d2 * euler[2])*2/3 + (d3 * euler[3]);
			fprintf(stdout,"t value: %g\t unified p value: %g\n",v,unified_p);
	
			}
			}
			break;
			}

			case ACT_F2P:
			if ((df1 + df2) <= dim){
			ex_err("Invalid degrees of freedom ((df1+df2)<=D), where D is the dimensionality of your search volume");
			}
			else{
			/*fprintf(stdout,"The f value is %g\n",ftmp[0]);*/
			for (;v<=ftmp[1];v+=ftmp[2]) {
			d0 = v;
			d1 = p1fv(1.0,v,vec[V_DF1],vec[V_DF2]); 
			d2 = p2fv(1.0,v,vec[V_DF1],vec[V_DF2]);
			d3 = pfv(1.0,v,vec[V_DF1],vec[V_DF2]);
			action_proc(&(d0),ACT_F2P,vec);
			/*fprintf(stdout,"%g\n",d0);*/
			/*fprintf(stdout,"%g\n",d1);*/
			/*fprintf(stdout,"%g\n",d2);*/
			/*fprintf(stdout,"%g\n",d3);*/
			/* now compute the unified P field; include 2/3 correction factor if specified by the user */

			if (corr == 0){
			unified_p = (d0 * euler[0]) + (d1 * euler[1]) + (d2 * euler[2]) + (d3 * euler[3]);
			fprintf(stdout,"F value: %g\t unified p value: %g\n",v,unified_p);
			}
			else {
			unified_p = (d0 * euler[0]) + (d1 * euler[1])*2/3 + (d2 * euler[2])*2/3 + (d3 * euler[3]);
			fprintf(stdout,"F value: %g\t unified p value: %g\n",v,unified_p);
	
			}
			}
			break;
			}

			case ACT_C2P:
			/*fprintf(stdout,"The Chi-squared value is %g\n",ftmp);*/
			for (;v<=ftmp[1];v+=ftmp[2]) {
			d0 = v;
			d1 = p1xv(1.0,v,vec[V_DF1]); 
			d2 = p2xv(1.0,v,vec[V_DF1]);
			d3 = pxv(1.0,v,vec[V_DF1]);
			action_proc(&(d0),ACT_C2P,vec);
			/*fprintf(stdout,"%g\n",d0); */
			/*fprintf(stdout,"%g\n",d1); */
			/*fprintf(stdout,"%g\n",d2); */
			/*fprintf(stdout,"%g\n",d3); */
			/* now compute the unified P field; include 2/3 correction factor if specified by the user */

			if (corr == 0){
			unified_p = (d0 * euler[0]) + (d1 * euler[1]) + (d2 * euler[2]) + (d3 * euler[3]);
			fprintf(stdout,"Chi-square value: %g\t unified p value: %g\n",v,unified_p);
			}
			else {
			unified_p = (d0 * euler[0]) + (d1 * euler[1])*2/3 + (d2 * euler[2])*2/3 + (d3 * euler[3]);
			fprintf(stdout,"Chi-square value: %g\t unified p value: %g\n",v,unified_p);
	
			}
			}
			break;
			}


	

/* done */	
	free(mask);
	free(data);
	free(tp);
	tal_exit(0);
	exit(0);
}
