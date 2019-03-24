#include "voxel.h"
#include "vl_util.h"
#include <stdio.h>

void vl_data_(long int dx, long int dy, long int dz, unsigned char *data,
	vset *set)
{
	long int i;

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

	return;
}

void vl_aux_data_(long int volnum,unsigned char *data,vset *set)
{

/* check for valid volume specification */
	if (volnum >= VL_MAX_VOLS) {
		vl_puts("Maximum number of volumes exceeded.");
		return;
	}
	set->aux_data[volnum] = data;

/* assure me a bit */
	set->data = set->aux_data[0];

	return;
}

