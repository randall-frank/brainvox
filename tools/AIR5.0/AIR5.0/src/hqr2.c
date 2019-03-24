/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/30/01 */


/*
 * This routine will compute the eigenvalues and eigenvectors of a real
 * upper Hessenberg matrix using the QR method
 *
 * It is based upon the SLATEC (EISPACK) Fortran routine with the same
 * name. In porting it to C, I have modified the array indices so that
 * they start at zero instead of one. This convention must be
 * adopted by the calling routine as well, so the variables low and igh
 * should be 1 less than they would be in the Fortran version.
 *
 * The return value of ierr will be -1 instead of zero if all goes
 * well in the routine.
 *
 * I have also converted the routine from floats to doubles.
 *
 * I have reverted to the use of the machine epsilon as implemented in the
 * original ALGOL procedure--elimination of epsilon appears to have been
 * a workaround for the fact that FORTRAN 77 did not provide easy access
 * to epsilon.
 *
 * The routine should otherwise be an exact port of the Fortran version.
 *
 * The Fortran routine itself was a translation of an ALGOL procedure
 * with the same name, NUM. MATH. 16, 181-204 (1970) by Peters and
 * Wilkinson. HANDBOOK FOR AUTO. COMP., VOL.II-LINEAR ALGEBRA, 372-
 * 395 (1971). The Fortran version was written by Smith, B.T., et.al.
 * and the Fortran version is in the public domain.
 *
 * This subroutine finds the eigenvalues and eigenvectors of a REAL
 * UPPER Hessenberg matrix by the QR method. The eigenvectors of a
 * REAL GENERAL matrix can also be found if ELMHES and ELTRAN or ORTHES
 * have been used to reduce this general matrix to Hessenberg form and
 * to accumulate the similarity transformations.
 *
 * On INPUT
 *	N is the order of the matrix H. N is an INTEGER variable
 *
 *	LOW and IGH are two INTEGER variables determined by the
 *	  balancing subroutine BALANC. If BALANC has not been used,
 *	  set LOW=0 and IGH=N-1
 *
 *	H contains the upper Hessenberg matrix. H is a two-dimensional
 *	  DOUBLE array, dimensioned H[N][N]
 *
 *	Z contains the transformation matrix produced by ELTRAN
 * 	  after the reduction by ELMHES, or by ORTRAN after the
 *	  reduction by ORTHES, if performed. If the eigenvectors
 *	  of the Hessenberg matrix are desired, Z must contain the
 *	  identity matrix. Z is a two-dimensional DOUBLE array,
 *	  dimensioned Z[N][N]
 *
 * On OUTPUT
 *	H has been destroyed
 *
 *	WR and WI contain the real and imaginary parts, respectively,
 *	  of the eigenvalues. The eigenvalues are unordered except
 *	  that the complex conjugate pairs of values appear
 *	  consecutively with the eigenvalue having the positive
 *	  imaginary part first. If an error exit is made, the
 *	  eigenvalues should be correct for indices IERR+1,IERR+2,
 *	  ...,N-1. WR and WI are one-dimensional DOUBLE arrays,
 *	  dimensioned WR[N] and WI[N].
 *
 * 	Z contains the real and imaginary parts of the eigenvectors.
 *	  If the J-th eigenvalue is real, the J-th column of Z
 *	  contains it eigenvector. If the J-th eigenvalue is complex
 *	  with positive imaginary part, the J-th and (J+1)-th columns
 *	  of Z contain the real and imaginary parts of its eigenvector.
 *	  The eigenvectors are unnormalized. If an error exit is made,
 *	  none of the eigenvectors has been found.
 *
 *	IERR is an INTEGER flag set to
 *	  N for normal return
 *	  J<N if the Jth eigenvalue has not been determined after a total
 *	    of 30*N iterations. The eigenvalues should be correct for
 *	    indices IERR+1,IERR+2,...,N-1, but no eigenvectors are
 *	    computed.
 *
 * Calls CDIV for complex division
 *
 * References: B.T. Smith, J.M. Boyle, J.J. Dongarra, B.S. Garbow,
 *    Y. Ikebe, V.C. Klema and C.B. Moler, Matrix Eigen-system
 *    Routines--EISPACK Guide, Springer-Verlag, 1976
 *
 * Modified to prevent division by zero.
 */

#include "AIR.h"
#include <float.h>

void AIR_hqr2(const unsigned int n, const unsigned int low, const unsigned int igh, double **h, double *wr, double *wi, double **z, unsigned int *ierr)

/* low and igh can take values from 0 to n-1 */

{
	/*Vectors of isolated roots*/

	*ierr=n;
	{
		double norm=0.0;
		unsigned int en=igh;	/*Can take values 0 to n-1*/
		{
			unsigned int k=0;
			unsigned int i;
			/*Store roots isolated by balanc and compute matrix norm*/
			
			for(i=0;i<n;i++){
				{
					unsigned int j;
					
					for(j=k;j<n;j++){
						norm+=fabs(h[j][i]);
					}/*40*/
				}
				k=i;
				if(i<low || i>igh){
					wr[i]=h[i][i];
					wi[i]=0.0;
				}
			}/*50*/
		}
		/*Search for next eigenvalues*/
		/*60*/

		if(en>=low){
			unsigned int its=0;
			unsigned int itn=30*n;
			double t=0.0;
			/*70*/

			for(;;){	/*Do this forever*/
				/*Look for single small sub-diagonal element*/
				unsigned int l;
				
				for(l=en;l>low;l--){
					/* Revert to ALGOL version */
					if(fabs(h[l-1][l])<=DBL_EPSILON*(fabs(h[l-1][l-1])+fabs(h[l][l]))) break;
				}/*80*//*100*/
				/*Form shift*/
				{				
					double x=h[en][en];

					if(l!=en){	/* OK to address en-1  since l<=en-l */
					
						unsigned int na=en-1;

						double y=h[na][na];
						double w=h[na][en]*h[en][na];

						if(l!=na){	/* OK to address en-2 since l<=en-2 */
						
							unsigned int enm2=en-2;

							if(itn--==0){
								/*Set error--no convergence after 30*n iterations*/
								/*1000*/
								*ierr=en;
								return;
							}
							if(its==10||its==20){
								/*Form exceptional shift*/
								t+=x;
								{
									unsigned int i;
									
									for(i=low;i<=en;i++){
										h[i][i]-=x;
									}/*120*/
								}
								{
									double s=fabs(h[na][en])+fabs(h[enm2][na]);
									x=0.75*s;
									y=x;
									w=-0.4375*s*s;
								}
							}/*130*/
							its++;
							/*Look for two consecutive small sub-diagonal elements*/
							{
								unsigned int m;
								double p;
								double q;
								double r;
								
								for(m=enm2;;m--){	/* m>=l initially */
								
									double zz=h[m][m];
									r=x-zz;
									{
										double s=y-zz;
										p=(r*s-w)/h[m][m+1]+h[m+1][m];
										q=h[m+1][m+1]-zz-r-s;
										r=h[m+1][m+2];
									}
									{
										double s=fabs(p)+fabs(q)+fabs(r);
										p/=s;
										q/=s;
										r/=s;
									}
									if(m==l) break;
									/* Revert to ALGOL version */
									if(fabs(h[m-1][m])*(fabs(q)+fabs(r))<=DBL_EPSILON*fabs(p)*(fabs(h[m-1][m-1])+fabs(zz)+fabs(h[m+1][m+1]))) break;
								}/*140*//*150*/
                                                                
								{						
									unsigned int mp2=m+2;
									unsigned int i;
									
									for(i=mp2;i<=en;i++){
										h[i-2][i]=0.0;
										if(i!=mp2){
											h[i-3][i]=0.0;
										}
									}/*160*/
									
								}
								/*Double QR step involving rows l to en and columns m to en*/
								{
									unsigned int k;
									
									for(k=m;k<=na;k++){

										double s;
										AIR_Boolean notlas=(k!=na);
										
											if(k!=m){
												p=h[k-1][k];
												q=h[k-1][k+1];
												r=0.0;
												if(notlas) r=h[k-1][k+2];
												{
													double x=fabs(p)+fabs(q)+fabs(r);
													if(x==0.0) continue;
													p/=x;
													q/=x;
													r/=x;
													s=sqrt(p*p+q*q+r*r);
													if(s==0.0) continue;		/* Added this line */
													if(p<0.0) s*=-1;
													h[k-1][k]=-s*x;		/*180*/
												}
											}/*170*/
											else{
												s=sqrt(p*p+q*q+r*r);
												if(s==0.0) continue;		/* Added this line */
												if(p<0.0) s*=-1;
												if(l!=m) h[k-1][k]*=-1;	/*190*/
											}
											p+=s;
										{
											double x=p/s;
											double y=q/s;
											double zz=r/s;
											
											q/=p;
											r/=p;
											/*Row modification*/
											{
												unsigned int j;
												
												for(j=k;j<n;j++){
													p=h[j][k]+q*h[j][k+1];
													if(notlas){
														p+=r*h[j][k+2];
														h[j][k+2]-=p*zz;
													}				/*200*/
													h[j][k+1]-=p*y;
													h[j][k]-=p*x;
												}					/*210*/
											}
											{
												unsigned int j;
												
												if(en<k+3) j=en;
												else j=k+3;
												/*Column modification*/
												{
													unsigned int i;
													
													for(i=0;i<=j;i++){
														p=x*h[k][i]+y*h[k+1][i];
														if(notlas){
															p+=zz*h[k+2][i];
															h[k+2][i]-=p*r;
														} /*220*/
														h[k+1][i]-=p*q;
														h[k][i]-=p;
													} /*230*/
												}
											}
											/*Accumulate transformations*/
											{
												unsigned int i;
												
												for(i=low;i<=igh;i++){
													p=x*z[k][i]+y*z[k+1][i];
													if(notlas){
														p+=zz*z[k+2][i];
														z[k+2][i]-=p*r;
													} /*240*/
													z[k+1][i]-=p*q;
													z[k][i]-=p;
												} /*250*/
											}
										}
									} /*260*/
								}
							}
						}
						else{
							/*Two roots found (l==na)*/
							/*280*/
							double p=(y-x)/2.0;
							double q=p*p+w;
							double zz=sqrt(fabs(q));
							h[en][en]=x+t;
							x=h[en][en];
							h[na][na]=y+t;
							if(q>=0.0){
								/*Real pair*/
								if(p<0) zz=p-zz;
								else zz=p+zz;
								wr[na]=x+zz;
								wr[en]=wr[na];
								if(zz!=0.0) wr[en]=x-w/zz;
								wi[na]=0.0;
								wi[en]=0.0;
								x=h[na][en];
								{
									double s=fabs(x)+fabs(zz);
									p=x/s;
									q=zz/s;
								}
								{
									double r=sqrt(p*p+q*q);
									p/=r;
									q/=r;
								}
								/*Row modification*/
								{
									unsigned int j;
									
									for(j=na;j<n;j++){
										zz=h[j][na];
										h[j][na]=q*zz+p*h[j][en];
										h[j][en]=q*h[j][en]-p*zz;
									}/*290*/
								}
								/*Column modification*/
								{
									unsigned int i;
									
									for(i=0;i<=en;i++){
										zz=h[na][i];
										h[na][i]=q*zz+p*h[en][i];
										h[en][i]=q*h[en][i]-p*zz;
									}/*300*/
								}
								/*Accumulate transformations*/
								{
									unsigned int i;
									
									for(i=low;i<=igh;i++){
										zz=z[na][i];
										z[na][i]=q*zz+p*z[en][i];
										z[en][i]=q*z[en][i]-p*zz;
									}/*310*/
								}
							}
							else{
								/*320*/
								/*Complex pair*/
								wr[na]=x+p;
								wr[en]=x+p;
								wi[na]=zz;
								wi[en]=-zz;
							}/*330*/
							if(en<=low+1) break;
							en-=2;
							its=0;
						}
					
					}
					else{
						/*One root found (l==en)*/
						/*270*/
						h[en][en]=x+t;
						wr[en]=h[en][en];
						wi[en]=0.0;
						if(en<=low) break;
						en--;
						its=0;
					}
				}
			}/*End of loop that forever returns you to 70*/	
		}/*340*/
		/*All roots found. Backsubstitute to find vectors of upper triangular form*/

		if(norm==0.0) return;
		
		/* zz, r and s as computed above do not enter into the computations below */
		/*  even though they look like they could. This is because complex  */
		/*  conjugate pairs are always stored with the positive member first */
		{
			unsigned int en;
			
			for(en=n;en--!=0;){
				double p=wr[en];
				double q=wi[en];
				if(q==0.0){
					/*600*/
					/*Real vector*/
					h[en][en]=1.0;
					if(en!=0){
						unsigned int m=en;
						unsigned int na=en-1;
                                        
						unsigned int i;
						
						for(i=en;i--!=0;){
						
							double zz=h[i][i]-p;
							double s=h[en][i];
							if(m<=na){
								unsigned int j;
								
								for(j=m;j<=na;j++){
									s+=h[j][i]*h[en][j];
								}/*610*/
							}/*620*/
					
							if(wi[i]==0.0){
								/* Real */
								m=i;
								{
									double t=zz;
									if(t==0.0){
										t=norm;
										/*632*/
										do{
												t*=0.5;
										}while(norm+t>norm);
										t*=2.0;
									}/*635*/
									h[en][i]=-s/t;
								}/*640*/
								continue;
							}
							/* Imaginary */
							{
								i--;
								{			
									double w=h[i][i]-p;
									double r=h[en][i];
									if(m<=na){
										unsigned int j;
										
										for(j=m;j<=na;j++){
											r+=h[j][i]*h[en][j];
										}/*610*/
									}/*620*/

									m=i;
									{
										double x=h[i+1][i];
										double y=h[i][i+1];
										q=(wr[i]-p)*(wr[i]-p)+wi[i]*wi[i];
										{
											double t=(x*s-zz*r)/q;
											h[en][i]=t;
											if(fabs(x)>fabs(zz)){
												h[en][i+1]=(-r-w*t)/x;
											}/*650*/
											else{
												h[en][i+1]=(-s-y*t)/zz;
											}
										}
									}
								}
							}
						}/*700*/
					}
				}
				else if(q<0.0){
				
					/*710*/
					/*Complex vector*/
					
					unsigned int na=en-1;
					
					/*Last vector component chosen imaginary so that eigenvector matrix is triangular*/
					if(fabs(h[na][en])>fabs(h[en][na])){
						h[na][na]=q/h[na][en];
						h[en][na]=-(h[en][en]-p)/h[na][en];

					}/*720*/
					else{
						(void)AIR_cdiver(0.0,-h[en][na],h[na][na]-p,q,&h[na][na],&h[en][na]);
					}/*730*/
					h[na][en]=0.0;
					h[en][en]=1.0;
					if(na!=0){
						unsigned int m=na;
						unsigned int i;
						
						for(i=na;i--!=0;){
						
							double zz=h[i][i]-p;
							double r=0.0;
							double s=h[en][i];
							
							{
								unsigned int j;
								
								for(j=m;j<=na;j++){
									r+=h[j][i]*h[na][j];
									s+=h[j][i]*h[en][j];
								}/*760*/
							}
					
							if(wi[i]==0.0){
								/* Real */
								m=i;
								(void)AIR_cdiver(-r,-s,zz,q,&h[na][i],&h[en][i]);
								continue;	/* 780 */
							}
							i--;
							{                                        
								double w=h[i][i]-p;
								double ra=0.0;
								double sa=h[en][i];
								{
									unsigned int j;
									
									for(j=m;j<=na;j++){
											ra+=h[j][i]*h[na][j];
											sa+=h[j][i]*h[en][j];
									}/*760*/
								}
								m=i;
								{
									/*Solve complex equations*/
									
									double x=h[i+1][i];
									double y=h[i][i+1];
									double vr=(wr[i]-p)*(wr[i]-p)+wi[i]*wi[i]-q*q;
									double vi=(wr[i]-p)*2.0*q;
									
									if(vr==0.0 && vi==0.0){
										/* Revert to ALGOL version */
										vr=DBL_EPSILON*norm*(fabs(w)+fabs(q)+fabs(x)+fabs(y)+fabs(zz));
									}/*783*/
									(void)AIR_cdiver(x*r-zz*ra+q*sa,x*s-zz*sa-q*ra,vr,vi,&h[na][i],&h[en][i]);
									if(fabs(x)>fabs(zz)+fabs(q)){
										h[na][i+1]=(-ra-w*h[na][i]+q*h[en][i])/x;
										h[en][i+1]=(-sa-w*h[en][i]-q*h[na][i])/x;

									}/*785*/
									else{
										(void)AIR_cdiver(-r-y*h[na][i], -s-y*h[en][i], zz, q, &h[na][i+1], &h[en][i+1]);
									}
								}
							}
						}/*790*/
					}
				}
			}/*800*/
		}
	}

	/*Vectors of isolated roots*/
	{
		unsigned int i;
		
		for(i=0;i<n;i++){
			if(i<low || i>igh){
				{
					unsigned int j;
					
					for(j=0;j<n;j++){
						z[j][i]=h[j][i];
					}/*820*/
				}
			}
		}/*840*/
	}
	/*Multiply by transformation matrix to give vectors of original full matrix*/
	{
		unsigned int j;
		
		for(j=n;j--!=low;){
		
			unsigned int m;
			
			if(j<igh) m=j;
			else m=igh;
			{
				unsigned int i;
				
				for(i=low;i<=igh;i++){
					double zz=0.0;
					{
						unsigned int k;
						
						for(k=low;k<=m;k++){
							zz+=z[k][i]*h[j][k];
						}/*860*/
					}
					z[j][i]=zz;
				}
			}
		}/*880*/
	}
}/*1001*/
