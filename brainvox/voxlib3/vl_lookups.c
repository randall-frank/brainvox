#include "voxel.h"
#include "vl_util.h"
#include <stdio.h>

void vl_lookups_(VLINT32 *rlut, VLINT32 *llut, VLINT32 background, vset *set)
{
	VLINT32 i;
	
	for(i=0; i<256;i++) {
/* lut is $aabbggrr */
		set->rlut[i] = rlut[i];
		set->llut[i] = llut[i]; 
	}
	set->backpack = background;

        vl_dirty_(VL_DIRTY_PALETTE,set);
}
