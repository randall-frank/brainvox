/* Copyright 1998-2001 Roger P. Woods, M.D. */
/* Modified 5/8/01 */

/*
 *
 * This routine will combine a 12 parameter affine transform with a
 *  nonlinear transform, thereby altering the nonlinear reslice file's identity
 *
 * Returns 0 if successful, error code otherwise
 */

#include "AIR.h"
#define COORDS_INT 2

AIR_Error AIR_blat40(double **es, double **ea, const unsigned int order)

{
	unsigned int coeffp=(order+1)*(order+2)/2;
	if(order>AIR_CONFIG_MAXORDER) return AIR_POLYNOMIAL_ORDER_ERROR;
	
	{
		double **out=AIR_matrix2(coeffp,COORDS_INT);
		if(!out){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("unable to allocate memory\n");
			return AIR_MEMORY_ALLOCATION_ERROR;
		}
		{
			unsigned int m;
			
			for(m=0;m<COORDS_INT;m++){
			
				unsigned int i;
				
				for(i=0;i<coeffp;i++){
					out[m][i]=0.0;
				}
			}
		}

		out[0][0]+=ea[0][0];
		out[1][0]+=ea[1][0];

		{
			unsigned int m;
			
			for(m=0;m<COORDS_INT;m++){
			
				unsigned int i;
				
				for(i=0;i<coeffp;i++){
					out[m][i]+=ea[m][1]*es[0][i];
					out[m][i]+=ea[m][2]*es[1][i];
				}
			}
		}
		{
			unsigned int m;
			
			for(m=0;m<COORDS_INT;m++){

				unsigned int i;
				
				for(i=0;i<coeffp;i++){
					es[m][i]=out[m][i];
				}
			}
		}
		AIR_free_2(out);
	}
	return 0;
}
