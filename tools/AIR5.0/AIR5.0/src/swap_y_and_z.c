/* Copyright 1993-2001 Roger P. Woods, M.D. */
/* Modified: 5/8/01 */

/*
 * This routine will return a volume with the y and z axes
 *  of the calling volume exchanged, but the data otherwise
 *  unaltered.
 *
 * This exchange is not a 90 degree rotation, but rather a mirror
 *  imaging across a line oriented at 45 degrees to the axes being
 *  exchanged.
 *
 * Combining this routine with flip_y or with flip_z will result
 *  in a 90 degree rotation around the x axis without mirror imaging
 *  of the output.       
 *
 * values in new_stats are assigned within this subroutine.
 *
 * stats and new_stats may be identical
 */

#include "AIR.h"

AIR_Pixels ***AIR_swap_y_and_z(AIR_Pixels ***volume, const struct AIR_Key_info *stats, struct AIR_Key_info *new_stats, AIR_Error *errcode)

{
	const unsigned int x_dim=stats->x_dim;
	const unsigned int y_dim=stats->y_dim;
	const unsigned int z_dim=stats->z_dim;
	
	AIR_Pixels ***new_volume=AIR_create_vol3(x_dim,z_dim,y_dim);
	if(!new_volume){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("memory allocation failure\n");
		*errcode=AIR_MEMORY_ALLOCATION_ERROR;
		return NULL;
	}
	/* Fill in the data */
	{
		AIR_Pixels ***k1=volume;
		unsigned int k=0;
		
		for (;k<z_dim;k++,k1++){
		
			AIR_Pixels **j1=*k1;
			unsigned int j=0;
			
			for (;j<y_dim;j++,j1++){
			
				AIR_Pixels *i1=*j1;
				AIR_Pixels *i2=*(*(new_volume+j)+k);
				AIR_Pixels *i1_end=i1+x_dim;
				
				for (;i1<i1_end;i1++,i2++){
					*i2=*i1;
				}
			}
		}
	}
	*new_stats=*stats;
	
	{
		unsigned int temp=stats->y_dim;
		
		new_stats->y_dim=stats->z_dim;
		new_stats->z_dim=temp;
	}
	{
		double temp=stats->y_size;
		
		new_stats->y_size=stats->z_size;
		new_stats->z_size=temp;
	}
	*errcode=0;

	return new_volume;
}
