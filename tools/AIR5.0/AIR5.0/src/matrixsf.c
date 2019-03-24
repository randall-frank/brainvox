/* Copyright 1999-2001 Roger P. Woods, M.D. */
/* Modified: 5/22/01 */

/*
 * float *******matrixsf()
 *
 * These routines allocate memory 
 *
 * Note that the indices are called in reverse of the accessing order,
 *  e.g. volume=matrix3f(x,y,z) assures volume[z-1][y-1][x-1] to be 
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

float *******AIR_matrix7f(const unsigned int g, const unsigned int f, const unsigned int e, const unsigned int d, const unsigned int c, const unsigned int b, const unsigned int a)
{
	float *******high=(float *******)malloc(a*sizeof(float ******));

	if(!high) return NULL;

	if(b!=NULL){
		float ******low=AIR_matrix6f(g,f,e,d,c,b*a);

		if(!low){
			free(high);
			return NULL;
		}
		{
			float *******ptr;
			
			for(ptr=high;ptr<high+a;ptr++,low+=b){
			
				*ptr=low;
			}
		}
	}
	return high;
}
float ******AIR_matrix6f(const unsigned int f, const unsigned int e, const unsigned int d, const unsigned int c, const unsigned int b, const unsigned int a)
{
	float ******high=(float ******)malloc(a*sizeof(float *****));

	if(!high) return NULL;

	if(b!=NULL){
		float *****low=AIR_matrix5f(f,e,d,c,b*a);

		if(!low){
			free(high);
			return NULL;
		}
		{
			float ******ptr;
			
			for(ptr=high;ptr<high+a;ptr++,low+=b){
			
				*ptr=low;
			}
		}
	}
	return high;
}
float *****AIR_matrix5f(const unsigned int e, const unsigned int d, const unsigned int c, const unsigned int b, const unsigned int a)
{
	float *****high=(float *****)malloc(a*sizeof(float ****));

	if(!high) return NULL;

	if(b!=NULL){
		float ****low=AIR_matrix4f(e,d,c,b*a);

		if(!low){
			free(high);
			return NULL;
		}
		{
			float *****ptr;
			
			for(ptr=high;ptr<high+a;ptr++,low+=b){
			
				*ptr=low;
			}
		}
	}
	return high;
}
float ****AIR_matrix4f(const unsigned int d, const unsigned int c, const unsigned int b, const unsigned int a)
{
	float ****high=(float ****)malloc(a*sizeof(float ***));

	if(!high) return NULL;

	if(b!=NULL){
		float ***low=AIR_matrix3f(d,c,b*a);

		if(!low){
			free(high);
			return NULL;
		}
		{
			float ****ptr;
			
			for(ptr=high;ptr<high+a;ptr++,low+=b){
			
				*ptr=low;
			}
		}
	}
	return high;
}
float ***AIR_matrix3f(const unsigned int c, const unsigned int b, const unsigned int a)
{
	float ***high=(float ***)malloc(a*sizeof(float **));

	if(!high) return NULL;

	if(b!=NULL){
		float **low=AIR_matrix2f(c,b*a);
		
		if(!low){
			free(high);
			return NULL;
		}
		{
			float ***ptr;
			
			for(ptr=high;ptr<high+a;ptr++,low+=b){
			
				*ptr=low;
			}
		}
	}
	return high;
}
float **AIR_matrix2f(const unsigned int b, const unsigned int a)
{
	float **high=(float **)malloc(a*sizeof(float *));

	if(!high) return NULL;

	if(b!=NULL){
		float *low=AIR_matrix1f(b*a);

		if(!low){
			free(high);
			return NULL;
		}
		{
			float **ptr;
			
			for(ptr=high;ptr<high+a;ptr++,low+=b){
			
				*ptr=low;
			}
		}
	}
	return high;
}
float *AIR_matrix1f(const unsigned int a)
{
	float *high=(float *)malloc(a*sizeof(float));

	if(!high) return NULL;

	return high;
}






