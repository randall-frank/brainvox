/* Copyright 1993-2001 Roger P. Woods, M.D. */
/* Modified 5/8/01 */

/*
 * compares two AIR_Key_info structs to see if voxels are the same size
 * 
 * Returns:
 *	0 if x,y,and z sizes match within tolerance
 *	error code if not
 *
 */

#include "AIR.h"

AIR_Error AIR_same_size(const struct AIR_Key_info *stats1, const struct AIR_Key_info *stats2)

{
	if(fabs(stats1->x_size-stats2->x_size)>AIR_CONFIG_PIX_SIZE_ERR||fabs(stats1->y_size-stats2->y_size)>AIR_CONFIG_PIX_SIZE_ERR||fabs(stats1->z_size-stats2->z_size)>AIR_CONFIG_PIX_SIZE_ERR){
		return AIR_INFO_SIZE_MISMATCH_ERROR;
	}
	return 0;
}
