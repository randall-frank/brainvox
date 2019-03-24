/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/16/01 */ 


/* double qvderivsLS12()
 *
 * For the least squares cost function (without intensity scaling),
 * this routine computes the cost function and its first and second
 * derivatives with respect to the spatial transformation parameters.
 *
 * Returns the cost function
 */

#include "AIR.h"

#define PARAM_INT 12
#define PARAM_INT_H PARAM_INT*(PARAM_INT+1)/2

#define BLOCK0 0	/* x domain */
#define BLOCK1 4	/* y domain */
#define BLOCK2 8	/* z domain */
#define BLOCK3 12	/* end of z domain */

double AIR_qvderivsLS12(const unsigned int spatial_parameters, double **es, double ***des, double ****ees, const unsigned int samplefactor, AIR_Pixels ***pixel2, const struct AIR_Key_info *stats2, AIR_Pixels ***pixel5, const struct AIR_Key_info *stats5, const AIR_Pixels threshold5, double *dcff, double **ecff, /*@unused@*/ unsigned int *count, /*@unused@*/ double *mean, /*@unused@*/ double *square, /*@unused@*/ double **dmean, /*@unused@*/ double **dsquare, /*@unused@*/ double ***emean, /*@unused@*/ double ***esquare, /*@unused@*/ const unsigned int partitions, /*@unused@*/ AIR_Pixels max_actual_value, /*@unused@*/ double scale, /*@unused@*/ const AIR_Boolean forward, unsigned int *error)

{
	unsigned long int counttotal=0;

	double 		cf=0.0;		/* cost function */
	double 		dcf[PARAM_INT];	/*partial derivatives of cost function with respect to parameters*/
	double 		ecfitems[PARAM_INT_H];
	
	double		dxyz[BLOCK1];

	double x_max1=stats2->x_dim-1.0;
	double y_max1=stats2->y_dim-1.0;
	double z_max1=stats2->z_dim-1.0;

	unsigned int x_dim2=stats5->x_dim;
	unsigned int y_dim2=stats5->y_dim;
	unsigned int z_dim2=stats5->z_dim;

	double		e00,e01,e02,e10,e11,e12,e20,e21,e22,e30,e31,e32;

	double		**ep[PARAM_INT];
	double		*dp[PARAM_INT];
	
	{
		double *dcft=dcf;
		double *ecfts=ecfitems;
		unsigned int t;
		
		for(t=0; t<PARAM_INT; t++, dcft++){
			
			*dcft=0;
			
			{
				unsigned int s;
				
				for (s=0; s<=t; s++, ecfts++){
				
					*ecfts=0;
					
				}
			}
		}
	}

	e00=es[0][0];
	e01=es[0][1];
	e02=es[0][2];
	e10=es[1][0];
	e11=es[1][1];
	e12=es[1][2];
	e20=es[2][0];
	e21=es[2][1];
	e22=es[2][2];
	e30=es[3][0];
	e31=es[3][1];
	e32=es[3][2];

	/*Examine pixels of standard file at samplefactor interval*/
	/*Note that it is assumed here that pixel5[z_dim][y_dim][x_dim] refers to the*/
	/* same pixel as *(pixel5[0][0]+z_dim*y_dim*x_dim), i.e. that all the pixels */
	/* are represented in a contiguous block of memory--see the routine*/
	/* "create_volume.c" for an illustration of how this is assured*/

	/*ARRAY STRUCTURE ASSUMPTIONS MADE HERE*/
	{
		unsigned long int r;
		unsigned long int r_term=x_dim2*y_dim2*z_dim2;

		AIR_Pixels *rr;
		
		for (r=0, rr=**pixel5; r<r_term; r+=samplefactor, rr+=samplefactor){

			/*Verify that pixel5>threshold*/
			if(*rr<=threshold5) continue;	
			{
				signed int n0,n1,n2,n3,n4,n5,n6,n7;
				double a,b,c,d,e,f;
				double pix3=(double)*rr;
				
				#include "findcoords12.cf"
	
				{				
					/*Calculate the trilinear interpolated voxel value*/
	
					double pix4=n0*d*e*f+n1*a*e*f+n2*d*b*f+n3*a*b*f+n4*d*e*c+n5*a*e*c+n6*d*b*c+n7*a*b*c;
	
					/*Some intermediate values needed to calculate derivatives efficiently*/
	
					double dxpix4=((e*f)*(n1-n0)+(b*f)*(n3-n2)+(c*e)*(n5-n4)+(b*c)*(n7-n6));
					double dypix4=((d*f)*(n2-n0)+(a*f)*(n3-n1)+(c*d)*(n6-n4)+(a*c)*(n7-n5));
					double dzpix4=((d*e)*(n4-n0)+(a*e)*(n5-n1)+(b*d)*(n6-n2)+(a*b)*(n7-n3));
	
					/*Calculate the square of the difference*/
					cf+=pix4*pix4-2.0*pix4*pix3+pix3*pix3;
	
	
					/*Calculate derivatives that are nonzero*/
	
					/*First derivatives*/
					{
						unsigned int t;
						double *dxyzt=dxyz;
	
						double *dcft1=dcf;
						double *dcft2=dcf+BLOCK1;
						double *dcft3=dcf+BLOCK2;
						
						for(t=BLOCK0; t<BLOCK1; t++, dcft1++, dcft2++, dcft3++, dxyzt++){
						
							/* dcf[t]+=2.0*(pix4-pix3)*dxpix4*dx[t];*/
							*dcft1+=2.0*(pix4-pix3)*dxpix4**dxyzt;		
							
							/* dcf[t+BLOCK1]+=2.0*(pix4-pix3)*dypix4*dy[t+BLOCK1];*/
							*dcft2+=2.0*(pix4-pix3)*dypix4**dxyzt;
							
							/* dcf[t+BLOCK2]+=2.0*(pix4-pix3)*dzpix4*dz[t+BLOCK2];*/
							*dcft3+=2.0*(pix4-pix3)*dzpix4**dxyzt;
						}
					}
								
					/*Second derivatives*/
					{
						double dydx[BLOCK1*BLOCK1];
						double dxdx[BLOCK1*(BLOCK1+1)/2];
	
						{
							double *dytdxs=dydx;
							double *dxtdxs=dxdx;
							
							double *dxyzt=dxyz;
							
							unsigned int t;
							
							for(t=BLOCK0; t<BLOCK1; t++, dxyzt++){
							
								double *dxyzs=dxyz;
								
								unsigned int s;
								
								for(s=BLOCK0; s<=t; s++, dxyzs++, dxtdxs++, dytdxs++){
								
									*dxtdxs=*dytdxs=2.0**dxyzt**dxyzs;
								}
								for(; s<BLOCK1; s++, dxyzs++, dytdxs++){
								
									*dytdxs=2.0**dxyzt**dxyzs;
								}
							}
						}
						{
							double *ecfts=ecfitems;
							unsigned int t;
							
							{
								double *dxtdxs=dxdx;
								
								for(t=BLOCK0; t<BLOCK1; t++){
								
									unsigned int s;
									
									for(s=BLOCK0; s<=t; s++, ecfts++, dxtdxs++){
									
										/* ecf[t][s]+=2.0*dx[t]*dx[s]*dxpix4*dxpix4;*/
										*ecfts+=*dxtdxs*dxpix4*dxpix4;
									}
	
								}
							}
							{
								double *dytdxs=dydx;
								double *dytdys=dxdx;
								
								for(;t<BLOCK2;t++){
								
									unsigned int s;
										
									for(s=BLOCK0; s<BLOCK1; s++, ecfts++, dytdxs++){
									
										/* ecf[t][s]+=2.0*dy[t]*dx[s]*(dxpix4*dypix4);*/
										*ecfts+=*dytdxs*(dxpix4*dypix4);
									}
	
									for(; s<=t; s++, ecfts++, dytdys++){
									
										/* ecf[t][s]+=2.0*dy[t]*dy[s]*dypix4*dypix4;*/
										*ecfts+=*dytdys*dypix4*dypix4;
									}
								
								}
							}
							{
								double *dztdxs=dydx;
								double *dztdys=dydx;
								double *dztdzs=dxdx;
														
								for(;t<BLOCK3;t++){
								
									unsigned int s;
	
									for(s=BLOCK0; s<BLOCK1; s++, ecfts++, dztdxs++){
									
										/* ecf[t][s]+=2.0*dz[t]*dx[s]*(dxpix4*dzpix4));*/
										*ecfts+=*dztdxs*(dxpix4*dzpix4);
									}
	
									for(; s<BLOCK2; s++, ecfts++, dztdys++){
									
										/* ecf[t][s]+=2.0*dz[t]*dy[s]*(dypix4*dzpix4);*/
										*ecfts+=*dztdys*(dypix4*dzpix4);
									}
	
									for(; s<=t; s++, ecfts++, dztdzs++){
									
										/* ecf[t][s]+=2.0*dz[t]*dz[s]*dzpix4*dzpix4;*/
										*ecfts+=*dztdzs*dzpix4*dzpix4;
									}
								}
							}
						}
					}
					counttotal++;
				}
			}
		}
	}

	/*Calculate the weighted, normalized standard deviation and its derivatives*/

	if(counttotal==0){
		(*error)++;
		return 0.0;
	}

	/*Normalize by the number of voxels*/

	cf/=counttotal;
	{
		double *dcft=dcf;
		double *ecfts=ecfitems;
		unsigned int t;
		
		for(t=0; t<PARAM_INT; t++, dcft++){
		
			*dcft/=counttotal;
			
			{
				unsigned int s;
				
				for(s=0; s<=t; s++, ecfts++){
				
					/* ecf[t][s]/=counttotal; */
					*ecfts/=counttotal;
				}
			}
		}
	}

	{
		unsigned int t;
		
		for(t=BLOCK0; t<BLOCK1; t++){
		
			dp[t]=des[t][0];
			ep[t]=ees[t][0];
		}
		{
			unsigned int r;
			
			for(r=0; t<BLOCK2; t++, r++){
			
				dp[t]=des[r][1];
				ep[t]=ees[r][1];
			}
		}
		{
			unsigned int r;
			
			for(r=0; t<BLOCK3; t++, r++){
			
				dp[t]=des[r][2];
				ep[t]=ees[r][2];
			}
		}
	}
	#include "param_chain_rule.cf"

	return cf;
}
