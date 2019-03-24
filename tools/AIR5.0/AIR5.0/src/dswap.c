/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/1/01 */

/*
 * void dswap()
 *
 * This routine will swap two double precision vectors
 */

#include "AIR.h"

void AIR_dswap(const unsigned int n, double *x, double *y)

{
	unsigned int i;

	for(i=0;i<n;i++){
		double temp=*x;
		*x++=*y;
		*y++=temp;
	}
}
