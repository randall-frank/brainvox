/*
 * Copyright (c), 1990-2005 Randall Frank and Thomas Grabowski
 *
 * $Id $
 * $HeadURL $
 *
 *                University of Iowa, Department of Neurology,
 *                Human Neuroanatomy and Neuroimaging Lab
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
#include "cvio_lib.h"
#include "as274.h"

typedef struct {
   char in[256];
   char out[256];
} name_sub;

#define MAX_VARS        50
#define MAX_OBS         500
#define MAX_OUTPUTS     50

#define CR_OUT_SS_ERROR			0x0001
#define CR_OUT_SS_VARIABLE		0x0002
#define CR_OUT_P_SS_VARIABLE		0x0004
#define CR_OUT_SS_TOTAL			0x0008
#define CR_OUT_SS_MODEL			0x0010
#define CR_OUT_R2_MODEL			0x0020
#define CR_OUT_R2_ADJ_MODEL		0x0040
#define CR_OUT_COVARIABLE_SLOPE		0x0080
#define CR_OUT_COVARIABLE_STD_ERROR	0x0100
#define CR_OUT_COEF_COVARIANCE		0x0200
#define CR_OUT_N		        0x0400

typedef struct {
        int32_t		ndim;
        int32_t         d[3];   /* data shape */
	int32_t		dsize;
	int32_t		nsamples;
        int32_t		nslab;      /* temporal slabbing */
        int32_t         curadd_slab;   /* current slab */
        int32_t         currem_slab;   /* current slab */
        int             log_xform;  /* log transform input data */
        char		*maskname;
        unsigned char   *maskdata;      /* buffer for 8bit volume mask */
        char		*weightname;
	uint32_t	Fweight;        /* cvio output timestamp volume */
        float           *weightdata;    /* buffer for float volume mask */
        char            *output_prefix;
        char            *input_prefix;
        CVIO_DT_TIME    datawindow;     /* size of data window */
	char 		*outname;
	uint32_t	Fout;
	CVIO_DT_TIME	out_time;
	int32_t		out_type;
        int32_t		ingate;
} volparams;

#define VAR_Y_VAL       1
#define VAR_CONST       2
#define VAR_COV_V       3
#define VAR_COV_C       4
#define VAR_CLASS       5
#define VAR_LABEL       6
#define VAR_Y_CONST     7

/*
For VAR_Y_VAL: type=1,name=filename,label=yname,data=imagebuffer
For VAR_Y_VAL: type=7,name="CONST",label=yname,value=ConstYValue,
                        data=imagebuffer(values=ConstYValue)
For VAR_CONST: type=2,name="CONST",label="CONST",value=1.0
For VAR_COV_V: type=3,name=filename,label=varname,data=imagebuffer
For VAR_COV_C: type=4,label=varname,value=ConstCovValue
For VAR_CLASS: type=5,name=classvalue,label=classname,value=classindex
For VAR_CLASS: type=6,name=classvalue,label=classname,value=classindex
*/
typedef struct {
        int             type;
        char            name[256];
        char            label[100];
        double          value;
        float 		*data;
	uint32_t	Fid;
        CVIO_DT_TIME    tNext;
        CVIO_DT_TIME    t;
} var;

typedef struct {
        int        	nvars;
        var             vars[MAX_VARS];
} model;

typedef struct {
	int	np;
	int	nrbar;

	double	**r;
	double 	**dd;
	double 	**theta;

	double	*ssumy;
	double	*sumy;
	double	*sser;
        int     *nobs;

	double	*ss,*tol,*work,*work2,*xtmp;
	int 	*XofV;
	int	*vorder,*norder,*lindep;
} regression;

typedef struct _obs {
	CVIO_DT_TIME	time;
	float		*alloc;
	float		*vars[MAX_VARS];
	struct _obs	*next;
} observation;

#define OUT_RES_ERR     1
#define OUT_ADJ_Y_M     2
#define OUT_CLASS_M     3
#define OUT_COV_COEF    4
#define OUT_SS_TOTAL    5
#define OUT_VAR_SS      6
#define OUT_SS_MODEL    7
#define OUT_VAR_PSS     8

/* proposed */
#define OUT_PRESS       9
#define OUT_R2_PRED     10
#define OUT_R2          11
#define OUT_R2_ADJ      12
#define OUT_COV_STDERR  13
#define OUT_VAR_COV     14
#define OUT_N           15

/*
For OUT_RES_ERR: type=1,tempname=filename,data=imagebuffer(residual SSerror)
For OUT_COV_COEF:type=4,tempname=filename,classname=varname,var=index
For OUT_SS_TOTAL:type=5,tempname=filename
For OUT_VAR_SS:  type=6,tempname=filename,classname=varname,var=index
For OUT_SS_MODEL:type=7,tempname=filename
For OUT_VAR_PSS: type=8,tempname=filename,classname=varname,var=index
For OUT_R2:      type=11,tempname=filename
For OUT_R2_ADJ:  type=12,tempname=filenametemp
For OUT_COV_STDERR:type=13,tempname=filename,classname=varname,var=index
For OUT_VAR_COV: type=14,tempname=filename,classname=varname,
                classvalue=var2name,var=index,var2=index2

Others will follow, probably fit coefficients, other SS, F-ratios?

*/
typedef struct {
        int		type;
        char            classname[100];
        char            classvalue[100];
        int		var;
        int	        var2;
        char            tempname[256];
	uint32_t	Fid;
        float           *data;
} output_s;

/* globals */
extern int	       noutputs;
extern output_s        outputs[MAX_OUTPUTS];
extern model           modeltemp;
extern volparams       volume;
extern int	       iVerbose;

/* prototypes */
int     parse_input(int cvioinput, uint32_t cviofile, FILE *fp,int icomp, int dontcreate);
void    ex_err(char *s);
void 	printvar(FILE *fp,var *v,int ob);
void    printoutput(FILE *fp,output_s *o);
void    exit_cvio_error(int32_t err);
void    exit_cvio_error2(char *s,int32_t err);
int32_t reg_read_sample(uint32_t stream,void *ptr,int32_t type, CVIO_DT_TIME *t,
        int32_t wait);
void    freebuffers(void);
void setup_regression(void);
void cleanup_regression(void);
void addrem_observation(observation *o,int remove);
void output_streams(int value);
void cleanup_observations(void);
void free_observation(observation *o);
observation *new_observation(CVIO_DT_TIME t);
void prune_observations(CVIO_DT_TIME t);
double  calc_pss(int np,int nrbar,int *vorder,double *dd,
        double *r,double *theta,double *x,double *tol,int *norder,
        int k,int j);
double calc_var_cov(int np, int nrbar,double *dd,double *r, int var1, int var2);
void update_input_streams(void);
void prime_input_streams(void);
void add_sub_name(char *in,char *out);
void sub_name(char *replace);

