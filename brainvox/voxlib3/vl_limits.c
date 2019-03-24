#include "voxel.h"
#include "vl_util.h"
#include <stdio.h>

void vl_limits_(VLINT32 xs, VLINT32 xe, VLINT32 ys, VLINT32 ye, VLINT32 zs, VLINT32 ze, vset *set)
{
	if (xe < xs) return;
	if (ye < ys) return;
	if (ze < zs) return;
	if ((xs < set->d[0]) && (xs >= 0)) set->start[0] = xs;
	if ((xe < set->d[0]) && (xe >= 0)) set->end[0] = xe;
	if ((ys < set->d[1]) && (ys >= 0)) set->start[1] = ys;
	if ((ye < set->d[1]) && (ye >= 0)) set->end[1] = ye;
	if ((zs < set->d[2]) && (zs >= 0)) set->start[2] = zs;
	if ((ze < set->d[2]) && (ze >= 0)) set->end[2] = ze;
}
