/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/22/01 */

/*
 * double dqage()
 *
 * Calculates an approximation result to a given definite integral:
 *	I=Integral of F over (A,B)
 *	hopefully satisfying the following claim for accuracy:
 *	abs(i-result)<=max(epsabs,epsrel*abs(i))
 *
 * Only the KEY<2 (7-15 points in Gauss-Kronrod pair was ported.
 *
 * Ported from the SLATEC Fortran routine which was written by
 * 	Robert Piessens and Elise de Doncker.
 *
 * on input,
 *
 * F	Function routine defining the integrand function f(x)
 * A	Lower limit of integration
 * B	Upper limit of integration
 * EPSABS absolute accuracy requested
 * EPSREL relative accuracy requested
 *	if epsabs<=0 && epsrel<max(50*rel.mach.acc,.5E-28)
 *	the routine will end with ier=6
 * LIMIT gives upper bound on # of subintervals in the partition
 *
 * on return,
 *
 * RESULT approximation to integral
 * ABSERR estimate of the modulus of the absolute error
 * NEVAL number of integrand evaluations
 * IER error message (0 if normal and reliable termination
 * ALIST vector of dimension at least LIMIT, the first LAST elements
 *  	of which are the left end points of the subintervals in the
 *	partition of the given integration range (A,B)
 * BLIST vector of dimension at least LIMIT, the first LAST elements
 * 	of which are the right end points of the subintervals in the
 *	partition of the given integration range (A,B)
 * RLIST vector of dimension at least LIMIT, the first LAST elements
 *	of which are the integral approximations on the subintervals
 * ELIST vector of dimension at least LIMIT, the first LAST elements
 *	of which are the moduli of the absolute error estimates on the
 *	subintervals
 * IORD vector of dimension LIMIT, the first K elements of which are
 *	pointers to the error estimates over the subintervals, such
 *	that ELIST[IORD[1]],...,ELIST[IORD[K]] for a decreasing
 *	sequence, with K=LAST. IF LAST<=(LIMIT/2+2) and K==LIMIT+1-LAST
 * 	otherwise
 * LAST	Number of subintervals actually produced in the subdivision
 *	process.
 */

#include "AIR.h"

double AIR_dqage(double (*f)(const double), const double a, const double b, const double epsabs, const double epsrel, const unsigned int limit, double *abserr, unsigned int *neval, unsigned int *ier, double *alist, double *blist, double *rlist, double *elist, unsigned int *iord, unsigned int *last)

{
	double errsum; 	/* sum of the errors over the subintervals*/
	double resabs;
	double result=0.0;

	double epmach= AIR_d1mach(4); /*machine constant: largest relative spacing*/

	/*test on validity of parameters*/
	*ier=0;
	*neval=0;
	*last=limit;
	*abserr=0.0;
	alist[0]=a;
	blist[0]=b;
	rlist[0]=0.0;
	elist[0]=0.0;
	iord[0]=limit;
	{
		double temp=50.0*epmach;
		if(temp<0.5e-28) temp=0.5e-28;
		
		if(epsabs<=0.0 && epsrel<temp){
			*ier=6;
			return result;
		}
	}
	
	/*First approximation to the integral*/
	{
		double defabs;
		
		AIR_dqk15(f,a,b,&result,abserr,&defabs,&resabs);
		*last=0;
		rlist[0]=result;
		elist[0]=*abserr;
		iord[0]=0;

		/*Test on accuracy*/
		{
			double errbnd=epsrel*fabs(result); /* requested accuracy max(epsabs,epsrel*abs[result]) */
			if(errbnd<epsabs) errbnd=epsabs;
			if(*abserr<=50.0*epmach*defabs && *abserr>errbnd) *ier=2;
			if(limit==1) *ier=1;
			if(*ier!=0||(*abserr<=errbnd && *abserr!=resabs) ||*abserr==0.0){
				*neval=30**neval+15;
				return result;
			}
		}
	}
	{
		/*Initialization*/
		double errmax=*abserr; /*elist[maxerr] */
		unsigned int maxerr=0; /* Pointer to the interval with the largest error estimate*/
		double area=result; /*sum of the integrals over the subintervals*/
		unsigned int nrmax=0;
		unsigned int iroff1=0;
		unsigned int iroff2=0;
		double uflow=AIR_d1mach(1); /* machine constant: smallest positive magnitude*/
		
		errsum=*abserr;

		/*Main do-loop*/
		for(*last=1;*last<limit;(*last)++){
			double error1,error2;
			double area1,area2;
			/*Bisect the subinterval with the largest error estimate*/
			double a1=alist[maxerr];
			double b1=0.5*(alist[maxerr]+blist[maxerr]);
			double a2=b1;
			double b2=blist[maxerr];
			{
				double defab1,defab2;
				
				AIR_dqk15(f,a1,b1,&area1,&error1,&resabs,&defab1);
				AIR_dqk15(f,a2,b2,&area2,&error2,&resabs,&defab2);

				/*Improve previous approximations to integral and error and test for accuracy*/
				(*neval)++;
				{
					double area12=area1+area2;
					double erro12=error1+error2;
					errsum+=erro12-errmax;
					area+=area12-rlist[maxerr];
					if(defab1!=error1 && defab2!=error2){
						if(fabs(rlist[maxerr]-area12)<=0.1e-4*fabs(area12) && erro12>=.99*errmax) iroff1++;
						if(*last>9 && erro12>errmax) iroff2++;
					}
				}
			}
			rlist[maxerr]=area1;
			rlist[*last]=area2;
			{
				double errbnd=epsrel*fabs(area); /* requested accuracy max(epsabs,epsrel*abs[result]) */
				if(errbnd<epsabs) errbnd=epsabs;
				if(errsum>errbnd){

					/*Test for roundoff error and eventually set error flag*/
					if(iroff1>=6||iroff2>=20) *ier=2;

					/*Set error flag in the case that the number of subintervals equals limit*/
					if(*last==limit-1) *ier=1;

					/*Set error flag in the case of bad integrand behaviour at a point in the integration range*/
					{
						double temp=fabs(a1);
			 			if(temp<fabs(b2)) temp=fabs(b2);
						if(temp<=(1.0+100.0*epmach)*(fabs(a2)+1000.0*uflow)) *ier=3;
					}
				}
				/*Append the newly created intervals to the list*/

				if(error2<=error1){
					alist[*last]=a2;
					blist[maxerr]=b1;
					blist[*last]=b2;
					elist[maxerr]=error1;
					elist[*last]=error2;
				}
				else{
					alist[maxerr]=a2;
					alist[*last]=a1;
					blist[*last]=b1;
					rlist[maxerr]=area2;
					rlist[*last]=area1;
					elist[maxerr]=error2;
					elist[*last]=error1;
				}
				
				/*call subroutine dqpsrt to maintain the descending ordering*/
				/*of the list of error estimates and select the subinterval*/
				/*with the largest error estimate (to be bisected next)*/

				AIR_dqpsrt(limit,*last,&maxerr,&errmax,elist,iord,&nrmax);
				if(*ier!=0||errsum<=errbnd) break;
			}
		}
	}
	/*Compute final result*/
	result=0.0;
	{
		unsigned int k;
		
		for(k=0;k<=*last;k++){
			result+=rlist[k];
		}
	}
	*abserr=errsum;
	*neval=30**neval+15;
	return result;
}
