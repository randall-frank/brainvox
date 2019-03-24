/* Copyright 2000-2001 Roger P. Woods, M.D. */
/* Modified 5/22/01 */

/*
 * void sylv()
 *
 * Solves for x in the complex equation ax+xb=c
 *
 * a and b are square and upper triangular
 *
 * x and c are rectangular and full, and may share storage
 *
 */

#include "AIR.h"

AIR_Error AIR_sylv(const unsigned int n, double **ar, double **ai, const unsigned int m, double **br, double **bi, double **cr, double **ci, double **xr, double **xi)

{
	unsigned int j;
	
	for(j=0; j<m; j++){
	
		unsigned int i;
		
		for(i=n ;i--!=0;){
		
			/* temp=c[j][i] */
		
			double tempr=cr[j][i];
			double tempi=ci[j][i];
			
			if(i<n-1){
				
				unsigned int k;
				
				for(k=i+1; k<n; k++){
				
					/* temp-=a[k][i]*x[j][k]; */
					
					tempr-=(ar[k][i]*xr[j][k]-ai[k][i]*xi[j][k]);
					tempi-=(ar[k][i]*xi[j][k]+ai[k][i]*xr[j][k]);

				}
			}
			if(j!=0){
				
				unsigned int k;
				
				for(k=0; k<j; k++){
				
					/* temp-=x[k][i]*b[j][k]; */
					
					tempr-=(xr[k][i]*br[j][k]-xi[k][i]*bi[j][k]);
					tempi-=(xr[k][i]*bi[j][k]+xi[k][i]*br[j][k]);

				}
			}

			/* x[j][i]=temp/(a[i][i]+b[j][j]); */
			{
				AIR_Error errcode=AIR_cdiver(tempr,tempi,ar[i][i]+br[j][j],ai[i][i]+bi[j][j],&xr[j][i],&xi[j][i]);				
				if(errcode!=0) return errcode;
			}
		}
	}
	return 0;
}
