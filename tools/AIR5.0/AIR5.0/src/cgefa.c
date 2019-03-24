/* Copyright 1995-2002 Roger P. Woods, M.D. */
/* Modified 1/16/02 */


/* unsigned int cgefa()
 *
 * This routine will factor a double precision matrix by Gaussian
 *  elimination.
 *
 * On entry
 *	AR	the matrix to be factored, real components
 *	AI	the matrix to be factored, imaginary components
 *	N	the order of the matrix A
 *
 * On return
 *	AR,AI	an upper triangular matrix and the multipliers
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

unsigned int AIR_cgefa(double **ar, double **ai, const unsigned int n, unsigned int *ipvt)

{
	if(n==0) return UINT_MAX;
	{
		unsigned int info=n;
		unsigned int k;

		for(k=0;k<n-1;k++){
		
			/*Find l=pivot index*/
		
			unsigned int l=AIR_icamax(n-k,&ar[k][k],&ai[k][k])+k;
			ipvt[k]=l;

			/*Zero pivot implies this column already triangularized*/
			if(ar[k][l]!=0.0||ai[k][l]!=0.0){

				/*Interchange if necessary*/
				if(l!=k){
					double tr=ar[k][l];
					double ti=ai[k][l];

					ar[k][l]=ar[k][k];
					ai[k][l]=ai[k][k];

					ar[k][k]=tr;
					ai[k][k]=ti;
				}
				{
					double tr,ti;
					
					/*Compute multipliers*/
					/* t=-(1.0,0.0)/a[k][k];*/

					/* Routine protects against division by zero */
					(void)AIR_cdiver(-1.0,0.0,ar[k][k],ai[k][k],&tr,&ti);

					AIR_cscal(n-k-1,tr,ti,&ar[k][k+1],&ai[k][k+1]);
				}

				/*Row elimination with column indexing*/
				{
					unsigned int j;
					
					for(j=k+1;j<n;j++){

						double tr=ar[j][l];
						double ti=ai[j][l];

						if(l!=k){
							ar[j][l]=ar[j][k];
							ai[j][l]=ai[j][k];

							ar[j][k]=tr;
							ai[j][k]=ti;

						}
						AIR_caxpy(n-k-1,tr,ti,&ar[k][k+1],&ai[k][k+1],&ar[j][k+1],&ai[j][k+1]);
					}
				}
			}
			else info=k;
		}
		ipvt[k]=k;
		if(ar[k][k]==0.0 && ai[k][k]==0.0) info=k;
		return info;
	}
}
