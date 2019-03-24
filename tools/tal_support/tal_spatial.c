/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski
 *
 * $Id: tal_spatial.c 1213 2005-08-27 20:51:21Z rjfrank $
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
#include "stats_inc.h"
#include "proto.h"


/* This application is a filter which takes the output of tal_cluster and
	appends a column containing the Probability basd on the equations
	found in:

	Friston KJ, Worsley KJ, Frackowiak RSJ, Mazziotta JC, Evans AC;
		Assessing the Significance of Focal Activations Using Their
		Spatial Extent. Human Brain Mapping 1:210-220 (1994).


Input:
	x	y	z	avgval	size	serial  thresh

Output:
	x	y	z	avgval	size	serial  thresh 	probability

*/

double	spatial_prob(double u,double k,double s,double w,double d);
double 	inv_prob(double u,double p,double s,double w,double d);
double	theerf(double u);
void cmd_err(char *s);

void ex_err(char *s)
{
        fprintf(stderr,"Fatal error:%s\n",s);
        tal_exit(1);
}

void cmd_err(char *s)
{
        fprintf(stderr,"(%s:%s) Usage: %s size FWHM dimension\n",__DATE__,TAL_VERSION,s);
        fprintf(stderr,"       size : Number of voxels in the search\n");
        fprintf(stderr,"       FWHM : Smoothness of the process\n");
        fprintf(stderr,"       dim  : Dimension of the process (3=3D,2=2D)\n");
        fprintf(stderr,"\n");
	fprintf(stderr,"This application is a filter which takes the output of tal_cluster and\n");
	fprintf(stderr,"appends a column containing the Probability basd on the equations\n");
	fprintf(stderr,"found in:\n");

	fprintf(stderr,"Friston KJ, Worsley KJ, Frackowiak RSJ, Mazziotta JC, Evans AC;\n");
	fprintf(stderr,"\tAssessing the Significance of Focal Activations Using Their\n");
	fprintf(stderr,"\tSpatial Extent. Human Brain Mapping 1:210-220 (1994).\n");
	fprintf(stderr,"\nNote:It is assumed that the clustered volume is a\n");
	fprintf(stderr,"     normal (Z) volume (SPM{z})\n");

	tal_exit(1);
}

int main(int argc,char **argv)
{

	double		s,d,w,p;
	double		x,y,z,v,num,ser,thr;
	double		u,k;
	char		tstr[256];
	long int	testmode = 0;

	if (argc < 4)  cmd_err(argv[0]);
	if (argc > 4)  testmode = 1;
	if (argc > 5)  testmode = 2;

	s=atof(argv[1]);
	w=atof(argv[2]);
	d=atof(argv[3]);

	w = w/sqrt(4.0*log(2.0));

	if (testmode == 1) {
	fprintf(stderr,"Entering testmode1.  Enter thres,num pairs:\n");
		while (fgets(tstr,256,stdin)) {
			if (sscanf(tstr,"%lf %lf",&u,&k) == 2) {
				p = spatial_prob(fabs(u),k,s,w,d);
				fprintf(stdout,"%f\t%f\t%f\n",u,k,p);
			}
		}
		tal_exit(0);
	}
	if (testmode == 2) {
	fprintf(stderr,"Entering testmode2.  Enter thres,alpha pairs:\n");
		while (fgets(tstr,256,stdin)) {
			if (sscanf(tstr,"%lf %lf",&u,&p) == 2) {
				k = inv_prob(fabs(u),p,s,w,d);
				fprintf(stdout,"%f\t%f\t%f\n",u,k,p);
			}
		}
	}
	
	while (fgets(tstr,256,stdin)) {
		if (sscanf(tstr,"%lf %lf %lf %lf %lf %lf %lf",
			&x,&y,&z,&v,&num,&ser,&thr) == 7) {
			if (thr > 50.0) {
				p = 0.0;
			} else {
				p = spatial_prob(fabs(thr),num,s,w,d);
			}
			fprintf(stdout,
		"%.3f\t%.3f\t%.3f\t%.4f\t%.0f\t%.0f\t%.4f\t%.6f\n",
				x,y,z,v,num,ser,thr,p);
		}
	}

	tal_exit(0);

	exit(0);
}


#ifdef APPROX
double	theerf(double u)
{
	return(exp(-(u*u)/2.0)/(u*sqrt(2.0*M_PI)));
}
#else
double	theerf(double u)
{
	return(1.0-CDF_normal(u,NULL));
}
#endif

double	spatial_prob(double u,double k,double s,double w,double d)
{
	double	eN,em,b,p;
	double	g,lg;

	eN= s*theerf(u);
	em= s*pow(2.0*M_PI,-(d+1.0)/2.0)*pow(w,-d)*
		pow(u,d-1.0)*exp(-(u*u)/2.0);
/*	lg = lgamma((d/2.0)+1.0); g = signgam*exp(lg); */
	lg = gammln((d/2.0)+1.0); g = exp(lg);
	b = pow((g)*em/eN,2.0/d);
	p = 1.0-exp(-em*exp(-b*pow(k,2.0/d)));

	return(p);
}


double inv_prob(double u,double p,double s,double w,double d)
{
	double	eN,em,b,k;
	double	g,lg;

	eN= s*theerf(u);
	em= s*pow(2.0*M_PI,-(d+1.0)/2.0)*pow(w,-d)*
		pow(u,d-1.0)*exp(-(u*u)/2.0);
/*	lg = lgamma((d/2.0)+1.0); g = signgam*exp(lg); */
	lg = gammln((d/2.0)+1.0); g = exp(lg);
	b = pow((g)*em/eN,2.0/d);

	k = pow(log(-em/log(1.0-p))/b,d/2.0);

	return(k);
}
