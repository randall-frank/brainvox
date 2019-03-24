/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/10/01 */


/*
 * This routine will apply es to i1,j1 to find *i_new,*j_new
 * Routine should return successfully so long as order is in bounds
 * Returns 0 if successful, error code otherwise
 *
 */

#include "AIR.h"

AIR_Error AIR_warp2D(double **es, const double i1, const double j1, double *i_new, double *j_new, const unsigned int order, double *dx)

{
	unsigned int coeffp=(order+1)*(order+2)/2;
	
	if(order>AIR_CONFIG_MAXORDER){
		*i_new=-10000;
		*j_new=-10000;
		return AIR_POLYNOMIAL_ORDER_ERROR;
	}

	dx[0]=1.0;
	if(order!=0){
		dx[1]=i1;
		dx[2]=j1;

	if(order>1){
	
		double i2=i1*i1;
		double j2=j1*j1;
		
		dx[3]=i2;
		dx[4]=i1*j1;
		dx[5]=j2;

	if(order>2){
	
		double i3=i2*i1;
		double j3=j2*j1;
		
		dx[6]=i3;
		dx[7]=i2*j1;
		dx[8]=i1*j2;
		dx[9]=j3;

	if(order>3){
	
		double i4=i3*i1;
		double j4=j3*j1;
		
		dx[10]=i4;
		dx[11]=i3*j1;
		dx[12]=i2*j2;
		dx[13]=i1*j3;
		dx[14]=j4;

	if(order>4){
	
		double i5=i4*i1;
		double j5=j4*j1;
		
		dx[15]=i5;
		dx[16]=i4*j1;
		dx[17]=i3*j2;
		dx[18]=i2*j3;
		dx[19]=i1*j4;
		dx[20]=j5;

	if(order>5){
	
		double i6=i5*i1;
		double j6=j5*j1;
		
		dx[21]=i6;
		dx[22]=i5*j1;
		dx[23]=i4*j2;
		dx[24]=i3*j3;
		dx[25]=i2*j4;
		dx[26]=i1*j5;
		dx[27]=j6;

	if(order>6){
	
		double i7=i6*i1;
		double j7=j6*j1;
		
		dx[28]=i7;
		dx[29]=i6*j1;
		dx[30]=i5*j2;
		dx[31]=i4*j3;
		dx[32]=i3*j4;
		dx[33]=i2*j5;
		dx[34]=i1*j6;
		dx[35]=j7;

	if(order>7){
	
		double i8=i7*i1;
		double j8=j7*j1;
		
		dx[36]=i8;
		dx[37]=i7*j1;
		dx[38]=i6*j2;
		dx[39]=i5*j3;
		dx[40]=i4*j4;
		dx[41]=i3*j5;
		dx[42]=i2*j6;
		dx[43]=i1*j7;
		dx[44]=j8;

	if(order>8){
	
		double i9=i8*i1;
		double j9=j8*j1;
		
		dx[45]=i9;
		dx[46]=i8*j1;
		dx[47]=i7*j2;
		dx[48]=i6*j3;
		dx[49]=i5*j4;
		dx[50]=i4*j5;
		dx[51]=i3*j6;
		dx[52]=i2*j7;
		dx[53]=i1*j8;
		dx[54]=j9;

	if(order>9){
	
		double i10=i9*i1;
		double j10=j9*j1;
		
		dx[55]=i10;
		dx[56]=i9*j1;
		dx[57]=i8*j2;
		dx[58]=i7*j3;
		dx[59]=i6*j4;
		dx[60]=i5*j5;
		dx[61]=i4*j6;
		dx[62]=i3*j7;
		dx[63]=i2*j8;
		dx[64]=i1*j9;
		dx[65]=j10;

	if(order>10){
	
		double i11=i10*i1;
		double j11=j10*j1;
		
		dx[66]=i11;
		dx[67]=i10*j1;
		dx[68]=i9*j2;
		dx[69]=i8*j3;
		dx[70]=i7*j4;
		dx[71]=i6*j5;
		dx[72]=i5*j6;
		dx[73]=i4*j7;
		dx[74]=i3*j8;
		dx[75]=i2*j9;
		dx[76]=i1*j10;
		dx[77]=j11;

	if(order>11){
	
		double i12=i11*i1;
		double j12=j11*j1;
		
		dx[78]=i12;
		dx[79]=i11*j1;
		dx[80]=i10*j2;
		dx[81]=i9*j3;
		dx[82]=i8*j4;
		dx[83]=i7*j5;
		dx[84]=i6*j6;
		dx[85]=i5*j7;
		dx[86]=i4*j8;
		dx[87]=i3*j9;
		dx[88]=i2*j10;
		dx[89]=i1*j11;
		dx[90]=j12;

	} /*End if(order>11)*/
	} /*End if(order>10)*/
	} /*End if(order>9)*/
	} /*End if(order>8)*/
	} /*End if(order>7)*/
	} /*End if(order>6)*/
	} /*End if(order>5)*/
	} /*End if(order>4)*/
	} /*End if(order>3)*/
	} /*End if(order>2)*/
	} /*End if(order>1)*/
	} /*End if(order>0)*/

	/* Find the x,y and z coordinates */
	{
		double x_p=0.0;
		double y_p=0.0;
		
		{
			double *p0=es[0];
			double *p1=es[1];
			double *p3=dx;
			
			unsigned int ii;
			
			for(ii=0;ii<coeffp;ii++){
				/* x_p+=e0[ii]*dx[ii]; */
				/* y_p+=e1[ii]*dx[ii]; */

				x_p+=*p0++**p3;
				y_p+=*p1++**p3++;
			}
		}

		*i_new=x_p;
		*j_new=y_p;
	}
	return 0;
}
