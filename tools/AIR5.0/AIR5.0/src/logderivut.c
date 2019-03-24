/* Copyright 2000-2001 Roger P. Woods, M.D. */
/* Modified 5/27/01 */

/*
 * Solves for z12=frechet derivative in the direction z12
 *
 * On input, z12=a12
 * On output, z12=x12
 *
 * x11 and x22 are upper triangular matrices, possibly of different sizes
 *
 * x and a are square and upper triangular
 *
 */

#include "AIR.h"
#include <float.h>

#define PADE 8

AIR_Error AIR_logderivut(const unsigned int n1, const unsigned int n2, double **zr12, double **zi12, double **xr11, double **xi11, double **xr22, double **xi22, double **ar11, double **ai11, double **ar22, double **ai22, const double *toproot, const double *botroot, double ***storage2)

{
	unsigned int k;
		
	/* Find k */
	{
		unsigned int k11=0;
		unsigned int k22=0;
		{
			double temp=0.0;
			unsigned int j;
			
			for(j=0;j<n1;j++){
			
				unsigned int i;
			
				for(i=0; i<=j; i++){
				
					temp+=xr11[j][i]*xr11[j][i];
					temp+=xi11[j][i]*xi11[j][i];
				}
			}
			temp=log(sqrt(temp)+DBL_EPSILON)/log(2)+2;
			temp=ceil(temp);
			if(temp>0.0) k11=(unsigned int)temp;
		}
		{
			double temp=0.0;
			unsigned int j;
			
			for(j=0;j<n2;j++){
			
				unsigned int i;
			
				for(i=0; i<=j; i++){
				
					temp+=xr22[j][i]*xr22[j][i];
					temp+=xi22[j][i]*xi22[j][i];
				}
			}
			temp=log(sqrt(temp)+DBL_EPSILON)/log(2)+2;
			temp=ceil(temp);
			if(temp>0.0) k22=(unsigned int)temp;
		}
		if(k11>k22) k=k11;
		else k=k22;
	}
	/* Square root Sylvester cascade */
	{
		if(k!=0){
			AIR_sqrtut(n1,ar11,ai11,ar11,ai11);
			AIR_sqrtut(n2,ar22,ai22,ar22,ai22);
		}
		{
			unsigned int j;
			
			for(j=0;j<k;j++){

				{
					AIR_Error errcode=AIR_sylv(n1,ar11,ai11,n2,ar22,ai22,zr12,zi12,zr12,zi12);
					if(errcode!=0) return errcode;
				}
				{
					unsigned int j2;
					
					for(j2=0;j2<n2;j2++){
					
						unsigned int i;
						
						for(i=0;i<n1;i++){
						
							zr12[j2][i]*=2;
							zi12[j2][i]*=2;
						}
					}
				}
				
				if(j<k-1){
					AIR_sqrtut(n1,ar11,ai11,ar11,ai11);
					AIR_sqrtut(n2,ar22,ai22,ar22,ai22);
				}
			}
		}
	}
			
	/* Hyperbolic Tangent Sylvester Cascade */
	{

		{
			AIR_Error errcode=AIR_sylv(n1,ar11,ai11,n2,ar22,ai22,zr12,zi12,zr12,zi12);
			if(errcode!=0) return errcode;
		}
		{
			unsigned int j;
			
			for(j=0;j<n2;j++){
				
				unsigned int i;
				
				for(i=0; i<n1; i++){
				
					zr12[j][i]*=2;
					zi12[j][i]*=2;
				}
			}
		}

		{
			double scale=2;
			double **mr11=*storage2++; /* 0 */
			double **mi11=*storage2++; /* 1 */
			double **mr22=*storage2++; /* 2 */
			double **mi22=*storage2++; /* 3 */

			{
				unsigned int i;
				
				for(i=0;i<k;i++){
					scale*=2;
				}
			}
			{
				unsigned int j;
				
				for(j=0;j<n1;j++){
				
					unsigned int i;
					
					for(i=0;i<n1;i++){
					
						mr11[j][i]=xr11[j][i]/scale;
						mi11[j][i]=xi11[j][i]/scale;
					}
				}
			}
			{
				unsigned int j;
				
				for(j=0;j<n2;j++){
				
					unsigned int i;
					
					for(i=0;i<n2;i++){
					
						mr22[j][i]=xr22[j][i]/scale;
						mi22[j][i]=xi22[j][i]/scale;
					}
				}
			}
			{
				double **leftr=*storage2++;	/* 4 */
				double **lefti=*storage2++;	/* 5 */
				double **rightr=*storage2++; /* 6 */
				double **righti=*storage2++; /* 7 */
				double **rr=*storage2++;		/* 8 */
				double **ri=*storage2++;		/* 9 */
				
				{
					unsigned int i;
					
					for(i=0;i<PADE;i++){
					
						{
							unsigned int j;
							
							for(j=0;j<n1;j++){
							
								unsigned int i2;
								
								for(i2=0;i2<n1;i2++){
								
									if(i2==j){
										leftr[j][i2]=.5;
										lefti[j][i2]=0;
									}
									else{
										leftr[j][i2]=0;
										lefti[j][i2]=0;
									}
									{
										double tempr;
										double tempi;
										
										(void)AIR_cdiver(mr11[j][i2],mi11[j][i2],0.0,botroot[i],&tempr,&tempi);
									
										leftr[j][i2]-=tempr;
										lefti[j][i2]-=tempi;
									}
								}		
							}
						}

						{
							{
								unsigned int j;
								
								for(j=0;j<n2;j++){
								
									unsigned int i2;
									
									for(i2=0;i2<n2;i2++){
									
										if(i2==j){
											rightr[j][i2]=.5;
											righti[j][i2]=0;
										}
										else{
											rightr[j][i2]=0;
											righti[j][i2]=0;
										}
										{
											double tempr;
											double tempi;
																						
											(void)AIR_cdiver(mr22[j][i2],mi22[j][i2],0.0,botroot[i],&tempr,&tempi);
											
											rightr[j][i2]+=tempr;
											righti[j][i2]+=tempi;
										}
									}		
								}
							}

							AIR_cgemm('n','n',n1,n2,n1,1.0,0.0,leftr,lefti,zr12,zi12,0.0,0.0,rr,ri);
							AIR_cgemm('n','n',n1,n2,n2,1.0,0.0,zr12,zi12,rightr,righti,1.0,0.0,rr,ri);

							{
								unsigned int j;
								
								for(j=0;j<n1;j++){
								
									unsigned int i2;
									
									for(i2=0;i2<n1;i2++){
									
										if(i2==j){
											leftr[j][i2]=.5;
											lefti[j][i2]=0;
										}
										else{
											leftr[j][i2]=0;
											lefti[j][i2]=0;
										}
										{
											double tempr;
											double tempi;
											
											(void)AIR_cdiver(mr11[j][i2],mi11[j][i2],0.0,toproot[i],&tempr,&tempi);
											
											leftr[j][i2]-=tempr;
											lefti[j][i2]-=tempi;
										}
									}		
								}
							}

							{
								unsigned int j;
								
								for(j=0;j<n2;j++){
								
									unsigned int i2;
									
									for(i2=0;i2<n2;i2++){
									
										if(i2==j){
											rightr[j][i2]=.5;
											righti[j][i2]=0;
										}
										else{
											rightr[j][i2]=0;
											righti[j][i2]=0;
										}
										{
											double tempr;
											double tempi;
											
											(void)AIR_cdiver(mr22[j][i2],mi22[j][i2],0.0,toproot[i],&tempr,&tempi);
											
											rightr[j][i2]+=tempr;
											righti[j][i2]+=tempi;
										}
									}		
								}
							}
							{
								AIR_Error errcode=AIR_sylv(n1,leftr,lefti,n2,rightr,righti,rr,ri,zr12,zi12);
								if(errcode!=0) return errcode;
							}
						}
					}
				}
			}
		}
	}
	return 0;
}
