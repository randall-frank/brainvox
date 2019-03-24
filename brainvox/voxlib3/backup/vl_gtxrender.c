#ifdef GLCODE

#include "voxel.h"
#include "vl_util.h"
#include <stdio.h>
#include <math.h>
#include "gl.h"
#include "device.h"

void vl_gtxrender(vset *set,long int *st,long int *en,long int *le,long int *ri,
	long int *step,long int *mult,long int *order,long int inter,
	long int *ret)
{
	float squeeze_factor;
	unsigned char *data;
	unsigned int *timage;
	register int count,lastinc;
	int d1,d2;
	long int a,b,c;
	int lut[256];
	int opacity[256];
	register int i,li;
	int imagex,imagey; 
	short interrpt ;
	int v[3];
	float vf[3];

        register long int loc_v, loc_st, loc_op, loc_ri, loc_le;
        register unsigned char *loc_dptr;

	squeeze_factor = (set->squeeze_factor);

	data = (set->data);
	interrpt  = inter;
/* extract the lut and the opacity */
	for(i=0; i<256;i++) {
		opacity[i] = ((set->rlut[i]) >> 24) & 0xff;
		lut[i] = (set->rlut[i]);
	};
/* clear */
	(void) blendfunction(BF_SA, BF_MSA); 
/*	(void) subpixel(TRUE);
	(void) pntsmooth(SMP_ON); */
/* Do it like the PI does... */
	(void) subpixel(TRUE);
	(void) pntsmooth(SMP_OFF); 

        (void) zbuffer(TRUE);
        (void) zfunction(ZF_ALWAYS);

	(void) getsize(&a,&b);
	(void) ortho(-(a/2.0),(a/2.0),-(b/2.0),(b/2.0),(a/2.0),-(a/2.0));
	imagex = a;
	imagey = b;

	(void) cpack(set->backpack);
	(void) clear();
	(void) lsetdepth(0L,(long)(a*8)); /* zbuffer has 3 bits subpix res */
	(void) zclear();
#ifdef QUICKIE
	timage = (unsigned int *)malloc(set->d[0]*set->d[1]*sizeof(long));
	loc_dptr = data;
	for(i=0;i<set->d[2];i++) {
/*
		for(li=0;li<set->d[0]*set->d[1];li++) {
			timage[li] = lut[*loc_dptr++];
		}
*/
		(void) lrectwrite(0,0,(set->d[0]/4)-1,(set->d[1]/4)-1,
				(unsigned long *)loc_dptr);
	}
	free(timage);
	(void) blendfunction(BF_ONE, BF_ZERO);
	(void) subpixel(FALSE);
	(void) pntsmooth(SMP_OFF); 
	(void) zbuffer(FALSE);
	return;
#endif

/* condition the display matrix */
	(void) pushmatrix();
	(void) translate((set->trans[0]),(set->trans[1]),(set->trans[2]));
	(void) rot((float)(set->rots[2]),'z');
	(void) rot((float)(set->rots[1]),'y');
	(void) rot((float)(set->rots[0]),'x');
/* center of volume transformation */
	(void) translate(-(set->d[0])/2.0,-(set->d[1])/2.0,-((set->d[2])/2.0)*squeeze_factor);

/* set variables to avoid the indexing operation */
	a = order[0];
	b = order[1];
	c = order[2];

	lastinc = mult[a]*step[a];


        loc_st = step[a];
        loc_ri = ri[a];
        loc_le = le[a];
	li = 0;

	(void) bgnpoint();
	for(v[c]=st[c]; ((v[c] <= ri[c]) && (v[c] >= le[c]));
		v[c]=v[c]+step[c]) {
		d1 = v[c] * mult[c];

		for(v[b]=st[b]; ((v[b] <= ri[b]) && (v[b] >= le[b]));
			v[b]=v[b]+step[b]) {
			loc_dptr = data + (v[b]*mult[b] + d1 + st[a]*mult[a]);

                        for(loc_v=st[a]; ((loc_v <= loc_ri) &&
                                (loc_v >= loc_le)); loc_v=loc_v+loc_st) {
/* get the voxel value */
                                i = *loc_dptr;
                                loc_dptr = loc_dptr + lastinc;

/* if it is visible plot it */
				if (opacity[i] > 1) {
					if (i != li) {
				        	(void) cpack(lut[i]);
						li = i;
					}
					v[a] = loc_v;
					vf[0] = v[0];
					vf[1] = v[1];
					vf[2] = v[2]*squeeze_factor;
					(void) v3f(vf);
				};
			};
		};
		if (getbutton(interrpt )) { (void) qreset;
						v[c] = en[c] + step[c];
                                                *ret = VL_ABORTED;
						};
	};
	(void) endpoint();
	(void) popmatrix();
/* copy image if the user requested it  and the sizes match */
	(void) blendfunction(BF_ONE, BF_ZERO);
	(void) subpixel(FALSE);
	(void) pntsmooth(SMP_OFF); 
	(void) zbuffer(FALSE);
        if (set->image != 0) {
                if ((imagex == set->imagex) &&
                         (imagey == set->imagex)) {

			timage = (unsigned int *)(set->image);
			(void) readsource(SRC_ZBUFFER);
			(void) lrectread(0,0,imagex-1,imagey-1,
					(unsigned long *)timage);

/* convert 32 bit Zbuffer to 16 bits signed */
			for(i=0;i<imagex*imagey; i++) {
				a = (timage[i] & 0x007fffff);
				if (a == 0x7fffff) {
					a = VL_INFINITY;
				} else {
					a = a - ((imagex/2) * 8);
				}
				(set->zbuffer)[i] = a;
			}

			timage = (unsigned int *)(set->image);
			(void) readsource(SRC_AUTO);
			(void) lrectread(0,0,imagex-1,imagey-1,
					(unsigned long *)timage);

                } else {
                        vl_puts("Unable to copy image to buffer.\n");
                        vl_puts("Buffer size does not match window size.\n");
                        *ret = VL_IMGERR;
                };
        };    
	return;
}
#else
void vl_gtxrender(vset *set,long int *st,long int *en,long int *le,long int *ri,
	long int *step,long int *mult,long int *order,long int inter,
	long int *ret)
{
	return;
}
#endif
