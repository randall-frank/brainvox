/* Copyright 1995-99 Roger P. Woods, M.D. */
/* Modified 11/23/99 */

/* void rg()
 *
 * This routine orchestrates computation of eigenvalues and eigenvectors
 *   of a GENERAL matrix
 *
 * On return:
 *
 *	IERR is an INTEGER flag set to
 *	  -1 for normal return
 *	  J if the Jth eigenvalue has not been determined after a total
 *	    of 30*N iterations. The eigenvalues should be correct for
 *	    indices IERR+1,IERR+2,...,N-1, but no eigenvectors are
 *	    computed.					
 */

#include "AIR.h"

void AIR_rg(const unsigned  int n, double **a, double *wr, double *wi, double **z, double *iv1, double *fv1, unsigned int *ierr)

{
	unsigned int is1,is2;

	AIR_balanc(n,a,&is1,&is2,fv1);

	AIR_orthes(n,is1,is2,a,iv1);

 	AIR_ortran(n,is1,is2,a,iv1,z);

	AIR_hqr2(n,is1,is2,a,wr,wi,z,ierr);

	if(*ierr==n){
		AIR_balbak(n,is1,is2,fv1,n,z);
	}
}
