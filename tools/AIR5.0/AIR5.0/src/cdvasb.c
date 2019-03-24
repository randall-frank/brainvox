/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/10/01 */

/*
 * This routine will solve X=A/B
 *  by solving X'=B'\A'
 *  which is to say: B'X'=A'
 *
 * All of the matrices and vectors are complex.
 *
 * On input
 * 	A must have m rows and n columns (overwritten by X on output)
 *	B must have n rows and n columns (destroyed)
 *	V is a work vector of length n
 *	IPVT is a work vector of length n
 *
 * On output
 *	A contains the desired output
 *	*errcode=0 if no errors occured, error code otherwise
 *
 * A check is made for the special case that A==B, in which case an
 *  identity matrix is returned.				
 */

#include "AIR.h"

AIR_Error AIR_cdvasb(const unsigned int n, const unsigned int m, double **ar, double **ai, double **br, double **bi, double *vr, double *vi, unsigned int *ipvt)

{
	/* if m==0, no error will return */
	/* if n==0 && m!=0, AIR_SINGULAR_CDVASB_ERROR will return due to cgefa */

	if(m==n){

		/* See if the matrices a and b are identical */
		AIR_Boolean different=FALSE;
		{
			unsigned int j;

			for(j=0;j<n;j++){
				{
					unsigned int i;

					for(i=0;i<n;i++){
						if(ar[j][i]!=br[j][i]||ai[j][i]!=bi[j][i]){
							different=TRUE;
							break;
						}
					}
				}
				if(different) break;
			}
		}
		if(!different){

			/* Return a unitary matrix if they are identical */

			unsigned int j;

			for(j=0;j<n;j++){

				unsigned int i;

				for(i=0;i<n;i++){
					if(i==j) ar[j][i]=1.0;
					else ar[j][i]=0.0;
					ai[j][i]=0.0;
				}
			}
			return 0;
		}
	}

	/* Factor b using Gaussian elimination */
	
	if(AIR_cgefa(br,bi,n,ipvt)!=n){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("singular matrix error in attempted matrix division by cdvasb\n");
		return AIR_SINGULAR_CDVASB_ERROR;
	}

	/* Compute the matrix division result */
	{
		unsigned int j;

		for(j=0;j<m;j++){

			/*Copy row m of a into v, conjugating as well*/
			{
				unsigned int i;

				for(i=0;i<n;i++){
					vr[i]=ar[i][j];
					vi[i]=-ai[i][j];
				}
			}

			/*Compute row m of output using cgesl in traspose mode*/
			AIR_cgesl(br,bi,n,ipvt,vr,vi,(AIR_Boolean)1);

			/*Copy this back into a, conjugating once again*/
			{
				unsigned int i;

				for(i=0;i<n;i++){
					ar[i][j]=vr[i];
					ai[i][j]=-vi[i];
				}
			}
		}
	}
	return 0;
}
