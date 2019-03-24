/* Copyright 1993-2001 Roger P. Woods, M.D. */
/* Modified 5/27/01 */

/*
 * This routine will fill a volume with zeros
 */

#include "AIR.h"

void AIR_zeros(AIR_Pixels ***volume, const struct AIR_Key_info *stats)

{
	const unsigned int
		x_dim=stats->x_dim,
		y_dim=stats->y_dim,
		z_dim=stats->z_dim;

	AIR_Pixels ***k1=volume;
	AIR_Pixels ***k1_end=k1+z_dim;
	
	for (;k1<k1_end;k1++){
	
		AIR_Pixels **j1=*k1;
		AIR_Pixels **j1_end=j1+y_dim;
		
		for (;j1<j1_end;j1++){
		
			AIR_Pixels *i1=*j1;
			AIR_Pixels *i1_end=i1+x_dim;
			
			for (;i1<i1_end;i1++){
				*i1=0;
			}
		}
	}
}
