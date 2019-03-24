/* Copyright 2000-2001 Roger P. Woods, M.D. */
/* Modified 5/27/01 */

/*
 * rsftocsf()
 *
 * This is a modification of the routine LINPACK routine qzval
 * It assumes that B==I, which makes Q=Z'
 *
 * On entry, a must be a real matrix in Schur form
 *
 * Note that the first index of a is the column number
 *
 * On return, a is unchanged and Z'aZ gives the complex Schur form
 *
 */
 
 #include "AIR.h"
 
 void AIR_rsftocsf(const unsigned int n, double **a, double **zr, double **zi)
 
 {
  	/* Set Z=I */
  	{
  		unsigned int j;
  		
  		for(j=0;j<n;j++){
  		
  			unsigned int i;
  			
  			for(i=0;i<n;i++){

  				if(i==j) zr[j][i]=1.0;
  				else zr[j][i]=0.0;

  				zi[j][i]=0.0;
  			}
  		}
  	}

  	/* Use QZ algorithm to find Z */
  	
  	{
  	  	unsigned int isw=1;
  		unsigned int en;
  		
  		for(en=n-1;en!=0;en--){
  		  			
  			if(isw==2){
  				isw=3-isw;
  				continue;
  			}
  			{
  				unsigned int na=en-1;
  				
  				if(a[na][en]!=0){
  				
 	  				/* 2 by 2 block */
  					double an=fabs(a[na][na])+fabs(a[en][na])+fabs(a[na][en])+fabs(a[en][en]);
  					double a11=a[na][na]/an;
  					double a12=a[en][na]/an;
  					double a21=a[na][en]/an;
  					double a22=a[en][en]/an;
  					double a1,a1i,a2,a2i;
  					double e,ei;
  					
  					{
  						{
		  					double t;
		  			  					
		  					if(fabs(a11)<=fabs(a22)){
		  					
			  					e=a11*2;
			  					t=(a22-a11)*2;
			  				}
			  				else{
			  				
			  					e=a22*2;
			  					t=(a11-a22)*2;
			  				}
			  				{
			  					double c,d;
				  				{
					  				double s=a21*4;
					  				c=0.5*t;
					  				d=c*c+s*a12;
					  				if(d>0) d=0; /* round-off error can make d positive when real part of eigenvalue is very near zero */
				  				}
				  				/* Two complex roots */
				  				
				  				e+=c;
				  				ei=sqrt(-d);
			  				}
		  				}
		  				{
			  				double a11r=a11-e/2;
			  				double a11i=ei/2;
			  				double a12r=a12;
			  				double a12i=0;
			  				double a22r=a22-e/2;
			  				double a22i=ei/2;
			  				
			  				if(fabs(a11r)+fabs(a11i)+fabs(a12r)+fabs(a12i)>=fabs(a21)+fabs(a22r)+fabs(a22i)){
			  					a1=a12r;
			  					a1i=a12i;
			  					a2=-a11r;
			  					a2i=-a11i;
			  				}
			  				else{
			  					a1=a22r;
			  					a1i=a22i;
			  					a2=-a21;
			  					a2i=0;
			  				}
		  				}
	  				}
	  				/* Choose complex Z */
	  				{
	  					double cz, szr, szi;
	  					
		  				cz=sqrt(a1*a1+a1i*a1i);
		  				if(cz!=0){
		  					szr=(a1*a2+a1i*a2i)/cz;
		  					szi=(a1*a2i-a1i*a2)/cz;
		  					{
			  					double r=sqrt(cz*cz+szr*szr+szi*szi);
			  					cz/=r;
			  					szr/=r;
			  					szi/=r;
		  					}
		  				}
		  				else{
		  					szr=1;
		  					szi=0;
		  				}
		  				if(an>=(fabs(e)+ei)*2){
		  					a1=cz/2;
		  					a1i=0;
		  					a2=szr/2;
		  					a2i=szi/2;
		  				}
		  				else{
		  					a1=cz*a11+szr*a12;
		  					a1i=szi*a12;
		  					a2=cz*a21+szr*a22;
		  					a2i=szi*a22;
		  				}
		  				
		  				zr[en][en]=cz;
		  				zr[na][na]=cz;
		  				
		  				zr[en][na]=-szr;
		  				zi[en][na]=szi;
		  				
		  				zr[na][en]=szr;
		  				zi[na][en]=szi;
					}
					isw=3-isw;
  				}
  				else{
  					/* 1 by 1 block, one real root */
  					continue;
  				}
  			}
  		}
  	}
 }
