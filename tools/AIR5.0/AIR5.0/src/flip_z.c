/* Copyright 1993-2001 Roger P. Woods, M.D. */
/* Modified: 5/8/01 */

/*
 * This routine will exchange the z_dimension ordering of a volume
 *
 * Returns:
 *	Pointer to reoriented volume if successful
 *	NULL pointer if unsuccessful
 *
 */

#include "AIR.h"

AIR_Pixels ***AIR_flip_z(AIR_Pixels ***volume, const struct AIR_Key_info *stats, AIR_Error *errcode)

{
	unsigned int x_dim=stats->x_dim;
	unsigned int y_dim=stats->y_dim;
	unsigned int z_dim=stats->z_dim;
	
	/* Allocate memory */
	AIR_Pixels ***new_volume=AIR_create_vol3(x_dim,y_dim,z_dim);
	if(!new_volume){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("memory allocation failure\n");
		*errcode=AIR_MEMORY_ALLOCATION_ERROR;
		return NULL;
	}
	/* Fill in the data */
	{
		AIR_Pixels ***k1=volume;
		AIR_Pixels ***k2=new_volume+z_dim-1;
		AIR_Pixels ***k1_end=k1+z_dim;
		
		for (;k1<k1_end;k1++,k2--){
		
			AIR_Pixels **j1=*k1;
			AIR_Pixels **j2=*k2;
			AIR_Pixels **j1_end=j1+y_dim;
			
			for (;j1<j1_end;j1++,j2++){
			
				AIR_Pixels *i1=*j1;
				AIR_Pixels *i2=*j2;
				AIR_Pixels *i1_end=i1+x_dim;
				
				for (;i1<i1_end;i1++,i2++){
					*i2=*i1;
				}
			}
		}
	}
	*errcode=0;
	return new_volume;
}
