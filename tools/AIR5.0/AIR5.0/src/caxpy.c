/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/2/01 */

/*
 * void caxpy()
 *
 * This routine will compute a complex contant times a complex vector
 * plus a complex vector
 * y=kx+y
 */

#include "AIR.h"

void AIR_caxpy(const unsigned int n, const double kr, const double ki, const double *xr, const double *xi, double *yr, double *yi)

{
	/* Does nothing if n==0 */
	unsigned int i;

	for(i=0;i<n;i++,xr++,xi++,yr++,yi++){
		*yr+=*xr*kr-*xi*ki;
		*yi+=*xr*ki+*xi*kr;
	}
}
