/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/5/01 */

/*
 * void rgschur()
 *
 * This routine orchestrates computation of the Schur decomposition
 *   of a GENERAL matrix
 *
 * Note that routines balanc() and balbak() must NOT be used
 *
 * On return, values in a that are theoretically zero may actually be tiny values
 * Consequently, wi should be used to determine whether a subdiagonal element is theoretically zero
 *
 * Normal return in *ierr is -1
 */

#include "AIR.h"

void AIR_rgschur(const unsigned int n, double **a, double *wr, double *wi, double **z, double *iv1, unsigned int *ierr)

{
	AIR_orthes(n,0,n-1,a,iv1);
 	AIR_ortran(n,0,n-1,a,iv1,z);
	AIR_hqr3(n,0,n-1,a,wr,wi,z,ierr);
}
