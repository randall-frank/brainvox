/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/22/01 */


/*
 * This routine computes the derivative of a warp at a specified point
 *
 * Returns 0 if successful, error code otherwise
 */

#include "AIR.h"

AIR_Error AIR_diffg2D(double **es, const double i, const double j, double **ed, const unsigned int order, double *dx_pi, double *dx_pj)

/* (i,j) is the point, ed is the derivative, es are the parameters and dx_pi and dx_pj are storage */

{
	unsigned int coeffp=(order+1)*(order+2)/2;

	if(order>AIR_CONFIG_MAXORDER){
		return AIR_POLYNOMIAL_ORDER_ERROR;
	}

	/* Initialize work arrays */
	{
		unsigned int ii;
		
		for(ii=0;ii<coeffp;ii++){
			dx_pi[ii]=0.0;
			dx_pj[ii]=0.0;
		}
	}

	if(order!=0){
	
		double i1=i;
		double j1=j;
		
		dx_pi[1]=1;
		dx_pj[2]=1;

	if(order>1){
	
		double i2=i1*i1;
		double j2=j1*j1;
		
		dx_pi[3]=2*i1;
		dx_pi[4]=j1;
		dx_pj[4]=i1;
		dx_pj[5]=2*j1;

	if(order>2){
	
		double i3=i2*i1;
		double j3=j2*j1;
		
		dx_pi[6]=3*i2;
		dx_pi[7]=2*i1*j1;
		dx_pj[7]=i2;
		dx_pi[8]=j2;
		dx_pj[8]=2*i1*j1;
		dx_pj[9]=3*j2;

	if(order>3){
	
		double i4=i3*i1;
		double j4=j3*j1;
		
		dx_pi[10]=4*i3;
		dx_pi[11]=3*i2*j1;
		dx_pj[11]=i3;
		dx_pi[12]=2*i1*j2;
		dx_pj[12]=2*i2*j1;
		dx_pi[13]=j3;
		dx_pj[13]=3*i1*j2;
		dx_pj[14]=4*j3;

	if(order>4){
	
		double i5=i4*i1;
		double j5=j4*j1;
		
		dx_pi[15]=5*i4;
		dx_pi[16]=4*i3*j1;
		dx_pj[16]=i4;
		dx_pi[17]=3*i2*j2;
		dx_pj[17]=2*i3*j1;
		dx_pi[18]=2*i1*j3;
		dx_pj[18]=3*i2*j2;
		dx_pi[19]=j4;
		dx_pj[19]=4*i1*j3;
		dx_pj[20]=5*j4;

	if(order>5){
	
		double i6=i5*i1;
		double j6=j5*j1;
		
		dx_pi[21]=6*i5;
		dx_pi[22]=5*i4*j1;
		dx_pj[22]=i5;
		dx_pi[23]=4*i3*j2;
		dx_pj[23]=2*i4*j1;
		dx_pi[24]=3*i2*j3;
		dx_pj[24]=3*i3*j2;
		dx_pi[25]=2*i1*j4;
		dx_pj[25]=4*i2*j3;
		dx_pi[26]=j5;
		dx_pj[26]=5*i1*j4;
		dx_pj[27]=6*j5;

	if(order>6){
	
		double i7=i6*i1;
		double j7=j6*j1;
		
		dx_pi[28]=7*i6;
		dx_pi[29]=6*i5*j1;
		dx_pj[29]=i6;
		dx_pi[30]=5*i4*j2;
		dx_pj[30]=2*i5*j1;
		dx_pi[31]=4*i3*j3;
		dx_pj[31]=3*i4*j2;
		dx_pi[32]=3*i2*j4;
		dx_pj[32]=4*i3*j3;
		dx_pi[33]=2*i1*j5;
		dx_pj[33]=5*i2*j4;
		dx_pi[34]=j6;
		dx_pj[34]=6*i1*j5;
		dx_pj[35]=7*j6;

	if(order>7){
	
		double i8=i7*i1;
		double j8=j7*j1;
		
		dx_pi[36]=8*i7;
		dx_pi[37]=7*i6*j1;
		dx_pj[37]=i7;
		dx_pi[38]=6*i5*j2;
		dx_pj[38]=2*i6*j1;
		dx_pi[39]=5*i4*j3;
		dx_pj[39]=3*i5*j2;
		dx_pi[40]=4*i3*j4;
		dx_pj[40]=4*i4*j3;
		dx_pi[41]=3*i2*j5;
		dx_pj[41]=5*i3*j4;
		dx_pi[42]=2*i1*j6;
		dx_pj[42]=6*i2*j5;
		dx_pi[43]=j7;
		dx_pj[43]=7*i1*j6;
		dx_pj[44]=8*j7;

	if(order>8){
	
		double i9=i8*i1;
		double j9=j8*j1;
		dx_pi[45]=9*i8;
		dx_pi[46]=8*i7*j1;
		dx_pj[46]=i8;
		dx_pi[47]=7*i6*j2;
		dx_pj[47]=2*i7*j1;
		dx_pi[48]=6*i5*j3;
		dx_pj[48]=3*i6*j2;
		dx_pi[49]=5*i4*j4;
		dx_pj[49]=4*i5*j3;
		dx_pi[50]=4*i3*j5;
		dx_pj[50]=5*i4*j4;
		dx_pi[51]=3*i2*j6;
		dx_pj[51]=6*i3*j5;
		dx_pi[52]=2*i1*j7;
		dx_pj[52]=7*i2*j6;
		dx_pi[53]=j8;
		dx_pj[53]=8*i1*j7;
		dx_pj[54]=9*j8;

	if(order>9){
	
		double i10=i9*i1;
		double j10=j9*j1;
		
		dx_pi[55]=10*i9;
		dx_pi[56]=9*i8*j1;
		dx_pj[56]=i9;
		dx_pi[57]=8*i7*j2;
		dx_pj[57]=2*i8*j1;
		dx_pi[58]=7*i6*j3;
		dx_pj[58]=3*i7*j2;
		dx_pi[59]=6*i5*j4;
		dx_pj[59]=4*i6*j3;
		dx_pi[60]=5*i4*j5;
		dx_pj[60]=5*i5*j4;
		dx_pi[61]=4*i3*j6;
		dx_pj[61]=6*i4*j5;
		dx_pi[62]=3*i2*j7;
		dx_pj[62]=7*i3*j6;
		dx_pi[63]=2*i1*j8;
		dx_pj[63]=8*i2*j7;
		dx_pi[64]=j9;
		dx_pj[64]=9*i1*j8;
		dx_pj[65]=10*j9;

	if(order>10){
	
		double i11=i10*i1;
		double j11=j10*j1;
		
		dx_pi[66]=11*i10;
		dx_pi[67]=10*i9*j1;
		dx_pj[67]=i10;
		dx_pi[68]=9*i8*j2;
		dx_pj[68]=2*i9*j1;
		dx_pi[69]=8*i7*j3;
		dx_pj[69]=3*i8*j2;
		dx_pi[70]=7*i6*j4;
		dx_pj[70]=4*i7*j3;
		dx_pi[71]=6*i5*j5;
		dx_pj[71]=5*i6*j4;
		dx_pi[72]=5*i4*j6;
		dx_pj[72]=6*i5*j5;
		dx_pi[73]=4*i3*j7;
		dx_pj[73]=7*i4*j6;
		dx_pi[74]=3*i2*j8;
		dx_pj[74]=8*i3*j7;
		dx_pi[75]=2*i1*j9;
		dx_pj[75]=9*i2*j8;
		dx_pi[76]=j10;
		dx_pj[76]=10*i1*j9;
		dx_pj[77]=11*j10;

	if(order>11){
	
		dx_pi[78]=12*i11;
		dx_pi[79]=11*i10*j1;
		dx_pj[79]=i11;
		dx_pi[80]=10*i9*j2;
		dx_pj[80]=2*i10*j1;
		dx_pi[81]=9*i8*j3;
		dx_pj[81]=3*i9*j2;
		dx_pi[82]=8*i7*j4;
		dx_pj[82]=4*i8*j3;
		dx_pi[83]=7*i6*j5;
		dx_pj[83]=5*i7*j4;
		dx_pi[84]=6*i5*j6;
		dx_pj[84]=6*i6*j5;
		dx_pi[85]=5*i4*j7;
		dx_pj[85]=7*i5*j6;
		dx_pi[86]=4*i3*j8;
		dx_pj[86]=8*i4*j7;
		dx_pi[87]=3*i2*j9;
		dx_pj[87]=9*i3*j8;
		dx_pi[88]=2*i1*j10;
		dx_pj[88]=10*i2*j9;
		dx_pi[89]=j11;
		dx_pj[89]=11*i1*j10;
		dx_pj[90]=12*j11;


	} /* End (if order>11) */
	} /* End (if order>10) */
	} /* End (if order>9) */
	} /* End (if order>8) */
	} /* End (if order>7) */
	} /* End (if order>6) */
	} /* End (if order>5) */
	} /* End (if order>4) */
	} /* End (if order>3) */
	} /* End (if order>2) */
	} /* End (if order>1) */
	} /* End (if order>0) */


	/* Find the x coordinate */
	{
		double dx_i=0;
		double dy_i=0;
		double dx_j=0;
		double dy_j=0;

		{
			double *p0=es[0];
			double *p1=es[1];
			
			double *pi=dx_pi;
			double *pj=dx_pj;
			
			unsigned int ii;
			
			for(ii=0;ii<coeffp;ii++){
				dx_i+=*p0**pi;
				dy_i+=*p1**pi++;
				dx_j+=*p0++**pj;
				dy_j+=*p1++**pj++;
			}
		}

		/* Do not assume that ed is only 3x3 */
		ed[0][0]=dx_i;
		ed[1][0]=dx_j;
		ed[2][0]=0;
		ed[0][1]=dy_i;
		ed[1][1]=dy_j;
		ed[2][1]=0;
		ed[0][2]=0;
		ed[1][2]=0;
		ed[2][2]=1;
	}
	return 0;
}
