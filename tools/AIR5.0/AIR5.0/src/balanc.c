/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/17/01 */

/*
 *
 * This subroutine balances a REAL matrix and isolates eigenvalues
 * whenever possible.
 *
 * This subroutine was ported from the Fortran subroutine with the
 * same name in the SLATEK (EISPACK) library. The Fortran version
 * is in the public domain. I have altered the routine such that
 * array indexing starts with zero instead of one. I have changed
 * from float representation to double representation.
 *
 *
 * The routine should otherwise be a direct port of the Fortran routine.
 *
 * The Fortran routine itself is a translation of the ALGOL procedure
 * BALANCE, NUM.MATH. 13,293-304(1969) by Parlett and Reinsch.
 * HANDBOOK FOR AUTO. COMP.,Vol.II-LINEAR ALGEBRA, 315-326 (1971), and
 * was written by Smith, B.T., et.al.
 *
 * On INPUT
 *	N is the order of the matrix A. N is an integer variable.
 *	A contains the input matrix to be balanced. A is a two-
 *	  dimensional DOUBLE array, dimensioned A[N][N].
 *
 * On OUTPUT
 *	A contains the balanced matrix.
 *	LOW and HIGH are two INTEGER pointers such that A[j][i] is
 *	  equal to zero if
 *		(1) i is greater than j and
 *		(2) j=0,...,*LOW-1 or i=*igh+1,...,N-1.
 *	SCALE contains information determining the permutations and
 *	  scaling factors used. SCALE is a one-dimensional DOUBLE
 *	  array, dimensioned SCALE[N].
 *
 * REFERENCES: B.T.Smith, J.M.Boyle, J.J.Dongarra, B.S.Garbow,Y.Ikebe,
 *  V.C.Klema and C.B.Moler, Matrix Eigensystem Routines--EISPACK
 *  Guide, Springer-Verlag, 1976.
 */


#include "AIR.h"
#include <float.h>

static void exc(double **a, double *scale, const unsigned int m, const unsigned int j, const unsigned int k, const unsigned int l, const unsigned int n)

/* m is the output diagonal location of eigenvalue */
/* j is the input diagonal location of eigenvalue */

{
	scale[m]=(double)j;
	if(j!=m){
		{
			unsigned int i;
	
			for(i=0;i<=l;i++){
				double f=a[j][i];
				a[j][i]=a[m][i];
				a[m][i]=f;
			}/*30*/
		}
		{
			unsigned int i;
			
			for(i=k;i<n;i++){
				double f=a[i][j];
				a[i][j]=a[i][m];
				a[i][m]=f;
			}
		}
	}/*50*/
}

void AIR_balanc(const unsigned int n, double **a, unsigned int *low, unsigned int *igh, double *scale)

{
	if(n==0) return;
	{
		unsigned int b2=(unsigned int)FLT_RADIX*FLT_RADIX;
		unsigned int k=0;
		unsigned int l=n-1;
		/*100*/
		/*Search 2nd index for preexisting eigenvalues*/
		{
			unsigned int j;
			
			for(j=l+1;(j--)!=0;){ /* j>=0 */
				{
					AIR_Boolean flag=FALSE;
					{
						unsigned int i;
						
						for(i=0;i<=l;i++){
							if(i!=j && a[i][j]!=0.0){
								/*Non-zero off-diagonal, so no eigenvector at this j*/
								flag=TRUE;
								break;
							}
						}/*110*/
					}
					if(flag) continue; /*On to the next j*/
				}
				/*All zeros off-diagonal, so trade it to the end*/
				{
					unsigned int m=l;
					exc(a,scale,m,j,k,l,n);
				}
				if(l!=0) l--;
				else{
					/*280*/
					/*The entire matrix was diagonalized*/
					*low=k;
					*igh=l;
					return;
				}
			}/*120*/
		}
	
		/*Search 1st index for preexisting eigenvalues*/
		{
			unsigned int j;
			
			for(j=k;j<=l;j++){
				{
					AIR_Boolean flag=0;
					unsigned int i;
					for(i=k;i<=l;i++){
						if(i!=j && a[j][i]!=0.0){
							/*Non-zero off-diagonal, so no eigenvector at this j*/
							flag=TRUE;
							break;
						}
					}/*150*/
					if(flag) continue; /*On to the next j*/
				}
				/*All zeros off-diagonal, so trade it to the beginning*/
				{
					unsigned int m=k;
					exc(a,scale,m,j,k,l,n);
				}
				k++;
			}/*170*/
		}
	
		/*Now balance the submatrix in rows k to l*/
		{
			unsigned int i;
			
			for(i=k;i<=l;i++){
				scale[i]=1;
			}/*180*/
		}
		/*Iterative loop for norm reduction*/
		{
			AIR_Boolean noconv;
			do{
				unsigned int i;
				noconv=FALSE;
				for(i=k;i<=l;i++){
					double c=0.0;
					double r=0.0;
					{
						unsigned int j;
						
						for(j=k;j<=l;j++){
							if(j!=i){
								c+=fabs(a[i][j]);
								r+=fabs(a[j][i]);
							}
						}/*200*/
					}
					/*Guard against zero c or r due to underflow*/
					if(c!=0.0 && r!=0.0){
		
						double g=r/FLT_RADIX;
						double f=1.0;
						double s=c+r;
		
						/*210*/
						while(c<g){
							f*=FLT_RADIX;
							c*=b2;
						}/*220*/
						g=r*FLT_RADIX;
						/*230*/
						while(c>=g){
							f/=FLT_RADIX;
							c/=b2;
						}/*240*/
						/*Now balance*/
						if((c+r)/f < 0.95*s){
							g=1.0/f;
							scale[i]*=f;
							noconv=TRUE;
							{
								unsigned int j;
								
								for(j=k;j<n;j++){
									a[j][i]*=g;
								}/*250*/
							}
							{
								unsigned int j;
								for(j=0;j<=l;j++){
									a[i][j]*=f;
								}/*260*/
							}
						}
					}
				}/*270*/
			}while(noconv);
		}
		*low=k;
		*igh=l;
	}
}
