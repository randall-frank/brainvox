#include "voxel.h"
#include "vl_util.h"
#include <stdio.h>

void vl_init_(vset *set)
{
	long int i;

/*	no data set */
	set->data = NIL;
	for(i=0;i<VL_MAX_VOLS;i++) {
		set->aux_data[i] = NIL;
	}

/*	start and ends all 0 */
	for(i=0; i<3; i++) {
		set->d[i] = 0;
		set->start[i] = 0;
		set->end[i] = 0;
/* rots and trans = 0 */
		set->rots[i] = 0.0;
		set->trans[i] = 0.0;
/* plane */
		set->planept[i] = 0.0;
		set->planevec[i] = 0.0;
	}
/* speed = 1 */

	set->speed = 1;
/* no output image */
	set->imagex = 0;
	set->imagey = 0;
	set->image = NIL;
	set->zbuffer = NIL;
/* no plane */
	set->plane = 0;
/* background is black */
	set->backpack = 0;
/* lookup tables are a gray ramp */
	for(i=0; i<256; i++) {
		set->rlut[i] = (i) | (i<<8) | (i<<16) | (i<<24);
		set->llut[i] = (i) | (i<<8) | (i<<16) | (i<<24);
	}

	(void) vl_calcmatrix(set->rots,set->xform);

	set->squeeze_factor = 1.0;
}
