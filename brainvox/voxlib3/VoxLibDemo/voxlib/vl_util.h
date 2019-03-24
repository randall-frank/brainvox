#ifndef __VL_UTIL_H__
#define __VL_UTIL_H__

#define NIL 0
#define ROUND(a) ((a) >0 ? (long int)(a+0.5) : -(long int)(0.5-a))
#define NEWSTRUCT(x) (struct x *)(malloc((unsigned)sizeof(struct x)))
#define SWAP(a,b) { a^=b; b^=a; a^=b;}
/* l when a=0 and h when a=1 */
#define LERP(a,l,h) ((l)+(((h)-(l))*(a)))
#define CLAMP(v,l,h) ((v)<(l) ? (l) : (v) > (h) ? (h) : (v))
#define MIN(a,b) (((a)<(b)?(a):(b)))
#define MAX(a,b) (((a)>(b)?(a):(b)))
#define ASSERT(x) \
if (!(x)) fprintf(stderr," Assert failed: x\n");

#ifdef __cplusplus
extern "C" {
#endif

/* matrix routines */
void vl_calcmatrix(double *rots,double xform[3][3]);     
void vl_matmult(double *in,double *out,double mat[3][3]);
void vl_imatmult(long int *in,double *out,double mat[3][3]);
void axisorder(long int *order, long int *dir, vset *set);
void vl_dopoint(long int *v, double *vd, vset *set);
void invert3x3(double mat[3][3]);

/* cutting plane routines */
void vl_makeplane(long int *pbuf, long int dx, long int dy, vset *set);

/* lighting (rendered) support */
void vl_get_inten(long int dx,long int dy,long int dz,long int *rb,long int *g);
void vl_build_l_table(vset *set);

/* rendering routines */
void vl_gtxrender(vset *set,long int *st,long int *en,long int *le,long int *ri,
   long int *step,long int *mult,long int *order,long int inter, long int *ret);
void vl_pirender(vset *set,long int *st,long int *en,long int *le,long int *ri,
   long int *step,long int *mult,long int *order,long int inter, long int flag,
   long int *ret);
long int med3x3(long int *b1,long int *b2,long int *b3);
long int avg3x3(long int *b1,long int *b2,long int *b3);
short savg3x3(short *b1,short *b2,short *b3);
short smed3x3(short *b1,short *b2,short *b3);

void vl_plrender(vset *set,long int *st,long int *en,long int *le,long int *ri,
   long int *step,long int *mult,long int *order,long int inter, long int *ret);
void vl_zpirender(vset *set,long int *st,long int *en,long int *le,long int *ri,
   long int *step,long int *mult,long int *order,long int inter, long int *ret);
void vl_zplrender(vset *set,long int *st,long int *en,long int *le,long int *ri,
   long int *step,long int *mult,long int *order,long int inter, long int *ret);
void vl_lpirender(vset *set,long int *st,long int *en,long int *le,long int *ri,
   long int *step,long int *mult,long int *order,long int inter, long int *ret);


long int vl_clipline(long int *vid,long int *vidd,long int nloops,
        long int Xmin, long int Ymin, long int Xmax, long int Ymax,
        long int *out);

#ifdef __cplusplus
 }
#endif

#endif
