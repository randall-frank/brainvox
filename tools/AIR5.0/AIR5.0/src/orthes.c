/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/5/01 */

/*
 * void orthes()
 *
 * Given a REAL GENERAL matrix, this subroutine reduces a submatrix
 * situated in rows and columns LOW through IGH to upper Hessenberg
 * form by orthogonal similarity transformations.
 *
 * This routine is a port of the Fortran routine with the same name.
 * I have converted the array indexing to zero offset and redefined
 * the variables as DOUBLE, but it is otherwise an exact port.
 *
 * The Fortran subroutine itself is a translation of the ALGOL procedure
 * ORTHES, NUM.MATH.12, 349-368(1968) by Martin and Wilkinson.
 * HANDBOOK FOR AUTO. COMP., Vol. II--LINEAR ALGEBRA, 339-358(1971).
 * The Fortran version is in the public domain and was written by
 * Smith, B.T., et.al.
 *
 * On INPUT
 *	N is the order of the matrix, A. N is an integer variable.
 *	LOW and IGH are two INTEGER variables determined by the
 *	  balancing subroutine BALANC. If BALANC has not been used,
 *	  set LOW=0 and IGH equal to N-1.
 *
 *	A contains the input matrix. A is a two-dimensional DOUBLE
 *	  array, dimensioned A[N][N].
 *
 * On OUTPUT
 *	A contains the upper Hessenberg matrix. Some information about
 *	  the orthogonal transformations used in the reduction is
 *	  stored in the remaining triangle under the Hessenberg matrix.
 *
 *	ORT contains further information about the orthogonal trans-
 *	  formations used in the reduction. Only elements LOW+1 through
 *	  IGH are used. ORT is a one-dimensional DOUBLE array,
 *	  dimensioned ORT[IGH]
 *
 *
 * REFERENCES: B.T.Smith, J.M.Boyle, J.J.Dongarra, B.S.Garbow, Y.Ikebe,
 *   V.C.Klema and C.B.Moler, Matrix Eigensystem Routines--EISPACK
 *   Guide, Springer-Verlag, 1976.
 *
 */

#include "AIR.h"

void AIR_orthes(const unsigned int n, const unsigned int low, const unsigned int igh, double **a, double *ort)

{
	unsigned int m;

	for(m=low+1;m<igh;m++){
		double scale=0.0;
		ort[m]=0.0;
		/*Scale column (ALGOL TOL then not needed)*/
		{
			unsigned int i;
			
			for(i=m;i<=igh;i++){
				scale+=fabs(a[m-1][i]);
			}/*90*/
		}
		if(scale!=0.0){
			double h=0.0;
			{
				unsigned int i;
				
				for(i=igh;i>=m;i--){	/* m>0 */
					ort[i]=a[m-1][i]/scale;
					h+=ort[i]*ort[i];
				}/*100*/
			}
			{
				double g;
				
				if(ort[m]>0.0) g=-sqrt(h);
				else g=sqrt(h);
				h-=ort[m]*g;
				ort[m]-=g;
				/*Form (i-(u*ut)/h)*a  */
				{
					unsigned int j;
					
					for(j=m;j<n;j++){
						double f=0.0;
						{
							unsigned int i;
							
							for(i=igh;i>=m;i--){	/* m>0 */
								f+=ort[i]*a[j][i];
							}/*110*/
						}
						f/=h;
						{
							unsigned int i;
							
							for(i=m;i<=igh;i++){
								a[j][i]-=f*ort[i];
							}/*120*/
						}
					}/*130*/
				}
				/*Form (i-(u*ut)/h)*a*(i-(u*ut)/h) */
				{
					unsigned int i;
					
					for(i=0;i<=igh;i++){
						double f=0.0;
						{
							unsigned int j;
							
							for(j=igh;j>=m;j--){	/* m>0 */
								f+=ort[j]*a[j][i];
							}/*140*/
						}
						f/=h;
						{	
							unsigned int j;
							
							for(j=m;j<=igh;j++){
								a[j][i]-=f*ort[j];
							}/*150*/
						}
					}/*160*/
				}
				ort[m]*=scale;
				a[m-1][m]=scale*g;
			}
		}
	}/*180*/
}
