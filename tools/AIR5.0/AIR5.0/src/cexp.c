/* Copyright 1995-99 Roger P. Woods, M.D. */
/* Modified 11/17/99 */

/*
 * This routine computes (c+di)=exp(a+bi)
 *  where i=sqrt(-1)
 */

#include "AIR.h"

void AIR_cexp(const double a, const double b, double *c, double *d)

{
	double	r=exp(a);

	*c=r*cos(b);
	*d=r*sin(b);

	return;
}
