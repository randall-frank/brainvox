/*
 * $Id: worsley.c 1250 2005-09-16 15:51:42Z dforeman $
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "stats_inc.h"

#include "proto.h"

void ex_err(char *s)
{
	fprintf(stderr,"Fatal error: %s\n",s);
	tal_exit(1);
}

double FWHMz(double f,double d,double dc);
double psv(double r,double s,double pvar,double n);
double ptv(double r,double t,double n);
double pfv(double r,double t,double kn,double vn);
double pxv(double r,double t,double n);
double pzv(double r,double t);
double pzsv3(double r,double t,double w1,double w2);
double pzsv2(double r,double t,double w1,double w2);
double pzsv1(double r,double t,double w1,double w2);
double pzsv0(double r,double t,double w1,double w2);

double pzv(double r,double t)
{
	double	pi,p;

	pi = atan2(0.0,-1.0);

	p = r * pow(4.0*log(2.0),3.0/2.0) * (pow(2.0*pi,-2.0)) * 
		(t*t -1.0)*exp(-0.5*t*t);

	return p;
}
double ptv(double r,double t,double n)
{
	double	pi,p;

	pi = atan2(0.0,-1.0);

	p = r * (pow(4.0*log(2.0),(3.0/2.0)) * pow(2.0*pi,-2.0));
	p = p * (((n-2.0)/(n-1.0))*t*t-1.0) * pow(1.0+t*t/(n-1.0),-0.5*(n-2.0));

	return p;
}
double pfv(double r,double t,double kn,double vn)
{
	double	pi,p,v,k;

	pi = atan2(0.0,-1.0);
	v = kn - 1.0;
	k = vn - 1.0;

	p = r * (pow(4.0*log(2.0),(3.0/2.0)) * pow(2*pi,-(3.0/2.0)));
	p = p * exp(gammln((v+k-3.0)/2.0)) * pow(2.0,-0.5);
	p = p / (exp(gammln(v/2.0))*exp(gammln(k/2.0)));
	p = p * pow((k*t)/v,0.5*(k-3.0))*pow(1.0+((k*t)/v),-0.5*(v+k-2.0));
	p = p * ((v-1.0)*(v-2.0)*pow((k*t)/v,2.0) - 
		(2.0*v*k -v -k -1.0)*((k*t)/v) +
		(k-1.0)*(k-2.0));

	return p;
}
double pxv(double r,double t,double n)
{
	double	pi,p,v;

	pi = atan2(0.0,-1.0);
	v = n - 1.0;

	p = r * (pow(4.0*log(2.0),(3.0/2.0)) * pow(2.0*pi,-(3.0/2.0)));
	p = p * pow(t,0.5*(v-3.0))*exp(-0.5*t);
	p = p / ( pow(2.0,0.5*(v-2.0))*exp(gammln(v/2.0)) );
	p = p * (t*t - (2.0*v-1.0)*t + (v-1.0)*(v-2.0));

	return p;
}
double psv(double r,double s,double pvar,double n)
{
	double	pi,u,top,bot,p;

	pi = atan2(0.0,-1.0);

	u = (n-1.0)*(s*s)/pvar;

	bot = pow(2.0*pi,3.0/2.0)*pow(2.0,0.5*(n-3.0))*exp(gammln((n-1.0)/2.0));
	top = pow(u,0.5*(n-4.0)) * exp(-0.5*u);

	p = r * pow(4.0*log(2.0),3.0/2.0);
	p = p * (top/bot) * ((u*u)-(2.0*n-3.0)*u+(n-2.0)*(n-3.0));

	return p;
}
double FWHMz(double f,double d,double dc)
{
	double	p,f1,f2,f3,fz;

	p = exp(-2.0*log(2.0)*(d*d)/(f*f));
	f1 = d*sqrt(4.0*log(2.0));
	f2 = sqrt(1.0-(1.0-p)/3.0);
	f3 = 1.0/sqrt(2.0*(1.0-p) - (dc/d)*(3.0-4.0*p+(p*p*p*p))/3.0);
	fz = f1*f2*f3;

	return fz;
}
double pzsv3(double r,double t,double w1,double w2)
{
	double u;
	double	pi,p;
	
	u = w1/w2;

	pi = atan2(0.0,-1.0);

	p = r * pow(4.0*log(2.0),3.0/2.0) * (pow(2.0*pi,-2.0)) * exp(-0.5*t*t);
	p = p * ( (sqrt(3.0/(4.0*pi))*((1.0-u*u*u)/3.0)*(t*t*t-t)) +
		  (((1.0+u*u*u)/2.0)*(t*t-1.0)) );

	return p;
}
double pzsv2(double r,double t,double w1,double w2)
{
	double u;
	double	pi,p;
	
	u = w1/w2;

	pi = atan2(0.0,-1.0);

	p = r * pow(4.0*log(2.0),1.0) * (pow(2.0*pi,-3.0/2.0)) * exp(-0.5*t*t);
	p = p * ( (sqrt(3.0/(4.0*pi))*((1.0-u*u)/2.0)*(t*t-(1.0/3.0))) +
		  (((1.0+u*u)/2.0)*(t)) );

	return p;
}
double pzsv1(double r,double t,double w1,double w2)
{
	double u;
	double	pi,p;
	
	u = w1/w2;

	pi = atan2(0.0,-1.0);

	p = r * pow(4.0*log(2.0),1.0/2.0) * (pow(2.0*pi,-1.0)) * exp(-0.5*t*t);
	p = p * ( (sqrt(3.0/(4.0*pi))*(1.0-u)*(t)) +
		  (((1.0+u)/2.0)) );

	return p;
}
/* CDF_normal(t,0L) */
double pzsv0(double r,double t,double w1,double w2)
{
	double u;
	double	pi,p;
	
	u = w1/w2;

	pi = atan2(0.0,-1.0);

	p = r * (pow(2.0*pi,-1.0/2.0));
	p = p * ( (sqrt(3.0/(4.0*pi))*(-log(u))*exp(-(t*t)/2.0)) );
	p = p + ( r * (CDF_normal(-t,0L)) );

	return p;
}

int main(int argc,char **argv)
{
	double		p,pvar,n,s,r,t,dc,f,pi,d,n2;
	double		r0,r1,r2,r3;

	pi = atan2(0,-1);
	if (argc == 1) {
		printf("(%s) Usage %s : func [params...]\n",__DATE__,argv[0]);
		printf("\nFunction  Parameters\n");
		printf("FWHMz     FWHMz dorig dnew [stopdnew step]\n");
		printf("          \n");
		printf("Psv       resels svalue Pvar n [stoptval step]\n");
		printf("          \n");
		printf("Pzv       resels Zvalue [stoptval step]\n");
		printf("          3D gaussian-field\n");
		printf("Ptv       resels tvalue n [stoptval step]\n");
		printf("          3D t-field\n");
		printf("Pfv       resels Fvalue kn vn [stoptval step]\n");
		printf("          3D f-field\n");
		printf("Pxv       resels ChiSquvalue n [stoptval step]\n");
		printf("          3D ChiSqu-field\n");
		printf("From: Worsley KJ, Marrett S, Neelin P, Vandal AC, Friston KJ, Evans AC, A\n");
		printf("Unified Statistical Approach for Determining Significant Signals in Images of\n");
		printf("Cerebral Activation, (submitted Human Brain Mapping)\n\n");
		printf("Pzsv      3Dres 2Dres 1Dres 0Dres Zvalue FWHM(min) FWHM(max) [stoptval step]\n");
		printf("          3D gaussian scale space (resels computed at min FWHM)\n");
		printf("From: Worsley KJ, Marrett S, Neelin P, Evans AC, Searching Scale Space for\n");
		printf("Activation in PET Images, (submitted Human Brain Mapping)\n\n");
		printf("Note: N values used here should be N and not df (N-1)\n");
		tal_exit(1);
	}

	if (strcmp(argv[1],"FWHMz") == 0) {

		if (argc < 5) tal_exit(1);
		f = atof(argv[2]);
		d = atof(argv[3]);
		dc = atof(argv[4]);
		if (argc == 7) {
			for(;dc<=atof(argv[5]);dc+=atof(argv[6])) {
				printf("%f\t%f\n",dc,FWHMz(f,d,dc));
			}
		} else {
			printf("FWHMzc\t%f\n",FWHMz(f,d,dc));
		}
		
	} else if (strcmp(argv[1],"Pzv") == 0) {

		if (argc < 4) tal_exit(1);
		r = atof(argv[2]);
		t = atof(argv[3]);
		if (argc == 6) {
			for(;t<=atof(argv[4]);t+=atof(argv[5])) {
				printf("%f\t%g\n",t,pzv(r,t));
			}
		} else {
			printf("Pzv(%f,%f)\t%g\n",r,t,pzv(r,t));
		}

	} else if (strcmp(argv[1],"Ptv") == 0) {

		if (argc < 5) tal_exit(1);
		r = atof(argv[2]);
		t = atof(argv[3]);
		n = atof(argv[4]);
		if (argc == 7) {
			for(;t<=atof(argv[5]);t+=atof(argv[6])) {
				printf("%f\t%g\n",t,ptv(r,t,n));
			}
		} else {
			printf("Ptv(%f,%f,%f)\t%g\n",r,t,n,ptv(r,t,n));
		}

	} else if (strcmp(argv[1],"Psv") == 0) {

		if (argc < 6) tal_exit(1);
		r = atof(argv[2]);
		s = atof(argv[3]);
		pvar = atof(argv[4]);
		n = atof(argv[5]);
		if (argc == 8) {
			for(;t<=atof(argv[6]);t+=atof(argv[7])) {
				printf("%f\t%g\n",s,psv(r,s,pvar,n));
			}
		} else {
			printf("Psv(%f,%f,%f)\t%g\n",r,s,n,psv(r,s,pvar,n));
		}

	} else if (strcmp(argv[1],"Pfv") == 0) {

		if (argc < 6) tal_exit(1);
		r = atof(argv[2]);
		t = atof(argv[3]);
		n = atof(argv[4]);
		n2 = atof(argv[5]);
		if (argc == 8) {
			for(;t<=atof(argv[6]);t+=atof(argv[7])) {
				printf("%f\t%g\n",t,pfv(r,t,n,n2));
			}
		} else {
			printf("Pfv(%f,%f,%f,%f)\t%g\n",r,t,n,n2,
				pfv(r,t,n,n2));
		}

	} else if (strcmp(argv[1],"Pxv") == 0) {

		if (argc < 5) tal_exit(1);
		r = atof(argv[2]);
		t = atof(argv[3]);
		n = atof(argv[4]);
		if (argc == 7) {
			for(;t<=atof(argv[5]);t+=atof(argv[6])) {
				printf("%f\t%g\n",t,pxv(r,t,n));
			}
		} else {
			printf("Pxv(%f,%f,%f)\t%g\n",r,t,n,pxv(r,t,n));
		}
 
	} else if (strcmp(argv[1],"Pzsv") == 0) {

		if (argc < 9) tal_exit(1);
		r3 = atof(argv[2]);
		r2 = atof(argv[3]);
		r1 = atof(argv[4]);
		r0 = atof(argv[5]);
		t = atof(argv[6]);
		n = atof(argv[7]);
		n2 = atof(argv[8]);
		if (argc == 11) {
			for(;t<=atof(argv[9]);t+=atof(argv[10])) {
				r = pzsv0(r0,t,n,n2);
				r += pzsv1(r1,t,n,n2);
				r += pzsv2(r2,t,n,n2);
				r += pzsv3(r3,t,n,n2);
				printf("%f\t%g\n",t,r);
			}
		} else {
			r = pzsv0(r0,t,n,n2);
			r += pzsv1(r1,t,n,n2);
			r += pzsv2(r2,t,n,n2);
			r += pzsv3(r3,t,n,n2);
			printf(
		 	    "Pzsv(%.1f,%.1f,%.1f,%.1f,%f,%f,%f)\t%g\n",
			    r0,r1,r2,r3,t,n,n2,r);
		}

	} else {
		fprintf(stderr,"Unknown function: %s\n",argv[1]);
		tal_exit(1);
	}
	tal_exit(0);
	return(0);
}
