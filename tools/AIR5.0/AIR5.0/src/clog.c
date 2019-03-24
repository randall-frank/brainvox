/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/27/01 */

/*
 * This routine computes (c+di)=log(a+bi)
 *  where i=sqrt(-1)
 * Returns 0, if successful, error code otherwise
 */

#include "AIR.h"

AIR_Error AIR_clog(double a, double b, double *c, double *d)

{
	/* Deal with undefined case*/
	if(b==0.0){
		if(a==0.0) return AIR_COMPLEX_LOG_UNDEFINED_ERROR;
		if(a==1.0){
			*c=0.0;
			*d=0.0;
			return 0;
		}
	}
	{
		/*Rescale complex plane by factor of fabs(a)+fabs(b)*/
		double scale=fabs(a)+fabs(b);
		a/=scale;
		b/=scale;

		/*Convert to polar coordinates*/
		{
			double r=sqrt(a*a+b*b);
			double theta=acos(a/r);
			if(b<0.0) theta*=-1;

			/*Compute log*/
			*c=log(r)+log(scale);
			*d=theta;
		}

		return 0;
	}
}
