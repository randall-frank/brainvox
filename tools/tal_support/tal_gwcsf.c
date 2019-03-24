/*
 * Copyright (c), 1990-1999 Randall Frank and Thomas Grabowski
 *
 * $Id: tal_gwcsf.c 1213 2005-08-27 20:51:21Z rjfrank $
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

#ifdef USE_NRC

#include <time.h>
#include "proto.h"
#include "nr.h"
#include "nrutil.h"
#include "as274.h"

/* $Id: tal_gwcsf.c 1213 2005-08-27 20:51:21Z rjfrank $ */

#define MIN(a,b) (((a)<(b)?(a):(b)))
#define MAX(a,b) (((a)>(b)?(a):(b)))
#define LERP(a,l,h) ((l)+(((h)-(l))*(a)))

#define G_CSF_AMP 0
#define G_CSF_MEAN 1
#define G_CSF_WIDTH 2
#define G_GRAY_AMP 3
#define G_GRAY_MEAN 4
#define G_GRAY_WIDTH 5
#define G_WHITE_AMP 6
#define G_WHITE_MEAN 7
#define G_WHITE_WIDTH 8

#define G_CSFGRAY_AMP 9
#define G_GRAYWHITE_AMP 10

#define G_NUM_PARAMS 11

#define O_AMP 0
#define O_MEAN 1
#define O_WIDTH 2

#define P_CSF 0
#define P_GRAY 1
#define P_WHITE 2

/* three gaussians each with thress parameters */
#define MFIT	3
#define MACUR	gbl_MA
#define MA 	(MFIT*3)+2

/* options: */
#define OPT_NONE	 0x000000
#define OPT_LATECSF	 0x000001
#define OPT_LCKMCSF 	 0x000002
#define OPT_LCKWCSF 	 0x000004
#define OPT_LCKWGRY 	 0x000008
#define OPT_LCKWWHT 	 0x000010
#define OPT_PROBAMP 	 0x000020
#define OPT_PROBGNM 	 0x000040
#define OPT_PROBSAT 	 0x000080
#define OPT_PINTERP	 0x000100
#define OPT_USEPOLYFIT	 0x000200
#define OPT_UNLCKGBLCSFM 0x000400
#define OPT_UNLCKGBLCSFW 0x000800
#define OPT_WEIGHTEDPROB 0x001000
#define OPT_LVARAVG 	 0x002000
#define OPT_GVARAVG 	 0x004000
#define OPT_MIXNOTLOCAL	 0x008000
#define OPT_PROBMIX	 0x010000
#define OPT_FILLMISSING	 0x020000
#define OPT_USE_A	 0x040000
#define OPT_USE_B	 0x080000
#define OPT_PROBWMIX	 0x100000

#define POLY_MAX	15
#define MAX_FITS	100000

typedef struct {
	long int	loc[3];
	long int	size[3];
	double		histn;
} detail;

typedef struct {
	long int	count;
	long int	loc[3];
	double		GOF;
	double		params[MA];
} Fit;

typedef struct {
	long int	iOrder;
	long int	iNum;
	double		*pCoef[MA];		
} PolyCoef;

/* Globals */
long int verbose = 0;

float	gbl_a01 = 1.0e-4;
float	gbl_a12 = 1.0e-4;
float	gbl_a02 = 0.0;
float	gbl_c01 = 0.0;
float	gbl_c12 = 0.0;
float	gbl_c02 = 0.0;

long int	gbl_MA = (3*MFIT);

/* local prototypes */
void Estimate_Unknown_Fits(Fit *fits[MAX_FITS],long int *rectinc,
	long int *blocks, long int order, PolyCoef *pCoef,
	long int dx,long int dy,long int icount);
void Gen_XVec(double *xvec, long int *loc, long int order);
void Set_Aux_Value(long x,long y,long z,double *data);
void Get_Aux_Value(long x,long y,long z,double *data);
int parse_aux_vol(char *vol);
void free_aux_vol(void);
void init_aux_vol(long dx,long dy,long swab);

void	Calc_Histo(long int *loc,long int *delta,long int *vsize,
		long int *dd,unsigned char *data,unsigned char *mask,
		double *histo,double *histn,float *Fvar,double *avgvar);
void	Calc_Voxels(long int *loc,long int *delta,long int *dd,
		unsigned char *data,unsigned char *mask,
		long int *voxels,long int *voxeln,
		float *weights,double is,double *ip);

long int Calc_Gfit(double *histo,double histn,double *varavg,
	double *gauss,double *fit,long int init,long int options);

void save_params(char *temp,long int slice,double *params,double fit,
	detail *loc);
void output_params(FILE *fp,double *gaus,double GOF,detail *loc);

void save_poly(char *temp,long int slice,PolyCoef *coef);

void save_histo(char *temp,long int slice,double *histo,double n);
void smooth_histo(double *histo, double *n,int width);

long int Get_Fit(long int *loc,long int *rectinc,long int *blocks);
void Get_Prob(double *prob,double *params,double *gparams,
	long int *pixels,long int npixels,long int opts,float *weights);
long int Interp_Probs(long int *loc,Fit *thefit,Fit *fits[MAX_FITS],
	long int *rectinc,long int *blocks,long int options,PolyCoef *coef);
void Voxel_Interp_Probs(long int *loc,Fit *thefit,Fit *fits[MAX_FITS],
	long int *rectinc,long int *blocks);

void mygauss(float x, float *a, float *y, float *dyda, int na);

/* external */
void do_rim_cleanup(unsigned char *data,unsigned char *mask,
        unsigned char *Gb,unsigned char *Wb,unsigned char *Cb,
        unsigned char *Fb,long int *dd,long int *size,
        char *G_O_temp,char *W_O_temp,char *C_O_temp,
	long int istart,long istep,long int swab,long int options);


void ex_err(char *s)
{
	fprintf(stderr,"Fatal error:%s\n",s);
	exit(1);
}

void cmd_err(char *s)
{
	fprintf(stderr,"(%s:%s) Usage: %s [options] intemp masktemp Gout Wout Cout Fout\n",__DATE__,TAL_VERSION,s);
	fprintf(stderr,"Options: -x(dx) image dx default:256\n");
	fprintf(stderr,"         -y(dy) image dy default:256\n");
	fprintf(stderr,"         -z(dz) output image depth in bytes (1 or 4 only) default:4\n");
	fprintf(stderr,"         Note: the Fout volume is always float format (dz=4)\n");
	fprintf(stderr,"         -f(start) first image number default:1\n");
	fprintf(stderr,"         -l(end) last image number default:124\n");
	fprintf(stderr,"         -i(step) image number step default:1\n");
	fprintf(stderr,"         -b swap bytes default:no\n");
	fprintf(stderr,"         -sip(ipx[:ipy]) interpixel spacing default:1.0:1.0]\n");
	fprintf(stderr,"         -sis(interslice) interslice spacing default:1.0\n");
	fprintf(stderr,"         -p(params) save the run parameters to disk default:none\n");
	fprintf(stderr,"         -S(savebase) save the fit data to disk default:none\n");
	fprintf(stderr,"         -N(temp) save 8bit histogram index data to disk default:none\n");
	fprintf(stderr,"         -v(vv...) increasing levels of verbose output default:none\n");
	fprintf(stderr,"         -rs(x:y:z) define local rect sizes in pixels default:45:45:45\n");
	fprintf(stderr,"         -ri(x:y:z) define local rect increment in pixels default:10:10:10\n");
	fprintf(stderr,"         -rt(x:y:z) define local test rect in pixels default:1:1:1\n");
	fprintf(stderr,"         -rp(percent) percent of valid voxels in a rect default:0.10\n");
	fprintf(stderr,"         -GXY(value) change an initial value for the global fit. X=GWC Y=AWM\n");
	fprintf(stderr,"                 Default values:\n");
	fprintf(stderr,"                             (G)ray (W)hite (C)SF\n");
	fprintf(stderr,"                 (A)mplitude  0.012  0.016  0.002\n");
	fprintf(stderr,"                 (W)idth       13.0   13.0   13.0\n");
	fprintf(stderr,"                 (M)ean       157.0  210.0   70.0\n");
	fprintf(stderr,"          -help list additional advanced options\n");
	fprintf(stderr,"\nintemp=z1,masktemp=z1,Gout=Wout=Fout=Cout=zX\n");

	exit(1);
}

void advanced_options(void) 
{
	fprintf(stderr,"Advanced options:\n");
	fprintf(stderr,"         -g global estimate default:local\n");
	fprintf(stderr,"         -n skip global fit, use local fits only default:compute global fit\n");
	fprintf(stderr,"         -w(width) the width of the histogram smoothing kernal default:3\n");
	fprintf(stderr,"         -P(order) order of the invalid fit interp polynomial default:5\n");
	fprintf(stderr,"         -L(limit) local GOF rejection threshold (*global GOF) default:1000\n");
	fprintf(stderr,"         -a0(weight) define csf-gray mean weighting default:%f\n",gbl_a01);
	fprintf(stderr,"         -a1(weight) define gray-white mean weighting default:%f\n",gbl_a12);
	fprintf(stderr,"         -a2(weight) define csf-white mean weighting default:%f\n",gbl_a02);
	fprintf(stderr,"         -V(temp) read a local var .flt vol for var weighted fits (-O(vV))\n");
	fprintf(stderr,"         -RXY(temp) read a .flt vol for initial local fit values. X=GWCab Y=AWM\n");
	fprintf(stderr,"         -WXY(temp) write a .flt vol of the local fit values. X=GWCab Y=AWM\n");
	fprintf(stderr,"         -o(options) define a list of options default:SPKGWfmwabde\n");
	fprintf(stderr,"-o option characters: \n");
	fprintf(stderr,"            L - Late CSF entry into the fit\n");
	fprintf(stderr,"            M - Lock CSF mean to global fit mean\n");
	fprintf(stderr,"            W - Lock CSF width to global fit width\n");
	fprintf(stderr,"            m - Unlock global fit CSF mean\n");
	fprintf(stderr,"            w - Unlock global fit CSF width\n");
	fprintf(stderr,"            X - Lock Gray width to global fit width\n");
	fprintf(stderr,"            Y - Lock White width to global fit width\n");
	fprintf(stderr,"            A - Normalize probabilities to local amplitude\n");
	fprintf(stderr,"            G - Normalize probabilities to local area under the gaussian\n");
	fprintf(stderr,"            S - Saturate probabilities at ends\n");
	fprintf(stderr,"            I - Interpolate 8 local parameter fits\n");
	fprintf(stderr,"            K - Weight local voxel probabilities by distance from center voxel\n");
	fprintf(stderr,"            P - Use polynomial for local param estimates (overrides 'I')\n");
	fprintf(stderr,"            V - Use variance weighting globally (requires -V be specified)\n");
	fprintf(stderr,"            v - Use variance weighting locally (requires -V be specified)\n");
	fprintf(stderr,"            a - Include C-G mixture gaussian\n");
	fprintf(stderr,"            b - Include G-W mixture gaussian (requires a)\n");
	fprintf(stderr,"            c - Do not use mixture gaussians on local fits\n");
	fprintf(stderr,"            d - Use mixture gaussians for probability lookups\n");
	fprintf(stderr,"            e - Weight mixture gaussians for probability lookups\n");
	fprintf(stderr,"            f - Fill in missing fits by average\n");
	fprintf(stderr,"\n");

	exit(1);
}

int main(int argc,char **argv)
{
	long int	dx = 256;
	long int	dy = 256;
	long int	dz = 4;
	long int	istart = 1;
	long int	iend = 124;
	long int	istep = 1;
	long int	swab = 0;
	long int	global = 0;
	long int	no_global = 0;
	long int	user_init = 0;
	long int	smooth_width = 3;
	long int	iPolyorder = 5;

/* options: SPKGWfmwabde */
	long int	options = (OPT_PROBSAT | OPT_USEPOLYFIT | 
				   OPT_WEIGHTEDPROB | OPT_PROBGNM | 
				   OPT_LCKWCSF | OPT_FILLMISSING | 
				   OPT_UNLCKGBLCSFM | OPT_UNLCKGBLCSFW | 
				   OPT_USE_A | OPT_USE_B | OPT_PROBMIX | 
				   OPT_PROBWMIX);

	char		*savebase = 0L;
	char		*savecmdline = 0L;
	char		*vartemp = 0L;
	long int	count = 0;
	long int	nattempt = 0;
	long int	rectsiz[3] = {45,45,45};
	long int	rectinc[3] = {10,10,10};
	long int	recttst[3] = {1,1,1};
	double		countper = 0.10;
	double		GOFLimit = 1000.0;
	double		islice = 1.0;
	double		ipixel[2] = {1.0,1.0};

	Fit		*fits[MAX_FITS];
	Fit		thefit;
	double		prob[3];
	long int	num_fits = 0;
	long int	blocks[3];

	char		intemp[256],masktemp[256];
	char		G_O_temp[256],W_O_temp[256],C_O_temp[256],F_O_temp[256];
	char		tstr[256];
	char		*C_N_temp = 0L;
	long int	i,err,j,k,x,y,z;
	long int	icount;
	double		histo[256];
	double		varavg[256];
	float 		*pfTmp;
	double		histn;
	unsigned char	*mask;
	unsigned char	*data;
	float		*Fvar = NULL;
	float		*Gw,*Ww,*Fw,*Cw;
	unsigned char	*Cn,*Gb,*Wb,*Cb;
	long int	loc[3],dd[3],vsize[3];
	double		gauss[MA],fit,mincount;
	double		vfit[MA+1];
	long int	reject;
	long int	*voxels,voxeln;
	double		fitlims[2] = {1000,-1000};
	double		Gsum,Wsum,Csum;
	
	detail		desc;
	PolyCoef	coef;

	float		*weights = NULL;

	memset(&(coef),sizeof(PolyCoef),0);

/* initial guess values */
	gauss[G_WHITE_AMP] = 0.016;
	gauss[G_WHITE_MEAN] = 210.0;
	gauss[G_WHITE_WIDTH] = 13.0;

	gauss[G_GRAY_AMP] = 0.012;
	gauss[G_GRAY_MEAN] = 157.0;
	gauss[G_GRAY_WIDTH] = 13.0;

	gauss[G_CSF_AMP] = 0.002;
	gauss[G_CSF_MEAN] = 70.0;
	gauss[G_CSF_WIDTH] = 13.0;

	init_aux_vol(-1,-1,-1);

/* parse the command line */
	i = 1;
	while ((i < argc) && (argv[i][0] == '-')) {
		if (argv[i][1] == '\0') break;
		switch (argv[i][1]) {
			case 'h':
				advanced_options();
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
				if ((dz != 1) && (dz != 4)) ex_err(
					"Only 1 or 4 bytes/pixels supported");
				break;
			case 'w':
				smooth_width = atoi(&(argv[i][2]));
				break;
			case 'N':
				C_N_temp = (&(argv[i][2]));
				break;
			case 'b':
				swab = 1;
				break;
			case 'v':
				verbose = strlen(argv[i]) - 1;
				break;
			case 'V':
				vartemp = &(argv[i][2]);
				break;
			case 'P':
				iPolyorder = atoi(&(argv[i][2]));
				if ((iPolyorder > POLY_MAX) ||
				    (iPolyorder <= 0)) {
					ex_err("Max polygon order exceeded.");
				}
				break;
			case 'L':
				GOFLimit = atof(&(argv[i][2]));
				break;
			case 'g':
				global = 1;
				break;
			case 'n':
				no_global = 1;
				break;
			case 'G':
				user_init = 1;	
				j = 2;
				switch (argv[i][j]) {
					case 'G':
						j++;
						switch (argv[i][j]) {
							case 'A':
				j++;
				gauss[G_GRAY_AMP] = atof(&(argv[i][j]));
								break;
							case 'W':
				j++;
				gauss[G_GRAY_WIDTH] = atof(&(argv[i][j]));
								break;
							case 'M':
				j++;
				gauss[G_GRAY_MEAN] = atof(&(argv[i][j]));
								break;
							default:
			fprintf(stderr,"Unknown GG option:%c\n",argv[i][j]);
								break;
						}
						break;
					case 'W':
						j++;
						switch (argv[i][j]) {
							case 'A':
				j++;
				gauss[G_WHITE_AMP] = atof(&(argv[i][j]));
								break;
							case 'W':
				j++;
				gauss[G_WHITE_WIDTH] = atof(&(argv[i][j]));
								break;
							case 'M':
				j++;
				gauss[G_WHITE_MEAN] = atof(&(argv[i][j]));
								break;
							default:
			fprintf(stderr,"Unknown GW option:%c\n",argv[i][j]);
								break;
						}
						break;
					case 'C':
						j++;
						switch (argv[i][j]) {
							case 'A':
				j++;
				gauss[G_CSF_AMP] = atof(&(argv[i][j]));
								break;
							case 'W':
				j++;
				gauss[G_CSF_WIDTH] = atof(&(argv[i][j]));
								break;
							case 'M':
				j++;
				gauss[G_CSF_MEAN] = atof(&(argv[i][j]));
								break;
							default:
			fprintf(stderr,"Unknown GC option:%c\n",argv[i][j]);
								break;
						}
						break;
					default:
			fprintf(stderr,"Unknown G option:%c\n",argv[i][j]);
						break;
				}
				break;
			case 'R':
			case 'W':
				if (parse_aux_vol(argv[i])) {
			fprintf(stderr,"Unknown option:%s\n",argv[i]);
				}
				break;
			case 'a':
				switch(argv[i][2]) {
					case '0':
						gbl_a01 = atof(&(argv[i][3]));
						break;
					case '1':
						gbl_a12 = atof(&(argv[i][3]));
						break;
					case '2':
						gbl_a02 = atof(&(argv[i][3]));
						break;
					default:
			fprintf(stderr,"Unknown a option:%c\n",argv[i][2]);
						break;
				}
				break;
			case 'r':
				j = 2;
				switch(argv[i][j]) {
					case 'p':
						j++;
						countper = atof(&(argv[i][j]));
						break;
					case 'i':
						j++;
						get_xyz(rectinc,&(argv[i][j]));
						break;
					case 's':
						j++;
						get_xyz(rectsiz,&(argv[i][j]));
						break;
					case 't':
						j++;
						get_xyz(recttst,&(argv[i][j]));
						break;
					default:
			fprintf(stderr,"Unknown rect option:%c\n",argv[i][j]);
						break;
				}
				break;
			case 'S':
				savebase = &(argv[i][2]);
				break;
			case 's':
				if (strncmp(argv[i],"-sip",4) == 0) {
					get_d_xy(ipixel,&(argv[i][4]));
				} else if (strncmp(argv[i],"-sis",4) == 0) {
					islice = atof(&(argv[i][4]));
				} else {
					cmd_err(argv[0]);
				}
				break;
			case 'p':
				savecmdline = &(argv[i][2]);
				break;
			case 'o':
				options = OPT_NONE;
				j = 2;
				while(argv[i][j] != '\0') {
					switch(argv[i][j]) {
						case 'L':
							options |= OPT_LATECSF;
							break;
						case 'M':
							options |= OPT_LCKMCSF;
							break;
						case 'W':
							options |= OPT_LCKWCSF;
							break;
						case 'X':
							options |= OPT_LCKWGRY;
							break;
						case 'Y':
							options |= OPT_LCKWWHT;
							break;
						case 'A':
							options |= OPT_PROBAMP;
							break;
						case 'G':
							options |= OPT_PROBGNM;
							break;
						case 'S':
							options |= OPT_PROBSAT;
							break;
						case 'V':
							options |= OPT_GVARAVG;
							break;
						case 'v':
							options |= OPT_LVARAVG;
							break;
						case 'I':
							options |= OPT_PINTERP;
							break;
						case 'P':
							options |= OPT_USEPOLYFIT;
							break;
						case 'm':
							options |= OPT_UNLCKGBLCSFM;
							break;
						case 'w':
							options |= OPT_UNLCKGBLCSFW;
							break;
						case 'K':
							options |= OPT_WEIGHTEDPROB;
							break;
						case 'f':
							options |= OPT_FILLMISSING;
							break;
						case 'b':
							options |= OPT_USE_B;
							break;
						case 'a':
							options |= OPT_USE_A;
							break;
						case 'c':
							options |= OPT_MIXNOTLOCAL;
							break;
						case 'd':
							options |= OPT_PROBMIX;
							break;
						case 'e':
							options |= OPT_PROBWMIX;
							break;
						default:
			fprintf(stderr,"Unknown option:%c\n",argv[i][j]);
							break;
					}
					j++;
				}
				break;
			default:	
				cmd_err(argv[0]);
				break;
		}
		i++;
	}
	if ((argc-i) != 6) cmd_err(argv[0]);
	strcpy(intemp,argv[i]);
	strcpy(masktemp,argv[i+1]);
	strcpy(G_O_temp,argv[i+2]);
	strcpy(W_O_temp,argv[i+3]);
	strcpy(C_O_temp,argv[i+4]);
	strcpy(F_O_temp,argv[i+5]);

/* Setup MACUR */
	if (options & OPT_USE_A) MACUR = MA-1;
	if (options & OPT_USE_B) MACUR = MA;

/* get the image memory */
	icount = 0;
	for(i=istart;i<=iend;i=i+istep) icount++;

/* store the volume to be interpolated */
	data = (unsigned char *)malloc(dx*dy*icount);/*data to be classified*/
	if (data == 0L) ex_err("Unable to allocate image memory.");
	mask = malloc(dx*dy*icount); /* mask volume */
	if (mask == 0L) ex_err("Unable to allocate image memory.");
	Cn = malloc(dx*dy); /* count volume */
	if (Cn == 0L) ex_err("Unable to allocate image memory.");

	if (vartemp) {
		Fvar = (float *)malloc(dx*dy*icount*sizeof(float));/* var vol */
		if (Fvar == 0L) ex_err("Unable to allocate image memory.");
	}

	Gw = (float *)malloc(dx*dy*sizeof(float)); /* single slice */
	if (Gw == 0L) ex_err("Unable to allocate image memory.");
	Ww = (float *)malloc(dx*dy*sizeof(float)); /* single slice */
	if (Ww == 0L) ex_err("Unable to allocate image memory.");
	Cw = (float *)malloc(dx*dy*sizeof(float)); /* single slice */
	if (Cw == 0L) ex_err("Unable to allocate image memory.");
	Fw = (float *)malloc(dx*dy*sizeof(float)); /* single slice */
	if (Fw == 0L) ex_err("Unable to allocate image memory.");
/* bytewise pointers */
	Gb = (unsigned char *)Gw;
	Wb = (unsigned char *)Ww;
	Cb = (unsigned char *)Cw;

/* init the aux vols */
	init_aux_vol(dx,dy,swab);

/* read the images */
	fprintf(stderr,"Reading %ld byte dataset for gwcsf computation\n",
		(dx*dy*icount));

	j = 0;
	k = 0;
	for(i=istart;i<=iend;i=i+istep) {
/* read a mri slice */
		name_changer(intemp,i,&err,tstr);
		fprintf(stderr,"Reading the file:%s\n",tstr);
		bin_io(tstr,'r',&(data[j]),dx,dy,1,1,swab,0L);

/* read a mask slice */
		name_changer(masktemp,i,&err,tstr);
		fprintf(stderr,"Reading the mask file:%s\n",tstr);
		bin_io(tstr,'r',&(mask[k]),dx,dy,1,1,swab,0L);

/* read the variance volume */
		if (vartemp) {
			name_changer(vartemp,i,&err,tstr);
			fprintf(stderr,"Reading the variance file:%s\n",tstr);
			bin_io(tstr,'r',&(Fvar[k]),dx,dy,sizeof(float),1,
				swab,0L);
		}

		j = j + (dx*dy*1);
		k = k + (dx*dy);
	}

/* minimum number of counts in a rectangle */
	mincount = countper*(rectsiz[0]*rectsiz[1]*rectsiz[2]);

/* setup volume paramters */
	desc.loc[0] = 0; 	desc.size[0] = dx;		dd[0] = 1;
	desc.loc[1] = 0; 	desc.size[1] = dy;		dd[1] = dx;
	desc.loc[2] = 0; 	desc.size[2] = icount;		dd[2] = dx*dy;

/* compute volume histogram */
	for(i=0;i<3;i++) vsize[i] = desc.size[i];
	pfTmp = (options & OPT_GVARAVG) ? Fvar : (float *)0L;
	Calc_Histo(desc.loc,desc.size,vsize,dd,data,mask,histo,&histn,
		pfTmp,varavg);
	smooth_histo(histo,&histn,smooth_width);
	desc.histn = histn;
	if (savebase) save_histo(savebase,count,histo,histn);

	if (verbose) {
		fprintf(stderr,"Fitting gaussians to volume histogram\n");
		if (user_init) {
			fprintf(stderr,"User supplied starting points:\n");
			output_params(stderr,gauss,0.0,0L);
		}
	}
/* temporarily save off the alpha and fit values */
	gbl_c01 = gbl_a01;
	gbl_c12 = gbl_a12;
	gbl_c02 = gbl_a02;
	gbl_a01 = 0.0;
	gbl_a12 = 0.0;
	gbl_a02 = 0.0;
	for(i=0;i<MACUR;i++) vfit[i] = gauss[i];
	vfit[G_CSFGRAY_AMP] = (vfit[G_CSF_AMP]+vfit[G_GRAY_AMP])*0.5;
	vfit[G_GRAYWHITE_AMP] = (vfit[G_WHITE_AMP]+vfit[G_GRAY_AMP])*0.5;

/* get the CSF locking options */
	i = (OPT_LCKMCSF|OPT_LCKWCSF);
	if (options & OPT_UNLCKGBLCSFM) i &= (~OPT_LCKMCSF);
	if (options & OPT_UNLCKGBLCSFW) i &= (~OPT_LCKWCSF);

/* compute the best fit to the gaussians */
	if (!no_global) {
	    if (Calc_Gfit(histo,histn,varavg,gauss,&fit,1,i)) {
		ex_err("Unable to fit gaussians to the whole brain histogram");
	    }
	} else {
/* skip the global fit, use input values as global fit values */
	    fit = 1.0;
	}

/* restore the alpha values */
	gbl_a01 = gbl_c01;
	gbl_a12 = gbl_c12;
	gbl_a02 = gbl_c02;

/* save the run parameters: */
	if (savecmdline) {
		FILE 	*fp;
		time_t	t;

		t = time(NULL);

		fp = fopen(savecmdline,"w") ;
		if (fp == 0L) ex_err("Unable to write run params file");

		for(i=1;i<argc;i++) fprintf(fp,"%s ",argv[i]);
		fprintf(fp,"\n");

		fprintf(fp,"#%s - %s", argv[0], ctime(&t) );
		fprintf(fp,"# Starting params:\n");
		fprintf(fp,"#\tGM      \tWM      \tCSF\n");
		fprintf(fp,"#A\t%f\t%f\t%f\n",
		  vfit[G_GRAY_AMP],vfit[G_WHITE_AMP],vfit[G_CSF_AMP]);
		fprintf(fp,"#M\t%f\t%f\t%f\n",
		  vfit[G_GRAY_MEAN],vfit[G_WHITE_MEAN],vfit[G_CSF_MEAN]);
		fprintf(fp,"#W\t%f\t%f\t%f\n",
		  fabs(vfit[G_GRAY_WIDTH]),fabs(vfit[G_WHITE_WIDTH]),fabs(vfit[G_CSF_WIDTH]));
		fprintf(fp,"#a0\t%f\ta1\t%f\ta2\t%f\n",gbl_a01,gbl_a12,gbl_a02);
		fprintf(fp,"# Global fit params:\n");
		fprintf(fp,"#\tGM      \tWM      \tCSF\n");
		fprintf(fp,"#A\t%f\t%f\t%f\n",
		  gauss[G_GRAY_AMP],gauss[G_WHITE_AMP],gauss[G_CSF_AMP]);
		fprintf(fp,"#M\t%f\t%f\t%f\n",
		  gauss[G_GRAY_MEAN],gauss[G_WHITE_MEAN],gauss[G_CSF_MEAN]);
		fprintf(fp,"#W\t%f\t%f\t%f\n",
		  fabs(gauss[G_GRAY_WIDTH]),fabs(gauss[G_WHITE_WIDTH]),fabs(gauss[G_CSF_WIDTH]));

		fclose(fp);
	}

/* save the fit params */
	if (savebase) save_params(savebase,count,gauss,fit,&desc);
	fitlims[0] = MIN(fit,fitlims[0]);
	fitlims[1] = MAX(fit,fitlims[1]);
	count++;
	nattempt++;

/* save the volume fits... */
	for(i=0;i<MACUR;i++) vfit[i] = gauss[i];
	vfit[MACUR] = fit;

/* if no local mixtures, reduce MACUR */
	if (options & OPT_MIXNOTLOCAL) MACUR = (MFIT*3);

/* get ready to save the local fits into main memory */
	num_fits = 1;
	for(i=0;i<3;i++) {
		blocks[i]=(desc.size[i]/rectinc[i])+1;
		num_fits = num_fits*blocks[i];
	}
	if (verbose) fprintf(stderr,"Fit grid size (num): %ld %ld %ld (%ld)\n",
		blocks[0],blocks[1],blocks[2],num_fits);
	if (num_fits >= MAX_FITS) {
		ex_err("Sampling grid is too dense");
	}
	for(i=0;i<num_fits;i++) fits[i] = 0L;

/* array to hold voxels for local testing */
	voxeln = (recttst[0]*recttst[1]*recttst[2]);
	if (voxeln < 1) ex_err("Voxel sample space is too small");
	voxels = (long int *)malloc(sizeof(long int)*voxeln);
	if (voxels == 0L) ex_err("Unable to allocate voxel array memory");

/* abort early... */
	if (global) goto cleanup;

/* do the work */
	if (verbose) fprintf(stderr,"Fitting gaussians to local histogram\n");

	for(i=0;i<3;i++) desc.size[i] = rectsiz[i];

	for(desc.loc[2]=0;desc.loc[2]<(icount); desc.loc[2]+=rectinc[2]) {

/* read a slice */
	Get_Aux_Value(0,0,desc.loc[2]+1,(double *)0L);

	for(desc.loc[1]=0;desc.loc[1]<(dy); desc.loc[1]+=rectinc[1]) {
	for(desc.loc[0]=0;desc.loc[0]<(dx); desc.loc[0]+=rectinc[0]) {
/* lower left corner is offset by 1/2 the size */
		for(i=0;i<3;i++) loc[i] = desc.loc[i] - (0.5*desc.size[i]);
/* compute local histogram */
		pfTmp = (options & OPT_LVARAVG) ? Fvar : (float *)0L;
		Calc_Histo(loc,desc.size,vsize,dd,data,mask,histo,&histn,
			pfTmp,varavg);
		smooth_histo(histo,&histn,smooth_width);
		desc.histn = histn;
/* compute the best fit to the gaussians */
		if (histn > mincount) {

/* save the volume fits... */
			for(i=0;i<MACUR;i++) gauss[i] = vfit[i];

/* count the number of attempts */
			nattempt += 1;

/* get any values from the base */
			Get_Aux_Value(desc.loc[0],desc.loc[1],desc.loc[2],
				gauss);

/* compute the global vars gbl_c01 and gbl_c12 */
/* difference in means between (csf and gray) and (gray and white) */
			gbl_c01 = (gauss[0+O_MEAN] - gauss[3+O_MEAN]);
			gbl_c12 = (gauss[3+O_MEAN] - gauss[6+O_MEAN]);
			gbl_c02 = (gauss[0+O_MEAN] - gauss[6+O_MEAN]);

			gauss[G_CSFGRAY_AMP] = (gauss[G_CSF_AMP]+
				gauss[G_GRAY_AMP])*0.5;
			gauss[G_GRAYWHITE_AMP] = (gauss[G_WHITE_AMP]+
				gauss[G_GRAY_AMP])*0.5;
			
			reject = 0;
			if (Calc_Gfit(histo,histn,varavg,gauss,&fit,0,options)){
if (verbose) fprintf(stderr,"Unable to fit gaussians to local histogram\n");
				reject = 1;
			} else {
/* is the fit ok? */
				long int ireject = -1;

				gauss[G_WHITE_WIDTH] = 
					fabs(gauss[G_WHITE_WIDTH]);
				gauss[G_GRAY_WIDTH] = 
					fabs(gauss[G_GRAY_WIDTH]);
				gauss[G_CSF_WIDTH] = 
					fabs(gauss[G_CSF_WIDTH]);

				reject = 0;
/* no need to negative test the mixture guassians */
				for (i=0;i<(MFIT*3);i++) {
/* it is OK for the CSF mean to go negative */
					if ((gauss[i] < 0) && 
						(i != G_CSF_MEAN)) {
						reject=1;
						ireject=i;
					}
				}
				if (gauss[G_WHITE_WIDTH] > 60.0) reject=2;
				if (gauss[G_GRAY_WIDTH] > 60.0) reject=2;
				if (gauss[G_CSF_WIDTH] > 60.0) reject=2;
				if ((gauss[G_WHITE_MEAN] - gauss[G_GRAY_MEAN]) 
				      <	0.0) reject = 3;
				if ((gauss[G_GRAY_MEAN] - gauss[G_CSF_MEAN]) 
				      <	0.0) reject = 4;

				if (reject == 1) {
	if (verbose) fprintf(stderr,"Fit rejected:negative parameter:%ld\n",
		ireject);
				} else if (reject == 2) {
	if (verbose) fprintf(stderr,"Fit rejected:width too large\n");
				} else if (reject == 3) {
	if (verbose) fprintf(stderr,"Fit rejected:GM mean > WM mean\n");
				} else if (reject == 4) {
	if (verbose) fprintf(stderr,"Fit rejected:CSF mean > GM mean\n");
				} else if (fit > GOFLimit*vfit[MACUR]) {
	if (verbose) fprintf(stderr,"Fit rejected:%f*GOF: %f %f\n",
		GOFLimit,fit,vfit[MACUR]);
					reject = 1;
				} else {
/* check on how well the constraints are working... */
					if (verbose > 1) {
						fprintf(stderr,
						    "Mean diffs: %f %f %f\n",
						    gbl_c01 - (gauss[0+O_MEAN]-
						    gauss[3+O_MEAN]),
						    gbl_c12 - (gauss[3+O_MEAN]-
						    gauss[6+O_MEAN]),
						    gbl_c02 - (gauss[0+O_MEAN]-
						    gauss[6+O_MEAN]));
					}

					fitlims[0] = MIN(fit,fitlims[0]);
					fitlims[1] = MAX(fit,fitlims[1]);
					if (savebase) {
						save_histo(savebase,count,
							histo,histn);
						save_params(savebase,count,
							gauss,fit,&desc);
					}
/* record the fit for later... */
					i = Get_Fit(desc.loc,rectinc,blocks);
					if ((i < 0) || (fits[i] != 0)) {
	ex_err("Invalid fit location accesed for storage");
					}
					if (fits[i] == 0) {
						fits[i]=(Fit *)malloc(
							sizeof(Fit));
						if (fits[i] == 0) {
	ex_err("Unable to allocate fit storage memory");
						}
						for(j=0;j<3;j++) {
						    fits[i]->loc[j]=desc.loc[j];
						}
						for(j=0;j<MACUR;j++) {
						    fits[i]->params[j]=gauss[j];
						}
						fits[i]->GOF = fit;
						fits[i]->count = count;
					}
					count++;
				}
/* mark the ones to be filled in */
				if ((options & OPT_FILLMISSING) && 
					(reject != 0)) {
					i = Get_Fit(desc.loc,rectinc,blocks);
					if ((i >= 0) && (!fits[i])) {
						fits[i] = (Fit *)1;
					}
				}
			}
		}
	}
	}

/* done with a slice */
	Get_Aux_Value(0,0,desc.loc[2]+1,(double *)-1L);

	}
/* report any useful results */
	if (verbose) fprintf(stderr,"GOF min:%f max:%f\n",
		fitlims[0],fitlims[1]);

/* we are done with any variance volume */
	if (Fvar) {
		free(Fvar);
		Fvar = NULL;
	}

	if (options & OPT_FILLMISSING) {
		long int 	x,y,z;
		long int	offset[6][3] = {{1,0,0},{-1,0,0},
					{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};

/* Walk the fits, filling out neighbors via average */
		for(z=0;z<blocks[2];z++) {
		for(y=0;y<blocks[1];y++) {
		for(x=0;x<blocks[0];x++) {
			i = x + (y*blocks[0])+ (z*blocks[0]*blocks[1]);
			if (fits[i] == (Fit *)1) {
				long int	n = 0;

				fits[i]=(Fit *)malloc(sizeof(Fit));
				if (fits[i] == 0) {
	ex_err("Unable to allocate fit storage memory");
				}
			        fits[i]->loc[0]=x*rectinc[0]-(0.5*desc.size[0]);
			        fits[i]->loc[1]=y*rectinc[1]-(0.5*desc.size[1]);
			        fits[i]->loc[2]=z*rectinc[2]-(0.5*desc.size[2]);
				for(j=0;j<MACUR;j++) {
					fits[i]->params[j]=0;
				}
				fits[i]->GOF = 0;
				fits[i]->count = 0;

				for(k=0;k<6;k++) {
				  long int	off;
				  if (x+offset[k][0] < 0) continue;
				  if (x+offset[k][0] >= blocks[0]) continue;
				  if (y+offset[k][1] < 0) continue;
				  if (y+offset[k][1] >= blocks[1]) continue;
				  if (z+offset[k][2] < 0) continue;
				  if (z+offset[k][2] >= blocks[2]) continue;
				  off  = (offset[k][0]);
				  off += (offset[k][1])*blocks[0];
				  off += (offset[k][2])*blocks[0]*blocks[1];
				  if ((fits[i+off])  && 
					(fits[i+off] !=(Fit *)1)) {
				    for(j=0;j<MACUR;j++) {
					fits[i]->params[j] +=
						fits[i+off]->params[j];
				    }
				    fits[i]->GOF += fits[i+off]->GOF;
				    fits[i]->count += fits[i+off]->count;
				    n += 1;
				  }
				}
				if (n > 0) {
					for(j=0;j<MACUR;j++) {
						fits[i]->params[j] /= (float)n;
					}
					fits[i]->GOF /= (float)n;
					fits[i]->count /= (float)n;
	if (verbose > 1) fprintf(stderr,"Filling in %ld (%ld)\n",i,n);
				} else {
					free(fits[i]);
					fits[i] = NULL;
	if (verbose > 1) fprintf(stderr,"Bad Filling in %ld (%ld)\n",i,n);
				}
			}
		}
		}
		}
		for(i=0;i<blocks[0]*blocks[1]*blocks[2];i++) {
			if (fits[i] == (Fit *)1) {
				fits[i] = NULL;
				if (verbose > 1) {
				    fprintf(stderr,
					"Empty edge block %ld (%ld %ld %ld\n",
					i,blocks[0],blocks[1],blocks[2]);
				}
			}
		}
	}

/* use a global curve fit to estimate any dropped blocks */
        Estimate_Unknown_Fits(fits,rectinc,blocks,iPolyorder,&coef,
		dx,dy,icount);
	if (savebase) {
		save_poly(savebase,0,&coef);
	}

/* get memory to store the weights */
	weights = (float *)malloc(sizeof(float)*recttst[0]*
			recttst[1]*recttst[2]);
	if (!weights) ex_err("Unable to allocate weights memory");

/* compute G/W/CSF ratios */
	Gsum = 0.0;
	Wsum = 0.0;
	Csum = 0.0;

/* redistribute the wealth */
	z = 0;
	for(i=istart;i<=iend;i=i+istep) {
/* clear the slice */
		j = 0;
		for(y=0;y<dy;y++) {
			for(x=0;x<dx;x++) {
				if (dz == 1) {
					Gb[j] = 0;
					Wb[j] = 0;
					Cb[j] = 0;
				} else {
					Gw[j] = 0;
					Ww[j] = 0;
					Cw[j] = 0;
				}
				Fw[j] = 0.0;
				Cn[j] = 0;
				j++;
			}
		}
		Set_Aux_Value(0,0,-1,vfit);  /* clear the slice to global */
		if ((z >= (recttst[2]/2)) && (z<icount-(recttst[2]/2))) {
/* compute the slice */
			for(y=(recttst[1]/2);y<(dy-(recttst[1]/2));y++) {
			for(x=(recttst[0]/2);x<(dx-(recttst[0]/2));x++) {
/* the the value is not currently valid and is on the inside... */
				j = x + (y*dd[1]);
				if (mask[j+(z*dd[2])] > 0) {
/* get the local data */
					desc.loc[0] = x-(recttst[0]/2);
					desc.loc[1] = y-(recttst[1]/2);
					desc.loc[2] = z-(recttst[2]/2);
					Calc_Voxels(desc.loc,recttst,dd,
						data,mask,voxels,&voxeln,
						weights,islice,ipixel);

					desc.loc[0] = x;
					desc.loc[1] = y;
					desc.loc[2] = z;
					k = Interp_Probs(desc.loc,&thefit,fits,
						rectinc,blocks,options,&coef);

					if (verbose > 2) {
	fprintf(stderr,"Looking at %ld %ld %ld, Fit=%ld, Voxelnum=%ld\n",
		desc.loc[0],desc.loc[1],desc.loc[2],k,voxeln);
					}

					Set_Aux_Value(x,y,i,thefit.params);
				
					if (k >= 0) {
						Get_Prob(prob,thefit.params,
						   vfit,voxels,voxeln,options,
						   weights);

						if (verbose > 2) {
	fprintf(stderr,"Probs: %f %f %f, GOF=%f\n",
		prob[P_GRAY],prob[P_WHITE],prob[P_CSF],thefit.GOF);
						}
					
						if (dz == 1) {
							Gb[j]=255*prob[P_GRAY];
							Wb[j]=255*prob[P_WHITE];
							Cb[j]=255*prob[P_CSF];
						} else {
							Gw[j] = prob[P_GRAY];
							Ww[j] = prob[P_WHITE];
							Cw[j] = prob[P_CSF];
						}
						Fw[j] = thefit.GOF;
						Cn[j] = thefit.count;
/* compute G/W/CSF ratios */
						Gsum += prob[P_GRAY];
						Wsum += prob[P_WHITE];
						Csum += prob[P_CSF];
					}
				} 
				j++;
			}
			}
		}
/* write output slices */
		name_changer(G_O_temp,i,&err,tstr);
		fprintf(stderr,"Writing the Gray file:%s\n",tstr);
		bin_io(tstr,'w',Gw,dx,dy,dz,1,swab,0L);

		name_changer(W_O_temp,i,&err,tstr);
		fprintf(stderr,"Writing the White file:%s\n",tstr);
		bin_io(tstr,'w',Ww,dx,dy,dz,1,swab,0L);

		name_changer(C_O_temp,i,&err,tstr);
		fprintf(stderr,"Writing the CSF file:%s\n",tstr);
		bin_io(tstr,'w',Cw,dx,dy,dz,1,swab,0L);

		name_changer(F_O_temp,i,&err,tstr);
		fprintf(stderr,"Writing the GOF file:%s\n",tstr);
		bin_io(tstr,'w',Fw,dx,dy,sizeof(float),1,swab,0L);

		if (C_N_temp != 0l) {
			name_changer(C_N_temp,i,&err,tstr);
			fprintf(stderr,"Writing the count file:%s\n",tstr);
			bin_io(tstr,'w',Cn,dx,dy,1,1,swab,0L);
		}

/* write any aux volumes */
		Set_Aux_Value(0,0,i,(double *)-1L);

/* next plane */
		z = z + 1;
	}  /* over the z planes */

/* report G/W/CSF ratios */
	fit = Gsum + Wsum + Csum;
	fprintf(stdout,"Gray\t%f\tWhite\t%f\tCSF\t%f\n",
		Gsum/fit,Wsum/fit,Csum/fit);
	fprintf(stdout,"Fits\t%ld\t-\t%ld\n",nattempt,count);

	if (savecmdline) {
		FILE 	*fp;
		time_t	t;

		t = time(NULL);

		fp = fopen(savecmdline,"a") ;
		if (fp == 0L) ex_err("Unable to add to run params file");

		fprintf(fp,"#Gray\t%f\tWhite\t%f\tCSF\t%f\n",
			Gsum/fit,Wsum/fit,Csum/fit);
		fprintf(fp,"#Fits\t%ld\t-\t%ld\n",nattempt,count);
		fprintf(fp,"#%s - %s", "End", ctime(&t) );

		fclose(fp);
	}

/* done */	
cleanup:
	for(i=0;i<num_fits;i++) if (fits[i] != 0) free(fits[i]);
	for(i=0;i<MACUR;i++) if (coef.pCoef[i]) free(coef.pCoef[i]);
	free(voxels);
	free(data);
	free(mask);
	free(Gw);
	free(Ww);
	free(Cw);
	free(Fw);
	if (Fvar) free(Fvar);
	free(Cn);
	if (weights) free(weights);

/* free the aux vols */
	free_aux_vol();

	exit(0);
}


/* compute a histogram of the retangular region defined with LL corner at
	loc and with sides of length given by delta */
/* dd is the increment from one slice to another, data is the 8bit data
	mask is an 8bit mask volume,hsito is the histogram 
	and histn is the total number of counts */

void	Calc_Histo(long int *loc,long int *delta,long int *vsize,
		long int *dd,unsigned char *data,unsigned char *mask,
		double *histo,double *histn,float *Fvar,double *varavg)
{
	long int	i,j,k,p;

	for(i=0;i<256;i++) {
		histo[i] = 0.0;
		varavg[i] = 0.0;
	}
	*histn = 0.0;

/* compute the histogram */
	for(k=MAX(loc[2],0);k<MIN(loc[2]+delta[2],vsize[2]);k++) {
	for(j=MAX(loc[1],0);j<MIN(loc[1]+delta[1],vsize[1]);j++) {
	p = (k*dd[2]) + (j*dd[1]) + (loc[0]*dd[0]);
	for(i=MAX(loc[0],0);i<MIN(loc[0]+delta[0],vsize[0]);i++) {
		if (mask[p] > 0) {
			histo[data[p]] += 1.0;
			(*histn) += 1.0;
/* optionally compute the average variance */
			if (Fvar) varavg[data[p]] += Fvar[p];
		}
		p += dd[0];
	}
	}
	}

/* compute the average variance, or return an array of 1.0 */
	if (Fvar) {
		for(i=0;i<256;i++) if (histo[i] > 0.0) varavg[i] /= histo[i];
	} else {
		for(i=0;i<256;i++) varavg[i] = 1.0;
	}

	return;
}

void	Calc_Voxels(long int *loc,long int *delta,long int *dd,
		unsigned char *data,unsigned char *mask,
		long int *voxels,long int *voxeln,
		float *weights,double is,double *ip)
{
	long int	i,j,k,p;
	long int	off[3];

	*voxeln = 0;

/* compute the shift from 0,0,0 to the center of the block */
	for(i=0;i<3;i++) off[i] = delta[i]/2;

	for(k=loc[2];k<loc[2]+delta[2];k++) {
	for(j=loc[1];j<loc[1]+delta[1];j++) {
	p = (k*dd[2]) + (j*dd[1]) + (loc[0]*dd[0]);
	for(i=loc[0];i<loc[0]+delta[0];i++) {
		if (mask[p] > 0) {
			voxels[*voxeln] = data[p];
			if (weights) {
/* correct for anisotropy */
				double	dx,dy,dz,d;
				dx = (float)(i-loc[0]-off[0]);
				dy = (float)(j-loc[1]-off[1])*(ip[1]/ip[0]);
				dz = (float)(k-loc[2]-off[2])*(is/ip[0]);  
				d = dx*dx + dy*dy + dz*dz;
				if (d < 1.0) d = 1.0;
				d = sqrt(d);
				weights[*voxeln] = 1.0/d;
			}
			(*voxeln) += 1;
		}
		p += dd[0];
	}
	}
	}

	return;
}

/* global histogram limits */
long int	hmax,hmin;

/* compute the best fit of a sum of three gaussians to histo.  Return
	the params in gauss[] and the GOF in fit.  If init is 0, then 
	the input gauss[] values are to serve as initial estimates */
long int Calc_Gfit(double *histo,double histn,double *varavg,double *gauss,
	double *fit,long int init,long int options)
{
	int	ia[MA+1];
	float	guess[MA+1];

	float	x[256+50],y[256+50],sig[256+50];
	float	alamda,chisq,ochisq,**covar,**alpha;

	long int	i,npts,done;

/* search the histogram, copy the needed values */
	npts = 0;
	if (init) {  /* compute bounds only for gbl fit */
		hmax = 0;
		hmin = 255;
	}
	for(i=0;i<256;i++) {
		if (histo[i] > 0) {
			if (init) {
				if (i > hmax) hmax= i;
				if (i < hmin) hmin= i;
			}
			npts++;
			x[npts] = i;
			y[npts] = histo[i]/histn;
			sig[npts] = varavg[i];
			if (sig[npts] < 1.0) sig[npts] = 1.0;
		}
	}
	hmax = 255;

/* pad out the histogram to force zeros (20 values on each side) */
	for(i=1;i<=10;i++) {
		npts++;
		x[npts] = hmax+(i*5);
		y[npts] = 0;
		sig[npts] = 1.0;
	}
	for(i=1;i<=10;i++) {
		npts++;
		x[npts] = hmin-(i*5);
		y[npts] = 0;
		sig[npts] = 1.0;
	}
	
/* Add 10 values at x > 1000.0 for constraints (if needed) */	
	if ((gbl_a01 != 0.0) || (gbl_a12 != 0.0) || (gbl_a02 != 0.0)) {
		for(i=1100;i<1110;i++) {
			npts++;
			x[npts] = i;
			y[npts] = 0; /* constraint value => 0 */
			sig[npts] = 1.0;
		}
	}
	
/* copy the guess and allow some variables to move */
	for(i=0;i<MACUR;i++) {
		ia[i+1] = 1;
		guess[i+1] = gauss[i];
	}
	if (verbose > 2) {
		fprintf(stderr,"Fitting gaussians to %ld points\n",npts);
		output_params(stderr,gauss,0.0,0L);
	}
/* disable CSF initially */
	if (options & OPT_LATECSF) {
		ia[G_CSF_AMP+1] = 0;
		ia[G_CSF_MEAN+1] = 0;
		ia[G_CSF_WIDTH+1] = 0;
		guess[G_CSF_AMP+1] = 1.0;
	}
	if (options & OPT_LCKMCSF) {
		ia[G_CSF_MEAN+1] = 0;
		guess[G_CSF_AMP+1] = 1.0;
	}
	if (options & OPT_LCKWCSF) {
		ia[G_CSF_WIDTH+1] = 0;
		guess[G_CSF_AMP+1] = 1.0;
	}
	if (options & OPT_LCKWGRY) {
		ia[G_GRAY_WIDTH+1] = 0;
	}
	if (options & OPT_LCKWWHT) {
		ia[G_WHITE_WIDTH+1] = 0;
	}

/* init the fit routine */
	covar=matrix(1,MACUR,1,MACUR);
	alpha=matrix(1,MACUR,1,MACUR);
	nr_error_flag = 0;
	alamda = -1;
	mrqmin(x,y,sig,npts,guess,ia,MACUR,covar,alpha,&chisq,mygauss,&alamda);
	if (nr_error_flag) goto err_out;

/* loop till convergence */
	done = 0;
	while(done < 6) {
		ochisq=chisq;
		mrqmin(x,y,sig,npts,guess,ia,MACUR,covar,alpha,&chisq,mygauss,
			&alamda);
		if (nr_error_flag) goto err_out;
		if (chisq > ochisq) {
			done = 0;
		} else if (fabs(ochisq-chisq) < 0.1) {
			done++;
/* late enable the CSF fitting... */
			if ((options & OPT_LATECSF) && (done == 3)) {
				if (verbose > 2) {
					fprintf(stderr,"Late converge added\n");
				}
				ia[G_CSF_AMP+1] = 1;
				ia[G_CSF_MEAN+1] = 1;
				ia[G_CSF_WIDTH+1] = 1;
				guess[G_CSF_AMP+1] = gauss[G_CSF_AMP];
/* restart??? */
				alamda = -1;
				mrqmin(x,y,sig,npts,guess,ia,MACUR,covar,alpha,
					&chisq,mygauss,&alamda);
				if (nr_error_flag) goto err_out;
			}
			if (verbose > 2) fprintf(stderr,"Converge %ld %g %g\n",done,ochisq,chisq);
		}
	}

/* get the final result */
	alamda=0.0;
	mrqmin(x,y,sig,npts,guess,ia,MACUR,covar,alpha,&chisq,mygauss,&alamda);
	if (nr_error_flag) goto err_out;

/* return the values */
	*fit = chisq;
	for(i=0;i<MACUR;i++) {
		gauss[i] = guess[i+1];
	}
	if (verbose > 2) {
		fprintf(stderr,"Final fit\n");
		output_params(stderr,gauss,*fit,0L);
	}

/* free up the matrix variables */
	free_matrix(alpha,1,MACUR,1,MACUR);
	free_matrix(covar,1,MACUR,1,MACUR);

	return(0); /* no error */

err_out:
/* free up the matrix variables */
	free_matrix(alpha,1,MACUR,1,MACUR);
	free_matrix(covar,1,MACUR,1,MACUR);

	return(1);
}

void smooth_histo(double *histo, double *n,int width)
{
	long int	i,j,k,num,w1,w2;
	double		tmp[256];
	double		count,sum;
	
	w1 = (width - 1)/2;
	w2 = width - 1 - w1;
	for(i=0;i<256;i++) {
		if (histo[i] > 0) {
			sum = histo[i];
			count = 1.0;
			j = i - 1;
			num = 0;
			while(j>=0) {
				if (histo[j] > 0) {
					sum += histo[j];
					count += 1.0;
					num += 1;
					if (num >= w1) break;
				}
				j--;
			}
			k = i + 1;
			num = 0;
			while(k<=255) {
				if (histo[k] > 0) {
					sum += histo[k];
					count += 1.0;
					num += 1;
					if (num >= w2) break;
				}
				k++;
			}
			tmp[i] = sum/count;
		} else {
			tmp[i] = 0;
		}
	}

#ifdef NEVER
/* fill in the zero bins!!! */
	for(i=1;i<256;i++) {
		if (tmp[i] == 0.0) {
			j = i - 1;
			while(j>=1) {
				if (tmp[j] > 0) break;
				j--;
			}
			k = i + 1;
			while(k<=255) {
				if (tmp[k] > 0) break;
				k++;
			}
			tmp[i]=LERP((double)(i-j)/(double)(k-j),tmp[j],tmp[k]);
		}
	}
#endif

	*n = 0;
	for(i=0;i<256;i++) {
		(*n) += tmp[i];
		histo[i] = tmp[i];
	}

	return;
}

void save_histo(char *temp,long int slice,double *histo,double n)
{
	char		tstr[256];
	FILE		*fp;
	long int	err,i;

	strcpy(tstr,temp);
	if (slice >= 0L) {
		name_changer(temp,slice,&err,tstr);
	}
	strcat(tstr,".hst");
	fp = fopen(tstr,"w");
	if (fp == 0L) ex_err("Unable to write histogram file");
	for(i=0;i<256;i++) {
		fprintf(fp,"%ld\t%f\n",i,histo[i]/n);
	}
	fclose(fp);

	return;
}

void save_poly(char *temp,long int slice,PolyCoef *pCoef)
{
	char		tstr[256];
	FILE		*fp;
	long int	err;
	long int	i,j;
	char		*names[MA] = {"CA","CM","CW","GA","GM","GW",
				      "WA","WM","WW","CGA","GWA"};

	strcpy(tstr,temp);
	if (slice >= 0L) {
		name_changer(temp,slice,&err,tstr);
	}
	strcat(tstr,".poly");
	fp = fopen(tstr,"w");
	if (fp == 0L) ex_err("Unable to write polynomial file");
	fprintf(fp,"File\t%s\n",tstr);

	fprintf(fp,"%ld %ld %ld\n",MACUR,pCoef->iOrder,pCoef->iNum);

	for(i=0;i<MACUR;i++) {
		fprintf(fp,"%s",names[i]);
		if (pCoef->pCoef[i]) {
			for(j=0;j<pCoef->iNum;j++) {
				fprintf(fp,"\t%f",pCoef->pCoef[i][j]);
			}
		} else {
			for(j=0;j<pCoef->iNum;j++) {
				fprintf(fp,"\t%f",1.0);
			}
		}
		fprintf(fp,"\n");
	}

	fprintf(fp,"\n");
	fclose(fp);

	return;
}

void save_params(char *temp,long int slice,double *params,double fit,
	detail *loc)
{
	char		tstr[256];
	FILE		*fp;
	long int	err;

	strcpy(tstr,temp);
	if (slice >= 0L) {
		name_changer(temp,slice,&err,tstr);
	}
	fp = fopen(tstr,"w");
	if (fp == 0L) ex_err("Unable to write params file");
	fprintf(fp,"File\t%s\n",tstr);
	output_params(fp,params,fit,loc);
	fprintf(fp,"\n");
	fclose(fp);

	return;
}

void output_params(FILE *fp,double *gaus,double GOF,detail *loc)
{
/* human readable form */
	if (loc != 0L) {
		fprintf(fp,"Location\t%ld %ld %ld\n",
			loc->loc[0],loc->loc[1],loc->loc[2]);
		fprintf(fp,"Size\t%ld %ld %ld %f\n",
			loc->size[0],loc->size[1],loc->size[2],loc->histn);
	}
	fprintf(fp,"GOF\t%f\n",GOF);
	fprintf(fp,"GRY\t%f\t%f\t%f\n",
		gaus[G_GRAY_AMP],gaus[G_GRAY_MEAN],gaus[G_GRAY_WIDTH]);
	fprintf(fp,"WHT\t%f\t%f\t%f\n",
		gaus[G_WHITE_AMP],gaus[G_WHITE_MEAN],gaus[G_WHITE_WIDTH]);
	fprintf(fp,"CSF\t%f\t%f\t%f\n",
		gaus[G_CSF_AMP],gaus[G_CSF_MEAN],gaus[G_CSF_WIDTH]);
	if (MACUR > (MFIT*3)) {
		fprintf(fp,"CSFGRY\t%f\t%f\t%f\n",
			gaus[G_CSFGRAY_AMP],
			(gaus[G_CSF_MEAN]+gaus[G_GRAY_MEAN])*0.5,
			(gaus[G_CSF_WIDTH]+gaus[G_GRAY_WIDTH])*0.5);
	}
	if (MACUR == MA) {
		fprintf(fp,"GRYWHT\t%f\t%f\t%f\n",
			gaus[G_GRAYWHITE_AMP],
			(gaus[G_WHITE_MEAN]+gaus[G_GRAY_MEAN])*0.5,
			(gaus[G_WHITE_WIDTH]+gaus[G_GRAY_WIDTH])*0.5);
	}

/* gnuplot form... */
	fprintf(fp,"Gnuplot:\n%f*exp(-((x-%f)/%f)**2.0)",
		gaus[G_GRAY_AMP],gaus[G_GRAY_MEAN],gaus[G_GRAY_WIDTH]);
	fprintf(fp,",%f*exp(-((x-%f)/%f)**2.0)",
		gaus[G_WHITE_AMP],gaus[G_WHITE_MEAN],gaus[G_WHITE_WIDTH]);
	fprintf(fp,",%f*exp(-((x-%f)/%f)**2.0)",
		gaus[G_CSF_AMP],gaus[G_CSF_MEAN],gaus[G_CSF_WIDTH]);
	if (MACUR > (MFIT*3)) {
		fprintf(fp,",%f*exp(-((x-%f)/%f)**2.0)\n",
			gaus[G_CSFGRAY_AMP],
			(gaus[G_CSF_MEAN]+gaus[G_GRAY_MEAN])*0.5,
			(gaus[G_CSF_WIDTH]+gaus[G_GRAY_WIDTH])*0.5);
	}
	if (MACUR == MA) {
		fprintf(fp,",%f*exp(-((x-%f)/%f)**2.0)\n",
			gaus[G_GRAYWHITE_AMP],
			(gaus[G_WHITE_MEAN]+gaus[G_GRAY_MEAN])*0.5,
			(gaus[G_WHITE_WIDTH]+gaus[G_GRAY_WIDTH])*0.5);
	}
	fprintf(fp,"\n");

	return;
}

/*
typedef struct {
        long int        count;
        long int        loc[3];
        double          GOF;
        double          params[MA];
} Fit;
*/
void Gen_XVec(double *xvec, long int *loc, long int order)
{
	long int	x,y,z,i;

	i = 0;
	for(z=0;z<order;z++) {
	for(y=0;y<order;y++) {
	for(x=0;x<order;x++) {
		xvec[i]  = pow((double)(loc[0]),(double)x);
		xvec[i] *= pow((double)(loc[1]),(double)y);
		xvec[i] *= pow((double)(loc[2]),(double)z);
		i += 1;
	}
	}
	}

	return;
}

void Estimate_Unknown_Fits(Fit *fits[MAX_FITS],long int *rectinc,
	long int *blocks,long int order, PolyCoef *pCoef,
	long int dx,long int dy,long int icount)
{
	long int 	i,j;
	long int	cur_param,loc[3];

        long int        np,nrbar,nobs,fail;
        double          sser,w;
        double          ysc;
        double          *dd,*r,*theta;
        double          *xvec,*tol,*work,*ss;
        long int        *lindep;
	long int	*valid;

/* count the number of observations (and mark them) */
	valid = (long int *)malloc(sizeof(long int)*
			blocks[0]*blocks[1]*blocks[2]);
	if (!valid) ex_err("Unable to allocate valid blocks.");
	nobs = 0;
	for(i=0;i<blocks[0]*blocks[1]*blocks[2];i++) {
		if (fits[i]) {
			valid[i] = 1;
			nobs += 1;
		} else {
			valid[i] = 0;
		}
	}

/* the fit is of order n^3 */
	np = order*order*order;

/* allocate memory */
        nrbar = np*(np-1)/2;
        r = (double *)malloc(nrbar*sizeof(double));
        dd = (double *)malloc(np*sizeof(double));
        ss = (double *)malloc(np*sizeof(double));
        theta = (double *)malloc(np*sizeof(double));
        tol = (double *)malloc(np*sizeof(double));
        work = (double *)malloc(np*sizeof(double));
        lindep = (long int *)malloc(np*sizeof(long int));
/* create the data vectors */
        xvec = (double *)malloc(np*sizeof(double));

/* check memory allocations */
        if ((r == 0) || (dd == 0) || (theta == 0) || (tol == 0)
             || (work == 0) || (lindep == 0) || (xvec == 0) || (ss == 0)) {
                ex_err("Unable to allocate regression memory space.");
        }

	if (verbose) fprintf(stderr,"Estimating fits for %ld  of %ld blocks\n",
			blocks[0]*blocks[1]*blocks[2]-nobs,
			blocks[0]*blocks[1]*blocks[2]);

/* for each parameter */
for(cur_param=0;cur_param<MACUR;cur_param++) {
	if (verbose) fprintf(stderr,"Working on parameter: %ld\n",cur_param);

/* setup a new regression */
        clear_(&np,&nrbar,dd,r,theta,&sser,&fail);

/* add the observations */
	for(i=0;i<blocks[0]*blocks[1]*blocks[2];i++) {
		if (valid[i]) {
			Gen_XVec(xvec,fits[i]->loc,order);
			ysc = fits[i]->params[cur_param];
                	w = 1;  /* all weighted the same */
                	includ_(&np,&nrbar,&w,xvec,&ysc,dd,r,theta,&sser,&fail);
		}
	}

/* set singular tolerances */
        tolset_(&np,&nrbar,dd,r,tol,work,&fail);
/* check for singularities */
        sing_(&np,&nrbar,dd,r,theta,&sser,tol,lindep,work,&fail);

/* extract needed regression info */
        ss_(&np,dd,theta,&sser,ss,&fail);  /* ss=SS left after adding var i */

/* get coefficients */
        regcf_(&np,&nrbar,dd,r,theta,tol,work,&np,&fail); /*work=coefficients*/

/* apply the curve */
	for(loc[2]=0;loc[2]<(icount); loc[2]+=rectinc[2]) {
	for(loc[1]=0;loc[1]<(dy); loc[1]+=rectinc[1]) {
	for(loc[0]=0;loc[0]<(dx); loc[0]+=rectinc[0]) {
		i = Get_Fit(loc,rectinc,blocks);
		if (i >= 0) {
		    if (fits[i] == 0) {
/* allocate a new Fit */
			fits[i] = (Fit *)malloc(sizeof(Fit));
			if (fits[i] == 0L) {
				ex_err("Unable to malloc interpolated fit");
			}
			fits[i]->GOF = 0.0;
			fits[i]->count = 0;
			fits[i]->loc[0] = loc[0];
			fits[i]->loc[1] = loc[1];
			fits[i]->loc[2] = loc[2];

/* we will use the polynomial to estimate the result */
			for(j=0;j<MACUR;j++) fits[i]->params[j] = 0.0;

		    }
/* estimate the parameter */
		    if (!valid[i]) {
			Gen_XVec(xvec,fits[i]->loc,order);
			ysc = 0.0;
			for(j=0;j<np;j++) ysc += work[j]*xvec[j];
			fits[i]->params[cur_param] = ysc;
		    }
		}
	}
	}
	}
/* save and return the fit coefficients */
	if (pCoef) {
		pCoef->iOrder = order;
		pCoef->iNum = np;
		pCoef->pCoef[cur_param] = work;
        	work = (double *)malloc(np*sizeof(double));
		if (!work) ex_err("Unable to allocate coef memory");
	}
}  /* next param */

/* done */
/* free up the memory */
        free(ss);
        free(r);
        free(dd);
        free(theta);
        free(tol);
        free(work);
        free(lindep);
        free(xvec);
        free(valid);

	return;
}


long int Get_Fit(long int *loc,long int *rectinc,long int *blocks)
{
	long int i;
	long int v[3];

	for(i=0;i<3;i++) v[i] = loc[i]/rectinc[i];

	if ((v[0] < 0) || (v[0] >= blocks[0])) return(-1);
	if ((v[1] < 0) || (v[1] >= blocks[1])) return(-1);
	if ((v[2] < 0) || (v[2] >= blocks[2])) return(-1);
	i = v[0] + (blocks[0]*v[1]) + (blocks[1]*blocks[0]*v[2]);

	return(i);
}

void Get_Prob(double *prob,double *params,double *gparams,
	long int *pixels,long int npixels,long int opts,float *weights)
{
	long int	j,v;
	double		tmp;
	double		wei = 1.0;
	
	prob[P_GRAY] = 0.0;
	prob[P_WHITE] = 0.0;
	prob[P_CSF] = 0.0;

/* sum up the probabilities */
	for(j=0;j<npixels;j++) {

		v = pixels[j];
		if (opts & OPT_PROBSAT) {
			if (v > params[G_WHITE_MEAN]) {
				v = params[G_WHITE_MEAN];
			}
			if (v < params[G_CSF_MEAN]) {
				v = params[G_CSF_MEAN];
			}
		}
		if ((opts & OPT_WEIGHTEDPROB) && (weights)) wei = weights[j];

/* using the default gaussian curve values */
		if (opts & OPT_PROBAMP) {
/* amplitude in the denominator */
                	tmp = (v-params[G_GRAY_MEAN]);
			tmp /= params[G_GRAY_WIDTH];
			prob[P_GRAY] += exp(-tmp*tmp)*wei;

                	tmp = (v-params[G_WHITE_MEAN]);
			tmp /= params[G_WHITE_WIDTH];
			prob[P_WHITE] += exp(-tmp*tmp)*wei;

                	tmp = (v-params[G_CSF_MEAN]);
			tmp /= params[G_CSF_WIDTH];
			prob[P_CSF] += exp(-tmp*tmp)*wei;

			if (opts & OPT_PROBMIX) {
				double	mean,width,par;
				if (MACUR > (MFIT*3)) {
					mean = (params[G_CSF_MEAN] +
						params[G_GRAY_MEAN])*0.5;
					width = (params[G_CSF_WIDTH] +
						params[G_GRAY_WIDTH])*0.5;
                			tmp = (v-mean);
					tmp /= width;
					tmp = exp(-tmp*tmp)*wei;
					par = 0.5;
/* weight the probability */
					if (opts & OPT_PROBWMIX) {
						par = (v-mean)/width;
						par = (par + 1.0)*0.5;
						if (par < 0.0) par = 0.0;
						if (par > 1.0) par = 1.0;
					}
					prob[P_CSF] += (1.0 - par)*tmp;
					prob[P_GRAY] += (par)*tmp;
				}
				if (MACUR == MA) {
					mean = (params[G_WHITE_MEAN] +
						params[G_GRAY_MEAN])*0.5;
					width = (params[G_WHITE_WIDTH] +
						params[G_GRAY_WIDTH])*0.5;
                			tmp = (v-mean);
					tmp /= width;
					tmp = exp(-tmp*tmp)*wei;
					par = 0.5;
/* weight the probability */
					if (opts & OPT_PROBWMIX) {
						par = (v-mean)/width;
						par = (par + 1.0)*0.5;
						if (par < 0.0) par = 0.0;
						if (par > 1.0) par = 1.0;
					}
					prob[P_GRAY] += (1.0 - par)*tmp;
					prob[P_WHITE] += (par)*tmp;
				}
			}

		} else if (opts & OPT_PROBGNM) {
/* area under the curve in denominator */
                	tmp = (v-params[G_GRAY_MEAN]);
			tmp /= params[G_GRAY_WIDTH];
			prob[P_GRAY] += (exp(-tmp*tmp)/
				fabs(params[G_GRAY_WIDTH]))*wei;

                	tmp = (v-params[G_WHITE_MEAN]);
			tmp /= params[G_WHITE_WIDTH];
			prob[P_WHITE] += (exp(-tmp*tmp)/
				fabs(params[G_WHITE_WIDTH]))*wei;

                	tmp = (v-params[G_CSF_MEAN]);
			tmp /= params[G_CSF_WIDTH];
			prob[P_CSF] += (exp(-tmp*tmp)/
				fabs(params[G_CSF_WIDTH]))*wei;

			if (opts & OPT_PROBMIX) {
				double	mean,width,par;
				if (MACUR > (MFIT*3)) {
					mean = (params[G_CSF_MEAN] +
						params[G_GRAY_MEAN])*0.5;
					width = (params[G_CSF_WIDTH] +
						params[G_GRAY_WIDTH])*0.5;
                			tmp = (v-mean);
					tmp /= width;
					tmp = (exp(-tmp*tmp)/
						fabs(width))*wei;
					par = 0.5;
					if (opts & OPT_PROBWMIX) {
						par = (v-mean)/width;
						par = (par + 1.0)*0.5;
						if (par < 0.0) par = 0.0;
						if (par > 1.0) par = 1.0;
					}
					prob[P_CSF] += (1.0 - par)*tmp;
					prob[P_GRAY] += (par)*tmp;
				}
				if (MACUR == MA) {
					mean = (params[G_WHITE_MEAN] +
						params[G_GRAY_MEAN])*0.5;
					width = (params[G_WHITE_WIDTH] +
						params[G_GRAY_WIDTH])*0.5;
                			tmp = (v-mean);
					tmp /= width;
					tmp = (exp(-tmp*tmp)/
						fabs(width))*wei;
					par = 0.5;
					if (opts & OPT_PROBWMIX) {
						par = (v-mean)/width;
						par = (par + 1.0)*0.5;
						if (par < 0.0) par = 0.0;
						if (par > 1.0) par = 1.0;
					}
					prob[P_GRAY] += (1.0 - par)*tmp;
					prob[P_WHITE] += (par)*tmp;
				}
			}

		} else {
/* prob = actual curve value */
                	tmp = (v-params[G_GRAY_MEAN]);
			tmp /= params[G_GRAY_WIDTH];
			prob[P_GRAY] += fabs(params[G_GRAY_AMP])*
				exp(-tmp*tmp)*wei;

                	tmp = (v-params[G_WHITE_MEAN]);
			tmp /= params[G_WHITE_WIDTH];
			prob[P_WHITE] += fabs(params[G_WHITE_AMP])*
				exp(-tmp*tmp)*wei;

                	tmp = (v-params[G_CSF_MEAN]);
			tmp /= params[G_CSF_WIDTH];
			prob[P_CSF] += fabs(params[G_CSF_AMP])*
				exp(-tmp*tmp)*wei;
			if (opts & OPT_PROBMIX) {
				double	mean,width,par = 0;
				if (MACUR > (MFIT*3)) {
					mean = (params[G_CSF_MEAN] +
						params[G_GRAY_MEAN])*0.5;
					width = (params[G_CSF_WIDTH] +
						params[G_GRAY_WIDTH])*0.5;
                			tmp = (v-mean);
					tmp /= width;
					tmp = fabs(params[G_CSFGRAY_AMP])*
						exp(-tmp*tmp)*wei;
					par = 0.5;
					if (opts & OPT_PROBWMIX) {
						par = (v-mean)/width;
						par = (par + 1.0)*0.5;
						if (par < 0.0) par = 0.0;
						if (par > 1.0) par = 1.0;
					}
					prob[P_CSF] += (1.0 - par)*tmp;
					prob[P_GRAY] += (par)*tmp;
				}
				if (MACUR == MA) {
					mean = (params[G_WHITE_MEAN] +
						params[G_GRAY_MEAN])*0.5;
					width = (params[G_WHITE_WIDTH] +
						params[G_GRAY_WIDTH])*0.5;
                			tmp = (v-mean);
					tmp /= width;
					tmp = fabs(params[G_GRAYWHITE_AMP])*
						exp(-tmp*tmp)*wei;
					if (opts & OPT_PROBWMIX) {
						par = (v-mean)/width;
						par = (par + 1.0)*0.5;
						if (par < 0.0) par = 0.0;
						if (par > 1.0) par = 1.0;
					}
					prob[P_GRAY] += (1.0 - par)*tmp;
					prob[P_WHITE] += (par)*tmp;
				}
			}
		}
	}

/* normalize the values */
	if (prob[P_GRAY] < 0.0) prob[P_GRAY] = 0.0;
	if (prob[P_WHITE] < 0.0) prob[P_WHITE] = 0.0;
	if (prob[P_CSF] < 0.0) prob[P_CSF] = 0.0;

	tmp = prob[P_GRAY] + prob[P_WHITE] + prob[P_CSF];
	if (tmp == 0.0) tmp = 1.0;  /* prevent divide by zero error */
	tmp = 1.0/tmp;

	prob[P_GRAY] *= tmp;
	prob[P_WHITE] *= tmp;
	prob[P_CSF] *= tmp;

	return;
}

/* fitting function */
void mygauss(float x, float *a, float *y, float *dyda, int na)
{
        int i;
        float fac,ex,arg;

/* start at 0.0 */
        *y=0.0;
	for(i=1;i<=na;i++) dyda[i] = 0.0;

/* penalties are for those values of x > 1000.0 */	
	if (x < 1000.0) {
/* perform the normal fit */	
            for (i=1;i<=(MFIT*3);i+=3) {
                arg=(x-a[i+O_MEAN])/a[i+O_WIDTH];
                ex=exp(-arg*arg);
                fac=a[i+O_AMP]*ex*2.0*arg;
                *y += a[i+O_AMP]*ex;
                dyda[i+O_AMP]=ex;
                dyda[i+O_MEAN]=fac/a[i+O_WIDTH];
                dyda[i+O_WIDTH]=fac*arg/a[i+O_WIDTH];

            }

/* add on the mixture gaussians */
#define M_CG_A (a[G_CSFGRAY_AMP+1])
#define M_CG_M ((a[G_CSF_MEAN+1] + a[G_GRAY_MEAN+1])*0.5)
#define M_CG_W ((a[G_CSF_WIDTH+1] + a[G_GRAY_WIDTH+1])*0.5)
#define M_GW_A (a[G_GRAYWHITE_AMP+1])
#define M_GW_M ((a[G_WHITE_MEAN+1] + a[G_GRAY_MEAN+1])*0.5)
#define M_GW_W ((a[G_WHITE_WIDTH+1] + a[G_GRAY_WIDTH+1])*0.5)
	    if (na > (MFIT*3)) {
		arg=(x-M_CG_M)/M_CG_W;
		ex=exp(-arg*arg);
		*y += M_CG_A*ex;
		dyda[1+G_CSFGRAY_AMP] = ex;

		fac = M_CG_A*(x-M_CG_M);
		fac /= (M_CG_W*M_CG_W);
		fac *= dyda[1+G_CSFGRAY_AMP];
		dyda[1+G_CSF_MEAN] += fac;
		dyda[1+G_GRAY_MEAN] += fac;

		fac = M_CG_A*(x-M_CG_M)*(x-M_CG_M);
		fac /= (M_CG_W*M_CG_W*M_CG_W);
		fac *= dyda[1+G_CSFGRAY_AMP];
		dyda[1+G_CSF_WIDTH] += fac;
		dyda[1+G_GRAY_WIDTH] += fac;
	    }
	    if (na > (MFIT*3)+1) {
		arg=(x-M_GW_M)/M_GW_W;
		ex=exp(-arg*arg);
		*y += M_GW_A*ex;
		dyda[1+G_GRAYWHITE_AMP] = ex;

		fac = M_GW_A*(x-M_GW_M);
		fac /= (M_GW_W*M_GW_W);
		fac *= dyda[1+G_CSFGRAY_AMP];
		dyda[1+G_WHITE_MEAN] += fac;
		dyda[1+G_GRAY_MEAN] += fac;

		fac = M_GW_A*(x-M_GW_M)*(x-M_GW_M);
		fac /= (M_GW_W*M_GW_W*M_GW_W);
		fac *= dyda[1+G_CSFGRAY_AMP];
		dyda[1+G_WHITE_WIDTH] += fac;
		dyda[1+G_GRAY_WIDTH] += fac;
	    }
#undef M_CG_A
#undef M_CG_M
#undef M_CG_W
#undef M_GW_A
#undef M_GW_M
#undef M_GW_W

	} else {
/* tack on the following terms (prevent cross overs and stretching):
	a01*(MEAN0-MEAN1-(omean0-omean1))^2
	a12*(MEAN1-MEAN2-(omean1-omean2))^2
	a02*(MEAN0-MEAN2-(omean0-omean2))^2
*/
	    if (gbl_a01 != 0.0) {
		*y += gbl_a01*(a[1+O_MEAN]-a[4+O_MEAN]-gbl_c01)*
			(a[1+O_MEAN]-a[4+O_MEAN]-gbl_c01);
		dyda[1+O_MEAN] += 
			gbl_a01*( 2.0*a[1+O_MEAN]-2.0*a[4+O_MEAN]-2.0*gbl_c01);
		dyda[4+O_MEAN] += 
			gbl_a01*(-2.0*a[1+O_MEAN]+2.0*gbl_c01+2.0*a[4+O_MEAN]);
	    }
	    if (gbl_a12 != 0.0) {
		*y += gbl_a12*(a[4+O_MEAN]-a[7+O_MEAN]-gbl_c12)*
			(a[4+O_MEAN]-a[7+O_MEAN]-gbl_c12);
		dyda[4+O_MEAN] += 
			gbl_a12*( 2.0*a[4+O_MEAN]-2.0*a[7+O_MEAN]-2.0*gbl_c12);
		dyda[7+O_MEAN] += 
			gbl_a12*(-2.0*a[4+O_MEAN]+2.0*gbl_c12+2.0*a[7+O_MEAN]);
	    }
	    if (gbl_a02 != 0.0) {
		*y += gbl_a02*(a[1+O_MEAN]-a[7+O_MEAN]-gbl_c02)*
			(a[1+O_MEAN]-a[7+O_MEAN]-gbl_c02);
		dyda[1+O_MEAN] += 
			gbl_a02*( 2.0*a[1+O_MEAN]-2.0*a[7+O_MEAN]-2.0*gbl_c02);
		dyda[7+O_MEAN] += 
			gbl_a02*(-2.0*a[1+O_MEAN]+2.0*gbl_c02+2.0*a[7+O_MEAN]);
	    }
	}
	return;
}

long int Interp_Probs(long int *loc,Fit *thefit,Fit *fits[MAX_FITS],
	long int *rectinc,long int *blocks,long int options, PolyCoef *coef)
{
	long int	del[26][3] = {

{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},     {1,1,0},{-1,1,0},{-1,-1,0},{1,-1,0},
{1,0,1},{-1,0,1},{0,1,1},{0,-1,1},     {1,1,1},{-1,1,1},{-1,-1,1},{1,-1,1},
{1,0,-1},{-1,0,-1},{0,1,-1},{0,-1,-1}, {1,1,-1},{-1,1,-1},{-1,-1,-1},{1,-1,-1},
{0,0,-1},{0,0,1}
					};

	long int	k,kk,v[3];
	long int	i,j,mult;
	
	Fit		avgfit;
	double		num;
	double		xvec[POLY_MAX*POLY_MAX*POLY_MAX];

/* try the polygon approach */
	if (coef && (options & OPT_USEPOLYFIT)) {
		Gen_XVec(xvec,loc,coef->iOrder);
		for(i=0;i<MACUR;i++) {
		    	thefit->params[i] = 0.0;
		    	for(j=0;j<coef->iNum;j++) {
				thefit->params[i] += coef->pCoef[i][j]*xvec[j];
		    	}
		}
		thefit->loc[0] = loc[0];
		thefit->loc[1] = loc[1];
		thefit->loc[2] = loc[2];
		thefit->count = 0;
		thefit->GOF = 0;
		i = Get_Fit(loc,rectinc,blocks);
		if (i >= 0) thefit->GOF = fits[i]->GOF;

		return(0);
	}

/* otherwise, the the block fits */
	k = Get_Fit(loc,rectinc,blocks);
/* completely invalid case */
	if (k < 0) return(-1);

/* did not find a valid fit */
	if (fits[k] == 0) {

/* this should not happen anymore (thanks to the estimate curvefit code) */
		fprintf(stderr,
			"Warning: unknown fit block detected:%ld %ld %ld\n", 
			loc[0],loc[1],loc[2]);

/* build a valid fit from local neighbors */
		avgfit.GOF = 0.0;
		avgfit.count = 0;
		for(j=0;j<MACUR;j++) avgfit.params[j] = 0.0;
		num = 0;
		mult = 1;

		while(num == 0) {

		for(i=0;i<26;i++) {
			for(j=0;j<3;j++) {
				v[j] = loc[j] + mult*del[i][j]*rectinc[j];
			}
			kk = Get_Fit(v,rectinc,blocks);
			if (kk >= 0) {
				if (fits[kk]) {
					avgfit.GOF += fits[kk]->GOF;
					avgfit.count = fits[kk]->count;

					for(j=0;j<MACUR;j++) avgfit.params[j] 
						+= fits[kk]->params[j];

					num += 1.0;
				}
			}
		}

		mult++;
		if (mult > 3) break;
		}

		if (num == 0) {
			if (verbose > 1) {
fprintf(stderr,"Unable to interpolate params for a missing fit (%ld).\n",k);
			}
			return(-1);
		}
/* compute average and store */
		fits[k] = (Fit *)malloc(sizeof(Fit));
		if (fits[k] == 0L) ex_err("Unable to malloc interpolated fit");
		fits[k]->GOF = 0.0;
		fits[k]->count = avgfit.count;
		for(j=0;j<MACUR;j++) fits[k]->params[j] = (avgfit.params[j]/num);
		for(j=0;j<3;j++) fits[k]->loc[j] = loc[j];
	}

/* interpolate the params (nearest neighbor for right now) */
	if (options & OPT_PINTERP) {
		Voxel_Interp_Probs(loc,thefit,fits,rectinc,blocks);
	} else {
		*thefit = *fits[k];
	}

	return(k);
}

#define INTERPOLATE(a,b,w,out) \
	(out)->GOF = LERP((w),(a)->GOF,(b)->GOF); \
	{ long int ii; \
	for(ii=0;ii<MACUR;ii++) \
	    (out)->params[ii] = LERP((w),(a)->params[ii],(b)->params[ii]); \
	}

void Voxel_Interp_Probs(long int *loc,Fit *thefit,Fit *fits[MAX_FITS],
	long int *rectinc,long int *blocks)
{
	long int	v[3];
	long int	bl[9];
	long int	p[3];
	long int	i,j;
	float		w0;
	Fit		tmpFit[6];

/* find the nearest */
	bl[8] = Get_Fit(loc,rectinc,blocks);

/* find the eight nearest blocks */
	i = 0;
	for(i=0;i<8;i++) {
		if (i & 1) {
			v[0] = loc[0] + (rectinc[0]/2);
		} else {
			v[0] = loc[0] - (rectinc[0]/2);
		}
		if (i & 2) {
			v[1] = loc[1] + (rectinc[1]/2);
		} else {
			v[1] = loc[1] - (rectinc[1]/2);
		}
		if (i & 4) {
			v[2] = loc[2] + (rectinc[2]/2);
		} else {
			v[2] = loc[2] - (rectinc[2]/2);
		}
/* pick the center of the first block */
		if (i == 0) {
			for(j=0;j<3;j++) {
				p[j] = (v[j]/rectinc[j]);
				p[j] = (p[j]*rectinc[j]) + (rectinc[j]/2);
			}
		}
		bl[i] = Get_Fit(v,rectinc,blocks);
/* copy the closest if no go... */
		if (fits[bl[i]] == 0) bl[i] = bl[8];
	}

/* interpolate in X 8->4 */
	w0 = (float)(loc[0] - p[0])/(float)(rectinc[0]);
	INTERPOLATE(fits[bl[0]],fits[bl[1]],w0,&(tmpFit[0]));
	INTERPOLATE(fits[bl[2]],fits[bl[3]],w0,&(tmpFit[1]));
	INTERPOLATE(fits[bl[4]],fits[bl[5]],w0,&(tmpFit[2]));
	INTERPOLATE(fits[bl[6]],fits[bl[7]],w0,&(tmpFit[3]));
/* interpolate in Y 4->2 */
	w0 = (float)(loc[1] - p[1])/(float)(rectinc[1]);
	INTERPOLATE(&(tmpFit[0]),&(tmpFit[1]),w0,&(tmpFit[4]));
	INTERPOLATE(&(tmpFit[2]),&(tmpFit[3]),w0,&(tmpFit[5]));
/* interpolate in Z 2->1 */
	w0 = (float)(loc[2] - p[2])/(float)(rectinc[2]);
	INTERPOLATE(&(tmpFit[4]),&(tmpFit[5]),w0,thefit);

	thefit->loc[0] = fits[bl[8]]->loc[0];
	thefit->loc[1] = fits[bl[8]]->loc[1];
	thefit->loc[2] = fits[bl[8]]->loc[2];
	thefit->count = fits[bl[8]]->count;

	return;
}

/* aux volumes for read/write */
char	*Aux_Read_Name[G_NUM_PARAMS];
char	*Aux_Write_Name[G_NUM_PARAMS];
float	*Aux_Read_Data[G_NUM_PARAMS];
float	*Aux_Write_Data[G_NUM_PARAMS];
long	aux_dx,aux_dy,aux_swab;

void init_aux_vol(long dx,long dy,long swab)
{
	long int	i;

	if ((dx < 0) || (dy < 0)) {
	    for(i=0;i<G_NUM_PARAMS;i++) {
		Aux_Read_Name[i] = 0L;
		Aux_Write_Name[i] = 0L;
		Aux_Read_Data[i] = 0L;
		Aux_Write_Data[i] = 0L;
	    }
	}

	aux_swab = swab;
	aux_dx = dx;
	aux_dy = dy;

	return;
}

void free_aux_vol()
{
	long int	i;
	for(i=0;i<G_NUM_PARAMS;i++) {
		if (Aux_Read_Data[i]) free(Aux_Read_Data[i]);
		if (Aux_Write_Data[i]) free(Aux_Write_Data[i]);
		Aux_Read_Data[i] = 0L;
		Aux_Write_Data[i] = 0L;
	}
}

int parse_aux_vol(char *vol)
{
	long int	which;

	if (strlen(vol) < 5) return(1);
	switch(vol[2]) {
		case 'C':
			which = 0;
			break;
		case 'G':
			which = 3;
			break;
		case 'W':
			which = 6;
			break;
		case 'a':
			which = 9;
			break;
		case 'b':
			which = 10;
			break;
		default:
			return(1);
			break;
	}

	if ((which >= 9) && (vol[3] != 'A')) return(1);

	switch(vol[3]) {
		case 'A':
			which += 0;
			break;
		case 'M':
			which += 1;
			break;
		case 'W':
			which += 2;
			break;
		default:
			return(1);
			break;
	}
	if (vol[1] == 'R') {
		Aux_Read_Name[which] = &(vol[4]);
		return(0);
	} else if (vol[1] == 'W') {
		Aux_Write_Name[which] = &(vol[4]);
		return(0);
	} else {
		return(1);
	}
	return(0);
}

void Get_Aux_Value(long x,long y,long z,double *data)
{
	long int	i,err;
	char		tstr[256];

/* if before a slice, read the files into memory */
	if (data == 0L) {
		for(i=0;i<G_NUM_PARAMS;i++) {
			if ((Aux_Read_Name[i]) && (!Aux_Read_Data[i])) {
				Aux_Read_Data[i] = (float *)malloc(
					aux_dx*aux_dy*sizeof(float));
if (!Aux_Read_Data[i]) ex_err("Unable to allocate aux volume memory");
			}
			if (Aux_Read_Data[i]) {
				name_changer(Aux_Read_Name[i],z,&err,tstr);
				fprintf(stderr,"Reading the file:%s\n",tstr);
				bin_io(tstr,'r',Aux_Read_Data[i],aux_dx,
					aux_dy,sizeof(float),1,aux_swab,0L);
			}
		}
		return;
/* after a slice, do nothing... */
	} else if (data == (double *)-1L) {
		return;
	}
	for(i=0;i<G_NUM_PARAMS;i++) {
		if (Aux_Read_Data[i]) {
			data[i] = Aux_Read_Data[i][x+(y*aux_dx)];
		}
	}
}

void Set_Aux_Value(long x,long y,long z,double *data)
{
	long int	i,err;
	char		tstr[256];
/* if before a slice, clear memory */
	if (z < 0) {
		for(i=0;i<G_NUM_PARAMS;i++) {
			if ((Aux_Write_Name[i]) && (!Aux_Write_Data[i])) {
				Aux_Write_Data[i] = (float *)malloc(
					aux_dx*aux_dy*sizeof(float));
if (!Aux_Write_Data[i]) ex_err("Unable to allocate aux volume memory");
			}
			if (Aux_Write_Data[i]) {
				for(err=0;err<aux_dx*aux_dy;err++) {
					Aux_Write_Data[i][err] = data[i];
				}
			}
		}
		return;
/* after a slice, write the file to disk */
	} else if (data == (double *)-1L) {
		for(i=0;i<G_NUM_PARAMS;i++) {
			if (Aux_Write_Data[i]) {
				name_changer(Aux_Write_Name[i],z,&err,tstr);
				fprintf(stderr,"Writing the file:%s\n",tstr);
				bin_io(tstr,'w',Aux_Write_Data[i],aux_dx,
					aux_dy,sizeof(float),1,aux_swab,0L);
			}
		}
		return;
	}
	for(i=0;i<G_NUM_PARAMS;i++) {
		if (Aux_Write_Data[i]) {
			Aux_Write_Data[i][x+(y*aux_dx)] = data[i];
		}
	}
}


/* callback functions */
void    vl_puts(char *in)
{
        fprintf(stderr,"%s\n",in);
}

#else
int main(int argc,char **argv)
{
	fprintf(stderr,"%s : requires Numerical Recipies in C code to run\n",
		argv[0]);
	exit(1);
}
void ex_err(char *s)
{
	fprintf(stderr,"Fatal error:%s\n",s);
	exit(1);
}
/* callback functions */
void    vl_puts(char *in)
{
        fprintf(stderr,"%s\n",in);
}

#endif
