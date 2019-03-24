/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 11/10/01 */


/* double uvderivsN15()
 *
 * This routine computes the first and second derivatives
 *  of the normalized standard deviation with respect to all
 *  spatial transformation parameters.
 *
 * Returns the normalized standard deviation			
 */

#include "AIR.h"

#define PARAM_INT 15
#define PARAM_INT_H PARAM_INT*(PARAM_INT+1)/2

#define BLOCK0 0	/* x domain */
#define BLOCK1 4	/* y domain */
#define BLOCK2 8	/* z domain */
#define BLOCK3 12	/* perspective domain */
#define BLOCK4 15	/* end of perspective domain */

double AIR_uvderivsN15(const unsigned int spatial_parameters, double **es, double ***des, double ****ees, const unsigned int samplefactor, AIR_Pixels ***pixel2, const struct AIR_Key_info *stats2, AIR_Pixels ***pixel5, const struct AIR_Key_info *stats5, const AIR_Pixels threshold5, double *dcff, double **ecff, unsigned int *count, double *mean, double *square, double **dmean, double **dsquare, double ***emean, double ***esquare, const unsigned int partitions, AIR_Pixels max_actual_value, /*@unused@*/ double scale, /*@unused@*/ const AIR_Boolean forward, unsigned int *error)

{

	double 		cf=0.0;

	double		dcf[PARAM_INT];
	double 		ecfitems[PARAM_INT_H];
	
	double 		dxyz[BLOCK1];
	
	double		dx[BLOCK4-BLOCK3];
	double		dy[BLOCK4-BLOCK3];
	double		dz[BLOCK4-BLOCK3];
	
	double		exyzitems[(BLOCK4-BLOCK3)*BLOCK1];
	
	double		exitems[(BLOCK4-BLOCK3)*(BLOCK4-BLOCK3+1)/2];
	double		eyitems[(BLOCK4-BLOCK3)*(BLOCK4-BLOCK3+1)/2];
	double		ezitems[(BLOCK4-BLOCK3)*(BLOCK4-BLOCK3+1)/2];


	double x_max1=stats2->x_dim-1;
	double y_max1=stats2->y_dim-1;
	double z_max1=stats2->z_dim-1;

	unsigned int x_dim2=stats5->x_dim;
	unsigned int y_dim2=stats5->y_dim;
	unsigned int z_dim2=stats5->z_dim;

	double		e00,e01,e02,e03,e10,e11,e12,e13,e20,e21,e22,e23,e30,e31,e32;

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
	e02=es[0][2];
	e03=es[0][3];
	e10=es[1][0];
	e11=es[1][1];
	e12=es[1][2];
	e13=es[1][3];
	e20=es[2][0];
	e21=es[2][1];
	e22=es[2][2];
	e23=es[2][3];
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
	
				#include "findcoords15.cf"
				
				{
					double ratio, dratio[PARAM_INT];
					{
						double pix4, dxpix4, dypix4, dzpix4, expix4, eypix4, ezpix4;
						
						/* The variable pix1 is the partition of the current standard pixel */
						/* Note that if partitions==1, all pixels are in the same partition */
	
						unsigned int pix1=(partitions-1)*(unsigned int)pix3/(unsigned int)max_actual_value;
						{
							/*Calculate the trilinear interpolated voxel value*/
	
							pix4=n0*d*e*f+n1*a*e*f+n2*d*b*f+n3*a*b*f+n4*d*e*c+n5*a*e*c+n6*d*b*c+n7*a*b*c;
	
	
							/*Some intermediate values needed to calculate derivatives efficiently*/
	
							dxpix4=((e*f)*(n1-n0)+(b*f)*(n3-n2)+(c*e)*(n5-n4)+(b*c)*(n7-n6));
							dypix4=((d*f)*(n2-n0)+(a*f)*(n3-n1)+(c*d)*(n6-n4)+(a*c)*(n7-n5));
							dzpix4=((d*e)*(n4-n0)+(a*e)*(n5-n1)+(b*d)*(n6-n2)+(a*b)*(n7-n3));
	
							expix4=((n7+n1-n5-n3)*a+(n6+n0-n4-n2)*d);
							eypix4=((n7+n2-n6-n3)*b+(n5+n0-n4-n1)*e);
							ezpix4=((n7+n4-n6-n5)*c+(n3+n0-n2-n1)*f);
						}
	
						/* Calculate values needed to compute standard deviation */
						ratio=pix4/pix3;
						mean[pix1]+=ratio;
						square[pix1]+=ratio*ratio;
						count[pix1]++;
	
	
						/*Calculate derivatives that are nonzero*/
						
						/* First derivatives */
						{
							double *dratiot3;
							double *dmeanpix1t3;
							double *dsquarepix1t3;
							
							{
								unsigned int t;
								double *dxyzt=dxyz;
								
								double *dratiot1=dratio;
								double *dratiot2=dratio+BLOCK1;
								
								double *dmeanpix1t1=dmean[pix1];
								double *dmeanpix1t2=dmean[pix1]+BLOCK1;
								
								double *dsquarepix1t1=dsquare[pix1];
								double *dsquarepix1t2=dsquare[pix1]+BLOCK1;
								
								dratiot3=dratio+BLOCK2;
								dmeanpix1t3=dmean[pix1]+BLOCK2;
								dsquarepix1t3=dsquare[pix1]+BLOCK2;
								
								for(t=BLOCK0; t<BLOCK1; t++, dxyzt++, dratiot1++, dratiot2++, dratiot3++, dmeanpix1t1++, dmeanpix1t2++, dmeanpix1t3++, dsquarepix1t1++, dsquarepix1t2++, dsquarepix1t3++){
	
									/* dratio[t]=dxpix4*dx[t]/pix3; */
									*dratiot1=dxpix4**dxyzt/pix3;
									
									/* dmean[pix1][t]+=dratio[t]; */
									*dmeanpix1t1+=*dratiot1;
									
									/* dsquare[pix1][t]+=2*ratio*dratio[t]*/
									*dsquarepix1t1+=2*ratio**dratiot1;
									
									/* dratio[t+BLOCK1]=dypix4*dy[t+BLOCK1]/pix3; */
									*dratiot2=dypix4**dxyzt/pix3;
									
									/* dmean[pix1][t+BLOCK1]+=dratio[t+BLOCK1]; */
									*dmeanpix1t2+=*dratiot2;
									
									/* dsquare[pix1][t]+=2*ratio*dratio[t]*/
									*dsquarepix1t2+=2*ratio**dratiot2;
									
									/* dratio[t+BLOCK2]=dzpix4*dz[t+BLOCK2]/pix3; */
									*dratiot3=dzpix4**dxyzt/pix3;
									
									/* dmean[pix1][t+BLOCK2]+=dratio[t+BLOCK2]; */
									*dmeanpix1t3+=*dratiot3;
									
									/* dsquare[pix1][t]+=2*ratio*dratio[t]*/
									*dsquarepix1t3+=2*ratio**dratiot3;
								}
							}
							{
								unsigned int t;
								double *dxt=dx;
								double *dyt=dy;
								double *dzt=dz;
								
								for(t=BLOCK3; t<BLOCK4; t++, dratiot3++, dmeanpix1t3++, dsquarepix1t3++, dxt++, dyt++, dzt++){
								
									/* dratio[t]=(dxpix4*dx[t]+dypix4*dy[t]+dzpix4*dz[t])/pix3; */
									*dratiot3=(dxpix4**dxt+dypix4**dyt+dzpix4**dzt)/pix3;
									
									/* dmean[pix1][t]+=dratio[t]; */
									*dmeanpix1t3+=*dratiot3;
									
									/* dmean[pix1][t]+=2*ratio*dratio[t]; */
									*dsquarepix1t3+=2*ratio**dratiot3;
								}
							}
						}
						
						/* Second derivatives */
						{
							double **emeanpix1t=emean[pix1]+BLOCK1;
							double *esquarepix1ts=*esquare[pix1];
							double *dratiot=dratio;
							unsigned int t;
							
							
							{
								double *dyt=dxyz;
								
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
												double *dxs=dxyz;
	
												
												for(s=BLOCK0; s<BLOCK1; s++, emeanpix1ts++, esquarepix1ts++, dratios++, dxs++){
												
													/* eratio[t][s]=ezpix4*dy[t]*dx[s]/pix3; */
													double eratiots=ezpix4**dyt**dxs/pix3;
													
													/* emeanpix1[t][s]+=eratio[t][s]; */
													*emeanpix1ts+=eratiots;
													
													/* esquare[pix1][t][s]+=2*(dratio[s]*dratio[t]+ratio*eratio[t][s]);*/
													*esquarepix1ts+=2*(*dratios**dratiot+ratio*eratiots);
												}
											}
										}
										for(; s<=t; s++, esquarepix1ts++, dratios++){
										
											/* esquare[pix1][t][s]+=2*(dratio[s]*dratio[t]);*/
											*esquarepix1ts+=2*(*dratios**dratiot);
										}
									}
								}
							}
							{
								double *dzt=dxyz;
								
								for(; t<BLOCK3; t++, dzt++, dratiot++, emeanpix1t++){
								
									
									{
										unsigned int s;
										double *dratios=dratio;
										{
											double *emeanpix1ts=*emeanpix1t;
	
											
											{
												double *dxs=dxyz;
												
												for(s=BLOCK0; s<BLOCK1; s++, emeanpix1ts++, esquarepix1ts++, dratios++, dxs++){
												
													/* eratio[t][s]=eypix4*dz[t]*dx[s]/pix3; */
													double eratiots=eypix4**dzt**dxs/pix3;
													
													/* emean[pix1][t][s]+=eratio[t][s];*/
													*emeanpix1ts+=eratiots;
													
													/* esquare[pix1][t][s]+=2*(dratio[s]*dratio[t]+ratio*eratio[t][s]);*/
													*esquarepix1ts+=2*(*dratios**dratiot+ratio*eratiots);
												}
											}
											{
												double *dys=dxyz;
												
												for(; s<BLOCK2; s++, emeanpix1ts++, esquarepix1ts++, dratios++, dys++){
												
													/* eratio[t][s]=expix4*dz[t]*dy[s]/pix3; */
													double eratiots=expix4**dzt**dys/pix3;
													
													/* emean[pix1][t][s]+=eratio[t][s]; */
													*emeanpix1ts+=eratiots;
													
													/* esquare[pix1][t][s]+=2*(dratio[s]*dratio[t]+ratio*eratio[t][s]);*/
													*esquarepix1ts+=2*(*dratios**dratiot+ratio*eratiots);
												}
											}
										}
										{
											for(; s<=t; s++, esquarepix1ts++, dratios++){
										
												/* esquare[pix1][t][s]+=2*(dratio[s]*dratio[t]);*/
												*esquarepix1ts+=2*(*dratios**dratiot);
											}
										}
										
									}
								}
							}
							{
								
								double *dxt=dx;
								double *dyt=dy;
								double *dzt=dz;
								
								double *exts=exyzitems;
								double *eyts=exyzitems;
								double *ezts=exyzitems;
								
								double *exts2=exitems;
								double *eyts2=eyitems;
								double *ezts2=ezitems;
								
								double *emeanpix1ts=*emeanpix1t;
								
								for(; t<BLOCK4; t++, dxt++, dyt++, dzt++, dratiot++){
								
									unsigned int s;
									double *dratios=dratio;
									
									{
										double *dxs=dxyz;
										
										for(s=BLOCK0; s<BLOCK1; s++, emeanpix1ts++, esquarepix1ts++, dratios++, dxs++, exts++){
										
											/* eratio[t][s]=dxpix4*ex[t][s]; */
											double eratiots=dxpix4**exts;
	
											/* eratio[t][s]+=dx[s]*(ezpix4*dy[t]+eypix4*dz[t]); */
											eratiots+=*dxs*(ezpix4**dyt+eypix4**dzt);
	
											/* eratio[t][s]/=pix3; */
											eratiots/=pix3;
											
											/* emean[pix1][t][s]+=eratio[t][s];*/
											*emeanpix1ts+=eratiots;
											
											/*esquare[pix1][t][s]+=2*(dratio[s]*dratio[t]+ratio*eratio[t][s]);*/
											*esquarepix1ts+=2*(*dratios**dratiot+ratio*eratiots);
										}
									}
									{
										double *dys=dxyz;
										
										for(; s<BLOCK2; s++, emeanpix1ts++, esquarepix1ts++, dratios++, dys++, eyts++){
										
											/* eratio[t][s]=dypix4*ey[t][s]; */
											double eratiots=dypix4**eyts;
											
											/* eratio[t][s]+=dy[s]*(ezpix4*dx[t]+expix4*dz[t]); */
											eratiots+=*dys*(ezpix4**dxt+expix4**dzt);
											
											/* eratio[t][s]/=pix3; */
											eratiots/=pix3;
											
											/* emean[pix1][t][s]+=eratio[t][s];*/
											*emeanpix1ts+=eratiots;
											
											/* esquare[pix1][t][s]+=2*(dratio[s]*dratio[t]+ratio*eratio[t][s]);*/
											*esquarepix1ts+=2*(*dratios**dratiot+ratio*eratiots);
										}
									}
									{
										double *dzs=dxyz;
										
										for(; s<BLOCK3; s++, emeanpix1ts++, esquarepix1ts++, dratios++, dzs++, ezts++){
										
											/* eratio[t][s]=dzpix4*ez[t][s]; */
											double eratiots=dzpix4**ezts;
											
											/* eratio[t][s]+=dz[s]*(expix4*dy[t]*eypix4*dx[t]); */
											eratiots+=*dzs*(expix4**dyt*eypix4**dxt);
											
											/* eratio[t][s]/=pix3; */
											eratiots/=pix3;
											
											/* emean[pix1][t][s]+=eratio[t][s];*/
											*emeanpix1ts+=eratiots;
											
											/* esquare[pix1][t][s]+=2*(dratio[s]*dratio[t]+ratio*eratio[t][s]);*/
											*esquarepix1ts+=2*(*dratios**dratiot+ratio*eratiots);
										}
									}
									{
										double *dxs=dx;
										double *dys=dy;
										double *dzs=dz;
										
										for(; s<=t; s++, emeanpix1ts++, esquarepix1ts++, dratios++, dxs++, dys++, dzs++, exts2++, eyts2++, ezts2++){
										
											/* eratio[t][s]=(dxpix4*ex[t][s]+dypix4*ey[t][s]+dzpix4*ez[t][s]);*/
											double eratiots=(dxpix4**exts2+dypix4**eyts2+dzpix4**ezts2);
											
											/* eratio[t][s]+=dx[s]*(ezpix4*dy[t]+eypix4*dz[t]); */
											eratiots+=*dxs*(ezpix4**dyt+eypix4**dzt);
											
											/* eratio[t][s]+=dy[s]*(ezpix4*dx[t]+expix4*dz[t]); */
											eratiots+=*dys*(ezpix4**dxt+expix4**dzt);
											
											/* eratio[t][s]+=dz[s]*(eypix4*dx[t]*expix4*dy[t]); */
											eratiots+=*dzs*(eypix4**dxt*expix4**dyt);
											
											/* eratio[t][s]/=pix3; */
											eratiots/=pix3;
											
											/* emean[pix1][t][s]+=eratio[t][s];*/
											*emeanpix1ts+=eratiots;
											
											/* esquare[pix1][t][s]+=2*(dratio[s]*dratio[t]+ratio*eratio[t][s]);*/
											*esquarepix1ts+=2*(*dratios**dratiot+ratio*eratiots);
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
		{
			unsigned int t;
			
			for(t=0;t<BLOCK1;t++){
			
				dp[t]=des[t][0];
				ep[t]=ees[t][0];
			}
			{
				{
					unsigned int r;
					
					for(r=0;t<BLOCK2;t++,r++){
						dp[t]=des[r][1];
						ep[t]=ees[r][1];
					}
				}
				{
					unsigned int r;
					
					for(r=0;t<BLOCK3;t++,r++){
						dp[t]=des[r][2];
						ep[t]=ees[r][2];
					}
				}
				{
					unsigned int r;
					
					for(r=0;t<BLOCK4;t++,r++){
						dp[t]=des[r][3];
						ep[t]=ees[r][3];
					}
				}
			}
		}
		
		#include "param_chain_rule.cf"

	}
	return cf;
}
