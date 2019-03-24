/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski
 *
 * $Id: tal_pca.c 1213 2005-08-27 20:51:21Z rjfrank $
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
#include <limits.h>
#include <time.h>
#include <string.h>
#include "proto.h"
#include "stats_inc.h"
#include "svd.h"

#define MIN(a,b) (((a)<(b)?(a):(b)))
#define MAX(a,b) (((a)>(b)?(a):(b)))
#define ABS(a)   (((a)<0)?-(a):(a))

/* verbose flag */
long int	verbose = 0;

#define MAX_OBS		200

/* local Prototypes */
void ex_err(char *s);
void cmd_err(char *s);
void pad_slice(float *data,long int dx,long int dy,long int xystep);
void make_test_data(void);

void ex_err(char *s)
{
	fprintf(stderr,"Fatal error:%s\n",s);
	tal_exit(1);
}

void cmd_err(char *s)
{
	fprintf(stderr,"(%s:%s) Usage: %s [options] ncomponents [-|vols1 vols2 ...]\n",__DATE__,TAL_VERSION,s);
	fprintf(stderr,"Options: -x(dx) image dx default:256\n");
	fprintf(stderr,"         -y(dy) image dy default:256\n");
	fprintf(stderr,"         -z(dz) image depth in bytes default:4\n");
	fprintf(stderr,"         -f(start) first image number default:1\n");
	fprintf(stderr,"         -l(end) last image number default:119\n");
	fprintf(stderr,"         -i(step) image number step default:2\n");
	fprintf(stderr,"         -b swap bytes with 16bit images default:no\n");
	fprintf(stderr,"         -s(skip) specify a step factor for the X,Y plane default:2\n");
	fprintf(stderr,"         -v verbose mode\n");
	fprintf(stderr,"         -c center columns before SVD (subtract mean) Default: not centered\n");
	fprintf(stderr,"         -o(offset) value added to each pixel default:0\n");
	fprintf(stderr,"         -O(num):(template) save eigen vector volume (num) to disk\n");
	fprintf(stderr,"         -m(masktemp) filename template for a mask volume\n");
	fprintf(stderr,"         -p(projfile) file where the projection data are saved Default:stdout\n");
	fprintf(stderr,"If '-' is used for the input volume, the template names are read from stdin.\n");
	fprintf(stderr,"Note:mask volumes must be 8bit.\n");
	tal_exit(1);
}

int main(int argc,char **argv)
{
/* cmdline parameters */
	long int	dx = 256;
	long int	dy = 256;
	long int	dz = 4;
	long int	istart = 1;
	long int	iend = 119;
	long int	istep = 2;
	long int	xystep = 2;
	long int	swab = 0;
	long int	center = 0;
	double		offset = 0;
	char		*masktemp = 0L;
	char		*projfile = 0L;

	char		tstr[256],*tp;
	long int	i,err,j,k,x,y;
	long int	nobs,n_components,np,job;
	long int	scount[512],idx,base;
	char		vols[MAX_OBS][256];
	char		*ovols[MAX_OBS];
	unsigned char	*buf,*mask;
	float		v,*fbuf;
	FILE		*projfp = stdout;

/* real work arrays */
	float		*U,*X,*work,*S,*E;
	float		*centers;

/* dummy params */
	float		V[1];
	long int	vs = 1;
	
/* clear out outputs */
	for(i=0;i<MAX_OBS;i++) ovols[i] = 0L;

/* parse the command line */
	nobs = 0;
	i = 1;
	while ((i < argc) && (argv[i][0] == '-')) {
		switch (argv[i][1]) {
			case '\0':
				break;
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
				xystep = atoi(&(argv[i][2]));
				break;
			case 'o':
				offset = atof(&(argv[i][2]));
				break;
			case 'O':
				idx = atoi(&(argv[i][2]));
				tp = strchr(argv[i],':');
				if ((idx < MAX_OBS) && (tp != 0L)) {
					ovols[idx-1] = tp + 1;
				} else {
					fprintf(stderr,
				"Invalid output volume selected: %ld:%s",
						idx,argv[i]);
					cmd_err(argv[0]);
				}
				break;
			case 'b':
				swab = 1;
				break;
			case 'c':
				center = 1;
				break;
			case 'v':
				verbose = strlen(argv[i])-1;
				break;
			case 'm':
				masktemp = &(argv[i][2]);
				break;
			case 'p':
				projfile = &(argv[i][2]);
				projfp = fopen(projfile,"w");
				if (projfp == 0L) {
		ex_err("Unable to open projection file");
				}
				break;
			case 'T':
				make_test_data();
				break;
			default:	
				cmd_err(argv[0]);
				break;
		}
		i++;
	}
/* get the number of subjects */
	if (i < argc) {
		n_components = atoi(argv[i]);
		if (n_components < 1) {
			ex_err("More than one component must be requested");
		}
		i++;
	} else {
		cmd_err(argv[0]);
		exit(0);
	}
/* get the volume names */
	if (i >= argc) cmd_err(argv[0]);
	if (strcmp(argv[i],"-") == 0L) {
/* stdio */
		while (fgets(tstr,256,stdin)) {
			sscanf(tstr,"%s",vols[nobs]);
			nobs++;
	if (nobs == MAX_OBS) ex_err("Maximum number of volumes exceeded");
		}
	} else {
/* cmdline */
		while (i < argc) {
			strcpy(vols[nobs],argv[i]);
			nobs++;
	if (nobs == MAX_OBS) ex_err("Maximum number of volumes exceeded");
			i++;
		}
	}
	if (nobs < n_components) {
		ex_err("Number of input volumes must exceed the number of requested components");
	}

/* allocate memory for the images */
	buf = malloc(dx*dy*sizeof(float));
	if (buf == 0) ex_err("Unable to allocate image buffer memory");
	fbuf = (float *)buf;
	mask = malloc(dx*dy);
	if (mask == 0) ex_err("Unable to allocate image buffer memory");
	for(i=0;i<dx*dy;i++) mask[i] = 1;  /* init to 1 */

/* count the number of vars in the mask */
	np = 0;
	for(i=istart;i<=iend;i=i+istep) {
/* read the mask */
		if (masktemp) {
			name_changer(masktemp,i,&err,tstr);
			fprintf(stderr,"Reading mask file:%s\n",tstr);
			bin_io(tstr,'r',mask,dx,dy,1,0,swab,0L);
		} 
		scount[i] = 0;
		for(y=0;y<dy;y+=xystep) {
		for(x=0;x<dx;x+=xystep) {
			k = x + (y*dx);
			if (mask[k]) {
				np += 1;
				scount[i] += 1;
			}
		}
		}
	}
	fprintf(stderr,"Working on %ld locations for %ld observations.\n",
		np,nobs);
/* memory = nobs*nvars*4 */
	X = (float *)malloc(np*nobs*sizeof(float));
	if (X == 0) ex_err("Unable to allocate observation memory");
	U = (float *)malloc(np*nobs*sizeof(float));
	if (U == 0) ex_err("Unable to allocate Eigen vector memory");

	work = (float *)malloc(np*sizeof(float));
	if (work == 0) ex_err("Unable to allocate workspace memory");
	S = (float *)malloc(MIN(np+1,nobs)*sizeof(float));
	if (S == 0) ex_err("Unable to allocate Eigen value memory");
	E = (float *)malloc(nobs*sizeof(float));
	if (E == 0) ex_err("Unable to allocate error memory");

/* get the input data */
	idx = 0;
	for(i=istart;i<=iend;i=i+istep) {
/* read the mask */
		if (masktemp) {
			name_changer(masktemp,i,&err,tstr);
			fprintf(stderr,"Reading mask file:%s\n",tstr);
			bin_io(tstr,'r',mask,dx,dy,1,0,swab,0L);
		} 
/* read the observations */
		for(j=0;j<nobs;j++) {
			name_changer(vols[j],i,&err,tstr);
			fprintf(stderr,"Reading the file:%s\n",tstr);
			bin_io(tstr,'r',buf,dx,dy,dz,0,swab,0L);
/* add observations (masked) (obs = j) */
			base = (np*j)+idx;
			for(y=0;y<dy;y+=xystep) {
			for(x=0;x<dx;x+=xystep) {
			    k = x + (y*dx);
			    if (mask[k]) {
				switch (dz) {
					case 1:
						v = ((unsigned char *)buf)[k];
						break;
					case 2:
						v = ((unsigned short *)buf)[k];
						break;
					case 4:
						v = ((float *)buf)[k];
						break;
				}
/* add it */
				X[base++] = v + offset;
			    }
			}
			}
		}
		idx += scount[i];
	}

/* centering? */
	if (center) {
		centers = (float *)malloc(np*sizeof(float));
		if (centers == 0) ex_err("Unable to allocate centering memory");
/* compute the mean */
		for(i=0;i<np;i++) {
			centers[i] = 0.0;
			for(j=0;j<nobs;j++) {
				centers[i] += X[(np*j)+i];
			}
			centers[i] /= (float)(nobs);
/* subtract the mean */
			for(j=0;j<nobs;j++) X[(np*j)+i] -= centers[i];
		}
	}

/* call the SVD to generate Eigen vectors and Eigen values */
	fprintf(stderr,"Computing the SVD of %ld observations and %ld values\n",
		nobs,np);
	job = 20;
	ssvdc_(X,&np,&np,&nobs,S,E,U,&np,V,&vs,work,&job,&err);

/* balance out eigen values */
	v = S[0]*S[0];
	for(i=1;i<n_components;i++) v += (S[i]*S[i]);
	for(i=0;i<n_components;i++) S[i]=((S[i]*S[i])/v)*((float)n_components);

/* output Eigen values */
	fprintf(stdout,"%f",S[0]);
	for(i=1;i<n_components;i++) fprintf(stdout,"\t%f",S[i]);
	fprintf(stdout,"\n");

	idx = 0;
	for(i=istart;i<=iend;i=i+istep) {
/* read the mask */
		if (masktemp) {
			name_changer(masktemp,i,&err,tstr);
			fprintf(stderr,"Reading the file:%s\n",tstr);
			bin_io(tstr,'r',mask,dx,dy,1,0,swab,0L);
		} 

/* for each Eigen vector */
		for(j=0;j<nobs;j++) {
/* output Eigen vector volumes */
			if (ovols[j]) {
				for(k=0;k<dx*dy;k++) fbuf[k] = 0.0;
				base = (np*j)+idx;
				for(y=0;y<dy;y+=xystep) {
				for(x=0;x<dx;x+=xystep) {
			    		k = x + (y*dx);
			    		if (mask[k]) {
						fbuf[k] = U[base++];

if (verbose > 2) {
	fprintf(stderr,"Eigenvector %ld = %f\n",j+1,fbuf[k]);
}

					}
				}
				}
/* pad the slice (repeat the pixels) */
				pad_slice(fbuf,dx,dy,xystep);
/* repeat the slice */
				for(k=i;k<i+istep;k++) {
				name_changer(ovols[j],k,&err,tstr);
				fprintf(stderr,"Writing the file:%s\n",tstr);
				bin_io(tstr,'w',fbuf,dx,dy,sizeof(float),0,
					swab,0L);
				}
			}

/* reload the X array (for projections) */

/* read the observations */
			name_changer(vols[j],i,&err,tstr);
			fprintf(stderr,"Reading the file:%s\n",tstr);
			bin_io(tstr,'r',buf,dx,dy,dz,0,swab,0L);
/* add observations (masked) (obs = j) */
			base = (np*j)+idx;
			for(y=0;y<dy;y+=xystep) {
			for(x=0;x<dx;x+=xystep) {
			    k = x + (y*dx);
			    if (mask[k]) {
				switch (dz) {
					case 1:
						v = ((unsigned char *)buf)[k];
						break;
					case 2:
						v = ((unsigned short *)buf)[k];
						break;
					case 4:
						v = ((float *)buf)[k];
						break;
				}
/* add it */
				X[base++] = v + offset;
			    }
			}
			}
		}
/* next slice */
		idx += scount[i];
	}

/* centering? */
	if (center) {
		for(i=0;i<np;i++) {
/* subtract the mean */
			for(j=0;j<nobs;j++) X[(np*j)+i] -= centers[i];
		}
	}

/* compute and output projections */
	for(i=0;i<nobs;i++) {
		for(j=0;j<n_components;j++) {
			work[j] = 0;
			for(k=0;k<np;k++) {
				work[j] += (U[(j*np)+k]*X[(i*np)+k]);
			}
		}
		fprintf(projfp,"%f",work[0]);
		for(j=1;j<n_components;j++) fprintf(projfp,"\t%f",work[j]);
		fprintf(projfp,"\n");
	}



/* Done ! */
	if (projfp != stdout) fclose(projfp);

	if (center) free(centers);

	free(buf);
	free(mask);

	free(U);
	free(X);
	free(S);
	free(E);
	free(work);

/* bye */
	tal_exit(0);

	exit(0);
}


void pad_slice(float *data,long int dx,long int dy,long int xystep)
{
	long int	x,y,i,j;
	
	for(y=0;y<dy;y+=xystep) {
	for(x=0;x<dx;x+=xystep) {
		for(j=y;j<y+xystep;j++) {
		for(i=x;i<x+xystep;i++) {
			data[i+(j*dx)] = data[x+(y*dx)];
		}
		}
	}
	}
	return;
}

#define NP 10
#define NOBS 4

void make_test_data()
{
	float		fdata[NOBS][NP];
	long int	i,j,err;
	char		tstr[256];
	FILE		*fp;

	for(i=1;i<=NP;i++) {
		for(j=1;j<=NOBS;j++) {
			fdata[j-1][i-1] = 1.0/((float)(i+j));
		}
	}
	for(j=1;j<=NOBS;j++) {
		name_changer("pca_%%%.flt",j,&err,tstr);
		printf("Writing:%s\n",tstr);
		fp = fopen(tstr,"w");
		if (fp) {
			fwrite(fdata[j-1],sizeof(float),NP,fp);
			fclose(fp);
		}
	}

	exit(0);
}

/* extra function to aid svd.f when f2c is used */
double   r_sign(float *a,float *b)
{
        if (*b < 0) return(-(*a));

        return(*a);
}

