#include "voxel.h"
#include "vl_util.h"
#include <stdio.h>

void vl_data_(VLINT32 dx, VLINT32 dy, VLINT32 dz, VLUCHAR *data,
	vset *set)
{
	VLINT32 i;

	set->data = data;
	set->d[0] = dx;
	set->d[1] = dy;
	set->d[2] = dz;
/* set the start and end limits to the data limits */
	for(i=0;i<3;i++){
		set->start[i] = 0;
		set->end[i] = (set->d[i])-1;
	}
	set->aux_data[0] = data;

	vl_dirty_(VL_DIRTY_VOLUME,set);

	return;
}

void vl_dirty_(VLINT32 what, vset *set)
{
        if (what == VL_DIRTY_NONE) {
		set->dirty_flags = 0;  /* all clean */
	} else if (what == VL_DIRTY_ALL) {
		set->dirty_flags = VL_DIRTY_ALL; /* all dirty */
	} else {
		set->dirty_flags |= what;  /* dirty the bits */
	}
}

void vl_aux_data_(VLINT32 volnum,VLUCHAR *data,vset *set)
{

/* check for valid volume specification */
	if (volnum >= VL_MAX_VOLS) {
		vl_puts("Maximum number of volumes exceeded.");
		return;
	}
	set->aux_data[volnum] = data;

/* assure me a bit */
	set->data = set->aux_data[0];

	vl_dirty_(VL_DIRTY_VOLUME,set);

	return;
}

