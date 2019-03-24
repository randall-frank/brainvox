
#include <voxel.h>
#include <vl_util.h>
#include <math.h>

VLINT32 vl_isinvolume_(vset * set, VLINT32 point [3]) {
VLINT32 i;
	if (!set->data) return FALSE;
	for (i=0; i<3; i++) {
		if ((point[i] < 0) || (point[i] >= set->d[i])) return FALSE;
	}
	return TRUE;
}

VLINT32 vl_volume2image_(vset *set,
					VLINT32 vx, VLINT32 vy, VLINT32 vz,
					VLINT32 *ix, VLINT32 *iy, 
					VLINT32 clip, VLINT32 *zvalue ){

VLINT32 retval = FALSE;
VLFLOAT64 		hdx,hdy,hdz;
VLFLOAT64 		fpt[3],temp[3];
VLINT32		xh,yh;

	if (!set->image) return retval;
/* rendered image space offsets */
	yh = set->imagey/2;
	xh = set->imagex/2;
/* volume space offsets */
	hdx = (VLFLOAT64)(set->d[0])/2.0;
	hdy = (VLFLOAT64)(set->d[1])/2.0;
	hdz = (VLFLOAT64)(set->d[2])/2.0;
	fpt[0] = vx - hdx;
	fpt[1] = vy - hdy;
	fpt[2] = ((VLFLOAT64)(vz - hdz)*(set->squeeze_factor));

/* transform the 3D point to screen space */
	vl_matmult(fpt,temp,set->xform);

/* should translate the points here!!! */
	temp[0] += set->trans[0] + 0.5;
	temp[1] += set->trans[1] + 0.5;
	temp[2] += set->trans[2] + 0.5;

/* put them into the renderer image */
	*ix = floor(temp[0] + ((VLFLOAT64) xh));
	*iy = floor(temp[1] + ((VLFLOAT64) yh));
	if (zvalue) {
		*zvalue = (temp[2]  *65536.0) ;
		*zvalue = *zvalue >> 13;
	}

	if (*ix < 0) return retval;
	if (*ix > set->imagex) return retval;
	if (*iy < 0) return retval;
	if (*iy > set->imagey) return retval;

/* should clip the Z value by the current Z buffer allowing for hidden point
	removals */
	if (clip) {
		VLINT32 x = *ix;
		VLINT32 y = *iy;
		VLUINT32 offset = (y * set->imagex) + x;
		VLINT32 z = set->zbuffer[offset];
		VLINT32 cz = (temp[2] + 3) *65536.0;
		if (z == VL_INFINITY) retval = TRUE;
		else
		if (z < (cz >> 13))  retval = TRUE;
	}
	else retval = TRUE;

/* got valid points */
	return retval;
}

VLINT32 vl_image2volume_( vset *set,
					  VLINT32 ix, VLINT32 iy, 
					  VLINT32 *vx, VLINT32 *vy, VLINT32 *vz){
VLINT32	i,j;
VLFLOAT64 		hdx,hdy,hdz;
VLFLOAT64 		fpt[3],temp[3], xform[3][3];
VLINT32	xh,yh;
VLINT32	screen[3];
VLINT32 retval = FALSE;

	if (!set->image) return retval;

/* rendered image space offsets */
	yh = set->imagey/2;
	xh = set->imagex/2;
/* volume space offsets */
	hdx = (VLFLOAT64)(set->d[0])/2.0;
	hdy = (VLFLOAT64)(set->d[1])/2.0;
	hdz = (VLFLOAT64)(set->d[2])/2.0;
/* get the original point */
	i = ix + set->imagex*iy;
	fpt[0] = (VLFLOAT64)(ix) - xh;
	fpt[1] = (VLFLOAT64)(iy) - yh;
	fpt[2] = (VLFLOAT64)(set->zbuffer[i])/8.0; /* from z buffer */
	fpt[2] -= 0.5;
	/*
	 // compensate for the 0.5 added durring rasterization
	*/

/* should translate the points here!!! */
	fpt[0] = fpt[0] - set->trans[0];
	fpt[1] = fpt[1] - set->trans[1];
	fpt[2] = fpt[2] - set->trans[2];

/* transform the 3D point to volume space via inverse projection matrix */
	for(i=0;i<3;i++) {
		for(j=0;j<3;j++) {
			xform[i][j] = set->xform[i][j];
		}
	}
	invert3x3(xform);
	vl_matmult(fpt,temp,xform);

/* put them into the volume space */
	screen[0] = floor(temp[0] + hdx + 0.5);
	screen[1] = floor(temp[1] + hdy + 0.5);
	screen[2] = floor((temp[2]/(set->squeeze_factor)) + hdz + 0.5);
/* if the point in the volume at all? */
	if (vl_isinvolume_(set,screen)) {
		*vx = screen[0];
		*vy = screen[1];
		*vz = screen[2];
		retval = TRUE;
	}

return retval;
}
