#include "voxel.h"
#include "vl_util.h"
#include <stdio.h>
#include <math.h>

void vl_plane_(VLINT32 toggle, VLFLOAT64 *point, VLFLOAT64 *normal, vset *set)
{
	VLINT32 i;
	VLFLOAT64 length;

	set->plane = toggle;
	length =normal[0]*normal[0] + normal[1]*normal[1] + normal[2]*normal[2];
	if (fabs(length) < 0.00001) length = 1.0;
	length = sqrt(length);
	for(i=0; i<3; i++) {
		set->planept[i] = point[i];
		set->planevec[i] = normal[i]/length;
	}
}
