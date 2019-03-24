/* Copyright 1999-2001 Roger P. Woods, M.D. */
/* Modified: 5/22/01 */

/*
 * char *******matrixsc()
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

char ********AIR_matrix8c(const unsigned int h, const unsigned int g, const unsigned int f, const unsigned int e, const unsigned int d, const unsigned int c, const unsigned int b, const unsigned int a)
{
	char ********high=(char ********)malloc(a*sizeof(char *******));

	if(!high) return NULL;

	if(b!=NULL){
		char *******low=AIR_matrix7c(h,g,f,e,d,c,b*a);

		if(!low){
			free(high);
			return NULL;
		}
		{
			char ********ptr;
			
			for(ptr=high;ptr<high+a;ptr++,low+=b){
			
				*ptr=low;
			}
		}
	}
	return high;
}
char *******AIR_matrix7c(const unsigned int g, const unsigned int f, const unsigned int e, const unsigned int d, const unsigned int c, const unsigned int b, const unsigned int a)
{
	char *******high=(char *******)malloc(a*sizeof(char ******));

	if(!high) return NULL;

	if(b!=NULL){
		char ******low=AIR_matrix6c(g,f,e,d,c,b*a);

		if(!low){
			free(high);
			return NULL;
		}
		{
			char *******ptr;
			
			for(ptr=high;ptr<high+a;ptr++,low+=b){
			
				*ptr=low;
			}
		}
	}
	return high;
}
char ******AIR_matrix6c(const unsigned int f, const unsigned int e, const unsigned int d, const unsigned int c, const unsigned int b, const unsigned int a)
{
	char ******high=(char ******)malloc(a*sizeof(char *****));

	if(!high) return NULL;

	if(b!=NULL){
		char *****low=AIR_matrix5c(f,e,d,c,b*a);

		if(!low){
			free(high);
			return NULL;
		}
		{
			char ******ptr;
			
			for(ptr=high;ptr<high+a;ptr++,low+=b){
			
				*ptr=low;
			}
		}
	}
	return high;
}
char *****AIR_matrix5c(const unsigned int e, const unsigned int d, const unsigned int c, const unsigned int b, const unsigned int a)
{
	char *****high=(char *****)malloc(a*sizeof(char ****));

	if(!high) return NULL;

	if(b!=NULL){
		char ****low=AIR_matrix4c(e,d,c,b*a);

		if(!low){
			free(high);
			return NULL;
		}
		{
			char *****ptr;

			for(ptr=high;ptr<high+a;ptr++,low+=b){
			
				*ptr=low;
			}
		}
	}
	return high;
}
char ****AIR_matrix4c(const unsigned int d, const unsigned int c, const unsigned int b, const unsigned int a)
{
	char ****high=(char ****)malloc(a*sizeof(char ***));

	if(!high) return NULL;

	if(b!=NULL){
		char ***low=AIR_matrix3c(d,c,b*a);

		if(!low){
			free(high);
			return NULL;
		}
		{
			char ****ptr;
		
			for(ptr=high;ptr<high+a;ptr++,low+=b){
			
				*ptr=low;
			}
		}
	}
	return high;
}
char ***AIR_matrix3c(const unsigned int c, const unsigned int b, const unsigned int a)
{
	char ***high=(char ***)malloc(a*sizeof(char **));

	if(!high) return NULL;

	if(b!=NULL){
		char **low=AIR_matrix2c(c,b*a);

		if(!low){
			free(high);
			return NULL;
		}
		{
			char ***ptr;
				
			for(ptr=high;ptr<high+a;ptr++,low+=b){
			
				*ptr=low;
			}
		}
	}
	return high;
}
char **AIR_matrix2c(const unsigned int b, const unsigned int a)
{
	char **high=(char **)malloc(a*sizeof(char *));

	if(!high) return NULL;

	if(b!=NULL){
		char *low=AIR_matrix1c(b*a);

		if(!low){
			free(high);
			return NULL;
		}
		{
			char **ptr;
				
			for(ptr=high;ptr<high+a;ptr++,low+=b){
			
				*ptr=low;
			}
		}
	}
	return high;
}
char *AIR_matrix1c(const unsigned int a)
{
	char *high=(char *)malloc(a*sizeof(char));

	if(!high) return NULL;

	return high;
}






