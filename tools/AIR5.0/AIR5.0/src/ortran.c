/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/30/01 */

/*
 * void ortran()
 *
 * This subroutine accumulates the orthogonal similarity
 * tranformations used in the reduction of a REAL GENERAL matrix to
 * upper Hessenberg form by ORTHES.
 *
 * This routine is ported from the Fortran routine in the SLATEC
 * (EISPACK) library with the same name. I have converted array
 * indices to zero offset and have converted from float to double
 * representation. The port is otherwise fairly literal.
 *
 * The Fortran version of this routine is in the public domain and
 * was itself a translation of an ALGOL procedure ORTRANS, NUM.MATH.
 * 16, 181-204(1970) by Peters and Wilkinson. HANDBOOK FOR AUTO.COMP.,
 * Vol.II--LINEAR ALGEBRA, 372-395(1971).
 *
 * On INPUT
 *	N is the order of the matrix A. N is an INTEGER variable.
 *	LOW and IGH are two INTEGER variables determined by the
 *	  balancing subroutine BALANC. If BALANC has not been used, set
 *	  LOW=0 and IGH=N-1.
 *	A contains some information about the orthogonal trans-
 *	  formations used in the reduction to Hessenberg form by
 *	  ORTHES in its strict lower triangle.  A
 *	  is a two-dimensional DOUBLE array, dimensioned A[IGH][IGH]
 *	ORT contains further information about the orthogonal trans-
 *	  formations used in the reduction by ORTHES. Only elements
 *	  LOW through IGH are used. ORT is a one-dimensional DOUBLE
 *	  array, dimensioned ORT[IGH].
 *
 * On OUTPUT
 *	Z contains the transformation matrix produced in the reduction
 *	  by ORTHES. Z is a two-dimensional DOUBLE array, dimensioned
 *	  Z[N][N].
 *	ORT has been used for temporary storage and is not restored
 *
 * REFERENCES: B.T.Smith, J.M.Boyle, J.J.Dongarra,B.S.Garbow,Y.Ikebe,
 *  V.C.Klema and C.B.Moler, Matrix Eigensystem Routines--EISPACK Guide,
 *  Springer-Verlag, 1976.
 *
 */

#include "AIR.h"

void AIR_ortran(const unsigned int n, const unsigned int low, const unsigned int igh, double **a, double *ort, double **z)

{
	{
		unsigned int i;
		
		for(i=0;i<n;i++){
			{
				unsigned int j;
				
				for(j=0;j<n;j++){
					z[j][i]=0.0;
				}
			}
			z[i][i]=1.0;
		}
	}
	if(low>=igh) return;
	/* now know that igh>0 */
	{
		unsigned int mp;
		
		for(mp=igh-1;mp!=low;mp--){
		
			if(a[mp-1][mp]==0.0) continue;
			{
				unsigned int i;
				
				for(i=mp+1;i<=igh;i++){
					ort[i]=a[mp-1][i];
				}
			}
			{
				unsigned int j;
				
				for(j=mp;j<=igh;j++){
					double g=0.0;
					{
						unsigned int i;
						
						for(i=mp;i<=igh;i++){
							g+=ort[i]*z[j][i];
						}
					}
					g=(g/ort[mp])/a[mp-1][mp];
					{
						unsigned int i;
						
						for(i=mp;i<=igh;i++){
							z[j][i]+=g*ort[i];
						}
					}
				}
			}
		}
	}
}
