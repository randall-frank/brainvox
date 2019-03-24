#include "voxel.h"
#include "vl_util.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include "colorin.h"
#include "vl_voxel_renderers.h"

#include "vl_parallel.h"

void vl_m_pirender(par_quant *par)
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
	long int flag = par->flag;
	unsigned long int *image = par->image;
	short *zbuf = par->zbuf;
	long int *pbuffer = par->pbuffer;

/* real stuff */
	unsigned char *data;
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
	register unsigned char *loc_dptr;
	register long int i,lastinc;

/* for plane render */
        long int RBllut[256],Gllut[256];
        long int lopacity[256];
        long int RBrlut[256],Grlut[256];
        long int ropacity[256];

/* for lighted rendering */
	long int wx,wy,wz,wi,wj;

	data = (set->data);
	squeeze_factor = (set->squeeze_factor);
/* extract the lut and the opacity */
	if (flag & VL_RENDERLIGHT) {
		for(i=0; i<256;i++) {
			ropacity[i] = ((set->rlut[i]) >> 24) & 0xff;
			RBrlut[i] = (set->rlut[i] & 0x00ff00ff);
			Grlut[i] = (set->rlut[i] & 0x0000ff00);
			ropacity[i] = 255 - ropacity[i];

                	lopacity[i] = ((set->llut[i]) >> 24) & 0xff;
                	RBllut[i] = (set->llut[i] & 0x00ff00ff);
                	Gllut[i] = (set->llut[i] & 0x0000ff00);
                	lopacity[i] = 255 - lopacity[i];
		}
	} else {
		for(i=0; i<256;i++) {
			ropacity[i] = ((set->rlut[i]) >> 24) & 0xff;
			RBrlut[i] = (set->rlut[i] & 0x00ff00ff) * ropacity[i];
			Grlut[i] = (set->rlut[i] & 0x0000ff00) * ropacity[i];
			ropacity[i] = 255 - ropacity[i];

                	lopacity[i] = ((set->llut[i]) >> 24) & 0xff;
                	RBllut[i] = (set->llut[i] & 0x00ff00ff) * lopacity[i];
                	Gllut[i] = (set->llut[i] & 0x0000ff00) * lopacity[i];
                	lopacity[i] = 255 - lopacity[i];
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
            		loc_dptr = data + d2;
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
			vid[0] += (vx[0]*vidd[0]);
			vid[1] += (vx[0]*vidd[1]);
			vid[2] += (vx[0]*vidd[2]);
		}
		if (vx[1] != (nloops)) {
			loc_ri = vx[1];
		}

			switch (flag) {
				case 0:
					REND_START;
					NORMAL;
					REND_END;
					break;
				case (VL_RENDERPLANE):
					REND_START;
					PLANE;
					REND_END;
					break;
				case (VL_RENDEROVER):
					REND_START;
					OVER;
					REND_END;
					break;
				case (VL_RENDERLIGHT):
					REND_START;
					LIGHT;
					REND_END;
					break;
				case (VL_RENDERPLANE | VL_RENDEROVER):
					REND_START;
					PLANEOVER;
					REND_END;
					break;
				case (VL_RENDERPLANE | VL_RENDERLIGHT):
					REND_START;
					PLANELIGHT;
					REND_END;
					break;
				case (VL_RENDERPLANE | VL_RENDEROVER |
						 VL_RENDERLIGHT):
					REND_START;
					PLANEOVERLIGHT;
					REND_END;
					break;
				case (VL_RENDEROVER | VL_RENDERLIGHT):
					REND_START;
					OVERLIGHT;
					REND_END;
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

void vl_m_done(par_quant *par)
{
        vset *set = par->set;
	if (set->image == 0) {
		(void) free((unsigned char *)par->image);
	};
	if (par->flag & VL_RENDERPLANE) free((unsigned char *)par->pbuffer);
}

void vl_m_init(par_quant *par)
{
        vset *set = par->set;
        long int flag = par->flag;

	long int		a,b,i;
	unsigned long int	*image=0L;
	long int		*pbuffer=0L;
	short			*zbuf=0L;

/* clear */
	if (par->ret != VL_BACKGROUND) {
		a = 256;
		b = 256;
	} else {
		if (set->image == NULL) {
			par->ret = VL_IMGERR;
			return;
		}
		a = set->imagex;
		b = set->imagey;
	}

/* get memory for the buffering */
	if (set->image != NULL) {
		a = set->imagex;
		b = set->imagey;
		image = (unsigned long int *)(set->image);
		zbuf = (short *)(set->zbuffer);
	} else {
		image = (unsigned long int *) malloc(
			(sizeof(short)+sizeof(long int))*a*b);
		zbuf = (short *)(&(image[a*b]));
	}
	if (image == NULL) {
	        vl_puts("Unable to allocate buffer memory.\n");
		par->ret = VL_MEMERR;
		return;
	}

	if (flag & VL_RENDERPLANE) {
/* get memory for the plane buffer */
        	pbuffer = ( long int *) malloc(sizeof(long int)*a*b);
        	if (pbuffer == NULL) {
       	         if (set->image == NULL) (void) free((unsigned char *)image);
       	         	vl_puts("Unable to allocate buffer memory.\n");
      	          	par->ret = VL_MEMERR;
       	         	return;
        	};
/* init the plane buffer */
		if (flag & VL_RENDERZCLIP) {
			for(i=0;i<a*b;i++) pbuffer[i] = zbuf[i];
		} else {
        		(void)vl_makeplane(pbuffer,a,b,set);
		}
	}

/* extra init for the lighted render */
	if (flag & VL_RENDERLIGHT) {
/* build the lighting table */
        	vl_build_l_table(set);
	}

	if ((flag & VL_RENDEROVER) == 0) {
/* init the frame buffer */
		for (i=0; i <a*b; i++) image[i] = set->backpack;
		for (i=0; i <a*b; i++) zbuf[i] = VL_INFINITY;
	}

	par->image = image;
	par->zbuf = zbuf;
	par->pbuffer = pbuffer;

	return;
}
