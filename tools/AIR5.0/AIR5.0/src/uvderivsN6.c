/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/16/01 */


/* double uvderivsN6()
 *
 * This routine computes the first and second derivatives
 *  of the normalized standard deviation with respect to all
 *  spatial transformation parameters.
 *
 * Returns the normalized standard deviation			
 */

#include "AIR.h"

#define PARAM_INT 6
#define PARAM_INT_H PARAM_INT*(PARAM_INT+1)/2

#define BLOCK0 0	/* x domain */
#define BLOCK1 3	/* y domain */
#define BLOCK2 6	/* end of y domain */

double AIR_uvderivsN6(const unsigned int spatial_parameters, double **es, double ***des, double ****ees, const unsigned int samplefactor, AIR_Pixels ***pixel2, const struct AIR_Key_info *stats2, AIR_Pixels ***pixel5, const struct AIR_Key_info *stats5, const AIR_Pixels threshold5, double *dcff, double **ecff, unsigned int *count, double *mean, double *square, double **dmean, double **dsquare, double ***emean, double ***esquare, const unsigned int partitions, AIR_Pixels max_actual_value, /*@unused@*/ double scale, /*@unused@*/ const AIR_Boolean forward, unsigned int *error)

{
	double 		cf=0.0;

	double		dcf[PARAM_INT];
	double 		ecfitems[PARAM_INT_H];
	
	double 		dxy[BLOCK1];

	double x_max1=stats2->x_dim-1;
	double y_max1=stats2->y_dim-1;

	unsigned int x_dim2=stats5->x_dim;
	unsigned int y_dim2=stats5->y_dim;
	unsigned int z_dim2=stats5->z_dim;

	double		e00,e01,e10,e11,e30,e31;

	if(partitions-1>(unsigned int)max_actual_value && partitions-1<(unsigned int)AIR_CONFIG_MAX_POSS_VALUE) max_actual_value=partitions-1;

	/*Initialize values*/
	{
		unsigned int *countjj=count;
		double *meanjj=mean;
		double *squarejj=square;
		
		double **dmeanjj=dmean;
		double **dsquarejj=dsquare;
		
		double ***emeanjj=emean;
		double ***esquarejj=esquare;
		
		unsigned int jj;
		
		for(jj=0; jj<partitions; jj++, countjj++, meanjj++, squarejj++, dmeanjj++, dsquarejj++, emeanjj++, esquarejj++){
		
			*countjj=0;
			*meanjj=*squarejj=0;
			{
				double *dmeanjjt=*dmeanjj;
				double *dsquarejjt=*dsquarejj;
				
				double *emeanjjts=**emeanjj;
				double *esquarejjts=**esquarejj;
				
				unsigned int t;
			
				for(t=0; t<PARAM_INT; t++, dmeanjjt++, dsquarejjt++){
				
					*dmeanjjt=0;
					*dsquarejjt=0;
					
					{
						unsigned int s;
						
						for(s=0; s<=t; s++, emeanjjts++, esquarejjts++){
						
							*emeanjjts=0;
							*esquarejjts=0;
						}
					}
				}
			}
		}		
	}

	e00=es[0][0];
	e01=es[0][1];
	e10=es[1][0];
	e11=es[1][1];
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
	
				#include "findcoords6.cf"
				
				{
					double ratio, dratio[PARAM_INT];
					{
						double pix4, dxpix4, dypix4, ezpix4;
						
						/* The variable pix1 is the partition of the current standard pixel */
						/* Note that if partitions==1, all pixels are in the same partition */
	
						unsigned int pix1=(partitions-1)*(unsigned int)pix3/(unsigned int)max_actual_value;
						{
							/*Calculate the trilinear interpolated voxel value*/
	
							pix4=n0*d*e+n1*a*e+n2*d*b+n3*a*b;
	
	
							/*Some intermediate values needed to calculate derivatives efficiently*/
	
							dxpix4=(e*(n1-n0)+b*(n3-n2));
							dypix4=(d*(n2-n0)+a*(n3-n1));
	
							ezpix4=(double)n3+n0-n2-n1;
						}
	
						/* Calculate values needed to compute standard deviation */
						ratio=pix4/pix3;
						mean[pix1]+=ratio;
						square[pix1]+=ratio*ratio;
						count[pix1]++;
	
	
						/*Calculate derivatives that are nonzero*/
						
						/* First derivatives */
						{
							unsigned int t;
							double *dxyt=dxy;
							
							double *dratiot1=dratio;
							
							double *dmeanpix1t1=dmean[pix1];
							
							double *dsquarepix1t1=dsquare[pix1];
							
							double *dratiot2=dratio+BLOCK1;
							double *dmeanpix1t2=dmean[pix1]+BLOCK1;
							double *dsquarepix1t2=dsquare[pix1]+BLOCK1;
							
							for(t=BLOCK0; t<BLOCK1; t++, dxyt++, dratiot1++, dratiot2++, dmeanpix1t1++, dmeanpix1t2++, dsquarepix1t1++, dsquarepix1t2++){
	
								/* dratio[t]=dxpix4*dx[t]/pix3; */
								*dratiot1=dxpix4**dxyt/pix3;
								
								/* dmean[pix1][t]+=dratio[t]; */
								*dmeanpix1t1+=*dratiot1;
								
								/* dsquare[pix1][t]+=2*ratio*dratio[t]*/
								*dsquarepix1t1+=2*ratio**dratiot1;
								
								/* dratio[t+BLOCK1]=dypix4*dy[t+BLOCK1]/pix3; */
								*dratiot2=dypix4**dxyt/pix3;
								
								/* dmean[pix1][t+BLOCK1]+=dratio[t+BLOCK1]; */
								*dmeanpix1t2+=*dratiot2;
								
								/* dsquare[pix1][t]+=2*ratio*dratio[t]*/
								*dsquarepix1t2+=2*ratio**dratiot2;
							}
						}
						
						/* Second derivatives */
						{
							double **emeanpix1t=emean[pix1]+BLOCK1;
							double *esquarepix1ts=*esquare[pix1];
							double *dratiot=dratio;
							unsigned int t;
							
							
							{
								double *dyt=dxy;
								
								for(t=BLOCK0;t<BLOCK1;t++, dratiot++){
								
									{
										unsigned int s;
										double *dratios=dratio;
	
										for(s=BLOCK0; s<=t; s++, esquarepix1ts++, dratios++){
										
											/* esquare[pix1][t][s]+=2*(dratio[s]*dratio[t]) */
											*esquarepix1ts+=2.0*(*dratios**dratiot);
										}
									}
								}
									
								
								for(; t<BLOCK2; t++, dyt++, dratiot++, emeanpix1t++){
								
									
									{
										unsigned int s;
										double *dratios=dratio;
										{
											double *emeanpix1ts=*emeanpix1t;
											{
												double *dxs=dxy;
	
												
												for(s=BLOCK0; s<BLOCK1; s++, emeanpix1ts++, esquarepix1ts++, dratios++, dxs++){
												
													/* eratio[t][s]=ezpix4*dy[t]*dx[s]/pix3; */
													double eratiots=ezpix4**dyt**dxs/pix3;
													
													/* emeanpix1[t][s]+=eratio[t][s]; */
													*emeanpix1ts+=eratiots;
													
													/* esquare[pix1][t][s]+=2*(dratio[s]*dratio[t]+ratio*eratio[t][s]);*/
													*esquarepix1ts+=2.0*(*dratios**dratiot+ratio*eratiots);
												}
											}
										}
										for(; s<=t; s++, esquarepix1ts++, dratios++){
										
											/* esquare[pix1][t][s]+=2*(dratio[s]*dratio[t]);*/
											*esquarepix1ts+=2.0*(*dratios**dratiot);
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	
	#include "norm_std_dev.cf"
	
	{
		double **ep[PARAM_INT];
		double *dp[PARAM_INT];
		
		dp[0]=des[0][0];
		dp[1]=des[1][0];
		dp[2]=des[3][0];
		dp[3]=des[0][1];
		dp[4]=des[1][1];
		dp[5]=des[3][1];
		
		ep[0]=ees[0][0];
		ep[1]=ees[1][0];
		ep[2]=ees[3][0];
		ep[3]=ees[0][1];
		ep[4]=ees[1][1];
		ep[5]=ees[3][1];
		
		#include "param_chain_rule.cf"

	}
	return cf;
}
