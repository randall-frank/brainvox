/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/17/01 */

/*
 * void balbak()
 *
 * This subroutine forms the eigenvectors of a REAL GENERAL matrix
 * by back transforming those of the corresponding balanced matrix
 * determined by BALANC.
 *
 * This routine is ported from a Fortran routine in the SLATEC (EISPAK)
 * library with the same name. I have changed the data type from float
 * to double and have changed the array indexing to zero offset.
 *
 * The Fortran routine BALBAK is in the public domain and is itself
 * a translation of the ALGOL procedure BALBAK, NUM.MATH. 13,293-304
 * (1969) by Parlett and Reinsch. HANDBOOK FOR AUTO. COMP.,Vol.II--
 * LINEAR ALGEBRA, 315-326(1971).
 *
 * On INPUT
 *	N is the number of components of the vectors in matrix Z.
 *	  N is an INTEGER variable.
 *	LOW and IGH are INTEGER variables determined by BALANC.
 *	SCALE contains information determining the permutations and
 *	  scaling factors used by BALANC. SCALE is a one-dimensional
 *	  DOUBLE array, dimensioned SCALE[N].
 *	M is the number of columns of Z to be back transformed.
 *	  M is an INTEGER variable.
 *	Z contains the real and imaginary parts of the eigenvectors
 *	  to be back transformed it its first M columns. Z is a
 *	  two-dimensional DOUBLE array, dimensioned Z[M][M]
 *
 * On OUTPUT
 *	Z contains the real and imaginary parts of the transformed
 *	  eigenvectors in its first M columns.
 *
 * REFERENCES: B.T.Smith, J.M.Boyle, J.J.Dongarra, B.S.Garbow, Y.Ikebe,
 *   V.C.Klema and C.B.Moler, Matrix Eigensystem Routines--EISPACK
 *   Guide, Springer-Verlag, 1976.
 *
 * left hand eigenvectors are back transformed if s=scale[i] is replaced
 * by s=1.0/scale[i]
 */

#include "AIR.h"

void AIR_balbak(const unsigned int n, const unsigned int low, const unsigned int igh, const double *scale, const unsigned int m, double **z)

{
	if(m!=0){
		if(igh!=low){
			{
				unsigned int i;

				for(i=low;i<=igh;i++){

					double s=scale[i];

					{
						unsigned int j;

						for(j=0;j<m;j++){
							z[j][i]*=s;
						}/*100*/
					}
				}/*110*/
			}
		}/*120*/
		{
			unsigned int ii;

			for(ii=0;ii<n;ii++){
				if(ii<low || ii>igh){

					unsigned int i;

					if(ii<low) i=low-ii-1;
					else i=ii;
					{
						unsigned int k=(unsigned int)scale[i];
						if(k!=i){
	
							unsigned int j;
	
							for(j=0;j<m;j++){
	
								double s=z[j][i];
								z[j][i]=z[j][k];
								z[j][k]=s;
							}/*130*/
						}
					}
				}
			}/*140*/
		}
	}/*200*/
}
