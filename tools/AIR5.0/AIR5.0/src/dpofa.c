/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/10/01 */
/* With special thanks to Kate Fissell for identifying a bug in the original*/


/* void dpofa()
 *
 * This routine will factor a double precision symmetric positive
 * definite matrix
 *
 * On entry
 *	A	the matrix to be factored. Only the diagonal and
 *		upper triangle are used
 *	N	the order of the matrix A
 *
 * On return
 *	A	an upper triangular matrix R so that A=TRANS(R)*R
 *		where TRANS(R) is the transpose.
 *		The strict lower triangle is unaltered
 *		If *info!=-1, the factorization is not complete
 *
 * Returns:
 *		N if no error
 *		K<N signals an error
 *			The leading minor of order K is not positive
 *			definite				
 */


#include "AIR.h"

unsigned int AIR_dpofa(double **a, const unsigned int n)

{
	unsigned int j;

	for(j=0;j<n;j++){

		double s=0.0;

		{
			unsigned int k;

			for(k=0;k<j;k++){
				double t=a[j][k]-AIR_ddot(k,a[k],a[j]);
				t/=a[k][k];
				a[j][k]=t;
				s+=t*t;
			}
		}
		s=a[j][j]-s;
		if(s<=0.0) return j;
		a[j][j]=sqrt(s);
	}
	return j;
}
