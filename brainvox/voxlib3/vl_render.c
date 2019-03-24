#include "voxel.h"
#include "vl_util.h"
#include <stdio.h>
#include <stdlib.h>

void vl_render_(VLINT32 flags, VLINT32 interrpt ,VLINT32 *ret, vset *set)
{
	VLINT32 order[3],step[3];
	VLINT32	mult[3];
	VLINT32 st[3],en[3],ri[3],le[3],sp[3];
	VLINT32 i,vflag;

/* get the axis starts/ends and steps */
	for(i=0;i<3;i++) {
		le[i] = set->start[i];
		ri[i] = set->end[i];
		st[i] = set->start[i];
		en[i] = set->end[i];
	};
	sp[0] = ((set->speed >> 16) & 0xff);
	sp[1] = ((set->speed >> 8) & 0xff);
	sp[2] = ((set->speed) & 0xff);
	(void) axisorder(order, step, set);
/* fudge speed so slowest axis has the skip */
	if ((set->speed & 0x00ffff00) == 0L) {
		sp[order[2]] = 0;
		sp[order[1]] = 0;
		sp[order[0]] = set->speed;
	}
/* apply the skip factor and swap the start.stop */
	for(i=0;i<3;i++) {
		if (step[i] < 0) {
			SWAP(st[i],en[i]);
		};
		if (sp[i] == 0) sp[i] = 1;
		step[i] = step[i] * (sp[i]);
	};
/* get the planar multiplies */
	mult[0] = 1;
	mult[1] = set->d[0];
	mult[2] = (set->d[0])*(set->d[1]);

/* assume no errors */
	*ret = VL_NOERR;
	if ((set->data) == 0) {
		vl_puts("This vset has no data assigned to it!!!\n.");
		*ret = VL_MEMERR;
	 	return;
	};
	if ((flags & VL_BACKGROUND) == VL_BACKGROUND) *ret = VL_BACKGROUND;

/* set up the vflag rendering type */
	vflag = (flags & 0xfffffffe);
	if (set->plane != 0) vflag = (vflag | VL_RENDERPLANE);
	vflag &= (~VL_RENDER3DTEX);
   	vl_pirender(set,st,en,le,ri,step,mult,order,interrpt,vflag,ret); 

	return;
}
