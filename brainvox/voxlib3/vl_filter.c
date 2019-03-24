#include "voxel.h"
#include "vl_util.h"
#include <stdio.h>
#include <stdlib.h>

void vl_filter_output_(VLINT32 *image,VLINT32 dx,VLINT32 dy,VLINT32 filter)
{
	VLINT32 i,j,plane;
	VLINT32 *b1,*b2,*b3;
	VLINT16 *simage,*oimage,*sb1,*sb2,*sb3;

	switch (filter) {
		case VL_MEDIAN:
			for (j=0; j<dy*dx;j++) {
			    image[j] &=  0xffffff;
			}
			for(plane=0; plane<3;plane++) {
/* filter the low byte plane and deposit into high byte plane*/
			for (j=1; j<dy-1;j++) {
				b1 = &(image[(j-1)*dx]);
				b2 = b1 + dx;
				b3 = b2 + dx;
				for (i=1; i<dx-1;i++) {	
					image[i+j*dx] &= 0xffffff;
					image[i+j*dx] |= 
						(med3x3(b1++,b2++,b3++) << 24);
				}
			}
/* shift the planes */
			for (j=0; j<dy*dx;j++) {
			    image[j] =  image[j] >> 8;
			    image[j] &=  0xffffff;
			}
			}
			break;
		case VL_AVERAGE:
			for (j=0; j<dy*dx;j++) {
			    image[j] &=  0xffffff;
			}
			for(plane=0; plane<3;plane++) {
/* filter the low byte plane and deposit into high byte plane*/
			for (j=1; j<dy-1;j++) {
				b1 = &(image[(j-1)*dx]);
				b2 = b1 + dx;
				b3 = b2 + dx;
				for (i=1; i<dx-1;i++) {	
					image[i+j*dx] &= 0xffffff;
					image[i+j*dx] |= 
						(avg3x3(b1++,b2++,b3++) << 24);
				}
			}
/* shift the planes */
			for (j=0; j<dy*dx;j++) {
			    image[j] =  image[j] >> 8;
			    image[j] &=  0xffffff;
			}
			}
			break;
		case VL_ZAVERAGE :
			oimage = (VLINT16 *)MALLOC(sizeof(VLINT16)*dx*dy);
			if (oimage == 0L) return;
			simage = (VLINT16 *)image;
			for(i=0;i<dx*dy;i++) oimage[i] = simage[i];
			for(j=1;j<dy-1;j++) {
				sb1 = &(oimage[(j-1)*dx]);
				sb2 = sb1 + dx;
				sb3 = sb2 + dx;
				for(i=1;i<dx-1;i++) {
					simage[i+j*dx] =
						 savg3x3(sb1++,sb2++,sb3++);
					if (simage[i+j*dx] < VL_OUTTHERE)
						simage[i+j*dx] = oimage[i+j*dx];
				}
			}
			FREE(oimage);
			break;
		case VL_ZMEDIAN :
			oimage = (VLINT16 *)MALLOC(sizeof(VLINT16)*dx*dy);
			if (oimage == 0L) return;
			simage = (VLINT16 *)image;
			for(i=0;i<dx*dy;i++) oimage[i] = simage[i];
			for(j=1;j<dy-1;j++) {
				sb1 = &(oimage[(j-1)*dx]);
				sb2 = sb1 + dx;
				sb3 = sb2 + dx;
				for(i=1;i<dx-1;i++) {
					simage[i+j*dx] =
						 smed3x3(sb1++,sb2++,sb3++);
					if (simage[i+j*dx] < VL_OUTTHERE)
						simage[i+j*dx] = oimage[i+j*dx];
				}
			}
			FREE(oimage);
			break;
	}
	return;
}


#define s2(a,b) {register VLINT32 t; if ((t=b-a)<0) {a+=t; b-=t;}}
#define mn3(a,b,c) s2(a,b); s2(a,c);
#define mx3(a,b,c) s2(b,c); s2(a,c);
#define mnmx3(a,b,c) mx3(a,b,c); s2(a,b);
#define mnmx4(a,b,c,d) s2(a,b); s2(c,d); s2(a,c); s2(b,d);
#define mnmx5(a,b,c,d,e) s2(a,b); s2(c,d); mn3(a,c,e); mx3(b,d,e);
#define mnmx6(a,b,c,d,e,f) s2(a,d); s2(b,e); s2(c,f);\
                            mn3(a,b,c); mx3(d,e,f);

#define ss2(a,b) {register VLINT16 t; if ((t=b-a)<0) {a+=t; b-=t;}}
#define smn3(a,b,c) ss2(a,b); ss2(a,c);
#define smx3(a,b,c) ss2(b,c); ss2(a,c);
#define smnmx3(a,b,c) smx3(a,b,c); ss2(a,b);
#define smnmx4(a,b,c,d) ss2(a,b); ss2(c,d); ss2(a,c); ss2(b,d);
#define smnmx5(a,b,c,d,e) ss2(a,b); ss2(c,d); smn3(a,c,e); smx3(b,d,e);
#define smnmx6(a,b,c,d,e,f) ss2(a,d); ss2(b,e); ss2(c,f);\
                            smn3(a,b,c); smx3(d,e,f);

VLINT32 med3x3(VLINT32 *b1,VLINT32 *b2,VLINT32 *b3)
/*
 * Find median on a 3x3 input box of integers.
 * b1, b2, b3 are pointers to the left-hand edge of three
 * parallel scan-lines to form a 3x3 spatial median.
 * Rewriting b2 and b3 as b1 yields code which forms median
 * on input presented as a linear array of nine elements.
 */
    {
    register VLINT32 r1, r2, r3, r4, r5, r6;
    r1 = b1[0] & 0xff;
    r2 = b1[1] & 0xff;
    r3 = b1[2] & 0xff;
    r4 = b2[0] & 0xff;
    r5 = b2[1] & 0xff;
    r6 = b2[2] & 0xff;
    mnmx6(r1, r2, r3, r4, r5, r6);
    r1 = b3[0] & 0xff;
    mnmx5(r1, r2, r3, r4, r5);
    r1 = b3[1] & 0xff;
    mnmx4(r1, r2, r3, r4);
    r1 = b3[2] & 0xff;
    mnmx3(r1, r2, r3);
    return(r2);
    }   

VLINT32 avg3x3(VLINT32 *b1,VLINT32 *b2,VLINT32 *b3)
{
/* implements kernel: 	1 1 1
			1 4 1
			1 1 1 
	See med3x3 for details
*/
	VLINT32 i;
	VLINT32 junk;
	i = b2[1] & 0xff;
	junk = 3*i;
	for(i=0;i<3;i++) {
		junk = junk + (b1[i] & 0xff);
		junk = junk + (b2[i] & 0xff);
		junk = junk + (b3[i] & 0xff);
	}
	return (junk / 12);
}

VLINT16 savg3x3(VLINT16 *b1,VLINT16 *b2,VLINT16 *b3)
{
/* implements kernel: 	1 1 1
			1 1 1
			1 1 1 
	See med3x3 for details
*/
	VLINT32 i;
	VLINT32 junk;
	i = b2[1];
/*
	junk = 3*i;
*/
	junk = 0;
	for(i=0;i<3;i++) {
		junk = junk + (b1[i]);
		junk = junk + (b2[i]);
		junk = junk + (b3[i]);
	}
	return ((VLINT16)(junk / 9));
}

VLINT16 smed3x3(VLINT16 *b1,VLINT16 *b2,VLINT16 *b3)
/*
 * See med3x3 for details
 */
    {
    register VLINT16 r1, r2, r3, r4, r5, r6;
    r1 = b1[0];
    r2 = b1[1];
    r3 = b1[2];
    r4 = b2[0];
    r5 = b2[1];
    r6 = b2[2];
    smnmx6(r1, r2, r3, r4, r5, r6);
    r1 = b3[0];
    smnmx5(r1, r2, r3, r4, r5);
    r1 = b3[1];
    smnmx4(r1, r2, r3, r4);
    r1 = b3[2];
    smnmx3(r1, r2, r3);
    return(r2);
    }   

