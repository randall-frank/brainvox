#include "voxel.h"
#include "vl_util.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include "colorin.h"
#include "vl_voxel_renderers.h"
#include "vl_postlight.h"

#include "vl_parallel.h"



void vl_m_pirend16(par_quant *par)
{
/* local copy of par_quant struct */
        vset *set = par->set;
        VLINT32 *st = par->st;
        VLINT32 *en = par->en;
        VLINT32 *le = par->le;
        VLINT32 *ri = par->ri;
        VLINT32 *step = par->step;
        VLINT32 *mult = par->mult;
        VLINT32 *order = par->order;
        VLINT32 inter = par->inter;
	VLINT32 flag = (par->flag & (~VL_RENDER16BIT));
	VLUINT32 *image = par->image;
	VLINT16 *zbuf = par->zbuf;
	VLINT32 *pbuffer = par->pbuffer;

/* real stuff */
	VLUCHAR *data1,*data2;
	VLINT32 d1,d2;
	VLINT32 a,b,c;
	VLINT32 v[3],vx[3];
	VLFLOAT64 v2[3];
	VLFLOAT64 vd[3];
	VLFLOAT64 tx,ty,tz;
	VLFLOAT64 tx2,ty2,tz2;
	VLINT32 imagex,imagey,ix2,iy2;
	VLFLOAT32 squeeze_factor;
	
	VLINT32 vidd[3],vid[3];

	VLFLOAT64 vd2[3],vdd[3];  /* incrementals  motion along an axis */

	VLINT32 nloops;
	register VLINT32 loc_v, loc_rop, loc_ri;
	register VLUCHAR *loc_dptr,*loc_d2ptr;
	register VLINT32 i,lastinc,i2;

/* for plane render */
        VLINT32 *RBlut,*Glut, *Opacity;
        VLINT32 lopacity[256];
        VLINT32 ropacity[256];

/* for lighted rendering */
	VLINT32 wx,wy,wz,wi,wj;

	RBlut = (VLINT32 *)MALLOC(65536*sizeof(VLINT32));
	Glut = (VLINT32 *)MALLOC(65536*sizeof(VLINT32));
	Opacity = ( VLINT32 *) MALLOC(65536*sizeof(VLINT32));

	data1 = (set->aux_data[0]);
	data2 = (set->aux_data[1]);
	squeeze_factor = (set->squeeze_factor);
/* extract the lut and the opacity */
	for(i=0; i<256;i++) {
		ropacity[i] = ((set->rlut[i]) >> 24) & 0xff;
        lopacity[i] = ((set->llut[i]) >> 24) & 0xff;
	}
	for(i=0; i<256;i++) {
		for(i2=0;i2<256;i2++) {
			a = (set->rlut[i] & 0x000000ff)*
				(set->llut[i2] & 0x000000ff);
			b = ((set->rlut[i] & 0x00ff0000) >> 16)*
				((set->llut[i2] & 0x00ff0000) >> 16);
			a = (a >> 8) & 0xff;
			b = (b >> 8) & 0xff;
			RBlut[i+(i2*256)] = a | (b << 16);

			a = ((set->rlut[i] & 0x0000ff00) >> 8)*
				((set->llut[i2] & 0x0000ff00) >> 8);
			a = (a >> 8) & 0xff;
			Glut[i+(i2*256)] = a << 8;
			loc_rop = (ropacity[i]*lopacity[i2])/255;
			/* multiply by opacity */
			if ( !( flag & VL_RENDERLIGHT )){
				RBlut[i+(i2*256)] *= loc_rop;
				Glut[i+(i2*256)]  *= loc_rop;
			}
			Opacity[i+(i2*256)] = 255- loc_rop;
		}

	}

/* extra init for the lighted render */
	if (flag & VL_RENDERLIGHT) {
/* guarantee that looking at neighbors w/o checking will be ok */
/* do this be starting one step late and ending one step early */
		for(i=0;i<3;i++) {
			st[i] = st[i] + step[i];
			ri[i] = ri[i] - 1;
			le[i] = le[i] + 1;
		}
	}

	imagex = set->imagex;
	imagey = set->imagey;
	ix2 = imagex/2;
	iy2 = imagey/2;
/* use simple vars instead */
	tx = -(VLFLOAT64)(set->d[0])/2.0;
	ty = -(VLFLOAT64)(set->d[1])/2.0;
	tz = -(VLFLOAT64)(set->d[2])/2.0;
	tx2 = (set->trans[0]);
	ty2 = (set->trans[1]);
	tz2 = (set->trans[2]);

/* set variables to avoid the indexing operation */
	a = order[0];
	b = order[1];
	c = order[2];

	lastinc = mult[a]*step[a];

/* get the number of innermost loops */
	nloops = 0;
	for(loc_v=st[a]; ((loc_v <= ri[a]) && (loc_v >= le[a]));
		loc_v += step[a]) nloops += 1;

/* get basic point */
	v2[a] = st[a];
	v2[b] = st[b];
	v2[c] = st[c];
/* translate to cmass */
	v2[0] = v2[0] + tx;
	v2[1] = v2[1] + ty;
	v2[2] = (v2[2] + tz)*squeeze_factor;
/* mult throught the matrix */
	(void) vl_matmult(v2,vd,(set->xform)); 
/* do the real translations */
	vd[0] = vd[0] + tx2;
	vd[1] = vd[1] + ty2;
	vd[2] = vd[2] + tz2;
/* inc along last axis */
	v2[a] = st[a] + step[a];
	v2[b] = st[b];
	v2[c] = st[c];
/* translate to cmass */
	v2[0] = v2[0] + tx;
	v2[1] = v2[1] + ty;
	v2[2] = (v2[2] + tz)*squeeze_factor;
/* mult throught the matrix */
	(void) vl_matmult(v2,vd2,(set->xform)); 
/* do the real translations */
	vd2[0] = vd2[0] + tx2;
	vd2[1] = vd2[1] + ty2;
	vd2[2] = vd2[2] + tz2;
/* compute delta vector */
	vdd[0] = vd2[0] - vd[0];
	vdd[1] = vd2[1] - vd[1];
	vdd[2] = vd2[2] - vd[2];
/* in fixed point form */
	vidd[0] = vdd[0] * 65536;
	vidd[1] = vdd[1] * 65536;
	vidd[2] = vdd[2] * 65536;

/* every 256 points we need to issue endpnt bgnpnt... */
	for(v[c]=st[c]; ((v[c] <= ri[c]) && (v[c] >= le[c]));
		v[c]=v[c]+step[c]) {
		d1 = v[c] * mult[c];

/* skip the slice if it is masked out */	
		if (vl_light_globals.z_slice_mask) {
			if (vl_light_globals.z_slice_mask[v[c]] == 0) {
				continue;
			}
		}

		for(v[b]=st[b]; ((v[b] <= ri[b]) && (v[b] >= le[b]));
			v[b]=v[b]+step[b]) {
			d2=v[b]*mult[b] + d1 + st[a]*mult[a];

/* get basic point */
			v2[a] = st[a];
			v2[b] = v[b];
			v2[c] = v[c];
/* translate to cmass */
			v2[0] = v2[0] + tx;
			v2[1] = v2[1] + ty;
			v2[2] = (v2[2] + tz)*squeeze_factor;
/* mult throught the matrix */
			(void) vl_matmult(v2,vd,(set->xform)); 
/* do the real translations */
			vid[0] = (vd[0] + tx2) * 65536;
			vid[1] = (vd[1] + ty2) * 65536;
			vid[2] = (vd[2] + tz2) * 65536;

/* add 1/2 (a pixel for rounding) and the x,y offset to change coords */
			vid[0] = vid[0] + (ix2 * 65536) + 0x00008000L;
			vid[1] = vid[1] + (iy2 * 65536) + 0x00008000L;
			vid[2] = vid[2] + 0x00008000L;
		
/*	
			loc_st = step[a];
			loc_ri = ri[a];
			loc_le = le[a];

VLINT32 vl_clipline(VLINT32 *vid,VLINT32 *vidd,VLINT32 nloops,
        VLINT32 Xmin, VLINT32 Ymin, VLINT32 Xmax, VLINT32 Ymax,
        VLINT32 *out);
*/

			loc_ri = nloops;
            loc_dptr = data1 + d2;
            loc_d2ptr = data2 + d2;
			loc_v = 0;
			
	if (vl_clipline(vid,vidd,nloops,
			((par->rect[0])<<16),  
			((par->rect[1])<<16),
			(par->rect[2])<<16,
			(par->rect[3])<<16,
			vx)){
		if (vx[0] != 0) {
			loc_v = vx[0];
			loc_dptr += (lastinc*vx[0]);
			loc_d2ptr += (lastinc*vx[0]);
			vid[0] += (vx[0]*vidd[0]);
			vid[1] += (vx[0]*vidd[1]);
			vid[2] += (vx[0]*vidd[2]);
		}
		if (vx[1] != (nloops)) {
			loc_ri = vx[1];
		}

			switch (flag) {
				case 0:
				case (VL_RENDEROVER):
					REND_START16;
					OVER16;
					REND_END16;
					break;
				case (VL_RENDERPLANE):
				case (VL_RENDERPLANE | VL_RENDEROVER):
					REND_START16;
					PLANEOVER16;
					REND_END16;
					break;
				case (VL_RENDERLIGHT):
				case (VL_RENDEROVER | VL_RENDERLIGHT):
					REND_START16;
					OVERLIGHT16; 
					REND_END16;
					break;
				case (VL_RENDERPLANE | VL_RENDERLIGHT):
				case (VL_RENDERPLANE | VL_RENDEROVER |
						 VL_RENDERLIGHT):
					REND_START16;
					PLANEOVERLIGHT16; 
					REND_END16;
					break;
				default:
					v[c] = en[c] + step[c];
					par->ret = VL_ABORTED;
					break;
			}
	}
		};

		if (par->mynum == par->cbnum) {
			if (vl_rcallback(par->ret,inter,set)) {
				v[c] = en[c] + step[c];
				par->ret = VL_ABORTED;
				*(par->abort) = 1;
			}
		} else {
			if (*(par->abort) != 0) {
				v[c] = en[c] + step[c];
				par->ret = VL_ABORTED;
			}
		}

	};
	if (par->ret == VL_BACKGROUND) par->ret = VL_NOERR;

	FREE(Glut);
	FREE(RBlut);
	FREE(Opacity);

	i = par->cbnum;
	par->cbnum = -2;
	if (i == -1) vl_exit_thread(0);

	return;
}

