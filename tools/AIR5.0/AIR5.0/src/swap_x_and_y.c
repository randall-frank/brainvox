/* Copyright 1993-2001 Roger P. Woods, M.D. */
/* Modified: 5/8/01 */

/*
 * AIR_Pixels ***swap_x_and_y()
 *
 * This routine will return a volume with the x and y axes
 *  of the calling volume exchanged, but the data otherwise
 *  unaltered.
 *
 * This exchange is not a 90 degree rotation, but rather a mirror
 *  imaging across a line oriented at 45 degrees to the axes being
 *  exchanged.
 *
 * Combining this routine with flip_x or with flip_y will result
 *  in a 90 degree rotation around the z axis without mirror imaging
 *  of the output.
 *
 * values in new_stats are assigned within this subroutine.
 *
 * stats and new_stats may be identical
 */

#include "AIR.h"

AIR_Pixels ***AIR_swap_x_and_y(AIR_Pixels ***volume,const struct AIR_Key_info *stats, struct AIR_Key_info *new_stats, AIR_Error *errcode)

{
	const unsigned int x_dim=stats->x_dim;
	const unsigned int y_dim=stats->y_dim;
	const unsigned int z_dim=stats->z_dim;
	
	/* Allocate memory */
	AIR_Pixels ***new_volume=AIR_create_vol3(y_dim,x_dim,z_dim);
	if(!new_volume){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("memory allocation failure\n");
		*errcode=AIR_MEMORY_ALLOCATION_ERROR;
		return NULL;
	}
	/* Fill in the data */
	{
		AIR_Pixels ***k1=volume;
		AIR_Pixels ***k2=new_volume;
		AIR_Pixels ***k1_end=k1+z_dim;
		
		for (;k1<k1_end;k1++,k2++){
		
			AIR_Pixels **j1=*k1;
			unsigned int j=0;
			
			for (;j<y_dim;j++,j1++){
			
				AIR_Pixels *i1=*j1;
				unsigned int i=0;
				
				for (;i<x_dim;i++,i1++){
					*(*(*k2+i)+j)=*i1;
				}
			}
		}
	}
	*new_stats=*stats;
	{
		unsigned int temp=stats->x_dim;
		
		new_stats->x_dim=stats->y_dim;
		new_stats->y_dim=temp;
	}
	{
		double temp=stats->x_size;
		
		new_stats->x_size=stats->y_size;
		new_stats->y_size=temp;
	}
	*errcode=0;

	return new_volume;
}
