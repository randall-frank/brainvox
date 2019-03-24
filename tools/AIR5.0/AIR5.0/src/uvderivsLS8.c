/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/16/01 */ 


/* double uvderivsLS8()
 *
 * For the least squares cost function (without intensity scaling),
 * this routine computes the cost function and its first and second
 * derivatives with respect to the spatial transformation parameters.
 *
 * Returns the cost function
 */

#include "AIR.h"

#define PARAM_INT 8
#define PARAM_INT_H PARAM_INT*(PARAM_INT+1)/2

#define BLOCK0 0	/* x domain */
#define BLOCK1 3	/* y domain */
#define BLOCK2 6	/* perspective domain */
#define BLOCK3 8	/* end of perspective domain */

double AIR_uvderivsLS8(const unsigned int spatial_parameters, double **es, double ***des, double ****ees, const unsigned int samplefactor, AIR_Pixels ***pixel2, const struct AIR_Key_info *stats2, AIR_Pixels ***pixel5, const struct AIR_Key_info *stats5, const AIR_Pixels threshold5, double *dcff, double **ecff, /*@unused@*/ unsigned int *count, /*@unused@*/ double *mean, /*@unused@*/ double *square, /*@unused@*/ double **dmean, /*@unused@*/ double **dsquare, /*@unused@*/ double ***emean, /*@unused@*/ double ***esquare, /*@unused@*/ const unsigned int partitions, /*@unused@*/ AIR_Pixels max_actual_value, /*@unused@*/ double scale, /*@unused@*/ const AIR_Boolean forward, unsigned int *error)

{
	unsigned long int counttotal=0;

	double 		cf=0.0;		/* cost function */
	double 		dcf[PARAM_INT];	/*partial derivatives of cost function with respect to parameters*/
	double 		ecfitems[PARAM_INT_H];
	
	double		dxy[BLOCK1];

	double		dx[BLOCK3-BLOCK2];
	double		dy[BLOCK3-BLOCK2];
	
	double		exyitems[(BLOCK3-BLOCK2)*BLOCK1];
	
	double		exitems[(BLOCK3-BLOCK2)*(BLOCK3-BLOCK2+1)/2];
	double		eyitems[(BLOCK3-BLOCK2)*(BLOCK3-BLOCK2+1)/2];

	double x_max1=stats2->x_dim-1;
	double y_max1=stats2->y_dim-1;

	unsigned int x_dim2=stats5->x_dim;
	unsigned int y_dim2=stats5->y_dim;
	unsigned int z_dim2=stats5->z_dim;

	double		e00,e01,e03,e10,e11,e13,e30,e31;

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
	e03=es[0][3];
	e10=es[1][0];
	e11=es[1][1];
	e13=es[1][3];
	e30=es[3][0];
	e31=es[3][1];

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
				signed int n0,n1,n2,n3;
				double a,b,d,e;
				double pix3=(double)*rr;
				
				#include "findcoords8.cf"
	
				{				
					/*Calculate the trilinear interpolated voxel value*/
	
					double pix4=n0*d*e+n1*a*e+n2*d*b+n3*a*b;
	
					/*Some intermediate values needed to calculate derivatives efficiently*/
	
					double dxpix4=(e*(n1-n0)+b*(n3-n2));
					double dypix4=(d*(n2-n0)+a*(n3-n1));
	
					double ezpix4=(double)n3+n0-n2-n1;
	
	
					/*Calculate the square of the difference*/
					cf+=pix4*pix4-2.0*pix4*pix3+pix3*pix3;
	
	
					/*Calculate derivatives that are nonzero*/
	
					/*First derivatives*/
					{
						double *dcft2;
	
						{
							unsigned int t;
							double *dxyt=dxy;
	
							double *dcft1=dcf;
							dcft2=dcf+BLOCK1;
							
							for(t=BLOCK0; t<BLOCK1; t++, dcft1++, dcft2++, dxyt++){
							
								/* dcf[t]+=2.0*(pix4-pix3)*dxpix4*dx[t];*/
								*dcft1+=2.0*(pix4-pix3)*dxpix4**dxyt;		
								
								/* dcf[t+BLOCK1]+=2.0*(pix4-pix3)*dypix4*dy[t+BLOCK1];*/
								*dcft2+=2.0*(pix4-pix3)*dypix4**dxyt;
							}
						}
								
						{
							unsigned int t;
							double *dxt=dx;
							double *dyt=dy;
							
							for(t=BLOCK2; t<BLOCK3; t++, dcft2++, dxt++, dyt++){
							
								/* dcf[t]+=2.0(pix4-pix3)*(dxpix4*dx[t]+dypix4*dy[t]);*/
								*dcft2+=2.0*(pix4-pix3)*(dxpix4**dxt+dypix4**dyt);
							}
						}
					}
								
					/*Second derivatives*/
					{
						double dydx[BLOCK1*BLOCK1];
						double dxdx[BLOCK1*(BLOCK1+1)/2];
	
						{
							double *dytdxs=dydx;
							double *dxtdxs=dxdx;
							
							double *dxyt=dxy;
							
							unsigned int t;
							
							for(t=BLOCK0; t<BLOCK1; t++, dxyt++){
							
								double *dxys=dxy;
								
								unsigned int s;
								
								for(s=BLOCK0; s<=t; s++, dxys++, dxtdxs++, dytdxs++){
								
									*dxtdxs=*dytdxs=2.0**dxyt**dxys;
								}
								for(; s<BLOCK1; s++, dxys++, dytdxs++){
								
									*dytdxs=2.0**dxyt**dxys;
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
									
										/* ecf[t][s]+=2.0*dy[t]*dx[s]*(dxpix4*dypix4+(pix4-pix3)*ezpix4);*/
										*ecfts+=*dytdxs*(dxpix4*dypix4+(pix4-pix3)*ezpix4);
									}
	
									for(; s<=t; s++, ecfts++, dytdys++){
									
										/* ecf[t][s]+=2.0*dy[t]*dy[s]*dypix4*dypix4;*/
										*ecfts+=*dytdys*dypix4*dypix4;
									}
								
								}
							}
							{
								double *dxt=dx;
								double *dyt=dy;
								
								double *exts=exyitems;
								double *eyts=exyitems;
								
								double *exts2=exitems;
								double *eyts2=eyitems;
								
								for(; t<BLOCK3; t++, dxt++, dyt++){
								
									unsigned int s;
									
									{
										double *dxs=dxy;
										
										for(s=BLOCK0; s<BLOCK1; s++, ecfts++, dxs++, exts++){
										
											/*ecf[t][s]+=2.0*dxpix4*dx[s]*(dxpix4*dx[t]+dypix4*dy[t]);*/
											*ecfts+=2.0*dxpix4**dxs*(dxpix4**dxt+dypix4**dyt);
	
											/* ecf[t][s]+=2.0*(pix4-pix3)*(dxpix4*ex[t][s]); */
											*ecfts+=2.0*(pix4-pix3)*(dxpix4**exts);
											
											/* ecf[t][s]+=2.0*(pix4-pix3)*dx[s]*(ezpix4*dy[t];*/
											*ecfts+=2.0*(pix4-pix3)**dxs*(ezpix4**dyt);
										}
									}
									{
										double *dys=dxy;
										
										for(; s<BLOCK2; s++, ecfts++, dys++, eyts++){
										
											/* ecf[t][s]+=2.0*dypix4*dy[s]*(dxpix4*dx[t]+dypix4*dy[t]);*/
											*ecfts+=2.0*dypix4**dys*(dxpix4**dxt+dypix4**dyt);
											
											/* ecf[t][s]+=2.0*(pix4-pix3)*(dypix4*ey[t][s]); */
											*ecfts+=2.0*(pix4-pix3)*(dypix4**eyts);
											
											/* ecf[t][s]+=2.0*(pix4-pix3)*dy[s]*(ezpix4*dx[t]); */
											*ecfts+=2.0*(pix4-pix3)**dys*(ezpix4**dxt);
										}
									}
									{
										double *dxs=dx;
										double *dys=dy;
										
										for(; s<=t; s++, ecfts++, dxs++, dys++, exts2++, eyts2++){
										
											/* ecf[t][s]+=2.0*(dxpix4*dx[s]+dypix4*dy[s])*(dxpix4*dx[t]+dypix4*dy[t]);*/
											*ecfts+=2.0*(dxpix4**dxs+dypix4**dys)*(dxpix4**dxt+dypix4**dyt);
											
											/* ecf[t][s]+=2.0*(pix4-pix3)*(dxpix4*ex[t][s]+dypix4*ey[t][s]); */
											*ecfts+=2.0*(pix4-pix3)*(dxpix4**exts2+dypix4**eyts2);
											
											/* ecf[t][s]+=2.0*(pix4-pix3)*dx[s]*(ezpix4*dy[t]); */
											*ecfts+=2.0*(pix4-pix3)**dxs*(ezpix4**dyt);
											
											/* ecf[t][s]+=2.0*(pix4-pix3)*dy[s]*(ezpix4*dx[t]); */
											*ecfts+=2.0*(pix4-pix3)**dys*(ezpix4**dxt);
										}
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
		dp[0]=des[0][0];
		dp[1]=des[1][0];
		dp[2]=des[3][0];
		dp[3]=des[0][1];
		dp[4]=des[1][1];
		dp[5]=des[3][1];
		dp[6]=des[0][3];
		dp[7]=des[1][3];
		
		ep[0]=ees[0][0];
		ep[1]=ees[1][0];
		ep[2]=ees[3][0];
		ep[3]=ees[0][1];
		ep[4]=ees[1][1];
		ep[5]=ees[3][1];
		ep[6]=ees[0][3];
		ep[7]=ees[1][3];
		
	}
	#include "param_chain_rule.cf"

	return cf;
}
