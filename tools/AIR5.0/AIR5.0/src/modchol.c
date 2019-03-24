/* Copyright 2000-2001 Roger P. Woods, M.D. */
/* Modified 5/22/01 */

/* 
 * This implements the revised modified Cholesky factorization
 * algorithm described by Schnabel and Eskow.
 *
 * If A is not positive definite, it solves a nearby matrix that is.
 *
 * Schnabel RB, Eskow E, A revised modified Cholesky factorization
 * algorithm,  Siam J Optim., 1999;9(4):1135-1148.
 *
 * Schnabel RB, Eskrow E. A new modified Cholesky factorization. Siam
 * J Sci Stat Comput, 1990;11(6):1136-1158
 *
 * Note that for a sufficiently positive definite matrix, the method is
 * effectively Algorithm 4.2.4 in the 2nd edition of Golub and van Loan's
 * Matrix Computations
 *
 * Returns 0 if matrix was sufficiently positive definite
 * Returns 1 if some other nearby matrix had to be factored
 */
 
#include "AIR.h"
#include <float.h>

unsigned int AIR_modchol(double **a, const unsigned int n, unsigned int *ipvt, double *g)

{
	double gamma=0;
	const double mu=0.1;
	const double tau=pow(DBL_EPSILON,1.0/3.0);
	const double tauhat=tau*tau;
	
	{
		unsigned int i;
 	 	
 		for(i=0;i<n;i++){
 		
 			double temp=fabs(a[i][i]);
 			if(temp>gamma) gamma=temp;
 		}
 	}
 
	 {
 		unsigned int j;

 		/* Phase I, A is potentially positive definite */
 	
 		for(j=0;j<n;j++){
 
 			unsigned int imax=j;

 			{
	 			double min=a[j][j];
	 			double max=min;
	 			
	 			{
	 				unsigned int i;
	 			
	 				for(i=j+1;i<n;i++){
	 				
	 					double temp=a[i][i];
	 					
	 					if(temp<min) min=temp;
	 					else if(temp>max){
	 						max=temp;
	 						imax=i;
	 					}
	 				}
	 			}
	 			if(max<tauhat*gamma) break;
	 			if(min<-mu*max) break;
 			}
 			/* Pivot on maximum diagonal of remaining submatrix */

 			ipvt[j]=imax;

 			if(imax!=j){
 
	 			{
	 				double temp=a[j][j];
	 				
	 				a[j][j]=a[imax][imax];
	 				a[imax][imax]=temp;
	 			}
	 			{
	 				unsigned int i;
	 				
	 				for(i=0;i<j;i++){
	 					
	 					double temp=a[j][i];
	 					a[j][i]=a[imax][i];
	 					a[imax][i]=temp;
	 				}
	 			}
	 			{
	 				unsigned int i;
	 				
	 				for(i=imax+1;i<n;i++){
	 				
	 					double temp=a[i][imax];
	 					a[i][imax]=a[i][j];
	 					a[i][j]=temp;
	 				}
	 			}
	 			{
	 				unsigned int i;
	 				
	 				for(i=j+1;i<imax;i++){
	 					
	 					double temp=a[i][j];
	 					a[i][j]=a[imax][i];
	 					a[imax][i]=temp;
	 				}
	 			}
	 		}
	 		{
	 			if(j<n-1){
	 			
		 			double min=a[j+1][j+1]-a[j+1][j]*a[j+1][j]/a[j][j];
		 			
		 			{
		 				unsigned int i;
		 				
		 				for(i=j+2;i<n;i++){
		 				
		 					double temp=a[i][i]-a[i][j]*a[i][j]/a[j][j];
		 					if(temp<min) min=temp;
		 				}
		 			}
		 			if(min<-mu*gamma) break;
	 			}
	 			
	 			/* Perform jth iteration of factorization */
	 			a[j][j]=sqrt(a[j][j]);
	 			{
	 				unsigned int i;
	 				
	 				for(i=j+1;i<n;i++){
	 				
	 					a[i][j]/=a[j][j];
	 					{
	 						unsigned int k;
	 						
	 						for(k=j+1;k<=i;k++){
	 							a[i][k]-=a[i][j]*a[k][j];
	 						}
	 					}
	 				}
	 			}
	 		}
	 	}

		if(j==n) return 0;
		 
		 /* Phase two, a not positive definite */
		if(j==n-1){
		 
			double delta=tau*(-a[j][j])/(1-tau);
			double temp=tauhat*gamma;
			
			ipvt[j]=j;
		 	
			if(temp>delta) delta=temp;
					 	
		 	a[j][j]=sqrt(delta);
		 	return 1;
		 }
		 else{
		 	unsigned int k=j;
		 	double deltaprev=0;
		 	
		 	/* Calculate the lower Gerschgorin Bounds of a[k]*/
		 	{
		 		unsigned int i;
		 		
		 		for(i=k;i<n;i++){
		 		
		 			g[i]=a[i][i];
		 			
		 			{
		 				unsigned int j1;
		 				
		 				for(j1=k;j1<i;j1++){
		 				
		 					g[i]-=fabs(a[i][j1]);
		 				}
		 			}
		 			{
		 				unsigned int j1;
		 				
		 				for(j1=i+1;j1<n;j1++){
		 				
		 					g[i]-=fabs(a[j1][i]);
		 				}
		 			}
		 		}
		 	}
			{
			 	/* Modified Cholesky decomposition */
			 	unsigned int j1;
			 	
			 	for(j1=k;j1+2<n;j1++){
			 	
			 		unsigned int imax=j1;
			 		double max=g[j1];
			 		
			 		/* Pivot on maximum lower Gerschgorin bound estimate */
			 		{
			 			unsigned int i;
			 			
			 			for(i=j1+1;i<n;i++){
			 			
			 				if(g[i]>max){
			 					max=g[i];
			 					imax=i;
			 				}
			 			}
			 		}
			 		ipvt[j1]=imax;
					if(imax!=j1){
					
			 			{
			 				double temp=a[j1][j1];
			 				
			 				a[j1][j1]=a[imax][imax];
			 				a[imax][imax]=temp;
			 			}
			 			{
			 				unsigned int i;
			 				
			 				for(i=0;i<j1;i++){
			 					
			 					double temp=a[j1][i];
			 					a[j1][i]=a[imax][i];
			 					a[imax][i]=temp;
			 				}
			 			}
			 			{
			 				unsigned int i;
			 				
			 				for(i=imax+1;i<n;i++){
			 				
			 					double temp=a[i][imax];
			 					a[i][imax]=a[i][j1];
			 					a[i][j1]=temp;
			 				}
			 			}
			 			{
			 				unsigned int i;
			 				
			 				for(i=j1+1;i<imax;i++){
			 					
			 					double temp=a[i][j1];
			 					a[i][j1]=a[imax][i];
			 					a[imax][i]=temp;
			 				}
			 			}
			 		}
			 		/* Calculate E[j1][j1] and add to diagonal */
			 		{
			 			double normj=0;
			 			double delta;
			 			
			 			{
			 				unsigned int i;
			 				
			 				for(i=j1+1;i<n;i++){
			 				
			 					normj+=fabs(a[i][j1]);
			 				}
			 			}
			 			delta=normj;
			 			if(tauhat*gamma>delta){
			 				delta=tauhat*gamma;
			 			}
						delta-=a[j1][j1];
						if(delta<0) delta=0;
						if(delta<deltaprev) delta=deltaprev;
						

						if(delta>0){
							a[j1][j1]+=delta;
							deltaprev=delta;
						}

				 		/* Update Gerschgorin bound estimates */
				 		if(a[j1][j1]!=normj){
				 		
				 			double temp=1-normj/a[j1][j1];
				 			
				 			{
				 				unsigned int i;
				 				
				 				for(i=j1+1;i<n;i++){
				 				
				 					g[i]+=fabs(a[i][j1])*temp;
				 				}
				 			}
				 		}
			 		}
			 		/* Perform j1th iteration of factorization */
					a[j1][j1]=sqrt(a[j1][j1]);
		 			{
		 				unsigned int i;
		 				
		 				for(i=j1+1;i<n;i++){
		 				
		 					a[i][j1]/=a[j1][j1];
		 					{
		 						unsigned int k1;
		 						
		 						for(k1=j1+1;k1<=i;k1++){
		 						
		 							a[i][k1]-=a[i][j1]*a[k1][j1];
		 						}
		 					}
		 				}
		 			}
			 	}
		 	}
		 	/* Final 2x2 submatrix */
		 	{
		 		double lambdahi=a[n-2][n-2]-a[n-1][n-1];
		 		double lambdalo;
		 		
		 		ipvt[n-2]=n-2;
		 		ipvt[n-1]=n-1;
		 		
		 		lambdahi=lambdahi*lambdahi;
		 		lambdahi+=4*a[n-1][n-2]*a[n-1][n-2];
		 		lambdahi=sqrt(lambdahi);
		 		lambdalo=-lambdahi;
		 		lambdahi+=(a[n-2][n-2]+a[n-1][n-1]);
		 		lambdalo+=(a[n-2][n-2]+a[n-1][n-1]);
		 		lambdahi/=2;
		 		lambdalo/=2;
		 		{
		 			double delta=tau*(lambdahi-lambdalo)/(1-tau);
		 			
		 			if(tauhat*gamma>delta) delta=tauhat*gamma;
		 			delta-=lambdalo;
		 			if(delta<0) delta=0;
		 			if(delta<deltaprev) delta=deltaprev;
		 			if(delta>0){
		 				a[n-2][n-2]+=delta;
		 				a[n-1][n-1]+=delta;
		 				deltaprev=delta;
		 			}
		 			a[n-2][n-2]=sqrt(a[n-2][n-2]);
		 			a[n-1][n-2]/=a[n-2][n-2];
		 			a[n-1][n-1]=sqrt(a[n-1][n-1]-a[n-1][n-2]*a[n-1][n-2]);
		 		}
			}
			return 1;
		}
	}
}


 		
 		
