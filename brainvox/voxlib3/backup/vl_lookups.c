#include "voxel.h"
#include "vl_util.h"
#include <stdio.h>

void vl_lookups_(long int *rlut, long int *llut, long int background, vset *set)
{
	long int i;
	
	for(i=0; i<256;i++) {
/* lut is $aabbggrr */
		set->rlut[i] = rlut[i];
		set->llut[i] = llut[i]; 
	}
	set->backpack = background;
}
