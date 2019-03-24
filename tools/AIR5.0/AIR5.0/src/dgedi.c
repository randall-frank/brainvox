/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/22/01 */

/*
 * void dgedi()
 *
 * This routine will invert a matrix using factors computed by
 *  DGECO or DGEFA
 *
 * On entry
 *	A	the output from DGECO or DGEFA
 *	N	the order of the matrix A
 *	*IPVT	the pivot vector from DGECO or DGEFA
 *	WORK	work vector, contents destroyed
 *
 * On return
 *	A	inverse of original matrix
 *
 */

#include "AIR.h"

void AIR_dgedi(double **a, const unsigned int n, const unsigned int *ipvt, double *work)

{
	/*Compute inverse(U)*/
	{
		unsigned int k;

		for(k=0;k<n;k++){
			a[k][k]=1.0/a[k][k];
			AIR_dscal(k,-a[k][k],a[k]);
			{
				unsigned int j;

				for(j=k+1;j<n;j++){
					double temp=a[j][k];
					a[j][k]=0.0;
					AIR_daxpy(k+1,temp,a[k],a[j]);
				}
			}
		}
	}

	/*form inverse(U)*inverse(L)*/
	if(n>1){
		unsigned int k;

		for(k=n-1;k--!=0;){
			{
				unsigned int i;

				for(i=k+1;i<n;i++){
					work[i]=a[k][i];
					a[k][i]=0.0;
				}
			}
			{
				unsigned int j;

				for(j=k+1;j<n;j++){
					AIR_daxpy(n,work[j],a[j],a[k]);
				}
			}
			{
				unsigned int l=ipvt[k];
				if(l!=k) AIR_dswap(n,a[k],a[l]);
			}
		}
	}
}
