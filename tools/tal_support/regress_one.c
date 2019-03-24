/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski
 *
 * $Id: regress_one.c 1250 2005-09-16 15:51:42Z dforeman $
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
#include "tal_regress.h"
#include "as274.h"
#include "proto.h"

/* local defines */
#define INDX (xp+(yp*volume.dx))

/* data global to this file */
typedef struct {
	long int	np,nrbar,t_nobs;
	double		sser,*yvec;
	double		*dd,*r,*theta;
	double		*x,*tol,*work,*work2,*ss;
	double		sumy,ssumy;
	long int	*vorder,*norder;
	long int	*lindep;
	long int	XofV[MAX_VARS];  /* for a given var index, what is
					the first index into the X vector */
} working_regression;

working_regression	Q;

void regress_one_data(long int xp,long int yp, float weight)
{
	long int	i,fail;
	double		w,y;
	
/* for each observation, add the data vector to the regression */
	for(i=0;i<nobs;i++) {
/* collect the observation */
		get_data_vector(&y,Q.x,i,Q.XofV,xp,yp);
		Q.yvec[i] = y;

/* add it to the datset */
		Q.t_nobs += 1;
		Q.sumy += y;
		Q.ssumy += (y*y);

		/* w = 1;   all weighted the same */
		w = obs[i].weight*weight;  /* weights are multiplied */
		includ_(&Q.np,&Q.nrbar,&w,Q.x,&y,Q.dd,Q.r,
			Q.theta,&Q.sser,&fail);
	}

/* if predicting a boolean, estimate weights and recompute */
/* note: this is ONLY legal if the volume is ONE VOXEL in size */
	if (volume.y_boolean) {

/* set singular tolerances */
		tolset_(&Q.np,&Q.nrbar,Q.dd,Q.r,Q.tol,Q.work,&fail);
/* check for singularities */
		sing_(&Q.np,&Q.nrbar,Q.dd,Q.r,Q.theta,&Q.sser,Q.tol,
			Q.lindep,Q.work,&fail);
/* get coefficients */
        	regcf_(&Q.np,&Q.nrbar,Q.dd,Q.r,Q.theta,Q.tol,
			Q.work,&Q.np,&fail); /*work=coefficients*/

/* reset things */
		clear_(&Q.np,&Q.nrbar,Q.dd,Q.r,Q.theta,&Q.sser,&fail);
/* compute again */
		for(i=0;i<nobs;i++) {
/* collect the observation */
			get_data_vector(&y,Q.x,i,Q.XofV,xp,yp);
			Q.yvec[i] = y;

/* compute weighting */
                	w = calc_resid_y(i,Q.work,Q.np,xp,yp); /* Y-Yhat */
			w = y - w; /* Yhat */
			w = (w*(1.0-w));  /* weighting factor (Yhat)(1-Yhat) */

			includ_(&Q.np,&Q.nrbar,&w,Q.x,&y,Q.dd,Q.r,
				Q.theta,&Q.sser,&fail);
		}
	}
	return;
}

/* if only one slice was used, the data for that slice are still in memory */
void regress_one_output()
{
	long int	i,j,fail,k,num;
	double		w,y;
	double		sst;
	double		d;
	double		press,hii;

/* set singular tolerances */
	tolset_(&Q.np,&Q.nrbar,Q.dd,Q.r,Q.tol,Q.work,&fail);
/* check for singularities */
	sing_(&Q.np,&Q.nrbar,Q.dd,Q.r,Q.theta,&Q.sser,Q.tol,
		Q.lindep,Q.work,&fail);

/* extract needed regression info */
	ss_(&Q.np,Q.dd,Q.theta,&Q.sser,Q.ss,&fail);/*ss=SS left after var i */
/* and SS_TOTAL sum(y*y -2.0*y*ybar + ybar*ybar) */
	d = Q.sumy/(double)(Q.t_nobs);
	sst = Q.ssumy -2.0*d*Q.sumy + d*d*(double)(Q.t_nobs);
	
/* get coefficients */
        regcf_(&Q.np,&Q.nrbar,Q.dd,Q.r,Q.theta,Q.tol,
		Q.work,&Q.np,&fail); /*work=coefficients*/

/* initialize the order structures */
	for(i=0;i<Q.np;i++) {
		Q.vorder[i] = i;
	}
 
	if (verbose > 3)  {
		fprintf(stdout,"Coefficients:\n");
		for(i=0;i<Q.np;i++) fprintf(stdout,"%.3f ",Q.work[i]);
		fprintf(stdout,"\nSS: Total:%.3f\n",sst);
		for(i=0;i<Q.np;i++) fprintf(stdout,"%.3f ",Q.ss[i]);
		fprintf(stdout,"\n");
	}

/* compute the PRESS (only if needed!) */
        press = 0.0;
if (volume.need_press) {
        for(i=0;i<nobs;i++) {
                get_data_vector(&y,Q.x,i,Q.XofV,0,0);
/* get the hat diagonal for obs i */
                hdiag_(Q.x,&Q.np,&Q.nrbar,Q.dd,Q.r,Q.tol,&Q.np,&hii,
			Q.work2,&fail);
/* calc the residual */
                w = calc_resid_y(i,Q.work,Q.np,0,0)/(1.0-hii);
/* build the PRESS */
                press += (w*w);
        }
}

/* fill in the output volumes (possibly extracting extra regression info?) */
	for(i=0;i<noutputs;i++) {
		switch(outputs[i].type) {
			case OUT_PRESS:
				d = press;
				fprintf(stdout,"%f\tSS PRESS\n",d);
				break;
			case OUT_R2_PRED:
				d = 1.0 - (press/sst);
				fprintf(stdout,"%f\tPred Rsqu\n",d);
				break;
			case OUT_R2:
				d = 1.0 - (Q.sser/sst);
				fprintf(stdout,"%f\tRsqu\n",d);
				break;
			case OUT_R2_ADJ:
				d = 1.0 - ((Q.sser/(Q.t_nobs - Q.np))/
					(sst/(Q.t_nobs - 1.0)));
				fprintf(stdout,"%f\tAdjusted Rsqu\n",d);
				break;
			case OUT_SS_MODEL:
				d = sst-Q.sser;
				fprintf(stdout,"%f\tSS Model\n",d);
				break;
			case OUT_SS_TOTAL:
				d = sst;
				fprintf(stdout,"%f\tSS Total\n",d);
				break;
			case OUT_VAR_SS:
				j = Q.XofV[outputs[i].var];
				if (modeltemp.vars[outputs[i].var].type ==
					VAR_CLASS)
				{
					num = modeltemp.vars[
						outputs[i].var].value-1;
					j += (num-1);
				}
				k = Q.XofV[outputs[i].var]-1;
				if (k < 0) {
					d = sst - Q.sser - Q.ss[j];
				} else {
					d = Q.ss[k] - Q.ss[j];
				}
				fprintf(stdout,"%f\tSequential SS(%s),%ld\n",
					d,outputs[i].classname,
					Q.XofV[outputs[i].var]);
				break;
			case OUT_VAR_PSS:
				j = Q.XofV[outputs[i].var];
				if (modeltemp.vars[outputs[i].var].type ==
					VAR_CLASS)
				{
					num = modeltemp.vars[
						outputs[i].var].value-1;
					j += (num-1);
				}
				k = Q.XofV[outputs[i].var]-1;
				d = calc_pss(Q.np,Q.nrbar,Q.vorder,Q.dd,Q.r,
					Q.theta,Q.ss,Q.tol,Q.norder,k,j);
				fprintf(stdout,"%f\tPartial SS(%s),%ld\n",
					d,outputs[i].classname,
					Q.XofV[outputs[i].var]);
				break;
			case OUT_COV_COEF:
				d = Q.work[Q.XofV[outputs[i].var]];
				fprintf(stdout,"%f\tCoeff(%s)\n",
					d,outputs[i].classname);
				break;
			case OUT_VAR_COV:
				d = calc_var_cov(Q.np,Q.nrbar,Q.dd,Q.r,
					outputs[i].var,outputs[i].var2);
				fprintf(stdout,"%f\tCovar(%s,%s)\n",
					d,outputs[i].classname,
					outputs[i].classvalue);
				break;
			case OUT_COV_STDERR:
				for(j=0;j<Q.np;j++) Q.x[j] = 0.0;
				Q.x[Q.XofV[outputs[i].var]] = 1.0;
				hdiag_(Q.x,&Q.np,&Q.nrbar,Q.dd,Q.r,Q.tol,&Q.np,
					&hii,Q.work2,&fail);
				d = Q.sser/(double)(Q.t_nobs-Q.np);
				d = sqrt(d)*sqrt(hii);
				fprintf(stdout,"%f\tStdError(%s)\n",
					d,outputs[i].classname);
				break;
			case OUT_RES_ERR:
				d = Q.sser;
				fprintf(stdout,"%f\tSS Error\n",d);
				break;
		}
	}
	return;
}


void regress_one_cleanup()
{
/* free up the memory */
	free(Q.ss);
	free(Q.r);
	free(Q.dd);
	free(Q.theta);
	free(Q.tol);
	free(Q.work);
	free(Q.work2);
	free(Q.lindep);
	free(Q.vorder);
	free(Q.norder);
	free(Q.x);
	free(Q.yvec);

/* done */
	return;
}

void regress_one_setup()
{	
	long int	i,fail;

/* count the number of variables, Y=1,CONST=1,COV=1,CLASS=value-1 */
	Q.np = 0;
	for(i=0;i<modeltemp.nvars;i++) {
		switch(modeltemp.vars[i].type) {
			case VAR_Y_VAL:
			case VAR_Y_CONST:
				break;
			case VAR_CONST:
			case VAR_COV_V:
			case VAR_COV_C:
				Q.np = Q.np + 1;
				break;
			case VAR_CLASS:
				Q.np = Q.np + (modeltemp.vars[i].value-1);
				break;
			case VAR_LABEL:
				break;
		}
	}

/* allocate memory */
	Q.nrbar = Q.np*(Q.np-1)/2;
	Q.r = (double *)malloc(Q.nrbar*sizeof(double));
	Q.dd = (double *)malloc(Q.np*sizeof(double));
	Q.ss = (double *)malloc(Q.np*sizeof(double));
	Q.theta = (double *)malloc(Q.np*sizeof(double));
	Q.tol = (double *)malloc(Q.np*sizeof(double));
	Q.work = (double *)malloc(Q.np*sizeof(double));
	Q.work2 = (double *)malloc(Q.np*sizeof(double));
	Q.lindep = (long int *)malloc(Q.np*sizeof(long int));
	Q.vorder = (long int *)malloc(Q.np*sizeof(long int));
	Q.norder = (long int *)malloc(Q.np*sizeof(long int));
/* create the data vectors */
	Q.x = (double *)malloc(Q.np*sizeof(double));
	Q.yvec = (double *)malloc(nobs*sizeof(double));

/* check memory allocations */
	if ((Q.r == 0) || (Q.dd == 0) || (Q.theta == 0) || (Q.tol == 0) ||
	    (Q.work == 0) || (Q.work2 == 0) || (Q.lindep == 0) || 
	    (Q.x == 0) || (Q.yvec == 0) || (Q.ss == 0)) {
		ex_err("Unable to allocate regression memory space.");
	}

/* setup a new regression */
	clear_(&Q.np,&Q.nrbar,Q.dd,Q.r,Q.theta,&Q.sser,&fail);
	Q.t_nobs = 0;
	Q.sumy = 0;
	Q.ssumy = 0;
	
	return;
}

