/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/2/01 */

/*
 * void cscal()
 *
 * This routine multiplies a complex vector by a complex constant
 * x=k*x for vector of length n
 *
 */

#include "AIR.h"

void AIR_cscal(const unsigned int n, const double kr, const double ki, double *xr, double *xi)

{
	/* Does nothing if n==0 */
	unsigned int i;

	for(i=0;i<n;i++){
			
		double tempr=*xr*kr-*xi*ki;
		double tempi=*xr*ki+*xi*kr;
		
		*xr++=tempr;
		*xi++=tempi;
	}
}
