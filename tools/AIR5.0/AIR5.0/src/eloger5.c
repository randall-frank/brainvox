/* Copyright 1996-2001 Roger P. Woods, M.D. */
/* Modified: 5/22/01 */


/*
 * This routine will compute the real matrix logarithm of
 * a real affine 4x4 matrix with a positive determinant.
 * 
 * When necesary, it uses the Schur-Frechet algorithm:
 * Kenney CS, Laub AJ. A Schur-Frechet algorithm for computing the
 * logarithm and exponential of a matrix. Siam J. Matrix Anal Appl
 * 1998;19(3):640-663.
 * 
 * When accuracy will not be compromised, it uses the faster Schur
 * decomposition approach described as Algorithm 11.1.1  in:
 * Golub GH, van Loan, CF. Matrix Computations, 2nd edition. The
 * Johns Hopkins University Press, Baltimore and London, 1989.
 *
 *
 * The result replaces the original matrix
 * Returns 0, if successful, errcode otherwise
 *
 * storage2 needs to be allocated matrix3(2,2,20)
 * and storage4 needs to be allocated matrix3(4,4,10)
 *
 * It is the responsibility of the calling routine to make sure that
 * the transformation is affine with positive determinant
 *
 */

#include "AIR.h"

#define TOOSMALL .0001

AIR_Error AIR_eloger5(double **a, const double *toproot, const double *botroot, double ***storage2, double ***storage4)

{
	double	**z=*storage4++; /* 0 */

	AIR_Boolean fastok=TRUE;
	double wi[4];
	
	{
		double wr[4];
		/* Perform real Schur decomposition */
		{
			double iv1[4];
			unsigned int ierr;
			
			AIR_rgschur(4,a,wr,wi,z,iv1,&ierr);
			if(ierr!=4){
				return AIR_SCHUR_FAILURE_ERROR;		
			}
		}

		/* Set values that are theoretically zero to be exactly zero */
		{
			a[0][2]=0;

			if(wi[0]==0.0){
				a[0][1]=0;
				/* 	
				R x x x
				0 x x x
				0 ? x x
				0 0 0 1
				*/
				if(wi[1]==0.0){
					a[1][2]=0;
					/* 	
					R x x x
					0 R x x
					0 0 R x
					0 0 0 1
					*/

					if(wr[0]<0 || wr[1]<0 || wr[2]<0 || fabs(wr[0]-wr[1])<TOOSMALL || fabs(wr[0]-wr[2])<TOOSMALL || fabs(wr[0]-wr[3])<TOOSMALL || fabs(wr[1]-wr[2])<TOOSMALL || fabs(wr[1]-wr[3])<TOOSMALL || fabs(wr[2]-wr[3])<TOOSMALL) fastok=FALSE;
				}
				else{
					/* 	
					R x x x
					0 I x x
					0 x I x
					0 0 0 1
					*/

					if(wr[0]<0 || fabs(wr[0]-wr[3])<TOOSMALL) fastok=FALSE;
				}
			}
			else{
				a[1][2]=0;
				/* 	
				I x x x
				x I x x
				0 0 R x
				0 0 0 1
				*/		

				if(wr[2]<0 || fabs(wr[2]-wr[3])<TOOSMALL) fastok=FALSE;
			}
		}
	}
	{
		double **zr=*storage4++;
		double **zi=*storage4++;
		double **er=*storage4++;
		double **ei=*storage4++;
		double **tempr=*storage4++;
				
		if(!fastok){
			
			double **imag=*storage4++;
			double **ar=*storage4++;
			double **ai=*storage4++;
			double **tempi=*storage4++;
				
			/* Convert real Schur form to complex Schur form */
			AIR_rsftocsf(4,a,zr,zi);

			{
				unsigned int j;
				
				for(j=0;j<4;j++){
					
					unsigned int i;
					
					for(i=0;i<4;i++){
					
						imag[j][i]=0;
					}
				}
			}
			
			AIR_cgemm('c','n',4,4,4,1.0,0.0,zr,zi,a,imag,0.0,0.0,tempr,tempi);
			AIR_cgemm('n','n',4,4,4,1.0,0.0,tempr,tempi,zr,zi,0.0,0.0,ar,ai);

			/* Find the log of a using the Schur-Frechet algorithm */
			{
				AIR_Error errcode=AIR_logut(4,ar,ai,er,ei,toproot,botroot,storage2);
				if(errcode!=0){
					return errcode;
				}
			}

			
			/* Convert complex Schur form back to real Schur form */
			AIR_cgemm('n','n',4,4,4,1.0,0.0,zr,zi,er,ei,0.0,0.0,tempr,tempi);
			AIR_cgemm('n','c',4,4,4,1.0,0.0,tempr,tempi,zr,zi,0.0,0.0,a,imag);
		}
		else{
		
			/* It's ok to use the faster algorithm */
			double **loga=*storage4++;
			double **egr=*storage2++;
			double **egi=*storage2++;
			
			
			/* If all the eigenvalues are real */
			if(wi[0]==0.0 && wi[1]==0.0){
			
			
				loga[0][0]=log(a[0][0]);
				loga[1][1]=log(a[1][1]);
				loga[2][2]=log(a[2][2]);
				loga[3][3]=0;
				
				loga[1][0]=a[1][0]*(loga[1][1]-loga[0][0])/(a[1][1]-a[0][0]);
				loga[2][1]=a[2][1]*(loga[2][2]-loga[1][1])/(a[2][2]-a[1][1]);
				loga[3][2]=a[3][2]*(-loga[2][2])/(1.0-a[2][2]);
								
				{
					double temp=a[2][0]*(loga[2][2]-loga[0][0]);
					temp+=(a[1][0]*loga[2][1]-loga[1][0]*a[2][1]);
					loga[2][0]=temp/(a[2][2]-a[0][0]);
				}
				{
					double temp=a[3][1]*(-loga[1][1]);
					temp+=(a[2][1]*loga[3][2]-loga[2][1]*a[3][2]);
					loga[3][1]=temp/(1.0-a[1][1]);
				}
				{
					double temp=a[3][0]*(-loga[0][0]);
					temp+=(a[1][0]*loga[3][1]-loga[1][0]*a[3][1]);
					temp+=(a[2][0]*loga[3][2]-loga[2][0]*a[3][2]);
					loga[3][0]=temp/(1.0-a[0][0]);
				}
				loga[0][1]=loga[0][2]=loga[1][2]=loga[0][3]=loga[1][3]=loga[2][3]=0.0;
			}
			else{
						
				/* We have an imaginary eigenvalue */

				if(wi[0]==0.0){
					/* First eigenvalue is real and the second two are not */
					/* x x x x */
					/* 0 x x x */
					/* 0 x x x */
					/* 0 0 0 1 */

					/* Swap row 0 with rows 1 and 2 */
					AIR_Error errcode=AIR_exchng(a,z,4,0,1,2);
					if(errcode!=0){	
						return errcode;
					}
					a[0][2]=0.0;
				}
				/* Now the first two eigenvalues are nonreal and the third is real */
				/* x x x x */
				/* x x x x */
				/* 0 0 x x */
				/* 0 0 0 1 */
				
				loga[2][2]=log(a[2][2]);
				loga[3][3]=0;
				loga[3][2]=a[3][2]*(-loga[2][2])/(1.0-a[2][2]);
				
				/* Copy 2 x 2 diagonal elements into egr */
				egr[0][0]=a[0][0];
				egr[0][1]=a[0][1];
				egr[1][0]=a[1][0];
				egr[1][1]=a[1][1];

				/* Compute eigenvalues and vectors */
				{
					double w2r[2];
					double w2i[2];
					{
						unsigned int ierr;
						double iv1[2];
						double fv1[2];
						
						AIR_rg(2,egr,w2r,w2i,er,iv1,fv1,&ierr);
						if(ierr!=2){
							return AIR_EIGEN_VECTOR_FAILURE;
						}
					}

					{
						unsigned int j;
						
						for(j=0;j<2;j++){
						
							unsigned int i;
							
							for(i=0;i<2;i++){

								if(w2i[i]>0.0){
									zr[i][j]=er[i][j];
									zi[i][j]=er[i+1][j];
								}
								else if (w2i[i]<0.0){
									zr[i][j]=er[i-1][j];
									zi[i][j]=-er[i][j];
								}
								else{
									/* Shouldn't happen in this context */
									zr[i][j]=er[i][j];
									zi[i][j]=0.0;
								}
							}
						}
					}

					/*Compute log of eigenvalues*/
					{
						unsigned int i;
						
						for(i=0;i<2;i++){
							AIR_Error errcode=AIR_clog(w2r[i],w2i[i],&w2r[i],&w2i[i]);
							if(errcode!=0) return errcode;
						}
					}
					/*copy w2i,w2r into egr,egi*/
					{
						unsigned int j;
						
						for(j=0;j<2;j++){
						
							unsigned int i;
							
							for(i=0;i<2;i++){
								if(i==j){
									egr[j][i]=w2r[i];
									egi[j][i]=w2i[i];
								}
								else{
									egr[j][i]=0.0;
									egi[j][i]=0.0;
								}
							}
						}
					}
				}

				/* Compute z*eg */
				AIR_cgemm('n','n',2,2,2,1.0,0.0,zr,zi,egr,egi,0.0,0.0,er,ei);


				/* Compute z*eg/z */
				{
					unsigned int ipvt[2];
					double vr[2];
					double vi[2];
					
					AIR_Error errcode=AIR_cdvasb(2,2,er,ei,zr,zi,vr,vi,ipvt);
					if(errcode!=0){
						return errcode;
					}
				}
				/* Copy result back into loga */
				loga[0][0]=er[0][0];
				loga[0][1]=er[0][1];
				loga[1][0]=er[1][0];
				loga[1][1]=er[1][1];

				/* Find loga[2][0] and loga[2][1] */

				egr[0][0]=(loga[0][0]-loga[2][2])*a[2][0]+loga[1][0]*a[2][1];
				egr[0][1]=loga[0][1]*a[2][0]+(loga[1][1]-loga[2][2])*a[2][1];

				er[0][0]=a[0][0]-a[2][2];
				er[1][0]=a[1][0];
				er[0][1]=a[0][1];
				er[1][1]=a[1][1]-a[2][2];
				{
					unsigned int ipvt[2];
						
					if(AIR_dgefa(er,2,ipvt)!=2) return AIR_SINGULAR_ELOGER_ERROR;
					AIR_dgesl(er,2,ipvt,egr[0],0);
				}
				loga[2][0]=egr[0][0];
				loga[2][1]=egr[0][1];

				/* Find loga[3][0] and loga[3][1] */
				
				egr[0][0]=loga[0][0]*a[3][0]+loga[1][0]*a[3][1]-loga[3][2]*a[2][0]+a[3][2]*loga[2][0];
				egr[0][1]=loga[0][1]*a[3][0]+loga[1][1]*a[3][1]-loga[3][2]*a[2][1]+a[3][2]*loga[2][1];

				er[0][0]=a[0][0]-1.0;
				er[1][0]=a[1][0];
				er[0][1]=a[0][1];
				er[1][1]=a[1][1]-1.0;
				{
					unsigned int ipvt[2];

					if(AIR_dgefa(er,2,ipvt)!=2) return AIR_SINGULAR_ELOGER_ERROR;
					AIR_dgesl(er,2,ipvt,egr[0],0);
				}
				loga[3][0]=egr[0][0];
				loga[3][1]=egr[0][1];

				/* Fill in zero values in loga */
				loga[0][2]=loga[1][2]=loga[0][3]=loga[1][3]=loga[2][3]=0.0;
			}
			/* Copy result back into a */
			{
				unsigned int j;
				
				for(j=0;j<4;j++){
				
					unsigned int i;
					
					for(i=0;i<4;i++){
					
						a[j][i]=loga[j][i];
					}
				}
			}
		}

		/* Undo original real Schur decomposition and any consquences of exchng */
		AIR_dgemm('n','n',4,4,4,1.0,z,a,0.0,tempr);
		AIR_dgemm('n','c',4,4,4,1.0,tempr,z,0.0,a);
	}

	return 0;
}
