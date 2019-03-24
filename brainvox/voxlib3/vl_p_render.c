#include "voxel.h"
#include "vl_util.h"
#include "vl_polygon.h"
#include "vl_putil.h"
#include "vl_tmasks.h"
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

#define PLOTIT(v2, SA, color, imdx, image, zbuf, extra) \
{ \
	register VLUINT32 RB; \
	register VLUINT32 ptr; \
	register VLUINT32 b2; \
	register VLUINT32 im_val; \
	register VLUINT32 Icolor; \
	VLUINT32 MSA; \
	ptr = ((v2)[0] + ((v2)[1]*(imdx))); \
	if (((zbuf)[ptr] < (v2)[2]) || ((extra) & VL_P_NOZCOMP)) { \
		MSA = 255 - SA; \
		im_val = (image)[ptr];  \
		b2 = im_val & 0x00ff00ff; \
		Icolor = (color) & 0x00ff00ff; \
		RB = ((Icolor*(SA)) + (b2*MSA)) & 0xff00ff00; \
		b2 = im_val & 0x00ff00; \
		Icolor = (color) & 0x00ff00; \
		RB = RB | (((Icolor*(SA)) + (b2*(MSA))) & 0x00ff0000); \
		(image)[ptr] = (RB) >> 8; \
		if ((((extra) & VL_P_NOZWRITE) == 0) && (SA != 0)) \
			 (zbuf)[ptr] = (v2)[2]; \
	} \
}


void vl_interpolate(vl_int_vertex *v1,vl_int_vertex *v2,vl_int_vertex *out,
	VLFLOAT64	per);
void vl_incrementalize(VLINT32 *acc,VLINT32 *inc,VLINT32 n,VLFLOAT64 dist);
void vl_increment(VLINT32 *acc,VLINT32 *inc,VLINT32 n);
/* point sorter */
void vl_cmp_swap(vl_int_vertex *v1,vl_int_vertex *v2);
/* horizontal line drawer */
void vl_p_rendhline(VLINT32 style,VLINT32 opts,vl_int_vertex *v1,
	vl_int_vertex *v2,vset *set);

/* incremental utilities */

void vl_incrementalize(VLINT32 *acc,VLINT32 *inc,VLINT32 n,VLFLOAT64 dist)
{
	VLINT32 i;
	VLFLOAT64	dd;

	for(i=0;i<n;i++) {
		dd = inc[i] - acc[i];
		dd = dd / dist;
		inc[i] = (VLINT32)(dd);
	}
}

void vl_increment(VLINT32 *acc,VLINT32 *inc,VLINT32 n)
{
	VLINT32 i;
	for(i=0;i<n;i++) acc[i] += inc[i];
}

void	vl_cmp_swap(vl_int_vertex *v1,vl_int_vertex *v2)
{
	vl_int_vertex	tempv;

/* if Y1 > Y2 or (Y1=Y2 and X1>X2) then swap */
	if ((v1->screen[1] > v2->screen[1]) || 
		((v1->screen[1] == v2->screen[1]) 
		&& (v1->screen[0] > v2->screen[0]))) {
		tempv = (*v1);
		*v1 = (*v2);
		*v2 = tempv;
	}
}

void vl_interpolate(vl_int_vertex *v1,vl_int_vertex *v2,vl_int_vertex *out,
	VLFLOAT64	per)
{
	register VLINT32 i;
	
	for(i=0;i<4;i++) {
		out->screen[i] = v1->screen[i] + 
			(per * (v2->screen[i] - v1->screen[i]));	
		out->loc[i] = v1->loc[i] + 
			(per * (v2->loc[i] - v1->loc[i]));	
		out->color[i] = v1->color[i] + 
			(per * (v2->color[i] - v1->color[i]));	
	}
	return;
}

/* rendering routines for the embedded geometry options */

void vl_p_rendpoly(VLINT32 style,VLINT32 opts,vl_int_vertex *vec,vset *set)
{
	VLFLOAT64	d1,d2,dx,dy,inc1,inc2;
	vl_int_vertex	v1,v2;
	VLINT32 i,y,yt,ym,yb;

/* SORT the points first */
	vl_cmp_swap(&(vec[0]),&(vec[1]));
	vl_cmp_swap(&(vec[1]),&(vec[2]));
	vl_cmp_swap(&(vec[0]),&(vec[1]));
#ifdef DEBUG
	for(i=0;i<3;i++)
	printf("Incoming: %lf %lf %lf :  %lf %lf %lf %lf\n",
		vec[i].screen[0],vec[i].screen[1],vec[i].screen[2],
	vec[i].color[0],vec[i].color[1],vec[i].color[2],vec[i].color[3]);
#endif

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
		vl_p_rendhline(style,opts,&(v1),&(v2),set);
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
		vl_p_rendhline(style,opts,&(v1),&(v2),set);
		inc1 = inc1 + 1.0;
		inc2 = inc2 + 1.0;
	}
	return;
}

void vl_p_rendhline(VLINT32 style,VLINT32 opts,vl_int_vertex *vi1,
	vl_int_vertex *vi2,vset *set)
{
	VLUINT32 S,n,c, topts;
	VLINT32	acc[MAX_ENT],inc[MAX_ENT],j,v[3],vt[3];
	VLUINT32	*img;
	vl_int_vertex 	*v1,*v2;
	MaskRec		*mask;

	VLINT32 	x1,x2;
	VLINT32	d,i;
	VLUINT32 defC, defS;
	VLUINT32 tdxdy;

	mask = 0L;
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
#ifdef DEBUG
	printf("Plotting %d from: %lf %lf %lf : %lf %lf %lf\n",style,
		v1->screen[0],v1->screen[1],v1->screen[2],
		v1->color[0],v1->color[1],v1->color[2]);
	printf("Plotting to: %lf %lf %lf : %lf %lf %lf\n",
		v2->screen[0],v2->screen[1],v2->screen[2],
		v2->color[0],v2->color[1],v2->color[2]);
#endif
	
	img = (VLUINT32 *)set->image;

	n = 0;
	acc[n] = _LTOF(x1); inc[n] = _LTOF(x2); n++;
	acc[n] = _DTOF(v1->screen[1]); inc[n] = _DTOF(v2->screen[1]); n++;
	acc[n] = _DTOF(v1->screen[2]); inc[n] = _DTOF(v2->screen[2]); n++;
	if (style == VL_P_TEXTURED) {
	acc[n] = _DTOF(v1->loc[0]); inc[n] = _DTOF(v2->loc[0]); n++;
	acc[n] = _DTOF(v1->loc[1]); inc[n] = _DTOF(v2->loc[1]); n++;
	acc[n] = _DTOF(v1->loc[2]/set->squeeze_factor);
	inc[n] = _DTOF(v2->loc[2]/set->squeeze_factor); n++;
	}
	else
	if (( style == VL_P_TEXTURE_2D ) ||
	    ( style == VL_P_TEXTURE_3D )){
	acc[n] = _DTOF(v1->loc[0]); inc[n] = _DTOF(v2->loc[0]); n++;
	acc[n] = _DTOF(v1->loc[1]); inc[n] = _DTOF(v2->loc[1]); n++;
	acc[n] = _DTOF(v1->loc[2]);	inc[n] = _DTOF(v2->loc[2]); n++;
	tdxdy = vl_polygon_globals.texture.d[0] * vl_polygon_globals.texture.d[1];
	acc[n]=_DTOF(v1->color[0]*255); inc[n]=_DTOF(v2->color[0]*255); n++;
	acc[n]=_DTOF(v1->color[1]*255); inc[n]=_DTOF(v2->color[1]*255); n++;
	acc[n]=_DTOF(v1->color[2]*255); inc[n]=_DTOF(v2->color[2]*255); n++;
	} 
	else {
	acc[n]=_DTOF(v1->color[0]*255); inc[n]=_DTOF(v2->color[0]*255); n++;
	acc[n]=_DTOF(v1->color[1]*255); inc[n]=_DTOF(v2->color[1]*255); n++;
	acc[n]=_DTOF(v1->color[2]*255); inc[n]=_DTOF(v2->color[2]*255); n++;
	}
/* flat style has no COLOR interpolations */
	if (style == VL_P_FLAT) n = n - 3;

/* create the deltas */
	vl_incrementalize(acc,inc,n,(VLFLOAT64)(d));

/* offset by 1/2 pixel
	for(j=0;j<3;j++) acc[j] += 0x00008000;
*/

	defS = S = (VLINT32)(v1->color[3] * 255.0);
	defC = 	(VLUINT32)(v1->color[0] * 255.0);
	defC |= ((VLUINT32)(v1->color[1] * 255.0)) << 8;
	defC |= ((VLUINT32)(v1->color[1] * 255.0)) << 16;

	if (vl_polygon_globals.screendoor) vl_set_screen_mask(&S,&mask);

	v[0] = _FTOL(acc[0]) + (set->imagex/2);
	v[1] = _FTOL(acc[1]) + (set->imagey/2);
	if (style == VL_P_TEXTURED) {
		for(i=0;i<d;i=i+1) {
			vt[0] = _FTOL(acc[3]) + (set->d[0]/2);
			vt[1] = _FTOL(acc[4]) + (set->d[1]/2);
			vt[2] = _FTOL(acc[5]) + (set->d[2]/2);
			if ((vt[0] >= 0) && 
				(vt[1] >= 0) && 
				(vt[2] >=0) &&
			    (vt[0] < set->d[0]) && 
			    (vt[1] < set->d[1]) && 
			    (vt[2] < set->d[2])) {
					j = (vt[0]) + (vt[1]*(set->d[0])) + (vt[2]*(set->d[0]*set->d[1]));
					c = set->rlut[set->data[j]];
					S = (c & 0xff000000) >> 24;
					if (vl_polygon_globals.screendoor) vl_set_screen_mask(&S,&mask);
			} 
			else {
				c = set->rlut[0];
				S = defS;
				if (vl_polygon_globals.screendoor) vl_set_screen_mask(&S,&mask);
			}
			v[2] = _FTOLx8(acc[2]);
			if (((v[0]) >= 0 ) && 
				((v[0]) < set->imagex) &&
				((v[1]) >= 0) && 
				((v[1]) < set->imagey)) {
					if (mask) {
						if (mask->v[v[0] % 4][v[1] % 4]) {
							PLOTIT(v,S,c,set->imagex,img,set->zbuffer,opts);
						}
					} 
					else {
						PLOTIT(v,S,c,set->imagex,img,set->zbuffer,opts);
					}
			}
			vl_increment(acc,inc,n);
			v[0] = v[0] + 1;
		}
	} 
	else
	if ((style == VL_P_TEXTURE_2D) && 
		vl_polygon_globals.texture.texture_ptr) {
		for(i=0;i<d;i=i+1) {
			v[2]  = _FTOLx8(acc[2]);
			vt[0] = _FTOL(acc[3]);
			vt[1] = _FTOL(acc[4]);
			if ((vt[0] >= 0) && 
				(vt[1] >= 0) && 
			    (vt[0] < vl_polygon_globals.texture.d[0]) && 
			    (vt[1] < vl_polygon_globals.texture.d[1])) {
					j = vt[0]+ ( vt[1]* vl_polygon_globals.texture.d[0]);
					c = vl_polygon_globals.texture.texture_ptr[j];
					c = vl_polygon_globals.texture.lut[c];
					S = (c & 0xff000000) >> 24;
					if ( c ) topts = VL_P_NOZWRITE;
					else topts = opts;
			} 
			
			defC = _FTOL(acc[6]);
			defC |= (_FTOL(acc[7])) << 8;
			defC |= (_FTOL(acc[8])) << 16;
			if (vl_polygon_globals.screendoor) vl_set_screen_mask(&defS,&mask);
			
			if (((v[0]) >= 0 ) && 
				((v[0]) < set->imagex) &&
				((v[1]) >= 0) && 
				((v[1]) < set->imagey)) {
				if (mask) {
					if (mask->v[v[0] % 4][v[1] % 4]) {
						PLOTIT(v,defS,defC,set->imagex,img,set->zbuffer, topts);
						PLOTIT(v,S,c,set->imagex,img,set->zbuffer,opts);
					}
				} 
				else {
					PLOTIT(v,defS,defC,set->imagex,img,set->zbuffer, topts );
					PLOTIT(v,S,c,set->imagex,img,set->zbuffer,opts);
				}
			}
			vl_increment(acc,inc,n);
			v[0] = v[0] + 1;
		}
	} 
	else
	if ((style == VL_P_TEXTURE_3D) && 
		vl_polygon_globals.texture.texture_ptr) {
		for(i=0;i<d;i=i+1) {
			v[2]  = _FTOLx8(acc[2]);
			vt[0] = _FTOL(acc[3]);
			vt[1] = _FTOL(acc[4]);
			vt[2] = _FTOL(acc[5]);
			if ((vt[0] >= 0) && 
				(vt[1] >= 0) && 
				(vt[2] >= 0) &&
			    (vt[0] < vl_polygon_globals.texture.d[0]) && 
			    (vt[1] < vl_polygon_globals.texture.d[1]) &&
			    (vt[2] < vl_polygon_globals.texture.d[2])) {
					j = vt[0] + ( vt[1]* vl_polygon_globals.texture.d[0]);
					j +=  vt[2] * tdxdy;
					c = vl_polygon_globals.texture.texture_ptr[j];
					c = vl_polygon_globals.texture.lut[c];
					S = (c & 0xff000000) >> 24;
					if ( c ) topts = VL_P_NOZWRITE;
					else topts = opts;
			} 
			
			defC = _FTOL(acc[6]);
			defC |= (_FTOL(acc[7])) << 8;
			defC |= (_FTOL(acc[8])) << 16;
			if (vl_polygon_globals.screendoor) vl_set_screen_mask(&defS,&mask);
			
			if (((v[0]) >= 0 ) && 
				((v[0]) < set->imagex) &&
				((v[1]) >= 0) && 
				((v[1]) < set->imagey)) {
				if (mask) {
					if (mask->v[v[0] % 4][v[1] % 4]) {
						PLOTIT(v,defS,defC,set->imagex,img,set->zbuffer, topts);
						PLOTIT(v,S,c,set->imagex,img,set->zbuffer,opts);
					}
				} 
				else {
					PLOTIT(v,defS,defC,set->imagex,img,set->zbuffer, topts);
						PLOTIT(v,S,c,set->imagex,img,set->zbuffer,opts);
				}
			}
			vl_increment(acc,inc,n);
			v[0] = v[0] + 1;
		}
	}
	else {
		for(i=0;i<d;i=i+1) {
			v[2] = _FTOLx8(acc[2]);
			c = _FTOL(acc[3]);
			c |= (_FTOL(acc[4])) << 8;
			c |= (_FTOL(acc[5])) << 16;
			if (((v[0]) >= 0 ) && 
				((v[0]) < set->imagex) &&
				((v[1]) >= 0) && 
				((v[1]) < set->imagey)) {
			if (mask) {
			if (mask->v[v[0] % 4][v[1] % 4]) {
			PLOTIT(v,S,c,set->imagex,img,set->zbuffer,opts);
			}
			} else {
			PLOTIT(v,S,c,set->imagex,img,set->zbuffer,opts);
			}
			}

			vl_increment(acc,inc,n);
			v[0] = v[0] + 1;
		}
	}
#ifdef DEBUG
	PLOTIT(v,S,c,set->imagex,img,set->zbuffer,opts);
	j = (v[0] + (v[1]*(set->imagex))); 
	printf("Lastcolor: %x %x %ld\n",img[j],c,S);
#endif
	return;
}


void vl_p_rendline(VLINT32 style,VLINT32 opts,vl_int_vertex *v1,
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

#ifdef DEBUG
	printf("Plotting %d from: %lf %lf %lf : %lf %lf %lf\n",style,
		v1->screen[0],v1->screen[1],v1->screen[2],
		v1->color[0],v1->color[1],v1->color[2]);
	printf("Plotting to: %lf %lf %lf : %lf %lf %lf\n",
		v2->screen[0],v2->screen[1],v2->screen[2],
		v2->color[0],v2->color[1],v2->color[2]);
#endif
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
	img = (VLUINT32 *)set->image;

	n = 0;
	acc[n] = _DTOF(v1->screen[0]); inc[n] = _DTOF(v2->screen[0]); n++;
	acc[n] = _DTOF(v1->screen[1]); inc[n] = _DTOF(v2->screen[1]); n++;
	acc[n] = _DTOF(v1->screen[2]); inc[n] = _DTOF(v2->screen[2]); n++;
	if (style == VL_P_TEXTURED) {
	acc[n] = _DTOF(v1->loc[0]); inc[n] = _DTOF(v2->loc[0]); n++;
	acc[n] = _DTOF(v1->loc[1]); inc[n] = _DTOF(v2->loc[1]); n++;
	acc[n] = _DTOF(v1->loc[2]/set->squeeze_factor);
	inc[n] = _DTOF(v2->loc[2]/set->squeeze_factor); n++;
	} else {
	acc[n]=_DTOF(v1->color[0]*255); inc[n]=_DTOF(v2->color[0]*255); n++;
	acc[n]=_DTOF(v1->color[1]*255); inc[n]=_DTOF(v2->color[1]*255); n++;
	acc[n]=_DTOF(v1->color[2]*255); inc[n]=_DTOF(v2->color[2]*255); n++;
	}
/* flat style has no COLOR interpolations */
	if (style == VL_P_FLAT) n = n - 3;
	
/* create the deltas */
	if (dd == 0.0) dd = 1.0;
	vl_incrementalize(acc,inc,n,dd);

	S = (VLINT32)(v1->color[3] * 255.0);
	if (vl_polygon_globals.screendoor) vl_set_screen_mask(&S,&mask);
	
	if (style == VL_P_TEXTURED) {
		if (ax>ay) {
/* textured X major */
			d = ay-(ax>>1);
			for(;;) {
				v[0] = _FTOL(acc[3]) + (set->d[0]/2);
				v[1] = _FTOL(acc[4]) + (set->d[1]/2);
				v[2] = _FTOL(acc[5]) + (set->d[2]/2);
				if ((v[0] >= 0) && (v[1] >= 0) && (v[2] >=0) &&
				   (v[0] < set->d[0]) && (v[1] < set->d[1])
				   && (v[2] < set->d[2])) {
					j = (v[0]) + (v[1]*(set->d[0])) + 
					(v[2]*(set->d[0]*set->d[1]));
					c = set->rlut[set->data[j]];
					S = (c & 0xff000000) >> 24;
	if (vl_polygon_globals.screendoor) vl_set_screen_mask(&S,&mask);
				} else {
					c = set->backpack;
					S = (VLINT32)(v1->color[3] * 255.0);
	if (vl_polygon_globals.screendoor) vl_set_screen_mask(&S,&mask);
				}
				v[0] = x + (set->imagex/2);
				v[1] = y + (set->imagey/2);
				v[2] = _FTOLx8(acc[2]);
				if (((v[0]) >= 0 ) && 
					((v[0]) < set->imagex) &&
					((v[1]) >= 0) && 
					((v[1]) < set->imagey)) {

			if (mask) {
			if (mask->v[v[0] % 4][v[1] % 4]) {
			PLOTIT(v,S,c,set->imagex,img,set->zbuffer,opts);
			}
			} else {
			PLOTIT(v,S,c,set->imagex,img,set->zbuffer,opts);
			}

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
/* textured Y major */
			d = ax-(ay>>1);
			for(;;) {
				v[0] = _FTOL(acc[3]) + (set->d[0]/2);
				v[1] = _FTOL(acc[4]) + (set->d[1]/2);
				v[2] = _FTOL(acc[5]) + (set->d[2]/2);
				if ((v[0] >= 0) && (v[1] >= 0) && (v[2] >=0) &&
				   (v[0] < set->d[0]) && (v[1] < set->d[1])
				   && (v[2] < set->d[2])) {
					j = (v[0]) + (v[1]*(set->d[0])) + 
					(v[2]*(set->d[0]*set->d[1]));
					c = set->rlut[set->data[j]];
					S = (c & 0xff000000) >> 24;
	if (vl_polygon_globals.screendoor) vl_set_screen_mask(&S,&mask);
				} else {
					c = set->backpack;
					S = (VLINT32)(v1->color[3] * 255.0);
	if (vl_polygon_globals.screendoor) vl_set_screen_mask(&S,&mask);
				}
				v[0] = x + (set->imagex/2);
				v[1] = y + (set->imagey/2);
				v[2] = _FTOLx8(acc[2]);
				if (((v[0]) >= 0 ) && 
					((v[0]) < set->imagex) &&
					((v[1]) >= 0) && 
					((v[1]) < set->imagey)) {

			if (mask) {
			if (mask->v[v[0] % 4][v[1] % 4]) {
			PLOTIT(v,S,c,set->imagex,img,set->zbuffer,opts);
			}
			} else {
			PLOTIT(v,S,c,set->imagex,img,set->zbuffer,opts);
			}

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
	} else {
		if (ax>ay) {
/* normal X major */
			d = ay-(ax>>1);
			for(;;) {
				v[0] = x + (set->imagex/2);
				v[1] = y + (set->imagey/2);
				v[2] = _FTOLx8(acc[2]);
				c = _FTOL(acc[3]);
				c |= (_FTOL(acc[4])) << 8;
				c |= (_FTOL(acc[5])) << 16;
				if (((v[0]) >= 0 ) && 
					((v[0]) < set->imagex) &&
					((v[1]) >= 0) && 
					((v[1]) < set->imagey)) {

			if (mask) {
			if (mask->v[v[0] % 4][v[1] % 4]) {
			PLOTIT(v,S,c,set->imagex,img,set->zbuffer,opts);
			}
			} else {
			PLOTIT(v,S,c,set->imagex,img,set->zbuffer,opts);
			}

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
				c = _FTOL(acc[3]);
				c |= (_FTOL(acc[4])) << 8;
				c |= (_FTOL(acc[5])) << 16;
				if (((v[0]) >= 0 ) && 
					((v[0]) < set->imagex) &&
					((v[1]) >= 0) && 
					((v[1]) < set->imagey)) {

			if (mask) {
			if (mask->v[v[0] % 4][v[1] % 4]) {
			PLOTIT(v,S,c,set->imagex,img,set->zbuffer,opts);
			}
			} else {
			PLOTIT(v,S,c,set->imagex,img,set->zbuffer,opts);
			}

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
}


void vl_set_screen_mask(VLUINT32 *A,MaskRec **mask)
{
	VLINT32	i,j;

	*mask = 0;
	if (*A >= 254) {
		i = 10;
	} if (*A <= 1) {
		i = 0;
	} else {
		i = (VLFLOAT32)(*A)/25.5;
	}
/* return no masking */
	if ((i > 9) || (i < 0)) return;

/* change the alpha value */
	*A = 255;
/* return a mask */
	*mask = &(vl_p_screendoor[i]);

#ifdef DEBUG
	printf("mask %ld\n",i);
	for(j=0;j<4;j++) {
		printf(" %ld %ld %ld %ld\n",
			vl_p_screendoor[i].v[j][0],
			vl_p_screendoor[i].v[j][1],
			vl_p_screendoor[i].v[j][2],
			vl_p_screendoor[i].v[j][3]);
	}
#endif

	return;
}
