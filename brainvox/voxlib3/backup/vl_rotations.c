#include "voxel.h"
#include "vl_util.h"
#include <stdio.h>

void vl_rotations_(double xr, double yr, double zr, vset *set)
{
	set->rots[0] = xr;
	set->rots[1] = yr;
	set->rots[2] = zr;
 	(void) vl_calcmatrix(set->rots,set->xform);
}
/* for FORTRAN callers: an integer version
	rotations are in tenths of degrees (like rot) */
void vl_rots_(long int xi, long int yi, long int zi, vset *set)
{
	double xr,yr,zr;
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
