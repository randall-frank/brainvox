#include "voxel.h"
#include "vl_util.h"
#include <stdio.h>

void vl_setsqu_(float *squ, vset *set)
{
	set->squeeze_factor = *squ;
}
void vl_getsqu_(float *squ, vset *set)
{
	*squ = set->squeeze_factor;
}
