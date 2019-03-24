/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/16/01 */

/*
 * double qvderiv2Dwarp()
 *
 * This routine computes the first and second derivatives
 *  of the normalized cost function with respect to all
 *  external parameters.
 *
 * The matrix indices have been adjusted to allow for a more
 *  orderly relationship between the matrix and its elements
 *
 * Returns the sum of squares
 *
 * Note that derivatives with respect to external parameters are ignored
 */

#include "AIR.h"

#define COORDS_INT 2


double AIR_qvderiv2Dwarp(const unsigned int parameters, double **es, const unsigned int samplefactor, AIR_Pixels ***pixel2, const struct AIR_Key_info *stats2, const AIR_Pixels threshold2, AIR_Pixels ***pixel5, const struct AIR_Key_info *stats5, const AIR_Pixels threshold5, double *dcf, double **ecf, double *dx, const double scale, unsigned int *error)

{
	unsigned long int counttotal=0;

	double cf=0.0;		/*intermediate value*/

	double x_max1=stats2->x_dim-1.0;
	double y_max1=stats2->y_dim-1.0;

	const unsigned int x_dim2=stats5->x_dim;
	const unsigned int y_dim2=stats5->y_dim;
	const unsigned int z_dim2=stats5->z_dim;
	
	const signed int threshold2i=(signed int)threshold2;	/* avoid need to keep doing type conversions */

	unsigned int order=12;
	unsigned int coeffp=91;
	
	if(parameters<182){
		order=11;
		coeffp=78;
	}
	if(parameters<156){
		order=10;
		coeffp=66;
	}
	if(parameters<132){
		order=9;
		coeffp=55;
	}
	if(parameters<110){
		order=8;
		coeffp=45;
	}
	if(parameters<90){
		order=7;
		coeffp=36;
	}
	if(parameters<72){
		order=6;
		coeffp=28;
	}
	if(parameters<56){
		order=5;
		coeffp=21;
	}
	if(parameters<42){
		order=4;
		coeffp=15;
	}
	if(parameters<30){
		order=3;
		coeffp=10;
	}
	if(parameters<20){
		order=2;
		coeffp=6;
	}
	if(parameters<12){
		order=1;
		coeffp=3;
	}

	{
		double *dxt=dx;
		unsigned int t;
		
		for(t=0;t<coeffp;t++){
			*dxt=0;
		}
	}
	{
		double *dcft=dcf;
		double *ecfts=*ecf;
		unsigned int t;
		
		for (t=0; t<=2*coeffp; t++, dcft++){
		
			unsigned int s;
			
			*dcf=0;
			
			for (s=0; s<=t; s++, ecfts++){
			
				*ecfts=0;
			}
		}
	}

	/*Examine pixels of standard file at samplefactor interval*/
	/*Note that it is assumed here that pixel5[z_dim][y_dim][x_dim] refers to the*/
	/* same pixel as *(pixel5[0][0]+z_dim*y_dim*x_dim), i.e. that all the pixels */
	/* are represented in a contiguous block of memory--see the routine*/
	/* "create_vol3.c" for an illustcfn of how this is assured*/

	/*ARRAY STRUCTURE ASSUMPTIONS MADE HERE*/
	{
		unsigned long int r;
		unsigned long int r_term=x_dim2*y_dim2*z_dim2;
		AIR_Pixels *rr;
		
		for (r=0,rr=**pixel5;r<r_term;r+=samplefactor,rr+=samplefactor){

			signed int n0,n1,n2,n3;
			double a,b,d,e;
			
			/*We don't yet verify that pixel5>threshold to allow for less biased function*/

			double pix3=(double)*rr;
			double spix3=pix3*scale;

			{
				/*Calculate coordinates (i,j,k) of pixel r in standard file*/
				
				unsigned int i,j,k;
				double i1, j1;
				{
						unsigned long int tempplane=x_dim2*y_dim2;
						unsigned long int tempremainder=r%tempplane;

						k=(unsigned int)r/tempplane;
						j=(unsigned int)tempremainder/x_dim2;
						i=(unsigned int)tempremainder%x_dim2;
				}
		
				i1=(double)i;
				j1=(double)j;
					
				/*Calculate coordinates (x_i,y_i) of corresponding pixel in reslice file*/
				/*Skip further computations if pixel is out of bounds*/
				{
					double *dxt=dx;

					*dxt++=(double)1.0;		/* 0 */
					*dxt++=i1;			/* 1 */
					*dxt++=j1;			/* 2 */
					if(order>1){
						double i2=i1*i1;
						double j2=j1*j1;
						*dxt++=i2;		/* 3 */
						*dxt++=i1*j1;		/* 4 */
						*dxt++=j2;		/* 5 */

					if(order>2){
						double i3=i2*i1;
						double j3=j2*j1;
						*dxt++=i3;		/* 6 */
						*dxt++=i2*j1;		/* 7 */
						*dxt++=i1*j2;		/* 8 */
						*dxt++=j3;		/* 9 */

					if(order>3){
						double i4=i3*i1;
						double j4=j3*j1;
						*dxt++=i4;		/* 10 */
						*dxt++=i3*j1;		/* 11 */
						*dxt++=i2*j2;		/* 12 */
						*dxt++=i1*j3;		/* 13 */
						*dxt++=j4;		/* 14 */

					if(order>4){
						double i5=i4*i1;
						double j5=j4*j1;
						*dxt++=i5;		/* 15 */
						*dxt++=i4*j1;		/* 16 */
						*dxt++=i3*j2;		/* 17 */
						*dxt++=i2*j3;		/* 18 */
						*dxt++=i1*j4;		/* 19 */
						*dxt++=j5;		/* 20 */

					if(order>5){
						double i6=i5*i1;
						double j6=j5*j1;
						*dxt++=i6;		/* 21 */
						*dxt++=i5*j1;		/* 22 */
						*dxt++=i4*j2;		/* 23 */
						*dxt++=i3*j3;		/* 24 */
						*dxt++=i2*j4;		/* 25 */
						*dxt++=i1*j5;		/* 26 */
						*dxt++=j6;		/* 27 */

					if(order>6){
						double i7=i6*i1;
						double j7=j6*j1;
						*dxt++=i7;		/* 28 */
						*dxt++=i6*j1;		/* 29 */
						*dxt++=i5*j2;		/* 30 */
						*dxt++=i4*j3;		/* 31 */
						*dxt++=i3*j4;		/* 32 */
						*dxt++=i2*j5;		/* 33 */
						*dxt++=i1*j6;		/* 34 */
						*dxt++=j7;		/* 35 */

					if(order>7){
						double i8=i7*i1;
						double j8=j7*j1;
						*dxt++=i8;		/* 36 */
						*dxt++=i7*j1;		/* 37 */
						*dxt++=i6*j2;		/* 38 */
						*dxt++=i5*j3;		/* 39 */
						*dxt++=i4*j4;		/* 40 */
						*dxt++=i3*j5;		/* 41 */
						*dxt++=i2*j6;		/* 42 */
						*dxt++=i1*j7;		/* 43 */
						*dxt++=j8;		/* 44 */

					if(order>8){
						double i9=i8*i1;
						double j9=j8*j1;
						*dxt++=i9;		/* 45 */
						*dxt++=i8*j1;		/* 46 */
						*dxt++=i7*j2;		/* 47 */
						*dxt++=i6*j3;		/* 48 */
						*dxt++=i5*j4;		/* 49 */
						*dxt++=i4*j5;		/* 50 */
						*dxt++=i3*j6;		/* 51 */
						*dxt++=i2*j7;		/* 52 */
						*dxt++=i1*j8;		/* 53 */
						*dxt++=j9;		/* 54 */

					if(order>9){
						double i10=i9*i1;
						double j10=j9*j1;
						*dxt++=i10;		/* 55 */
						*dxt++=i9*j1;		/* 56 */
						*dxt++=i8*j2;		/* 57 */
						*dxt++=i7*j3;		/* 58 */
						*dxt++=i6*j4;		/* 59 */
						*dxt++=i5*j5;		/* 60 */
						*dxt++=i4*j6;		/* 61 */
						*dxt++=i3*j7;		/* 62 */
						*dxt++=i2*j8;		/* 63 */
						*dxt++=i1*j9;		/* 64 */
						*dxt++=j10;		/* 65 */

					if(order>10){
						double i11=i10*i1;
						double j11=j10*j1;
						*dxt++=i11;		/* 66 */
						*dxt++=i10*j1;		/* 67 */
						*dxt++=i9*j2;		/* 68 */
						*dxt++=i8*j3;		/* 69 */
						*dxt++=i7*j4;		/* 70 */
						*dxt++=i6*j5;		/* 71 */
						*dxt++=i5*j6;		/* 72 */
						*dxt++=i4*j7;		/* 73 */
						*dxt++=i3*j8;		/* 74 */
						*dxt++=i2*j9;		/* 75 */
						*dxt++=i1*j10;		/* 76 */
						*dxt++=j11;		/* 77 */

					if(order>11){
						double i12=i11*i1;
						double j12=j11*j1;
						*dxt++=i12;		/* 78 */
						*dxt++=i11*j1;		/* 79 */
						*dxt++=i10*j2;		/* 80 */
						*dxt++=i9*j3;		/* 81 */
						*dxt++=i8*j4;		/* 82 */
						*dxt++=i7*j5;		/* 83 */
						*dxt++=i6*j6;		/* 84 */
						*dxt++=i5*j7;		/* 85 */
						*dxt++=i4*j8;		/* 86 */
						*dxt++=i3*j9;		/* 87 */
						*dxt++=i2*j10;		/* 88 */
						*dxt++=i1*j11;		/* 89 */
						*dxt++=j12;		/* 90 */

					}	/* end if(order>11) */
					}	/* end if(order>10) */
					}	/* end if(order>9) */
					}	/* end if(order>8) */
					}	/* end if(order>7) */
					}	/* end if(order>6) */
					}	/* end if(order>5) */
					}	/* end if(order>4) */
					}	/* end if(order>3) */
					}	/* end if(order>2) */
					}	/* end if(order>1) */
				}
				/* Find the x, y, and z coordinates */
				{

					double x_i=0.0;
					double y_i=0.0;
					
					{
						double *es0ii=es[0];
						double *es1ii=es[1];
						double *dxii=dx;
						unsigned int ii;
						
						for(ii=0;ii<coeffp;ii++, es0ii++, es1ii++, dxii++){
							/* x_i+=e0[ii]*dx[ii]; */
							/* y_i+=e1[ii]*dx[ii]; */

							x_i+=*es0ii**dxii;
							y_i+=*es1ii**dxii;
						}
					}
					if(x_i<0 || x_i>x_max1) continue;
					if(y_i<0 || y_i>y_max1) continue;


					/*Get the coordinates of the 4 voxels surrounding the designated pixel*/
					/* in the reslice file*/
					{
						unsigned int x_up=(unsigned int)ceil(x_i);
						unsigned int x_down=(unsigned int)floor(x_i);
						unsigned int y_up=(unsigned int)ceil(y_i);
						unsigned int y_down=(unsigned int)floor(y_i);

						if(x_up==x_down){
							if(x_up<(unsigned int)x_max1) x_up++;
							else x_down--;
						}
						a=x_i-x_down;
						d=x_up-x_i;

						if(y_up==y_down){
							if(y_up<(unsigned int)y_max1) y_up++;
							else y_down--;
						}
						b=y_i-y_down;
						e=y_up-y_i;


						/*Get the values of these 4 voxels*/

						n0=(int)pixel2[k][y_down][x_down];
						n1=(int)pixel2[k][y_down][x_up];
						n2=(int)pixel2[k][y_up][x_down];
						n3=(int)pixel2[k][y_up][x_up];
					}
				}
			}

			/*Verify that something is above threshold here*/
			if(*rr<=threshold5 && n0<=threshold2i && n1<=threshold2i && n2<=threshold2i && n3<=threshold2i) continue;

			{
				/*Calculate the trilinear interpolated voxel value*/

				double pix4=n0*d*e+n1*a*e+n2*d*b+n3*a*b;


				/*Some intermediate values needed to calculate derivatives efficiently*/

				double dxpix4=(e*(n1-n0)+b*(n3-n2));
				double dypix4=(d*(n2-n0)+a*(n3-n1));

				/*Calculate the square of the difference*/
				cf+=pix4*pix4-2.0*pix4*spix3+spix3*spix3;


				/*Calculate derivatives that are nonzero*/
				{
					double *dcft=dcf;
					unsigned int t;
					
					{
						double *dxt=dx;
						
						for(t=0;t<coeffp;t++, dcft++, dxt++){
						
							/* dcf[t]=2.0*(pix4-pix3*scale)*dxpix4*dx[t];*/
							*dcft+=2.0*(pix4-spix3)*dxpix4**dxt;
						}
					}
					{
						double *dyt=dx;
						
						for(;t<2*coeffp;t++, dcft++, dyt++){
						
							/* dcf[t]=2.0*(pix4-pix3*scale)*dypix4*dx[t-coeffp];*/
							*dcft+=2.0*(pix4-spix3)*dypix4**dyt;
						}
					}
					*dcft+=-2.0*pix3*(pix4-spix3);	/* formerly dcf_scale */
				}
				{
					double *ecfts=*ecf;		/* Note assumption that ecf was allocated by tritrix2() */
					unsigned int t;
					{
						double *dxt=dx;
						
						for(t=0;t<coeffp;t++, dxt++){
						
							unsigned int s;
							
							{
								double *dxs=dx;
								
								for(s=0;s<=t;s++, ecfts++, dxs++){
								
									*ecfts+=2.0*dxpix4*dxpix4**dxt**dxs;
								}
							}
						}
					}
					{
						double *dyt=dx;
						
						for(;t<2*coeffp;t++, dyt++){
						
							unsigned int s;
							
							{
								double *dxs=dx;
								
								for(s=0;s<coeffp;s++, ecfts++, dxs++){
								
									*ecfts+=2.0**dyt**dxs*(dxpix4*dypix4);
								}
							}
							{
								double *dys=dx;
								
								for(;s<=t;s++, ecfts++, dys++){
								
									*ecfts+=2.0*dypix4*dypix4**dyt**dys;
								}
							}
						}
					}
					{
						unsigned int s;
						
						{
							double *dxs=dx;
							
							for(s=0;s<coeffp;s++, ecfts++, dxs++){
							
								*ecfts+=-2.0*pix3*dxpix4**dxs;
							}
						}
						{
							double *dys=dx;
							
							for(;s<2*coeffp;s++, ecfts++, dys++){
							
								*ecfts+=-2.0*pix3*dypix4**dys;
							}
						}
						*ecfts+=2.0*pix3*pix3;	/* formerly ecf_scale_scale */
					}
				}
				counttotal++;
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
		double *ecfts=*ecf;
		unsigned int t;
		
		for(t=0;t<=2*coeffp;t++, dcft++){
		
			*dcft/=counttotal;
			{
				unsigned int s;
				
				for(s=0;s<=t;s++, ecfts++){
				
					*ecfts/=counttotal;
				}
			}
		}
	}
	
	return cf;
}
