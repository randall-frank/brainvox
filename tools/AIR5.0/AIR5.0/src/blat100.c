/* Copyright 1998-2001 Roger P. Woods, M.D. */
/* Modified 5/22/01 */

/*
 * This routine will combine a 12 parameter affine transform with a
 *  2D nonlinear transform, thereby altering the standard file's identity
 *
 * Returns 0 if successful, error code otherwise
 */

#include "AIR.h"
#define	COORDS_INT 2


static void free_function(/*@only@*/ double **out, /*@only@*/ double *work1, /*@only@*/ double *work2)

{
	if(out) AIR_free_2(out);
	if(work1) free(work1);
	if(work2) free(work2);
}

static void by2(double *a, double *b, double *c, const unsigned int order)
{
	unsigned int coeffp=(order+1)*(order+2)/2;
	{
		unsigned int i;
		double *cptr=c;
		
		for(i=coeffp;i--!=0;){
			*cptr++=0.0;
		}
	}

	if(order!=0){

		c[0]+=a[0]*b[0];

		c[1]+=a[1]*b[0];

		c[2]+=a[2]*b[0];

	if(order>1){

		c[1]+=a[0]*b[1];
		c[2]+=a[0]*b[2];

		c[3]+=a[1]*b[1];
		c[4]+=a[1]*b[2];

		c[4]+=a[2]*b[1];
		c[5]+=a[2]*b[2];

	if(order>2){

		c[3]+=a[0]*b[3];
		c[4]+=a[0]*b[4];
		c[5]+=a[0]*b[5];

		c[6]+=a[1]*b[3];
		c[7]+=a[1]*b[4];
		c[8]+=a[1]*b[5];

		c[7]+=a[2]*b[3];
		c[8]+=a[2]*b[4];
		c[9]+=a[2]*b[5];

	if(order>3){

		c[6]+=a[0]*b[6];
		c[7]+=a[0]*b[7];
		c[8]+=a[0]*b[8];
		c[9]+=a[0]*b[9];

		c[10]+=a[1]*b[6];
		c[11]+=a[1]*b[7];
		c[12]+=a[1]*b[8];
		c[13]+=a[1]*b[9];

		c[11]+=a[2]*b[6];
		c[12]+=a[2]*b[7];
		c[13]+=a[2]*b[8];
		c[14]+=a[2]*b[9];

	if(order>4){

		c[10]+=a[0]*b[10];
		c[11]+=a[0]*b[11];
		c[12]+=a[0]*b[12];
		c[13]+=a[0]*b[13];
		c[14]+=a[0]*b[14];

		c[15]+=a[1]*b[10];
		c[16]+=a[1]*b[11];
		c[17]+=a[1]*b[12];
		c[18]+=a[1]*b[13];
		c[19]+=a[1]*b[14];

		c[16]+=a[2]*b[10];
		c[17]+=a[2]*b[11];
		c[18]+=a[2]*b[12];
		c[19]+=a[2]*b[13];
		c[20]+=a[2]*b[14];

	if(order>5){

		c[15]+=a[0]*b[15];
		c[16]+=a[0]*b[16];
		c[17]+=a[0]*b[17];
		c[18]+=a[0]*b[18];
		c[19]+=a[0]*b[19];
		c[20]+=a[0]*b[20];

		c[21]+=a[1]*b[15];
		c[22]+=a[1]*b[16];
		c[23]+=a[1]*b[17];
		c[24]+=a[1]*b[18];
		c[25]+=a[1]*b[19];
		c[26]+=a[1]*b[20];

		c[22]+=a[2]*b[15];
		c[23]+=a[2]*b[16];
		c[24]+=a[2]*b[17];
		c[25]+=a[2]*b[18];
		c[26]+=a[2]*b[19];
		c[27]+=a[2]*b[20];

	if(order>6){

		c[21]+=a[0]*b[21];
		c[22]+=a[0]*b[22];
		c[23]+=a[0]*b[23];
		c[24]+=a[0]*b[24];
		c[25]+=a[0]*b[25];
		c[26]+=a[0]*b[26];
		c[27]+=a[0]*b[27];

		c[28]+=a[1]*b[21];
		c[29]+=a[1]*b[22];
		c[30]+=a[1]*b[23];
		c[31]+=a[1]*b[24];
		c[32]+=a[1]*b[25];
		c[33]+=a[1]*b[26];
		c[34]+=a[1]*b[27];

		c[29]+=a[2]*b[21];
		c[30]+=a[2]*b[22];
		c[31]+=a[2]*b[23];
		c[32]+=a[2]*b[24];
		c[33]+=a[2]*b[25];
		c[34]+=a[2]*b[26];
		c[35]+=a[2]*b[27];

	if(order>7){

		c[28]+=a[0]*b[28];
		c[29]+=a[0]*b[29];
		c[30]+=a[0]*b[30];
		c[31]+=a[0]*b[31];
		c[32]+=a[0]*b[32];
		c[33]+=a[0]*b[33];
		c[34]+=a[0]*b[34];
		c[35]+=a[0]*b[35];

		c[36]+=a[1]*b[28];
		c[37]+=a[1]*b[29];
		c[38]+=a[1]*b[30];
		c[39]+=a[1]*b[31];
		c[40]+=a[1]*b[32];
		c[41]+=a[1]*b[33];
		c[42]+=a[1]*b[34];
		c[43]+=a[1]*b[35];

		c[37]+=a[2]*b[28];
		c[38]+=a[2]*b[29];
		c[39]+=a[2]*b[30];
		c[40]+=a[2]*b[31];
		c[41]+=a[2]*b[32];
		c[42]+=a[2]*b[33];
		c[43]+=a[2]*b[34];
		c[44]+=a[2]*b[35];

	if(order>8){

		c[36]+=a[0]*b[36];
		c[37]+=a[0]*b[37];
		c[38]+=a[0]*b[38];
		c[39]+=a[0]*b[39];
		c[40]+=a[0]*b[40];
		c[41]+=a[0]*b[41];
		c[42]+=a[0]*b[42];
		c[43]+=a[0]*b[43];
		c[44]+=a[0]*b[44];

		c[45]+=a[1]*b[36];
		c[46]+=a[1]*b[37];
		c[47]+=a[1]*b[38];
		c[48]+=a[1]*b[39];
		c[49]+=a[1]*b[40];
		c[50]+=a[1]*b[41];
		c[51]+=a[1]*b[42];
		c[52]+=a[1]*b[43];
		c[53]+=a[1]*b[44];

		c[46]+=a[2]*b[36];
		c[47]+=a[2]*b[37];
		c[48]+=a[2]*b[38];
		c[49]+=a[2]*b[39];
		c[50]+=a[2]*b[40];
		c[51]+=a[2]*b[41];
		c[52]+=a[2]*b[42];
		c[53]+=a[2]*b[43];
		c[54]+=a[2]*b[44];

	if(order>9){

		c[45]+=a[0]*b[45];
		c[46]+=a[0]*b[46];
		c[47]+=a[0]*b[47];
		c[48]+=a[0]*b[48];
		c[49]+=a[0]*b[49];
		c[50]+=a[0]*b[50];
		c[51]+=a[0]*b[51];
		c[52]+=a[0]*b[52];
		c[53]+=a[0]*b[53];
		c[54]+=a[0]*b[54];

		c[55]+=a[1]*b[45];
		c[56]+=a[1]*b[46];
		c[57]+=a[1]*b[47];
		c[58]+=a[1]*b[48];
		c[59]+=a[1]*b[49];
		c[60]+=a[1]*b[50];
		c[61]+=a[1]*b[51];
		c[62]+=a[1]*b[52];
		c[63]+=a[1]*b[53];
		c[64]+=a[1]*b[54];

		c[56]+=a[2]*b[45];
		c[57]+=a[2]*b[46];
		c[58]+=a[2]*b[47];
		c[59]+=a[2]*b[48];
		c[60]+=a[2]*b[49];
		c[61]+=a[2]*b[50];
		c[62]+=a[2]*b[51];
		c[63]+=a[2]*b[52];
		c[64]+=a[2]*b[53];
		c[65]+=a[2]*b[54];

	if(order>10){

		c[55]+=a[0]*b[55];
		c[56]+=a[0]*b[56];
		c[57]+=a[0]*b[57];
		c[58]+=a[0]*b[58];
		c[59]+=a[0]*b[59];
		c[60]+=a[0]*b[60];
		c[61]+=a[0]*b[61];
		c[62]+=a[0]*b[62];
		c[63]+=a[0]*b[63];
		c[64]+=a[0]*b[64];
		c[65]+=a[0]*b[65];

		c[66]+=a[1]*b[55];
		c[67]+=a[1]*b[56];
		c[68]+=a[1]*b[57];
		c[69]+=a[1]*b[58];
		c[70]+=a[1]*b[59];
		c[71]+=a[1]*b[60];
		c[72]+=a[1]*b[61];
		c[73]+=a[1]*b[62];
		c[74]+=a[1]*b[63];
		c[75]+=a[1]*b[64];
		c[76]+=a[1]*b[65];

		c[67]+=a[2]*b[55];
		c[68]+=a[2]*b[56];
		c[69]+=a[2]*b[57];
		c[70]+=a[2]*b[58];
		c[71]+=a[2]*b[59];
		c[72]+=a[2]*b[60];
		c[73]+=a[2]*b[61];
		c[74]+=a[2]*b[62];
		c[75]+=a[2]*b[63];
		c[76]+=a[2]*b[64];
		c[77]+=a[2]*b[65];

	if(order>11){

		c[66]+=a[0]*b[66];
		c[67]+=a[0]*b[67];
		c[68]+=a[0]*b[68];
		c[69]+=a[0]*b[69];
		c[70]+=a[0]*b[70];
		c[71]+=a[0]*b[71];
		c[72]+=a[0]*b[72];
		c[73]+=a[0]*b[73];
		c[74]+=a[0]*b[74];
		c[75]+=a[0]*b[75];
		c[76]+=a[0]*b[76];
		c[77]+=a[0]*b[77];

		c[78]+=a[1]*b[66];
		c[79]+=a[1]*b[67];
		c[80]+=a[1]*b[68];
		c[81]+=a[1]*b[69];
		c[82]+=a[1]*b[70];
		c[83]+=a[1]*b[71];
		c[84]+=a[1]*b[72];
		c[85]+=a[1]*b[73];
		c[86]+=a[1]*b[74];
		c[87]+=a[1]*b[75];
		c[88]+=a[1]*b[76];
		c[89]+=a[1]*b[77];

		c[79]+=a[2]*b[66];
		c[80]+=a[2]*b[67];
		c[81]+=a[2]*b[68];
		c[82]+=a[2]*b[69];
		c[83]+=a[2]*b[70];
		c[84]+=a[2]*b[71];
		c[85]+=a[2]*b[72];
		c[86]+=a[2]*b[73];
		c[87]+=a[2]*b[74];
		c[88]+=a[2]*b[75];
		c[89]+=a[2]*b[76];
		c[90]+=a[2]*b[77];
	} /* End if(order>11) */
	} /* End if(order>10) */
	} /* End if(order>9) */
	} /* End if(order>8) */
	} /* End if(order>7) */
	} /* End if(order>6) */
	} /* End if(order>5) */
	} /* End if(order>4) */
	} /* End if(order>3) */
	} /* End if(order>2) */
	} /* End if(order>1) */
	} /* End if(order>0) */
}

AIR_Error AIR_blat100(double **nl, double **lin, const unsigned int order)
{
	if(order>AIR_CONFIG_MAXORDER) return AIR_POLYNOMIAL_ORDER_ERROR;
	{
		unsigned int coeffp=(order+1)*(order+2)/2;

		double **out=AIR_matrix2(coeffp,COORDS_INT);
		double *work1=AIR_matrix1(coeffp);
		double *work2=AIR_matrix1(coeffp);
	
		if(!out || !work1 || !work2){
			free_function(out,work1,work2);
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("memory allocation failure\n");
			return AIR_MEMORY_ALLOCATION_ERROR;
		}
		{
			unsigned int m;
			
			for(m=0;m<COORDS_INT;m++){
				{
					unsigned int i;
					
					for(i=0;i<coeffp;i++){
						out[m][i]=0.0;
					}
				}
				out[m][0]+=nl[m][0];	/* Zeroth order */
			}
		}
		{
			unsigned int m;
			
			for(m=0;m<COORDS_INT;m++){
	
				unsigned int i=0;
				unsigned int currentorder=0;
	
				while(currentorder<order){
	
					currentorder++;
					coeffp=(currentorder+1)*(currentorder+2)/2;
	
					{
						unsigned int j;
						
						for(j=0;j<coeffp;j++){
							work1[j]=0.0;
							work2[j]=0.0;
						}
					}
					{
						unsigned int xs=currentorder+1;
						unsigned int ys=0;
						
						for(;xs--!=0;ys++){
						
							unsigned int k=1;
							signed int parity=1;
							
							i++;
							work1[0]=1.0;
							{
								unsigned int j;
								
								for(j=0;j<xs;j++){
									if(parity==1) by2(lin[0],work1,work2,k);
									else by2(lin[0],work2,work1,k);
									k++;
									parity*=-1;
								}
							}
							{
								unsigned int j;
								
								for(j=0;j<ys;j++){
									if(parity==1) by2(lin[1],work1,work2,k);
									else by2(lin[1],work2,work1,k);
									k++;
									parity*=-1;
								}
							}
							{
								unsigned int j;
								
								for(j=0;j<coeffp;j++){
									if(parity==-1){
										out[m][j]+=work2[j]*nl[m][i];
									}
									else{
										out[m][j]+=work1[j]*nl[m][i];
									}
								}
							}
						}
					}
				}
			}
		}
		{
			unsigned int m;
			
			for(m=0;m<COORDS_INT;m++){
			
				unsigned int j;
				
				for(j=0;j<coeffp;j++){
					nl[m][j]=out[m][j];
				}
			}
		}
		free_function(out,work1,work2);
		return 0;
	}
}

