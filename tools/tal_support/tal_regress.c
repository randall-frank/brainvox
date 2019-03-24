/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski
 *
 * $Id: tal_regress.c 1213 2005-08-27 20:51:21Z rjfrank $
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
#define GLOBAL_DEFS 1
#include "tal_regress.h"
#include "as274.h"
#include "proto.h"

double log_xform(double x);
void regress_data(long int x,long int y,float weight);
double get_v_value(unsigned char *data,long int dz,long int loc);

void ex_err(char *s)
{
	fprintf(stderr,"Fatal error:%s\n",s);
	tal_exit(1);
}

void cmd_err(char *s)
{
	fprintf(stderr,"(%s:%s) Usage: %s [options] datafile\n",__DATE__,TAL_VERSION,s);
	fprintf(stderr,"Options: -v[v[v[v]]]	verbose mode\n");
	fprintf(stderr,"\nDatafile definition language:\n");
	fprintf(stderr,"\tVOLPARAMS\n");
	fprintf(stderr,"\t\tDX value (image x dimensions default=256)\n");
	fprintf(stderr,"\t\tDY value (image y dimensions default=256)\n");
	fprintf(stderr,"\t\tFIRST value (first slice number default=1)\n");
	fprintf(stderr,"\t\tLAST value (last slice number default=119)\n");
	fprintf(stderr,"\t\tSTEP value (slice number increment default=1)\n");
	fprintf(stderr,"\t\tOFFSET value (data offset value default=0)\n");
	fprintf(stderr,"\t\tMASK template (8bit mask volume template default=full volume)\n");
	fprintf(stderr,"\t\tWEIGHT template (float weight volume template default= =1.0)\n");
	fprintf(stderr,"\t\tSWAP (enables byte swapping default=no byte swapping)\n");
	fprintf(stderr,"\t\tADJ_ALL_VARS (adj Y by covariates and class variables)\n");
	fprintf(stderr,"\t\tADJ_COV_ONLY (adj Y by covariates only, this is the default)\n");
	fprintf(stderr,"\t\tLOG_TRANSFORM (Apply a log10 transform to all input data)\n");
	fprintf(stderr,"\t\tSINGLE_FIT (Perform a single regression default=voxelwise)\n"); 
	fprintf(stderr,"\t\tY_BOOLEAN (Y value is boolean 0 or 1, use weighted regression)\n"); 
	fprintf(stderr,"\t\tINPUT_PREFIX dirname (prepended to input files default=none)\n"); 
	fprintf(stderr,"\t\tOUTPUT_PREFIX dirname (prepended to output files default=none)\n"); 
	fprintf(stderr,"\tMODEL\n");
	fprintf(stderr,"\t\tY name dz\n");
	fprintf(stderr,"\t\tY_CONST name\n");
	fprintf(stderr,"\t\tCONST\n");
	fprintf(stderr,"\t\tCOVARIABLE_VOL name mean dz\n");
	fprintf(stderr,"\t\tCOVARIABLE_CONST name mean\n");
	fprintf(stderr,"\t\tCLASS classname\n");
	fprintf(stderr,"\t\tLABELCLASS classname\n");
	fprintf(stderr,"\tOUTPUT\n");
	fprintf(stderr,"\t\tSS_ERROR template (SS error remaining)\n");
	fprintf(stderr,"\t\tSS_VARIABLE varname template\n");
	fprintf(stderr,"\t\tP_SS_VARIABLE varname template\n");
	fprintf(stderr,"\t\tSS_TOTAL template\n");
	fprintf(stderr,"\t\tSS_MODEL template\n");
	fprintf(stderr,"\t\tSS_PRESS template\n");
	fprintf(stderr,"\t\tR2_MODEL template\n");
	fprintf(stderr,"\t\tR2_ADJ_MODEL template\n");
	fprintf(stderr,"\t\tR2_PRED_MODEL template\n");
	fprintf(stderr,"\t\tADJ_Y_MEAN template\n");
	fprintf(stderr,"\t\tADJ_Y_CLASS_MEAN classname classlevel template\n");
	fprintf(stderr,"\t\tCOVARIABLE_SLOPE covname template\n");
	fprintf(stderr,"\t\tCOVARIABLE_STD_ERROR covname template\n");
	fprintf(stderr,"\t\tCOEF_COVARIANCE varname1 varname2 template\n");
	fprintf(stderr,"\tOBSERVATIONS\n");
	fprintf(stderr,"\t\tFor Y: template [weight]\n");
	fprintf(stderr,"\t\tFor Y_CONST: value [weight]\n");
	fprintf(stderr,"\t\tFor CONST: value (generally it should be 1.0)\n");
	fprintf(stderr,"\t\tFor COVARIABLE_VOL: template\n");
	fprintf(stderr,"\t\tFor COVARIABLE_CONST: value\n");
	fprintf(stderr,"\t\tFor CLASS: level\n");
	fprintf(stderr,"\t\tFor LABELCLASS: level\n");
	fprintf(stderr,"\tEND\n");
	tal_exit(1);
}

int main(int argc,char **argv)
{
	FILE 		*fp;
	long int	i,err,x,y,j,k;
	char		tstr[256];


	i = 1;
	while ((i < argc) && (argv[i][0] == '-')) {
		if (argv[i][1] == '\0') break;
		switch (argv[i][1]) {
			case 'v':
				verbose = strlen(argv[i])-1;
				break;
			default:	
				cmd_err(argv[0]);
				break;
		}
		i++;
	}
	if ((argc-i) != 1) cmd_err(argv[0]);
	fp = fopen(argv[i],"r");
	if (fp == 0L) ex_err("Unable to open data definition file");
	if (parse_input(fp) != 0) ex_err("Error in data definition file format");
	fclose(fp);

	if (verbose) {
		fprintf(stdout,"\nVolumeparams:\n");
		fprintf(stdout,"Matrix:%ld %ld\n",volume.dx,volume.dy);
		fprintf(stdout,"First:%ld Last:%ld Step:%ld\n",volume.first,
			volume.last,volume.step);
		fprintf(stdout,"Offset:%f\n",volume.offset);
		if (volume.swap) fprintf(stdout,"Byte-swaped\n");
		if (volume.single_fit) {
			fprintf(stdout,"Single regression\n");
		} else {
			fprintf(stdout,"Voxel-wise regression\n");
		}
		if (volume.masktemp) {
			fprintf(stdout,"Masktemp:%s\n",volume.masktemp);
		} else {
			fprintf(stdout,"No mask\n");
		}
		if (volume.cov_only) {
			fprintf(stdout,"Y adjusted by covariates only\n");
		} else {
			fprintf(stdout,"Y adjusted by all variables\n");
		}
		if (volume.weighttemp) {
			fprintf(stdout,"Weighttemp:%s\n",volume.weighttemp);
		}
		if (volume.y_boolean) {
			fprintf(stdout,"Y assumed to be a boolean variable\n");
		}
		fprintf(stdout,"\nVariables: %ld\n",modeltemp.nvars-1);
		for(i=0;i<modeltemp.nvars;i++) {
			printvar(stdout,&(modeltemp.vars[i]),0);
		}
		j = 0;
		for(i=0;i<modeltemp.nvars;i++) {
			switch(modeltemp.vars[i].type) {
				case VAR_Y_VAL:
				case VAR_Y_CONST:
					break;
				case VAR_CONST:
				case VAR_COV_V:
				case VAR_COV_C:
					j = j + 1;
					break;
				case VAR_CLASS:
					j = j + (modeltemp.vars[i].value-1);
					break;
				case VAR_LABEL:
					break;
			}
		}
		fprintf(stdout,"Error df=%ld\n",nobs-1-j);
		fprintf(stdout,"Total df=%ld\n",nobs-1);

		fprintf(stdout,"\nOutputs: %ld\n",noutputs);
		for(i=0;i<noutputs;i++) {
			printoutput(stdout,&(outputs[i]));
		}
		fprintf(stdout,"\nObservations: %ld\n",nobs);
		if (verbose > 1) {
			for(i=0;i<nobs;i++) {
				printf("WEIGHT %f\n",obs[i].weight);
				for(j=0;j<modeltemp.nvars;j++) {
					printvar(stdout,&(obs[i].vars[j]),1);
				}
				fprintf(stdout,"\n");
			}
		}
	}

/* if single regression, initialize the accumulators */
	if (volume.single_fit) regress_one_setup();

/* work through the volume */
	for(i=volume.first;i<=volume.last;i=i+volume.step) {
/* read mask image */
		if (volume.masktemp != 0L) {
			name_changer(volume.masktemp,i,&err,tstr);
			fprintf(stderr,"Reading mask file:%s\n",tstr);
			bin_io(tstr,'r',volume.maskdata,volume.dx,volume.dy,1,
				0,0,0L);
		} else {
			for(x=0;x<(volume.dx*volume.dy);x++) {
				volume.maskdata[x] = 1;
			}
		}

/* read weight image */
		if (volume.weighttemp != 0L) {
			name_changer(volume.weighttemp,i,&err,tstr);
			fprintf(stderr,"Reading weight file:%s\n",tstr);
			bin_io(tstr,'r',volume.weightdata,volume.dx,volume.dy,
				sizeof(float),0,volume.swap,0L);
		} else {
			for(x=0;x<(volume.dx*volume.dy);x++) {
				volume.weightdata[x] = 1.0;
			}
		}
		
/* read variable images */
		for(j=0;j<nobs;j++) {
			for(k=0;k<obs[j].nvars;k++) {
/* if a volume is defined, read from slice template in name field */
				if (obs[j].vars[k].data != 0) {
					name_changer(obs[j].vars[k].name,i,
						&err,tstr);
			fprintf(stderr,"Reading obs:%3ld var:%s file:%s\n",
				j,obs[j].vars[k].label,tstr);
					bin_io(tstr,'r',obs[j].vars[k].data,
						volume.dx,volume.dy,
						obs[j].vars[k].dz,
						0,volume.swap,0L);
				}
			}
		}

/* only in the multi-fit case */
		if (volume.single_fit == 0) {
/* clear the output images */
		for(j=0;j<noutputs;j++) {
			if (outputs[j].data != 0) {
				for(k=0;k<(volume.dx*volume.dy);k++) {
					outputs[j].data[k] = 0.0;
				}
			}
		}
		}

/* loop through all the pixels */
		for(y=0;y<volume.dy;y++) {
		for(x=0;x<volume.dx;x++) {
/* do the computation if not masked out */
			if (volume.maskdata[x+(volume.dx*y)] != 0) {
				if (volume.single_fit) {
					regress_one_data(x,y,
					    volume.weightdata[x+(volume.dx*y)]);
				} else {
					regress_data(x,y,
					    volume.weightdata[x+(volume.dx*y)]);
				}
			}
		}
		}

/* only in the multi-fit case */
		if (volume.single_fit == 0) {
/* write out the output images */
		for(j=0;j<noutputs;j++) {
			if (outputs[j].data != 0) {
				name_changer(outputs[j].tempname,i,&err,tstr);
				fprintf(stderr,"Writing output file:%s\n",tstr);
				bin_io(tstr,'w',outputs[j].data,volume.dx,
					volume.dy,sizeof(float),0,0,0L);
			}
		}
		}
/* next slice */
	}

/* output results and clean up after single regression case */
	if (volume.single_fit) {
		regress_one_output();  /* Note: for a one slice volume, the
					  observations are still in memory */
		regress_one_cleanup();
	}

/* done, free up the memory */
	freebuffers();

/* Whew! */
	tal_exit(0);

	exit(0);
}

/* image plane data access function */
double get_v_value(unsigned char *data,long int dz,long int loc)
{
	double		y;

	switch(dz) {
		case 1:
			y = ((unsigned char *)data)[loc];
			break;
		case 2:
			y = ((unsigned short *)data)[loc];
			y += volume.offset;
			break;
		case 4:
			y = ((float *)data)[loc];
			break;
		default:
			y = 0;
			break;
	}

	return(y);
}

#define INDX (xp+(yp*volume.dx))

void regress_data(long int xp,long int yp,float weight)
{	
	long int	i,j,fail,k,num;
	long int	np,nrbar;
	double 		ssumy,sumy;
	double		sser,w,y;
	double		sst,*yvec;
	double		press,hii;
	double		*dd,*r,*theta;
	double		*x,*tol,*work,*work2,*ss;
	long int	*vorder,*norder;
	long int	*lindep;
	long int	XofV[MAX_VARS];  /* for a given var index, what is
					the first index into the X vector */

/* count the number of variables, Y=1,CONST=1,COV=1,CLASS=value-1 */
	np = 0;
	for(i=0;i<modeltemp.nvars;i++) {
		switch(modeltemp.vars[i].type) {
			case VAR_Y_VAL:
			case VAR_Y_CONST:
				break;
			case VAR_CONST:
			case VAR_COV_V:
			case VAR_COV_C:
				np = np + 1;
				break;
			case VAR_CLASS:
				np = np + (modeltemp.vars[i].value-1);
				break;
			case VAR_LABEL:
				break;
		}
	}

/* allocate memory */
	nrbar = np*(np-1)/2;
	r = (double *)malloc(nrbar*sizeof(double));
	dd = (double *)malloc(np*sizeof(double));
	ss = (double *)malloc(np*sizeof(double));
	theta = (double *)malloc(np*sizeof(double));
	tol = (double *)malloc(np*sizeof(double));
	work = (double *)malloc(np*sizeof(double));
	work2 = (double *)malloc(np*sizeof(double));
	lindep = (long int *)malloc(np*sizeof(long int));
	vorder = (long int *)malloc(np*sizeof(long int));
	norder = (long int *)malloc(np*sizeof(long int));
/* create the data vectors */
	x = (double *)malloc(np*sizeof(double));
	yvec = (double *)malloc(nobs*sizeof(double));

/* check memory allocations */
	if ((r == 0) || (dd == 0) || (theta == 0) || (tol == 0) || (work2 == 0)
	     ||	(work == 0) || (lindep == 0) || (x == 0) || (yvec == 0) ||
	     (ss == 0)) {
		ex_err("Unable to allocate regression memory space.");
	}

/* setup a new regression */
	clear_(&np,&nrbar,dd,r,theta,&sser,&fail);

	sumy = 0;
	ssumy = 0;

/* for each observation, add the data vector to the regression */
	for(i=0;i<nobs;i++) {
/* collect the observation */
		get_data_vector(&y,x,i,XofV,xp,yp);
		yvec[i] = y;
/* add it to the datset */
		sumy += y;
		ssumy += (y*y);
		/* w = 1;  all weighted the same */
	        w = obs[i].weight*weight;
		includ_(&np,&nrbar,&w,x,&y,dd,r,theta,&sser,&fail);
	}
	
/* if predicting a boolean, estimate weights and recompute */
	if (volume.y_boolean) {

/* set singular tolerances */
                tolset_(&np,&nrbar,dd,r,tol,work,&fail);
/* check for singularities */
                sing_(&np,&nrbar,dd,r,theta,&sser,tol,
                        lindep,work,&fail);
/* get coefficients */
                regcf_(&np,&nrbar,dd,r,theta,tol,
                        work,&np,&fail); /*work=coefficients*/

/* reset things */
                clear_(&np,&nrbar,dd,r,theta,&sser,&fail);
/* compute again */
                for(i=0;i<nobs;i++) {
/* collect the observation */
                        get_data_vector(&y,x,i,XofV,xp,yp);
                        yvec[i] = y;
/* compute weighting */
                        w = calc_resid_y(i,work,np,xp,yp); /* Y-Yhat */
                        w = y - w; /* Yhat */
                        w = (w*(1.0-w));  /* weighting factor (Yhat)(1-Yhat) */

                        includ_(&np,&nrbar,&w,x,&y,dd,r,
                                theta,&sser,&fail);
                }
	}

/* set singular tolerances */
	tolset_(&np,&nrbar,dd,r,tol,work,&fail);
/* check for singularities */
	sing_(&np,&nrbar,dd,r,theta,&sser,tol,lindep,work,&fail);

/* extract needed regression info */
	ss_(&np,dd,theta,&sser,ss,&fail);  /* ss=SS left after adding var i */

/* and SS_TOTAL sum(y*y -2.0*y*ybar + ybar*ybar) */
	y = sumy/(double)(nobs);
	sst = ssumy -2.0*y*sumy + y*y*(double)(nobs);
	
/* get coefficients */
        regcf_(&np,&nrbar,dd,r,theta,tol,work,&np,&fail); /*work=coefficients*/

/* initialize the order structures */
	for(i=0;i<np;i++) {
		vorder[i] = i;
	}
 
	if (verbose > 3)  {
		fprintf(stdout,"Coefficients:\n");
		for(i=0;i<np;i++) fprintf(stdout,"%.3f ",work[i]);
		fprintf(stdout,"\nSS: Total:%.3f\n",sst);
		for(i=0;i<np;i++) fprintf(stdout,"%.3f ",ss[i]);
		fprintf(stdout,"\n");
	}

/* compute the PRESS (only if needed!) */
	press = 0.0;
if (volume.need_press) {
	for(i=0;i<nobs;i++) {
		get_data_vector(&y,x,i,XofV,xp,yp);
/* get the hat diagonal for obs i */
		hdiag_(x,&np,&nrbar,dd,r,tol,&np,&hii,work2,&fail);
/* calc the residual */
		w = calc_resid_y(i,work,np,xp,yp)/(1.0-hii);
/* build the PRESS */
		press += (w*w);
	}
}

/* fill in the output volumes (possibly extracting extra regression info?) */
	for(i=0;i<noutputs;i++) {
		switch(outputs[i].type) {
			case OUT_PRESS:
				outputs[i].data[INDX] = press;
	if (verbose > 3) {
		fprintf(stdout,"PRESS=%f\n",outputs[i].data[INDX]);
	}
				break;
			case OUT_R2_PRED:
				outputs[i].data[INDX] = 1.0 - (press/sst);
	if (verbose > 3) {
		fprintf(stdout,"Prediction Rsqu=%f\n",outputs[i].data[INDX]);
	}
				break;
			case OUT_R2:
				outputs[i].data[INDX] = 1.0 - (sser/sst);
	if (verbose > 3) {
		fprintf(stdout,"Rsqu=%f\n",outputs[i].data[INDX]);
	}
				break;
			case OUT_R2_ADJ:
				outputs[i].data[INDX] = 1.0 - 
					((sser/(nobs-np))/(sst/(nobs-1)));
	if (verbose > 3) {
		fprintf(stdout,"Adj Rsqu=%f\n",outputs[i].data[INDX]);
	}
				break;
			case OUT_SS_MODEL:
				outputs[i].data[INDX] = sst-sser;
	if (verbose > 3) {
		fprintf(stdout,"SS Model=%f\n",outputs[i].data[INDX]);
	}
				break;
			case OUT_SS_TOTAL:
				outputs[i].data[INDX] = sst;
	if (verbose > 3) {
		fprintf(stdout,"SS Total=%f\n",outputs[i].data[INDX]);
	}
				break;
			case OUT_VAR_SS:
				j = XofV[outputs[i].var];
				if (modeltemp.vars[outputs[i].var].type ==
					VAR_CLASS)
				{
					num = modeltemp.vars[
						outputs[i].var].value-1;
					j += (num-1);
				}
				k = XofV[outputs[i].var]-1;
				if (k < 0) {
					outputs[i].data[INDX] = sst-sser-ss[j];
				} else {
					outputs[i].data[INDX] = ss[k] - ss[j];
				}

	if (verbose > 3) {
		fprintf(stdout,"Sequential SS(%s)=%f,%ld\n",
			outputs[i].classname,outputs[i].data[INDX],
			XofV[outputs[i].var]);
	}
				break;
			case OUT_VAR_PSS:
				j = XofV[outputs[i].var];
				if (modeltemp.vars[outputs[i].var].type ==
					VAR_CLASS)
				{
					num = modeltemp.vars[
						outputs[i].var].value-1;
					j += (num-1);
				}
				k = XofV[outputs[i].var]-1;
				outputs[i].data[INDX] = calc_pss(np,nrbar,
					vorder,dd,r,theta,ss,tol,norder,k,j);

	if (verbose > 3) {
		fprintf(stdout,"Partial SS(%s)=%f,%ld\n",
			outputs[i].classname,outputs[i].data[INDX],
			XofV[outputs[i].var]);
	}
				break;
			case OUT_COV_COEF:
				outputs[i].data[INDX] = work[XofV[
					outputs[i].var]];
	if (verbose > 3) {
		fprintf(stdout,"Coeff(%s)=%f\n",outputs[i].classname,
				outputs[i].data[INDX]);
	}
				break;
			case OUT_VAR_COV:
				outputs[i].data[INDX] = calc_var_cov(np,nrbar,
					dd,r,outputs[i].var,outputs[i].var2);
	if (verbose > 3) {
		fprintf(stdout,"Covar(%s,%s)=%f\n",outputs[i].classname,
				outputs[i].classvalue,outputs[i].data[INDX]);
	}
				break;
			case OUT_COV_STDERR:
				for(j=0;j<np;j++) x[j] = 0.0;
				x[XofV[outputs[i].var]] = 1.0;
				hdiag_(x,&np,&nrbar,dd,r,tol,&np,&hii,
					work2,&fail);
				w = sser/(double)(nobs-np);
				outputs[i].data[INDX] = sqrt(w)*sqrt(hii);
	if (verbose > 3) {
		fprintf(stdout,"StdError(%s)=%f\n",outputs[i].classname, 
			outputs[i].data[INDX]);
	}
				break;
			case OUT_RES_ERR:
				outputs[i].data[INDX] = sser;
	if (verbose > 3) {
		fprintf(stdout,"SS Error=%f\n",sser);
	}
				break;
			case OUT_ADJ_Y_M:
				y = 0;
				for(j=0;j<nobs;j++){
/* adjust for everything */
					y += calc_adj_y(j,-1,work,np,xp,yp); 
				}
				outputs[i].data[INDX] = y/(double)(nobs);
	if (verbose > 3) {
		fprintf(stdout,"ADJ_Y_M=%f\n",outputs[i].data[INDX]);
	}
				break;
			case OUT_CLASS_M:
				y = 0;
				num = 0;
/* find all obs */
				for(j=0;j<nobs;j++) {
/* with a CLASS variable */
				for(k=0;k<modeltemp.nvars;k++) {
	if ((obs[j].vars[k].type == VAR_CLASS) || 
			(obs[j].vars[k].type == VAR_LABEL)) {
/* whose label matches the output classname */
					if (strcmp(outputs[i].classname,
						obs[j].vars[k].label) == 0) {
/* and name matches the classvalue */
					if (strcmp(outputs[i].classvalue,
						obs[j].vars[k].name) == 0) {
/* get adjusted value, but do not adjust for variable k */
						y += calc_adj_y(j,k,work,np,
							xp,yp); 
						num++;
					}
					}
					}
				}
				}
				if (num > 0) {
					outputs[i].data[INDX] = y/(double)(num);
				} else {
					outputs[i].data[INDX] = 0.0;
				}
	if (verbose > 3) {
		fprintf(stdout,"CLASS_M(%s,%s)=%f\n",outputs[i].classname,
			outputs[i].classvalue,outputs[i].data[INDX]);
	}
				break;
		}
	}

/* free up the memory */
	free(ss);
	free(r);
	free(dd);
	free(theta);
	free(tol);
	free(work);
	free(work2);
	free(lindep);
	free(vorder);
	free(norder);
	free(x);
	free(yvec);

/* done */
	return;
}

/* for a given observation, skipping variable SKIP, return adjusted Y */
double calc_adj_y(long int i,long int skip,double *coef,long int np,
	long int xp,long int yp)
{
	long int	XofV[MAX_VARS]; 
	double		y;
	double		*x;
	long int	j,k,num;

	x = (double *)malloc(np*sizeof(double));
	if (x == 0) ex_err("Unable to allocate vector memory.");

/* get the data */
	get_data_vector(&y,x,i,XofV,xp,yp);
/* find covariates */
	for(j=1;j<obs[i].nvars;j++) {
/* skip this variable? */
		if (j == skip) continue;
/* subtract the difference from the mean times the coefficient */
		if (obs[i].vars[j].type == VAR_COV_C) {
			y = y - coef[XofV[j]]*(x[XofV[j]] -
				 modeltemp.vars[j].mean);
		} else if (obs[i].vars[j].type == VAR_COV_V) {
			y = y - coef[XofV[j]]*(x[XofV[j]] -
				modeltemp.vars[j].mean);
		} else if (volume.cov_only) {
/* stop here if covariates only */
			continue;
		} else if (obs[i].vars[j].type == VAR_CONST) {
/* Constants are subtracted */
			y = y - coef[XofV[j]]*x[XofV[j]];
		} else if (obs[i].vars[j].type == VAR_CLASS) {
/* normalize to the first level of each CLASS */
			num = modeltemp.vars[j].value-1;
			for(k=XofV[j];k<(XofV[j]+num);k++) {
				y = y - coef[k]*x[k];
			}
		}
	}
/* done */
	free(x);

	return(y);
}

/* compute the residual Y value */
double calc_resid_y(long int i,double *coef,long int np,long int xp,
	long int yp)
{
	long int	XofV[MAX_VARS]; 
	double		y;
	double		*x;
	long int	j,k,num;

	x = (double *)malloc(np*sizeof(double));
	if (x == 0) ex_err("Unable to allocate vector memory.");

/* get the data */
	get_data_vector(&y,x,i,XofV,xp,yp);
/* find covariates */
	for(j=1;j<obs[i].nvars;j++) {
/* subtract the difference from the mean times the coefficient */
		if (obs[i].vars[j].type == VAR_COV_C) {
			y = y - coef[XofV[j]]*x[XofV[j]];
		} else if (obs[i].vars[j].type == VAR_COV_V) {
			y = y - coef[XofV[j]]*x[XofV[j]];
		} else if (obs[i].vars[j].type == VAR_CONST) {
			y = y - coef[XofV[j]]*x[XofV[j]];
		} else if (obs[i].vars[j].type == VAR_CLASS) {
/* normalize to the first level of each CLASS */
			num = modeltemp.vars[j].value-1;
			for(k=XofV[j];k<(XofV[j]+num);k++) {
				y = y - coef[k]*x[k];
			}
		}
	}
/* done */
	free(x);

	return(y);
}

/* get the data vector for a given observation */
void get_data_vector(double *y,double *x,long int i,long int *XofV,
		long int xp,long int yp)
{
	long int	l,j,k;
	long int	num;

/* collect the observations */
	*y = get_v_value(obs[i].vars[0].data,obs[i].vars[0].dz,INDX);

	*y = log_xform(*y);

/* first value is always the Y VAR, others get placed into the x arrary */
	k = 0; /* pointer into the X array */
	for(j=1;j<obs[i].nvars;j++) {
		switch(obs[i].vars[j].type) {
			case VAR_Y_VAL:  /* should never happen */
			case VAR_Y_CONST:  /* should never happen */
				ex_err("Found Y value in a bad location.");
				break;
			case VAR_CONST:
				x[k] = obs[i].vars[j].value;
				x[k] = log_xform(x[k]);
				XofV[j] = k;
				k++;
				break;
			case VAR_COV_V:
				x[k] = get_v_value(obs[i].vars[j].data,
					obs[i].vars[j].dz,INDX);
				x[k] = log_xform(x[k]);
				XofV[j] = k;
				k++;
				break;
			case VAR_COV_C:
				x[k] = obs[i].vars[j].value;
				x[k] = log_xform(x[k]);
				XofV[j] = k;
				k++;
				break;
			case VAR_CLASS:
/* number of variables need for this class (levels-1) */
				if (modeltemp.vars[j].value < 2) {
		ex_err("All CLASS variables must have at least two levels");
				}
				num = modeltemp.vars[j].value-1;
				for(l=0;l<num;l++) x[k+l] = 0;
				if (obs[i].vars[j].value > 1.0) {
					l = obs[i].vars[j].value;
					x[k+l-2]=1.0;
				}
				XofV[j] = k;
				k += num;
				break;
			case VAR_LABEL:
				break;
		}
	}
	return;
}

/* log transform */
double log_xform(double x)
{
	static	int log_warn = 0;

	if (volume.log_xform) {
		if (x > 0) {
			return(log10(x));
		} else {
			if (log_warn == 0) {
				log_warn = 1;
				fprintf(stderr,
	"Warning, log10 of %f attempted.  This is your only warning.\n",x);
			}
			return(0);
		}
	} else {
		return(x);
	}
}

/* order so that the current coefficients are last... */
double	calc_pss(long int np,long int nrbar,long int *vorder,double *dd,
	double *r,double *theta,double *x,double *tol,long int *norder,
	long int k,long int j)
{
	long int	pos1 = 1;
	long int	i,num,l;
	long int	ptr,l1;
	long int	fail,done,TO,FROM;
	double		pss;

	ptr = 0;
	for(i=0;i<=k;i++) norder[ptr++] = i;
	for(i=j+1;i<np;i++) norder[ptr++] = i;
	l1 = ptr;
	for(i=k+1;i<=j;i++) norder[ptr++] = i;

	num = np-1;

if (verbose > 4) {
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

/*
      SUBROUTINE VMOVE(NP, NRBAR, VORDER, D, RBAR, THETAB, RSS, FROM,
     +    TO, TOL, IER)
C     Move variable from position FROM to position TO in an


 	OLDER CODE!!!
	reordr_(&np,&nrbar,vorder,dd,r,theta,x,tol,norder,&num,&pos1,&fail);

*/

	pss = x[l1-1] - x[np-1];

if (verbose > 4) {
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
}

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

/*
	reordr_(&np,&nrbar,vorder,dd,r,theta,x,tol,norder,&num,&pos1,&fail);
*/

if (verbose > 4) {
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
}

	return(pss);
}

double calc_var_cov(long int np, long int nrbar,double *dd,double *r, 
	long int var1, long int var2)
{
	double	*inv,*covmat,*sterr;
	double	d = 0.0,var = 1.0;
	long int fail,dimcov,i,j,k;

	dimcov = np*(np+1)/2;
	inv = (double *)malloc(sizeof(double)*nrbar);
	covmat = (double *)malloc(sizeof(double)*dimcov);
	sterr = (double *)malloc(sizeof(double)*np);
	if ((!inv) || (!covmat) || (!sterr)) {
		ex_err("Unable to allocate var_cov memory\n");
	}

	cov_(&np,&nrbar,dd,r,&np,inv,&var,covmat,&dimcov,sterr,&fail);

	k = 0;
	for(i=0;i<np;i++) {
		for(j=i;j<np;j++) {
			if ((i == var1) && (j == var2)) d = covmat[k];
			if ((i == var2) && (j == var1)) d = covmat[k];
			k++;
		}
	}

	free(inv);
	free(covmat);
	free(sterr);

	return(d);
}
