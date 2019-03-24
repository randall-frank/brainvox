/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/10/01 */


/* void cgesl()
 *
 * This routine will solve A*X=B or CTRANS(A)*X=B  using factors
 *	computed by CGECO or CGEFA
 *
 * On entry
 *	ar,ai	the output from CGECO or CGEFA
 *	n	the order of the matrix A
 *	*ipvt	the pivot vector from CGECO or CGEFA
 *	br,bi	the right hand side vector (replaced by soln vector)
 *	transpose
 *		=0		to solve A*X=B,
 *		=nonzero	to solve CTRANS(A)*X=B where
 *				CTRANS(A) is the conjugate transpose
 *
 * On return
 *	br,bi	the solution vector x
 *
 * Error Condition
 *
 *	A division by zero will occur if the input factor contains a
 *	zero on the diagonal. Technically this indicates singularity
 *	but it is often caused by improper arguments. It will not
 *	occur if the subroutines are called correctly and if CGEFA
 *	has set INFO==N
 *
 * To compute INVERSE(A)*C where C is a matrix with P columns
 *
 *	if(CGEFA(AR,AI,N,IPVT)!=N) { handle singular matrix }
 *	else{
 *		for(j=0;j<p;j++){
 *			CGESL(AR,AI,N,IPVT,CR[j],CI[j],0)
 *		}
 *	}									
 */

#include "AIR.h"

void AIR_cgesl(double **ar, double **ai, const unsigned int n, const unsigned int *ipvt, double *br, double *bi, const AIR_Boolean transpose)

/* ar and ai are the real and imaginary outputs from cgeco or cgefa */
/* the result is placed in b */

{
	if(n==0) return;
	if(!transpose){
		/*  Solve A*X=B */
		/*  First solve l*y=b  */

		{
			unsigned int k;
			
			for(k=0;k<n-1;k++){
				unsigned int l=ipvt[k];
		
				double tr=br[l];
				double ti=bi[l];
	
				if(l!=k){
					br[l]=br[k];
					bi[l]=bi[k];
	
					br[k]=tr;
					bi[k]=ti;
	
				}
				AIR_caxpy(n-k-1,tr,ti,&ar[k][k+1],&ai[k][k+1],&br[k+1],&bi[k+1]);
			}
		}

	
		/*  Now solve U*x=y  */
		{
			unsigned int k;
			
			for(k=n-1;k!=0;k--){
	
				/* Avoid division by zero by checking cgefa or cgeco output */
				(void)AIR_cdiver(br[k],bi[k],ar[k][k],ai[k][k],&br[k],&bi[k]);
				{
					double tr=-br[k];
					double ti=-bi[k];
		
					AIR_caxpy(k,tr,ti,ar[k],ai[k],br,bi);
				}
			}
			/* Unlike Fortran version, handle k==0 seperately */
			(void)AIR_cdiver(br[0],bi[0],ar[0][0],ai[0][0],&br[0],&bi[0]);
		}
	}
	else{
		/*  Solve CTRANS(A)*X=B */
		/*  First solve CTRANS(U)*Y=B */
		(void)AIR_cdiver(br[0],bi[0],ar[0][0],-ai[0][0],&br[0],&bi[0]);
		{
			unsigned int k;
			
			for(k=1;k<n;k++){
			
				double tr,ti;
	
				AIR_cdotc(k,ar[k],ai[k],br,bi,&tr,&ti);
	
				/* Avoid division by zero by checking cgefa or cgeco output */
				(void)AIR_cdiver(br[k]-tr,bi[k]-ti,ar[k][k],-ai[k][k],&br[k],&bi[k]);
			}
		}
		/*  Now solve CTRANS(L)*X=Y */
		if(n>1){
			unsigned int k;
			
			for(k=n-1;(k--)!=0;){
			
				double tr, ti;
	
				AIR_cdotc(n-k-1,&ar[k][k+1],&ai[k][k+1],&br[k+1],&bi[k+1],&tr,&ti);
	
				br[k]+=tr;
				bi[k]+=ti;
				{
					unsigned int l=ipvt[k];
					if(l!=k){
						tr=br[l];
						ti=bi[l];
		
						br[l]=br[k];
						bi[l]=bi[k];
		
						br[k]=tr;
						bi[k]=ti;
					}
				}
			}
		}
	}
}
