/* Copyright 1993-2001 Roger P. Woods, M.D. */
/* Modified 5/12/01 */

/*
 * returns:
 *	1 if files are identical in dimension,pixel size,and pixel value
 *	0 if not
 */

#include "AIR.h"

AIR_Boolean AIR_identical(AIR_Pixels ***volume1, const struct AIR_Key_info *stats1, AIR_Pixels ***volume2, const struct AIR_Key_info *stats2)

{
	if(stats1->x_dim != stats2->x_dim) return 0;
	if(stats1->y_dim != stats2->y_dim) return 0;
	if(stats1->z_dim != stats2->z_dim) return 0;

	if(fabs(stats1->x_size-stats2->x_size)>AIR_CONFIG_PIX_SIZE_ERR) return 0;
	if(fabs(stats1->y_size-stats2->y_size)>AIR_CONFIG_PIX_SIZE_ERR) return 0;
	if(fabs(stats1->z_size-stats2->z_size)>AIR_CONFIG_PIX_SIZE_ERR) return 0;

	{
		unsigned int x_dim=stats1->x_dim;
		unsigned int y_dim=stats1->y_dim;
		unsigned int z_dim=stats1->z_dim;
		
		unsigned int k;
		AIR_Pixels ***k1,***k2;

		for (k=0,k1=volume1,k2=volume2;k<z_dim;k++,k1++,k2++){
		
			unsigned int j;
			AIR_Pixels **j1,**j2;
			
			for (j=0,j1=*k1,j2=*k2;j<y_dim;j++,j1++,j2++){
			
				unsigned int i;
				AIR_Pixels *i1,*i2;
				
				for (i=0,i1=*j1,i2=*j2;i<x_dim;i++,i1++,i2++){
					if (*i2!=*i1) return 0;
				}
			}
		}
		return TRUE;
	}
}
