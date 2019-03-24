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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include "cvio_regress.h"

/* Possible additions/issues:
 * 1) handling of EOF w/writers
 * 2) defer creation of output files to external processes
 * 3) allow a stream (other than the first "Y") to gate input samples
 */

/* globals */
int             noutputs = 0;
output_s        outputs[MAX_OUTPUTS];
model           modeltemp;
volparams       volume;
int		iVerbose = 0;

void cmd_err(char *app)
{
        fprintf(stderr,"(%s) Usage: %s [options] datafile\n",__DATE__,app);
        fprintf(stderr,"Options:\n");
        fprintf(stderr,"        -v verbose\n");
        fprintf(stderr,"        -r in out  Replace IN file with PUT file when parsing datafile. Can repeat.\n");
	fprintf(stderr,"        -d don't create output files (they already exist)\n");
	fprintf(stderr,"	-C model file is stored in cvio format\n");
        fprintf(stderr,"\nDatafile definition language:\n");
        fprintf(stderr,"\tVOLPARAMS\n");
        fprintf(stderr,"\t\tMASK cviofile (8bit mask volume template default=full volume)\n");
/* not currently supported 
        fprintf(stderr,"\t\tWEIGHT cviofile (float weight volume template default= =1.0)\n");
*/
        fprintf(stderr,"\t\tLOG_TRANSFORM (Apply a log10 transform to all input data)\n");
        fprintf(stderr,"\t\tINPUT_PREFIX dirname (prepended to input files default=none)\n");
        fprintf(stderr,"\t\tOUTPUT_PREFIX dirname (prepended to output files default=none)\n");
        fprintf(stderr,"\t\tWINDOW seconds (window width in seconds default=none)\n");
        fprintf(stderr,"\t\tNUM_SLABS count (# of samples for one observation default=1)\n");
        fprintf(stderr,"\tMODEL\n");
        fprintf(stderr,"\t\tY name\n");
        fprintf(stderr,"\t\tCONST\n");
        fprintf(stderr,"\t\tCOVARIABLE_VOL name\n");
        fprintf(stderr,"\t\tCLASS classname nclasslevels\n");
        fprintf(stderr,"\tOUTPUT\n");
        fprintf(stderr,"\t\tTIMESTAMPS cviofile\n");
        fprintf(stderr,"\t\tN cviofile (number of observations)\n");
        fprintf(stderr,"\t\tSS_ERROR cviofile (SS error remaining)\n");
        fprintf(stderr,"\t\tSS_VARIABLE varname cviofile\n");
        fprintf(stderr,"\t\tP_SS_VARIABLE varname cviofile\n");
        fprintf(stderr,"\t\tSS_TOTAL cviofile\n");
        fprintf(stderr,"\t\tSS_MODEL cviofile\n");
        fprintf(stderr,"\t\tR2_MODEL cviofile\n");
        fprintf(stderr,"\t\tR2_ADJ_MODEL cviofile\n");
        fprintf(stderr,"\t\tCOVARIABLE_SLOPE covname cviofile\n");
        fprintf(stderr,"\t\tCOVARIABLE_STD_ERROR covname cviofile\n");
        fprintf(stderr,"\t\tCOEF_COVARIANCE varname1 varname2 cviofile\n");
        fprintf(stderr,"\tOBSERVATIONS\n");
        fprintf(stderr,"\t\tFor Y: cviofile\n");
        fprintf(stderr,"\t\tFor CONST: value (generally it should be 1.0)\n");
        fprintf(stderr,"\t\tFor COVARIABLE_VOL: cviofile\n");
        fprintf(stderr,"\t\tFor CLASS: cviofile\n");
        fprintf(stderr,"\tEND\n");
        fprintf(stderr,"\n");
        fprintf(stderr,"TIMESTAMPS is a stream of scalars formed by ORing the following:\n");
        fprintf(stderr,"SS_ERROR               1\n");
        fprintf(stderr,"SS_VARIABLE            2\n");
        fprintf(stderr,"P_SS_VARIABLE          4\n");
        fprintf(stderr,"SS_TOTAL               8\n");
        fprintf(stderr,"SS_MODEL               16\n");
        fprintf(stderr,"R2_MODEL               32\n");
        fprintf(stderr,"R2_ADJ_MODEL           64\n");
        fprintf(stderr,"COVARIABLE_SLOPE       128\n");
        fprintf(stderr,"COVARIABLE_STD_ERROR   256\n");
        fprintf(stderr,"COEF_COVARIANCE        512\n");
        fprintf(stderr,"N                     1024\n");
        fprintf(stderr,"A different collection of outputs may be selected for each output timestep.\n");
        exit(1);
}

void    ex_err(char *s)
{
	fprintf(stderr,"Fatal error:%s\n",s);
	cvio_cleanup(0);
	exit(1);
}
void	exit_cvio_error2(char *s,int32_t err) 
{
	fprintf(stderr,"%s",s);
        exit_cvio_error(err);
}
void	exit_cvio_error(int32_t err) 
{
	char	tstr[256];
	int32_t	tlen = 256;

	cvio_get_error_message(err,tstr,&tlen);
	fprintf(stderr,"Fatal error:%s\n",tstr);

	cvio_cleanup(0);
	exit(1);
}

int main(int argc, char *argv[])
{
  	FILE		*fp;
	int32_t		i,j;
	int32_t		err;
	uint32_t	cviofile;

	int32_t		cviomodel = 0;
	int32_t		dontcreate = 0;
        int32_t 	iCompress = 0;
        int32_t 	iStdin = 0;
	int 		inputdone;

        i = 1;
        while ((i < argc) && (argv[i][0] == '-')) {
                if (argv[i][1] == '\0') break;
                switch (argv[i][1]) {
			case 'C':
				cviomodel = 1;
				break;
                        case 'v':
                                iVerbose = strlen(argv[i])-1;
                                break;
                        case 'c':
                                iCompress = CVIO_CREATE_GZIP;
                                break;
			case 'd':
				dontcreate = 1;
				break;
			case 'r':
				if (i+2 < argc) {
					add_sub_name(argv[i+1],argv[i+2]);
                  			i += 2;
                                } else {
         				cmd_err(argv[0]);
                                }
				break;
                        case '-':
                                iStdin = 1;
                                break;
                        default:
                                cmd_err(argv[0]);
                                break;
                }
                i++;
        }
 	if (cvio_init()) exit(1);
        if (iStdin) {
        	fp = stdin;
 	} else {
        	if ((argc-i) != 1) cmd_err(argv[0]);
		if (cviomodel) {
			fp = NULL;
			err = cvio_open(argv[i], CVIO_ACCESS_READ, &cviofile);
			if (err != CVIO_ERR_OK)
				ex_err("Unable to open data definition file");
		} else {
	        	fp = fopen(argv[i],"r");
	        	if (fp == 0L) ex_err("Unable to open data definition file");
		}
	}
        if (parse_input(cviomodel, cviofile, fp,iCompress, dontcreate) != 0) {
		ex_err("Error in data definition file format");
 	}
        if (!iStdin) {
		if (cviomodel)
			cvio_close(cviofile);
		else
	 		fclose(fp);
	}

        if (iVerbose) {
                fprintf(stdout,"\nVolumeparams:\n");
                fprintf(stdout,"Matrix:%d %d %d\n",volume.d[0],volume.d[1],volume.d[2]);
                if (volume.maskname) {
                        fprintf(stdout,"Mask stream:%s\n",volume.maskname);
                } else {
                        fprintf(stdout,"No mask\n");
                }
                if (volume.weightname) {
                        fprintf(stdout,"Weight stream:%s\n",volume.weightname);
                }
                if (volume.nslab > 1) {
                        fprintf(stdout,"Number of slabs:%d\n",volume.nslab);
                }
                if (volume.datawindow) {
                        fprintf(stdout,"Data window: %f sec\n",
                ((float)volume.datawindow)/((float)CVIO_TIME_TICKS_PER_SEC));
                }
                fprintf(stdout,"\nVariables: %d\n",modeltemp.nvars-1);
                for(i=0;i<modeltemp.nvars;i++) {
                        printvar(stdout,&(modeltemp.vars[i]),0);
                }
                j = 0;
                for(i=0;i<modeltemp.nvars;i++) {
                        switch(modeltemp.vars[i].type) {
                                case VAR_Y_VAL:
                                        break;
                                case VAR_CONST:
                                case VAR_COV_V:
                                        j = j + 1;
                                        break;
                                case VAR_CLASS:
                                        j = j + (modeltemp.vars[i].value-1);
                                        break;
                        }
                }
                fprintf(stdout,"\nOutputs: %d\n",noutputs);
                fprintf(stdout,"Output gate stream: %s\n",volume.outname);
                for(i=0;i<noutputs;i++) {
                        printoutput(stdout,&(outputs[i]));
                }
                fprintf(stdout,"\nObservations:\n");
                for(i=0;i<modeltemp.nvars;i++) {
                        switch(modeltemp.vars[i].type) {
                                case VAR_Y_VAL:
                fprintf(stdout,"Y stream: %s\n",modeltemp.vars[i].name);
                                        break;
                                case VAR_COV_V:
                fprintf(stdout,"Covariable stream: %s\n",modeltemp.vars[i].name);
                                        break;
                                case VAR_CLASS:
                fprintf(stdout,"Covariable class stream: %s\n",modeltemp.vars[i].name);
                                        break;
                        }
                 }
                fprintf(stdout,"\n");
        }

	/* set up the regressions */
	setup_regression();

        /* read some initial data */
        prime_input_streams();

        /* the entire process is gated by the Fout stream timing */
   	inputdone = 0;
	while(!inputdone) {
            int32_t num = 1;
	    uint32_t value;
	    err = cvio_read_next_samples_blocking(volume.Fout,
			&volume.out_time,&value,&num,0,0);
	    if (err == CVIO_ERR_EOF) {
		inputdone = 1;
		err = CVIO_ERR_OK;
                continue;
	    } else if (err) {
		exit_cvio_error(err);
	    }
            if (iVerbose) fprintf(stderr,"Working on ts: %d\n",volume.out_time);
	    if (num == 1) {
                uint32_t t = 0;

		/* Process the inputs up to volume.out_time */
		update_input_streams();

                /* remove any that have fallen outside of the window */
                prune_observations(volume.out_time);

		/* compute any requested outputs */
		switch(volume.out_type) {
		    case CVIO_TYP_BYTE:
		    case CVIO_TYP_BOOL:
		    case CVIO_TYP_STRING:
                        { unsigned char *uc = (unsigned char *)(&value);
                          t = uc[0]; }
			break;
		    case CVIO_TYP_SHORT:
                        { short *s = (short *)(&value);
			  t = s[0]; }
			break;
		    case CVIO_TYP_USHORT:
                        { unsigned short *us = (unsigned short *)(&value);
			  t = us[0]; }
			break;
		    case CVIO_TYP_FLOAT:
                        { float *f = (float *)(&value);
			  t = f[0]; }
			break;
                }
		/* various possible outputs */
		output_streams(t);
	    }
        }

	/* cleanup and exit */
	cleanup_observations();
        cleanup_regression();
	freebuffers();
	cvio_cleanup(0);

	exit(0);
}

static regression reg = {0};
static observation *obs_free_table = NULL;
static observation *obs_active_table = NULL;

/* get a new, active observation */
observation *new_observation(CVIO_DT_TIME t)
{
    observation *o;

    if (iVerbose > 1) fprintf(stderr, "New obs at %d\n",t);

    /* use a freed one, or allocate a new one */
    o = obs_free_table;
    if (o) {
        obs_free_table = o->next;
        o->next = NULL;
    } else {
        int i;
        o = (observation *)malloc(sizeof(observation));
        if (!o) ex_err("Unable to allocate observation memory");
        o->alloc = (float *)malloc(volume.dsize*volume.nslab*
                                   modeltemp.nvars*sizeof(float));
        if (!o->alloc) ex_err("Unable to allocate observation memory");
        /* "cook" up pointers to individual variables (samplesize*num slabs) */
        for(i=0;i<modeltemp.nvars;i++) {
           o->vars[i] = o->alloc + volume.dsize*volume.nslab*i;
        }
    }
    o->time = t;

    /* add this to the active table */
    o->next = obs_active_table;
    obs_active_table = o;

    return(o);
}

/* torch all the observations */
void cleanup_observations(void)
{
    observation *o;

    o = obs_free_table;
    while(o) {
       observation *z = o->next;
       free(o->alloc);
       free(o);
       o = z;
    }

    o = obs_active_table;
    while(o) {
       observation *z = o->next;
       free(o->alloc);
       free(o);
       o = z;
    }

    obs_active_table = NULL;
    obs_free_table = NULL;

    return;
}

/* since they are added to the beginning, we must remove from
 * the end to make sure the slabbing ordeer is corrent */
void prune_observations(CVIO_DT_TIME t)
{
    while(1) {
       observation *o;
       /* any left? */
       o = obs_active_table;
       if (!o) return;
       /* find the last observation */
       while(o->next) o = o->next;
       /* consider removing it */
       if (volume.datawindow == 0) {
          /* with inf window, we never remove any obs */
          /* but we can free the obs */
          free_observation(o);
       } else if (o->time+volume.datawindow < t) {
          addrem_observation(o,1);
          free_observation(o);
       } else {
          return;
       }
    }
}

/* move the current observation from the active table to the
 * free table */
void free_observation(observation *o)
{
    observation *a,*b;

    /* find it in the active table */
    a = obs_active_table;
    b = NULL;
    while(a != o) {
        b = a;
        a = a->next;
    }
    if (a != o) ex_err("Internal error!!! free non-allocated obs");
    if (b) {
        b->next = a->next;
    } else {
        obs_active_table = a->next;
    }

    /* add it to the free table */
    o->next = obs_free_table;
    obs_free_table = o;

    return;
}

/* compute outputs and send them out */
void output_streams(int value)
{
    int i,j,k,l,slab;
    int32_t err;

    if (iVerbose>1) fprintf(stderr,"Output streams: %d\n",value);

    if (!value) return;

    slab = volume.curadd_slab*volume.dsize;
    for(i=0;i<volume.dsize;i++) {
       int fail;
       double sst, y;

       if (!volume.maskdata[i+slab] || (reg.nobs[i+slab] <= reg.np)) {
          for(j=0;j<noutputs;j++)
            outputs[j].data[i] = 0;
          continue;
       }

       /* basic stuff */
       /* set singular tolerances */
       tolset_(&reg.np,&reg.nrbar,reg.dd[i+slab],reg.r[i+slab],reg.tol,
               reg.work,&fail);
       /* singularity check */
       sing_(&reg.np,&reg.nrbar,reg.dd[i+slab],reg.r[i+slab],reg.theta[i+slab],
             &(reg.sser[i+slab]),reg.tol,reg.lindep,reg.work,&fail);
       /* get reg info */
       ss_(&reg.np,reg.dd[i+slab],reg.theta[i+slab],&(reg.sser[i+slab]),
           reg.ss,&fail);
       /* sst */
       y = reg.sumy[i+slab]/(double)(reg.nobs[i+slab]);
       sst = reg.ssumy[i+slab] -2.0*y*reg.sumy[i+slab] + 
             y*y*(double)(reg.nobs[i+slab]);
       /* coef */
       regcf_(&reg.np,&reg.nrbar,reg.dd[i+slab],reg.r[i+slab],reg.theta[i+slab],
              reg.tol, reg.work, &reg.np,&fail);
       /* current order */
       for(j=0;j<reg.np;j++) reg.vorder[j] = j;

       for(j=0;j<noutputs;j++) {
          if ((outputs[j].type == OUT_N) && 
                     (value & CR_OUT_N)) {
             outputs[j].data[i] = reg.nobs[i+slab];
          } else if ((outputs[j].type == OUT_RES_ERR) && 
                     (value & CR_OUT_SS_ERROR)) {
             outputs[j].data[i] = reg.sser[i+slab];
          } else if ((outputs[j].type == OUT_COV_COEF) && 
                     (value & CR_OUT_COVARIABLE_SLOPE)) {
             outputs[j].data[i] = reg.work[reg.XofV[outputs[j].var]];
          } else if ((outputs[j].type == OUT_SS_TOTAL) && 
                     (value & CR_OUT_SS_TOTAL)) {
             outputs[j].data[i] = sst;
          } else if ((outputs[j].type == OUT_VAR_SS) && 
                     (value & CR_OUT_SS_VARIABLE)) {
             k = reg.XofV[outputs[j].var];
             if (modeltemp.vars[outputs[j].var].type == VAR_CLASS) {
                int num = modeltemp.vars[outputs[j].var].value-1;
                k += (num-1);
             }
             l = reg.XofV[outputs[j].var]-1;
             if (l < 0) {
                outputs[j].data[i] = sst-reg.sser[i+slab]-reg.ss[k];
             } else {
                outputs[j].data[i] = reg.ss[l] - reg.ss[k];
             }
          } else if ((outputs[j].type == OUT_SS_MODEL) && 
                     (value & CR_OUT_SS_MODEL)) {
             outputs[j].data[i] = sst - reg.sser[i+slab];
          } else if ((outputs[j].type == OUT_VAR_PSS) && 
                     (value & CR_OUT_P_SS_VARIABLE)) {
             k = reg.XofV[outputs[j].var];
             if (modeltemp.vars[outputs[j].var].type == VAR_CLASS) {
                int num = modeltemp.vars[outputs[j].var].value-1;
                k += (num-1);
             }
             l = reg.XofV[outputs[j].var]-1;
             outputs[j].data[i] = calc_pss(reg.np,reg.nrbar,reg.vorder,
                    reg.dd[i+slab], reg.r[i+slab], reg.theta[i+slab],reg.ss, 
                    reg.tol, reg.norder, l, k);
          } else if ((outputs[j].type == OUT_R2) && 
                     (value & CR_OUT_R2_MODEL)) {
             if (sst == 0.0) {
                outputs[j].data[i] = 1.0;
             } else {
                outputs[j].data[i] = 1.0 - (reg.sser[i+slab]/sst);
             }
          } else if ((outputs[j].type == OUT_R2_ADJ) && 
                     (value & CR_OUT_R2_ADJ_MODEL)) {
             if (sst == 0.0) {
                outputs[j].data[i] = 1.0;
             } else {
                outputs[j].data[i] = 1.0 - 
         ((reg.sser[i+slab]/(reg.nobs[i+slab]-reg.np))/(sst/(reg.nobs[i+slab]-1)));
             }
          } else if ((outputs[j].type == OUT_COV_STDERR) && 
                     (value & CR_OUT_COVARIABLE_STD_ERROR)) {
             double w,hii;
             for(k=0;k<reg.np;k++) reg.xtmp[k] = 0.0;
             reg.xtmp[reg.XofV[outputs[j].var]] = 1.0;
             hdiag_(reg.xtmp,&reg.np,&reg.nrbar,reg.dd[i+slab],reg.r[i+slab],
                    reg.tol,&reg.np,&hii,reg.work2,&fail);
             w = reg.sser[i+slab]/(double)(reg.nobs[i+slab]-reg.np);
             outputs[j].data[i] = sqrt(w)*sqrt(hii);
          } else if ((outputs[j].type == OUT_VAR_COV) && 
                     (value & CR_OUT_COEF_COVARIANCE)) {
             outputs[j].data[i] = calc_var_cov(reg.np,reg.nrbar,reg.dd[i+slab],
                                   reg.r[i+slab],outputs[j].var,outputs[j].var2);
          }
       }
    }
    /* write to disk ? */
    for(j=0;j<noutputs;j++) {
       int write = 0;
       if ((outputs[j].type == OUT_N) && 
                  (value & CR_OUT_N)) {
          write = 1;
       } else if ((outputs[j].type == OUT_RES_ERR) && 
                  (value & CR_OUT_SS_ERROR)) {
          write = 1;
       } else if ((outputs[j].type == OUT_COV_COEF) && 
                  (value & CR_OUT_COVARIABLE_SLOPE)) {
          write = 1;
       } else if ((outputs[j].type == OUT_SS_TOTAL) && 
                  (value & CR_OUT_SS_TOTAL)) {
          write = 1;
       } else if ((outputs[j].type == OUT_VAR_SS) && 
                  (value & CR_OUT_SS_VARIABLE)) {
          write = 1;
       } else if ((outputs[j].type == OUT_SS_MODEL) && 
                  (value & CR_OUT_SS_MODEL)) {
          write = 1;
       } else if ((outputs[j].type == OUT_VAR_PSS) && 
                  (value & CR_OUT_P_SS_VARIABLE)) {
          write = 1;
       } else if ((outputs[j].type == OUT_R2) && 
                  (value & CR_OUT_R2_MODEL)) {
          write = 1;
       } else if ((outputs[j].type == OUT_R2_ADJ) && 
               (value & CR_OUT_R2_ADJ_MODEL)) {
          write = 1;
       } else if ((outputs[j].type == OUT_COV_STDERR) && 
                  (value & CR_OUT_COVARIABLE_STD_ERROR)) {
          write = 1;
       } else if ((outputs[j].type == OUT_VAR_COV) && 
                  (value & CR_OUT_COEF_COVARIANCE)) {
          write = 1;
       }
       if (write) {
          /* send the sample */
          err = cvio_add_samples(outputs[j].Fid,&(volume.out_time),
                                 outputs[j].data,1);
	  if (err) exit_cvio_error(err);
       }
    }
    return;
}

/* start up the input streams */
void prime_input_streams(void)
{
   int i;
   int32_t err;

   for(i=0;i<modeltemp.nvars;i++) {
      if (i == volume.ingate) {
         /* one sample for the gate stream */
         err = reg_read_sample(modeltemp.vars[i].Fid,modeltemp.vars[i].data,
                  CVIO_TYP_FLOAT,&(modeltemp.vars[i].tNext),1);
         if (err == CVIO_ERR_EOF) {
            modeltemp.vars[i].tNext = 0xfffffffe;
            err = CVIO_ERR_OK;
         } else if (err) {
	    exit_cvio_error(err);
         }
      } else {
         if ((modeltemp.vars[i].type == VAR_Y_VAL) ||
             (modeltemp.vars[i].type == VAR_COV_V) ||
             (modeltemp.vars[i].type == VAR_CLASS)) {
         /* two samples for all others */
         err = reg_read_sample(modeltemp.vars[i].Fid,modeltemp.vars[i].data,
                  CVIO_TYP_FLOAT,&(modeltemp.vars[i].tNext),1);
         if (err == CVIO_ERR_EOF) {
            modeltemp.vars[i].tNext = 0xfffffffe;
            err = CVIO_ERR_OK;
         } else if (err) {
	    exit_cvio_error(err);
         } else {
            err = reg_read_sample(modeltemp.vars[i].Fid,
                     modeltemp.vars[i].data+volume.dsize,
                     CVIO_TYP_FLOAT,&(modeltemp.vars[i].tNext),1);
            if (err == CVIO_ERR_EOF) {
               modeltemp.vars[i].tNext = 0xfffffffe;
               err = CVIO_ERR_OK;
            } else if (err) {
   	       exit_cvio_error(err);
            }
         }
         }
      }
   }
   return;
}

/* read the input streams, adding observations until we 
 * reach the next output target.  We do this by stepping 
 * through the inputs according to the samples in the 
 * input gate stream */
void update_input_streams(void)
{
   int i;
   int32_t err;
   observation *obs;

   if (iVerbose > 1) {
      fprintf(stderr, "Update to: %d\n",modeltemp.vars[volume.ingate].tNext);
   }
   /* continue until the next input gate time is past out_time */
   while(modeltemp.vars[volume.ingate].tNext <= volume.out_time) {

      /* bring all the other inputs past tNext[gate] */
      for(i=0;i<modeltemp.nvars;i++) {
         if (i == volume.ingate) continue;
         if ((modeltemp.vars[i].type == VAR_Y_VAL) ||
             (modeltemp.vars[i].type == VAR_COV_V) ||
             (modeltemp.vars[i].type == VAR_CLASS)) {
         while(modeltemp.vars[i].tNext < modeltemp.vars[volume.ingate].tNext){
            /* shift the samples */
            memcpy(modeltemp.vars[i].data,modeltemp.vars[i].data+volume.dsize,
                   volume.dsize*sizeof(float));
            if (iVerbose > 1) {
               fprintf(stderr, "In(%d) to: %d\n",i,modeltemp.vars[i].tNext);
            }
            /* read the next */
            err = reg_read_sample(modeltemp.vars[i].Fid,
                     modeltemp.vars[i].data+volume.dsize,
                     CVIO_TYP_FLOAT,&(modeltemp.vars[i].tNext),1);
            if (err == CVIO_ERR_EOF) {
               modeltemp.vars[i].tNext = 0xfffffffe;
               err = CVIO_ERR_OK;
            } else if (err) {
   	       exit_cvio_error(err);
            }
         }
         }
      }

      /* add an input sample at tNext[gate] */
      obs = new_observation(modeltemp.vars[volume.ingate].tNext);

      /* and fill it with data */
      for(i=0;i<modeltemp.nvars;i++) {
         if ((modeltemp.vars[i].type == VAR_Y_VAL) ||
             (modeltemp.vars[i].type == VAR_CLASS) ||
             (modeltemp.vars[i].type == VAR_COV_V)) {
            memcpy(obs->vars[i],modeltemp.vars[i].data,
                   volume.dsize*sizeof(float));
         } else if (modeltemp.vars[i].type == VAR_CONST) {
            obs->vars[i][0] = modeltemp.vars[i].value;
         }
      }

      /* add it to the regressions... */
      addrem_observation(obs,0);

      /* bump tNext[gate] forward (in case we need more) */
      err = reg_read_sample(modeltemp.vars[volume.ingate].Fid,
               modeltemp.vars[volume.ingate].data,
               CVIO_TYP_FLOAT,&(modeltemp.vars[volume.ingate].tNext),1);
      if (err == CVIO_ERR_EOF) {
         modeltemp.vars[volume.ingate].tNext = 0xfffffffe;
         err = CVIO_ERR_OK;
      } else if (err) {
         exit_cvio_error(err);
      }
   }

   return;
}

/* add or remove this observation to/from the regression */
void addrem_observation(observation *o,int remove)
{
    int i,j,slab;
    double y_here;

    if (remove) {
       if (iVerbose) fprintf(stderr, "Remove obs at %d\n",o->time);
       /* bump to next rem slab */
       volume.currem_slab += 1;
       if (volume.currem_slab >= volume.nslab) volume.currem_slab = 0;
       slab = volume.currem_slab*volume.dsize;
    } else {
       if (iVerbose) fprintf(stderr, "Adding obs at %d\n",o->time);
       /* bump to next add slab */
       volume.curadd_slab += 1;
       if (volume.curadd_slab >= volume.nslab) volume.curadd_slab = 0;
       slab = volume.curadd_slab*volume.dsize;
    }
    for(i=0;i<volume.dsize;i++) {
        if (volume.maskdata[i+slab]) {
            double w = 1.0;
            int fail;
            memset(reg.xtmp,0,sizeof(double)*reg.np);
            for(j=0;j<modeltemp.nvars;j++) {
                switch(modeltemp.vars[j].type) {
                    case VAR_Y_VAL:
                    case VAR_Y_CONST:
                        y_here = o->vars[j][i];
                        break;
                    case VAR_CONST:
                        reg.xtmp[reg.XofV[j]] = o->vars[j][0];
                        break;
                    case VAR_COV_V:
                    case VAR_COV_C:
                        reg.xtmp[reg.XofV[j]] = o->vars[j][i];
                        break;
                    case VAR_CLASS:
                        if (o->vars[j][i]) {
                            reg.xtmp[reg.XofV[j]+(int)o->vars[j][i]-1] = 1.0;
                        }
                        break;
                    case VAR_LABEL:
                        break;
                }
            }
            if (volume.weightdata) w = volume.weightdata[i];
            if (remove) {
#ifdef DEBUG
printf("Remove: %f %f %f %d\n",y_here,reg.xtmp[0],reg.xtmp[1],slab);
#endif
                reg.nobs[i+slab] -= 1;
                reg.sumy[i+slab] -= y_here;
                reg.ssumy[i+slab] -= (y_here*y_here);
                remove_(&reg.np,&reg.nrbar,&w,reg.xtmp,&y_here,reg.dd[i+slab],
                      reg.r[i+slab],reg.theta[i+slab],&(reg.sser[i+slab]),&fail);
            } else {
#ifdef DEBUG
printf("Add: %f %f %f %d\n",y_here,reg.xtmp[0],reg.xtmp[1],slab);
#endif
                reg.nobs[i+slab] += 1;
                reg.sumy[i+slab] += y_here;
                reg.ssumy[i+slab] += (y_here*y_here);
                includ_(&reg.np,&reg.nrbar,&w,reg.xtmp,&y_here,reg.dd[i+slab],
                      reg.r[i+slab],reg.theta[i+slab],&(reg.sser[i+slab]),&fail);
            }
        }
    }
    return;
}

void setup_regression(void)
{
    int i,k,n;

/* count the number of variables, Y=1,CONST=1,COV=1,CLASS=value-1 */
    reg.np = 0;
    for(i=0;i<modeltemp.nvars;i++) {
        switch(modeltemp.vars[i].type) {
            case VAR_Y_VAL:
            case VAR_Y_CONST:
                break;
            case VAR_CONST:
            case VAR_COV_V:
            case VAR_COV_C:
                reg.np = reg.np + 1;
                break;
            case VAR_CLASS:
                reg.np = reg.np + (modeltemp.vars[i].value-1);
                break;
            case VAR_LABEL:
                break;
        }
    }

    reg.nrbar = reg.np*(reg.np-1)/2;

    n = volume.dsize*volume.nslab;

    reg.r = (double **)calloc(n,sizeof(double *));
    reg.dd = (double **)calloc(n,sizeof(double *));
    reg.theta = (double **)calloc(n,sizeof(double *));
    reg.ssumy = (double *)calloc(n,sizeof(double));
    reg.sumy = (double *)calloc(n,sizeof(double));
    reg.sser = (double *)calloc(n,sizeof(double));
    reg.nobs = (int *)calloc(n,sizeof(int));
    if (!reg.r || !reg.dd || !reg.theta || !reg.ssumy || !reg.sumy 
               || !reg.sser || !reg.nobs){
        ex_err("Unable to allocate regression arrays");
    }

    for(i=0;i<n;i++) {
        if (volume.maskdata[i]) {
            reg.r[i] = (double *)calloc(reg.nrbar,sizeof(double));
            reg.dd[i] = (double *)calloc(reg.np,sizeof(double));
            reg.theta[i] = (double *)calloc(reg.np,sizeof(double));
            if (!reg.r[i] || !reg.dd[i] || !reg.theta[i]) {
                ex_err("Unable to allocate regression arrays");
            }
        }
    }

    reg.ss = (double *)calloc(reg.np,sizeof(double));
    reg.xtmp = (double *)calloc(reg.np,sizeof(double));
    reg.tol = (double *)calloc(reg.np,sizeof(double));
    reg.work = (double *)calloc(reg.np,sizeof(double));
    reg.work2 = (double *)calloc(reg.np,sizeof(double));
    reg.vorder = (int *)calloc(reg.np,sizeof(int));
    reg.norder = (int *)calloc(reg.np,sizeof(int));
    reg.lindep = (int *)calloc(reg.np,sizeof(int));
    reg.XofV = (int *)calloc(modeltemp.nvars,sizeof(int));
    if (!reg.ss || !reg.tol || !reg.work || !reg.vorder || !reg.norder ||
	    !reg.lindep || !reg.XofV || !reg.xtmp || !reg.work2) {
        ex_err("Unable to allocate regression arrays");
    }

    /* set up the regressions */
    for(i=0;i<n;i++) {
        if (volume.maskdata[i]) {
            int fail;
            clear_(&reg.np,&reg.nrbar,reg.dd[i],reg.r[i],reg.theta[i],
			&(reg.sser[i]),&fail);
            reg.nobs[i] = 0;
        }
    }

    /* fill out XofV */
    k = 0;
    for(i=0;i<modeltemp.nvars;i++) {
        reg.XofV[i] = k;
        switch(modeltemp.vars[i].type) {
            case VAR_Y_VAL:
            case VAR_Y_CONST:
                break;
            case VAR_CONST:
            case VAR_COV_V:
            case VAR_COV_C:
                k = k + 1;
                break;
            case VAR_CLASS:
                k = k + (modeltemp.vars[i].value-1);
                break;
            case VAR_LABEL:
                break;
        }
    }

    return;
}

void cleanup_regression(void)
{
    int i,n;

    n = volume.dsize*volume.nslab;

    for(i=0;i<n;i++) {
        if (volume.maskdata[i]) {
            if (reg.r && reg.r[i]) free(reg.r[i]);
            if (reg.dd && reg.dd[i]) free(reg.dd[i]);
            if (reg.theta && reg.theta[i]) free(reg.theta[i]);
        }
    }

    if (reg.r) free(reg.r);
    if (reg.dd) free(reg.dd);
    if (reg.theta) free(reg.theta);
    if (reg.ssumy) free(reg.ssumy);
    if (reg.sumy) free(reg.sumy);
    if (reg.sser) free(reg.sser);
    if (reg.nobs) free(reg.nobs);

    if (reg.xtmp) free(reg.xtmp);
    if (reg.ss) free(reg.ss);
    if (reg.tol) free(reg.tol);
    if (reg.work) free(reg.work);
    if (reg.work2) free(reg.work2);
    if (reg.vorder) free(reg.vorder);
    if (reg.norder) free(reg.norder);
    if (reg.lindep) free(reg.lindep);
    if (reg.XofV) free(reg.XofV);

    return;
}

/* order so that the current coefficients are last... */
double  calc_pss(int np,int nrbar,int *vorder,double *dd,
        double *r,double *theta,double *x,double *tol,int *norder,
        int k,int j)
{
        int        pos1 = 1;
        int        i,num,l;
        int        ptr,l1;
        int        fail,done,TO,FROM;
        double          pss;

        ptr = 0;
        for(i=0;i<=k;i++) norder[ptr++] = i;
        for(i=j+1;i<np;i++) norder[ptr++] = i;
        l1 = ptr;
        for(i=k+1;i<=j;i++) norder[ptr++] = i;

        num = np-1;

if (iVerbose > 4) {
        printf("BEFORE Pss from %d to %d\n",k,j);
        printf("norder=");
        for(i=0;i<np;i++) printf(" %d",norder[i]);
        printf("\n");
        printf("vorder=");
        for(i=0;i<np;i++) printf(" %d",vorder[i]);
        printf("\n");
        printf("RSS=");
        for(i=0;i<np;i++) printf(" %f",x[i]);
        printf("\n");
        printf("num=%d,pos1=%d\n",num,pos1);
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

if (iVerbose > 4) {
        printf("AFTER Pss from %d to %d\n",k,j);
        printf("norder=");
        for(i=0;i<np;i++) printf(" %d",norder[i]);
        printf("\n");
        printf("vorder=");
        for(i=0;i<np;i++) printf(" %d",vorder[i]);
        printf("\n");
        printf("RSS=");
        for(i=0;i<np;i++) printf(" %f",x[i]);
        printf("\n");
        printf("num=%d,pos1=%d,l1=%d,pss=%f\n",num,pos1,l1,pss);
        printf("fail = %d\n",fail);
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

if (iVerbose > 4) {
        printf("AFTER reorder #2 %d %d\n",k,j);
        printf("norder=");
        for(i=0;i<np;i++) printf(" %d",norder[i]);
        printf("\n");
        printf("vorder=");
        for(i=0;i<np;i++) printf(" %d",vorder[i]);
        printf("\n");
        printf("RSS=");
        for(i=0;i<np;i++) printf(" %f",x[i]);
        printf("\n");
}

        return(pss);
}

double calc_var_cov(int np, int nrbar,double *dd,double *r, int var1, int var2)
{
        double  *inv,*covmat,*sterr;
        double  d = 0.0,var = 1.0;
        int fail,dimcov,i,j,k;

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

