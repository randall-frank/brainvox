/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/2/01 */

/*
 * void daxpy()
 *
 * This routine will compute a contant times a vector plus a vector
 * y=kx+y for vector of length n
 */

#include "AIR.h"

void AIR_daxpy(const unsigned int n, const double k, const double *x, double *y)

{
	/* Does nothing if n==0 */
	unsigned int i;

	for(i=0;i<n;i++){
		*y+++=k**x++;
	}
}
