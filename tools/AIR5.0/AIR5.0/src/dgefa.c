/* Copyright 1995-2002 Roger P. Woods, M.D. */
/* Modified 1/16/02 */

/*
 * unsigned int dgefa()
 *
 * This routine will factor a double precision matrix by Gaussian
 *  elimination.
 *
 * On entry
 *	A	the matrix to be factored
 *	N	the order of the matrix A
 *
 * On return
 *	A	an upper triangular matrix and the multipliers
 *		which were used to obtain it.
 *		The factorization can be written A=L*U where
 *		L is a product of permutation and unit lower
 *		triangular matrices and U is upper triangular.
 *
 * 	*IPVT	an integer vector of pivot indices
 * Returns
 *		N normally
 *		K<N if U(K,K)==0.0. This is not an error
 *			condition for this subroutine, but it does
 *			indicate that DGESL or DGEDI will divide by
 *			zero if called.
 *		UINT_MAX if N==0
 */

#include "AIR.h"

unsigned int AIR_dgefa(double **a, const unsigned int n, unsigned int *ipvt)

{
	if(n==0) return UINT_MAX;
	{
		unsigned int info=n;	
		unsigned int k;

		for(k=0;k<n-1;k++){

			/*Find l=pivot index*/

			unsigned int l=AIR_idamax(n-k,&a[k][k])+k;
			ipvt[k]=l;

			/*Zero pivot implies this column already triangularized*/
			if(a[k][l]!=0.0){

				/*Interchange if necessary*/
				if(l!=k){
					double t=a[k][l];
					a[k][l]=a[k][k];
					a[k][k]=t;
				}

				{
					/*Compute multipliers*/
					double t=-1.0/a[k][k];
					AIR_dscal(n-k-1,t,&a[k][k+1]);
				}

				/*Row elimination with column indexing*/
				{
					unsigned int j;
					
					for(j=k+1;j<n;j++){
					
						double t=a[j][l];
						if(l!=k){
							a[j][l]=a[j][k];
							a[j][k]=t;
						}
						AIR_daxpy(n-k-1,t,&a[k][k+1],&a[j][k+1]);
					}
				}

			}
			else info=k;
		}
		ipvt[k]=k;
		if(a[k][k]==0.0) info=k;
		return info;
	}
}
