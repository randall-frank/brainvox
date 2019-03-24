/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/10/01 */

/*
 * void dmodposl()
 *
 * This routine will solve a*x=b using factors computed by
 *  MODCHOL
 *
 * On entry
 *	a	the output from DGECO or DGEFA
 *	n	the order of the matrix A
 *	b	the right hand side vector (replaced by soln vector)
 *  ipvt the pivot vector from MODCHOL
 *
 * On return
 *	b	the solution vector x
 *
 */

#include "AIR.h"

void AIR_dmodposl(double **a, const unsigned int n, double *b, const unsigned int *ipvt)

{
	/*  Solve TRANS(R)*y=b  */
	{
		unsigned int k;
		
		for(k=0;k<n;k++){
			double t;
			
			/* Undo pivoting */
			{
				unsigned int l=ipvt[k];
				double t2=b[l];
				if(l!=k){
					b[l]=b[k];
					b[k]=t2;
				}
			}
		
			t=AIR_ddot(k,a[k],b);
			
			b[k]=(b[k]-t)/a[k][k];
		}
	}

	/*Solve R*x=y */
	if(n!=0){
		unsigned int k;
		
		for(k=n;(k--)!=0;){
					
			b[k]/=a[k][k];
			AIR_daxpy(k,-b[k],a[k],b);
			
			/* Undo pivoting */
			{
				unsigned int l=ipvt[k];
				
				if(l!=k){
				
					double t=b[l];
					
					b[l]=b[k];
					b[k]=t;
				}
			}
		}
	}
}
