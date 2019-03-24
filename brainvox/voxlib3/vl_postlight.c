#include "voxel.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "vl_util.h"
#include "vl_postlight.h"

/*
typedef struct {
	VLFLOAT64 	zstretch;
	VLFLOAT64 	jitter_frac;
	VLINT32	mult_min;
	VLINT32	mult_max; 
	VLFLOAT64  rl_x;
	VLFLOAT64  rl_y;
	VLFLOAT64  rl_z;
	VLFLOAT64  rl_inten;
	VLFLOAT64  rl_srcinten;
	VLINT32 *l_table;
	VLINT32 hz_near;
	VLINT32 hz_far;

	VL_INT32 *z_slice_mask;
} vl_light_params;
*/

vl_light_params vl_light_globals={2.0,0.0,30,255,0.0,0.0,1.0,64.0,192.0,0L,
	256*8,-256*8,0L};

void getznorm(VLINT32 x, VLINT32 y,VLINT16 *zbuf, VLFLOAT64 *norm,
		VLINT32 dx,VLINT32 dy);
void recolor(VLFLOAT64 *norm,VLFLOAT64 inten,VLINT32 *image,VLINT16 *zbuf,
		VLINT32 *out,VLINT32 dx,VLINT32 dy);
void	vl_build_l_table(vset *set);

void getznorm(VLINT32 x, VLINT32 y,VLINT16 *zbuf, VLFLOAT64 *norm,
		VLINT32 dx,VLINT32 dy)
{
	VLINT32 px1,px2,py1,py2,pt;
	VLFLOAT64 l;

	pt = zbuf[(x)+(y*dx)];
	px1 = zbuf[(x-1)+(y*dx)];
	px2 = zbuf[(x+1)+(y*dx)];

	py1 = zbuf[x+((y-1)*dx)];
	py2 = zbuf[x+((y+1)*dx)];

	if (px1 < VL_OUTTHERE) px1 = pt;
	if (px2 < VL_OUTTHERE) px2 = pt;
	if (py1 < VL_OUTTHERE) py1 = pt;
	if (py2 < VL_OUTTHERE) py2 = pt;

	norm[0] = px1 - px2;
	norm[1] = py1 - py2;
	norm[2] = vl_light_globals.zstretch * 8; /* fixed point zbuffer */

#define FRAC vl_light_globals.jitter_frac

	if (FRAC > 0.0) {
	norm[0] = norm[0] + 
		(((VLFLOAT64)(rand() & 0xff)/255.0) * FRAC) - (FRAC/2);
	norm[1] = norm[1] + 
		(((VLFLOAT64)(rand() & 0xff)/255.0) * FRAC) - (FRAC/2);
	}
	l = norm[0]*norm[0] + norm[1]*norm[1] + norm[2]*norm[2];
	if (l == 0.0) l = 1.0;
	l = sqrt(l);

	norm[0] = norm[0] / l;
	norm[1] = norm[1] / l;
	norm[2] = norm[2] / l;
}

void recolor(VLFLOAT64 *norm,VLFLOAT64 inten,VLINT32 *image,VLINT16 *zbuf,
		VLINT32 *out,VLINT32 dx,VLINT32 dy)
{
	register VLINT32 x,y; 
	VLFLOAT64 znorm[3];
	register VLFLOAT64 place;
	register VLINT32 R,B,G;
	register VLINT32 mult;

	if (inten == 0.0) return;
	for(y=1; y<dy-1; y++) {
		for(x=1; x<dx-1; x++) {
			if (zbuf[x+(y*dx)] < (VL_OUTTHERE))  {
				out[x+(y*dx)] = image[x+(y*dx)];
			} else {
			getznorm(x,y,zbuf,znorm,dx,dy);
			R = (image[x+(y*dx)] & 0x000000ff);
			G = (image[x+(y*dx)] & 0x0000ff00) >> 8;
			B = (image[x+(y*dx)] & 0x00ff0000) >> 16;

			place = (znorm[0] * norm[0]) + (znorm[1] * norm[1])
				 + (znorm[2] * norm[2]);
			place = place * inten;

			mult = (VLINT32)(255.0 * place);
			if (mult < vl_light_globals.mult_min) 
				mult = vl_light_globals.mult_min;
			if (mult >  vl_light_globals.mult_max) 
				mult = vl_light_globals.mult_max;

/* divide and saturate */
			R = (R * mult) >> 8;
			G = (G * mult) >> 8;
			B = (B * mult) >> 8;
			if (R > 255) R = 255;
			if (G > 255) G = 255;
			if (B > 255) B = 255;
			out[x+(y*dx)] = (R | (G << 8) | (B << 16));
			}
		}
	}
}

void vl_set_light_param_(VLINT32 param, VLFLOAT64 *value)
{
	switch (param) {
		case VL_HAZE_NEAR:
			vl_light_globals.hz_near = (*value)*8.0;
			break;
		case VL_HAZE_FAR:
			vl_light_globals.hz_far = (*value)*8.0;
			break;
		case VL_HAZE_RESET:
			vl_light_globals.hz_near = 256*8;
			vl_light_globals.hz_far = -256*8;
			break;
		case VL_JITTER:
			vl_light_globals.jitter_frac = (*value);
			break;
		case VL_MIN_INTEN:
			vl_light_globals.mult_min = (*value);
			break;
		case VL_MAX_INTEN:
			vl_light_globals.mult_max = (*value);
			break;
		case VL_ZSTRETCH:
			vl_light_globals.zstretch = (*value);
			if (vl_light_globals.zstretch == 0.0) {
				vl_light_globals.zstretch = 2.0;
				vl_puts("Error: Zero ZSTRETCH not allowed");
			}
			break;
		case VL_ZSLICE_MASK:
			vl_light_globals.z_slice_mask = (VLINT32 *)value;
			break;
		case VL_RESET_LIGHTS:
			vl_light_globals.jitter_frac = 0.0;
			vl_light_globals.mult_min = 30;
			vl_light_globals.mult_max = 255;
			vl_light_globals.zstretch = 2.0;
			break;
		case VL_ILIGHT_DX:
			vl_light_globals.rl_x = (*value);
			break;
		case VL_ILIGHT_DY:
			vl_light_globals.rl_y = (*value);
			break;
		case VL_ILIGHT_DZ:
			vl_light_globals.rl_z = (*value);
			break;
		case VL_ILIGHT_INTEN:
			vl_light_globals.rl_inten = (*value)*256.0;
			if (vl_light_globals.rl_inten > 256.0) {
				vl_light_globals.rl_inten = 256.0;
			}
			if (vl_light_globals.rl_inten < 0.0) {
				vl_light_globals.rl_inten = 0.0;
			}
			break;
		case VL_ILIGHT_SRCINTEN:
			vl_light_globals.rl_srcinten = (*value)*256.0;
			if (vl_light_globals.rl_srcinten > 768.0) {
				vl_light_globals.rl_srcinten = 768.0;
			}
			if (vl_light_globals.rl_srcinten < 0.0) {
				vl_light_globals.rl_srcinten = 0.0;
			}
			break;
		case VL_ILIGHT_RESET:
			vl_light_globals.rl_x = 0.0;
			vl_light_globals.rl_y = 0.0;
			vl_light_globals.rl_z = 1.0;
			vl_light_globals.rl_inten = 64.0;
			vl_light_globals.rl_srcinten = 192.0;
			break;
	}
	return;
}

void vl_addhaze_(VLINT32 *image,VLINT16 *zbuf,VLINT32 *out,
			VLINT32 dx,VLINT32 dy,VLFLOAT64 *w)
{
	VLINT32 y,x,k;
	VLFLOAT64	mult;
	VLINT32 R,G,B;
	VLFLOAT64	tmp;
/*
	mult = w0 - (w0-w1)*(Zn-Zp)/(Zn-Zf)
	w0 = weight at Zn
	w1 = weight at Zf
	Zn = z-near
	Zf = z-far
	Zp = z-at current pixel 
*/
	tmp = (w[0] - w[1])/(VLFLOAT64)(vl_light_globals.hz_near -
		vl_light_globals.hz_far);
	for(y=0; y<dy; y++) {
		k = (y*dx);
		for(x=0; x<dx; x++) {
			if (zbuf[k] < (VL_OUTTHERE))  {
				out[k] = image[k];
			} else {
				mult = (w[0] - (tmp*
				   (VLFLOAT64)(vl_light_globals.hz_near-zbuf[k])));
				R = (image[k] & 0x000000ff);
				G = (image[k] & 0x0000ff00) >> 8;
				B = (image[k] & 0x00ff0000) >> 16;
/* divide and saturate */
				R = (R * mult);
				G = (G * mult);
				B = (B * mult);
				if (R > 255) R = 255;
				if (G > 255) G = 255;
				if (B > 255) B = 255;
				out[k] = (R | (G << 8) | (B << 16));
			}
			k += 1;
		}
	}
	return;
}

void vl_postlight_(VLINT32 *image,VLINT16 *zbuf,VLINT32 *out,
			VLINT32 imagex,VLINT32 imagey,VLFLOAT64 *norm)
{
/* norm = dx,dy,dz,intens */
	VLFLOAT64 inten,l;
	VLFLOAT64 tnorm[3];

	inten = norm[3];

	l = norm[0]*norm[0] + norm[1]*norm[1] + norm[2]*norm[2];
	if (l == 0.0) {
		vl_puts("Error: Zero length lighting normal");
		return;
	}
	l = sqrt(l);

	tnorm[0] = norm[0] / l;
	tnorm[1] = norm[1] / l;
	tnorm[2] = norm[2] / l;

	recolor(tnorm,inten,image,zbuf,out,imagex,imagey);

	return;
}

void	vl_build_l_table(vset *set)
{
	VLUINT32 i,j;
	VLSCHAR	dx,dy,dz;
	VLFLOAT64		ddx,ddy,ddz;
	VLFLOAT64		d,inten,n,nin[3],nout[3];
	VLFLOAT64		temp[3][3];

/* VALID ? */
	if (vl_light_globals.l_table != 0L) FREE(vl_light_globals.l_table);

/* normalize */
	d = (vl_light_globals.rl_x*vl_light_globals.rl_x);
	d += (vl_light_globals.rl_y*vl_light_globals.rl_y);
	d += (vl_light_globals.rl_z*vl_light_globals.rl_z);
	if (d == 0.0) d = 1.0;
	d = sqrt(d);
	nin[0] = -vl_light_globals.rl_x/d;
	nin[1] = -vl_light_globals.rl_y/d;
	nin[2] = -vl_light_globals.rl_z/d;
/* rotate */
	for(i=0;i<3;i++) for(j=0;j<3;j++) temp[i][j] = (set->xform)[i][j];
	invert3x3(temp);
	vl_matmult(nin,nout,temp);

	vl_light_globals.l_table = (VLINT32 *)MALLOC(sizeof(VLINT32)*32768);
/* for each possible normal */
	for(i=0;i<32768;i++) {
/* get components (5bits each) */
		dx = ((i & 0x00007c00) >> 7);
		dy = ((i & 0x000003e0) >> 2);
		dz = ((i & 0x0000001f) << 3);
		ddx = dx;
		ddy = dy;
		ddz = dz;
/* normalize */
		d = (ddx*ddx) + (ddy*ddy) + (ddz*ddz);
		if (d == 0.0) d = 1.0;
		d = sqrt(d);
/* lighting normal (dot) voxel normal */
		n = ((VLFLOAT64)(dx))/d;
		inten = (n*nout[0]);
		n = ((VLFLOAT64)(dy))/d;
		inten += (n*nout[1]);
		n = ((VLFLOAT64)(dz))/d;
		inten += (n*nout[2]);
		if (inten < 0.0) inten = 0.0;
/* store off a fixed point representation */
		j = vl_light_globals.rl_inten + 
			(inten * (vl_light_globals.rl_srcinten));
		if (j < 0 ) j = 0;
		vl_light_globals.l_table[i] = j;
#ifdef DEBUG
		if ((i & 0x006318) == 0) printf("%d : %d %d %d : %d\n",i,(long)(dx),(long)(dy),(long)(dz),j);
#endif
	}
	return;
}

void vl_get_inten(VLINT32 dx,VLINT32 dy,VLINT32 dz,VLINT32 *rb,VLINT32 *g)
{
	register VLUINT32 i,r,b,it;
/* index is: 0xxx xxyy yyyz zzzz */
/* from: xxxx x000, yyyy y000, zzzz z000 */
	if (vl_light_globals.l_table == 0L) return;
	i = ((VLUINT32)(dx & 0xf8) << 7) | 
		((VLUINT32)(dy & 0xf8) << 2) |
		((VLUINT32)(dz & 0xf8) >> 3);
#ifdef DEBUG
	printf("test: %d %d %d : %x\n",dx,dy,dz,i);
#endif
	it = vl_light_globals.l_table[i];
/* brighten  x = (x*inten) */
	r = ((*rb) & 0x000000ff) * it;
	b = (((*rb) & 0x00ff0000) >> 16) * it;
	r = r >> 8;
	b = b >> 8;
	if (r < 0) r = 0;
	if (b < 0) b = 0;
	if (r > 255) r = 255;
	if (b > 255) b = 255;
	*rb = (r) | (b << 16);
	r = (((*g) & 0x0000ff00) >> 8) * it;
	r = r >> 8;
	if (r < 0) r = 0;
	if (r > 255) r = 255;
	*g = (r << 8);
/*
	*rb = (((*rb) * it) & 0xff00ff00) >> 8;
	*g = (((*g) * it) & 0x00ff0000) >> 8;
*/

	return;
}
