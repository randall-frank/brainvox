/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski
 *
 * $Id: tal_standardize.c 1213 2005-08-27 20:51:21Z rjfrank $
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
#include "stats_inc.h"

/* actions */
#define ACT_T2Z 1
#define ACT_T2P 2
#define ACT_Z2P 3
#define ACT_F2P 4

/* params */
#define V_DF1 0
#define V_DF2 1

typedef  double ( * CumDistFunction )(double v,double *vec);

/* local prototypes */
void action(double *val,long int act,double *vec);
double interp_dist(double v,CumDistFunction func,double *vec);
void ex_err(char *s);
void cmd_err(char *s);
void lookup_val(double *ftmp,double *table,double tmin,double tmax,double tstep);

double	vprec;

/* interpolate a value from the table */
void lookup_val(double *ftmp,double *table,double tmin,double tmax,double tstep)
{
	double		v1,v2;
	long int	j;
	long int num=((tmax-tmin)/tstep)+1;

#ifdef DEBUG
	printf("inval=%f ",*ftmp);
#endif
	if (*ftmp <= tmin) {  /* clamp */
		fprintf(stderr,"Warning:the value %f was clamped to %f\n",
			*ftmp,table[0]);
		*ftmp = table[0];
	} else if (*ftmp >= tmax) {  /* clamp */
		fprintf(stderr,"Warning:the value %f was clamped to %f\n",
			*ftmp,table[num-1]);
		*ftmp = table[num-1];
	} else {   /* interpolate */
		j = floor((*ftmp - tmin)/tstep);
		v1 = table[j];  v2 = table[j+1];
		*ftmp = v2 - ((v2-v1)/(tstep))*( ((j+1)*tstep+tmin) -(*ftmp));
	}
#ifdef DEBUG
	printf(" outval=%f\n",*ftmp);
#endif
	return;
}
/* find the value v in CDF function func, return x where v=func(x) */
double interp_dist(double v,CumDistFunction func,double *vec)
{
	double x,ox,on,n,tmp;

        x = 0.0;  /* start in the middle */
        ox = x+0.1;  /* offset a bit */
        on = func(ox,vec);
        while (1) {
                n = func(x,vec);
                if (fabs(n-v) < vprec) break;
                tmp = ox - ((on-v)/(on-n))*(ox-x);
                ox = x;
                on = n;
                x = tmp;
        }

	return(x);
}

void action_proc(double *val,long int act,double *vec)
{
	double	p,x;

	switch(act) {
		case ACT_T2Z:
			p = CDF_t(*val,vec);
			x = interp_dist(p,CDF_normal,NULL);
#ifdef TABLE_DEBUG
	printf("t(%f,%f)=%f=>Z=%f\n",*val,vec[0],p,x);
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
		case ACT_Z2P:
/* output = 1.0 - CDF(absvalue(z)) */
			p = fabs(*val);
			x = 1.0 - CDF_normal(p,NULL);
#ifdef TABLE_DEBUG
	printf("Z=%f -> p=%f\n",*val,x);
#endif
			*val = x;
			break;
	}

	return;
}


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
	fprintf(stderr,"         -z(dz) image depth in bytes default:4\n");
	fprintf(stderr,"         -f(start) first image number default:1\n");
	fprintf(stderr,"         -l(end) last image number default:15\n");
	fprintf(stderr,"         -i(step) image number step default:1\n");
	fprintf(stderr,"         -b swap bytes with 16bit images default:no\n");
	fprintf(stderr,"         -o(offset) default:0.0\n");
	fprintf(stderr,"         -c command line mode\n");
	fprintf(stderr,"         -df1(degrees) degrees of freedom 1 default:1\n");
	fprintf(stderr,"         -df2(degrees) degrees of freedom 2 default:1\n");
	fprintf(stderr,"         -tmin(min) lookup table min value default:-10.0\n");
	fprintf(stderr,"         -tmax(max) lookup table max value default:10.0\n");
	fprintf(stderr,"         -tstep(step) lookup table step size default:0.01\n");
	fprintf(stderr,"         -tprec(prec) set the precision for table generation default:5.0e-16\n");
	fprintf(stderr,"         -At2z Choose t dist to z dist. This is the default.\n");
	fprintf(stderr,"         -At2p Convert abs(t) score to a cumulative one tailed probability\n");
	fprintf(stderr,"         -Az2p Convert abs(z) score to a cumulative one tailed probability\n");
	fprintf(stderr,"         -Af2p Convert abs(f) score to a cumulative one tailed probability\n");
	tal_exit(1);
}

int main(int argc,char **argv)
{
	long int	dx = 256;
	long int	dy = 256;
	long int	dz = 4;
	long int	istart = 1;
	long int	iend = 15;
	long int	istep = 1;
	long int	swab = 0;
	long int	action = ACT_T2Z;
	long int	interactive = 0;
	double		offset = 0.0;
	double		df1 = 1.0;
	double		df2 = 1.0;

	double		tmin = -10.0;
	double		tmax = 10.0;
	double		tstep = 0.01;
	double		tprec = 5.0e-16;
	double		*table = 0;

	char		intemp[256],outtemp[256];
	char		tstr[256];
	long int	i,err,j;
	double		vec[10];
	unsigned char 	*data,*oimage;
	unsigned char	*bd1,*bd2;
	unsigned short	*sd1,*sd2;
	float 		*fd1,*fd2;
	double		ftmp;

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
			case 'o':
				offset = atof(&(argv[i][2]));
				break;
			case 'b':
				swab = 1;
				break;
			case 'c':
				interactive = 1;
				break;
			case 'A':
				if (strcmp(argv[i],"-At2z") == 0) {
					action = ACT_T2Z;
				} else if (strcmp(argv[i],"-At2p") == 0) {
					action = ACT_T2P;
				} else if (strcmp(argv[i],"-Az2p") == 0) {
					action = ACT_Z2P;
				} else if (strcmp(argv[i],"-Af2p") == 0) {
					action = ACT_F2P;
				} else {
					cmd_err(argv[0]);
				}
				break;
			case 't':
				if (strncmp(argv[i],"-tmin",5) == 0) {
					tmin = atof(&(argv[i][5]));
				} else if (strncmp(argv[i],"-tmax",5) == 0) {
					tmax = atof(&(argv[i][5]));
				} else if (strncmp(argv[i],"-tstep",6) == 0) {
					tstep = atof(&(argv[i][6]));
				} else if (strncmp(argv[i],"-tprec",6) == 0) {
					tprec = atof(&(argv[i][6]));
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
	vprec = tprec;
	if ((argc-i) != 2) cmd_err(argv[0]);
	strcpy(intemp,argv[i]);
	strcpy(outtemp,argv[i+1]);
/* errors */
	if ((df1 < 1.0) || (df2 < 1.0)) ex_err("Invalid degrees of freedom (<1.0)");

	if (tmin >= tmax)ex_err("tmin is greater than tmax");

	i = ((tmax-tmin)/tstep)+1;
	if (i < 1) ex_err("Bad choice of tstep value for tmin and tmax");

	table = malloc(i*sizeof(double));
	if (table == 0) ex_err("Unable to allocate lookup table memory");
/* get the data vector */
	vec[V_DF1] = df1;
	vec[V_DF2] = df2;
/* build the lookup table */
	fprintf(stderr,"Building the lookup table from %f to %f by %f...\n",
		tmin,tmax,tstep);
	ftmp = tmin;
	for(j=0;j<i;j++) {

		table[j] = ftmp;
		action_proc(&(table[j]),action,vec);

		ftmp += tstep;
	}

/* interactive mode */
	if (interactive) {
		fprintf(stderr,"Interactive mode (df1=%f df2=%f)\nEnter a number to convert:",vec[V_DF1],vec[V_DF2]);
		while (fgets(tstr,256,stdin)) {
			sscanf(tstr,"%lf",&ftmp);
			fprintf(stdout,"%g\t",ftmp);
			lookup_val(&ftmp,table,tmin,tmax,tstep);
			fprintf(stdout,"%g\n",ftmp);
		}
		goto exit_prog;
	}

/* get the image memory */
	data = malloc(dz*dx*dy);
	if (data == 0L) ex_err("Unable to allocate image memory.");
	oimage = malloc(dx*dy*dz);
	if (oimage == 0L) ex_err("Unable to allocate image memory.");
/* read the images */
	for(i=istart;i<=iend;i=i+istep) {
/* read one */
		name_changer(intemp,i,&err,tstr);
		fprintf(stderr,"Reading the file:%s\n",tstr);
		bin_io(tstr,'r',data,dx,dy,dz,0,swab,0L);
/* apply transformation */
		if (dz == 1) {
			bd1 = (unsigned char *)data;
			bd2 = (unsigned char *)oimage;
			for(j=0;j<(dx*dy);j++) {
				ftmp =  *bd1 - offset;
				lookup_val(&ftmp,table,tmin,tmax,tstep);
				ftmp += offset;
				if (ftmp < 0.0) ftmp = 0.0;
				if (ftmp > 255.0) ftmp = 255.0;
				*bd2 = ftmp;
				bd2++;
				bd1++;
			}
		} else if (dz == 2) {
			sd1 = (unsigned short *)data;
			sd2 = (unsigned short *)oimage;
			for(j=0;j<(dx*dy);j++) {
				ftmp =  *sd1 - offset;
				lookup_val(&ftmp,table,tmin,tmax,tstep);
				ftmp += offset;
				if (ftmp < 0.0) ftmp = 0.0;
				if (ftmp > 65535.0) ftmp = 65535.0;
				*sd2 = ftmp;
				sd2++;
				sd1++;
			}
		} else if (dz == 4) {
			fd1 = (float *)data;
			fd2 = (float *)oimage;
			for(j=0;j<(dx*dy);j++) {
				ftmp =  *fd1 - offset;
				lookup_val(&ftmp,table,tmin,tmax,tstep);
				ftmp += offset;
				*fd2 = ftmp;
				fd2++;
				fd1++;
			}
		}
/* write the output image */
		name_changer(outtemp,i,&err,tstr);
		fprintf(stderr,"Writing sampled file:%s\n",tstr);
		bin_io(tstr,'w',oimage,dx,dy,dz,0,swab,0L);
	}

/* all done */
exit_prog:
	free(oimage);
	free(data);
	free(table);

	tal_exit(0);
	exit(0);
}
