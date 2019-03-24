#include "voxel.h"
#include "vl_util.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include "colorin.h"
#include "vl_voxel_renderers.h"

void vl_pirender(vset *set,VLINT32 *st,VLINT32 *en,VLINT32 *le,VLINT32 *ri,
	VLINT32 *step,VLINT32 *mult,VLINT32 *order,VLINT32 inter,
	VLINT32 flag,VLINT32 *ret)
{
	VLUCHAR *data;
	VLUINT32 *image;
	VLINT16 *zbuf;
	VLINT32 d1,d2;
	VLINT32 a,b,c;
	VLINT32 k;
	VLINT32 v[3],vx[3];
	VLFLOAT64 v2[3];
	VLFLOAT64 vd[3];
	VLFLOAT64 tx,ty,tz;
	VLFLOAT64 tx2,ty2,tz2;
	VLINT32 imagex,imagey,ix2,iy2;
	VLFLOAT32 squeeze_factor;
	
	VLINT32 vidd[3],vid[3];

	VLFLOAT64 vd2[3],vdd[3];  /* incrementals  motion along an axis */

	VLINT32 loc_st,nloops;
	register VLINT32 loc_v, loc_rop, loc_lop, loc_ri, loc_le;
	register VLUCHAR *loc_dptr;
	register VLINT32 i,lastinc;

/* for plane render */
	VLINT32 *pbuffer;	
        VLINT32 RBllut[256],Gllut[256];
        VLINT32 lopacity[256];
        VLINT32 RBrlut[256],Grlut[256];
        VLINT32 ropacity[256];

/* for lighted rendering */
	VLINT32 wx,wy,wz,wi,wj;

	data = (set->data);
	squeeze_factor = (set->squeeze_factor);

        if ((flag & VL_RENDERMIP) || (flag & VL_RENDERMNIP)) {
/* if a MIP mode, setup opacity and color straight */                 
                for(i=0; i<256;i++) {
                        ropacity[i] = ((set->rlut[i]) >> 24) & 0xff;
                        RBrlut[i] = set->rlut[i] & 0x00ffffff;
                        if ((flag & VL_RENDERMNIP) && (ropacity[i] != 0)) {
                                ropacity[i] = 255 - ropacity[i];
                        }

                        lopacity[i] = ((set->llut[i]) >> 24) & 0xff;
                        RBllut[i] = set->llut[i] & 0x00ffffff;
                        if ((flag & VL_RENDERMNIP) && (lopacity[i] != 0)) {
                                lopacity[i] = 255 - lopacity[i];
                        } 
                }
        } else if (flag & VL_RENDERLIGHT) {
/* extract the lut and the opacity */
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
/* clear */
	if (*ret != VL_BACKGROUND) {
		a = 256;
		b = 256;
	} else {
		if (set->image == NULL) {
			*ret = VL_IMGERR;
			return;
		}
		a = set->imagex;
		b = set->imagey;
	}

/* get memory for the buffering */
	if (set->image != NULL) {
		a = set->imagex;
		b = set->imagey;
		image = (VLUINT32 *)(set->image);
		zbuf = (VLINT16 *)(set->zbuffer);
	} else {
		image = (VLUINT32 *) malloc(
			(sizeof(VLINT16)+sizeof(VLINT32))*a*b);
		zbuf = (VLINT16 *)(&(image[a*b]));
	}
	if (image == NULL) {
	        vl_puts("Unable to allocate buffer memory.\n");
		*ret = VL_MEMERR;
		return;
	}

	if (flag & VL_RENDERPLANE) {
/* get memory for the plane buffer */
        	pbuffer = ( VLINT32 *) malloc(sizeof(VLINT32)*a*b);
        	if (pbuffer == NULL) {
       	         if (set->image == NULL) (void) free((VLUCHAR *)image);
       	         	vl_puts("Unable to allocate buffer memory.\n");
      	          	*ret = VL_MEMERR;
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
/* guarantee that looking at neighbors w/o checking will be ok */
/* do this be starting one step late and ending one step early */
		for(i=0;i<3;i++) {
			st[i] = st[i] + step[i];
			ri[i] = ri[i] - 1;
			le[i] = le[i] + 1;
		}
	}

	if ((flag & VL_RENDEROVER) == 0) {
/* init the frame buffer */
		for (i=0; i <a*b; i++) image[i] = set->backpack;
		for (i=0; i <a*b; i++) zbuf[i] = VL_INFINITY;
	}
/* clear the alpha channel */
	for (i=0; i <a*b; i++) image[i] &= 0x00ffffff;

	imagex = a;
	imagey = b;
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
            		loc_dptr = data + d2;
			loc_v = 0;
			
	if (vl_clipline(vid,vidd,nloops,1<<16,1<<16,(imagex-1)<<16,
			(imagey-1)<<16,vx)){
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
				case (VL_RENDERMIP):
				case (VL_RENDEROVER | VL_RENDERMIP):
					REND_START;
					MIPOVER;
					REND_END;
					break;
				case (VL_RENDERMNIP):
				case (VL_RENDEROVER | VL_RENDERMNIP):
					REND_START;
					MIPOVER;
					REND_END;
					break;
				default:
					v[c] = en[c] + step[c];
					*ret = VL_ABORTED;
					break;
			}
	}
		};
		if (vl_rcallback(*ret,inter,set)) {
			v[c] = en[c] + step[c];
			*ret = VL_ABORTED;
		}
	};
/* copy image if the user requested it  and the sizes match */
	if (set->image == 0) {
		(void) free((VLUCHAR *)image);
	};
	if (flag & VL_RENDERPLANE) free((VLUCHAR *)pbuffer);
	if (*ret == VL_BACKGROUND) *ret = VL_NOERR;

	return;
}
