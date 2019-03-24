
#include "voxel.h"
#include "vl_util.h"
#ifdef GLCODE
#include <gl.h>
#endif
#include <stdio.h>

void vl_drawcube_(vset *set)
{
	double	vd1[3],vd2[3],vd3[3],vd4[3],vd5[3],vd6[3],vd7[3],vd8[3];
	long int	v1[3],v2[3],v3[3],v4[3],v5[3],v6[3],v7[3],v8[3];
	long int	i;

/* get the vectors */

	v1[0] = (set->start[0]);
	v1[1] = (set->start[1]);
	v1[2] = (set->start[2]);

	v2[0] = (set->start[0]);
	v2[1] = (set->end[1]);
	v2[2] = (set->start[2]);

	v3[0] = (set->end[0]);
	v3[1] = (set->end[1]);
	v3[2] = (set->start[2]);

	v4[0] = (set->end[0]);
	v4[1] = (set->start[1]);
	v4[2] = (set->start[2]);

	v5[0] = (set->start[0]);
	v5[1] = (set->start[1]);
	v5[2] = (set->end[2]);

	v6[0] = (set->start[0]);
	v6[1] = (set->end[1]);
	v6[2] = (set->end[2]);

	v7[0] = (set->end[0]);
	v7[1] = (set->end[1]);
	v7[2] = (set->end[2]);

	v8[0] = (set->end[0]);
	v8[1] = (set->start[1]);
	v8[2] = (set->end[2]);

/* xform them */
	(void) vl_dopoint(v1,vd1,set);
	(void) vl_dopoint(v2,vd2,set);

	(void) vl_dopoint(v3,vd3,set);
	(void) vl_dopoint(v4,vd4,set);

	(void) vl_dopoint(v5,vd5,set);
	(void) vl_dopoint(v6,vd6,set);

	(void) vl_dopoint(v7,vd7,set);
	(void) vl_dopoint(v8,vd8,set);

#ifdef GLCODE
/* draw them */
/* bottom box */
	(void) bgnline();
	(void) v3d(vd1);
	(void) v3d(vd2);
	(void) v3d(vd3);
	(void) v3d(vd4);
	(void) v3d(vd1);
	(void) endline();

/* top box */
	(void) bgnline();
	(void) v3d(vd5);
	(void) v3d(vd6);
	(void) v3d(vd7);
	(void) v3d(vd8);
	(void) v3d(vd5);
	(void) endline();

/* connecting lines */
	(void) bgnline();
	(void) v3d(vd1);
	(void) v3d(vd5);
	(void) endline();

	(void) bgnline();
	(void) v3d(vd2);
	(void) v3d(vd6);
	(void) endline();

	(void) bgnline();
	(void) v3d(vd3);
	(void) v3d(vd7);
	(void) endline();

	(void) bgnline();
	(void) v3d(vd4);
	(void) v3d(vd8);
	(void) endline();

/* thick pimary lines */

	(void) linewidth((short)2);

	(void) bgnline();
	(void) v3d(vd1);
	(void) v3d(vd2);
	(void) endline();

	(void) bgnline();
	(void) v3d(vd1);
	(void) v3d(vd4);
	(void) endline();

	(void) bgnline();
	(void) v3d(vd1);
	(void) v3d(vd5);
	(void) endline();

	(void) linewidth((short)1);

	(void) cmov2((Coord)vd2[0],(Coord)vd2[1]);
	(void) charstr("Y\0");
	(void) cmov2((Coord)vd4[0],(Coord)vd4[1]);
	(void) charstr("X\0");
	(void) cmov2((Coord)vd5[0],(Coord)vd5[1]);
	(void) charstr("Z\0");

/* now the cutting plane if any */
	if (set->plane == 0) return;
/* compute the points */
	for(i=0; i<3; i++ ) {
		v1[i] = set->planept[i];
		v2[i] = v1[i] + ((set->planevec[i])*10);
	}
/* xform the points 
	(void) vl_dopoint(v1,vd1,set);
	(void) vl_dopoint(v2,vd2,set); */
/* draw the line */
	(void) bgnline();
	(void) v3i(v1);
	(void) v3i(v2);
	(void) endline();
#endif
}

void vl_dopoint(long int *v, double *vd, vset *set)
{
	double	tx,ty,tz;
	long int	tx2,ty2,tz2;
	long int	v2[3];

/* calculate the offsets */

        tx = -(set->d[0])/2.0;
        ty = -(set->d[1])/2.0;
        tz = -(set->d[2])/2.0;
        tx2 = (set->trans[0]);
        ty2 = (set->trans[1]);
        tz2 = (set->trans[2]);

/* transform the point to CMASS */

	v2[0] = v[0] + tx;
	v2[1] = v[1] + ty;
	v2[2] = (v[2] + tz)*(set->squeeze_factor);

/* Mult through rotations */

        (void) vl_imatmult(v2,vd,(set->xform));

/* add the translations */

        vd[0] = vd[0] + tx2;
        vd[1] = vd[1] + ty2;
        vd[2] = vd[2] + tz2;

}
