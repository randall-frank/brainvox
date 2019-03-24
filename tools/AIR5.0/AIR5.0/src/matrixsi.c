/* Copyright 1999-2001 Roger P. Woods, M.D. */
/* Modified: 5/22/01 */

/*
 * int *******matrixsi()
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

int *******AIR_matrix7i(const unsigned int g, const unsigned int f, const unsigned int e, const unsigned int d, const unsigned int c, const unsigned int b, const unsigned int a)
{
	int *******high=(int *******)malloc(a*sizeof(int ******));

	if(!high) return NULL;

	if(b!=NULL){
		int ******low=AIR_matrix6i(g,f,e,d,c,b*a);

		if(!low){
			free(high);
			return NULL;
		}
		{
			int *******ptr;
			
			for(ptr=high;ptr<high+a;ptr++,low+=b){
			
				*ptr=low;
			}
		}
	}
	return high;
}
int ******AIR_matrix6i(const unsigned int f, const unsigned int e, const unsigned int d, const unsigned int c, const unsigned int b, const unsigned int a)
{
	int ******high=(int ******)malloc(a*sizeof(int *****));

	if(!high) return NULL;

	if(b!=NULL){
		int *****low=AIR_matrix5i(f,e,d,c,b*a);

		if(!low){
			free(high);
			return NULL;
		}
		{
			int ******ptr;
			
			for(ptr=high;ptr<high+a;ptr++,low+=b){
			
				*ptr=low;
			}
		}
	}
	return high;
}
int *****AIR_matrix5i(const unsigned int e, const unsigned int d, const unsigned int c, const unsigned int b, const unsigned int a)
{
	int *****high=(int *****)malloc(a*sizeof(int ****));

	if(!high) return NULL;

	if(b!=NULL){
		int ****low=AIR_matrix4i(e,d,c,b*a);

		if(!low){
			free(high);
			return NULL;
		}
		{
			int *****ptr;
			
			for(ptr=high;ptr<high+a;ptr++,low+=b){
			
				*ptr=low;
			}
		}
	}
	return high;
}
int ****AIR_matrix4i(const unsigned int d, const unsigned int c, const unsigned int b, const unsigned int a)
{
	int ****high=(int ****)malloc(a*sizeof(int ***));

	if(!high) return NULL;

	if(b!=NULL){
		int ***low=AIR_matrix3i(d,c,b*a);

		if(!low){
			free(high);
			return NULL;
		}
		{
			int ****ptr;
			
			for(ptr=high;ptr<high+a;ptr++,low+=b){
			
				*ptr=low;
			}
		}
	}
	return high;
}
int ***AIR_matrix3i(const unsigned int c, const unsigned int b, const unsigned int a)
{
	int ***high=(int ***)malloc(a*sizeof(int **));

	if(!high) return NULL;

	if(b!=NULL){
		int **low=AIR_matrix2i(c,b*a);

		if(!low){
			free(high);
			return NULL;
		}
		{
			int ***ptr;
			
			for(ptr=high;ptr<high+a;ptr++,low+=b){
			
				*ptr=low;
			}
		}
	}
	return high;
}
int **AIR_matrix2i(const unsigned int b, const unsigned int a)
{
	int **high=(int **)malloc(a*sizeof(int *));

	if(!high) return NULL;

	if(b!=NULL){
		int *low=AIR_matrix1i(b*a);

		if(!low){
			free(high);
			return NULL;
		}
		{
			int **ptr;
			
			for(ptr=high;ptr<high+a;ptr++,low+=b){
			
				*ptr=low;
			}
		}
	}
	return high;
}
int *AIR_matrix1i(const unsigned int a)
{
	int *high=(int *)malloc(a*sizeof(int));

	if(!high) return NULL;

	return high;
}






