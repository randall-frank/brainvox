/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 5/17/01 */

/*
 * This routine is machine dependent. It returns the machine
 * dependent constants needed by SLATEC derived routines
 *
 *
 * Assume (not quite correctly for the Sun due to the hidden
 * bit of significance) that double precision numbers are
 * represented in the T-digit, base-B (2 for Sun) form:
 *	sign(B**E)*((x[1]/B)+...+(x[T]/B**T))
 *
 * Then if EMIN is the smallest exponent of E and EMAX is the
 * largest exponent of E:
 *
 * D1MACH(1)=B**(EMIN-1), the smallest positive magnitude
 * D1MACH(2)=B**EMAX*(1-B**(-T)), the largest magnitude
 * D1MACH(3)=B**(-T), the smallest relative spacing
 * D1MACH(4)=B**(1-T), the largest relative spacing
 * D1MACH(5)=LOG10(B)
 *
 * For the SUN, B=2 and T=53 (52+1 "hidden" bit)
 * EMAX=1023, EMIN=-1023	
 */

#include "AIR.h"
#include <float.h>

double	AIR_d1mach(const unsigned int i)

{
	if(i==1){
		return DBL_MIN;
	}
	else if(i==2){
		return DBL_MAX;
	}
	else if(i==3){
		return DBL_EPSILON/FLT_RADIX;
	}
	else if(i==4){
		return DBL_EPSILON;
	}
	else if(i==5){
		return log10((double)FLT_RADIX);
	}
	else{
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("d1mach: called with value out of range 1-5\n");
		return 0.0;
	}
}
