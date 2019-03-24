/* Copyright 1998-2001 Roger P. Woods, M.D. */
/* Modified: 5/22/01 */

/*
 * double **promote()
 *
 * promotes a 2 coordinate .warp file matrix to a 3 three coordinate matrix
 *
 * memory associated with the original matrix is NOT freed
 *
 * returns:
 *	**e if successful
 *	NULL if unsuccessful					
 */


#include "AIR.h"

double **AIR_promote(const unsigned int order, double **e, AIR_Error *errcode)

{

	double	**g;

	if(order>AIR_CONFIG_MAXORDER){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("Unimplemented order in function promote()\n");
		*errcode=AIR_POLYNOMIAL_ORDER_ERROR;
		return NULL;
	}

	g=AIR_matrix2((order+1)*(order+2)*(order+3)/6,3);
	if(!g){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("memory allocation failure\n");
		*errcode=AIR_MEMORY_ALLOCATION_ERROR;
		return NULL;
	}
	{
		double *p0g=g[0];
		double *p1g=g[1];
		double *p2g=g[2];
		double* p0e=e[0];
		double *p1e=e[1];
		
		unsigned int i=0;
		unsigned int ii=0;
		unsigned int k;
		
		for(k=0;k<=order;k++){
			for(;i<(k+1)*(k+2)/2;i++,ii++){
				*p0g++=*p0e++;
				*p1g++=*p1e++;
				*p2g++=0.0;
			}
			for(;ii<(k+1)*(k+2)*(k+3)/6;ii++){
				*p0g++=0.0;
				*p1g++=0.0;
				*p2g++=0.0;
			}
		}
	}
	if(order!=0) g[2][3]=1.0;

	*errcode=0;
	return g;
}
