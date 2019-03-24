/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski
 *
 * $Id: tal_histcheck.c 1213 2005-08-27 20:51:21Z rjfrank $
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

#undef DEBUG

#define MIN(a,b) (((a)<(b)?(a):(b)))
#define MAX(a,b) (((a)>(b)?(a):(b)))

#define B_MAX 100
#define B_WID buck_width

void get_dist1d(double *data,long int npts,long int ncols,long int dcol,
	double c,double bucketsize,double *dist,double **sdata,long int *n);
void get_dist2d(double *data,long int npts,long int ncols,long int dcol,
	long int dcol2,double c,double d,double bucketsize,double *dist,
	double **sdata,long int *n);
void mannwhitney(double *data1,long int n1,double *data2,long int n2,
	double *u,double *prob);
static int rankcheck(const void *d1,const void *d2);

void ex_err(char *s)
{
	fprintf(stderr,"Fatal error:%s\n",s);
	tal_exit(1);
}

void cmd_err(char *s)
{
	fprintf(stderr,"(%s:%s) Usage: %s [options] infile1 infile2\n",__DATE__,TAL_VERSION,s);
	fprintf(stderr,"Options: -o(outputfile) file to output data to default:stdout\n");
	fprintf(stderr,"         -b(bucketsize) bucketsize for axis default:0.1\n");
	fprintf(stderr,"         -d(bucketsize) bucketsize for data default:10\n");
	fprintf(stderr,"         -n(numberofbuckets) number of buckets for data default:20\n");
	fprintf(stderr,"         -x(column) column to use as axis default:1\n");
	fprintf(stderr,"         -y(column) column to use as second axis default:2\n");
	fprintf(stderr,"         -2 perform comparisons along two axis default: single axis\n");
	fprintf(stderr,"Input is data lines of whitespace deliminated floats.\n");
	tal_exit(1);
}

#define	DLOOK(n,col)  ((n*ncols)+col)

double		buck_width = 10.0;
long int	num_buck = 20;

int main(int argc,char **argv)
{
	double		bucketsize = 0.1;
	double		c,d;
	long int	dcol = 1;
	long int	dcol2 = 2;
	long int	histcheck2d = 0;
	FILE		*output = stdout;
	FILE		*input;
	char		infile[2][256];
	
	double		*data[2];
	long int	ncols,npts[2],allocpts[2];
	double		dist[2][B_MAX];
	char		t1[256],*t2,text[256];
	long int 	i,j,k,l;
/* statistics */
	double		df,chi,prob;
	double		*sdata[2],ks,ksprob;
	double		u,mwprob;
	long int	n0,n1;

	i = 1;
	while ((i < argc) && (argv[i][0] == '-')) {
		if (argv[i][1] == '\0') break;
		switch (argv[i][1]) {
			case 'o':
				output = fopen(&(argv[i][2]),"w");
				if (output == 0L) {
		fprintf(stderr,"Unable to open output file %s\n",&(argv[i][2]));
					tal_exit(1);
				}
				break;
			case 'x':
				dcol = atoi(&(argv[i][2]));
				break;
			case 'y':
				dcol2 = atoi(&(argv[i][2]));
				break;
			case 'b':
				bucketsize = atof(&(argv[i][2]));
				break;
			case 'd':
				buck_width = atof(&(argv[i][2]));
				break;
			case '2':
				histcheck2d = 1;
				break;
			case 'n':
				num_buck = atoi(&(argv[i][2]));
				if (num_buck > B_MAX) num_buck = B_MAX;
				break;
			default:	
				cmd_err(argv[0]);
				break;
		}
		i++;
	}
	if ((argc-i) != 2) {
		cmd_err(argv[0]);
	}
/* allocate the initial data storage */
	data[0] = 0L; data[1] = 0L;
	ncols = -1;
	npts[0] = 0; npts[1] = 0;
	allocpts[0] = 0; allocpts[1] = 0;

strcpy(infile[0],"<None>");
strcpy(infile[1],"<None>");
j = 0;
l = i;
while(l<argc) {
	strcpy(infile[j],argv[l]);
	input = fopen(argv[l],"r");
	if (input == 0L) {
		fprintf(stdout,"Unable to open input file %s\n",argv[l]);
		tal_exit(1);
	}
#ifdef DEBUG
	fprintf(stderr,"Opening file %s\n",argv[l]);
#endif
/* read the data into memory */
	while (fgets(text,256,input) != NULL) {
		if (ncols == -1) {
/* search text for number of columns */
			strcpy(t1,text);
			t2 = strtok(t1,"\t ");
			ncols = 0;
			while (t2 != NULL) {
				ncols++;
				t2 = strtok(NULL,"\t ");
			}
#ifdef DEBUG
			fprintf(stderr,"Found %ld columns\n",ncols);
#endif
/* allocate initial memory */
			for(k=0;k<2;k++) {
				allocpts[k] = 500;
				npts[k] = 0;
			data[k] = (double *)malloc(sizeof(double)*allocpts[k]*ncols);
			if (data[k] == 0L) ex_err("Unable to allocate memory.");
			}
		}
		if (npts[j]+10 > allocpts[j]) {
			allocpts[j] += 500;  /* space for 500 more lines */
			data[j] = (double *)realloc(data[j],sizeof(double)*allocpts[j]*ncols);
			if (data[j] == 0L) ex_err("Unable to allocate memory.");
#ifdef DEBUG
			fprintf(stderr,"Allocated %ld rows\n",allocpts[j]);
#endif
		}
		strcpy(t1,text);
		t2 = strtok(t1,"\t ");
		i = 0;
		while ((t2 != NULL) && (i < ncols)) {
			data[j][DLOOK(npts[j],i)] = 0;
			sscanf(t2,"%lf",&(data[j][DLOOK(npts[j],i)]));
			i++;
			t2 = strtok(NULL,"\t ");
		}
		npts[j]++;
	}
#ifdef DEBUG
	fprintf(stderr,"Read %ld rows %ld columns\n",npts[j],ncols);
#endif
	fclose(input);
	l++;
	j++;
}

/* for each bin */
if (histcheck2d) {
	fprintf(output,"%s vs %s\n",infile[0],infile[1]);
	for(c=-1.0;c<=1.0;c=c+bucketsize) {
	for(d=-1.0;d<=1.0;d=d+bucketsize) {
		get_dist2d(data[0],npts[0],ncols,dcol,dcol2,c,d,bucketsize,
			dist[0],&(sdata[0]),&n0);
		get_dist2d(data[1],npts[1],ncols,dcol,dcol2,c,d,bucketsize,
			dist[1],&(sdata[1]),&n1);
/* check significance */
		if ((n0 == 0) || (n1 == 0)) {
			chi = 0; prob = 1.0;
			ks = 0; ksprob = 1.0;
			u = 0; mwprob = 1.0;
		} else {
		chstwo(dist[0],dist[1],num_buck,-1,&df,&chi,&prob);
		kstwo(sdata[0],n0,sdata[1],n1,&ks,&ksprob);
		mannwhitney(sdata[0],n0,sdata[1],n1,&u,&mwprob);
		}
/* output data */
		if (u < 0) {
		fprintf(output,"%5.2f,%5.2f:chi=%7.4f chip=%6.4f ks=%6.4f ksp=%6.4f u=%6.1f,%ld,%ld\n",
			c,d,chi,prob,ks,ksprob,-u,n0,n1);
		} else {
		fprintf(output,"%5.2f,%5.2f:chi=%7.4f chip=%6.4f ks=%6.4f ksp=%6.4f u=%6.1f mwp=%6.4f\n",
			c,d,chi,prob,ks,ksprob,u,mwprob);
		}
		free(sdata[0]);
		free(sdata[1]);
	}
	}
} else {
	fprintf(output,"%s vs %s\n",infile[0],infile[1]);
	for(c=-1.0;c<=1.0;c=c+bucketsize) {
		get_dist1d(data[0],npts[0],ncols,dcol,c,bucketsize,
			dist[0],&(sdata[0]),&n0);
		get_dist1d(data[1],npts[1],ncols,dcol,c,bucketsize,
			dist[1],&(sdata[1]),&n1);
/* check significance */
		if ((n0 == 0) || (n1 == 0)) {
			chi = 0; prob = 1.0;
			ks = 0; ksprob = 1.0;
			u = 0; mwprob = 1.0;
		} else {
		chstwo(dist[0],dist[1],num_buck,-1,&df,&chi,&prob);
		kstwo(sdata[0],n0,sdata[1],n1,&ks,&ksprob);
		mannwhitney(sdata[0],n0,sdata[1],n1,&u,&mwprob);
		}
/* output data */
		if (u < 0) {
		fprintf(output,"%5.2f:chi=%7.4f chip=%6.4f ks=%6.4f ksp=%6.4f u=%6.1f,%ld,%ld\n",
			c,chi,prob,ks,ksprob,-u,n0,n1);
		} else {
		fprintf(output,"%5.2f:chi=%7.4f chip=%6.4f ks=%6.4f ksp=%6.4f u=%6.1f mwp=%6.4f\n",
			c,chi,prob,ks,ksprob,u,mwprob);
		}
		free(sdata[0]);
		free(sdata[1]);
	}
}
/* cleanup and exit */
	if (output != stdout) fclose(output);
	for(k=0;k<1;k++) {
		if (data[k] != 0L) free(data[k]);
	}
	tal_exit(0);
	exit(0);
}

void get_dist1d(double *data,long int npts,long int ncols,long int dcol,
	double c,double bucketsize,double *dist,double **sdata,long int *n)
{
	long int i,j,alloc;
	
	alloc = 100;
	*sdata = (double *)malloc(sizeof(double)*alloc);
	if (*sdata == 0L) ex_err("Unable to allocate sample memory");
	*n = 0;

	for(i=0;i<B_MAX;i++) dist[i] = 0;
	for(i=0;i<npts;i++) {
		if (fabs((data[DLOOK(i,dcol)]-c)/bucketsize)<(bucketsize/2)) {
			j = data[DLOOK(i,0)]/B_WID;
			if (j<0) j = 0;
			if (j>=num_buck) j = num_buck-1;
			dist[j] += 1;
			if ((*n)+10 > alloc) {
				alloc += 100;
				*sdata = (double *)realloc(*sdata,
					sizeof(double)*alloc);
			}
			(*sdata)[*n] = data[DLOOK(i,0)];
			(*n)++;
		}
	}
	return;
}

void get_dist2d(double *data,long int npts,long int ncols,long int dcol,
	long int dcol2,double c,double d,double bucketsize,double *dist,
	double **sdata,long int *n)
{
	long int i,j,alloc;
	
	alloc = 100;
	*sdata = (double *)malloc(sizeof(double)*alloc);
	if (*sdata == 0L) ex_err("Unable to allocate sample memory");
	*n = 0;

	for(i=0;i<B_MAX;i++) dist[i] = 0;
	for(i=0;i<npts;i++) {
		if ((fabs((data[DLOOK(i,dcol)]-c)/bucketsize)<(bucketsize/2))
		&& (fabs((data[DLOOK(i,dcol2)]-d)/bucketsize)<(bucketsize/2))) {
			j = data[DLOOK(i,0)]/B_WID;
			if (j<0) j = 0;
			if (j>=num_buck) j = num_buck-1;
			dist[j] += 1;
			if ((*n)+10 > alloc) {
				alloc += 100;
				*sdata = (double *)realloc(*sdata,
					sizeof(double)*alloc);
			}
			(*sdata)[*n] = data[DLOOK(i,0)];
			(*n)++;
		}
	}
	return;
}

typedef struct {
	double	data;
	short	set;
	double	rank;
} mwdata;

static int rankcheck(const void *d1,const void *d2)
{
        mwdata 	*v1,*v2;

        v1 = (mwdata *)d1;
        v2 = (mwdata *)d2;

        if (v1->data > v2->data) return(1);
        if (v2->data > v1->data) return(-1);

        return(0);
}

void mannwhitney(double *data1,long int n1,double *data2,long int n2,
	double *u,double *prob)
{
	mwdata		*ranks;
	long int	i,j;
	double		sum,z,mean,stdev;
	long int 	k;

	*u = 0;
	*prob = -1;

	ranks = (mwdata *)malloc(sizeof(mwdata)*(n1+n2));
	if (ranks == 0L) return;

	j = 0;
	for(i=0;i<n1;i++) {
		ranks[j].data = data1[i];
		ranks[j].set = 1;
		ranks[j].rank = 0;
		j++;
	}
	for(i=0;i<n2;i++) {
		ranks[j].data = data2[i];
		ranks[j].set = 2;
		ranks[j].rank = 0;
		j++;
	}
	qsort(ranks,(n1+n2),sizeof(mwdata),rankcheck);
/* build the rankings */
	i = 0;
	while (i<(n1+n2)) {
		j=i;
		while ((ranks[i].data == ranks[j].data) && (j < (n1+n2))) j++;
		sum = 0;
		for(k=i;k<j;k++) {
			sum += (k+1);
		}
		sum = sum / (double)(j-i);
		for(k=i;k<j;k++) {
			ranks[k].rank = sum;
		}
		i = j;
	}
/* compute u */
	sum = 0;
	for(i=0;i<(n1+n2);i++) {
		if (ranks[i].set == 1) {
			sum += ranks[i].rank;
		}
	}
	*u = (n1*n2) + ((n1*(n1+1))/2) - sum;
	if (*u < ((n1*n2)-sum)) *u = (n1*n2)-sum;
/* compute the probability */
	if ((n1 > 20) || (n2 > 20)) {
		mean = (n1*n2)/2;
		stdev = sqrt((n1*n2*(n1+n2+1))/12.0);
		z = ((*u) - mean)/stdev;
/* normal distribution mean=0,var=1 */
		*prob = 1.0 - (2.0*(CDF_normal(fabs(z),NULL)-0.5));
	} else {
		*u = -(*u);
		*prob = 0;
	}
	free(ranks);
	return;
}
