/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski
 *
 * $Id: tal_mrinorm.c 1213 2005-08-27 20:51:21Z rjfrank $
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
#include "as274.h"
#include "proto.h"

#define MIN(a,b) (((a)<(b)?(a):(b)))
#define MAX(a,b) (((a)>(b)?(a):(b)))

#define	RJF_ISLICE	1.6

long int	verbose = 0;

void Calc_Shift(long int *length,double *cent,double *centn,unsigned char *data,
	long int lineprpt,long int lineelse,long int linepdir);

void ex_err(char *s)
{
	fprintf(stderr,"Fatal error:%s\n",s);
	tal_exit(1);
}

void cmd_err(char *s)
{
	fprintf(stderr,"(%s:%s) Usage: %s [options] input output\n",__DATE__,TAL_VERSION,s);
	fprintf(stderr,"Options: -x(dx) image dx default:256\n");
	fprintf(stderr,"         -y(dy) image dy default:256\n");
	fprintf(stderr,"         -f(start) first image number default:1\n");
	fprintf(stderr,"         -l(end) last image number default:124\n");
	fprintf(stderr,"         -i(step) image number step default:1\n");
/*
	fprintf(stderr,"         -b swap bytes with 16bit images default:no\n");
*/
	fprintf(stderr,"         -v verbose mode\n");
	fprintf(stderr,"         -m use exp end model default:linear\n");
	fprintf(stderr,"         -s(is) interslice spacing (used w/-m to scale template) default:1.6\n");
	fprintf(stderr,"         -o(order) fit order default:4\n");
	fprintf(stderr,"         -d(dumpfile) save the polynomial fit default:none\n");
	fprintf(stderr,"         -r(dumpfile) read the polynomial fit default:none\n");
	fprintf(stderr,"         -e(err) maximum error allowed in an applied fit default:100\n");
	fprintf(stderr,"         -axis(X|Y|Z) axis to correct along default:Z\n");
	fprintf(stderr,"         -avg(X|Y|Z)(X|Y|Z) line profile averages in (dim)(report) default:none\n");
	fprintf(stderr,"         -N restrict output pixel values to the original histogram default:not\n");
	tal_exit(1);
}

int main(int argc,char **argv)
{
	long int	dx = 256;
	long int	dy = 256;
	long int	dz = 1;
	long int	istart = 1;
	long int	iend = 124;
	long int	istep = 1;
	long int	swab = 0;
	long int	scandir = 2;
	long int	maxerr = 100;
	long int	forder = 4;
	long int	bend_ends = 0;
	long int	linepdir = -1;
	long int	lineprpt = -1;
	long int	hist_restr = 0;
	long int	lineelse;
	char		*readdump = 0;
	char		*savedump = 0;
	long int	useline = 1;
	float		islice = 1.6;

	float		t_x,t_y,t_i;
	char		intemp[256],outtemp[256];
	char		tstr[256];
	long int	i,err,j,k,x,y;
	long int	icount,n_slices;
	unsigned char	*data,*ptr;
	long int	left,right;
	long int	length[3];
	long int	mins,maxs;
	double		template[512];

/* stats */
	double		sum[512];
	double		ss[512];
	double		num[512];
	double		cent[512];
	double		centn[512];
	long int	hist[256];
	long int	max_hist,min_hist;
	double		avg_hist_gap;

/* GLM stuff */
	FILE		*fp;
	long int	np,nrbar,v[3],state;
	double		*r,*dd,*theta,*tol,*work,*xvec;
	long int	*lindep;
	long int	fail;
	double		sser,w,yval,vp;

	i = 1;
	while ((i < argc) && (argv[i][0] == '-')) {
		if (argv[i][1] == '\0') break;
		switch (argv[i][1]) {
			case 'f':
				istart = atoi(&(argv[i][2]));
				break;
			case 'l':
				iend = atoi(&(argv[i][2]));
				break;
			case 'i':
				istep = atoi(&(argv[i][2]));
				break;
			case 'x':
				dx = atoi(&(argv[i][2]));
				break;
			case 'y':
				dy = atoi(&(argv[i][2]));
				break;
			case 'r':
				readdump = &(argv[i][2]);
				break;
			case 'd':
				savedump = &(argv[i][2]);
				break;
			case 'v':
				verbose = 1;
				break;
			case 'b':
				swab = 1;
				break;
			case 'm':
				useline = 0;
				break;
			case 'N':
				hist_restr = 1;
				break;
			case 'e':
				maxerr = atoi(&(argv[i][2]));
				break;
			case 's':
				islice = atof(&(argv[i][2]));
				break;
			case 'o':
				forder = atoi(&(argv[i][2]));
				break;
			case 'B':
				bend_ends = atoi(&(argv[i][2]));
				break;
			case 'a':
				if (strncmp(argv[i],"-axis",5) == 0) {
					if (argv[i][5] == 'X') {
						scandir = 0;
					} else if (argv[i][5] == 'Y') {
						scandir = 1;
					} else if (argv[i][5] == 'Z') {
						scandir = 2;
					}
				} else if (strncmp(argv[i],"-avg",4) == 0) {
					if (argv[i][4] == 'X') {
						linepdir = 0;
					} else if (argv[i][4] == 'Y') {
						linepdir = 1;
					} else if (argv[i][4] == 'Z') {
						linepdir = 2;
					}
					if (argv[i][5] == 'X') {
						lineprpt = 0;
					} else if (argv[i][5] == 'Y') {
						lineprpt = 1;
					} else if (argv[i][5] == 'Z') {
						lineprpt = 2;
					}
				}
				break;
			default:	
				cmd_err(argv[0]);
				break;
		}
		i++;
	}
	if ((argc-i) != 2) cmd_err(argv[0]);
	strcpy(intemp,argv[i]);
	strcpy(outtemp,argv[i+1]);

/* get the image memory */
	icount = 0;
	for(i=istart;i<=iend;i=i+istep) icount++;

/* store the volume to be warped 8bit thresholded */
	data = malloc(dx*dy*icount*dz);
	if (data == 0L) ex_err("Unable to allocate image memory.");

/* get memory for line averaging if needed */
	if (linepdir >= 0) {
		length[0] = dx;
		length[1] = dy;
		length[2] = icount;
	}

/* read the images */
	fprintf(stderr,"Reading %ld byte dataset for curve computation\n",
		(dx*dy*icount));

	j = 0;
	for(i=istart;i<=iend;i=i+istep) {
		name_changer(intemp,i,&err,tstr);
		fprintf(stderr,"Reading the file:%s\n",tstr);
		bin_io(tstr,'r',&(data[j]),dx,dy,dz,1,swab,0L);
		j = j + (dx*dy);
	}

/* calc volume histogram */
	for(i=0;i<256;i++) hist[i] = 0;
	for(i=0;i<dx*dy*icount;i++) {
		hist[data[i]] += 1;
	}
	max_hist = 0;
	min_hist = 255;
	for(i=1;i<256;i++) {
		if (hist[i] > 0) {
			if (i > max_hist) max_hist = i;
			if (i < min_hist) min_hist = i;
		}
	}

/* find and size gaps */
	j = 0;
	num[j] = 0.0;
	for(i=min_hist+1;i<max_hist;i++) {
		num[j] += 1.0;
		if (hist[i] > 0) {
			j += 1;
			num[j] = 0.0;
		}
	}

/* find average gap */
	avg_hist_gap = 0.0;
	for(i=0;i<j;i++) avg_hist_gap += num[j];
	avg_hist_gap /= (double)(j);

/* allow 0 to be on the bottom */
	if (hist[0] > 0) min_hist = 0;

/* compute the plane-wise averages */
	if (scandir == 0) n_slices=dx;
	if (scandir == 1) n_slices=dy;
	if (scandir == 2) n_slices=icount;
	for(i=0;i<n_slices;i++) {
		sum[i] = 0;
		ss[i] = 0;
		num[i] = 0;
	}
	ptr = data;
	for(k=0;k<icount;k++) {
	for(j=0;j<dy;j++) {
	for(i=0;i<dx;i++) {
		if (*ptr != 0) {
			switch(scandir) {
				case 0:
					sum[i] += *ptr;
					ss[i] += ((*ptr)*(*ptr));
					num[i] += 1.0;
					break;
				case 1:
					sum[j] += *ptr;
					ss[j] += ((*ptr)*(*ptr));
					num[j] += 1.0;
					break;
				case 2:
					sum[k] += *ptr;
					ss[k] += ((*ptr)*(*ptr));
					num[k] += 1.0;
					break;
			}
/* next voxel */
		}
		ptr++;
	}
	}
	}
/* reduce to means (and stdev) */
	for(i=0;i<n_slices;i++) {
		if (num[i] < 2) num[i] = 2;
		sum[i] = sum[i] / num[i];
		ss[i] = (ss[i] - (num[i]*sum[i]*sum[i]))/(num[i] - 1.0);
		if (verbose && (linepdir < 0)) {
			fprintf(stdout,"%ld\t%f\t%f\t%f\n",
				i,num[i],sum[i],sqrt(ss[i]));
		}
	}
/* 
	state: 0_____1NNNNN2____3NNNNNN4______5
*/
	if (linepdir >= 0) {
		for(i=0;i<3;i++) {
			if ((i != lineprpt) && (i != linepdir)) lineelse = i;
		}
		for(v[lineprpt]=0;v[lineprpt]<length[lineprpt];v[lineprpt]++) {

		cent[v[lineprpt]] = 0;
		centn[v[lineprpt]] = 0;

		for(v[lineelse]=40;v[lineelse]<length[lineelse];v[lineelse]++) {

		state = 0;
		k = 0;
		for(v[linepdir]=0;v[linepdir]<length[linepdir];v[linepdir]++) {
			i = (v[2]*dx*dy)+(v[1]*dx)+v[0];
			j = data[i];
			switch(state) {
				case 0:
					if (j != 0) state = 1;
					break;
				case 1:
					if (j == 0) {
						left= v[linepdir];
						state = 2;
					}
					break;
				case 2:
/* only allow center gaps between 4 and 50 pixels */
					k++;
					if (k > 25) {
						state = 6;
					} else {
						if (j != 0) {
							if (k > 2) {
							    right= v[linepdir];
							    state = 3;
							} else {
							    state= 6;
							}
						}
					}
					break;
				case 3:
					if (j == 0) state = 4;
					break;
				case 4:
					if (j != 0) state = 6;
					break;
			}
		}  /* along a scan line */

/* did we end up at the right state? */
		if (state == 4) {
			centn[v[lineprpt]]++;
			cent[v[lineprpt]] += (left+((right-left)/2.0));
		}

		} /* for all "same" plane scan lines */
		if (centn[v[lineprpt]] != 0) {
			cent[v[lineprpt]] /= centn[v[lineprpt]];
		}
#ifdef DEBUG
		if (verbose) {
			fprintf(stdout,"%ld\t%lf\t%lf\n",v[lineprpt],	
				cent[v[lineprpt]],centn[v[lineprpt]]);
		}
#endif
		} /* for each new plane */
/* compute and apply midline shift */
		Calc_Shift(length,cent,centn,data,lineprpt,lineelse,linepdir);
	}

/* compute the fitting function */
	np = forder+1;  /* a4(xxxx) + a3(xxx) + a2(xx) + a1(x) + a0 = y */
	nrbar = np*(np-1)/2;

	r = (double *)malloc(nrbar*sizeof(double));
	dd = (double *)malloc(np*sizeof(double));
	theta = (double *)malloc(np*sizeof(double));
	tol = (double *)malloc(np*sizeof(double));
	work = (double *)malloc(np*sizeof(double));
	lindep = (long int *)malloc(np*sizeof(long int));
	xvec = (double *)malloc(np*sizeof(double));

	if (!readdump) {

/* setup a new regression */
        	clear_(&np,&nrbar,dd,r,theta,&sser,&fail);

		mins = n_slices;
		maxs = 0;
		for(i=0;i<n_slices;i++) {
/* get a datum */
			xvec[0] = 1;
			for(j=1;j<np;j++) {
				xvec[j] = xvec[j-1]*i;
			}
			yval = sum[i];
			if (num[i] > 2000) { /* only use large bins */
				if (i > maxs) maxs = i;
				if (i < mins) mins = i;
/* add it to the datset */
                   w = 1;  /* all weighted the same */
                   includ_(&np,&nrbar,&w,xvec,&yval,dd,r,theta,&sser,&fail);
			}
		}

		if (bend_ends > 0) {
/* add the endpoint zeros (mins,maxs) */
			for(i=0;i<bend_ends;i++) {
/* min end */
			xvec[0] = 1;
			for(j=1;j<np;j++) {
				xvec[j] = xvec[j-1]*(mins-i-1);
			}
			yval = 0;
/* add it to the datset */
                w = 1;  /* all weighted the same */
                includ_(&np,&nrbar,&w,xvec,&yval,dd,r,theta,&sser,&fail);
/* max end */
			xvec[0] = 1;
			for(j=1;j<np;j++) {
				xvec[j] = xvec[j-1]*(maxs+i+1);
			}
			yval = 0;
/* add it to the datset */
                w = 1;  /* all weighted the same */
                includ_(&np,&nrbar,&w,xvec,&yval,dd,r,theta,&sser,&fail);
			}
		}

/* set singular tolerances */
        	tolset_(&np,&nrbar,dd,r,tol,work,&fail);

/* check for singularities */
        	sing_(&np,&nrbar,dd,r,theta,&sser,tol,lindep,work,&fail);

/* extract needed regression info */
        	ss_(&np,dd,theta,&sser,xvec,&fail);  
		/* x=SS left after adding var i */

/* get coefficients */
        	regcf_(&np,&nrbar,dd,r,theta,tol,work,&np,&fail); 
		/*work=coefficients*/
	}

/* free up the GLM memory */
       	free(r);
       	free(dd);
       	free(theta);
       	free(tol);
 	free(lindep);
       	free(xvec);

/* report */
	if (verbose) {
		fprintf(stderr,"Y=");
		for(i=np-1;i>0;i--) {
			fprintf(stderr,"%g*x**%ld+",work[i],i);
		}
		fprintf(stderr,"%g\n",work[0]);
	}
/* compute the template to mold the curve to */
	if (useline) {
		for(i=0;i<n_slices;i++) {
			template[i] = 128.0;
		}
	} else {
		x = n_slices;
		y = 0;
/* find the first and last slices... */
		for(i=0;i<n_slices;i++) {
			if (num[i] > 2000) {
				if (i < x) x = i;
				if (i > y) y = i;
			}
		}
		t_x = (float)(x)*(islice/RJF_ISLICE);
		t_y = (float)(y)*(islice/RJF_ISLICE);
		for(i=0;i<n_slices;i++) {
			t_i = (float)(i)*(islice/RJF_ISLICE);
			if (i < (x+y)/2) {
				template[i]=128.0-60.0*exp(-0.35*(t_i-(t_x-4)));
			} else {
				template[i]=128.0-60.0*exp(-0.25*((6+t_y)-t_i));
			}
		}
	}
/* compute the mean shift */
	for(i=0;i<n_slices;i++) {
		ss[i] = work[0];
		for(j=1;j<np;j++) {
			ss[i] += (work[j]*pow(i,j));
		}
/* how accurate was the fit? */
		yval = ss[i] - sum[i];
/* if close enough, use it */
		if (fabs(yval) < maxerr) {
			ss[i] = template[i]/ss[i];
		} else {
			ss[i] = 1.0;
		}
	}
        free(work);

/* save the fit mults */
	if (savedump) {
		fp = fopen(savedump,"w");
		if (fp != 0) {
			fprintf(fp,"%ld\n",n_slices);
			for(i=0;i<n_slices;i++) {
				fprintf(fp,"%f\n",(ss[i]));
			}
			fclose(fp);
		} else {
			ex_err("Unable to open dump file for writing.");
		}
	}

/* read the fit mults */
	if (readdump) {
		fp = fopen(readdump,"r");
		if (fp != 0) {
			fscanf(fp,"%ld\n",&j);
			if (j == n_slices) {
				for(i=0;i<n_slices;i++) {
					fscanf(fp,"%lf\n",&(ss[i]));
				}
			} else {
			ex_err("Dump file does not match current dataset.");
			}
			fclose(fp);
		} else {
			ex_err("Unable to open dump file for reading.");
		}
	}

/* apply the fitted function */
	ptr = data;
	for(k=0;k<icount;k++) {
	for(j=0;j<dy;j++) {
	for(i=0;i<dx;i++) {
		if (*ptr != 0) {
			switch(scandir) {
				case 0:
					vp=(*ptr) * (ss[i]);
					break;
				case 1:
					vp=(*ptr) * (ss[j]);
					break;
				case 2:
					vp=(*ptr) * (ss[k]);
					break;
			}
			err = vp;

/* restrict to the original pixel histogram values */
			if (hist_restr) {
				err = (int)(vp/avg_hist_gap);
				err = (int)((double)(err)*avg_hist_gap);
#ifdef NEVER
			    if (err < max_hist) {
				while(err < 256) {
					if (hist[err] > 0) break;
					err++;
				}
				if (err == 256) {
					while(err > 0) {
						if (hist[err] > 0) break;
						err--;
					}
				}
			    }
#endif

			}

/* clamp and store */
			if (err > 255) err = 255;
			if (err < 0) err = 0;

			*ptr = err;
		}
		ptr++;
	}
	}
	}

/* write the output images */
	j = 0;
	for(i=istart;i<=iend;i=i+istep) {
		name_changer(outtemp,i,&err,tstr);
		fprintf(stderr,"Writing the file:%s\n",tstr);
		bin_io(tstr,'w',&(data[j]),dx,dy,dz,1,swab,0L);
		j = j + (dx*dy);
	}

/* done */	
	if (linepdir >= 0) {
	}
	free(data);

	tal_exit(0);

	exit(0);
}


void Calc_Shift(long int *length,double *cent,double *centn,
	unsigned char *data,
	long int lineprpt,long int lineelse,long int linepdir)
{
        long int        np,nrbar,v[3],i,j,dx,dy;
        double          *r,*dd,*theta,*tol,*work,*xvec;
        long int        *lindep;
        long int        fail;
        double          sser,w,yval,ss[512];
	unsigned char	tscan[512];

	dx = length[0];
	dy = length[1];

/* compute the fitting function */
        np = 4+1;  /* a4(xxxx) + a3(xxx) + a2(xx) + a1(x) + a0 = y */
        nrbar = np*(np-1)/2;

        r = (double *)malloc(nrbar*sizeof(double));
        dd = (double *)malloc(np*sizeof(double));
        theta = (double *)malloc(np*sizeof(double));
        tol = (double *)malloc(np*sizeof(double));
        work = (double *)malloc(np*sizeof(double));
        lindep = (long int *)malloc(np*sizeof(long int));
        xvec = (double *)malloc(np*sizeof(double));

/* setup a new regression */
        clear_(&np,&nrbar,dd,r,theta,&sser,&fail);

        for(i=0;i<length[lineprpt];i++) {
/* get a datum */
                xvec[0] = 1;
                for(j=1;j<np;j++) {
                        xvec[j] = xvec[j-1]*i;
                }
                yval = cent[i];
                if (centn[i] > 0) { /* only use large bins */
/* add it to the datset */
                   w = 1;  /* all weighted the same */
                   includ_(&np,&nrbar,&w,xvec,&yval,dd,r,theta,&sser,&fail);
                }
        }

/* set singular tolerances */
        tolset_(&np,&nrbar,dd,r,tol,work,&fail);

/* check for singularities */
        sing_(&np,&nrbar,dd,r,theta,&sser,tol,lindep,work,&fail);

/* extract needed regression info */
        ss_(&np,dd,theta,&sser,xvec,&fail);
                /* x=SS left after adding var i */

/* get coefficients */
        regcf_(&np,&nrbar,dd,r,theta,tol,work,&np,&fail);
        /*work=coefficients*/

/* free up the GLM memory */
        free(r);
        free(dd);
        free(theta);
        free(tol);
        free(lindep);
        free(xvec);

/* printout */
	if (verbose) {
		fprintf(stderr,"Y=");
		for(i=np-1;i>0;i--) {
			fprintf(stderr,"%g*x**%ld+",work[i],i);
		}
		fprintf(stderr,"%g\n",work[0]);

		for(i=0;i<length[lineprpt];i++) {
			fprintf(stdout,"%ld\t%f\n",i,cent[i]);
		}
	}

/* compute the shift distances */
	for(i=0;i<length[lineprpt];i++) {
		ss[i] = work[0];
		for(j=1;j<np;j++) {
			ss[i] += (work[j]*pow(i,j));
		}
/* how accurate was the fit? */
		ss[i] = ss[i] - 128;
/* if close enough, use it */
		if (fabs(ss[i]) > 40) {
			ss[i] = 0.0;
		}
	}

/* free up the last of it */
	free(work);

/* apply the shift! */
	for(v[lineprpt]=0;v[lineprpt]<length[lineprpt];v[lineprpt]++) {
	for(v[lineelse]=0;v[lineelse]<length[lineelse];v[lineelse]++) {

/* copy temporary scanline */
	for(v[linepdir]=0;v[linepdir]<length[linepdir];v[linepdir]++) {
		j = (v[2]*dx*dy)+(v[1]*dx)+v[0];
		tscan[v[linepdir]] = data[j];
	}
/* shift the scanline */
	for(v[linepdir]=0;v[linepdir]<length[linepdir];v[linepdir]++) {
		j = (v[2]*dx*dy)+(v[1]*dx)+v[0];
		i = v[linepdir]+ss[ v[lineprpt] ];
		if (i<0) i = 0;
		if (i>=length[linepdir]) i = length[linepdir]-1;
		data[j] = tscan[i];
	}

	}
	}

	return;
}
