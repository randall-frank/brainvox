/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski
 *
 * $Id: regress_parse.c 275 2001-09-03 22:13:39Z rjfrank $
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
#include "cvio_regress.h"

/* local prptotypes */
int     get_tok_int(char *s);
double 	get_tok_double(char *s);
char 	*get_tok_string(char *s);
double 	get_class_value(int n,char *token);
void 	ucase(char *str);
void	unknown(char *blk,char *val);
void	prefix_filename(char *file,int output);
int     findvar(char *name);

void ucase(char *str)
{
	int i;
	i = 0;
	while (str[i] != '\0') {
		str[i] = toupper((int)(str[i]));
		i++;
	}
	return;
}

void	unknown(char *blk,char *val)
{
	if (val[0] == '#') return; /* comment */

	fprintf(stderr,"Warning: unknown token %s found in block %s\n",
		val,blk);
	return;
}

int	parse_input(int cvioinput, uint32_t cviofile, FILE *fp,int iCompress, int dontcreate)
{
	CVIO_DT_TIME dummy;
	char	text[256],token[256];
	int	varnum = 0;
	int	state;
	uint32_t wrtflags;
	int32_t	 err,i, count;
	int32_t typ,ndim,dims[3];

/* defaults */
	volume.d[0] = 0;
	volume.d[1] = 0;
	volume.d[2] = 0;
        volume.nslab = 1; 
        volume.curadd_slab = volume.nslab-1;
        volume.currem_slab = volume.nslab-1;
	volume.maskname = NULL;
	volume.maskdata = NULL;
	volume.weightname = NULL;
	volume.weightdata = NULL;
	volume.log_xform = 0;
	volume.input_prefix = NULL;
	volume.output_prefix = NULL;
	volume.datawindow = 0;
        volume.outname = NULL;
	volume.out_time = 0;
	volume.out_type = 0;
        volume.ingate = 0;

	state = 0;
	while(cvioinput?dummy = CVIO_TIME_NEXT, count = 1,
	      CVIO_ERR_OK == cvio_read_samples_blocking(cviofile, &dummy, text, &count, 0, 0):
	      fgets(text,256,fp)!=0L) {
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
				} else if (strcmp(token,"MASK")==0) {
				    volume.maskname = get_tok_string(text);
				    prefix_filename(volume.maskname,0);
				} else if (strcmp(token,"WEIGHT")==0) {
				    volume.weightname = get_tok_string(text);
				    prefix_filename(volume.weightname,0);
				} else if (strcmp(token,"LOG_TRANSFORM")==0) {
				    volume.log_xform = 1;
				} else if (strcmp(token,"INPUT_PREFIX")==0) {
				    volume.input_prefix = get_tok_string(text);
                                    sub_name(volume.input_prefix);
				} else if (strcmp(token,"OUTPUT_PREFIX")==0) {
				    volume.output_prefix = get_tok_string(text);
                                    sub_name(volume.output_prefix);
				} else if (strcmp(token,"NUM_SLABS")==0) {
                                    char t0[100],t1[100];
                                    sscanf(text,"%s %s",t0,t1);
                                    sub_name(t1);
				    volume.nslab = get_tok_int(t1);
                                    if (volume.nslab < 1) volume.nslab = 1;
                                    volume.curadd_slab = volume.nslab-1;
                                    volume.currem_slab = volume.nslab-1;
				} else if (strcmp(token,"WINDOW")==0) {
                                    double sec;
                                    char t0[100],t1[100];
                                    sscanf(text,"%s %s",t0,t1);
                                    sub_name(t1);
				    sec = get_tok_double(t1);
				    volume.datawindow=sec*CVIO_TIME_TICKS_PER_SEC;
				} else {
				    unknown("VOLPARAMS",token);
				}
				break;
			case 2: /* read a MODEL block */
/* too many VARs */
				if (modeltemp.nvars >= MAX_VARS-2) ex_err(
				    "Maximum number of variables exceeded.");
				if ((modeltemp.nvars == 1) &&
				    (modeltemp.vars[0].type != VAR_Y_VAL)) {
			ex_err("The first variable in a MODEL must be Y");
				}

				modeltemp.vars[modeltemp.nvars].data = NULL;
				if (strcmp(token,"OUTPUT")==0) {
					state=3;
					noutputs = 0;
					if (modeltemp.nvars < 2) {  /* Y,X */
				ex_err("Not enough variables in the model.");
					}
				} else if (strcmp(token,"Y") == 0) {
/* Y name (must be first!) */
			if (modeltemp.nvars != 0) {
	ex_err("The Y value in a MODEL must be the first variable.");
			}
			modeltemp.vars[modeltemp.nvars].type = VAR_Y_VAL;
			sscanf(text,"%s %s",token,
				modeltemp.vars[modeltemp.nvars].label);
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
/* COVARIABLE_VOL name */
			modeltemp.vars[modeltemp.nvars].type = VAR_COV_V;
			sscanf(text,"%s %s",token,
				modeltemp.vars[modeltemp.nvars].label);
			modeltemp.nvars++;

				} else if (strcmp(token,"CLASS") == 0) {
/* CLASS name nvalues */
			float f = 0;
			modeltemp.vars[modeltemp.nvars].type = VAR_CLASS;
			sscanf(text,"%s %s %f",token,
				modeltemp.vars[modeltemp.nvars].label,&f);
			modeltemp.vars[modeltemp.nvars].value = f;
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
					if (noutputs < 1) {
					ex_err("No output data requested.");
					}
				} else if (strcmp(token,"TIMESTAMPS")==0) {
                                        volume.outname = get_tok_string(text);
				        prefix_filename(volume.outname,1);
				} else if (strcmp(token,"N")==0) {
/* N template */
					outputs[noutputs].type = OUT_N;
					sscanf(text,"%s %s",token,
						outputs[noutputs].tempname);
				prefix_filename(outputs[noutputs].tempname,1);
					noutputs++;
				} else if (strcmp(token,"SS_ERROR")==0) {
/* SS_ERROR template */
					outputs[noutputs].type = OUT_RES_ERR;
					sscanf(text,"%s %s",token,
						outputs[noutputs].tempname);
				prefix_filename(outputs[noutputs].tempname,1);
					noutputs++;
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
	if ((outputs[noutputs].var < 0) || (outputs[noutputs].var2 < 0)) {
		ex_err("Cov coef output requested for non-existant variable");
	}
	if ((modeltemp.vars[outputs[noutputs].var].type == VAR_CLASS) ||
	    (modeltemp.vars[outputs[noutputs].var2].type == VAR_CLASS)) {
		ex_err("Coef output can not be requested for a class variable");
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
				} else {
					unknown("OUTPUT",token);
				}
				break;
			case 4: /* read an OBS block */

        if (!volume.outname) {
		ex_err("The OUTPUT TIMESTAMPS cviofile must be specified");
	}
				if (strcmp(token,"END")==0) {
					state=5;
					break;
				}
/* re-read the token (IN LOWERCASE) */
				sscanf(text,"%s",token);
/* read the data for variable VARNUM into obs[nobs].vars[varnum] */
				switch(modeltemp.vars[varnum].type) {

					case VAR_CONST:
				modeltemp.vars[varnum].value = atof(token);
						break;

					case VAR_Y_VAL:
				strcpy(modeltemp.vars[varnum].name,token);
				prefix_filename(modeltemp.vars[varnum].name,0);
						break;

					case VAR_COV_V:
				strcpy(modeltemp.vars[varnum].name,token);
				prefix_filename(modeltemp.vars[varnum].name,0);
						break;

					case VAR_CLASS:
				strcpy(modeltemp.vars[varnum].name,token);
				prefix_filename(modeltemp.vars[varnum].name,0);
						break;

				}
				varnum += 1;
/* if we have read all the variables, we have our obervation... */
				if (varnum >= modeltemp.nvars) {
					varnum = 0;
                                        state=5;
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

	wrtflags = 0;
	if (iCompress) wrtflags |= CVIO_CREATE_GZIP;

	/* open up all the input files */
	volume.dsize = 0;
	volume.nsamples = 0;
	for(i=0;i<modeltemp.nvars;i++) {
	    int32_t j,n;
	    uint32_t len;
	    switch(modeltemp.vars[i].type) {
	        case VAR_Y_VAL:
		case VAR_COV_V:
		case VAR_CLASS:
		    err = cvio_open(modeltemp.vars[i].name,
                               CVIO_ACCESS_READ,&(modeltemp.vars[i].Fid));
		    if (err) exit_cvio_error(err);
		    err = cvio_datatype(modeltemp.vars[i].Fid,&typ,&ndim,dims);
		    if (err) exit_cvio_error(err);
		    err = cvio_max_length(modeltemp.vars[i].Fid,&len);
		    if (err) exit_cvio_error(err);
		    /* pick the biggest size and length */
		    if (len > volume.nsamples) volume.nsamples = len;
		    n = 1;
		    for(j=0;j<ndim;j++) n *= dims[j];
		    if (n > volume.dsize) {
                        volume.dsize = n;
                        volume.d[0] = dims[0];
                        volume.d[1] = dims[1];
                        volume.d[2] = dims[2];
		        volume.ndim = ndim;
		    }
		    break;
	    }
	}
	for(i=0;i<modeltemp.nvars;i++) {
            int32_t n,j;
	    switch(modeltemp.vars[i].type) {
	        case VAR_Y_VAL:
		case VAR_COV_V:
		case VAR_CLASS:
		    err = cvio_datatype(modeltemp.vars[i].Fid,&typ,&ndim,dims);
		    n = 1;
		    for(j=0;j<ndim;j++) n *= dims[j];
		    /* scalar or same size ok */
                    if ((n != 1) && (n != volume.dsize)) {
			ex_err("Input volume size does not match largest.");
                    }
                    modeltemp.vars[i].data = (float *)calloc(volume.dsize,
                                                        sizeof(float)*2);
	            if (!modeltemp.vars[i].data) {
			ex_err("Unable to allocate buffer memory.");
		    }
		    break;
	    }
        }

	volume.dsize = 1;
	for(i=0;i<volume.ndim;i++) volume.dsize *= volume.d[i];

	/* some optional files */
	/* by default, the mask is fully set */
	volume.maskdata = (unsigned char *)malloc(volume.dsize*volume.nslab);
	if (!volume.maskdata) ex_err("Unable to allocate mask memory.");
	memset(volume.maskdata,1,volume.dsize*volume.nslab);
	/* the mask volume, one sample */
	if (volume.maskname) {
		CVIO_DT_TIME ts;
		uint32_t Ft;
		err = cvio_open(volume.maskname,CVIO_ACCESS_READ,&Ft);
		if (err) exit_cvio_error(err);
                for(i=0;i<volume.nslab;i++) {
		    ts = CVIO_TIME_NEXT;
		    reg_read_sample(Ft,volume.maskdata+(i*volume.dsize),
                                    CVIO_TYP_BYTE,&ts,1);
                }
		cvio_close(Ft);
	}
	/* a weighting volume */
	if (volume.weightname) {
		volume.weightdata=(float *)calloc(volume.dsize*volume.nslab,
                                                  sizeof(float));
		if (!volume.weightdata) {
			ex_err("Unable to allocate weight memory.");
		}
		err = cvio_open(volume.weightname,CVIO_ACCESS_READ,
				&volume.Fweight);
		if (err) exit_cvio_error(err);
	}

	/* open up all the output files */
	/* the timing file */
	err = cvio_open(volume.outname,CVIO_ACCESS_READ,&volume.Fout);
	if (err) exit_cvio_error(err);
	err = cvio_datatype(volume.Fout,&volume.out_type,&ndim,dims);
	if (err) exit_cvio_error(err);
        if ((ndim != 1) && (dims[0] != 1)) {
		ex_err("The TIMESTAMPS volume must be a scalar.");
        }
	/* the output files */
	for(i=0;i<noutputs;i++) {
		if (!dontcreate) {
			err = cvio_create(outputs[i].tempname,volume.nsamples,
					CVIO_TYP_FLOAT,volume.ndim,volume.d,wrtflags);
		        if (err) exit_cvio_error(err);
		}
		err = cvio_open(outputs[i].tempname,CVIO_ACCESS_APPEND,
				&(outputs[i].Fid));
		if (err) exit_cvio_error(err);
	        outputs[i].data = (float *)calloc(volume.dsize,sizeof(float)); 
		if (!outputs[i].data) ex_err("Unable to allocate output memory.");
        }

	return(0);
}

void freebuffers()
{
	int	i;

	for(i=0;i<modeltemp.nvars;i++) {
	    switch(modeltemp.vars[i].type) {
	        case VAR_Y_VAL:
		case VAR_COV_V:
		case VAR_CLASS:
		    cvio_close(modeltemp.vars[i].Fid);
		    free(modeltemp.vars[i].data);
		    break;
	    }
        }
	cvio_close(volume.Fout);
	if (volume.weightname) {
		cvio_close(volume.Fweight);
		free(volume.weightdata);
	}
	free(volume.maskdata);
	for(i=0;i<noutputs;i++) {
		cvio_close(outputs[i].Fid);
		free(outputs[i].data);
	}

/* free the prefix buffers */
	if (volume.input_prefix != 0) free(volume.input_prefix);
	if (volume.output_prefix != 0) free(volume.output_prefix);

	return;
}

int get_tok_int(char *s)
{
	char		junk[256];
	int	i;

	sscanf(s,"%d",&i);
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

	junk2 = (char *)calloc(256,1);
	if (junk2 == 0L) ex_err("Unable to allocate string buffer memory");

	sscanf(s,"%s %s",junk,junk2);
	return(junk2);
}

void printvar(FILE *fp,var *v,int ob)
{
	switch(v->type) {
		case VAR_Y_VAL:
			if (ob) {
			fprintf(fp,"Y %s %s\n",v->label,v->name);
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
			fprintf(fp,"COV %s %s\n",v->label,v->name);
			} else {
			fprintf(fp,"COV=%s df=1\n",v->label);
			}
			break;
		case VAR_COV_C:
			if (ob) {
			fprintf(fp,"COV %s %f\n",v->label,v->value);
			} else {
			fprintf(fp,"COV=%s df=1\n",v->label);
			}
			break;
		case VAR_CLASS:
			if (ob) {
			fprintf(fp,"CLASS %s %s %d\n",v->label,v->name,
				(int)(v->value));
			} else {
			fprintf(fp,"CLASS=%s levels:%d df=%d\n",v->label,
				(int)(v->value),(int)(v->value)-1);
			}
			break;
		case VAR_LABEL:
			if (ob) {
			fprintf(fp,"LABELCLASS %s %s %d\n",v->label,v->name,
				(int)(v->value));
			} else {
			fprintf(fp,"LABELCLASS=%s levels:%d\n",v->label,
				(int)(v->value));
			}
			break;
	}
	return;
}

void    printoutput(FILE *fp,output_s *o)
{
	switch(o->type) {
		case OUT_N:
			fprintf(fp,"N=%s\n",o->tempname);
			break;
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

int        findvar(char *name)
{
	int 	i;

	for(i=0;i<modeltemp.nvars;i++) {
		if (strcmp(modeltemp.vars[i].label,name) == 0) return(i);
	}
	return(-1);
}

void	prefix_filename(char *file,int output)
{
	char	tstr[256];

/* perform command line substitution first */
        sub_name(file);

/* does it need prefixing at all??? */
	if (strcmp(file,"-") == 0) return;  /* null filename */
	if (file[0] == '=') return;  /* value filename */
	if (file[0] == '/') return;  /* full pathname */

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

static void *tmp_io_buf = NULL;
int32_t reg_read_sample(uint32_t stream,void *ptr,int32_t type, CVIO_DT_TIME *t,
	int32_t wait)
{
	int32_t err,i,n;
	int32_t ndim,dims[3],typ;
	int32_t	count;
	int32_t size;

	if (!tmp_io_buf) tmp_io_buf = (void *)calloc(volume.dsize,sizeof(float));

	err = cvio_datatype(stream,&typ,&ndim,dims);
	if (err) return(err);

	size = (typ & CVIO_TYP_SIZE_MASK)/8;

	n = 1;
	for(i=0;i<ndim;i++) n *= dims[i];

	if ((n != volume.dsize) && (n != 1)) return(CVIO_ERR_BADDIMENSIONS);

	count = 1;
	err = cvio_read_next_samples_blocking(stream,t,tmp_io_buf,&count,0,0);
	if (err) return(err);

	if (type != typ) {
		unsigned char *ucs,*ucd;
		unsigned short *us;
		short  *s;
		float  *f;
		switch(type) {
		  case CVIO_TYP_BOOL:
		  case CVIO_TYP_STRING:
		  case CVIO_TYP_BYTE:
		    ucd = (unsigned char *)ptr;
		    switch(typ) {
		      case CVIO_TYP_BOOL:
		      case CVIO_TYP_STRING:
	    	      case CVIO_TYP_BYTE:
			ucs = (unsigned char *)tmp_io_buf;
			for(i=0;i<n;i++) ucd[i] = ucs[i];
		        break;
		      case CVIO_TYP_SHORT:
			s = (short *)tmp_io_buf;
			for(i=0;i<n;i++) ucd[i] = (unsigned char)(s[i]);
		        break;
		      case CVIO_TYP_USHORT:
			us = (unsigned short *)tmp_io_buf;
			for(i=0;i<n;i++) ucd[i] = (unsigned char)(us[i]);
		        break;
		      case CVIO_TYP_FLOAT:
			f = (float *)tmp_io_buf;
			for(i=0;i<n;i++) ucd[i] = (unsigned char)(f[i]);
		        break;
		    }
		    break;
		  case CVIO_TYP_SHORT:
		    s = (short *)ptr;
		    switch(typ) {
		      case CVIO_TYP_BOOL:
		      case CVIO_TYP_STRING:
		      case CVIO_TYP_BYTE:
			ucs = (unsigned char *)tmp_io_buf;
			for(i=0;i<n;i++) s[i] = (short)(ucs[i]);
		        break;
		      case CVIO_TYP_USHORT:
			us = (unsigned short *)tmp_io_buf;
			for(i=0;i<n;i++) s[i] = (short)(us[i]);
		        break;
		      case CVIO_TYP_FLOAT:
			f = (float *)tmp_io_buf;
			for(i=0;i<n;i++) s[i] = (short)(f[i]);
		        break;
		    }
		    break;
		  case CVIO_TYP_USHORT:
		    us = (unsigned short *)ptr;
		    switch(typ) {
		      case CVIO_TYP_BOOL:
		      case CVIO_TYP_STRING:
		      case CVIO_TYP_BYTE:
			ucs = (unsigned char *)tmp_io_buf;
			for(i=0;i<n;i++) us[i] = (unsigned short)(ucs[i]);
		        break;
		      case CVIO_TYP_SHORT:
			s = (short *)tmp_io_buf;
			for(i=0;i<n;i++) us[i] = (unsigned short)(s[i]);
		        break;
		      case CVIO_TYP_FLOAT:
			f = (float *)tmp_io_buf;
			for(i=0;i<n;i++) us[i] = (unsigned short)(f[i]);
		        break;
		    }
		    break;
		  case CVIO_TYP_FLOAT:
		    f = (float *)ptr;
		    switch(typ) {
		      case CVIO_TYP_BOOL:
		      case CVIO_TYP_STRING:
		      case CVIO_TYP_BYTE:
			ucs = (unsigned char *)tmp_io_buf;
			for(i=0;i<n;i++) f[i] = (float)(ucs[i]);
		        break;
		      case CVIO_TYP_SHORT:
			s = (short *)tmp_io_buf;
			for(i=0;i<n;i++) f[i] = (float)(s[i]);
		        break;
		      case CVIO_TYP_USHORT:
			us = (unsigned short *)tmp_io_buf;
			for(i=0;i<n;i++) f[i] = (float)(us[i]);
		        break;
		    }
		    break;
		}
	} else {
		memcpy(ptr,tmp_io_buf,size*n);
	}
	
	/* scalar to vector conversion */
	if (n == 1) {
		unsigned char *dst = (unsigned char *)ptr;
		dst += size;
		for(i=1;i<volume.dsize;i++) {
			memcpy(dst,ptr,size);
			dst += size;
		}
	}

	return(err);
}


static int nsub_names = 0;
name_sub *sub_names = {0};

void add_sub_name(char *in,char *out)
{
   if (nsub_names) {
      sub_names = (name_sub*)realloc(sub_names,sizeof(name_sub)*(nsub_names+1));
   } else {
      sub_names = (name_sub*)malloc(sizeof(name_sub));
   }
   if (sub_names) {
      strcpy(sub_names[nsub_names].in,in);
      strcpy(sub_names[nsub_names].out,out);
      nsub_names += 1;
   }
   return;
}

void sub_name(char *replace)
{
   int i;
   for(i=0;i<nsub_names;i++) {
      if (strcmp(replace,sub_names[i].in) == 0) {
         if (iVerbose) {
            fprintf(stderr,"Replacing %s with %s\n",
                           sub_names[i].in,sub_names[i].out);
         }
         strcpy(replace,sub_names[i].out);
         return;
      }
   }
   return;
}

