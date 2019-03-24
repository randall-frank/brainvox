#include "voxel.h"
#include "vl_util.h"
#include "vl_polygon.h"
#include "vl_putil.h"
#include <stdio.h>
#include <math.h>

#define MAX_ENT 10

/* convert a VLFLOAT64 to a fixed point VLINT32 */
#define _DTOF(d) ((VLINT32)((d)*65536.0))
/* convert a VLINT32 to a fixed point VLINT32 */
#define _LTOF(d) ((d)*65536)

/* convert a fixed point to a VLINT32 */
#define _FTOL(d) ((d)>>16)
/* convert a fixed point to a VLINT32*8 (for 3 bits of subpixel resolution) */
#define _FTOLx8(d) ((d)>>13)

#define ABS(a)	(((a)<0) ? -(a) : (a))
#define SGN(a)	(((a)<0) ? -1 : (a)>0 ? 1 : 0)

#define PICKIT(v2, imdx, image, zbuf) \
{ \
	register VLUINT32 ptr; \
	ptr = ((v2)[0] + ((v2)[1]*(imdx))); \
	if ((zbuf)[ptr] <= (v2)[2]) { \
		(image)[ptr] = vl_polygon_globals.object_id; \
	} \
}


void vl_interpolate(vl_int_vertex *v1,vl_int_vertex *v2,vl_int_vertex *out,
	VLFLOAT64	per);
void vl_incrementalize(VLINT32 *acc,VLINT32 *inc,VLINT32 n,VLFLOAT64 dist);
void vl_increment(VLINT32 *acc,VLINT32 *inc,VLINT32 n);
/* point sorter */
void vl_cmp_swap(vl_int_vertex *v1,vl_int_vertex *v2);
void vl_p_pickline(VLINT32 style,VLINT32 opts,vl_int_vertex *v1,
	vl_int_vertex *v2,vset *set);

void vl_p_rendpickline(VLINT32 style,VLINT32 opts,vl_int_vertex *vi1,
	vl_int_vertex *vi2,vset *set);


/* rendering routines for the embedded geometry options */

void vl_p_rendpickbuffer(VLINT32 style,VLINT32 opts,vl_int_vertex *vec,vset *set)
{
	VLFLOAT64	d1,d2,dx,dy,inc1,inc2;
	vl_int_vertex	v1,v2;
	VLINT32 i,y,yt,ym,yb;

/* SORT the points first */
	vl_cmp_swap(&(vec[0]),&(vec[1]));
	vl_cmp_swap(&(vec[1]),&(vec[2]));
	vl_cmp_swap(&(vec[0]),&(vec[1]));


/* scan from Ay to By plotting pixels between AB and AC */
	yt = vec[0].screen[1];
	ym = vec[1].screen[1];
	yb = vec[2].screen[1];
	d1 = ABS(vec[0].screen[1] - vec[1].screen[1]);
	d2 = ABS(vec[0].screen[1] - vec[2].screen[1]);
	inc1 = 0.0;
	inc2 = 0.0;
	for(y=yt;y<ym;y++) {
		vl_interpolate(&(vec[0]),&(vec[1]),&v1,(inc1/d1));
		vl_interpolate(&(vec[0]),&(vec[2]),&v2,(inc2/d2));
		v1.screen[1] = y;
		v2.screen[1] = y;
		vl_p_rendpickline(style,opts,&(v1),&(v2),set);
		inc1 = inc1 + 1.0;
		inc2 = inc2 + 1.0;
	}
/* scan from By to Cy plotting pixels between BC and AC */
	d1 = ABS(vec[1].screen[1] - vec[2].screen[1]);
	inc1 = 0.0;
	for(y=ym;y<yb;y++) {
		vl_interpolate(&(vec[1]),&(vec[2]),&v1,(inc1/d1));
		vl_interpolate(&(vec[0]),&(vec[2]),&v2,(inc2/d2));
		v1.screen[1] = y;
		v2.screen[1] = y;
		vl_p_rendpickline(style,opts,&(v1),&(v2),set);
		inc1 = inc1 + 1.0;
		inc2 = inc2 + 1.0;
	}
	return;
}

void vl_p_rendpickline(VLINT32 style,VLINT32 opts,vl_int_vertex *vi1,
	vl_int_vertex *vi2,vset *set)
{
	VLUINT32 S,n,c;
	VLINT32	acc[MAX_ENT],inc[MAX_ENT],j,v[3],vt[3];
	VLUINT32	*img;
	vl_int_vertex 	*v1,*v2;
	MaskRec		*mask;

	VLINT32 	x1,x2;
	VLINT32	d,i;
	VLUINT32 defC, defS;
	VLUINT32 tdxdy;

	mask = 0L;
	S = (VLINT32)(vi1->color[3] * 255.0);
	x1 = vi1->screen[0] + 0.45;
	x2 = vi2->screen[0] + 0.45;
	d = (x2 - x1);
	if (d < 0.0) {
		v1 = vi2;
		v2 = vi1;
		d = -d;
		i = x1;
		x1 = x2;
		x2 = i;
	} else {
		v1 = vi1;
		v2 = vi2;
	}
	if (d < 1) return;

	
	img = set->pickbuffer;

	n = 0;
	acc[n] = _LTOF(x1); inc[n] = _LTOF(x2); n++;
	acc[n] = _DTOF(v1->screen[1]); inc[n] = _DTOF(v2->screen[1]); n++;
	acc[n] = _DTOF(v1->screen[2]); inc[n] = _DTOF(v2->screen[2]); n++;
	

/* create the deltas */
	vl_incrementalize(acc,inc,n,(VLFLOAT64)(d));

/* offset by 1/2 pixel
	for(j=0;j<3;j++) acc[j] += 0x00008000;
*/

	if (vl_polygon_globals.screendoor) vl_set_screen_mask(&S,&mask);

	v[0] = _FTOL(acc[0]) + (set->imagex/2);
	v[1] = _FTOL(acc[1]) + (set->imagey/2);
	
		for(i=0;i<d;i=i+1) {
			v[2] = _FTOLx8(acc[2]);
			
			if (((v[0]) >= 0 ) && 
				((v[0]) < set->imagex) &&
				((v[1]) >= 0) && 
				((v[1]) < set->imagey)) {
			
				PICKIT(v,set->imagex,img,set->zbuffer);
			}

			vl_increment(acc,inc,n);
			v[0] = v[0] + 1;
		}
	return;
}


void vl_p_pickline(VLINT32 style,VLINT32 opts,vl_int_vertex *v1,
	vl_int_vertex *v2,vset *set)
{
	VLUINT32 S,n,c;
	VLINT32 x2,y2,sx,sy,dx,dy;
	VLINT32 d,x,y,ax,ay;
	VLFLOAT64	 dd;	
	VLINT32 acc[MAX_ENT],inc[MAX_ENT],j,v[3];
	VLUINT32	*img;
	MaskRec		*mask;

	mask = 0L;


	x = v1->screen[0] + 0.45;
	y = v1->screen[1] + 0.45;
	x2 = v2->screen[0] + 0.45;
	y2 = v2->screen[1] + 0.45;
	dx = (x2 - x);
	dy = (y2 - y);
	ax = ABS(dx) << 1;
	ay = ABS(dy) << 1;
	sx = SGN(dx);
	sy = SGN(dy);
	if (ax>ay) {
		dd = ABS(dx);
	} else {
		dd = ABS(dy);
	}	
	img = set->pickbuffer;

	n = 0;
	acc[n] = _DTOF(v1->screen[0]); inc[n] = _DTOF(v2->screen[0]); n++;
	acc[n] = _DTOF(v1->screen[1]); inc[n] = _DTOF(v2->screen[1]); n++;
	acc[n] = _DTOF(v1->screen[2]); inc[n] = _DTOF(v2->screen[2]); n++;
	
	
/* create the deltas */
	if (dd == 0.0) dd = 1.0;
	vl_incrementalize(acc,inc,n,dd);
 
	
	

		if (ax>ay) {
/* normal X major */
			d = ay-(ax>>1);
			for(;;) {
				v[0] = x + (set->imagex/2);
				v[1] = y + (set->imagey/2);
				v[2] = _FTOLx8(acc[2]);
				if (((v[0]) >= 0 ) && 
					((v[0]) < set->imagex) &&
					((v[1]) >= 0) && 
					((v[1]) < set->imagey)) {

					PICKIT(v,set->imagex,img,set->zbuffer);
				}

				if (x == x2) return;
				if (d>=0) {
					y += sy;
					d -= ax;
				}
				vl_increment(acc,inc,n);
				x += sx;
				d += ay;
			}
		} else {
/* normal Y major */
			d = ax-(ay>>1);
			for(;;) {
				v[0] = x + (set->imagex/2);
				v[1] = y + (set->imagey/2);
				v[2] = _FTOLx8(acc[2]);
				if (((v[0]) >= 0 ) && 
					((v[0]) < set->imagex) &&
					((v[1]) >= 0) && 
					((v[1]) < set->imagey)) {

					PICKIT(v,set->imagex,img,set->zbuffer);
				}

				if (y == y2) return;
				if (d>=0) {
					x += sx;
					d -= ay;
				}
				vl_increment(acc,inc,n);
				y += sy;
				d += ax;
			}
	}
}


