/* Copyright 1997-2001 Roger P. Woods, M.D. */
/* Modified 5/16/01 */

/* void qrstep()
 *
 * Performs one implicit QR step on the upper Hessenberg matrix a.
 * The shift is determined by p, q, and r, and is applied to rows and
 * columns nl through nu. The transformations are accumulated in v.
 *
 * Based on the FORTRAN routine QRSTEP:
 * Stewart GW. ACM Transactions on Mathematical Software 1976;2(3):275-280.
 *
 * Revised to eliminate a problem with division by zero.
 *
 * a is an upper Hessenberg matrix
 * v is the accumalated transformations
 * nl is the zero indexed lower limit and nu is the upper limit
 * n is the order of a
 */
 
#include "AIR.h"

void AIR_qrstep(double **a, double **v, double p, double q, double r, const unsigned int nl, const unsigned int nu, const unsigned int n)

{	
	{
		unsigned int i;
		
		for(i=nl+2;i<=nu;i++) a[i-2][i]=0.0;
	}
	if((nl+2)!=nu){
		{
			unsigned int i;
			
			for(i=nl+3;i<=nu;i++) a[i-3][i]=0.0;
		}
	}
	{
		unsigned int k;
		
		for(k=nl;k<nu;k++){
			/* Determine the transformation */
			
			double s;
			AIR_Boolean last=(k==(nu-1));
				
			if(k!=nl){
				p=a[k-1][k];
				q=a[k-1][k+1];
				r=0.0;
				if(!last) r=a[k-1][k+2];
				{
					double x=fabs(p)+fabs(q)+fabs(r);
					if(x==0.0) continue;
					p/=x;
					q/=x;
					r/=x;
					s=sqrt(p*p+q*q+r*r);
					if(s==0.0) continue;	/* Added this line */
					if(p<0.0) s*=-1;
					a[k-1][k]=-s*x;
				}
			}
			else{
				s=sqrt(p*p+q*q+r*r);
				if(s==0.0) continue;	/* Added this line */
				if(p<0.0) s*=-1;
				if (nl!=0) a[k-1][k]*=-1;
			}
			p+=s;
			{
				double x=p/s;
				double y=q/s;
				double z=r/s;
				
				q/=p;
				r/=p;
				
				/*Premultiply*/
				{
					unsigned int j;
					
					for(j=k;j<n;j++){
						p=a[j][k]+q*a[j][k+1];
						if(!last){
							p+=r*a[j][k+2];
							a[j][k+2]-=p*z;
						}
						a[j][k+1]-=p*y;
						a[j][k]-=p*x;
					}
				}
				/*Postmultiply*/
				{
					unsigned int j;
					
					j=k+3;
					if(nu<j) j=nu;
					{
						unsigned int i;
						
						for(i=0;i<=j;i++){
							p=x*a[k][i]+y*a[k+1][i];
							if(!last){
								p+=z*a[k+2][i];
								a[k+2][i]-=p*r;
							}
							a[k+1][i]-=p*q;
							a[k][i]-=p;
						}
					}
				}
				/*Accumulate the transformation in v*/
				{
					unsigned int i;
					
					for(i=0;i<n;i++){
						p=x*v[k][i]+y*v[k+1][i];
						if(!last){
							p+=z*v[k+2][i];
							v[k+2][i]-=p*r;
						}
						v[k+1][i]-=p*q;
						v[k][i]-=p;
					}
				}
			}
		}
	}
}
