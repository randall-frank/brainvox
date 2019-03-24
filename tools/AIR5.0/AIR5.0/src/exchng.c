/* Copyright 1997-2001 Roger P. Woods, M.D. */
/* Modified 5/30/01 */

/*
 * Given the upper Hessenberg matrix a with consecutive
 * b1xb1 and b2xb2 diagonal blocks (b1,b2<=2)
 * starting at a[l][l], exchng produces a unitary
 * similarity transformation that exhcnages the blocks
 * along with their eigenvalues. The transformation
 * is accumulated in v. 
 *
 * Based on the FORTRAN routine EXCHNG:
 * Stewart GW. ACM Transactions on Mathematical Software 1976;2(3):275-280.
 *
 * a is the matrix whose blocks are to be interchanged
 * v is the array into which tranformations are accumulated
 * n is the order of a
 * l is the zero offset position of the blocks
 * b1 is the size of the first block (1 or 2)
 * b2 is the size of the second block (1 or 2)
 */

#include "AIR.h"
#include <float.h>

AIR_Error AIR_exchng(double **a, double **v, const unsigned int n, const unsigned int l, const unsigned int b1, const unsigned int b2)

{	
	if(b1==2 || b2==2){
		/* Compute matrix norm */
		double norm=0.0;
		{
			unsigned int k=0;
			unsigned int i;

			for(i=0;i<n;i++){

				unsigned int j;

				for(j=k;j<n;j++){
					norm+=fabs(a[j][i]);
				}
				k=i;
			}
		}

		if(b1==2){
			/* Interchange 2x2 and b2xb2 blocks */
			unsigned int m=l+2;
			double x=a[l+1][l+1];
			double y=a[l][l];
			double w=a[l][l+1]*a[l+1][l];

			if(b2==2) m++;

			AIR_qrstep(a,v,(double)1.0,(double)1.0,(double)1.0,l,m,n);
			{
				unsigned int it;

				for(it=0;it<30;it++){

					double p,q,r,s,z;
					z=a[l][l];
					r=x-z;
					s=y-z;
					p=(r*s-w)/a[l][l+1]+a[l+1][l];
					q=a[l+1][l+1]-z-r-s;
					r=a[l+1][l+2];
					s=fabs(p)+fabs(q)+fabs(r);
					p/=s;
					q/=s;
					r/=s;
					AIR_qrstep(a,v,p,q,r,l,m,n);
					if(fabs(a[m-2][m-1])<=DBL_EPSILON*(fabs(a[m-1][m-1])+fabs(a[m-2][m-2]))){
						a[m-2][m-1]=0.0;
						return 0;
					}
				}
			}
			return AIR_EXCHNG_CONVERGENCE_ERROR;
		}
		if(b2==2){
			/* Interchange 1x1 and 2x2 blocks */
			double x=a[l][l];
			AIR_qrstep(a,v,(double)1.0,(double)1.0,(double)1.0,l,l+2,n);
			{
				unsigned int it;

				for(it=0;it<30;it++){
					double p=a[l][l]-x;
					double q=a[l][l+1];
					double r=0.0;
					AIR_qrstep(a,v,p,q,r,l,l+2,n);
					if(fabs(a[l+1][l+2])<=DBL_EPSILON*(fabs(a[l+1][l+1])+fabs(a[l+2][l+2]))){
						a[l+1][l+2]=0.0;
						return 0;
					}
				}
			}
			return AIR_EXCHNG_CONVERGENCE_ERROR;
		}
	}
	/* Interchange 1x1 and 1x1 blocks */
	else{
		unsigned int l1=l+1;
		double q=a[l+1][l+1]-a[l][l];
		double p=a[l+1][l];
		{
			double r=fabs(p);

			if(fabs(q)>r) r=fabs(q);
			if(r==0.0) return 0;
			p/=r;
			q/=r;
		}
		{
			double r=sqrt(p*p+q*q);

			p/=r;
			q/=r;
		}
		{
			unsigned int j;

			for(j=l;j<n;j++){
				double s=p*a[j][l]+q*a[j][l+1];
				a[j][l+1]=p*a[j][l+1]-q*a[j][l];
				a[j][l]=s;
			}
		}
		{
			unsigned int i;

			for(i=0;i<=l1;i++){
				double s=p*a[l][i]+q*a[l+1][i];
				a[l+1][i]=p*a[l+1][i]-q*a[l][i];
				a[l][i]=s;
			}
		}
		{
			unsigned int i;

			for(i=0;i<n;i++){
				double s=p*v[l][i]+q*v[l+1][i];
				v[l+1][i]=p*v[l+1][i]-q*v[l][i];
				v[l][i]=s;
			}
		}
		a[l][l+1]=0.0;
	}
	return 0;
}
		
			
			

