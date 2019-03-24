#include "voxel.h"
#include "vl_util.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include "colorin.h"
#include "vl_voxel_renderers.h"

#include "vl_parallel.h"

void vl_m_pirend16(par_quant *par)
{
/* local copy of par_quant struct */
        vset *set = par->set;
        long int *st = par->st;
        long int *en = par->en;
        long int *le = par->le;
        long int *ri = par->ri;
        long int *step = par->step;
        long int *mult = par->mult;
        long int *order = par->order;
        long int inter = par->inter;
	long int flag = (par->flag & (~VL_RENDER16BIT));
	unsigned long int *image = par->image;
	short *zbuf = par->zbuf;
	long int *pbuffer = par->pbuffer;

/* real stuff */
	unsigned char *data1,*data2;
	long int d1,d2;
	long int a,b,c;
	long int k;
	long int v[3],vx[3];
	double v2[3];
	double vd[3];
	double tx,ty,tz;
	double tx2,ty2,tz2;
	long int imagex,imagey,ix2,iy2;
	float squeeze_factor;
	
	long int vidd[3],vid[3];

	double vd2[3],vdd[3];  /* incrementals  motion along an axis */

	long int loc_st,nloops;
	register long int loc_v, loc_rop, loc_lop, loc_ri, loc_le;
	register unsigned char *loc_dptr,*loc_d2ptr;
	register long int i,lastinc,i2;

/* for plane render */
        long int RBlut[65536],Glut[65536];
        long int lopacity[256];
        long int ropacity[256];

/* for lighted rendering */
	long int wx,wy,wz,wi,wj;

	data1 = (set->aux_data[0]);
	data2 = (set->aux_data[1]);
	squeeze_factor = (set->squeeze_factor);
/* extract the lut and the opacity */
	for(i=0; i<256;i++) {
		ropacity[i] = ((set->rlut[i]) >> 24) & 0xff;
               	lopacity[i] = ((set->llut[i]) >> 24) & 0xff;

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
	tx = -(double)(set->d[0])/2.0;
	ty = -(double)(set->d[1])/2.0;
	tz = -(double)(set->d[2])/2.0;
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

long int vl_clipline(long int *vid,long int *vidd,long int nloops,
        long int Xmin, long int Ymin, long int Xmax, long int Ymax,
        long int *out);
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
			if (vl_rcallback(par->ret,set)) {
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

	par->cbnum = -2;
	if (par->cbnum == -1) vl_exit_thread(NULL);

	return;
}

