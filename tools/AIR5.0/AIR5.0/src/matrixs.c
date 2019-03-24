/* Copyright 1999-2001 Roger P. Woods, M.D. */
/* Modified: 5/22/01 */

/*
 * double *******matrixs()
 *
 * These routines allocate memory 
 *
 * Note that the indices are called in reverse of the accessing order,
 *  e.g. volume=matrix3(x,y,z) assures volume[z-1][y-1][x-1] to be 
 *  a legitimate address
 *
 * If a dimension is listed as zero, it and all dimensions to its left
 *  are unallocated.
 *
 * Returns:
 *	pointer to allocated memory if successful
 *	NULL pointer if unsuccessful
 *
 */

#include "AIR.h"

double *******AIR_matrix7(const unsigned int g, const unsigned int f, const unsigned int e, const unsigned int d, const unsigned int c, const unsigned int b, const unsigned int a)
{
	double *******high=(double *******)malloc(a*sizeof(double ******));


	if(!high) return NULL;

	if(b!=NULL){
		double ******low=AIR_matrix6(g,f,e,d,c,b*a);

		if(!low){
			free(high);
			return NULL;
		}
		{
			double *******ptr;
				
			for(ptr=high;ptr<high+a;ptr++,low+=b){
			
				*ptr=low;
			}
		}
	}
	return high;
}
double ******AIR_matrix6(const unsigned int f, const unsigned int e, const unsigned int d, const unsigned int c, const unsigned int b, const unsigned int a)
{
	double ******high=(double ******)malloc(a*sizeof(double *****));


	if(!high) return NULL;

	if(b!=NULL){
		double *****low=AIR_matrix5(f,e,d,c,b*a);

		if(!low){
			free(high);
			return NULL;
		}
		{
			double ******ptr;
			
			for(ptr=high;ptr<high+a;ptr++,low+=b){
			
				*ptr=low;
			}
		}
	}
	return high;
}
double *****AIR_matrix5(const unsigned int e, const unsigned int d, const unsigned int c, const unsigned int b, const unsigned int a)
{
	double *****high=(double *****)malloc(a*sizeof(double ****));

	if(!high) return NULL;

	if(b!=NULL){
		double ****low=AIR_matrix4(e,d,c,b*a);


		if(!low){
			free(high);
			return NULL;
		}
		{
			double *****ptr;
			
			for(ptr=high;ptr<high+a;ptr++,low+=b){
			
				*ptr=low;
			}
		}
	}
	return high;
}
double ****AIR_matrix4(const unsigned int d, const unsigned int c, const unsigned int b, const unsigned int a)
{
	double ****high=(double ****)malloc(a*sizeof(double ***));

	if(!high) return NULL;

	if(b!=NULL){
		double ***low=AIR_matrix3(d,c,b*a);

		if(!low){
			free(high);
			return NULL;
		}
		{
			double ****ptr;
			
			for(ptr=high;ptr<high+a;ptr++,low+=b){
			
				*ptr=low;
			}
		}
	}
	return high;
}
double ***AIR_matrix3(const unsigned int c, const unsigned int b, const unsigned int a)
{
	double ***high=(double ***)malloc(a*sizeof(double **));

	if(!high) return NULL;

	if(b!=NULL){
		double **low=AIR_matrix2(c,b*a);


		if(!low){
			free(high);
			return NULL;
		}
		{
			double ***ptr;
			
			for(ptr=high;ptr<high+a;ptr++,low+=b){
			
				*ptr=low;
			}
		}
	}
	return high;
}
double **AIR_matrix2(const unsigned int b, const unsigned int a)
{
	double **high=(double **)malloc(a*sizeof(double *));

	if(!high) return NULL;

	if(b!=NULL){
		double *low=AIR_matrix1(b*a);

		if(!low){
			free(high);
			return NULL;
		}
		{
			double **ptr;
			
			for(ptr=high;ptr<high+a;ptr++,low+=b){
			
				*ptr=low;
			}
		}
	}
	return high;
}
double *AIR_matrix1(const unsigned int a)
{
	double *high=(double *)malloc(a*sizeof(double));

	if(!high) return NULL;

	return high;
}






