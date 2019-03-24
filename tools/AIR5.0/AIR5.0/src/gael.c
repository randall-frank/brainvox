/* Copyright 1994-2001 Roger P. Woods, M.D. */
/* Modified: 5/8/01 */

/*
 * Inverts a 4 by 4 matrix
 *
 *
 * Returns:
 *	0 if successful
 *	errcode if unsuccessful
 */

#include "AIR.h"
#define SIZE 4

AIR_Error AIR_gael(double **er)

{
	unsigned int ipvt[SIZE];

	if(AIR_dgefa(er,SIZE,ipvt)!=SIZE) return AIR_SINGULAR_GAEL_ERROR;
	{
		double	work[SIZE];

		AIR_dgedi(er,SIZE,ipvt,work);
	}
	return 0;
}
				
