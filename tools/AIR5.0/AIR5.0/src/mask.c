/* Copyright 1994-2001 Roger P. Woods, M.D. */
/* Modified 5/22/01 */

/*
 * This routine will mask file1 based on file2's 0's
 * with permission, excess data in volume1 will be trucated
 * Warning is issued for incompatible units of measure
 */

#include "AIR.h"

AIR_Error AIR_mask(AIR_Pixels ***volume1, struct AIR_Key_info *stats1, AIR_Pixels ***volume2, const struct AIR_Key_info *stats2, const AIR_Boolean permission)

/* permission allows trucation of data if data>mask*/

{
	const unsigned int
		x_dim=stats1->x_dim,
		y_dim=stats1->y_dim,
		z_dim=stats1->z_dim;
	
	if(stats1->bits!=8*sizeof(AIR_Pixels)||stats2->bits!=8*sizeof(AIR_Pixels)){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("subroutine mask() received unexpected number of bits/pixel\n");
		return AIR_INVALID_MASK_ERROR;
	}

	if(stats1->z_dim>stats2->z_dim){
		if(permission){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("%u planes trucated\n",stats1->z_dim-stats2->z_dim);
			stats1->z_dim=stats2->z_dim;
		}
	}

	{
		AIR_Error errcode=AIR_same_dim(stats1,stats2);
		if(errcode!=0){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("unable to mask due to incompatible dimensions:\n");
			printf("file %u x %u x %u\n",stats1->x_dim,stats1->y_dim,stats1->z_dim);
			printf("mask %u x %u x %u\n",stats2->x_dim,stats2->y_dim,stats2->z_dim);
			return errcode;
		}
	}

	{
		AIR_Error errcode=AIR_same_size(stats1,stats2);
		if(errcode!=0){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("WARNING, mask and data have different units, check headers\n");
			printf("file %e x %e x %e\n",stats1->x_size,stats1->y_size,stats1->z_size);
			printf("mask %e x %e x %e\n",stats2->x_size,stats2->y_size,stats2->z_size);
		}
	}


	{
		AIR_Pixels
			***k1=volume1,
			***k2=volume2,
			***k1_end=volume1+z_dim;
		
		for (;k1<k1_end;k1++,k2++){
		
			AIR_Pixels
				**j1=*k1,
				**j2=*k2,
				**j1_end=*k1+y_dim;
			
			for (;j1<j1_end;j1++,j2++){
			
				AIR_Pixels
					*i1=*j1,
					*i2=*j2,
					*i1_end=*j1+x_dim;
				
				for (;i1<i1_end;i1++,i2++){
					if(*i2==0) *i1=0;
				}
			}
		}
	}
	return 0;
}
