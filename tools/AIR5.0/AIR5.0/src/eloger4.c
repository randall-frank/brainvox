/* Copyright 1996-2001 Roger P. Woods, M.D. */
/* Modified: 5/27/01 */


/*
 * This routine will compute the real matrix logarithm of
 * a real 1x1, 2x2, 3x3 or 4x4 matrix with a positive determinant.
 * 
 * It uses the Schur-Frechet algorithm:
 * Kenney CS, Laub AJ. A Schur-Frechet algorithm for computing the
 * logarithm and exponential of a matrix. Siam J. Matrix Anal Appl
 * 1998;19(3):640-663.
 *
 *
 * The result replaces the original matrix
 * Returns 0, if successful, errcode otherwise
 *
 * To support matrices up to 4x4, storage2 needs to be allocated matrix3(2,2,20)
 * and storage4 needs to be allocated matrix3(4,4,10)
 */

#include "AIR.h"

AIR_Error AIR_eloger4(const unsigned int n, double **a, const double *toproot, const double *botroot, double ***storage2, double ***storage4)

{
	double	**z=*storage4++; /* 0 */
	
	if(n==1){
		if(a[0][0]<=0){
			return AIR_NONPOSITIVE_ELOGER_ERROR;
		}
		a[0][0]=log(a[0][0]);
		return 0;
	}
	{
		double wr[4];
		double wi[4];
		
		/* Perform real Schur decomposition */
		{
			double iv1[4];
			unsigned int ierr;
			
			AIR_rgschur(n,a,wr,wi,z,iv1,&ierr);
			if(ierr!=n){
				return AIR_SCHUR_FAILURE_ERROR;		
			}
		}
		
		/* Check determinant and set values that are theoretically zero to be exactly zero */
		if(n==2){
			if(wi[0]==0.0){
				a[0][1]=0;
				/*
				R x
				0 R
				*/
				if(wr[0]*wr[1]<=0){
					return AIR_NONPOSITIVE_ELOGER_ERROR;
				}
			}
			/* else both are imaginary and determinant is positive */
		}
		else if(n==3){
			a[0][2]=0;
			if(wi[0]==0.0){
				a[0][1]=0;		
				/* 	
				R x x
				0 x x
				0 ? x
				*/
			
				if(wi[1]==0.0){
					a[1][2]=0;
					/* 	
					R x x
					0 R x
					0 0 R
					*/
					if(wr[0]*wr[1]*wr[2]<=0){
						return AIR_NONPOSITIVE_ELOGER_ERROR;
					}
				}
				else{
					/* 	
					R x x
					0 I x
					0 x I
					*/
					if(wr[0]<=0){
						return AIR_NONPOSITIVE_ELOGER_ERROR;
					}
				}
			}
			else{
				a[1][2]=0;
				/* 	
				I x x
				x I x
				0 0 R
				*/
				if(wr[2]<=0){
					return AIR_NONPOSITIVE_ELOGER_ERROR;
				}
			}
		}
		else if(n==4){
			a[0][2]=0;
			a[0][3]=0;
			a[1][3]=0;
			if(wi[0]==0.0){
				a[0][1]=0;
				/* 	
				R x x x
				0 x x x
				0 ? x x
				0 0 ? x
				*/
				if(wi[1]==0.0){
					a[1][2]=0;
					/* 	
					R x x x
					0 R x x
					0 0 x x
					0 0 ? x
					*/
					if(wi[2]==0.0){
						a[2][3]=0;
						/* 	
						R x x x
						0 R x x
						0 0 R x
						0 0 0 R
						*/
						if(wr[0]*wr[1]*wr[2]*wr[3]<=0){
							return AIR_NONPOSITIVE_ELOGER_ERROR;
						}
					}
					else{
						/* 	
						R x x x
						0 R x x
						0 0 I x
						0 0 x I
						*/
						if(wr[0]*wr[1]<=0){
							return AIR_NONPOSITIVE_ELOGER_ERROR;
						}
					}
				}
				else{
					a[2][3]=0;
					/* 	
					R x x x
					0 I x x
					0 x I x
					0 0 0 R
					*/
					if(wr[0]*wr[3]<=0){
						return AIR_NONPOSITIVE_ELOGER_ERROR;
					}
				}
			}
			else{
				a[1][2]=0;
				/* 	
				I x x x
				x I x x
				0 0 x x
				0 0 ? x
				*/		
				if(wi[2]==0.0){
					a[2][3]=0;
					/* 	
					I x x x
					x I x x
					0 0 R x
					0 0 0 R
					*/	
					if(wr[2]*wr[3]<=0){
						return AIR_NONPOSITIVE_ELOGER_ERROR;
					}
				}
				/* else all are imaginary and determinant is positive */	
			}
		}
	}
	{
		double	**zr=*storage4++;
		double	**zi=*storage4++;
		double	**er=*storage4++;
		double	**ei=*storage4++;
		double	**imag=*storage4++;
		double 	**ar=*storage4++;
		double 	**ai=*storage4++;
		double  **tempr=*storage4++;
		double  **tempi=*storage4++;
				
		/* Convert real Schur form to complex Schur form */
		AIR_rsftocsf(n,a,zr,zi);
		{
			unsigned int j;
			
			for(j=0;j<n;j++){
				
				unsigned int i;
				
				for(i=0;i<n;i++){
				
					imag[j][i]=0;
				}
			}
		}
		
		AIR_cgemm('c','n',n,n,n,1.0,0.0,zr,zi,a,imag,0.0,0.0,tempr,tempi);
		AIR_cgemm('n','n',n,n,n,1.0,0.0,tempr,tempi,zr,zi,0.0,0.0,ar,ai);

		/* Find the log of a using the Schur-Frechet algorithm */
		{
			AIR_Error errcode=AIR_logut(n,ar,ai,er,ei,toproot,botroot,storage2);
			if(errcode!=0){
				return errcode;
			}
		}

		/* Convert complex Schur form back to real Schur form */
		AIR_cgemm('n','n',n,n,n,1.0,0.0,zr,zi,er,ei,0.0,0.0,tempr,tempi);
		AIR_cgemm('n','c',n,n,n,1.0,0.0,tempr,tempi,zr,zi,0.0,0.0,a,imag);

		/* Undo original real Schur decomposition */
		AIR_dgemm('n','n',n,n,n,1.0,z,a,0.0,tempr);
		AIR_dgemm('n','c',n,n,n,1.0,tempr,z,0.0,a);
	}
	return 0;
}
