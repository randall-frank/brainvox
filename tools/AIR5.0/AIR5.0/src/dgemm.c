/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/23/01 */

/*
 * DGEMM performs one of the matrix-matrix operations
 *	C=alpha*op(A)*op(B)+beta*C
 * where op(X) is one of
 *	op(X)=X or op(X)=X'
 * alpha and beta are scalars, and A, B, and C are matrices, with
 * op(A) an m by k matrix, op(B) a k by n matrix and C an m by n matrix
 *
 *  TRANSA-character
 *	On entry, TRANSA specifies the form of op(A) to be used in
 *	the matrix multiplication as follows:
 *		TRANSA='n', op(A)=A.
 *		TRANSA='t', op(A)=A'.
 *		TRANSA='c', op(A)=A'.
 *	On exit, unchanged.
 *
 *  TRANSB-character
 *	On entry, TRANSB specifies the form of op(B) to be used in
 *	the matrix multiplication as follows:
 *		TRANSB='n', op(B)=B.
 *		TRANSB='t', op(B)=B'.
 *		TRANSB='c', op(B)=B'.
 *	On exit, unchanged
 *
 *  M-integer
 *	On entry, M specifies the number of rows of the matrix op(A)
 *	  and of the matrix C. M must be at least zero.
 *	On exit, unchanged
 *  N-integer
 *	On entry, N specifies the number of columns of the matrix op(B)
 *	  and of the matrix C. N must be at least zero.
 *	On exit, unchanged
 *  K-integer
 *	On entry, K specifies the number of columns of the matrix op(A)
 *	  and the number of rows of matrix op(B). K must be at least
 *	  zero.
 *	On exit, unchanged
 *  ALPHA 	DOUBLE precision scalar, unchanged on exit
 *  A	-two dimensional DOUBLE input array, unchanged on exit
 *  B	-two dimensional DOUBLE input array, unchanged on exit
 *  BETA	DOUBLE precision scalar, unchanged on exit
 *  C	-two dimensional DOUBLE input and output array
 *	unless beta is zero, the input value of C will effect output.
 *	In any non-error case, the original matrix C will be overwritten
 *
 */

#include "AIR.h"

void AIR_dgemm(const char transa, const char transb, const unsigned int m, const unsigned int n, const unsigned int k, const double alpha, double **a, double **b, const double beta, double **c)

{
	AIR_Boolean nota,notb;

	/*Set NOTA and NOTB as true if A and B respectively are not	*/
	/*transposed and set NROWA,NCOLA and NROWB as the number of rows*/
	/*and columns of A and the number of rows of B respectively	*/

	nota=(transa=='n');
	notb=(transb=='n');

	/*Test the input parameters*/
	if(!nota && transa!='c' && transa!='t'){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("programming error, first argument to subroutine dgemm must be the character n, t or c\n");
		return;
	}
	if(!notb && transb!='c' && transb!='t'){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("programming error, second argument to subroutine dgemm must be the character n, t or c\n");
		return;
	}


	/*Quick return if possible*/
	if(m==0||n==0||(((alpha==0.0)||k==0)&&(beta==1.0))){
		return;
	}

	/*And if alpha==0.0 */
	if(alpha==0.0){
		if(beta==0.0){

			unsigned int j;

			for(j=0;j<n;j++){

				unsigned int i;

				for(i=0;i<m;i++){
					c[j][i]=0.0;
				}
			}
		}
		else{

			unsigned int j;

			for(j=0;j<n;j++){

				unsigned int i;

				for(i=0;i<m;i++){
					c[j][i]*=beta;
				}
			}
		}
		return;
	}

	/*Start the operations*/
	if(notb){
		if(nota){
			/*form C=alpha*A*B+beta*C */

			unsigned int j;

			for(j=0;j<n;j++){
				if(beta==0.0){

					unsigned int i;

					for(i=0;i<m;i++){
						c[j][i]=0.0;
					}
				}
				else if(beta!=1.0){

					unsigned int i;

					for(i=0;i<m;i++){
						c[j][i]*=beta;
					}
				}
				{
					unsigned int l;

					for(l=0;l<k;l++){
						if(b[j][l]!=0.0){
							double temp=alpha*b[j][l];

							unsigned int i;

							for(i=0;i<m;i++){
								c[j][i]+=temp*a[l][i];
							}
						}
					}
				}
			}
		}
		else{
			/*form C=alpha*A'*B+beta*C */

			unsigned int j;

			for(j=0;j<n;j++){

				unsigned int i;

				for(i=0;i<m;i++){
					double temp=0.0;

					{
						unsigned int l;

						for(l=0;l<k;l++){
							temp+=a[i][l]*b[j][l];
						}
					}
					if(beta==0.0){
						c[j][i]=alpha*temp;
					}
					else{
						c[j][i]*=beta;
						c[j][i]+=alpha*temp;
					}
				}
			}
		}
	}
	else{
		if(nota){
			/*form C=alpha*A*B'+beta*C */

			unsigned int j;

			for(j=0;j<n;j++){
				if(beta==0.0){

					unsigned int i;

					for(i=0;i<m;i++){
						c[j][i]=0.0;
					}
				}
				else if(beta!=1.0){

					unsigned int i;

					for(i=0;i<m;i++){
						c[j][i]*=beta;
					}
				}
				{
					unsigned int l;

					for(l=0;l<k;l++){
						if(b[l][j]!=0.0){
							double temp=alpha*b[l][j];

							unsigned int i;

							for(i=0;i<m;i++){
								c[j][i]+=temp*a[l][i];
							}
						}
					}
				}
			}
		}
		else{
			/*form C=alpha*A'*B'+beta*C*/
			
			unsigned int j;

			for(j=0;j<n;j++){

				unsigned int i;

				for(i=0;i<m;i++){
					double temp=0.0;
					{
						unsigned int l;

						for(l=0;l<k;l++){
							temp+=a[i][l]*b[l][j];
						}
					}
					if(beta==0.0){
						c[j][i]=alpha*temp;
					}
					else{
						c[j][i]*=beta;
						c[j][i]+=alpha*temp;
					}
				}
			}
		}
	}	
}
