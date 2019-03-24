/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/2/01 */

/*
 * void cdotc()
 *
 * This routine will compute dot product of two complex vectors
 *	Z=SUM(CONJ(X)*Y)
 *
 * Any prior data *zr, *zi will be overwritten					
 */

#include "AIR.h"

void AIR_cdotc(const unsigned int n, const double *xr, const double *xi, const double *yr, const double *yi, double *zr, double *zi)

{
	/* Returns complex zero in zr, zi if n==0 */
	unsigned int i;

	*zr=0;
	*zi=0;

	for(i=0;i<n;i++,xr++,xi++,yr++,yi++){
		*zr+=*xr**yr+*xi**yi;
		*zi+=-*xi**yr+*xr**yi;
	}
}
