/* Copyright 2000-2001 Roger P. Woods, M.D. */
/* Modified 5/18/01 */

/*
 * void sqrtut()
 *
 * Solves for x in the complex equation x*x=a
 *
 * x and a are square and upper triangular
 *
 * this function can be called using the same storage for a and x
 *
 */

#include "AIR.h"

void AIR_sqrtut(const unsigned int n, double **ar, double **ai, double **xr, double **xi)

{
	/* Compute the sqrt along the diagonal */
	{
		unsigned int i;
		
		for(i=0; i<n; i++){
		
			/* x[i][i]=sqrt(a[i][i]) */
			
			AIR_csqrt(ar[i][i], ai[i][i], &xr[i][i], &xi[i][i]);
		}
	}
	/* Deal with the first upper diagonal of x */
	{
		unsigned int i;
		
		for(i=0; i<n-1; i++){
			unsigned int j=i+1;
			
			/* x[j][i]=a[j][i]/(x[i][i]+x[j][j]) */
			
			AIR_cdiver(ar[j][i], ai[j][i], xr[i][i]+xr[j][j], xi[i][i]+xi[j][j], &xr[j][i], &xi[j][i]);
		}
	}
	/* Deal with any other upper diagonals of x */
	{
		unsigned int d;
		
		for(d=1;d<n-1;d++){
		
			unsigned int i;
			
			for(i=0; i<n-d-1; i++){
			
				unsigned int j=i+d;
				{
					double
						tempr=ar[j][i],
						tempi=ai[j][i];
					{
						unsigned int k;
						
						for(k=i+1; k<=j-1; k++){
						
							tempr-=(xr[k][i]*xr[j][k]-xi[k][i]*xi[j][k]);
							tempi-=(xr[k][i]*xi[j][k]+xi[k][i]*xr[j][k]);
						}
					}
					AIR_cdiver(tempr, tempi, xr[i][i]+xr[j][j], xi[i][i]+xi[j][j], &xr[j][i], &xi[j][i]);
				}
			}
		}
	}	
}
