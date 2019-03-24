/* Copyright 1995-2002 Roger P. Woods, M.D. */
/* Modified 1/16/02 */

/*
 *
 * This routine will find first occurence of element with largest
 *  absolute value
 *
 * Returns element of interest (0 if N==0)
 */

#include "AIR.h"

unsigned int AIR_idamax(const unsigned int n, const double *x)

{
	if(n==0) return UINT_MAX;
	{
		unsigned int max=0;
		{
			double temp=-1;
			unsigned int i;
	
			for(i=0;i<n;i++,x++){
	
				double temp2=fabs(*x);
	
				if(temp2>temp){
					temp=temp2;
					max=i;
				}
			}
		}
		return max;
	}
}
