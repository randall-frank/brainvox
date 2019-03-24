/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/10/01 */

/*
 * void dgesl()
 *
 * This routine will solve a*x=b using factors computed by
 *  DGECO or DGEFA
 *
 * On entry
 *	a	the output from DGECO or DGEFA
 *	n	the order of the matrix A
 *	*ipvt	the pivot vector from DGECO or DGEFA
 *	b	the right hand side vector (replaced by soln vector)
 *	transpose
 *		=0		to solve A*X=B,
 *		=nonzero	to solve TRANS(A)*X=B where
 *				TRANS(A) is the transpose
 *
 * On return
 *	b	the solution vector x
 *
 * Error Condition
 *
 *	A division by zero will occur if the input factor contains a
 *	zero on the diagonal. Technically this indicates singularity
 *	but it is often caused by improper arguments. It will not
 *	occur if the subroutines are called correctly and if DGEFA
 *	has set INFO==-1
 *
 * To compute INVERSE(A)*C where C is a matrix with P columns
 *	if(DGEFA(A,N,IPVT)!=N) { handle singular matrix }
 *  else{
 *		for(j=0;j<p;j++){
 *			DGESL(a,n,ipvt,c[j],0)
 *		}
 *	}
 */

#include "AIR.h"

void AIR_dgesl(double **a, const unsigned int n, const unsigned int *ipvt, double *b, const AIR_Boolean transpose)

{
	if(n==0) return;
	if(!transpose){
		/*  Solve A*X=B */
		/*  First solve l*y=b  */
		{
			unsigned int k;

			for(k=0;k<n-1;k++){
			
				unsigned int l=ipvt[k];
				double t=b[l];
				if(l!=k){
					b[l]=b[k];
					b[k]=t;
				}
				AIR_daxpy(n-k-1,t,&a[k][k+1],&b[k+1]);
			}
		}
	
		/*  Now solve U*x=y  */
		{
			unsigned int k;

			for(k=n-1;k!=0;k--){
				b[k]/=a[k][k];
				AIR_daxpy(k,-b[k],a[k],b);
			}
			b[0]/=a[0][0];	/* Unlike Fortran version, handle zero case seperately */
		}
	}
	else{
		/*  Solve TRANS(A)*X=B */
		/*  First solve TRANS(U)*Y=B */
		b[0]/=a[0][0];
		{
			unsigned int k;

			for(k=1;k<n;k++){
				double t=AIR_ddot(k,a[k],b);
				b[k]=(b[k]-t)/a[k][k];
			}
		}
		
		/*  Now solve TRANS(L)*X=Y */
		if(n>1){
			unsigned int k;

			for(k=n-1;(k--)!=0;){
				b[k]+=AIR_ddot(n-k-1,&a[k][k+1],&b[k+1]);
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
}
