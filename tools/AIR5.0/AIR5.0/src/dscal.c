/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/2/01 */

/*
 * void dscal()
 *
 * This routine multiplies a vector by a constant
 * x=k*x for vector of length n
 */

#include "AIR.h"

void AIR_dscal(const unsigned int n, const double k, double *x)

{
	/* Does nothing if n==0 */
	unsigned int i;

	for(i=0;i<n;i++){
		*x++*=k;
	}
}
