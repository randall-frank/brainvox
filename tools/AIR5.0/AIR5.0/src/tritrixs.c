/* Copyright 1999-2001 Roger P. Woods, M.D. */
/* Modified: 5/22/01 */

/*
 * double *******tritrixs()
 *
 * These routines allocate memory 
 *
 * Note that the indices are called in reverse of the accessing order,
 *  e.g. volume=tritrix3(x,z) assures volume[z-1][x-1][x-1] to be 
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

static double *localtritrix1(const unsigned int a)
{
	double *high=(double *)malloc(a*sizeof(double));

	if(!high) return NULL;

	return high;
}

static double **localtritrix2(const unsigned int b, const unsigned int a)
{
	double **high=(double **)malloc(a*sizeof(double *));

	if(!high) return NULL;

	if(b!=NULL){
		double *low=localtritrix1((b+1)*a/2);

		if(!low){
			free(high);
			return NULL;
		}
		{
			double **ptr;
			unsigned int bb=0;
			
			for(ptr=high; ptr<high+a; ptr++, low+=bb){
				*ptr=low;
				bb++;
				if(bb>b) bb=1;
			}
		}
	}
	return high;
}


double *******AIR_tritrix7(const unsigned int f, const unsigned int e, const unsigned int d, const unsigned int c, const unsigned int b, const unsigned int a)
{
	double *******high=(double *******)malloc(a*sizeof(double ******));
	

	if(!high) return NULL;

	if(b!=NULL){
		double ******low=AIR_tritrix6(f,e,d,c,b*a);

		if(!low){
			free(high);
			return NULL;
		}
		{
			double *******ptr;

			for(ptr=high; ptr<high+a; ptr++, low+=b){
				*ptr=low;
			}
		}
	}
	return high;
}
double ******AIR_tritrix6(const unsigned int e, const unsigned int d, const unsigned int c, const unsigned int b, const unsigned int a)
{
	double ******high=(double ******)malloc(a*sizeof(double *****));

	if(!high) return NULL;

	if(b!=NULL){
		double *****low=AIR_tritrix5(e,d,c,b*a);

		if(!low){
			free(high);
			return NULL;
		}
		{
			double ******ptr;
				
			for(ptr=high; ptr<high+a; ptr++, low+=b){
				*ptr=low;
			}
		}
	}
	return high;
}
double *****AIR_tritrix5(const unsigned int d, const unsigned int c, const unsigned int b, const unsigned int a)
{
	double *****high=(double *****)malloc(a*sizeof(double ****));

	if(!high) return NULL;

	if(b!=NULL){
		double ****low=AIR_tritrix4(d,c,b*a);

		if(!low){
			free(high);
			return NULL;
		}
		{
			double *****ptr;

			for(ptr=high; ptr<high+a; ptr++, low+=b){
				*ptr=low;
			}
		}
	}
	return high;
}
double ****AIR_tritrix4(const unsigned int c, const unsigned int b, const unsigned int a)
{
	double ****high=(double ****)malloc(a*sizeof(double ***));

	if(!high) return NULL;

	if(b!=NULL){
		double ***low=AIR_tritrix3(c,b*a);

		if(!low){
			free(high);
			return NULL;
		}
		{
			double ****ptr;

			for(ptr=high; ptr<high+a; ptr++, low+=b){
				*ptr=low;
			}
		}
	}
	return high;
}
double ***AIR_tritrix3(const unsigned int b, const unsigned int a)
{
	double ***high=(double ***)malloc(a*sizeof(double **));

	if(!high) return NULL;

	if(b!=NULL){
		double **low=localtritrix2(b,b*a);

		if(!low){
			free(high);
			return NULL;
		}
		{
			double ***ptr;

			for(ptr=high; ptr<high+a; ptr++, low+=b){
				*ptr=low;
			}
		}
	}
	return high;
}
double **AIR_tritrix2(const unsigned int a)
{
	double **high=(double **)malloc(a*sizeof(double *));

	if(!high) return NULL;

	if(a!=0){
		double *low=localtritrix1((a+1)*a/2);

		if(!low){
			free(high);
			return NULL;
		}
		{
			double **ptr;
			unsigned int bb=0;
			
			for(ptr=high; ptr<high+a; ptr++, low+=bb){
				*ptr=low;
				bb++;
			}
		}
	}
	return high;
}







