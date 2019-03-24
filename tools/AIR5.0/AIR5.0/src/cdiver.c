/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/8/01 */

/*
 * This routine computes ther complex quotient of two complex numbers
 *
 * Returns 0 if successful, error code otherwise
 *									
 */

#include "AIR.h"

AIR_Error AIR_cdiver(double ar, double ai, double br, double bi, double *cr, double *ci)

{
	double	s;

	if(br==0.0 && bi==0.0){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("WARNING: Division by zero in function cdiver()\n");
		return AIR_COMPLEX_ZERO_DIVIDE_ERROR;
	}
	
	s=fabs(br)+fabs(bi);
	ar/=s;
	ai/=s;
	br/=s;
	bi/=s;

	s=br*br+bi*bi;

	*cr=(ar*br+ai*bi)/s;
	*ci=(ai*br-ar*bi)/s;

	return 0;

}
