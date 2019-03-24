/* Copyright 1993-2001 Roger P. Woods, M.D. */
/* Modified 5/8/01 */

/*
 * compares two AIR_Key_info structs to see if matrices have the
 *  same dimensions.
 *
 * Returns:
 *	0 if x,y,and z dimensions match
 *	error code if not
 *
 */

#include "AIR.h"

AIR_Error AIR_same_dim(const struct AIR_Key_info *stats1, const struct AIR_Key_info *stats2)

{
	if(stats1->x_dim!=stats2->x_dim||stats1->y_dim!=stats2->y_dim||stats1->z_dim!=stats2->z_dim){
		return AIR_DIMENSION_MISMATCH_ERROR;
	}
	return 0;
}
