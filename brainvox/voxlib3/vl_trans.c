#include "voxel.h"
#include "vl_util.h"
#include <stdio.h>

void vl_translations_(double tx, double ty, double tz, vset *set)
{
	set->trans[0] = tx;
	set->trans[1] = ty;
	set->trans[2] = tz;
}
/* for FORTRAN users who cannot pass doubles by value */
/* or people who just prefer integers */
void vl_trans_(VLINT32 ix, VLINT32 iy, VLINT32 iz, vset *set)
{
	double tx,ty,tz;
	tx = ix;	
	ty = iy;	
	tz = iz;	
	set->trans[0] = tx;
	set->trans[1] = ty;
	set->trans[2] = tz;
}
