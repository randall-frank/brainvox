/* Copyright 2000-2001 Roger P. Woods, M.D. */
/* Modified 5/22/01 */

/*
 * void logut()
 *
 * Solves for z12=frechet derivative in the direction d12
 *
 * x11 and x22 are upper triangular matrices, possibly of different sizes
 *
 * x and a are square and upper triangular
 *
 * storage2 needs to be a pointer to 20 matrices, dimensioned 2x2 (for n<=4)
 *
 */

#include "AIR.h"
#include <float.h>

AIR_Error AIR_exput(const unsigned int n, double **ar, double **ai, double **xr, double **xi, const double *toproot, const double *botroot, double ***storage2)
{
	double
		**x11r=*storage2++, /* 0 */
		**x11i=*storage2++, /* 1 */
	
		**x22r=*storage2++, /* 2 */
		**x22i=*storage2++, /* 3 */

		**a11r=*storage2++, /* 4 */
		**a11i=*storage2++, /* 5 */
	
		**a22r=*storage2++, /* 6 */
		**a22i=*storage2++, /* 7 */
	
		**a12r=*storage2++, /* 8 */
		**a12i=*storage2++; /* 9 */
	
	double und=pow(2.0,(DBL_MIN_EXP*(log((double)FLT_RADIX)/log(2.0))-1)/2);

	unsigned int scale=1;
	unsigned int numsquare=0;	
	
	double
		vsr[4],
		vsi[4];
	
	/* vs=exp(diag(X)) */
	{
		unsigned int i;
		
		for(i=0;i<n;i++){
		
			AIR_cexp(xr[i][i],xi[i][i],&vsr[i],&vsi[i]);
		}
	}
	
	{
		unsigned int iscale;
		double
			vr[4],
			vi[4];
		
		for(iscale=1;iscale<=2000;iscale++){			
			
			{
				unsigned int i;
				
				for(i=0;i<n;i++){
					
					AIR_cexp(xr[i][i]/scale, xi[i][i]/scale, &vr[i], &vi[i]);
				}
			}
			{
				AIR_Boolean toobig=FALSE;
				{
					unsigned int i;
					
					for(i=0;i<n;i++){
						
						if(sqrt(vr[i]*vr[i]+vi[i]*vi[i])<und){
							toobig=TRUE;
							break;
						}
					}
				}
				if(toobig){
					scale*=2;
					numsquare=iscale;
				}
				else break;
			}
		}
		{
			unsigned int j;
			
			for(j=0;j<n;j++){
			
				unsigned int i;
				
				for(i=0;i<n;i++){
					
					if(i==j){
						
						ar[i][i]=vr[i];
						ai[i][i]=vi[i];
						
						xr[i][i]/=scale;
						xi[i][i]/=scale;
					}
					else{
						ar[j][i]=0;
						ai[j][i]=0;
						
						xr[j][i]/=scale;
						xi[j][i]/=scale;
					}
				}
			}
		}
	}

	if(n==2){
		/* First 1x1 block */
		x11r[0][0]=xr[0][0];
		x11i[0][0]=xi[0][0];
		x22r[0][0]=xr[1][1];
		x22i[0][0]=xi[1][1];
		
		a11r[0][0]=ar[0][0];
		a11i[0][0]=ai[0][0];
		a22r[0][0]=ar[1][1];
		a22i[0][0]=ai[1][1];
		
		a12r[0][0]=xr[1][0];
		a12i[0][0]=xi[1][0];
				
		{
			AIR_Error errcode=AIR_expderivut(1,1,a12r,a12i,a11r,a11i,a22r,a22i,x11r,x11i,x22r,x22i,toproot,botroot,storage2);
			if(errcode!=0) return errcode;
		}
		
		ar[1][0]=a12r[0][0];
		ai[1][0]=a12i[0][0];
	}
	else if(n==3){
		/* First 1x1 block */
		x11r[0][0]=xr[0][0];
		x11i[0][0]=xi[0][0];
		x22r[0][0]=xr[1][1];
		x22i[0][0]=xi[1][1];
		
		a11r[0][0]=ar[0][0];
		a11i[0][0]=ai[0][0];
		a22r[0][0]=ar[1][1];
		a22i[0][0]=ai[1][1];
		
		a12r[0][0]=xr[1][0];
		a12i[0][0]=xi[1][0];
		
		{
			AIR_Error errcode=AIR_expderivut(1,1,a12r,a12i,a11r,a11i,a22r,a22i,x11r,x11i,x22r,x22i,toproot,botroot,storage2);
			if(errcode!=0) return errcode;
		}
		
		ar[1][0]=a12r[0][0];
		ai[1][0]=a12i[0][0];
		
		/* 2x1 block */
		x11r[0][0]=xr[0][0];
		x11r[0][1]=xr[0][1];
		x11r[1][0]=xr[1][0];
		x11r[1][1]=xr[1][1];
		
		x11i[0][0]=xi[0][0];
		x11i[0][1]=xi[0][1];
		x11i[1][0]=xi[1][0];
		x11i[1][1]=xi[1][1];
		
		x22r[0][0]=xr[2][2];
		x22i[0][0]=xi[2][2];
		
		a11r[0][0]=ar[0][0];
		a11r[0][1]=ar[0][1];
		a11r[1][0]=ar[1][0];
		a11r[1][1]=ar[1][1];
		
		a11i[0][0]=ai[0][0];
		a11i[0][1]=ai[0][1];
		a11i[1][0]=ai[1][0];
		a11i[1][1]=ai[1][1];
		
		a22r[0][0]=ar[2][2];
		a22i[0][0]=ai[2][2];
		
		a12r[0][0]=xr[2][0];
		a12i[0][0]=xi[2][0];
		a12r[0][1]=xr[2][1];
		a12i[0][1]=xi[2][1];
		{
			AIR_Error errcode=AIR_expderivut(2,1,a12r,a12i,a11r,a11i,a22r,a22i,x11r,x11i,x22r,x22i,toproot,botroot,storage2);
			if(errcode!=0) return errcode;
		}
		ar[2][0]=a12r[0][0];
		ai[2][0]=a12i[0][0];
		ar[2][1]=a12r[0][1];
		ai[2][1]=a12i[0][1];
	}
	else if(n==4){
	

		/* First 1x1 block */
		x11r[0][0]=xr[0][0];
		x11i[0][0]=xi[0][0];
		x22r[0][0]=xr[1][1];
		x22i[0][0]=xi[1][1];
		
		a11r[0][0]=ar[0][0];
		a11i[0][0]=ai[0][0];
		a22r[0][0]=ar[1][1];
		a22i[0][0]=ai[1][1];
		
		a12r[0][0]=xr[1][0];
		a12i[0][0]=xi[1][0];
		{
			AIR_Error errcode=AIR_expderivut(1,1,a12r,a12i,a11r,a11i,a22r,a22i,x11r,x11i,x22r,x22i,toproot,botroot,storage2);

			if(errcode!=0) return errcode;
		}
		
		ar[1][0]=a12r[0][0];
		ai[1][0]=a12i[0][0];
		
		/* Last 1x1 block*/
		x11r[0][0]=xr[2][2];
		x11i[0][0]=xi[2][2];
		x22r[0][0]=xr[3][3];
		x22i[0][0]=xi[3][3];
		
		a11r[0][0]=ar[2][2];
		a11i[0][0]=ai[2][2];
		a22r[0][0]=ar[3][3];
		a22i[0][0]=ai[3][3];
		
		a12r[0][0]=xr[3][2];
		a12i[0][0]=xi[3][2];

		{
			AIR_Error errcode=AIR_expderivut(1,1,a12r,a12i,a11r,a11i,a22r,a22i,x11r,x11i,x22r,x22i,toproot,botroot,storage2);

			if(errcode!=0) return errcode;
		}
		ar[3][2]=a12r[0][0];
		ai[3][2]=a12i[0][0];
		
		/* 2x2 block */
		x11r[0][0]=xr[0][0];
		x11r[0][1]=xr[0][1];
		x11r[1][0]=xr[1][0];
		x11r[1][1]=xr[1][1];
		
		x11i[0][0]=xi[0][0];
		x11i[0][1]=xi[0][1];
		x11i[1][0]=xi[1][0];
		x11i[1][1]=xi[1][1];
		
		x22r[0][0]=xr[2][2];
		x22r[0][1]=xr[2][3];
		x22r[1][0]=xr[3][2];
		x22r[1][1]=xr[3][3];
		
		x22i[0][0]=xi[2][2];
		x22i[0][1]=xi[2][3];
		x22i[1][0]=xi[3][2];
		x22i[1][1]=xi[3][3];
		
		a11r[0][0]=ar[0][0];
		a11r[0][1]=ar[0][1];
		a11r[1][0]=ar[1][0];
		a11r[1][1]=ar[1][1];
		
		a11i[0][0]=ai[0][0];
		a11i[0][1]=ai[0][1];
		a11i[1][0]=ai[1][0];
		a11i[1][1]=ai[1][1];
		
		a22r[0][0]=ar[2][2];
		a22r[0][1]=ar[2][3];
		a22r[1][0]=ar[3][2];
		a22r[1][1]=ar[3][3];
		
		a22i[0][0]=ai[2][2];
		a22i[0][1]=ai[2][3];
		a22i[1][0]=ai[3][2];
		a22i[1][1]=ai[3][3];
		
		a12r[0][0]=xr[2][0];
		a12r[0][1]=xr[2][1];
		a12r[1][0]=xr[3][0];
		a12r[1][1]=xr[3][1];
		
		a12i[0][0]=xi[2][0];
		a12i[0][1]=xi[2][1];
		a12i[1][0]=xi[3][0];
		a12i[1][1]=xi[3][1];
		

		{
			AIR_Error errcode=AIR_expderivut(2,2,a12r,a12i,a11r,a11i,a22r,a22i,x11r,x11i,x22r,x22i,toproot,botroot,storage2);

			if(errcode!=0) return errcode;
		}
		ar[2][0]=a12r[0][0];
		ar[2][1]=a12r[0][1];
		ar[3][0]=a12r[1][0];
		ar[3][1]=a12r[1][1];
		
		ai[2][0]=a12i[0][0];
		ai[2][1]=a12i[0][1];
		ai[3][0]=a12i[1][0];
		ai[3][1]=a12i[1][1];
	}
	else return AIR_INVALID_UT_ERROR;
	
	/* Square A the required number of times */
	{
		unsigned int j2;
		
		for(j2=0;j2<numsquare;j2++){
		
			AIR_cgemm('n','n',n,n,n,1.0,0.0,ar,ai,ar,ai,0.0,0.0,xr,xi);
			
			{
				unsigned int j;
				
				for(j=0;j<n;j++){
				
					unsigned int i;
					
					for(i=0;i<n;i++){
					
						ar[j][i]=xr[j][i];
						ai[j][i]=xi[j][i];
					}
				}
			}
		}
	}
	/* Restore most accurate assessment of the diagonal elements */
	{
		unsigned int i;
		
		for(i=0;i<n;i++){
		
			ar[i][i]=vsr[i];
			ai[i][i]=vsi[i];
		}
	}
	return 0;
}
