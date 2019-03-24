/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski
 *
 * $Id: regress_parse.c 1599 2006-01-22 03:45:10Z rjfrank $
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
#include <ctype.h>  /* toupper */
#include "tal_regress.h"

/* local prptotypes */
long int get_tok_int(char *s);
double 	get_tok_double(char *s);
char 	*get_tok_string(char *s);
double 	get_class_value(long int n,char *token);
void 	ucase(char *str);
void	unknown(char *blk,char *val);
void	prefix_filename(char *file,long int output);

void ucase(char *str)
{
	long int i;
	i = 0;
	while (str[i] != '\0') {
		str[i] = toupper((int)(str[i]));
		i++;
	}
	return;
}

double get_class_value(long int n,char *token)
{
	long int 	i;
	double 		count;

	count = 0;
	i = 0;
	while(i<nobs) {
/* if there is a match, return the matched value */
		if (strcmp(token,obs[i].vars[n].name) == 0) {
			return(obs[i].vars[n].value);
		} else {
/* else retain the largest value */
			if (obs[i].vars[n].value > count) {
				count = obs[i].vars[n].value;
			}
		}
		i++;
	}
/* if no matches, assign the value count+1 as we have a new classvalue */
	count = count + 1.0;
/* the value in the modeltemp is the number of classvalues */
	modeltemp.vars[n].value = count;
/* return the value */
	return(count);
}

void	unknown(char *blk,char *val)
{
	if (val[0] == '#') return; /* comment */

	fprintf(stderr,"Warning: unknown token %s found in block %s\n",
		val,blk);
	return;
}

int	parse_input(FILE *fp)
{
	char		text[256],token[256];
	long int	i,j;
	long int	varnum = 0;
	long int	state;
	float		wei;

/* defaults */
	volume.dx = 256;
	volume.dy = 256;	
	volume.first = 1;
	volume.last = 119;
	volume.step = 1;
	volume.icount = 0;
	volume.swap = 0;
	volume.offset = 0.0;
	volume.masktemp = 0;
	volume.maskdata = 0;
	volume.weighttemp = 0;
	volume.weightdata = 0;
	volume.cov_only = 1;
	volume.log_xform = 0;
	volume.single_fit = 0;
	volume.need_press = 0;
	volume.y_boolean = 0;
	volume.input_prefix = 0L;
	volume.output_prefix = 0L;

        modeltemp.vars = (var *)calloc(MAX_VARS,sizeof(var));
        if (!modeltemp.vars) ex_err("Unable to allocate modeltemp.vars");

	state = 0;
	while(fgets(text,256,fp)!=0L) {
		sscanf(text,"%s",token);
		if (token[0] == '#') continue;
		ucase(token);
		switch(state) {
			case 0: /* wait for next block type */
				if (strcmp(token,"VOLPARAMS")==0) {
					state=1;
				};
				break;
			case 1: /* read a VOLPARAMS block */
				if (strcmp(token,"MODEL")==0) { 
					state=2;
					modeltemp.nvars = 0;
					for(i=volume.first;i<=volume.last;
					    i+=volume.step) volume.icount += 1;
/* check for ability to handle y_boolean */
					if (volume.single_fit && 
						volume.y_boolean) {
						if ((volume.dx != 1) ||
						    (volume.dy != 1) ||
						    (volume.icount != 1)) {
   ex_err("(Y_BOOLEAN + SINGLE_FIT) is only valid with a single pixel volume");
						}
					}
					if ((!volume.single_fit) &&
						volume.weighttemp) {
   fprintf(stderr,"Warning: WEIGHT param makes most sense with SINGLE_FIT");
					}
				} else if (strcmp(token,"DX")==0) {
					volume.dx = get_tok_int(text);
				} else if (strcmp(token,"DY")==0) {
					volume.dy = get_tok_int(text);
				} else if (strcmp(token,"FIRST")==0) {
					volume.first = get_tok_int(text);
				} else if (strcmp(token,"LAST")==0) {
					volume.last = get_tok_int(text);
				} else if (strcmp(token,"STEP")==0) {
					volume.step = get_tok_int(text);
				} else if (strcmp(token,"OFFSET")==0) {
					volume.offset = get_tok_double(text);
				} else if (strcmp(token,"MASK")==0) {
					volume.masktemp = get_tok_string(text);
				} else if (strcmp(token,"WEIGHT")==0) {
					volume.weighttemp = get_tok_string(text);	
				} else if (strcmp(token,"SWAP")==0) {
					volume.swap = 1;
				} else if (strcmp(token,"ADJ_COV_ONLY")==0) {
					volume.cov_only = 1;
				} else if (strcmp(token,"ADJ_ALL_VARS")==0) {
					volume.cov_only = 0;
				} else if (strcmp(token,"LOG_TRANSFORM")==0) {
					volume.log_xform = 1;
				} else if (strcmp(token,"SINGLE_FIT")==0) {
					volume.single_fit = 1;
				} else if (strcmp(token,"Y_BOOLEAN")==0) {
					volume.y_boolean = 1;
				} else if (strcmp(token,"INPUT_PREFIX")==0) {
				    volume.input_prefix = get_tok_string(text);
				} else if (strcmp(token,"OUTPUT_PREFIX")==0) {
				    volume.output_prefix = get_tok_string(text);
				} else {
					unknown("VOLPARAMS",token);
				}
				break;
			case 2: /* read a MODEL block */
/* too many VARs */
				if (modeltemp.nvars >= MAX_VARS-2) ex_err(
				    "Maximum number of variables exceeded.");
				if ((modeltemp.nvars == 1) &&
				((modeltemp.vars[0].type != VAR_Y_VAL)
				 && (modeltemp.vars[0].type != VAR_Y_CONST))) {
			ex_err("The first variable in a MODEL must be Y");
				}

				modeltemp.vars[modeltemp.nvars].data = 0L;
				modeltemp.vars[modeltemp.nvars].dz = 0;

				if (strcmp(token,"OUTPUT")==0) {
					state=3;
					noutputs = 0;
					if (modeltemp.nvars < 2) {  /* Y,X */
				ex_err("Not enough variables in the model.");
					}
				} else if (strcmp(token,"Y") == 0) {
/* Y name dz (must be first!) */
			if (modeltemp.nvars != 0) {
	ex_err("The Y value in a MODEL must be the first variable.");
			}
			modeltemp.vars[modeltemp.nvars].type = VAR_Y_VAL;
			sscanf(text,"%s %s %ld",token,
				modeltemp.vars[modeltemp.nvars].label,
				&(modeltemp.vars[modeltemp.nvars].dz));
			modeltemp.nvars++;

				} else if (strcmp(token,"Y_CONST") == 0) {
/* Y_CONST name (must be first!) */
			if (modeltemp.nvars != 0) {
	ex_err("The Y value in a MODEL must be the first variable.");
			}
			modeltemp.vars[modeltemp.nvars].type = VAR_Y_CONST;
			sscanf(text,"%s %s",token,
				modeltemp.vars[modeltemp.nvars].label);
			modeltemp.vars[modeltemp.nvars].dz = 4; /* float */
			modeltemp.nvars++;

				} else if (strcmp(token,"CONST") == 0) {
/* CONST (1.0 implied) */
			if (modeltemp.nvars != 1) {
	ex_err("The CONST value in a MODEL must be the second variable.");
			}
			modeltemp.vars[modeltemp.nvars].type = VAR_CONST;
			strcpy(modeltemp.vars[modeltemp.nvars].label,"CONST");
			modeltemp.vars[modeltemp.nvars].value = 1.0;
			modeltemp.nvars++;

				} else if (strcmp(token,"COVARIABLE_VOL") == 0) {
/* COVARIABLE_VOL name mean dz */
			modeltemp.vars[modeltemp.nvars].type = VAR_COV_V;
			sscanf(text,"%s %s %lf %ld",token,
				modeltemp.vars[modeltemp.nvars].label,
				&(modeltemp.vars[modeltemp.nvars].mean),
				&(modeltemp.vars[modeltemp.nvars].dz));
			modeltemp.nvars++;

				} else if (strcmp(token,"COVARIABLE_CONST") == 0) {
/* COVARIABLE_CONST name mean */
			modeltemp.vars[modeltemp.nvars].type = VAR_COV_C;
			sscanf(text,"%s %s %lf",token,
				modeltemp.vars[modeltemp.nvars].label,
				&(modeltemp.vars[modeltemp.nvars].mean));
			modeltemp.nvars++;

				} else if (strcmp(token,"CLASS") == 0) {
/* CLASS name */
			modeltemp.vars[modeltemp.nvars].type = VAR_CLASS;
			sscanf(text,"%s %s",token,
				modeltemp.vars[modeltemp.nvars].label);
/* no classvalues yet */
			modeltemp.vars[modeltemp.nvars].value = 0.0; 
			modeltemp.nvars++;

				} else if (strcmp(token,"LABELCLASS") == 0) {
/* LABELCLASS name */
			modeltemp.vars[modeltemp.nvars].type = VAR_LABEL;
			sscanf(text,"%s %s",token,
				modeltemp.vars[modeltemp.nvars].label);
/* no classvalues yet */
			modeltemp.vars[modeltemp.nvars].value = 0.0; 
			modeltemp.nvars++;

				} else {
					unknown("MODEL",token);
				}
				break;
			case 3: /* read the OUTPUT block */
/* too many OUTPUT lines */
				if (noutputs >= MAX_OUTPUTS-1) ex_err(
				  "Maximum number of output volumes exceeded.");

				outputs[noutputs].data = 0L;
				if (strcmp(token,"OBSERVATIONS")==0) {
					state=4;
					varnum = 0;
					nobs = 0;
					obs[nobs].weight = 1.0;
					obs[nobs].vars = NULL;
					if (noutputs < 1) {
					ex_err("No output data requested.");
					}
				} else if (strcmp(token,"SS_ERROR")==0) {
/* SS_ERROR template */
					outputs[noutputs].type = OUT_RES_ERR;
					sscanf(text,"%s %s",token,
						outputs[noutputs].tempname);
				prefix_filename(outputs[noutputs].tempname,1);
					noutputs++;
				} else if (strcmp(token,"ADJ_Y_CLASS_MEAN")==0) {
/* ADJ_Y_CLASS_MEAN classname classvalue template */
					outputs[noutputs].type = OUT_CLASS_M;
					sscanf(text,"%s %s %s %s",token,
						outputs[noutputs].classname,
						outputs[noutputs].classvalue,
						outputs[noutputs].tempname);
				prefix_filename(outputs[noutputs].tempname,1);
					outputs[noutputs].var = findvar(
						outputs[noutputs].classname);
					if (outputs[noutputs].var < 0) {
	ex_err("Adj mean class output requested for non-existant class");
					}
					noutputs++;
	if (volume.single_fit) {
	ex_err("ADJ_Y_CLASS_MEAN cannot be output for a single regression");
	}
				} else if (strcmp(token,"ADJ_Y_MEAN")==0) {
/* ADJ_Y_MEAN template */
					outputs[noutputs].type = OUT_ADJ_Y_M;
					sscanf(text,"%s %s",token,
						outputs[noutputs].tempname);
				prefix_filename(outputs[noutputs].tempname,1);
					noutputs++;
	if (volume.single_fit) {
	ex_err("ADJ_Y_MEAN cannot be output for a single regression");
	}
				} else if (strcmp(token,"SS_VARIABLE")==0) {
/* SS_VARIABLE varname template */
					outputs[noutputs].type = OUT_VAR_SS;
					sscanf(text,"%s %s %s",token,
						outputs[noutputs].classname,
						outputs[noutputs].tempname);
				prefix_filename(outputs[noutputs].tempname,1);
					outputs[noutputs].var = findvar(
						outputs[noutputs].classname);
					if (outputs[noutputs].var < 0) {
	ex_err("SS output requested for non-existant variable");
					}
	if (modeltemp.vars[outputs[noutputs].var].type == VAR_LABEL) {
	ex_err("SS output cannot be computed for a labelclass");
	}
					noutputs++;
				} else if (strcmp(token,"P_SS_VARIABLE")==0) {
/* P_SS_VARIABLE varname template */
					outputs[noutputs].type = OUT_VAR_PSS;
					sscanf(text,"%s %s %s",token,
						outputs[noutputs].classname,
						outputs[noutputs].tempname);
				prefix_filename(outputs[noutputs].tempname,1);
					outputs[noutputs].var = findvar(
						outputs[noutputs].classname);
					if (outputs[noutputs].var < 0) {
	ex_err("Partial SS output requested for non-existant variable");
					}
	if (modeltemp.vars[outputs[noutputs].var].type == VAR_LABEL) {
	ex_err("Partial SS output cannot be computed for a labelclass");
	}
					noutputs++;
				} else if (strcmp(token,"COVARIABLE_SLOPE")==0) {
/* COVARIABLE_SLOPE covname template */
					outputs[noutputs].type = OUT_COV_COEF;
					sscanf(text,"%s %s %s",token,
						outputs[noutputs].classname,
						outputs[noutputs].tempname);
				prefix_filename(outputs[noutputs].tempname,1);
					outputs[noutputs].var = findvar(
						outputs[noutputs].classname);
					if (outputs[noutputs].var < 0) {
	ex_err("Cov coef output requested for non-existant variable");
					}
	if (modeltemp.vars[outputs[noutputs].var].type == VAR_CLASS) {
	ex_err("Coef output can not be requested for a class variable");
	}
	if (modeltemp.vars[outputs[noutputs].var].type == VAR_LABEL) {
	ex_err("Coef output can not be requested for a labelclass variable");
	}
					noutputs++;
				} else if (strcmp(token,"COEF_COVARIANCE")==0) {
/* COEF_COVARIANCE varname1 varname2 template */
					outputs[noutputs].type = OUT_VAR_COV;
					sscanf(text,"%s %s %s %s",token,
						outputs[noutputs].classname,
						outputs[noutputs].classvalue,
						outputs[noutputs].tempname);
				prefix_filename(outputs[noutputs].tempname,1);
					outputs[noutputs].var = findvar(
						outputs[noutputs].classname);
					outputs[noutputs].var2 = findvar(
						outputs[noutputs].classvalue);
					if ((outputs[noutputs].var < 0) ||
				            (outputs[noutputs].var2 < 0)) {
	ex_err("Cov coef output requested for non-existant variable");
					}
	if ((modeltemp.vars[outputs[noutputs].var].type == VAR_CLASS) ||
	    (modeltemp.vars[outputs[noutputs].var2].type == VAR_CLASS)) {
	ex_err("Coef output can not be requested for a class variable");
	}
	if ((modeltemp.vars[outputs[noutputs].var].type == VAR_LABEL) ||
	    (modeltemp.vars[outputs[noutputs].var2].type == VAR_LABEL)) {
	ex_err("Coef output can not be requested for a labelclass variable");
	}
					noutputs++;
				} else if (strcmp(token,"COVARIABLE_STD_ERROR")==0) {
/* COVARIABLE_STD_ERROR covname template */
					outputs[noutputs].type = OUT_COV_STDERR;
					sscanf(text,"%s %s %s",token,
						outputs[noutputs].classname,
						outputs[noutputs].tempname);
				prefix_filename(outputs[noutputs].tempname,1);
					outputs[noutputs].var = findvar(
						outputs[noutputs].classname);
					if (outputs[noutputs].var < 0) {
	ex_err("Cov coef output requested for non-existant variable");
					}
	if (modeltemp.vars[outputs[noutputs].var].type == VAR_CLASS) {
	ex_err("Coef output can not be requested for a class variable");
	}
	if (modeltemp.vars[outputs[noutputs].var].type == VAR_LABEL) {
	ex_err("Coef output can not be requested for a labelclass variable");
	}
					noutputs++;
				} else if (strcmp(token,"SS_TOTAL")==0) {
/* SS_TOTAL template */
					outputs[noutputs].type = OUT_SS_TOTAL;
					sscanf(text,"%s %s",token,
						outputs[noutputs].tempname);
				prefix_filename(outputs[noutputs].tempname,1);
					noutputs++;
				} else if (strcmp(token,"SS_MODEL")==0) {
/* SS_MODEL template */
					outputs[noutputs].type = OUT_SS_MODEL;
					sscanf(text,"%s %s",token,
						outputs[noutputs].tempname);
				prefix_filename(outputs[noutputs].tempname,1);
					noutputs++;
				} else if (strcmp(token,"SS_PRESS")==0) {
/* SS_PRESS template */
	if ((volume.single_fit) && ((volume.dx != 1) || (volume.dy != 1) ||
		(volume.icount != 1))) {
  ex_err("SS_PRESS cannot be output for SINGLE_FIT w/o a single pixel volume");
	}
					outputs[noutputs].type = OUT_PRESS;
					sscanf(text,"%s %s",token,
						outputs[noutputs].tempname);
				prefix_filename(outputs[noutputs].tempname,1);
					noutputs++;
					volume.need_press = 1;
				} else if (strcmp(token,"R2_MODEL")==0) {
/* R2_MODEL template */
					outputs[noutputs].type = OUT_R2;
					sscanf(text,"%s %s",token,
						outputs[noutputs].tempname);
				prefix_filename(outputs[noutputs].tempname,1);
					noutputs++;
				} else if (strcmp(token,"R2_ADJ_MODEL")==0) {
/* R2_ADJ_MODEL template */
					outputs[noutputs].type = OUT_R2_ADJ;
					sscanf(text,"%s %s",token,
						outputs[noutputs].tempname);
				prefix_filename(outputs[noutputs].tempname,1);
					noutputs++;
				} else if (strcmp(token,"R2_PRED_MODEL")==0) {
/* R2_PRED_MODEL template */
	if ((volume.single_fit) && ((volume.dx != 1) || (volume.dy != 1) ||
		(volume.icount != 1))) {
  ex_err("R2_PRED_MODEL cannot be output for SINGLE_FIT w/o a single pixel volume");
	}
					outputs[noutputs].type = OUT_R2_PRED;
					sscanf(text,"%s %s",token,
						outputs[noutputs].tempname);
				prefix_filename(outputs[noutputs].tempname,1);
					noutputs++;
					volume.need_press = 1;
				} else {
					unknown("OUTPUT",token);
				}
				break;
			case 4: /* read an OBS block */
/* too many OBS objects? */
				if (nobs >= MAX_OBS-2) ex_err(
				    "Maximum number of observations exceeded.");
					
				if (strcmp(token,"END")==0) {
					state=5;
					break;
				}
/* re-read the token (IN LOWERCASE) */
				sscanf(text,"%s",token);
/* init obs[nobs].vars[varnum] */
				obs[nobs].nvars = modeltemp.nvars;
                                if (!obs[nobs].vars) {
				  obs[nobs].vars=(var *)calloc(modeltemp.nvars,
                                                             sizeof(var));
            			  if (!obs[nobs].vars) {
				    ex_err("Unable to allocate obs memory");
                                  }
               			}
				obs[nobs].vars[varnum].data = 0L;
				obs[nobs].vars[varnum].dz = 
					modeltemp.vars[varnum].dz;
/* copy the label and the type */
				obs[nobs].vars[varnum].type = 
					modeltemp.vars[varnum].type;
				strcpy(obs[nobs].vars[varnum].label,
					modeltemp.vars[varnum].label);
/* read the data for variable VARNUM into obs[nobs].vars[varnum] */
				switch(obs[nobs].vars[varnum].type) {

					case VAR_Y_VAL:
				wei = -1.0;
				sscanf(text,"%s %f",
					obs[nobs].vars[varnum].name,&wei);
				if (wei != -1.0) obs[nobs].weight = wei;
				prefix_filename(obs[nobs].vars[varnum].name,0);
						break;

					case VAR_Y_CONST:
/* cheat and make the name a volume constant */
				wei = -1.0;
				sscanf(text,"%lf %f",
					&(obs[nobs].vars[varnum].value),&wei);
				if (wei != -1.0) obs[nobs].weight = wei;
				sprintf(obs[nobs].vars[varnum].name,"=%f",
					obs[nobs].vars[varnum].value); 
						break;

					case VAR_CONST:
				obs[nobs].vars[varnum].value = atof(token);
				strcpy(obs[nobs].vars[varnum].name,"CONST");
						break;

					case VAR_COV_V:
				strcpy(obs[nobs].vars[varnum].name,token);
				prefix_filename(obs[nobs].vars[varnum].name,0);
				obs[nobs].vars[varnum].mean =
					modeltemp.vars[varnum].mean;
						break;

					case VAR_COV_C:
				obs[nobs].vars[varnum].value = atof(token);
				obs[nobs].vars[varnum].mean =
					modeltemp.vars[varnum].mean;
						break;

					case VAR_CLASS:
				strcpy(obs[nobs].vars[varnum].name,token);
				obs[nobs].vars[varnum].value = get_class_value(
					varnum,token);
						break;

					case VAR_LABEL:
				strcpy(obs[nobs].vars[varnum].name,token);
				obs[nobs].vars[varnum].value = get_class_value(
					varnum,token);
						break;
				}
/* if we have read all the variables, count this as one observation */
				varnum += 1;
				if (varnum >= modeltemp.nvars) {
					nobs += 1;
					obs[nobs].weight = 1.0;
					obs[nobs].vars = NULL;
					varnum = 0;
				}
				break;

			case 5: /* ignore trailing text */
				break;
		}
	}
/* check validity */
	if (state < 4) {
		ex_err("Premature end of file. No regression defined.");
	}
	if ((nobs < 2) && (volume.single_fit == 0)) {
		ex_err("More than two observations needed for an analysis.");
	}

/* allocate the image buffers */
	volume.maskdata = malloc(volume.dx*volume.dy); /* 8bit */
	if (volume.maskdata == 0) ex_err("Unable to allocate image memory.");
	
	
/* allocate the weight buffer */
	volume.weightdata = (float *)malloc(volume.dx*volume.dy*sizeof(float)); /* floating point 32-bit */
	if (volume.weightdata == 0) ex_err("Unable to allocate memory for weight image.");
		
/* allocate image buffer for volumetric inputs */
	for(i=0;i<nobs;i++) {
		for(j=0;j<obs[i].nvars;j++) {
			switch(obs[i].vars[j].type) {
				case VAR_Y_VAL:
				case VAR_Y_CONST:
				case VAR_COV_V:
					obs[i].vars[j].data = malloc(volume.dx*
						volume.dy*obs[i].vars[j].dz);
					if (obs[i].vars[j].data == 0) ex_err(
					"Unable to allocate image memory.");
					break;
			}
		}
	}
/* all outputs have volume data (for now) */
	for(i=0;i<noutputs;i++) {
		outputs[i].data = malloc(volume.dx*volume.dy*sizeof(float));
		if (outputs[i].data == 0) ex_err(
			"Unable to allocate image memory.");
	}

	return(0);
}

void freebuffers()
{
	long int	i,j;

/* free the data buffers */
	if (volume.maskdata != 0) free(volume.maskdata);
	if (volume.masktemp != 0) free(volume.masktemp);
	volume.maskdata = 0L;
	volume.masktemp = 0L;
	if (volume.weighttemp != 0) free(volume.weighttemp);
	if (volume.weightdata != 0) free(volume.weightdata);
	volume.weightdata = 0L;
	volume.weighttemp = 0L;
	for(i=0;i<nobs;i++) {
		for(j=0;j<obs[i].nvars;j++) {
			if (obs[i].vars[j].data != 0) free(obs[i].vars[j].data);
			obs[i].vars[j].data = 0L;
		}
	}
	for(i=0;i<noutputs;i++) {
		if (outputs[i].data != 0L) free(outputs[i].data);
		outputs[i].data = 0L;
	}

/* free the prefix buffers */
	if (volume.input_prefix != 0) free(volume.input_prefix);
	if (volume.output_prefix != 0) free(volume.output_prefix);

	return;
}

long int get_tok_int(char *s)
{
	char		junk[256];
	long int	i;

	sscanf(s,"%s %ld",junk,&i);
	return(i);
}

double get_tok_double(char *s)
{
	char		junk[256];
	double		i;

	sscanf(s,"%s %lf",junk,&i);
	return(i);
}

char *get_tok_string(char *s)
{
	char		junk[256];
	char		*junk2;

	junk2 = (char *)malloc(256);
	if (junk2 == 0L) ex_err("Unable to allocate string buffer memory");

	sscanf(s,"%s %s",junk,junk2);
	return(junk2);
}

void printvar(FILE *fp,var *v,long int ob)
{
	switch(v->type) {
		case VAR_Y_VAL:
			if (ob) {
			fprintf(fp,"Y %s %s %ld\n",v->label,v->name,v->dz);
			} else {
			fprintf(fp,"Y=%s\n",v->label);
			}
			break;
		case VAR_Y_CONST:
			if (ob) {
			fprintf(fp,"Y %s %f\n",v->label,v->value);
			} else {
			fprintf(fp,"Y=%s\n",v->label);
			}
			break;
		case VAR_CONST:
			if (ob) {
			fprintf(fp,"CONST %f\n",v->value);
			} else {
			fprintf(fp,"CONST df=1\n");
			}
			break;
		case VAR_COV_V:
			if (ob) {
			fprintf(fp,"COV %s %s %ld\n",v->label,v->name,v->dz);
			} else {
			fprintf(fp,"COV=%s mean:%f df=1\n",v->label,v->mean);
			}
			break;
		case VAR_COV_C:
			if (ob) {
			fprintf(fp,"COV %s %f\n",v->label,v->value);
			} else {
			fprintf(fp,"COV=%s mean:%f df=1\n",v->label,v->mean);
			}
			break;
		case VAR_CLASS:
			if (ob) {
			fprintf(fp,"CLASS %s %s %ld\n",v->label,v->name,
				(long int)(v->value));
			} else {
			fprintf(fp,"CLASS=%s levels:%ld df=%ld\n",v->label,
				(long int)(v->value),(long int)(v->value)-1);
			}
			break;
		case VAR_LABEL:
			if (ob) {
			fprintf(fp,"LABELCLASS %s %s %ld\n",v->label,v->name,
				(long int)(v->value));
			} else {
			fprintf(fp,"LABELCLASS=%s levels:%ld\n",v->label,
				(long int)(v->value));
			}
			break;
	}
	return;
}

void    printoutput(FILE *fp,output_s *o)
{
	switch(o->type) {
		case OUT_RES_ERR:
			fprintf(fp,"Residual SS=%s\n",o->tempname);
			break;
		case OUT_ADJ_Y_M:
			fprintf(fp,"Mean adjusted Y=%s\n",o->tempname);
			break;
		case OUT_CLASS_M:
			fprintf(fp,"Mean class(%s,%s)=%s\n",o->classname,
				o->classvalue,o->tempname);
			break;
		case OUT_COV_COEF:
			fprintf(fp,"Covariate coefficient (%s)=%s\n",
				o->classname,o->tempname);
			break;
		case OUT_VAR_COV:
			fprintf(fp,"Coefficient covariance (%s,%s)=%s\n",
				o->classname,o->classvalue,o->tempname);
			break;
		case OUT_COV_STDERR:
			fprintf(fp,"Covariate standard error (%s)=%s\n",
				o->classname,o->tempname);
			break;
		case OUT_SS_TOTAL:
			fprintf(fp,"Total SS=%s\n",o->tempname);
			break;
		case OUT_SS_MODEL:
			fprintf(fp,"Model SS=%s\n",o->tempname);
			break;
		case OUT_PRESS:
			fprintf(fp,"PRESS=%s\n",o->tempname);
			break;
		case OUT_R2:
			fprintf(fp,"Rsqu=%s\n",o->tempname);
			break;
		case OUT_R2_ADJ:
			fprintf(fp,"Adjusted Rsqu=%s\n",o->tempname);
			break;
		case OUT_R2_PRED:
			fprintf(fp,"Prediction Rsqu=%s\n",o->tempname);
			break;
		case OUT_VAR_SS:
			fprintf(fp,"Variable Sequential SS (%s)=%s\n",
				o->classname,o->tempname);
			break;
		case OUT_VAR_PSS:
			fprintf(fp,"Variable Partial SS (%s)=%s\n",
				o->classname,o->tempname);
			break;
	}
	return;
}

long int        findvar(char *name)
{
	long int 	i;

	for(i=0;i<modeltemp.nvars;i++) {
		if (strcmp(modeltemp.vars[i].label,name) == 0) return(i);
	}
	return(-1);
}

/* a prototype I hoped would never become visible... */
int tal_shm_is_file(char *file);

void	prefix_filename(char *file,long int output)
{
	char	tstr[256];

/* does it need prefixing at all??? */
	if (strcmp(file,"-") == 0) return;  /* null filename */
	if (file[0] == '=') return;  /* value filename */
	if (file[0] == '/') return;  /* full pathname */
	if (tal_shm_is_file(file)) return;  /* shared memory filename */

	if (output) {
/* output prefix */
		if (volume.output_prefix == 0L) return;
		strcpy(tstr,volume.output_prefix);
	} else {
/* input prefix */
		if (volume.input_prefix == 0L) return;
		strcpy(tstr,volume.input_prefix);
	}

/* complete the transaction */
	strcat(tstr,"/");
	strcat(tstr,file);
	strcpy(file,tstr);

	return;
}
