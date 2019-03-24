
/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski
 *
 * $Id: tal_regress.h 1599 2006-01-22 03:45:10Z rjfrank $
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
	Defines for the tal_regress program
*/

#define MAX_VARS	100
#define MAX_OBS		10000
#define MAX_OUTPUTS	50

typedef struct {
	long int	dx;	/* image matrix X */
	long int	dy;	/* image matrix Y */
	long int	first;	/* image numbering scheme */
	long int	last;
	long int	step;
	long int	icount; /* number of slices */
	long int	swap;	/* 16 bit byte swapping */
	long int	cov_only;  /* adjust y by covariates only */
	long int	log_xform;  /* log transform input data */
	long int	single_fit;  /* Single curve fit to data */
	long int	need_press;  /* PRESS is to be computed */
	long int	y_boolean;   /* Y variable has discrete values 0-1 */
	double		offset;	/* data offset value */
	char		*masktemp;	/* mask volume image template */
	unsigned char	*maskdata;	/* buffer for 8bit volume mask */
	char		*weighttemp;	/* temp for weightvolume */
	float		*weightdata;	/* buffer for float volume mask */
	char		*output_prefix;
	char		*input_prefix;
} volparams;

#define	VAR_Y_VAL	1
#define	VAR_CONST	2
#define	VAR_COV_V	3
#define	VAR_COV_C	4
#define VAR_CLASS	5
#define VAR_LABEL	6
#define	VAR_Y_CONST 	7

/*
For VAR_Y_VAL: type=1,name=volumetemplate,label=yname,data=imagebuffer,dz
For VAR_Y_VAL: type=7,name="CONST",label=yname,value=ConstYValue,
			data=imagebuffer(values=ConstYValue),dz
For VAR_CONST: type=2,name="CONST",label="CONST",value=1.0
For VAR_COV_V: type=3,name=volumetemplate,label=varname,data=imagebuffer,
			mean=adjustmean,dz
For VAR_COV_C: type=4,label=varname,value=ConstCovValue,mean=adjustmean
For VAR_CLASS: type=5,name=classvalue,label=classname,value=classindex
For VAR_CLASS: type=6,name=classvalue,label=classname,value=classindex
*/
typedef struct {
	long int	type;
	char		name[256];
	char		label[256];
	double		value;
	double		mean;
	long int	dz;  
	unsigned char	*data;
} var;

typedef struct {
	long int	nvars;
	float		weight;
	var		*vars;
} model;

#define	OUT_RES_ERR	1
#define	OUT_ADJ_Y_M	2
#define	OUT_CLASS_M	3
#define	OUT_COV_COEF	4
#define	OUT_SS_TOTAL	5
#define	OUT_VAR_SS	6
#define	OUT_SS_MODEL	7
#define	OUT_VAR_PSS	8

/* proposed */
#define	OUT_PRESS	9
#define	OUT_R2_PRED	10
#define	OUT_R2		11
#define	OUT_R2_ADJ	12
#define	OUT_COV_STDERR	13

#define	OUT_VAR_COV	14

/*
For OUT_RES_ERR: type=1,tempname=filenametemp,data=imagebuffer(residual SSerror)
For OUT_ADJ_Y_M: type=2,tempname=filenametemp,data=imagebuffer(mean adj Y)
For OUT_CLASS_M: type=3,classname=class,tempname=filenametemp,classvalue=cl_val,
		var=index,data=imagebuffer(adjmean for class)
For OUT_COV_COEF:type=4,tempname=filenametemp,classname=varname,var=index
For OUT_SS_TOTAL:type=5,tempname=filenametemp
For OUT_VAR_SS:  type=6,tempname=filenametemp,classname=varname,var=index
For OUT_SS_MODEL:type=7,tempname=filenametemp
For OUT_VAR_PSS: type=8,tempname=filenametemp,classname=varname,var=index

For OUT_PRESS:   type=9,tempname=filenametemp
For OUT_R2_PRED: type=10,tempname=filenametemp
For OUT_R2:      type=11,tempname=filenametemp
For OUT_R2_ADJ:  type=12,tempname=filenametemp
For OUT_COV_STDERR:type=13,tempname=filenametemp,classname=varname,var=index
For OUT_VAR_COV: type=14,tempname=filenametemp,classname=varname,
		classvalue=var2name,var=index,var2=index2

Others will follow, probably fit coefficients, other SS, F-ratios?

*/
typedef struct {
	long int	type;
	char		classname[256];
	char		classvalue[256];
	long int	var;
	long int	var2;
	char		tempname[256];
	float		*data;
} output_s;

/* prototypes */
int     parse_input(FILE *fp);
void 	freebuffers(void);
void 	printvar(FILE *fp,var *v,long int ob);
void 	printoutput(FILE *fp,output_s *o);
long int	findvar(char *name);
void 	ex_err(char *s);

void 	regress_one_setup(void);
void 	regress_one_data(long int xp,long int yp, float weight);
void 	regress_one_output(void);
void 	regress_one_cleanup(void);

void get_data_vector(double *y,double *x,long int i,long int *XofV,
        long int xp,long int yp);
double calc_adj_y(long int i,long int skip,double *coef,long int np,
        long int xp,long int yp);
double calc_resid_y(long int i,double *coef,long int np,long int xp,
        long int yp);
double  calc_pss(long int np,long int nrbar,long int *vorder,double *dd,
        double *r,double *theta,double *x,double *tol,long int *norder,
        long int k,long int j);
double calc_var_cov(long int np, long int nrbar,double *dd,double *r,
        long int var1, long int var2);

/* global variables */
#ifdef GLOBAL_DEFS
long int	nobs = 0;
model		obs[MAX_OBS];
long int	noutputs = 0;
output_s	outputs[MAX_OUTPUTS];
model		modeltemp;
volparams	volume;
long int	verbose = 0;
#else
extern long int		nobs;
extern model		obs[MAX_OBS];
extern long int		noutputs;
extern output_s		outputs[MAX_OUTPUTS];
extern model		modeltemp;
extern volparams	volume;
extern long int		verbose;
#endif
