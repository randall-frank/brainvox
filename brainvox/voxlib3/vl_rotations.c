#include "voxel.h"
#include "vl_util.h"
#include <stdio.h>

void vl_rotations_(VLFLOAT64 xr, VLFLOAT64 yr, VLFLOAT64 zr, vset *set)
{
	set->rots[0] = xr;
	set->rots[1] = yr;
	set->rots[2] = zr;
 	(void) vl_calcmatrix(set->rots,set->xform);
}
/* for FORTRAN callers: an integer version
	rotations are in tenths of degrees (like rot) */
void vl_rots_(VLINT32 xi, VLINT32 yi, VLINT32 zi, vset *set)
{
	VLFLOAT64 xr,yr,zr;
/* convert to doubles */ 
	xr = xi;    
	yr = yi;    
	zr = zi;    
/* convert to degrees */
	xr = xr / 10.0;
	yr = yr / 10.0;
	zr = zr / 10.0;
/* assign the values */
	set->rots[0] = xr;
	set->rots[1] = yr;
	set->rots[2] = zr;

 	(void) vl_calcmatrix(set->rots,set->xform);
}
