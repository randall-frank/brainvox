/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/22/01 */

/*
 * void cgemm()
 *
 * CGEMM performs one of the complex matrix-matrix operations
 *	C=alpha*op(A)*op(B)+beta*C
 * where op(X) is one of
 *	op(X)=X or op(X)=X' or op(X)=conjg(X')
 * alpha and beta are scalars, and A, B, and C are matrices, with
 * op(A) an m by k matrix, op(B) a k by n matrix and C an m by n matrix	
 *
 *  TRANSA-character
 *	On entry, TRANSA specifies the form of op(A) to be used in
 *	the matrix multiplication as follows:
 *		TRANSA='n', op(A)=A.
 *		TRANSA='t', op(A)=A'.
 *		TRANSA='c', op(A)=conjg(A').
 *	On exit, unchanged.
 *
 *  TRANSB-character
 *	On entry, TRANSB specifies the form of op(B) to be used in
 *	the matrix multiplication as follows:
 *		TRANSB='n', op(B)=B.
 *		TRANSB='t', op(B)=B'.
 *		TRANSB='c', op(B)=conjg(B').
 *	On exit, unchanged
 *
 *  M-integer
 *	On entry, M specifies the number of rows of the matrix op(A)
 *	  and of the matrix C. M must be at least zero.
 *
 *  N-integer
 *	On entry, N specifies the number of columns of the matrix op(B)
 *	  and of the matrix C. N must be at least zero.
 *
 *  K-integer
 *	On entry, K specifies the number of columns of the matrix op(A)
 *	  and the number of rows of matrix op(B). K must be at least
 *	  zero.
 *
 *  ALPHAr,ALPHAi 	COMPLEX DOUBLE precision scalar
 *  Ar,Ai-two dimensional COMPLEX DOUBLE input array, unchanged on exit
 *  Br,Bi-two dimensional COMPLEX DOUBLE input array, unchanged on exit
 *  BETAr,BETAi		COMPLEX DOUBLE precision scalar
 *  Cr,Ci-two dimensional COMPLEX DOUBLE input and output array
 *	unless beta is zero, the input value of C will effect output.
 *	In any non-error case, the original matrix C will be overwritten
 *
 */

#include "AIR.h"

void AIR_cgemm(const char transa, const char transb, const unsigned int m, const unsigned int n, const unsigned int k, const double alphar, const double alphai, double **ar, double **ai, double **br, double **bi, const double betar, const double betai, double **cr, double **ci)

{
	AIR_Boolean nota,notb;
	AIR_Boolean conja,conjb;

	/*Set NOTA and NOTB as true if A and B respectively are not	*/
	/*transposed and set NROWA,NCOLA and NROWB as the number of rows*/
	/*and columns of A and the number of rows of B respectively	*/

	nota=(transa=='n');
	notb=(transb=='n');
	conja=(transa=='c');
	conjb=(transb=='c');

	/*Test the input parameters*/
	if(!nota && !conja && transa!='t'){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("programming error, first argument to subroutine cgemm must be the character n, t or c\n");
		return;
	}
	if(!notb && !conjb && transb!='t'){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("programming error, second argument to subroutine cgemm must be the character n, t or c\n");
		return;
	}


	/*Quick return if possible*/
	if(m==0||n==0||(((alphar==0.0 && alphai==0.0)||k==0)&&(betar==1.0 && betai==0.0))){
		return;
	}

	/*And if alpha==0.0 */
	if(alphar==0.0 && alphai==0.0){
		if(betar==0.0 && betai==0.0){

			unsigned int j;

			for(j=0;j<n;j++){

				unsigned int i;

				for(i=0;i<m;i++){
					cr[j][i]=0.0;
					ci[j][i]=0.0;
				}
			}
		}
		else{

			unsigned int j;

			for(j=0;j<n;j++){

				unsigned int i;

				for(i=0;i<m;i++){
					/* c[j][i]*=beta*/
					double tempr=cr[j][i]*betar-ci[j][i]*betai;
					double tempi=cr[j][i]*betai+ci[j][i]*betar;
					cr[j][i]=tempr;
					ci[j][i]=tempi;
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
				if(betar==0.0 && betai==0.0){

					unsigned int i;

					for(i=0;i<m;i++){
						cr[j][i]=0.0;
						ci[j][i]=0.0;
					}
				}
				else if(betar!=1.0 || betai!=0.0){

					unsigned int i;

					for(i=0;i<m;i++){
						/* c=c*beta */
						double tempr=cr[j][i]*betar-ci[j][i]*betai;
						double tempi=cr[j][i]*betai+ci[j][i]*betar;
						cr[j][i]=tempr;
						ci[j][i]=tempi;
					}
				}
				{
					unsigned int l;

					for(l=0;l<k;l++){
						if(br[j][l]!=0.0 || bi[j][l]!=0.0){
							/* temp=b*alpha */
							double tempr=br[j][l]*alphar-bi[j][l]*alphai;
							double tempi=br[j][l]*alphai+bi[j][l]*alphar;
							{
								unsigned int i;

								for(i=0;i<m;i++){
									/* c+=temp*alpha */
									cr[j][i]+=ar[l][i]*tempr-ai[l][i]*tempi;
									ci[j][i]+=ar[l][i]*tempi+ai[l][i]*tempr;
								}
							}
						}
					}
				}
			}
		}
		else if(conja){
			/*form C=alpha*conjg(A')*B+beta*C */

			unsigned int j;

			for(j=0;j<n;j++){

				unsigned int i;

				for(i=0;i<m;i++){
					double tempr=0.0;
					double tempi=0.0;
					{
						unsigned int l;

						for(l=0;l<k;l++){
							/* temp+=congj(a)*b */
							tempr+=ar[i][l]*br[j][l]+ai[i][l]*bi[j][l];
							tempi+=ar[i][l]*bi[j][l]-ai[i][l]*br[j][l];
						}
					}
					if(betar==0.0 && betai==0.0){
						/* c=temp*alpha */
						cr[j][i]=tempr*alphar-tempi*alphai;
						ci[j][i]=tempr*alphai+tempi*alphar;
					}
					else{
						/* c=c*beta+conjg(a)*b*alpha */
						double temp2r=cr[j][i]*betar-ci[j][i]*betai;
						double temp2i=cr[j][i]*betai+ci[j][i]*betar;
						cr[j][i]=temp2r+alphar*tempr-alphai*tempi;
						ci[j][i]=temp2i+alphar*tempi+alphai*tempr;
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
					double tempr=0.0;
					double tempi=0.0;

					{
						unsigned int l;

						for(l=0;l<k;l++){
							/* temp=a*b */
							tempr+=ar[i][l]*br[j][l]-ai[i][l]*bi[j][l];
							tempi+=ar[i][l]*bi[j][l]+ai[i][l]*br[j][l];
						}
					}
					if(betar==0.0 && betai==0.0){
						/* c= a*b*alpha */
						cr[j][i]=tempr*alphar-tempi*alphai;
						ci[j][i]=tempr*alphai+tempi*alphar;
					}
					else{
						/* c= c*beta +a*b*alpha */
						double temp2r=cr[j][i]*betar-ci[j][i]*betai;
						double temp2i=cr[j][i]*betai+ci[j][i]*betar;
						cr[j][i]=temp2r+alphar*tempr-alphai*tempi;
						ci[j][i]=temp2i+alphar*tempi+alphai*tempr;
					}
				}
			}
		}
	}
	else if(nota){
		if(conjb){
			/*form C=alpha*A*conj(B')+beta*C */

			unsigned int j;

			for(j=0;j<n;j++){
				if(betar==0.0 && betai==0.0){

					unsigned int i;

					for(i=0;i<m;i++){
						cr[j][i]=0.0;
						ci[j][i]=0.0;
					}
				}
				else if(betar!=1.0 || betai!=0.0){

					unsigned int i;

					for(i=0;i<m;i++){
						/* c=c*beta */
						double tempr=cr[j][i]*betar-ci[j][i]*betai;
						double tempi=cr[j][i]*betai+ci[j][i]*betar;
						cr[j][i]=tempr;
						ci[j][i]=tempi;
					}
				}
				{
					unsigned int l;

					for(l=0;l<k;l++){
						if(br[l][j]!=0.0 || bi[l][j]!=0.0){
							/* temp=conjg(b)*alpha */
							double tempr=br[l][j]*alphar+bi[l][j]*alphai;
							double tempi=br[l][j]*alphai-bi[l][j]*alphar;
							{
								unsigned int i;

								for(i=0;i<m;i++){
									/* c+=a*conjg(b)*alpha */
									cr[j][i]+=ar[l][i]*tempr-ai[l][i]*tempi;
									ci[j][i]+=ar[l][i]*tempi+ai[l][i]*tempr;
								}
							}
						}
					}
				}
			}
		}
		else{
			/*form C=alpha*A*B'+beta*C */

			unsigned int j;

			for(j=0;j<n;j++){
				if(betar==0.0 && betai==0.0){

					unsigned int i;

					for(i=0;i<m;i++){
						cr[j][i]=0.0;
						ci[j][i]=0.0;
					}
				}
				else if(betar!=1.0 || betai!=0.0){

					unsigned int i;

					for(i=0;i<m;i++){
						/* c=c*beta */
						double tempr=cr[j][i]*betar-ci[j][i]*betai;
						double tempi=cr[j][i]*betai+ci[j][i]*betar;
						cr[j][i]=tempr;
						ci[j][i]=tempi;
					}
				}
				{
					unsigned int l;

					for(l=0;l<k;l++){
						if(br[l][j]!=0.0 || bi[l][j]!=0.0){
							/* temp=b*alpha */
							double tempr=br[l][j]*alphar-bi[l][j]*alphai;
							double tempi=br[l][j]*alphai+bi[l][j]*alphar;

							{
								unsigned int i;

								for(i=0;i<m;i++){
									/* cr+=a*b*alpha */
									cr[j][i]+=ar[l][i]*tempr-ai[l][i]*tempi;
									ci[j][i]+=ar[l][i]*tempi+ai[l][i]*tempr;
								}
							}
						}
					}
				}
			}
		}
	}
	else if(conja){
		if(conjb){
			/*form C=alpha*conj(A')*conj(B')+beta*C */

			unsigned int j;

			for(j=0;j<n;j++){

				unsigned int i;

				for(i=0;i<m;i++){
					double tempr=0.0;
					double tempi=0.0;

					{
						unsigned int l;

						for(l=0;l<k;l++){
							/* temp+=conjg(a)*conjg(b) */
							tempr+=ar[i][l]*br[l][j]-ai[i][l]*bi[l][j];
							tempi+=-ar[i][l]*bi[l][j]-ai[i][l]*br[l][j];
						}
					}
					if(betar==0.0 && betai==0.0){
						/* c=alpha*conjg(a)*conjg(b) */
						cr[j][i]=alphar*tempr-alphai*tempi;
						ci[j][i]=alphar*tempi+alphai*tempr;
					}
					else{
						/* c=c*beta+alpha*conjg(a)*conjg(b) */
						double temp2r=cr[j][i]*betar-ci[j][i]*betai;
						double temp2i=cr[j][i]*betai+ci[j][i]*betar;
						cr[j][i]=temp2r+alphar*tempr-alphai*tempi;
						ci[j][i]=temp2i+alphar*tempi+alphai*tempr;
					}
				}
			}
		}
		else{
			/*form C=alpha*conjg(A')*B'+beta*C */

			unsigned int j;

			for(j=0;j<n;j++){

				unsigned int i;

				for(i=0;i<m;i++){
					double tempr=0.0;
					double tempi=0.0;
					{
						unsigned int l;

						for(l=0;l<k;l++){
							/* temp+=conjg(a)*b */
							tempr+=ar[i][l]*br[l][j]+ai[i][l]*bi[l][j];
							tempi+=ar[i][l]*bi[l][j]-ai[i][l]*br[l][j];
						}
					}
					if(betar==0.0 && betai==0.0){
						/* c=alpha*conjg(a)*b */
						cr[j][i]=alphar*tempr-alphai*tempi;
						ci[j][i]=alphar*tempi+alphai*tempr;
					}
					else{
						/* c= c*beta+alpha*conjg(a)*b */
						double temp2r=cr[j][i]*betar-ci[j][i]*betai;
						double temp2i=cr[j][i]*betai+ci[j][i]*betar;
						cr[j][i]=temp2r+alphar*tempr-alphai*tempi;
						ci[j][i]=temp2i+alphar*tempi+alphai*tempr;
					}
				}
			}
		}
	}

	else{
		if(conjb){
			/*form C=alpha*A'*conjg(B')+beta*C */

			unsigned int j;

			for(j=0;j<n;j++){

				unsigned int i;

				for(i=0;i<m;i++){
					double tempr=0.0;
					double tempi=0.0;

					{
						unsigned int l;

						for(l=0;l<k;l++){
							/* temp+=conj(b)*a */
							tempr+=ar[i][l]*br[l][j]+ai[i][l]*bi[l][j];
							tempi+=-ar[i][l]*bi[l][j]+ai[i][l]*br[l][j];
						}
					}
					if(betar==0.0 && betai==0.0){
						/* c=alpha*conj(b)*a */
						cr[j][i]=alphar*tempr-alphai*tempi;
						ci[j][i]=alphar*tempi+alphai*tempr;
					}
					else{
						/* c=c*beta + alpha*conj(b)*a */
						double temp2r=cr[j][i]*betar-ci[j][i]*betai;
						double temp2i=cr[j][i]*betai+ci[j][i]*betar;
						cr[j][i]=temp2r+alphar*tempr-alphai*tempi;
						ci[j][i]=temp2i+alphar*tempi+alphai*tempr;
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
					double tempr=0.0;
					double tempi=0.0;

					{
						unsigned int l;

						for(l=0;l<k;l++){
							/* temp+=a*b */
							tempr+=ar[i][l]*br[l][j]-ai[i][l]*bi[l][j];
							tempi+=ar[i][l]*bi[l][j]+ai[i][l]*br[l][j];
						}
					}
					if(betar==0.0 && betai==0.0){
						/* c=alpha*a*b */
						cr[j][i]=alphar*tempr-alphai*tempi;
						ci[j][i]=alphar*tempi+alphai*tempr;
					}
					else{
						/* c=c*beta+alpha*a*b */
						double temp2r=cr[j][i]*betar-ci[j][i]*betai;
						double temp2i=cr[j][i]*betai+ci[j][i]*betar;
						cr[j][i]=temp2r+alphar*tempr-alphai*tempi;
						ci[j][i]=temp2i+alphar*tempi+alphai*tempr;
					}
				}
			}
		}
	}	
}
