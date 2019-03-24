#include "voxel.h"
#include "vl_util.h"
#include <stdio.h>

void vl_setsqu_(VLFLOAT32 *squ, vset *set)
{
	set->squeeze_factor = *squ;
}
void vl_getsqu_(VLFLOAT32 *squ, vset *set)
{
	*squ = set->squeeze_factor;
}
void vl_setinterp_(VLINT32 interp, vset *set)
{
	set->num_interp = interp;
}
