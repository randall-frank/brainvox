#ifndef __VL_UTIL_H__
#define __VL_UTIL_H__

#define NIL 0
#define ROUND(a) ((a) >0 ? (VLINT32)(a+0.5) : -(VLINT32)(0.5-a))
#define NEWSTRUCT(x) (struct x *)(malloc((unsigned)sizeof(struct x)))
#define SWAP(a,b) { a^=b; b^=a; a^=b;}
/* l when a=0 and h when a=1 */
#define LERP(a,l,h) ((l)+(((h)-(l))*(a)))
#define CLAMP(v,l,h) ((v)<(l) ? (l) : (v) > (h) ? (h) : (v))
#define MIN(a,b) (((a)<(b)?(a):(b)))
#define MAX(a,b) (((a)>(b)?(a):(b)))
#ifndef ASSERT
#define ASSERT(x) \
if (!(x)) fprintf(stderr," Assert failed: x\n");
#endif
#ifdef __cplusplus
extern "C" {
#endif

/* matrix routines */
void vl_calcmatrix(VLFLOAT64 *rots,VLFLOAT64 xform[3][3]);     
void vl_matmult(VLFLOAT64 *in,VLFLOAT64 *out,VLFLOAT64 mat[3][3]);
void vl_imatmult(VLINT32 *in,VLFLOAT64 *out,VLFLOAT64 mat[3][3]);
void vl_matmult4x4(VLFLOAT64 *in,VLFLOAT64 *out,VLFLOAT64 mat[4][4]);

void axisorder(VLINT32 *order, VLINT32 *dir, vset *set);
void vl_dopoint(VLINT32 *v, VLFLOAT64 *vd, vset *set);
void invert3x3(VLFLOAT64 mat[3][3]);

/* cutting plane routines */
void vl_makeplane(VLINT32 *pbuf, VLINT32 dx, VLINT32 dy, vset *set);

/* lighting (rendered) support */
void vl_get_inten(VLINT32 dx,VLINT32 dy,VLINT32 dz,VLINT32 *rb,VLINT32 *g);
void vl_build_l_table(vset *set);

void vl_pirender(vset *set,VLINT32 *st,VLINT32 *en,VLINT32 *le,VLINT32 *ri,
   VLINT32 *step,VLINT32 *mult,VLINT32 *order,VLINT32 inter, VLINT32 flag,
   VLINT32 *ret);
VLINT32 med3x3(VLINT32 *b1,VLINT32 *b2,VLINT32 *b3);
VLINT32 avg3x3(VLINT32 *b1,VLINT32 *b2,VLINT32 *b3);
VLINT16 savg3x3(VLINT16 *b1,VLINT16 *b2,VLINT16 *b3);
VLINT16 smed3x3(VLINT16 *b1,VLINT16 *b2,VLINT16 *b3);

VLINT32 vl_clipline(VLINT32 *vid,VLINT32 *vidd,VLINT32 nloops,
        VLINT32 Xmin, VLINT32 Ymin, VLINT32 Xmax, VLINT32 Ymax,
        VLINT32 *out);

#ifdef __cplusplus
 }
#endif

#endif
