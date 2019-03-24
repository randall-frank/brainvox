#include "voxel.h"
#include "vl_util.h"
#include <stdio.h>

void vl_image_(VLINT32 imagex,VLINT32 imagey, VLUCHAR *image,
		VLINT16 *zbuf, vset *set)
{
	set->image = image;
	set->zbuffer = zbuf;
	set->imagex = imagex;
	set->imagey = imagey;

        vl_dirty_(VL_DIRTY_IMAGE,set);
}

void vl_pickbuffer_(VLUINT32 * pbuffer , vset * set){
	set->pickbuffer = pbuffer;
}

