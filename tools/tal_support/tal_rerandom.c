/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski
 *
 * $Id: tal_rerandom.c 1213 2005-08-27 20:51:21Z rjfrank $
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
#ifndef WIN32
#include <unistd.h>
#else
#define srand48 srand
#define lrand48 rand
#endif
#include <sys/types.h>
#include <sys/stat.h>

#include "proto.h"
#include "stats_inc.h"

#define MAX_VOLS	200

#define VOL_SUM 0
#define VOL_SS 1
#define VOL_T 2
#define VOL_VAR 3
#define VOL_SVAR 4
#define VOL_MAX 5

#define MIN(a,b) (((a)<(b)?(a):(b)))
#define MAX(a,b) (((a)>(b)?(a):(b)))
#define ABS(a)   (((a)<0)?-(a):(a))

#define FIXED_16MULT 512
#define FIXED_16MIN -32767
#define FIXED_16MAX 32767

/* kernal params */
double		*ktab = 0;
double		ksum = 0;
long int	kdx = 1;
long int	kdy = 1;
long int	kdz = 1;

/* verbose flag */
long int	verbose = 0;

/* globals to hold the list of rerandomizations to try */
#define MAX_RANDOMIZATIONS 10000L

long int	n_random = 0;
long int	sample_mode = 0;
unsigned char	*rand_list[MAX_RANDOMIZATIONS];

/* local Prototypes */
void dealloc_list(void);
void alloc_list_entry(long int size);
void make_rerandom_list(long subs,long maxnum);
long int rand_limit(long int m);
void ex_err(char *s);
void read_kernal(char *file);
void cmd_err(char *s);
void add_slice(char *temp,long int sl,void *data,long int dx,long int dy,
	long int dz,long int kdz,long int swab,long int mask,double offset);
void shiftvolume(void *data,long int dx,long int dy,long int dz,long int kdz);
void New_Slice(float *data,short *slice,long int dx,long int dy,
	long int kdz,unsigned char AB,long int power);
int N_Choose_M(long int *cnt,long int n,long int m,long int *arr);
void Convolve(float *in,float *out,unsigned char *mask,
		long int dx,long int dy);
long int nPm(long int n,long int m);
long int nCm(long int n,long int m);
void dump_histo(char *histfile,double histmin,double histmax,double histsize,
	long int histbins,long int histkeep,double *hist,
	double *Tmax,long int n_random);
void	dump_tmax(char *tmaxfile,double *Tmax,long int n_random,
	long int iteration);
void	dump_stats(char *statsfile,char *tmaxfile,double *Tmax,
	long int n_random,long int iteration,long int abs_tmax,
	long int t,long int s_time);
int	cmp_dbl(const void *a,const void *b);
void image_2_fixed16(void *in,long int dx,long int dy,long int dz,
        double offset,short *out);
void checkpoint(char *checkfile,int argc,char **argv,double *Tmax,
	long int *slice,long int nvols,char svols[MAX_VOLS][256],
	long int *s_time);

void ex_err(char *s)
{
	fprintf(stderr,"Fatal error:%s\n",s);
	tal_exit(1);
}

void read_kernal(char *file)
{
	FILE		*fp;
	long int	i,j,k,l;

	fp = fopen(file,"r");
	if (fp == 0) {
		fprintf(stderr,"Unable to open %s\n",file);
		return;
	}
	l = fscanf(fp,"%ld %ld %ld",&i,&j,&k);
	if (l == 3) {
		ktab = (double *)malloc(sizeof(double)*i*j*k);
		if (ktab == 0L) ex_err("Memory allocation error");
		ksum = 0;
		for(l=0;l<(i*j*k);l++) {
			fscanf(fp,"%lf",&(ktab[l]));
			ksum += ktab[l];
		}
		kdx = i;
		kdy = j;
		kdz = k;
	} else {
		fprintf(stderr,"Invalid kernal file format\n");
	}
	fclose(fp);
	return;
}

void cmd_err(char *s)
{
	fprintf(stderr,"(%s:%s) Usage: %s [options] nsubjects [-|vols1 vols2 ...]\n",__DATE__,TAL_VERSION,s);
	fprintf(stderr,"Options: -x(dx) image dx default:256\n");
	fprintf(stderr,"         -y(dy) image dy default:256\n");
	fprintf(stderr,"         -z(dz) image depth in bytes default:4\n");
	fprintf(stderr,"         -f(start) first image number default:1\n");
	fprintf(stderr,"         -l(end) last image number default:119\n");
	fprintf(stderr,"         -i(step) image number step default:1\n");
	fprintf(stderr,"         -b swap bytes with 16bit images default:no\n");
	fprintf(stderr,"         -s(seed) specify a seed for the random sample generator default:1\n");
	fprintf(stderr,"         -R(file) specify the base filename for checkpointing default:none\n");
	fprintf(stderr,"         -M(maxtrials) set the max number of randomizations default:%ld\n",1000L);
	fprintf(stderr,"         -v verbose mode\n");
	fprintf(stderr,"         -V(outvolume) T volume template for correct AB mapping. default:none\n");
	fprintf(stderr,"         -W(outvolume) variance vol temp for correct AB mapping. default:none\n");
	fprintf(stderr,"         -Oabs use absolute values when computing Tmax default:direct value\n");
	fprintf(stderr,"         -Otmax(file) file to output the Tmax distribution to default:none\n");
	fprintf(stderr,"         -Ostats(file) file to output stats to default:stdout\n");
	fprintf(stderr,"         -o(offset) value added to each pixel default:0\n");
	fprintf(stderr,"         -Hfile(histofile) file to save Tmax histogram in default:none\n");
	fprintf(stderr,"         -Hmin(minval) minimum histogram bin center value default:-10.0\n");
	fprintf(stderr,"         -Hmax(maxval) maximum histogram bin center value default:10.0\n");
	fprintf(stderr,"         -Hsize(binsize) size (width) of a histogram bin default:0.1\n");
	fprintf(stderr,"         -Hkeep map pixels outside min/max to min/max bins default:drop them\n");
	fprintf(stderr,"         -m(masktemp) filename template for a mask volume\n");
	fprintf(stderr,"         -k(kernalfile) filename of a kernal used for smoothing of var\n");
	fprintf(stderr,"output =  stats of (p+offset)\n");
	fprintf(stderr,"If '-' is used for the input volume, the template names are read from stdin.\n");
	fprintf(stderr,"Note:mask volumes must be 8bit.\n");
	tal_exit(1);
}

void	add_slice(char *temp,long int sl,void *data,long int dx,long int dy,
	long int dz,long int kdz,long int swab,long int mask,double offset)
{
	unsigned char	*tbuf;
	char		tstr[256];
	long int	i,err;
	unsigned char	*p1 = (unsigned char *)data;

/* move the data down and clear out this slice */
	if (mask) {
		shiftvolume(data,dx,dy,1,kdz);
	} else {
		shiftvolume(data,dx,dy,2,kdz);
	}

/* if the filename is null, then this is a null mask template */
/* we return a TRUE image in this case */
	if ((strcmp(temp,"+") == 0L) && (mask == 1)) {
		for(i=0;i<(dx*dy);i++) p1[i] = 1;
		return;
	}

/* Read the file */
	name_changer(temp,sl,&err,tstr);
	fprintf(stderr,"Reading the file:%s\n",tstr);
	if (mask) {
		bin_io(tstr,'r',p1,dx,dy,dz,0,swab,0L);
	} else {
		tbuf = malloc(dx*dy*dz);
		if (tbuf == 0L) ex_err("No memory for image buffering");
		bin_io(tstr,'r',tbuf,dx,dy,dz,0,swab,0L);
/* convert from disk form into FIXED form */
		image_2_fixed16(tbuf,dx,dy,dz,offset,(short *)p1);

		free(tbuf);
	}

	return;
}

void	shiftvolume(void *data,long int dx,long int dy,long int dz,long int kdz)
{
	long int 	i;
	unsigned char	*p1 = (unsigned char *)data;

/* push slices down */
	for(i=((dx*dy*dz)*(kdz-1))-1;i>=0;i--) p1[i+(dx*dy*dz)] = p1[i];

/* clear out the top slice */
	if (dz == 1) {
		for(i=0;i<(dx*dy);i++) ((unsigned char *)data)[i] = 0;
	} else if (dz == 2) {
		for(i=0;i<(dx*dy);i++) ((unsigned short *)data)[i] = 0;
	} else if (dz == 4) {
		for(i=0;i<(dx*dy);i++) ((float *)data)[i] = 0.0;
	}
	return;
}

int main(int argc,char **argv)
{
/* cmdline parameters */
	unsigned int 	randseed = 1;
	long int	dx = 256;
	long int	dy = 256;
	long int	dz = 4;
	long int	istart = 1;
	long int	iend = 119;
	long int	istep = 1;
	long int	swab = 0;
	long int	maxnum = 1000;
	double		offset = 0;
	double		histmin = -10.0;
	double		histmax = 10.0;
	double		histsize = 0.1;
	long int	histkeep = 0;
	long int	abs_tmax = 0;
	char		*outtemp = NULL;
	char		*outvar = NULL;
	char		*tmaxfile = NULL;
	char		*statsfile = NULL;
	long int	n_subjects,num_r;
	char		*restartfile = NULL;

	char		svols[MAX_VOLS][256];
	short		*svdata[MAX_VOLS];  /* subject data */
	char		masktemp[256];
	char		tstr[256];
	long int	i,err,j,k,r;
	long int	nvols,histbins;
	long int	sliceskip;
	
	float		*data[VOL_MAX];  /* work arrays */
	unsigned char	*maskimage;
	char		*histfile = 0L;
	double		d,*hist = 0L;
	double		*Tmax;
	long int	slicecount,f;
	long int	l_time,s_time;
	double		ss,mn;

/* init some strings */
	strcpy(masktemp,"+");

/* parse the command line */
	nvols = 0;
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
				randseed = atoi(&(argv[i][2]));
				break;
			case 'o':
				offset = atof(&(argv[i][2]));
				break;
			case 'M':
				maxnum = atof(&(argv[i][2]));
				break;
			case 'b':
				swab = 1;
				break;
			case 'v':
				verbose = strlen(argv[i])-1;
				break;
			case 'k':
				read_kernal(&(argv[i][2]));
				break;
			case 'm':
				strcpy(masktemp,&(argv[i][2]));
				break;
			case 'R':
				restartfile = &(argv[i][2]);
				break;
			case 'V':
				outtemp = &(argv[i][2]);
				break;
			case 'W':
				outvar = &(argv[i][2]);
				break;
			case 'H':
				if (strncmp("-Hfile",argv[i],6) == 0) {
					histfile = &(argv[i][6]);
				} else if (strncmp("-Hmin",argv[i],5) == 0) {
					histmin = atof(&(argv[i][5]));
				} else if (strncmp("-Hmax",argv[i],5) == 0) {
					histmax = atof(&(argv[i][5]));
				} else if (strncmp("-Hsize",argv[i],6) == 0) {
					histsize = atof(&(argv[i][6]));
				} else if (strncmp("-Hkeep",argv[i],6) == 0) {
					histkeep = 1;
				} else {
					cmd_err(argv[0]);
				}
				break;
			case 'O':
				if (strncmp("-Otmax",argv[i],6) == 0) {
					tmaxfile = &(argv[i][6]);
				} else if (strncmp("-Ostats",argv[i],7) == 0) {
					statsfile = &(argv[i][7]);
				} else if (strncmp("-Oabs",argv[i],5) == 0) {
					abs_tmax = 1;
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
/* get the number of subjects */
	if (i < argc) {
		n_subjects = atoi(argv[i]);
		i++;
	} else cmd_err(argv[0]);
/* get the volume names */
	if (i >= argc) cmd_err(argv[0]);
	if (strcmp(argv[i],"-") == 0L) {
/* stdio */
		while (fgets(tstr,256,stdin)) {
			sscanf(tstr,"%s",svols[nvols]);
			nvols++;
	if (nvols == MAX_VOLS) ex_err("Maximum number of volumes exceeded");
		}
	} else {
/* cmdline */
		while (i < argc) {
			strcpy(svols[nvols],argv[i]);
			nvols++;
	if (nvols == MAX_VOLS) ex_err("Maximum number of volumes exceeded");
			i++;
		}
	}
	if (nvols != n_subjects) {
		ex_err("Number of input volumes does not equal number of subjects");
	}

	s_time = 0;
/* compute number of iterations */
	srand48(randseed);
	make_rerandom_list(n_subjects,maxnum);

/* allocate and init the structs */
	Tmax = (double *)malloc(n_random*sizeof(double));
	if (Tmax == NULL) ex_err("Unable to allocate memory for Tmax");
	if (abs_tmax) {
		for(i=0;i<n_random;i++) Tmax[i] = 0.0;
	} else {
		for(i=0;i<n_random;i++) Tmax[i] = -1.0e+20;
	}

/* fix up a kernal if none was specified */
	if (ktab == NULL) {
		kdx = 1;
		kdy = 1;
		kdz = 1;
		ktab = (double *)malloc(sizeof(double)*kdx*kdy*kdz);
		if (ktab == 0L) ex_err("Memory allocation error");
		ktab[0] = 1.0;
		ksum = 1.0;
	}

/* if restartfile was specified */
	sliceskip = -1000;
	if (restartfile != 0L) {
		checkpoint(restartfile,0L,0L,Tmax,&sliceskip,nvols,svols,
			&s_time);
	}
	
/* report to user */
	if (verbose) {
		fprintf(stdout,"Number of subjects: %ld\n",n_subjects);
		fprintf(stdout,"Number of rerandomizations: %ld\n",n_random-1);
		fprintf(stdout,"Computing for the following volumes:\n");
		for(i=0;i<nvols;i++) fprintf(stdout,"%s\n",svols[i]);
		if (ktab != 0) {
			fprintf(stdout,"Smoothed Variance\n");
			fprintf(stdout,"Kernal: %ldx%ldx%ld sum=%f\n",
				kdx,kdy,kdz,ksum);
			err = 0;
			for(k=0;k<kdz;k++) {
			for(j=0;j<kdy;j++) {
				for(i=0;i<kdx;i++) {
					fprintf(stdout,"%.3f ",ktab[err]);
					err++;
				}
				fprintf(stdout,"\n");
			}
			}
			fprintf(stdout,"\n");
		} else {
			fprintf(stdout,"Unsmoothed Variance\n");
		}
		fprintf(stdout,"Data offset=%f\n",offset);
		if (strcmp(masktemp,"+") == 0) {
			fprintf(stdout,"No masking used\n");
		} else {
			fprintf(stdout,"Mask used:%s\n",masktemp);
		}
		if (histfile != 0) {
			fprintf(stdout,"Histogram output to:%s\n",histfile);
			fprintf(stdout,"Min=%f max=%f Binsize=%f\n",
				histmin,histmax,histsize);
			if (histkeep) {
	fprintf(stdout,"Values outside min/max are retained in outer bins\n");
			} else {
	fprintf(stdout,"Values outside min/max are not recorded\n");
			}
		} else {
			fprintf(stdout,"No histogram generated\n");
		}
		if (outtemp != 0) {
			fprintf(stdout,"Output T0 volume:%s\n",outtemp);
		}
		if (outvar != 0) {
			fprintf(stdout,"Output T0 variance volume:%s\n",outvar);
		}
	}

/* get the histogram memory */
	hist = 0L;
	if (histfile != 0L) {
		histbins = 1 + ((histmax - histmin)/histsize);
		if (histbins <= 1) {
			fprintf(stderr,"Invalid histogram min/max/size values.  Histogram will not be computed.\n");
			histfile = 0L;
		} else {
			hist = (double *)malloc(histbins*sizeof(double));
		}
	}

/* get memory for the volumes (VOL_SUM),(VOL_SS),(VOL_T),(VOL_VAR) */
	for(i=0;i<VOL_MAX;i++) {
		data[i] = (float *)malloc(sizeof(float)*dx*dy*kdz);
		if (data[i] == 0L) ex_err("Unable to allocate image memory.");
	}
/* get memory for the subject volumes */
	for(i=0;i<nvols;i++) {
		svdata[i] = (short *)malloc(sizeof(short)*dx*dy*kdz);
		if (svdata[i] == 0L) ex_err("Unable to allocate image memory.");
	}
/* get mask volume memory (8bit) */
	maskimage = malloc(dx*dy*kdz);
	if (maskimage == 0L) ex_err("Unable to allocate image memory.");

/* clear out all the data volumes to zero (especially the float case) */
	for(j=0;j<VOL_MAX;j++) {
		for(i=0;i<(dx*dy*kdz);i++) data[j][i] = 0.0;
	}
	for(j=0;j<nvols;j++) {
		for(i=0;i<(dx*dy*kdz);i++) svdata[j][i] = 0;
	}
	for(i=0;i<(dx*dy*kdz);i++) ((unsigned char *)maskimage)[i] = 0;
/* all volumes are clear */

/* for all slices in the volumes */
	slicecount = 0;
	for(f=istart;f<=(iend+(kdz/2)*istep);f+=istep) {
		slicecount += 1;

		l_time = time(NULL);

/* load the data */
		if (f>iend) {
/* if past the last slice (smoothing) just shift the stacks */
/* don't bother reading the slices or writing the var slices */
			for(i=0;i<nvols;i++) {
				shiftvolume(svdata[i],dx,dy,2,kdz);
			}
			shiftvolume(maskimage,dx,dy,1,kdz);
		} else {
/* read the mask slice */
			add_slice(masktemp,f,maskimage,dx,dy,1,kdz,
				swab,1,offset);
/* read the subject images */
			for(i=0;i<nvols;i++) {
			   	add_slice(svols[i],f,svdata[i],dx,dy,dz,kdz,
					swab,0,offset);
			}
		}

/* check restart */
		if ((restartfile) && (sliceskip >= f)) continue;

/* if deep enough (kdz images), do the work for all the chosen rerandoms */
		if (slicecount > (kdz/2)) {

/* for all rerandomizations */
		if (sample_mode) {
			num_r = n_random;
		} else {
/* (r - T0)/2 + T0 */
			num_r = ( (n_random-1)/2 ) + 1;
		}
		for(r=0;r<num_r;r++) {

/* calc SUM and SS slices */
			if (r == 0) {
/* SS only once */
			for(k=0;k<dx*dy*kdz;k++) {
				data[VOL_SS][k] = 0.0;
			}
			for(i=0;i<nvols;i++) {
				New_Slice(data[VOL_SS],svdata[i],dx,dy,
					kdz,rand_list[r][i],2);
			}
			}

/* SUM for every randomization */
			for(k=0;k<dx*dy*kdz;k++) {
				data[VOL_SUM][k] = 0.0;
			}
			for(i=0;i<nvols;i++) {
				New_Slice(data[VOL_SUM],svdata[i],dx,dy,
					kdz,rand_list[r][i],1);
			}

/* calc VAR slices (VAR*VAR)=(SS/n) - ((SUM/n)*(SUM/n)) */
			for(k=0;k<dx*dy*kdz;k++) {
				if (maskimage[k] != 0) {
					ss = data[VOL_SS][k]/(double)(nvols);
					mn = data[VOL_SUM][k]/(double)(nvols);
					data[VOL_VAR][k] = ss - (mn*mn);
				} else {
					data[VOL_VAR][k] = 0.0;
				}
			}

/* if deep enough (kdz images), smooth VAR to SVAR and calculate Tmax value! */
			i = (kdz/2)*(dx*dy);  /* offset to center slice */
if (verbose > 1) {
	fprintf(stderr,"Smoothing variance case %ld...\n",r);
}
/* smooth it */
			Convolve(data[VOL_VAR],data[VOL_SVAR],maskimage,dx,dy);

/* Tmax = (SUM/n)/sqrt(VARs*VARs/n)   df=n-1 */
			for(k=0;k<dx*dy;k++) {
			if (maskimage[k+i] != 0) {

#ifdef DEBUG
	printf("VOL_SUM,VOL_SS,VOL_VAR,VOL_SVAR=%f %f %f %f\n",
data[VOL_SUM][k+i],data[VOL_SS][k+i],data[VOL_VAR][k+i],data[VOL_SVAR][k+i]);
#endif
			        mn=data[VOL_SUM][k+i]/(double)(nvols);
				ss=data[VOL_SVAR][k+i]/(double)(nvols);
/* compute the Tmax candidate */
				if (ss > 0) {
					ss = sqrt(ss);
					d = mn/ss;
				} else {
					d = 0;
				}
				data[VOL_T][k+i] = d;
/* record the Tmax Value */
				if (abs_tmax) {
				    if (ABS(d) > ABS(Tmax[r])) {
					Tmax[r] = d;
				    }
				} else {
				    if (d > Tmax[r]) {
					Tmax[r] = d;
				    }
				}
/* do the 1-a side if we are in complete mode */
				if ((!sample_mode) && (r > 0)) {
					j = n_random-r;
					if (abs_tmax) {
				    		if (ABS(-d) > ABS(Tmax[j])) {
							Tmax[j] = -d;
				    		}
					} else {
				    		if (-d > Tmax[j]) {
							Tmax[j] = -d;
				    		}
					}
				}
/* if !mask */
			} else {
				data[VOL_T][k+i] = 0;
			}

			} /* next pixel */

if (verbose > 2) {
	fprintf(stderr,"Tmax(%ld)=%f\n",r,Tmax[r]);
	if ((!sample_mode) && (r > 0)) {
		j = n_random-r;
		fprintf(stderr,"Tmax(%ld)=%f\n",j,Tmax[j]);
	}
}

/* save the new Tmax slice */
			if ((outtemp != NULL) && (r == 0)) {
				name_changer(outtemp,f-(istep*(kdz/2)),&k,tstr);
				fprintf(stderr,"Writing the file:%s\n",tstr);
				bin_io(tstr,'w',&(data[VOL_T][i]),dx,dy,4,
					0,swab,0L);
			}
			if ((outvar != NULL) && (r == 0)) {
				name_changer(outvar,f-(istep*(kdz/2)),&k,tstr);
				fprintf(stderr,"Writing the file:%s\n",tstr);
				bin_io(tstr,'w',&(data[VOL_SVAR][i]),dx,dy,4,
					0,swab,0L);
			}

		}  /* next r */

		}  /* slicecount */

		l_time = time(NULL) - l_time;  /* how long for a slice */
		s_time += l_time;

/* save the current Tmax histogram!!! */
	dump_histo(histfile,histmin,histmax,histsize,histbins,histkeep,hist,
		Tmax,n_random);

/* save the current stats */
	dump_stats(statsfile,tmaxfile,Tmax,n_random,n_random,abs_tmax,
		l_time,s_time);

/* check point the system */
	checkpoint(restartfile,argc,argv,Tmax,&f,nvols,svols,&s_time);

	}  /* next slice */


/* free up the memory */
	for(i=0;i<VOL_MAX;i++) {
		free(data[i]);
	}
	for(i=0;i<nvols;i++) {
		free(svdata[i]);
	}
	free(maskimage);
	if (ktab != 0) free(ktab);
	if (hist != 0) free(hist);
	free(Tmax);
	dealloc_list();

/* that's all she wrote */
	tal_exit(0);

	exit(0);
}

void New_Slice(float *data,short *slice,long int dx,long int dy,
	long int kdz,unsigned char AB,long int power)
{
	long int 	k;
	double		ws;
	double		v;

	if (AB == 0) {
/* default, add value in */
		ws = 1.0;
	} else if (AB == 1) {
/* flip the sign of the data */
		ws = -1.0;
	} else {
		fprintf(stderr,"Warning, unknown selector value!\n");
		return;  /* strange case (should not happen) */
	}
/* ws has no effect on squared volumes */
	if (power == 2) {
		ws = 1.0;
	}

/* modify the data from the slice */
	for(k=0;k<dx*dy*kdz;k++) {
		v = slice[k];
		v = v / FIXED_16MULT;
		if (power == 2) v = v*v;
		data[k] += (ws*v);
	}

	return;
}

int N_Choose_M(long int *cnt,long int n,long int m,long int *arr)
{
	long int	i;
	long int	have,need,top;
	
	
/* init a struct */	
	if (*cnt == 0) {
#ifdef DEBUGN
		printf("First call\n");
#endif	
		for(i=0;i<n;i++) arr[i] = 0;
		have = 0;
		need = m;
		top = n;
		if (m == 0) {
			*cnt += 1;
			return(1);
		}
	} else {
		top = n;
		have = 0;
		for(i=0;i<n;i++) have += arr[i];
		need = m - have;
	}
	
/* pop to the next 0 */
	while(1) {
		i = top-1;
		if (i < 0) return(0);
#ifdef DEBUGN
		printf("Looking at %ld at position %ld\n",arr[i],i);
#endif		
		if (arr[i] == 0) {
/* is toggling this one to 1 legal ? */
/* ie is there space in the stack to fill out the requirements should this become a 1 */
			if (((need-1) <= (n - top)) && ((have+1) <= m)) {
#ifdef DEBUGN
		printf("Toggle a 0 to a 1\n");
#endif		
				arr[i] = 1;
				need -= 1;
				have += 1;
				break;
			} 
		} else {
#ifdef DEBUGN
		printf("Removing a 1\n");
#endif		
/* remove the 1 from the stack */
			arr[i] = 0;
			need += 1;
			have -= 1;
		}
		top = i;
	}
	
/* fill down the stack */
	while(top < n) {
		if (have == m) {
/* fill rest of stack with 0 */
#ifdef DEBUGN
	printf("fill stack with 0\n");
#endif
			while(top < n) arr[top++] = 0;
		} else if (need == (n - top)) {
/* fill rest of stack with 1 */
#ifdef DEBUGN
	printf("fill stack with 1\n");
#endif
			while(top < n) arr[top++] = 1;
			need = 0;
			have = m;
		} else {
#ifdef DEBUGN
	printf("Add a 0 to the stack\n");
#endif
			arr[top++] = 0;
		}
	}
	
/* return the new selection */	
	*cnt += 1;
	return(1);	
}

void	Convolve(float *in,float *out,unsigned char *mask,
		long int dx,long int dy)
{
	long int	i,j,k,l;
	long int	x,y;
	long int	po,pi;
	double		sum,msum;

/* over all points in the plane */
for(y=(kdy/2);y<(dy-(kdy/2));y++) {
for(x=(kdx/2);x<(dx-(kdx/2));x++) {

/* get the pointer to the pixel in question */
	po = x + (y*dx) + ((kdz/2)*(dx*dy));
	
	if (mask[po] != 0) {
/* apply the kernal */
		l = 0;
		sum = 0.0;
		msum = 0.0;

/* complete size of the kernal */
		for(k=-(kdz/2);k<=(kdz/2);k++) {
		for(j=-(kdy/2);j<=(kdy/2);j++) {
		pi = po + (-(kdx/2)+(j*dx)+(k*dx*dy));
		for(i=-(kdx/2);i<=(kdx/2);i++) {
/* it is legal 
			if (((x+i)>=0) && ((x+i) <dx)) {
			if (((y+j)>=0) && ((y+j) <dy)) {
	
and incremented 
			pi = po + (i+(j*dx)+(k*dx*dy));
*/

			if (mask[pi] != 0) {
/* get the pointer to the value  and accumulate */
				sum += ktab[l]*in[pi];
				msum += ktab[l];
			}
/* legality is assured due to limits on x,y 	
			}
			}
*/
			pi++; /* next pixel */
			l++; /* next kernal value */
		}
		}
		}
/* compute the value */
		if (msum > 0.0) {
			sum = sum/msum;
		} else {
			sum = 0.0;
		}
		out[po] = sum;
	} else {
		out[po] = 0.0;
	}
}
}
	return;
}

long int nCm(long int n,long int m)
{
	double		d;
	long int 	i;
	
	d = factorial(n)/(factorial(m)*factorial(n-m));
	
	i = d;

	return(i);
}

long int nPm(long int n,long int m)
{
	double		d;
	long int 	i;
	
	d = factorial(n)/factorial(n-m);
	
	i = d;

	return(i);
}

void dump_histo(char *histfile,double histmin,double histmax,double histsize,
	long int histbins,long int histkeep,double *hist,
	double *Tmax,long int n_random)
{
	FILE		*fp;
	long int	i,j;
	double		d;

/* output the histogram file */
	if (histfile != 0L) {
/* compute the historgram of the Tmax array */
		for(i=0;i<histbins;i++) hist[i] = 0.0;
		for(i=0;i<n_random;i++) {
			d = Tmax[i];
			j = (d-histmin)/histsize;
			if (histkeep) {
				if (j<0) j = 0;
				if (j>=histbins) j = histbins-1;
			}
			if ((j>=0) && (j<histbins)) hist[j]++;
		}
/* save it */
		fp = fopen(histfile,"w");
		if (fp != 0L) {
			d = histmin;
			for(i=0;i<histbins;i++) {
				fprintf(fp,"%.5f\t%.5f\n",
					d,hist[i]);
				d += histsize;
			}
			fclose(fp);
		}
	}
	return;
}

int	cmp_dbl(const void *a,const void *b)
{
	if ((*(double *)a) > (*(double *)b)) return(1);
	if ((*(double *)a) < (*(double *)b)) return(-1);
	return(0);
}

void	dump_stats(char *statsfile,char *tmaxfile,double *Tmax,
	long int n_random,long int iteration,long int abs_tmax,
	long int t,long int s_time)
{
	FILE 		*fp = 0L;
	long int	i,j,cnt;
	double		*tt,f;

	if (iteration == 0) return;

	if (statsfile == 0) {
		fp = stdout;
	} else {
		fp = fopen(statsfile,"w");
	}
	if (fp == 0L) return;
		
	fprintf(fp,"Rerandomization output data\n");
	fprintf(fp,"%ld of %ld trials\n",iteration,n_random);

	tt = (double *)malloc(sizeof(double)*n_random);
	if (tt == 0L) ex_err("Unable to allocate sorting memory");

	j = 0;
	cnt = 0;
	for(i=1;i<n_random;i++) {
		if (abs_tmax) {
			if (ABS(Tmax[0]) > ABS(Tmax[i])) j++;
		} else {
			if (Tmax[0] > Tmax[i]) j++;
		}
		tt[cnt] = Tmax[i];
		cnt++;
	}

	qsort(tt,cnt,sizeof(double),cmp_dbl);

	if (cnt < 1) cnt = 1;
	fprintf(fp,"%f less than %f (T0)\n",
		(double)(j)/(double)(cnt),Tmax[0]);

	fprintf(fp,"Last slice took %ld seconds to compute.\n",t);

	fprintf(fp,"Total time %ld seconds.\n",s_time);

	if (fp != stdout) fclose(fp);

	if (tmaxfile) {
		fp = fopen(tmaxfile,"w");
		if (fp != NULL) {
			f = 1.0/(double)(cnt-1);
			for(i=0;i<cnt;i++) {
				fprintf(fp,"%ld\t%f\t%f\n",
					i+1,f*(double)(i),tt[i]);
			}
			fprintf(fp,"%ld\t%f\t%f\n",0L,
				(double)(j)/(double)(cnt),Tmax[0]);
			fclose(fp);
		}
		
	}

	free(tt);

	return;
}


/* allocate memory for a new list entry */	
void alloc_list_entry(long int size)
{
	if (n_random >= MAX_RANDOMIZATIONS) {
		fprintf(stderr,"Maximum number of randomizations is:%ld\n",
			MAX_RANDOMIZATIONS);
		ex_err("Please reduce the size of your request\n");
	}
	rand_list[n_random] = malloc(size);
	if (rand_list[n_random] == 0L) {
		ex_err("Unable to allocate RAM for the randomization table");
	}
	n_random++;

	return;
}

/* remove all list entries */
void dealloc_list()
{
	long int i;

	for(i=0;i<n_random;i++) {
		free(rand_list[i]);
		rand_list[i] = 0L;
	}
	n_random  = 0;

	return;
}

#define FACTOR 1.95

/* build a table of rerendomizations */
void make_rerandom_list(long subs,long maxnum)
{
	long int	count,half,i,j;
	long int	odd,iter,nflips;
	long int	arr[MAX_VOLS];

/* the first case 0 0 0 0 ... */
	sample_mode = 0;
	alloc_list_entry(subs);
	for(i=0;i<subs;i++) {
		rand_list[n_random-1][i] = 0;
	}
/* check the complete cases first */
	half = subs/2;
	if ((subs & 1) == 1) {
/* odd case */
		odd = 1;
		count = nCm(subs,half);
		count += nCm(subs,subs-half);
		if (count <= (maxnum*FACTOR)) {
/* get them all */
			iter = 0;
			while(N_Choose_M(&iter,subs,half,arr)) {
				alloc_list_entry(subs);
				for(i=0;i<subs;i++) {
					rand_list[n_random-1][i] = arr[i];
				}
			}
			iter = 0;
			while(N_Choose_M(&iter,subs,subs-half,arr)) {
				alloc_list_entry(subs);
				for(i=0;i<subs;i++) {
					rand_list[n_random-1][i] = arr[i];
				}
			}
			return;
		}
	} else {
/* even case */
		odd = 0;
		count = nCm(subs,half);
		if (count <= (maxnum*FACTOR)) {
/* get them all */
			iter = 0;
			while(N_Choose_M(&iter,subs,half,arr)) {
				alloc_list_entry(subs);
				for(i=0;i<subs;i++) {
					rand_list[n_random-1][i] = arr[i];
				}
			}
			return;
		} 
	}
/* sample maxnum randomizations */
	sample_mode = 1;
	for(i=0;i<maxnum;i++) {
		nflips = half;
		if (odd) {
			if (rand_limit(2) == 1) {
				nflips = subs - half;
			}
		}
/* clear the new entry */
		alloc_list_entry(subs);
		for(j=0;j<subs;j++) rand_list[n_random-1][j] = 0;
/* allocate nflips set bits */
		while(nflips > 1) {
/* pick a random spot */
			j = rand_limit(subs);
/* if the spot is set, pick another */
			while(rand_list[n_random-1][j] == 1) {
				j = rand_limit(subs);
			}
/* pick the spot */
			rand_list[n_random-1][j] = 1;
			nflips--;
		}
	}

	return;
}

/* return a long int randomly from 0 to max-1 */
long int rand_limit(long int m)
{
	return(lrand48() % m);
}

/* convert an image to a fixed point (32.1024) image */
void image_2_fixed16(void *in,long int dx,long int dy,long int dz,
        double offset,short *out)
{
        long int        i;
        double          v;

        for(i=0;i<dx*dy;i++) {
                switch(dz) {
                        case 1:
                                v = ((unsigned char *)in)[i];
                                break;
                        case 2:
                                v = ((unsigned short *)in)[i];
                                break;
                        case 4:
                                v = ((float *)in)[i];
                                break;
                }
                v += offset;
                v *= FIXED_16MULT;
                if (v < FIXED_16MIN) v = FIXED_16MIN;
                if (v > FIXED_16MAX) v = FIXED_16MAX;
                out[i] = v;
        }
        return;
}


void checkpoint(char *checkfile,int argc,char **argv,double *Tmax,
	long int *slice,long int nvols,char svols[MAX_VOLS][256],
	long int *s_time)
{
	FILE		*fp;
	char		file[256];
	long int	i,j,k;

	if (checkfile == 0L) return;

	if (argc == 0) {

		strcpy(file,checkfile);
		strcat(file,".kill");

/* have I arisen from the dead??? */
		fp = fopen(file,"r");
		if (fp != 0L) {
			fclose(fp);

/* remove kill file */
			unlink(file);

		} else {
			return;
		}

/* read the data file */
		strcpy(file,checkfile);
		strcat(file,".restart_data");

		fprintf(stderr,"Restarting from the file %s.\n",file);

/* Is the restart data there? */
		fp = fopen(file,"r");
		if (fp != 0L) {
			i = fscanf(fp,"%ld %ld %ld %ld\n",
				&n_random,&sample_mode,slice,s_time);
			if (i == 3) {
				*s_time = 0;
			}
			for(i=0;i<n_random;i++) {
				fscanf(fp,"%lf ",&(Tmax[i]));
				for(j=0;j<nvols;j++) {
					fscanf(fp,"%ld ",&k);
					rand_list[i][j] = k;
				}
			}
			fclose(fp);
			fprintf(stderr,"Left off on slice %ld.\n",*slice);
		}
		return;
		
	} else {

/* write the restart file */
		strcpy(file,checkfile);
		strcat(file,".restart");
		fp = fopen(file,"w");
		if (fp == 0L) return;
		fprintf(fp,"#!/bin/csh\n");
		fprintf(fp,"%s ",argv[0]);
		i = 1;
		while(argv[i][0] == '-') {
			fprintf(fp,"%s ",argv[i]);
			i++;
		}
		fprintf(fp,"-R%s ",checkfile);
		fprintf(fp,"%s ",argv[i]);
		for(i=0;i<nvols;i++) {
			fprintf(fp,"\\\n%s ",svols[i]);
		}
		fprintf(fp,"\n");
		fclose(fp);
		chmod(file,00777);  /* make it executable */

/* write the restartdata file */
                strcpy(file,checkfile);
                strcat(file,".restart_data");
                fp = fopen(file,"w");
                if (fp == 0L) return;
		fprintf(fp,"%ld %ld %ld %ld\n",
			n_random,sample_mode,*slice,*s_time);
		for(i=0;i<n_random;i++) {
			fprintf(fp,"%f ",Tmax[i]);
			for(j=0;j<nvols;j++) {
				fprintf(fp,"%ld ",(long)(rand_list[i][j]));
			}
			fprintf(fp,"\n");
		}
		fclose(fp);

/* am I to die now? */
		strcpy(file,checkfile);
		strcat(file,".kill");
		fp = fopen(file,"r");
		if (fp != 0L) {
			fclose(fp);

			fprintf(stderr,"Restart request detected.\n");
			fprintf(stderr,"Checkpoint file:%s.restart written.\n",
				checkfile);

			tal_exit(1);

		}  /* kill file found */

	} /* read or write */

	return;
}
