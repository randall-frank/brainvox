/*
 * Copyright (c), 1990-2000 Randall Frank and Thomas Grabowski
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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "export.h"
#include "../AIR.h"
#include "air_interface.h"

#define M_CVIO_BADDIMS -1
extern IDL_MSG_BLOCK msg_block;

/*
 * TODO:
 */

/* err=AIR_ALIGN(standard,reslice,endparams[,COST_FUNCTION=c][CONVERGENCE=c]
*         [FIT_MODEL=f][INTERPOLATION=i][PARTITIONS=pp][THRESHOLD=t]
*                 [STEPS=sss])
*/
IDL_VPTR IDL_CDECL idl_air_align(int iargc, IDL_VPTR inargv[], char *argk)
{
	int		ret = 1,argc;
	IDL_VPTR	vpStandard,vpReslice,vpTmp,argv[4];
	double		*fit;
	char		*fixed,*in;
	int		dims[3],i;
	AlignParams	p;
	IDL_LONG	iFitlen;

static  IDL_LONG  	iCost;
static  IDL_LONG  	iModel;
static  IDL_LONG  	iInterp;
static  IDL_LONG	iPart[2];
static  IDL_LONG	iSfac[3];
static  IDL_LONG	iThreshold[2];
static  float  		fConv;
static  IDL_KW_ARR_DESC dPartitions = {IDL_CHARA(iPart), 1, 2, 0};
static  IDL_KW_ARR_DESC dSteps = {IDL_CHARA(iSfac), 1, 3, 0};
static  IDL_KW_ARR_DESC dThres = {IDL_CHARA(iThreshold), 1, 2, 0};
static  IDL_KW_PAR      kw_pars[] = { IDL_KW_FAST_SCAN,
        {"CONVERGENCE",IDL_TYP_FLOAT,1,0,0,IDL_CHARA(fConv)},
        {"COST_FUNCTION",IDL_TYP_LONG,1,0,0,IDL_CHARA(iCost)},
        {"FIT_MODEL",IDL_TYP_LONG,1,0,0,IDL_CHARA(iModel)},
        {"INTERPOLATION",IDL_TYP_LONG,1,0,0,IDL_CHARA(iInterp)},
        {"PARTITIONS",IDL_TYP_LONG,1,IDL_KW_ARRAY,0,IDL_CHARA(dPartitions)},
        {"STEPS",IDL_TYP_LONG,1,IDL_KW_ARRAY,0,IDL_CHARA(dSteps)},
        {"THRESHOLD",IDL_TYP_LONG,1,IDL_KW_ARRAY,0,IDL_CHARA(dThres)},
    	{NULL}
};

	/* default value */
	iCost = 2;
	iModel = 23;
	iInterp = 1;
	fConv = 0.00001;
	iSfac[0] = 81;
	iSfac[1] = 3;
	iSfac[2] = 1;
	iPart[0] = 1;
	iPart[1] = 1;
	iThreshold[0] = 7000;
	iThreshold[1] = 7000;

	/* parse keywords */
	IDL_KWCleanup(IDL_KW_MARK);
	argc = IDL_KWGetParams(iargc,inargv,argk,kw_pars,argv,1);

	/* get the input data */
	IDL_ENSURE_ARRAY(argv[1]);
	IDL_ENSURE_ARRAY(argv[2]);
	vpStandard = IDL_CvtUInt(1,&(argv[1]));
	IDL_EXCLUDE_EXPR(argv[2]);
	vpReslice = IDL_CvtUInt(1,&(argv[2]));
	IDL_EXCLUDE_EXPR(argv[3]);

#ifdef NEVER
	iFitlen = iModel+1;
	if (iModel > 20) iFitlen = iModel - 20 + 1;
#endif	
	iFitlen = 17;
	fit = (double *)IDL_MakeTempVector(IDL_TYP_DOUBLE,iFitlen,
		IDL_ARR_INI_ZERO, &vpTmp);

	/* set up parameters */
	align_param_init(&p);
	p.samplefactor = iSfac[0];
	p.sffactor = iSfac[1];
	p.samplefactor2 = iSfac[2];
	p.partitions1 = iPart[0];
	p.partitions2 = iPart[1];
	p.interp = iInterp;
	p.model = iModel;
	p.precision = fConv;
	p.costfxn = iCost;
	p.threshold1 = iThreshold[0];
	p.threshold1 = iThreshold[1];

	/* we fit and write over the "reslice" array */
	if (vpStandard->value.arr->n_dim != vpReslice->value.arr->n_dim) {
		IDL_MessageFromBlock(msg_block, M_CVIO_BADDIMS,IDL_MSG_LONGJMP);
	}
	dims[0] = 1;
	dims[1] = 1;
	dims[2] = 1;
	for(i=0;i<vpStandard->value.arr->n_dim;i++) {
		dims[i] = vpStandard->value.arr->dim[i];
		if (dims[i] != vpReslice->value.arr->dim[i]) {
			IDL_MessageFromBlock(msg_block, M_CVIO_BADDIMS,
					IDL_MSG_LONGJMP);
		}
	}

	fixed = (char*)vpStandard->value.arr->data;
	in = (char*)vpReslice->value.arr->data;
	ret = align_images(fixed,in,in,dims,&p,fit,fit+1);

	/* free any temps */
	if (vpStandard != argv[1]) IDL_Deltmp(vpStandard);
	if (vpReslice != argv[2]) IDL_VarCopy(vpReslice,argv[2]);
	IDL_VarCopy(vpTmp,argv[3]);

	/* cleanup */
	IDL_KWCleanup(IDL_KW_CLEAN);

	return(IDL_GettmpLong(ret));
}
