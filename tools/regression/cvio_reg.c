
/*
 * Copyright (c), 1990-2000 Randall Frank and Thomas Grabowski
 *
 * $Id: cvio_reg.c 306 2003-06-22 01:46:11Z rjfrank $
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
#include "export.h"
#include "regression.h"

/*
 * TODO:
 */

#define ARRLEN(arr) (sizeof(arr)/sizeof(arr[0]))

typedef struct _rlink {
	struct _rlink *next;
	Regression	*reg;
	IDL_ULONG	count;
} rlink;

static	rlink		*rlist = NULL;
static	IDL_ULONG	rbase = 100;

static Regression	*find_reg(IDL_ULONG count);

static void			remove_reg(Regression *reg)
{
	rlink	*p = rlist;
	rlink	*prev = NULL;

	if (reg == NULL) return;
	while(p) {
		if (p->reg == reg) {
			reg_destroy(reg);
			if (prev) {
				prev->next = p->next;
			} else {
				rlist = p->next;
			}
			IDL_MemFree( p, "", IDL_MSG_RET);
			return;
		}
		prev = p;
		p = p->next;
	}
	return;
}

static Regression	*find_reg(IDL_ULONG count)
{
	rlink	*p = rlist;

	while(p) {
		if (p->count == count) return(p->reg);
		p = p->next;
	}
	return(NULL);
}


/*
 * Define message codes and their corresponding printf(3) format
 * strings. Note that message codes start at zero and each one is
 * one less that the previous one. Codes must be monotonic and
 * contiguous.
 */

#define M_REG_ERROR 0
#define M_REG_BADDIMS -1
#define M_REG_SHORTARR -2
#define M_REG_REGNUM -3

static IDL_MSG_DEF msg_arr[] =
{
  {  "M_REG_ERROR",	"%NError: %s." },
  {  "M_REG_BADDIMS",	"%NDimensions must match stream sample dimensions." },
  {  "M_REG_SHORTARR",	"%NInsufficient number of samples presented." },
  {  "M_REG_REGNUM",   "%NInvalid regression number."},
};

/*
 * The load function fills in this message block handle with the
 * opaque handle to the message block used for this module. The other
 * routines can then use it to throw errors from this block.
 */
IDL_MSG_BLOCK msg_block;

void cvio_reg_exit_handler(void)
{
	while(rlist) remove_reg(rlist->reg);
	return;
}

/* X=REG_CREATE(vec,nvalues[,mask]) */
IDL_VPTR IDL_CDECL idl_reg_create(int argc, IDL_VPTR argv[], char *argk)
{
	Regression	*r;
	long int	num;
	IDL_VPTR	vpVec,vpMask;
	unsigned char	*mask = NULL;
	long int	num_values;
	rlink		*p;

	IDL_ENSURE_ARRAY(argv[0]);
	vpVec = IDL_CvtLng(1,&(argv[0]));
	num = vpVec->value.arr->n_elts;
	num_values = IDL_LongScalar(argv[1]);

	if (argc >= 3) {
		vpMask = IDL_CvtByte(1,&(argv[2]));
		if (num_values != vpMask->value.arr->n_elts) {
		        IDL_MessageFromBlock(msg_block, M_REG_SHORTARR,
                		IDL_MSG_LONGJMP);

		}
		mask = (unsigned char *)vpMask->value.arr->data;
	}

	r = reg_create(num,(long int *)vpVec->value.arr->data,num_values,mask);

	if (vpVec != argv[0]) IDL_Deltmp(vpVec);
	if (mask) {
		if (vpMask != argv[2]) IDL_Deltmp(vpMask);
	}

	p = (rlink *)IDL_MemAlloc(sizeof(rlink),"",IDL_MSG_RET);
	if (!p) {
		reg_destroy(r);
		return(IDL_GettmpULong((IDL_ULONG)-1));
	}
	p->count = rbase++;
	p->reg = r;
	p->next = rlist;
	rlist = p;

	return(IDL_GettmpULong(p->count));
}

/* x=REG_DESTROY(A) */
IDL_VPTR IDL_CDECL idl_reg_destroy(int argc, IDL_VPTR argv[], char *argk)
{
	Regression	*r = find_reg(IDL_ULongScalar(argv[0]));

	remove_reg(r);

	return(IDL_GettmpLong(r ? 0 : -1));
}

IDL_VPTR IDL_CDECL idl_reg_X_obs(int argc,IDL_VPTR argv[],char *argk,int iAdd)
{
	int		ret;
	double		*y;
	double		**obs;
	double		*x;
	double		weight = 1.0;
	IDL_VPTR	vpTmp,vpX,vpY;
	IDL_LONG	i;
	Regression	*r;

	r = find_reg(IDL_ULongScalar(argv[0]));
	if (r == NULL) {
		IDL_MessageFromBlock(msg_block,M_REG_REGNUM,IDL_MSG_LONGJMP);
	}

	IDL_ENSURE_ARRAY(argv[1]);
	IDL_ENSURE_ARRAY(argv[2]);

	if (argv[1]->value.arr->n_elts != r->num_values) {
		IDL_MessageFromBlock(msg_block,M_REG_BADDIMS,IDL_MSG_LONGJMP);
	}
	if (argv[2]->value.arr->n_elts != r->num_values*r->num_vars) {
		IDL_MessageFromBlock(msg_block,M_REG_SHORTARR,IDL_MSG_LONGJMP);
	}

	/* user specified weight */
	if (argc == 4) weight = IDL_DoubleScalar(argv[3]);

	/* convert to doubles */
	vpY = IDL_CvtDbl(1,&(argv[1]));
	y = (double *)vpY->value.arr->data;

	vpX = IDL_CvtDbl(1,&(argv[2]));
	x = (double *)vpX->value.arr->data;

	/* allocate pointer array */
	obs = (double **)IDL_GetScratch(&vpTmp,r->num_values,sizeof(double *));
	/* copy values */
	for(i=0;i<r->num_values;i++) {
		obs[i] = x + (i*r->num_values);
	}

	/* do it */
	if (iAdd) {
		ret = reg_add_obs(r,y,weight,obs);
	} else {
		ret = reg_remove_obs(r,y,weight,obs);
	}
	
	/* clean up */
	if (vpY != argv[1]) IDL_Deltmp(vpY);
	if (vpX != argv[2]) IDL_Deltmp(vpX);

	IDL_Deltmp(vpTmp);

	return(IDL_GettmpLong(ret));
}

/* X=REG_REMOVE_OBS(R,Y,X) */
IDL_VPTR IDL_CDECL idl_reg_remove_obs(int argc, IDL_VPTR argv[], char *argk)
{
	return(idl_reg_X_obs(argc,argv,argk,0));
}

/* X=REG_ADD_OBS(R,Y,X) */
IDL_VPTR IDL_CDECL idl_reg_add_obs(int argc, IDL_VPTR argv[], char *argk)
{
	return(idl_reg_X_obs(argc,argv,argk,1));
}

/* X=REG_CALC_SS(R[,mask],partial,sst,sse,ssvar) */
IDL_VPTR IDL_CDECL idl_reg_calc_ss(int argc, IDL_VPTR argv[], char *argk)
{
	int		ret;
	unsigned char	*mask = NULL;
	double		*sst,*sse,*ssv;
	Regression	*r;
	int		bck = 1;
	IDL_VPTR	vpSST,vpSSE,vpSSV,vpMask;
	IDL_MEMINT	num;

	r = find_reg(IDL_ULongScalar(argv[0]));
	if (r == NULL) {
		IDL_MessageFromBlock(msg_block,M_REG_REGNUM,IDL_MSG_LONGJMP);
	}
	if (argc >= 6) {
		bck = 2;
		vpMask = IDL_CvtByte(1,&(argv[1]));
		if (r->num_values != vpMask->value.arr->n_elts) {
		        IDL_MessageFromBlock(msg_block, M_REG_SHORTARR,
                		IDL_MSG_LONGJMP);

		}
		mask = (unsigned char *)vpMask->value.arr->data;
	}

	IDL_EXCLUDE_EXPR(argv[bck+1]);
	IDL_EXCLUDE_EXPR(argv[bck+2]);
	IDL_EXCLUDE_EXPR(argv[bck+3]);

	num = r->num_values;
	sst = (double *)IDL_MakeTempVector(IDL_TYP_DOUBLE,num,
		IDL_ARR_INI_NOP,&vpSST);
	sse = (double *)IDL_MakeTempVector(IDL_TYP_DOUBLE,num,
		IDL_ARR_INI_NOP,&vpSSE);
	num = r->num_values*r->num_vars;
	ssv = (double *)IDL_MakeTempVector(IDL_TYP_DOUBLE,num,
		IDL_ARR_INI_NOP,&vpSSV);

	ret = reg_calc_ss(r,mask,IDL_LongScalar(argv[bck]),sst,sse,ssv);

	IDL_VarCopy(vpSST,argv[bck+1]);
	IDL_VarCopy(vpSSE,argv[bck+2]);
	IDL_VarCopy(vpSSV,argv[bck+3]);
	if (mask) {
		if (vpMask != argv[1]) IDL_Deltmp(vpMask);
	}

	return(IDL_GettmpLong(ret));
}

/* X=REG_CALC_COEF(R[,mask],COEF[,STD_ERROR=e]) */
IDL_VPTR IDL_CDECL idl_reg_calc_coef(int iargc, IDL_VPTR inargv[], char *argk)
{
	int		ret,argc;
	unsigned char	*mask = NULL;
	double		*coef;
	Regression	*r;
	int		bck = 1;
	IDL_VPTR	vpTmp,vpSTETmp,vpMask,argv[3];
	IDL_MEMINT	num;
	double		*sterr = NULL;

static  IDL_VPTR  	vpSTE;
static  IDL_KW_PAR      kw_pars[] = { IDL_KW_FAST_SCAN,
        {"STD_ERROR",IDL_TYP_UNDEF,1,IDL_KW_OUT|IDL_KW_ZERO,0,IDL_CHARA(vpSTE)},
    	{NULL}
};

	/* parse keywords */
	IDL_KWCleanup(IDL_KW_MARK);
	argc = IDL_KWGetParams(iargc,inargv,argk,kw_pars,argv,1);

	r = find_reg(IDL_ULongScalar(argv[0]));
	if (r == NULL) {
		IDL_MessageFromBlock(msg_block,M_REG_REGNUM,IDL_MSG_LONGJMP);
	}
	if (argc == 3) {
		bck = 2;
		vpMask = IDL_CvtByte(1,&(argv[1]));
		if (r->num_values != vpMask->value.arr->n_elts) {
		        IDL_MessageFromBlock(msg_block, M_REG_SHORTARR,
                		IDL_MSG_LONGJMP);

		}
		mask = (unsigned char *)vpMask->value.arr->data;
	}

	IDL_EXCLUDE_EXPR(argv[bck]);
	num = r->num_values*r->num_vars;
	coef = (double *)IDL_MakeTempVector(IDL_TYP_DOUBLE,num,
		IDL_ARR_INI_NOP,&vpTmp);

        if (vpSTE) {
		sterr = (double *)IDL_MakeTempVector(IDL_TYP_DOUBLE,
			num,IDL_BARR_INI_NOP,&vpSTETmp);
	}

	ret = reg_calc_coef(r,mask,coef,sterr);

	IDL_VarCopy(vpTmp,argv[bck]);

        if (vpSTE) IDL_VarCopy(vpSTETmp,vpSTE);

	if (mask) {
		if (vpMask != argv[1]) IDL_Deltmp(vpMask);
	}

	/* cleanup */
	IDL_KWCleanup(IDL_KW_CLEAN);

	return(IDL_GettmpLong(ret));
}

int IDL_Load(void)
{
  static IDL_SYSFUN_DEF function_addr[] = {
    { idl_reg_create,	"REG_CREATE",	2, 3, 0},
    { idl_reg_destroy,	"REG_DESTROY",	1, 1, 0},
    { idl_reg_add_obs,	"REG_ADD_OBS",	3, 4, 0},
    { idl_reg_remove_obs,"REG_REMOVE_OBS",	3, 4, 0},
    { idl_reg_calc_ss,	"REG_CALC_SS",	5, 6, 0},
    { idl_reg_calc_coef,"REG_CALC_COEF",	2, 3, IDL_SYSFUN_DEF_F_KEYWORDS},
  };

  if (!(msg_block = IDL_MessageDefineBlock("cvio_reg", ARRLEN(msg_arr),
	   msg_arr))) {
	return IDL_FALSE;
  }

  if (!IDL_AddSystemRoutine(function_addr, TRUE, ARRLEN(function_addr))) {
	return IDL_FALSE;
  }

  IDL_ExitRegister(cvio_reg_exit_handler);

  return IDL_TRUE;
}
