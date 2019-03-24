
/*
 * Copyright (c), 1990-2000 Randall Frank and Thomas Grabowski
 *
 * $Id: regression.c 1239 2005-09-08 16:42:56Z dforeman $
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
#include <string.h>
#include <math.h>
#include "as274.h"
#ifdef WIN32
#else
#include <unistd.h>
#endif
#include "regression.h"

static double  calc_pss(long int np,long int nrbar,long int *vorder,double *dd,
        double *r,double *theta,double *x,double *tol,long int *norder,
        long int k,long int j);

static int reg_addrem_obs(Regression *temp, double *y, double w, 
	double **obs, int add);

Regression *reg_create(long int nvars, long int *nclasses, long int nvalues, 
	char *valuemask)
{
	Regression *temp;
	long int fail, i,k;
	
	temp = (Regression *)malloc(sizeof(Regression));
	if (!temp) return(NULL);

	k = 0;
	temp->np=0;
	for(i=0; i<nvars; i++) {
		if (nclasses[i] == 0) {
			temp->np += 1;
		} else if (nclasses[i] == -1) {
			temp->np += 1;
			k++;
		} else {
			temp->np += nclasses[i]-1;
		}
	}
	if (k > 1) {
		free(temp);
		return(NULL);
	}
	
	temp->mask_value = (char *)malloc(nvalues*sizeof(char));
	temp->num_classes = (long int *)malloc(nvars*sizeof(long int));
	temp->XofV = (long int *)malloc(nvars*sizeof(long int));
	temp->xtmp = (double *)malloc(temp->np*sizeof(double));

	if ((temp->mask_value == 0) || (temp->num_classes == 0) ||
            (temp->XofV == 0) || (temp->xtmp == 0)) {
		if (temp->mask_value) free(temp->mask_value);
		if (temp->num_classes) free(temp->num_classes);
		if (temp->XofV) free(temp->XofV);
		if (temp->xtmp) free(temp->xtmp);
		free(temp);
		return(NULL);
	}

	temp->num_vars = nvars;
	temp->num_values = nvalues;

	/* clone the bitmask */
	if (valuemask) {
		memcpy(temp->mask_value,valuemask,nvalues);
	} else {
		memset(temp->mask_value,1,nvalues);
	}

	/* compute the variable start table... */
	k = 0;
	for(i=0;i<temp->num_vars;i++) {
		temp->num_classes[i] = nclasses[i];
		temp->XofV[i] = k;
		if (nclasses[i] == 0) {
			k++;
		} else if (nclasses[i] == -1) {
			k++;
		} else {
			k += nclasses[i];
		}
	}

	/* allocate memory */
	temp->nrbar = temp->np*(temp->np-1)/2;

	temp->ss = (double *)malloc(temp->np*sizeof(double));
	temp->tol = (double *)malloc(temp->np*sizeof(double));
	temp->work = (double *)malloc(temp->np*sizeof(double));
	temp->lindep = (long int *)malloc(temp->np*sizeof(long int));
	temp->vorder = (long int *)malloc(temp->np*sizeof(long int));
	temp->norder = (long int *)malloc(temp->np*sizeof(long int));

	temp->reg = (Individual *)calloc(temp->num_values,sizeof(Individual));
	for(i=0;i<temp->num_values;i++) {
		if (temp->mask_value[i]) {
		
			temp->reg[i].r = (double *)malloc(temp->nrbar*sizeof(double));
			temp->reg[i].dd = (double *)malloc(temp->np*sizeof(double));
			temp->reg[i].theta = (double *)malloc(temp->np*sizeof(double));

			/* setup a new regression */
			temp->reg[i].sser = 0;
			clear_(&temp->np,&temp->nrbar,temp->reg[i].dd,
				temp->reg[i].r,temp->reg[i].theta,
				&temp->reg[i].sser,&fail);

			temp->reg[i].sumy = 0;
			temp->reg[i].ssumy = 0;
		}
	}

	temp->nobs = 0;

	return(temp);
}

void reg_destroy(Regression *temp)
{
	long int	i;
	/* Free up the memory */
	for(i=0;i<temp->num_values;i++) {
		if (temp->mask_value[i]) {
			free(temp->reg[i].r);
			free(temp->reg[i].dd); 
			free(temp->reg[i].theta); 
		}
	}

	free(temp->reg);

	free(temp->ss); 
	free(temp->tol);
	free(temp->work);
	free(temp->lindep);
	free(temp->vorder); 
	free(temp->norder);

	free(temp->mask_value);
	free(temp->num_classes);
	free(temp->xtmp);
	free(temp->XofV);

	free(temp);

	return;
}

int reg_add_obs(Regression *temp, double *y, double weight, double **obs)
{
	return(reg_addrem_obs(temp, y, weight, obs, 1));
}

int reg_remove_obs(Regression *temp, double *y, double weight, double **obs)
{
	return(reg_addrem_obs(temp, y, weight, obs, 0));
}

static int reg_addrem_obs(Regression *temp, double *y, double weight, 
	double **obs, int add)
{
	long int i, j, l;
	long int num, fail;
	double w, y_here;

	for (i=0; i<temp->num_values; i++) {
	    if (temp->mask_value[i]) {
		y_here = y[i];
		if (add) {
			temp->reg[i].sumy += y_here;
			temp->reg[i].ssumy += (y_here*y_here);
		} else {
			temp->reg[i].sumy -= y_here;
			temp->reg[i].ssumy -= (y_here*y_here);
		}

		/* reform the input vector (contains classes) into a vector
		   of length np which contains only covariables. */
		for (j=0; j<temp->num_vars; j++) {
			if (temp->num_classes[j] == 0) {
				temp->xtmp[temp->XofV[j]] = obs[j][i];
			} else if (temp->num_classes[j] == -1) {
				temp->xtmp[temp->XofV[j]] = 1.0;
			} else {
				num = temp->num_classes[j]-1;
				for (l=0; l<num; l++) {
					temp->xtmp[temp->XofV[j]+l] = 0.0;
				} /* end for l */
				/* range must be 1.0 to num_classes[j] */
				if (obs[j][i] > 1.0) {
					l = obs[j][i];
					temp->xtmp[temp->XofV[j]+l-2] = 1.0;
				} /* end if */
			} /* end else */
		} /* end for j */

#ifdef DEBUG
printf("Adding: %f = %f %f i=%d nv=%d\n",y_here,temp->xtmp[0],temp->xtmp[1],
		i,temp->num_vars);
#endif
		/* add it to the building regression at that point */
		w=weight;
 		if (add) {
		    includ_(&temp->np,&temp->nrbar,&w,temp->xtmp,&y_here,
			temp->reg[i].dd,temp->reg[i].r,
			temp->reg[i].theta,&temp->reg[i].sser,&fail);
		} else {
		    remove_(&temp->np,&temp->nrbar,&w,temp->xtmp,&y_here,
			temp->reg[i].dd,temp->reg[i].r,
			temp->reg[i].theta,&temp->reg[i].sser,&fail);
		}
	    }
	} /* end for i */

	if (add) {
		temp->nobs++;
        } else {
		temp->nobs--;
	}

	return(0);
}

int reg_calc_ss(Regression *temp, char *mask, long int partial, 
	double *ss_total, double *ss_error, double *ss_var)
{
	long int i, j, k, c;
	double sst, d;
	long int fail;
	
	for (i=0; i<temp->num_values; i++) {
	    if ((!mask && temp->mask_value[i]) || 
		(mask && mask[i] && temp->mask_value[i])) {

		/* set singular tolerances */
        	tolset_(&temp->np,&temp->nrbar,temp->reg[i].dd,temp->reg[i].r,
			temp->tol,temp->work,&fail);
		
		/* check for singularities */
        	sing_(&temp->np,&temp->nrbar,temp->reg[i].dd,temp->reg[i].r,
			temp->reg[i].theta,&temp->reg[i].sser,temp->tol,
			temp->lindep,temp->work,&fail);

		/* extract needed regression info */
		/* ss=SS left after adding var i */
		ss_(&temp->np,temp->reg[i].dd,temp->reg[i].theta,
			&temp->reg[i].sser,temp->ss,&fail);

		/* and SS_TOTAL sum(d*d -2.0*d*ybar + ybar*ybar) */
		d = temp->reg[i].sumy/(double)(temp->nobs);
		sst = temp->reg[i].ssumy - 2.0*d*temp->reg[i].sumy + 
			d*d*(double)(temp->nobs);
		ss_total[i] = sst;
		ss_error[i] = temp->reg[i].sser;
				
		if (partial == 0) {
			for(c=0;c<temp->num_vars;c++) {
				j = temp->XofV[c];
				if (temp->num_classes[c] > 0) {
					j += (temp->num_classes[c] - 2);
				}
				k = temp->XofV[c] - 1;
				if (k < 0) {
					d = sst-temp->reg[i].sser-temp->ss[j];
				} else {
					d = temp->ss[k] - temp->ss[j];
				}
				if (temp->num_classes[c] == -1) {
					ss_var[i*temp->num_vars+c] = 0.0;
				} else {
					ss_var[i*temp->num_vars+c] = d;
				}
			}
		} else {
			for(c=0;c<temp->np;c++) temp->vorder[c] = c;
			for(c=0;c<temp->num_vars;c++) {
				j = temp->XofV[c];
				if (temp->num_classes[c] > 0) {
					j += (temp->num_classes[c] - 2);
				}
				k = temp->XofV[c] - 1;
				d = calc_pss(temp->np,temp->nrbar,temp->vorder,
					temp->reg[i].dd,temp->reg[i].r,
					temp->reg[i].theta,temp->ss,
					temp->tol,temp->norder,k,j);
				if (temp->num_classes[c] == -1) {
					ss_var[i*temp->num_vars+c] = 0.0;
				} else {
					ss_var[i*temp->num_vars+c] = d;
				}
			}
		}
	    } else {
		ss_total[i]=0;
		ss_error[i]=0;
		for (j=0; j<temp->num_vars; j++) {
			ss_var[i*temp->num_vars+j] = 0;
		} /* end for j */
	    }
	} 
               
	return(0);
}

int reg_calc_coef(Regression *temp, char *mask, double *coef,double *sterr)
{
	long int i, j, k;
	long int fail;
	double   hii,w;

	for (i=0; i<temp->num_values; i++) {
	    	if ((!mask && temp->mask_value[i]) || 
		    (mask && mask[i] && temp->mask_value[i])) {

			/* set singular tolerances */
        		tolset_(&temp->np,&temp->nrbar,temp->reg[i].dd,
				temp->reg[i].r,temp->tol,
				temp->work,&fail);
		
			/* check for singularities */
		        sing_(&temp->np,&temp->nrbar,temp->reg[i].dd,
				temp->reg[i].r,temp->reg[i].theta,
				&temp->reg[i].sser,temp->tol,
		                temp->lindep,temp->work,&fail);
			                       
			/* get coefficients, work=coefficients */
		        regcf_(&temp->np,&temp->nrbar,temp->reg[i].dd,
				temp->reg[i].r,temp->reg[i].theta,
				temp->tol,temp->work,&temp->np,&fail);
	
			for (j=0; j<temp->num_vars; j++) {
				coef[i*temp->num_vars+j] = temp->work[temp->XofV[j]];
				if (temp->num_classes[j] > 0) {
					coef[i*temp->num_vars+j] = 0.0;
				} 
			}
			if (sterr) {
				for (j=0; j<temp->num_vars; j++) {
					if (temp->num_classes[j] > 0) {
						coef[i*temp->num_vars+j] = 0.0;
					} else {
						for(k=0;k<temp->np;k++) {
							temp->xtmp[k]= 0.0;
						}
						temp->xtmp[temp->XofV[j]] = 1.0;
						hdiag_(temp->xtmp,&temp->np,
							&temp->nrbar,
							temp->reg[i].dd,
							temp->reg[i].r,
							temp->tol,
							&temp->np,&hii,
							temp->work, &fail);
						w = temp->reg[i].sser/(double)
							(temp->nobs-temp->np);
						sterr[i*temp->num_vars+j] = 
							sqrt(w)*sqrt(hii);
					}
				}
			} /* end for j */
		} /* end if in mask */
		else {
			for (j=0; j<temp->num_vars; j++) {
				coef[i*temp->num_vars+j] = 0.0;
			} /* end for j */
			if (sterr) {
				for (j=0; j<temp->num_vars; j++) {
					sterr[i*temp->num_vars+j] = 0.0;
				} /* end for j */
			}
		} /* end else */
	} /* end for i */

	return(0);
}

/* order so that the current coefficients are last... */
static double  calc_pss(long int np,long int nrbar,long int *vorder,double *dd,
        double *r,double *theta,double *x,double *tol,long int *norder,
        long int k,long int j)
{
        long int        pos1 = 1;
        long int        i,num,l;
        long int        ptr,l1;
        long int        fail,done,TO,FROM;
        double          pss;

        ptr = 0;
        for(i=0;i<=k;i++) norder[ptr++] = i;
        for(i=j+1;i<np;i++) norder[ptr++] = i;
        l1 = ptr;
        for(i=k+1;i<=j;i++) norder[ptr++] = i;

        num = np-1;

#ifdef DEBUG
	printf("BEFORE Pss from %ld to %ld\n",k,j);
	printf("norder=");
	for(i=0;i<np;i++) printf(" %ld",norder[i]);
	printf("\n");
	printf("vorder=");
	for(i=0;i<np;i++) printf(" %ld",vorder[i]);
	printf("\n");
	printf("RSS=");
	for(i=0;i<np;i++) printf(" %f",x[i]);
	printf("\n");
	printf("num=%ld,pos1=%ld\n",num,pos1);
#endif
/* reorder */
        done = 0;
        while (!done) {
                done = 1;
                for(i=np-1;i>=0;i--) {
                        if (norder[i] != vorder[i]) {
                                TO = i+1;
                                for(l=0;l<np;l++) {
                                        if (vorder[l] == norder[i]) {
                                                FROM = l+1;
                                        }
                                }
                                vmove_(&np,&nrbar,vorder,dd,r,theta,x,
                                        &FROM,&TO,tol,&fail);
                                done = 0;
                        }
                }
        }
        pss = x[l1-1] - x[np-1];

#ifdef DEBUG
	printf("AFTER Pss from %ld to %ld\n",k,j);
	printf("norder=");
	for(i=0;i<np;i++) printf(" %ld",norder[i]);
	printf("\n");
	printf("vorder=");
	for(i=0;i<np;i++) printf(" %ld",vorder[i]);
	printf("\n");
	printf("RSS=");
	for(i=0;i<np;i++) printf(" %f",x[i]);
	printf("\n");
	printf("num=%ld,pos1=%ld,l1=%ld,pss=%f\n",num,pos1,l1,pss);
	printf("fail = %ld\n",fail);
#endif

/* return the data back to "default" order */
        for(i=0;i<np;i++) {
                norder[i] = i;
        }

/* reorder */
        done = 0;
        while (!done) {
                done = 1;
                for(i=np-1;i>=0;i--) {
                        if (norder[i] != vorder[i]) {
                                TO = i+1;
                                for(l=0;l<np;l++) {
                                        if (vorder[l] == norder[i]) {
                                                FROM = l+1;
                                        }
                                }
                                vmove_(&np,&nrbar,vorder,dd,r,theta,x,
                                        &FROM,&TO,tol,&fail);
                                done = 0;
                        }
                }
        }

#ifdef DEBUG
	printf("AFTER reorder #2 %ld %ld\n",k,j);
	printf("norder=");
	for(i=0;i<np;i++) printf(" %ld",norder[i]);
	printf("\n");
	printf("vorder=");
	for(i=0;i<np;i++) printf(" %ld",vorder[i]);
	printf("\n");
	printf("RSS=");
	for(i=0;i<np;i++) printf(" %f",x[i]);
	printf("\n");
#endif
        return(pss);
}
