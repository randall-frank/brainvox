#include "voxel.h"
#include "vl_util.h"
#include <stdio.h>
/* this routine returns the current matrix and its inverse in a form
 acceptable to all calling languages.  */

void	vl_getmatrix_(VLFLOAT32 *mat, VLFLOAT32 *inv, vset *set)
{
	VLFLOAT64 temp[3][3];
	VLINT32	i,j,k;
/* copy the xform into the temp array */
        for(i=0;i<3;i++) {
                for(j=0;j<3;j++) {
                        temp[i][j] = set->xform[i][j];
                }
        }
#ifdef DEBUG
	printf("Original Matrix:\n");
	printf(" %fl %fl %fl \n",temp[0][0],temp[0][1],temp[0][2]);
	printf(" %fl %fl %fl \n",temp[1][0],temp[1][1],temp[1][2]);
	printf(" %fl %fl %fl \n",temp[2][0],temp[2][1],temp[2][2]);
#endif
/* copy the temp array to mat */
	k = 0;
        for(i=0;i<3;i++) {
                for(j=0;j<3;j++) {
                        mat[k++] = (VLFLOAT32)temp[i][j];
                }
        }
#ifdef DEBUG
	printf("Float Matrix:\n");
	printf(" %fl %fl %fl \n",mat[0],mat[1],mat[2]);            
	printf(" %fl %fl %fl \n",mat[3],mat[4],mat[5]);            
	printf(" %fl %fl %fl \n",mat[6],mat[7],mat[8]);            
#endif
/* invert the temp array */
	(void) invert3x3(temp);
/* copy the temp array to inv */
	k = 0;
        for(i=0;i<3;i++) {
                for(j=0;j<3;j++) {
                        inv[k++] = (VLFLOAT32)temp[i][j];
                }
        }
#ifdef DEBUG
	printf("Inverse Matrix:\n");
	printf(" %fl %fl %fl \n",inv[0],inv[1],inv[2]);            
	printf(" %fl %fl %fl \n",inv[3],inv[4],inv[5]);            
	printf(" %fl %fl %fl \n",inv[6],inv[7],inv[8]);            
#endif
/* and we're done... */
	return;

}
/* inversion routines */
void	invert3x3(VLFLOAT64 A[3][3])
{
/*
	a b c a b
	d e f d e
	g h i g h
	a b c a b
	d e f d e
*/
	VLFLOAT64 	a,b,c,d,e,f,g,h,i;
	VLFLOAT64  Da,Db,Dc,Dd,De,Df,Dg,Dh,Di;
	VLFLOAT64	detA;
	VLINT32	k,j;
/* assign temp vars */
	a = A[0][0];
	b = A[0][1];
	c = A[0][2];
	d = A[1][0];
	e = A[1][1];
	f = A[1][2];
	g = A[2][0];
	h = A[2][1];
	i = A[2][2];
/* calc determinat of 3x3 A */
	detA = (a * e * i) + (d * h * c) + (b * f * g);
	detA = detA - (c * e * g) - (a * f * h) - (b * d * i);
/* check for singular matrix */
	if (detA == 0.0) {
		vl_puts("Warning, singular matrix detected.\n");
		return;
	}
/* calc sub (2x2) determinats if Dx row and column is eliminated */
/* and mult by (-1)^i+j */
	Da =  ((e * i) - (f * h));
	Db = ((f * g) - (d * i));
	Dc =  ((d * h) - (g * e));
	Dd = ((h * c) - (b * i));
	De =  ((i * a) - (c * g));
	Df = ((g * b) - (a * h));
	Dg =  ((b * f) - (e * c));
	Dh = ((c * d) - (a * f));
	Di =  ((a * e) - (b * d));
/* build the transpose matrix from the sub determinats */
	A[0][0] = Da;
	A[1][0] = Db;
	A[2][0] = Dc;
	A[0][1] = Dd;
	A[1][1] = De;
	A[2][1] = Df;
	A[0][2] = Dg;
	A[1][2] = Dh;
	A[2][2] = Di;
/* devide by detA */    
	for (k=0; k<3; k++) {
		for (j=0; j<3; j++) A[k][j] = A[k][j]/detA;
	};
}
