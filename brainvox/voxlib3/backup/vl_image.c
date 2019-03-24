#include "voxel.h"
#include "vl_util.h"
#include <stdio.h>

void vl_image_(long int imagex,long int imagey, unsigned char *image,
		short *zbuf, vset *set)
{
	set->image = image;
	set->zbuffer = zbuf;
	set->imagex = imagex;
	set->imagey = imagey;
}
