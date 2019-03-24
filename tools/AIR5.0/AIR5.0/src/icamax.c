/* Copyright 1995-2002 Roger P. Woods, M.D. */
/* Modified 1/16/02 */

/*
 *
 * This routine will find first occurence of element with largest
 *  sum of magnitudes of real and imaginary parts.
 *
 * Returns element of interest (zero if n==0)
 */

#include "AIR.h"

unsigned int AIR_icamax(const unsigned int n, const double *xr, const double *xi)

{
	if(n==0) return UINT_MAX;
	{
		unsigned int max=0;
		{
			double temp=-1;
			unsigned int i;
	
			for(i=0;i<n;i++,xr++,xi++){
	
				double temp2=fabs(*xr)+fabs(*xi);
	
				if(temp2>temp){
					temp=temp2;
					max=i;
				}
			}
		}
		return max;
	}
}
