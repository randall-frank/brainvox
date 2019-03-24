/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski
 *
 * $Id: tal_stat.c 1833 2006-07-23 22:42:41Z rjfrank $
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

/*
 *  TODO: 
 *  	More "user proc" variables: number of volumes, size of kernal, etc
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <string.h>
#include <float.h>
#ifndef WIN32
#include <unistd.h>
#else
#include <windows.h>
#endif
#include "proto.h"
#include "macroprotos.h"

#ifdef WIN32
#define TAL_SUPPORT_DIR "C:\\Program Files\\U of Iowa LCN\\Brainvox\\bin"
#else
#ifdef __APPLE__
#define TAL_SUPPORT_DIR "/Applications/Brainvox.app/lib"
#else
#define TAL_SUPPORT_DIR "/usr/local/lib"
#endif
#endif

#define MAX_VOLS	2000

#define I_AVG 0
#define I_VAR 1
#define I_VOL 2
#define I_VOX 3
#define I_SOS 4
#define I_SUM 5
#define I_MIN 6
#define I_MAX 7
#define I_SDV 8
#define I_COR 9
#define I_SDS 10
#define I_COC 11
#define I_CUS 12
#define I_EOL 13

#define MIN(a,b) (((a)<(b)?(a):(b)))
#define MAX(a,b) (((a)>(b)?(a):(b)))

typedef struct {
	double		n;
	double		vol;
	double		mean;
	double		var;
	double		sum;
	double		ss;
	double		min;
	double		max;
	double		sds;
	double		cor;
	double		coc;
	double		custom;
	long int	nvols;
	double		histmin;
	double		histmax;
	double		histsize;
	double		*hist;
	long int	histbins;
	long int	histkeep;
} mystats;

/* kernal params */
double		*ktab = 0L;
int 		*kmtab = 0L;
double		ksum = 0.0;
long int	kdx = 1;
long int	kdy = 1;
long int	kdz = 1;

/* verbose flag */
long int	verbose = 0;

void global_stats(unsigned char *data[],unsigned char *mask,long int dx,
	long int dy,long int dz,long int nvols,double offset,mystats *gstats);
void local_stats(unsigned char *data[],unsigned char *mask,long int dx,
	long int dy,long int dz,long int nvols,double offset,char *onames[],
	long int slice,double back);
void ex_err(char *s);
void read_kernal(char *file);
void cmd_err(char *s);
void add_slice(char *temp,long int sl,void *data,long int dx,long int dy,
	long int dz,long int kdz,long int swab);
void shiftvolume(void *data,long int dx,long int dy,long int dz,long int kdz);
void    tal_stat_setup(void);
int real_tal_stat_main(int argc,char **argv);

/* expression handling */
long int User_Identifiers(char *name, double *value);
long int eval_expression(double *value,double x,double y,double z,
		double valA,double valB,double kerA,double kerB,double kerS);
void build_compiled_exp(char *expr,char *exe,char *comment);

long int (*user_proc)(double *value,double x,double y,double z,
                double valA,double valB,double kerA,double kerB,double kerS) = 
		eval_expression;
char    *user_expression = NULL;
char    *user_comment = NULL;
int	compiled_expression = 0;

void ex_err(char *s)
{
	fprintf(stderr,"Fatal error:%s\n",s);
	tal_exit(1);
}

void read_kernal(char *file)
{
	FILE		*fp;
	long int	i,j,k,l;
	char		tstr[256];

	fp = fopen(file,"r");
	if (fp == 0) {
		fprintf(stderr,"Unable to open %s\n",file);
		return;
	}
/* read the header */
	l = fscanf(fp,"%ld %ld %ld",&i,&j,&k);
	if (l == 3) {
/* read the actual kernel values */
		ktab = (double *)malloc(sizeof(double)*i*j*k);
		if (ktab == 0L) ex_err("Memory allocation error");
		kmtab = (int *)malloc(sizeof(int)*i*j*k);
		if (kmtab == 0L) ex_err("Memory allocation error");
		ksum = 0;
		for(l=0;l<(i*j*k);l++) {
			fscanf(fp,"%lf",&(ktab[l]));
			kmtab[l] = 1;
			ksum += ktab[l];
		}
		kdx = i;
		kdy = j;
		kdz = k;
	} else {
		fprintf(stderr,"Invalid kernal file format\n");
	}
/* read a mask if one was provided */
	tstr[0] = '\0';
	fgets(tstr,sizeof(tstr),fp);
	if (strstr(tstr,"KERNALMASK")) {
		ksum = 0;
		for(l=0;l<(i*j*k);l++) {
			fscanf(fp,"%d",&(kmtab[l]));
			if (kmtab[l]) ksum += ktab[l];
		}
	}
/* done */
	fclose(fp);
	return;
}

void cmd_err(char *s)
{
	fprintf(stderr,"(%s:%s) Usage: %s [options] [-|vol1temp ...]\n",__DATE__,TAL_VERSION,s);
	if (compiled_expression) {
		fprintf(stderr,"This is a custom tal_stat program.\n");
		if (user_comment) fprintf(stderr,"Comment:%s\n",user_comment);
	}
	fprintf(stderr,"Options: -x(dx) image dx default:256\n");
	fprintf(stderr,"         -y(dy) image dy default:256\n");
	fprintf(stderr,"         -z(dz) image depth in bytes default:2\n");
	fprintf(stderr,"         -f(start) first image number default:1\n");
	fprintf(stderr,"         -l(end) last image number default:15\n");
	fprintf(stderr,"         -i(step) image number step default:1\n");
	fprintf(stderr,"         -d(back) background value output outside of the mask default:0.0\n");
	fprintf(stderr,"         -b swap bytes with 16bit images default:no\n");
	fprintf(stderr,"         -v verbose mode\n");
	fprintf(stderr,"         -o(offset) value added to each pixel default:0\n");
	fprintf(stderr,"         -Hfile(histofile) file to save multi-volume histogram in default:none\n");
	fprintf(stderr,"         -Hmin(minval) minimum histogram bin center value default:0.0\n");
	fprintf(stderr,"         -Hmax(maxval) maximum histogram bin center value default:255.0\n");
	fprintf(stderr,"         -Hsize(binsize) size (width) of a histogram bin default:1.0\n");
	fprintf(stderr,"         -Hkeep map pixels outside min/max to min/max bins default:drop them\n");
	fprintf(stderr,"         -m(masktemp) filename template for a mask volume\n");
	fprintf(stderr,"         -k(kernalfile) filename of a kernal used for local stats\n");
	fprintf(stderr,"         -help print help on expressions\n");
	if (!compiled_expression) {
	        fprintf(stderr,"         -c(exename) Create a custom executable to execute the expression.\n");
	        fprintf(stderr,"         -C(comment)  Supply a user comment for a custom executable.\n");
		fprintf(stderr,"         -e(expression) custom local statistic expression. default:none\n");
	}
	fprintf(stderr,"If a kernalfile is specified, output volumes may be generated.  The voxels\n");
	fprintf(stderr,"in these volumes will be the values of statistics computed for voxels\n");
	fprintf(stderr,"in the neighborhood defined by the kernal around the output voxel.\n");
	fprintf(stderr,"The default kernal is a single pixel of weight 1.0.\n");
	fprintf(stderr,"Output volumes templates are selected by setting local statistics templates.\n");
	fprintf(stderr,"Output volumes are always 4byte (float) volumes.\n");
	fprintf(stderr,"         -Lavg(meantemp) local mean stats are output in the meantemp volume\n");
	fprintf(stderr,"         -Lvar(vartemp) local variance stats are output in the vartemp volume\n");
	fprintf(stderr,"         -Lvol(voltemp) local volume counts are output in the voltemp volume\n");
	fprintf(stderr,"         -Lvox(voxtemp) local voxel counts are output in the voxtemp volume\n");
	fprintf(stderr,"         -Lsos(sstemp) local sum-squares stats are output in the sstemp volume\n");
	fprintf(stderr,"         -Lsum(sumtemp) local sum stats are output in the sumtemp volume\n");
	fprintf(stderr,"         -Lmin(mintemp) local min stats are output in the mintemp volume\n");
	fprintf(stderr,"         -Lmax(maxtemp) local max stats are output in the maxtemp volume\n");
	fprintf(stderr,"         -Lsdv(sdvtemp) local st-dev stats are output in the sdvtemp volume\n");
	fprintf(stderr,"         -Lcor(cortemp) local correlation is output in the cortemp volume\n");
	fprintf(stderr,"         -Lsds(sdstemp) local sum of diff squares is output in sdstemp volume\n");
	fprintf(stderr,"         -Lcoc(coctemp) local corr coef is output in the coctemp volume\n");
	fprintf(stderr,"         -Lcus(custemp) local custom expr output in the custemp volume\n");
	if (compiled_expression) {
		fprintf(stderr,"Custom expression=%s\n",user_expression);
	}
	fprintf(stderr,"output =  stats of (p+offset)\n");
	fprintf(stderr,"If '-' is used for the input volume, the template names are read from stdin.\n");
	fprintf(stderr,"Note:mask volumes must be 8bit\n");
	tal_exit(1);
}

void	add_slice(char *temp,long int sl,void *data,long int dx,long int dy,
	long int dz,long int kdz,long int swab)
{
	char		tstr[256];
	long int	i,err,j;
	unsigned char	*p1 = (unsigned char *)data;

/* move the data down and clear out this slice */
	shiftvolume(data,dx,dy,dz,kdz);

/* if the filename is null, then this is a null mask template */
/* we return a TRUE image in this case */
	if ((strcmp(temp,"+") == 0L) && (dz == 1)) {
		j = dx*dy*dz*(kdz-1);
		for(i=0;i<(dx*dy);i++) p1[j++] = 1;
		return;
	}

/* get the proper filename */
	name_changer(temp,sl,&err,tstr);

/* read the file */
	fprintf(stderr,"Reading the file:%s\n",tstr);
	j = dx*dy*dz*(kdz-1);
	bin_io(tstr,'r',&(p1[j]),dx,dy,dz,1,swab,0L);

	return;
}
void	shiftvolume(void *data,long int dx,long int dy,long int dz,long int kdz)
{
	long int 	i;
	unsigned char	*p1,*p2;

	p1 = (unsigned char *)data;
	p2 = p1 + (dx*dy*dz);
	for(i=0;i<(dx*dy*dz*(kdz-1));i++) {
		*p1 = *p2;
		p1++;
		p2++;
	}
/* clear out the top slice */
	if (dz == 1) {
		for(i=0;i<(dx*dy);i++) ((unsigned char *)p1)[i] = 0;
	} else if (dz == 2) {
		for(i=0;i<(dx*dy);i++) ((unsigned short *)p1)[i] = 0;
	} else if (dz == 4) {
		for(i=0;i<(dx*dy);i++) ((float *)p1)[i] = 0.0;
	}
	return;
}

int real_tal_stat_main(int argc,char **argv)
{
	long int	dx = 256;
	long int	dy = 256;
	long int	dz = 2;
	long int	istart = 1;
	long int	iend = 15;
	long int	istep = 1;
	long int	swab = 0;
	double		offset = 0;
	double		back = 0.0;
	double		histmin = 0.0;
	double		histmax = 255.0;
	double		histsize = 1.0;
	long int	histkeep = 0;
	long int	have_local = 0;
	char            *exename = 0;
	char            *comment = 0;
			
	char		*vols[MAX_VOLS],masktemp[256];
	char		*onames[I_EOL];
	char		tstr[256];
	long int	i,err,j,k;
	long int	nvols,histbins = 0;
	mystats		gstats;
	
	FILE		*fp;
	unsigned char	*data[MAX_VOLS];
	unsigned char	*maskimage;
	char		*histfile = 0L;
	double		d,*hist = 0L;

/* init some strings */
	strcpy(masktemp,"+");
	for(i=0;i<I_EOL;i++) onames[i] = 0L;

/* get the setup */
	tal_stat_setup();

/* parse the command line */
	nvols = 0;
	i = 1;
	while ((i < argc) && (argv[i][0] == '-')) {
		switch (argv[i][1]) {
			case '\0':
				if (i != argc-1) cmd_err(argv[0]);
				while (fgets(tstr,256,stdin)) {
					char	temp[256];
					sscanf(tstr,"%s",temp);
					vols[nvols] = strdup(temp);
					nvols++;
	if (nvols == MAX_VOLS) ex_err("Max number of volumes exceeded");
				}
				break;
			case 'c':
				if (compiled_expression) cmd_err(argv[0]);
				exename = &(argv[i][2]);
				break;
			case 'C':
				if (compiled_expression) cmd_err(argv[0]);
				comment = &(argv[i][2]);
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
			case 'o':
				offset = atof(&(argv[i][2]));
				break;
			case 'd':
				back = atof(&(argv[i][2]));
				break;
			case 'b':
				swab = 1;
				break;
			case 'v':
				verbose = 1;
				break;
			case 'k':
				read_kernal(&(argv[i][2]));
				break;
			case 'm':
				strcpy(masktemp,&(argv[i][2]));
				break;
			case 'e':
				if (compiled_expression) cmd_err(argv[0]);
				user_expression = &(argv[i][2]);
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
			case 'L':
				have_local = 1;
				if (strncmp("-Lavg",argv[i],5) == 0) {
					onames[I_AVG] = &(argv[i][5]);
				} else if (strncmp("-Lvar",argv[i],5) == 0) {
					onames[I_VAR] = &(argv[i][5]);
				} else if (strncmp("-Lvol",argv[i],5) == 0) {
					onames[I_VOL] = &(argv[i][5]);
				} else if (strncmp("-Lvox",argv[i],5) == 0) {
					onames[I_VOX] = &(argv[i][5]);
				} else if (strncmp("-Lsos",argv[i],5) == 0) {
					onames[I_SOS] = &(argv[i][5]);
				} else if (strncmp("-Lsum",argv[i],5) == 0) {
					onames[I_SUM] = &(argv[i][5]);
				} else if (strncmp("-Lmin",argv[i],5) == 0) {
					onames[I_MIN] = &(argv[i][5]);
				} else if (strncmp("-Lmax",argv[i],5) == 0) {
					onames[I_MAX] = &(argv[i][5]);
				} else if (strncmp("-Lsdv",argv[i],5) == 0) {
					onames[I_SDV] = &(argv[i][5]);
				} else if (strncmp("-Lcor",argv[i],5) == 0) {
					onames[I_COR] = &(argv[i][5]);
				} else if (strncmp("-Lsds",argv[i],5) == 0) {
					onames[I_SDS] = &(argv[i][5]);
				} else if (strncmp("-Lcoc",argv[i],5) == 0) {
					onames[I_COC] = &(argv[i][5]);
				} else if (strncmp("-Lcus",argv[i],5) == 0) {
					onames[I_CUS] = &(argv[i][5]);
				} else {
					cmd_err(argv[0]);
				}
				break;
			default:	
				if (strcmp(argv[i],"-help") == 0) {
					print_exp_help(argv[0],2);
					tal_exit(0);
				} else {
					cmd_err(argv[0]);
				}
				break;
		}
		i++;
	}
	if (exename) {
		if (!user_expression) {
			fprintf(stderr,
			"Error: Custom output requires an expression (-e)\n");
			exit(1);
		}
		build_compiled_exp(user_expression,exename,comment);
		tal_exit(0);
	}
	if (onames[I_CUS] && (!user_expression)) {
		fprintf(stderr,
			"Error: Custom output requires an expression (-e)\n");
		exit(1);
	}
/* get the volume names */
	while (i < argc) {
		vols[nvols] = strdup(argv[i]);
		nvols++;
		if (nvols == MAX_VOLS) ex_err("Max number of volumes exceeded");
		i++;
	}
	if (nvols < 1) cmd_err(argv[0]);

/* make the default kernal */
	if ((ktab == 0L) && (have_local)) {
		ktab = (double *)malloc(sizeof(double));
		if (ktab == 0L) ex_err("Memory allocation error");
		kmtab = (int *)malloc(sizeof(int));
		if (kmtab == 0L) ex_err("Memory allocation error");
		ktab[0] = 1.0;
		kmtab[0] = 1;
		ksum = 1.0;
	}

/* report to user */
	if (verbose) {
		fprintf(stdout,"Computing for the following volumes:\n");
		for(i=0;i<nvols;i++) fprintf(stdout,"%s\n",vols[i]);
		fprintf(stdout,"Global ");
		if (ktab != 0) {
			fprintf(stdout,"and local stats.\n");
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
			fprintf(stdout,"\n");
		}
		} else {
			fprintf(stdout,"stats only\n");
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
/* get memory for all the volumes */
	for(i=0;i<nvols;i++) {
		data[i] = malloc(dz*dx*dy*kdz);
		if (data[i] == 0L) ex_err("Unable to allocate image memory.");
	}
/* get mask volume memory (8bit) */
	maskimage = malloc(dx*dy*kdz);
	if (maskimage == 0L) ex_err("Unable to allocate image memory.");

/* clear out all the data volumes to zero (especially the float case) */
	for(j=0;j<nvols;j++) {
	if (dz == 1) {
		for(i=0;i<(dx*dy*kdz);i++) ((unsigned char *)(data[j]))[i] = 0;
	} else if (dz == 2) {
		for(i=0;i<(dx*dy*kdz);i++) ((unsigned short *)(data[j]))[i] = 0;
	} else if (dz == 4) {
		for(i=0;i<(dx*dy*kdz);i++) ((float *)data[j])[i] = 0.0;
	}
	}
	for(i=0;i<(dx*dy*kdz);i++) ((unsigned char *)maskimage)[i] = 0;
/* all volumes are clear */

/* initialize the global stats */
	gstats.nvols = nvols;
	gstats.n = 0;
	gstats.vol = 0;
	gstats.mean = 0;
	gstats.var = 0;
	gstats.sum = 0;
	gstats.ss = 0;
	gstats.min = DBL_MAX;
	gstats.max = -DBL_MAX;
	gstats.cor = 0;
	gstats.coc = 0;
	gstats.custom = 0;
	gstats.sds = 0;
	gstats.hist = hist;
	gstats.histmin = histmin;
	gstats.histmax = histmax;
	gstats.histsize = histsize;
	gstats.histbins = histbins;
	gstats.histkeep = histkeep;

/* preread the first kdz/2 images */
	i = istart;
	for(j=0;j<(kdz/2);j++) {
		for(k=0;k<nvols;k++) {
			if (i <= iend) {
				add_slice(vols[k],i,data[k],dx,dy,dz,kdz,swab);
			} else {
				shiftvolume(data[k],dx,dy,dz,kdz);
			}
		}
		if (i <= iend) {
			add_slice(masktemp,i,maskimage,dx,dy,1,kdz,swab);
		} else {
			shiftvolume(maskimage,dx,dy,1,kdz);
		}
		i += istep;
	}
/* now, the data vols are preped and the work can begin */
/* i is the next image to be added to the data vols, j is the center image */
	for(j=istart;j<=iend;j+=istep) {
/* load the images */
		for(k=0;k<nvols;k++) {
			if (i <= iend) {
				add_slice(vols[k],i,data[k],dx,dy,dz,kdz,swab);
			} else {
				shiftvolume(data[k],dx,dy,dz,kdz);
			}
		}
/* if the mask is still in the volume, read in a new image, else,
	load in a FALSE image */
		if (i <= iend) {
			add_slice(masktemp,i,maskimage,dx,dy,1,kdz,swab);
		} else {
			shiftvolume(maskimage,dx,dy,1,kdz);
		}
/* do the requested work */
		global_stats(data,maskimage,dx,dy,dz,nvols,offset,&gstats);
		if (ktab != 0) {
		    local_stats(data,maskimage,dx,dy,dz,nvols,offset,onames,j,
			back);
		}
/* next slice */
		i += istep;
	}
/* calc any global stats */
	if (gstats.n != 0.0) gstats.mean = gstats.sum/gstats.n;
	if (gstats.n > 1.0) {
		gstats.var = gstats.ss - gstats.n*(gstats.mean*gstats.mean);
		gstats.var = gstats.var / (gstats.n - 1.0);
	}
/* print out globals stats */
	fprintf(stdout,"voxels\t%f\n",gstats.n);
	fprintf(stdout,"volume\t%f\n",gstats.vol);
	fprintf(stdout,"mean\t%f\n",gstats.mean);
	fprintf(stdout,"var\t%f\n",gstats.var);
	fprintf(stdout,"min\t%f\n",gstats.min);
	fprintf(stdout,"max\t%f\n",gstats.max);
	fprintf(stdout,"sum\t%f\n",gstats.sum);
	fprintf(stdout,"ss\t%f\n",gstats.ss);
	fprintf(stdout,"stdev\t%f\n",sqrt(gstats.var));
	fprintf(stdout,"nvols\t%ld\n",gstats.nvols);

#if 0
/* CUS, COR, COC and SDS are NOT global stats */
	fprintf(stdout,"cor\t%f\n",gstats.cor);
	fprintf(stdout,"sds\t%f\n",gstats.sds);
	fprintf(stdout,"coc\t%f\n",gstats.coc);
	fprintf(stdout,"cus\t%f\n",gstats.custom);
#endif

/* output the histogram file */
	if (histfile != 0L) {
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

/* free up the memory */
	for(i=0;i<nvols;i++) {
		free(data[i]);
		free(vols[i]);
	}
	free(maskimage);
	if (ktab != 0) free(ktab);
	if (kmtab != 0) free(kmtab);
	if (hist != 0) free(hist);

/* that's all she wrote */
	tal_exit(0);
	exit(0);
}

/* computations for the globals stats */
void global_stats(unsigned char *data[],unsigned char *mask,long int dx,
	long int dy,long int dz,long int nvols,double offset,mystats *gstats)
{
	long int	i,j,k,c,m;
	double		val;

	c = dx*dy*(kdz/2);
	for(j=0;j<dy;j++) {
	for(i=0;i<dx;i++) {
		if (mask[c] != 0) {
			for(k=0;k<nvols;k++) {
				if (dz == 1) {
					val = ((unsigned char *)(data[k]))[c];
				} else if (dz == 2) {
					val = ((unsigned short *)(data[k]))[c];
				} else if (dz == 4) {
					val = ((float *)(data[k]))[c];
				} else {
					val = 0;
				}
				val += offset;
/* add to the stats */
				gstats->n += 1;
				gstats->sum += val;
				gstats->ss += (val*val);
				if (val > gstats->max) gstats->max = val;
				if (val < gstats->min) gstats->min = val;
/* do histogram */
				if (gstats->hist != 0L) {
/* binnum = floor(value-minval+0.5binsize)/binsize) */
m=floor((val-(gstats->histmin)+(0.5*(gstats->histsize)))/(gstats->histsize));
					if (gstats->histkeep) {
						if (m < 0) m = 0;
						if (m >= gstats->histbins) 
							m = gstats->histbins-1;
						gstats->hist[m] += 1.0;
					} else {
						if ((m >= 0) && (m <=
							gstats->histbins-1)) {
							gstats->hist[m] += 1.0;
						}
		
					}
				
				}

			} /* over all volumes */
			gstats->vol += 1;
		} /* mask check */
		c++;
	} /* x */
	} /* y */
	return;
}

void local_stats(unsigned char *data[],unsigned char *mask,long int dx,
	long int dy,long int dz,long int nvols,double offset,char *oname[],
	long int slice,double back)
{
	float		*odata[I_EOL];
	float		val;
	long int 	i,j,k,n;
	mystats		acc;
	char		tstr[256];
	long int	x,y,z,p;
	long int	dzz,dxy;
	long int	center_offset = 0;

	dxy = dx*dy;
	dzz = (kdz/2)*dxy; /* offset to center plane */

	if (oname[I_CUS]) {
		k = 0;
		for(z=-(kdz/2);z<=(kdz/2);z++) {
		for(y=-(kdy/2);y<=(kdy/2);y++) {
		for(x=-(kdx/2);x<=(kdx/2);x++) {
			if ((x==0)&&(y==0)&&(z==0)) center_offset = k;
			k++;
		}
		}
		}

		/* setup user defined IDENTIFIER search */
		if (!compiled_expression) {
       			set_user_identifier(User_Identifiers);
       			if (init_parser(user_expression,1L)) {
	    			fprintf(stderr,"Error: Invalid expression.\n");
	    			tal_exit(1);
	    			exit(1);
			}
		}

	}
/* allocate temp memory */
	for(i=0;i<I_EOL;i++) {
		if (oname[i] != 0L) {
			odata[i] = (float *)malloc(sizeof(float)*dxy);
			if (odata[i] == 0) {
				ex_err("Unable to allocate memory.");
			}
		}
	}
/* convolve over all valid pixels in the plane */
	for(j=0;j<dy;j++) {
	for(i=0;i<dx;i++) {
/* check the mask volume */
	if (mask[dzz+(j*dx)+i] != 0) {
		double	cor_d0,cor_d1,cor_d2,cor_d3;
/* single voxel convolution */
		acc.nvols = nvols;
		acc.n = 0;
		acc.vol = 0;
		acc.mean = 0;
		acc.var = 0;
		acc.sum = 0;
		acc.ss = 0;
		acc.min = DBL_MAX;
		acc.max = -DBL_MAX;
		acc.cor = 0;
		acc.coc = 0;
		acc.custom = 0;
		acc.sds = 0;
		cor_d0 = 0;
		cor_d1 = 0;
		cor_d2 = 0;
		cor_d3 = 0;

/* compute kernal data */
		k = 0;   /* kernal index */
		for(z=-(kdz/2);z<=(kdz/2);z++) {
		for(y=-(kdy/2);y<=(kdy/2);y++) {
		for(x=-(kdx/2);x<=(kdx/2);x++) {
		p = (dzz+(j*dx)+i);  /* base voxel location */
		p += ((z*dxy) + (y*dx) + x);  /* kernal offsets */
/* accumulate only if the pixel is in the mask and composite x,y is valid */
		if (((x+i) >= 0) && ((x+i) < dx)) {
		if (((y+j) >= 0) && ((y+j) < dy)) {
		if ((mask[p] != 0) && (kmtab[k] != 0)) {
/* work over all loaded volumes */
			for(n=0;n<nvols;n++) {
				if (dz == 1) {
					val = ((unsigned char *)(data[n]))[p];
				} else if (dz == 2) {
					val = ((unsigned short *)(data[n]))[p];
				} else if (dz == 4) {
					val = ((float *)(data[n]))[p];
				} else {
					val = 0;
				}
				val += offset;
/* accumulate the data */
				acc.n += (ktab[k]); /* number of voxels */
				acc.sum += (val*ktab[k]);
				acc.ss += ((val*val)*ktab[k]);
				if (val > acc.max) acc.max = val;
				if (val < acc.min) acc.min = val;
				acc.sds += ((val-ktab[k])*(val-ktab[k]));

				/* five parts to correlation */
				cor_d0 += (val*val);
				cor_d1 += (ktab[k]*ktab[k]);
				cor_d2 += ktab[k];
				acc.cor += (val*ktab[k]);
				acc.coc += val; 

				/* custom expression */
				if (oname[I_CUS]) {
				    double  	valc = 0.0;
				    double	f = 0.0;
				    long int	pp;

				    pp = (dzz+(j*dx)+i); 
				    if (dz == 1) {
					valc=((unsigned char *)(data[n]))[pp];
				    } else if (dz == 2) {
					valc=((unsigned short *)(data[n]))[pp];
				    } else if (dz == 4) {
					valc=((float *)(data[n]))[pp];
				    }
				    valc += offset;
				    user_proc(&f,x,y,z,val,valc,
					ktab[k],ktab[center_offset],ksum);
				    acc.custom += f;
				}
			}
			acc.vol += 1;
		} /* mask check for pixels */
		} /* legal composite y check */
		} /* legal composite x check */
		k++;
		} /* kernal x */
		} /* kernal y */
		} /* kernal z */

/* calc any composite local stats */
		if (acc.n != 0.0) acc.mean = acc.sum/acc.n;
		if (acc.n > 1.0) {
			acc.var = acc.ss - acc.n*(acc.mean*acc.mean);
			acc.var = acc.var / (acc.n - 1.0);
		}

/* calc coef r=(n*#(xy) - #(x)*#(y))/
               sqrt( (n*#(x*x) - #(x)*#(x))*(n*#(y*y) - #(y)*#(y)))
*/
		cor_d3 = sqrt( ((acc.vol*nvols)*cor_d0 - (acc.coc*acc.coc)) *
			       ((acc.vol*nvols)*cor_d1 - (cor_d2*cor_d2)) );
		if (cor_d3 == 0.0) cor_d3 = 1.0;
		acc.coc = ((acc.vol*nvols)*acc.cor - acc.coc*cor_d2) / cor_d3;

/* calc regular correlation: #(xy) / sqrt( #(xx) * #(yy) ) */
		cor_d0 = cor_d0*cor_d1;
		if (cor_d0 == 0.0) cor_d0 = 1.0;
		acc.cor = acc.cor/sqrt(cor_d0);

/* store results in the appropriate location */
		if (oname[I_AVG] != 0) odata[I_AVG][(j*dx)+i] = acc.mean;
		if (oname[I_VAR] != 0) odata[I_VAR][(j*dx)+i] = acc.var;
		if (oname[I_VOL] != 0) odata[I_VOL][(j*dx)+i] = acc.vol;
		if (oname[I_VOX] != 0) odata[I_VOX][(j*dx)+i] = acc.n;
		if (oname[I_SOS] != 0) odata[I_SOS][(j*dx)+i] = acc.ss;
		if (oname[I_SUM] != 0) odata[I_SUM][(j*dx)+i] = acc.sum;
		if (oname[I_MIN] != 0) odata[I_MIN][(j*dx)+i] = acc.min;
		if (oname[I_MAX] != 0) odata[I_MAX][(j*dx)+i] = acc.max;
		if (oname[I_SDV] != 0) odata[I_SDV][(j*dx)+i] = sqrt(acc.var);
		if (oname[I_COR] != 0) odata[I_COR][(j*dx)+i] = acc.cor;
		if (oname[I_SDS] != 0) odata[I_SDS][(j*dx)+i] = acc.sds;
		if (oname[I_COC] != 0) odata[I_COC][(j*dx)+i] = acc.coc;
		if (oname[I_CUS] != 0) odata[I_CUS][(j*dx)+i] = acc.custom;
	} else {
/* outside the mask, all values are (back) */
		for(k=0;k<I_EOL;k++) {
			if (oname[k] != 0L) odata[k][(j*dx)+i] = back;
		}
	} /* mask test */
/* next pixel in the plane */
	} /* next x */
	} /* next y */
/* write the volumes to disk and free up the memory */
	for(i=0;i<I_EOL;i++) {
		if (oname[i] != 0L) {
       	         	name_changer(oname[i],slice,&k,tstr);
       	         	fprintf(stderr,"Writing stats file:%s\n",tstr);
	                bin_io(tstr,'w',odata[i],dx,dy,sizeof(float),1,0,0L);
			free(odata[i]); 
		}
	}

/* remove user defined IDENTIFIER search */
	if ((!compiled_expression) && oname[I_CUS]) set_user_identifier(0L);

/* done */
	return;
}


/* custom expression code */
typedef struct {
        char *name;
        double value;
        } Identifiers;

#define DX_ID	0
#define DY_ID	1
#define DZ_ID	2
#define PA_ID	3
#define PB_ID	4
#define KA_ID	5
#define KB_ID	6
#define KS_ID	7

Identifiers ident_list[] = {	
				{"dx",0.0},
				{"dy",0.0},
				{"dz",0.0},
				{"pa",0.0},
				{"pb",0.0},
				{"ka",0.0},
				{"kb",0.0},
				{"ks",0.0},
				{0L,0.0}
			   };

long int User_Identifiers(char *name, double *value)
{
        long int i;

        i = 0;
        while (ident_list[i].name != 0L) {
                if (strcmp(ident_list[i].name,name) == 0L) {
                        *value = ident_list[i].value;
                        return(1);
                }
                i++;
        }
        return(0);
}

long int eval_expression(double *value,double x,double y,double z,
		double valA,double valB,double kerA,double kerB,double kerS)
{
	char		out[256];
	long int	ret = 0;

	*value = 0.0;

	/* set up identifiers */
	ident_list[DX_ID].value = x;
	ident_list[DY_ID].value = y;
	ident_list[DZ_ID].value = z;
	ident_list[PA_ID].value = valA;
	ident_list[PB_ID].value = valB;
	ident_list[KA_ID].value = kerA;
	ident_list[KB_ID].value = kerB;
	ident_list[KS_ID].value = kerS;

	if (parse_math(out,value) != 0L) ret = 1;
	if (reset_tokenlist_ptr()) ret = 1;

#ifdef NEVER
printf("%f=f(%f,%f,%f,%f,%f,%f,%f,%f)\n",*value,x,y,z,valA,valB,kerA,kerB,kerS);
#endif

	return(ret);
}

void build_compiled_exp(char *expr,char *exe,char *comment)
{
	FILE	*fp;
	char	file[L_tmpnam+4];
	char	comp[4096];
	char	*env;

	if (init_parser(expr,1)) {
		fprintf(stderr,"Unable to parse:%s\n",expr);
	        tal_exit(1);
	}
	if (C_parse_math(comp)) {
	        fprintf(stderr,"Unable to parse:%s\n",expr);
	        tal_exit(1);
	}

	tmpnam(file);
	strcat(file,".c");

	fp = fopen(file,"w");
	if (fp == 0L) {
	        fprintf(stderr,"Unable to create temp .c file\n");
	        tal_exit(1);
	}
	fprintf(fp,"#include <stdio.h>\n");
	fprintf(fp,"#include <stdlib.h>\n");
	fprintf(fp,"#include <math.h>\n");
	fprintf(fp,"#include <string.h>\n");
	fprintf(fp,"extern long int (*user_proc)(double *value,double dx,double dy,double dz,double valA,double valB,double kerA,double kerB,double kerS);\n");
	fprintf(fp,"extern char     *user_expression;\n");
	fprintf(fp,"extern char     *user_comment;\n");
	fprintf(fp,"extern int      compiled_expression;\n");
	fprintf(fp,"int real_tal_stat_main(int argc,char **argv);\n");
	fprintf(fp,"long int comp_expression(double *value,double dx,double dy,double dz,double valA,double valB,double kerA,double kerB,double kerS);\n");
        fprintf(fp,"void tal_stat_setup()\n");
	fprintf(fp,"{\n");
		fprintf(fp,"user_proc = comp_expression;\n");
	        fprintf(fp,"user_expression = \"%s\";\n",expr);
	        if (comment) {
	        	fprintf(fp,"user_comment = \"%s\";\n",comment);
	        } else {
	                fprintf(fp,"user_comment = 0L;\n");
	        }
	        fprintf(fp,"compiled_expression = 1;\n");
	fprintf(fp,"}\n");
	fprintf(fp,"long int comp_expression(double *value,double dx,double dy,double dz,double pa,double pb,double ka,double kb,double ks)\n");

	fprintf(fp,"{\n");
		fprintf(fp,"*value = %s\n",comp);
		fprintf(fp,"return(0);\n");
	fprintf(fp,"}\n");

	fprintf(fp,"int main(int argc,char **argv)\n");
	fprintf(fp,"{\n");
		fprintf(fp,"   return(real_tal_stat_main(argc,argv));\n");
        fprintf(fp,"}\n");
        fclose(fp);

        env=getenv("TAL_SUPPORT_DIR");
        if (env == 0L) {
#ifdef WIN32
                char path[256],*p0;
                GetModuleFileName(NULL,path,sizeof(path));
                p0 = strrchr(path,'\\');
                if (p0) p0[1] = '\0';
                env = strdup(p0);
#else
                env = TAL_SUPPORT_DIR;
#endif
        }
#ifdef WIN32
        sprintf(comp,"cl -DWIN32 -Oait -Gs -nologo %s \"%s\\libtal_stat.lib\" /Fe%s",
                file,env,exe);
#else
        sprintf(comp,"cc -s -O %s -L%s -ltal_stat -o %s -lm",
                file,env,exe);
#endif

/* not the safest way to do this, I know! */
        system(comp);

#ifdef DEBUG
        sprintf(comp,"cp %s .",file);
        system(comp);
#endif

        unlink(file);

        tal_exit(0);
}
