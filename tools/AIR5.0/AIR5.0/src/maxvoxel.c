/* Copyright 1998-2001 Roger P. Woods, M.D. */
/* Modified: 5/5/01 */

/* 
 * AIR_Pixels maxvoxel()
 *
 * This routine finds the voxel with the highest value
 *
 * Returns:
 *	the highest voxel value
 */

#include "AIR.h"

AIR_Pixels AIR_maxvoxel(AIR_Pixels ***volume, const struct AIR_Key_info *stats)

{
	AIR_Pixels n=0;

	const unsigned int x_dim=stats->x_dim;
	const unsigned int y_dim=stats->y_dim;
	const unsigned int z_dim=stats->z_dim;
	
	{
		AIR_Pixels ***k0=volume;
		AIR_Pixels ***k_end=k0+z_dim;
		
		for(;k0<k_end;k0++){
		
			AIR_Pixels **j0=*k0;
			AIR_Pixels **j_end=j0+y_dim;
			
			for(;j0<j_end;j0++){
			
				AIR_Pixels *i0=*j0;
				AIR_Pixels *i_end=i0+x_dim;
				
				for(;i0<i_end;i0++){
					if(*i0>n) n=*i0;
				}
			}
		}
	}
	return n;
}
