#include "voxel.h"
#include "vl_util.h"
#include "gl.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define V2D v3d

typedef struct {
        double x[3],y[3],p[3],n[3];
        vset *theset;
        long int imdx,imdy;
} sample_common;

extern sample_common held_data;

long int withinthevolume(long int *p,vset *set);
void vl_resample_drvr_(long int dx,long int dy,short *image,float *rots,
		float *dist,vset *set,unsigned char *data);
void vl_splane_(long int dx,long int dy,float *rots,float *dist,vset *set);
void vl_resample_(long int dx,long int dy,short *image,float *rots,float *dist,
		vset *set);
void vl_resample16_(long int dx,long int dy,short *image,float *rots,
		float *dist,vset *set);

void do_filter_proc_(long int *image,short *zbuf,long int dx,long int dy,
	long int filt)
{
	switch(filt) {
		case VL_MEDIAN:
		case VL_AVERAGE:
			vl_filter_output_(image,dx,dy,filt);
			break;	
		case VL_ZMEDIAN:
		case VL_ZAVERAGE:
			vl_filter_output_((long int *)zbuf,dx,dy,filt);
			break;
	}
	return;
}
void DO_FILTER_PROC(long int *image,short *zbuf,long int dx,long int dy,
	long int filt)
{
do_filter_proc_(image,zbuf,dx,dy,filt);
}

void do_postlight_(long int *image,short *zbuf,long int *out,long int dx,
	long int dy,float *n)
{
	double	norm[4];

	norm[0] = n[0];
	norm[1] = n[1];
	norm[2] = n[2];
	norm[3] = n[3];
	vl_postlight_(image,zbuf,out,dx,dy,norm);
	return;
}
void DO_POSTLIGHT(long int *image,short *zbuf,long int *out,long int dx,
	long int dy,float *n)
{
do_postlight_(image,zbuf,out,dx,dy,n);
}

void set_intrinsic_lighting_(long int *what,long int *flag)
{
/* what = on/off int, dx,dy,dz,l1,l2 */
	double	v;

	if (what[0] == 0) {
		*flag = 0L;
	} else {
		v = ((float *)what)[1];
		vl_set_light_param_(VL_ILIGHT_DX,&(v));
		v = ((float *)what)[2];
		vl_set_light_param_(VL_ILIGHT_DY,&(v));
		v = ((float *)what)[3];
		vl_set_light_param_(VL_ILIGHT_DZ,&(v));
		v = ((float *)what)[4];
		vl_set_light_param_(VL_ILIGHT_SRCINTEN,&(v));
		v = ((float *)what)[5];
		vl_set_light_param_(VL_ILIGHT_INTEN,&(v));
		*flag = VL_RENDERLIGHT;
	}
	return;
}
void SET_INTRINSIC_LIGHTING(long int *what,long int *flag)
{
set_intrinsic_lighting_(what,flag);
}

void vl_getextents_(long int *extents,vset *set)
{

	extents[0] = set->start[0];
	extents[1] = set->end[0];
	extents[2] = set->start[1];
	extents[3] = set->end[1];
	extents[4] = set->start[2];
	extents[5] = set->end[2];

	return;
}
void VL_GETEXTENTS(long int *extents,vset *set)
{
vl_getextents_(extents,set);
}

void vl_getbufs_(long *image, long *zbuffer, long *dx, long *dy, vset *set)
{
	*image = (long)(set->image);
	*zbuffer = (long)(set->zbuffer);
	*dx = (set->imagex);
	*dy = (set->imagey);
}
void VL_GETBUFS(long *image, long *zbuffer, long *dx, long *dy, vset *set)
{
vl_getbufs_(image,zbuffer,dx,dy,set);
}

void vl_getpals_(long *rlut, long *llut, vset *set)
{
	int i;
	for(i=0;i<256; i++) {
		rlut[i] = set->rlut[i];
		llut[i] = set->llut[i];
	}
}
void VL_GETPALS(long *rlut, long *llut, vset *set)
{
vl_getpals_(rlut,llut,set);
}
void vl_getpals2_(long *rlut,long *ropac,long *llut,long *lopac,
                  long *bcolor, vset *set)
{
	int i;
	for(i=0;i<256; i++) {
		rlut[i    ] = (set->rlut[i] >>  0) & 0xff;
		rlut[i+256] = (set->rlut[i] >>  8) & 0xff;
		rlut[i+512] = (set->rlut[i] >> 16) & 0xff;
		ropac[i]    = (set->rlut[i] >> 24) & 0xff;
		ropac[i+256] = 0; ropac[i+512] = 0;
		llut[i    ] = (set->llut[i] >>  0) & 0xff;
		llut[i+256] = (set->llut[i] >>  8) & 0xff;
		llut[i+512] = (set->llut[i] >> 16) & 0xff;
		lopac[i]    = (set->llut[i] >> 24) & 0xff;
		lopac[i+256] = 0; lopac[i+512] = 0;
	}
	*bcolor = set->backpack;
}
void VL_GETPALS2(long *rlut,long *ropac,long *llut,long *lopac,
		long *bcolor, vset *set)
{
vl_getpals2_(rlut,ropac,llut,lopac,bcolor,set);
}



void vl_resample_(long int dx,long int dy,short *image,float *rots,float *dist,
		vset *set)
{
	vl_resample_drvr_(dx,dy,image,rots,dist,set,set->data);
	return;
}
void VL_RESAMPLE(long int dx,long int dy,short *image,float *rots,float *dist,
		vset *set)
{
vl_resample_(dx,dy,image,rots,dist,set);
}

void vl_resample16_(long int dx,long int dy,short *image,float *rots,
		float *dist,vset *set)
{
	if (set->aux_data[1] == NULL) return;

	vl_resample_drvr_(dx,dy,image,rots,dist,set,set->aux_data[1]);
	return;
}
void VL_RESAMPLE16(long int dx,long int dy,short *image,float *rots,
		float *dist,vset *set)
{
vl_resample16_(dx,dy,image,rots,dist,set);
}

void vl_resample_drvr_(long int dx,long int dy,short *image,float *rots,
		float *dist,vset *set,unsigned char *data)
{
	double xform[3][3];
	double drots[3];
	double p[3],temp[3],n[3],y[3],x[3];
	double xh,yh,x1,y1;
	long int ptr,ptr2,s[3];
	long int ddx,ddy,ddz,i;
	double hdx,hdy,hdz;
	double samp[3];

/* copy the rotations */
	for(i=0;i<3;i++) drots[i] = rots[i];

/* initial point is 0,0,0 */
	p[0] = 0.0;
	p[1] = 0.0;
	p[2] = 0.0;

/* compute matrix */
	vl_calcmatrix(drots,xform);

/* initial normal is 0,0,1 */
	temp[0] = 0.0;
	temp[1] = 0.0;
	temp[2] = 1.0;
/* mult N by matrix */
	vl_matmult(temp,n,xform);

/* intial y vector is 0,1,0 */
	temp[0] = 0.0;
	temp[1] = 1.0;
	temp[2] = 0.0;
/* mult Y by matrix */
	vl_matmult(temp,y,xform);

/* compute X by Y x N */
	x[0] = (y[1]*n[2])-(n[1]*y[2]);
	x[1] = (y[2]*n[0])-(n[2]*y[0]);
	x[2] = (y[0]*n[1])-(n[0]*y[1]);

/* compute p = p + Dn */
	p[0] = p[0] + (*dist)*n[0];
	p[1] = p[1] + (*dist)*n[1];
	p[2] = p[2] + (*dist)*n[2];

/* image space variables */
	ptr = 0;
	yh = (dy)/2.0;
	xh = (dx)/2.0;
/* volume space variables */
	ddx = 1;
	ddy = (set->d[0]);
	ddz = (set->d[0])*(set->d[1]);
	hdx = (set->d[0])/2.0;
	hdy = (set->d[1])/2.0;
	hdz = (set->d[2])/2.0;

/* sample the image */
	for(y1=(-yh);y1<(yh); y1=y1+1.0) {
/* compute scan baseline point for an X scan */
		samp[0] = p[0] + (y1)*y[0] + (-xh)*x[0];
		samp[1] = p[1] + (y1)*y[1] + (-xh)*x[1];
		samp[2] = p[2] + (y1)*y[2] + (-xh)*x[2];
/* do an X scan */
		for(x1=(-xh);x1<xh;x1=x1+1.0) {
/* bump into volume coordinates */
/* NOTE: divizion of Z value by squeeze_factor  */
			s[0] = samp[0] + hdx;
			s[1] = samp[1] + hdy;
			s[2] = (samp[2]/(set->squeeze_factor)) + hdz;
/* is it in the volume */
			if (withinthevolume(s,set) == 1) {
/* get the point */
				ptr2 = s[0] + s[1]*ddy + s[2]*ddz;
/* store the data */
				image[ptr++] = data[ptr2]+256;
			} else {
				image[ptr++] = 0+256;
			}
/* bump along X vector */
			samp[0] = samp[0] + x[0];
			samp[1] = samp[1] + x[1];
			samp[2] = samp[2] + x[2];
		}
	}
}

long int withinthevolume(long int *p,vset *set)
{
	long int i;
	for (i=0; i<3; i++) {
		if ((p[i] < 0) || (p[i] >= set->d[i])) return(0);
	}
	return(1);
}

void vl_getvoxel_(vset *set,int x, int y, int z, int vol, int *val)
{
	int i;
	*val = -1;
        if ((vol < 0) || (vol > VL_MAX_VOLS)) return;
	if (!set->aux_data[vol]) return;
	i = x + y*set->d[0] + z*set->d[0]*set->d[1];
	*val = set->aux_data[vol][i];
	return;
}
void VL_GETVOXEL(vset *set,int x, int y, int z, int vol, int *val)
{
vl_getvoxel_(set,x, y, z, vol, val);
}


void vl_splane_(long int dx,long int dy,float *rots,float *dist,vset *set)
{
	double	vd1[3],vd2[3],vd3[3],vd4[3];
	long int	v1[3],v2[3],v3[3],v4[3];
	long int	i;

	double xform[3][3];
	double drots[3];
	double p[3],temp[3],n[3],y[3],x[3];
	double xh,yh;

/* copy the rotations */
	for(i=0;i<3;i++) drots[i] = rots[i];

#ifdef DEBUG
	printf("rx,ry,rz,d = %f %f %f %f\n",drots[0],drots[1],drots[2],*dist);
	printf("dx,dy = %d %d\n",dx,dy);
#endif

/* initial point is 0,0,0  (center of volume) */
	p[0] = (set->d[0])/2.0;
	p[1] = (set->d[1])/2.0;
	p[2] = (set->d[2])/2.0;
	p[2] = p[2]*(set->squeeze_factor);

/* compute matrix */
	vl_calcmatrix(drots,xform);

/* initial normal is 0,0,1 */
	temp[0] = 0.0;
	temp[1] = 0.0;
	temp[2] = 1.0;
/* mult N by matrix */
	vl_matmult(temp,n,xform);

/* intial y vector is 0,1,0 */
	temp[0] = 0.0;
	temp[1] = 1.0;
	temp[2] = 0.0;
/* mult Y by matrix */
	vl_matmult(temp,y,xform);

/* compute X by Y x N */
	x[0] = (y[1]*n[2])-(n[1]*y[2]);
	x[1] = (y[2]*n[0])-(n[2]*y[0]);
	x[2] = (y[0]*n[1])-(n[0]*y[1]);

/* compute p = p + Dn */
	p[0] = p[0] + (*dist)*n[0];
	p[1] = p[1] + (*dist)*n[1];
	p[2] = p[2] + (*dist)*n[2];

#ifdef DEBUG
	printf("X = %f %f %f\n",x[0],x[1],x[2]);
	printf("Y = %f %f %f\n",y[0],y[1],y[2]);
	printf("N = %f %f %f\n",n[0],n[1],n[2]);
	printf("P = %f %f %f\n",p[0],p[1],p[2]);
#endif

/* get the vectors */
	yh = (dy)/2.0;
	xh = (dx)/2.0;
	for(i=0; i<3; i++) v1[i] = p[i] + (-yh)*y[i] + (-xh)*x[i];
	for(i=0; i<3; i++) v2[i] = p[i] + (-yh)*y[i] + (xh)*x[i];
	for(i=0; i<3; i++) v3[i] = p[i] + (yh)*y[i] + (xh)*x[i];
	for(i=0; i<3; i++) v4[i] = p[i] + (yh)*y[i] + (-xh)*x[i];

#ifdef DEBUG
	printf("V1 = %d %d %d\n",v1[0],v1[1],v1[2]);
	printf("V2 = %d %d %d\n",v2[0],v2[1],v2[2]);
	printf("V3 = %d %d %d\n",v3[0],v3[1],v3[2]);
	printf("V4 = %d %d %d\n",v4[0],v4[1],v4[2]);
#endif

/* Since vl_dopoint does Z axis compression we must unsqueeze the point
	passed so that vl_dopoint can UNDO this */
	v1[2] = v1[2] / (set->squeeze_factor);
	v2[2] = v2[2] / (set->squeeze_factor);
	v3[2] = v3[2] / (set->squeeze_factor);
	v4[2] = v4[2] / (set->squeeze_factor);
/* xform them */
	(void) vl_dopoint(v1,vd1,set);
	(void) vl_dopoint(v2,vd2,set);
	(void) vl_dopoint(v3,vd3,set);
	(void) vl_dopoint(v4,vd4,set);

/* draw them */
	(void) bgnline();
	(void) v2d(vd1);
	(void) v2d(vd2);
	(void) v2d(vd3);
	(void) v2d(vd4);
	(void) v2d(vd1);
	(void) endline();
}
void VL_SPLANE(long int dx,long int dy,float *rots,float *dist,vset *set)
{
vl_splane_(dx,dy,rots,dist,set);
}

void vl_resample_axis_drvr_(long int dx, long int dy,short *image, float *x,
		float *y,float *n,float *p,vset *set,float *ratio,
		unsigned char *data);

void vl_resample_axis_(long int dx, long int dy,short *image, float *x,
		 float *y,float *n,float *p,vset *set,float *ratio)
{
	vl_resample_axis_drvr_(dx,dy,image,x,y,n,p,set,ratio,set->data);
}
void VL_RESAMPLE_AXIS(long int dx, long int dy,short *image, float *x,
		 float *y,float *n,float *p,vset *set,float *ratio)
{
vl_resample_axis_(dx,dy,image,x,y,n,p,set,ratio);
}

void vl_resample_axis16_(long int dx, long int dy,short *image, float *x,
		 float *y,float *n,float *p,vset *set,float *ratio)
{
	if (set->aux_data[1] == 0l) return;

	vl_resample_axis_drvr_(dx,dy,image,x,y,n,p,set,ratio,set->aux_data[1]);

	return;
}
void VL_RESAMPLE_AXIS16(long int dx, long int dy,short *image, float *x,
		 float *y,float *n,float *p,vset *set,float *ratio)
{
vl_resample_axis16_(dx,dy,image,x,y,n,p,set,ratio);
}

void vl_resample_axis_drvr_(long int dx, long int dy,short *image, float *x,
		float *y,float *n,float *p,vset *set,float *ratio,
		unsigned char *data)
{
	double temp[3];
	double xh,yh,x1,y1;
	long int ptr,ptr2,s[3];
	long int ddx,ddy,ddz,i;
	double hdx,hdy,hdz;
	double samp[3],xt[3],yt[3];

#ifdef DEBUG
	printf("dx,dy = %d %d\n",dx,dy);
	printf("xv = %f %f %f\n",x[0],x[1],x[2]);
	printf("yv = %f %f %f\n",y[0],y[1],y[2]);
	printf("nv = %f %f %f\n",n[0],n[1],n[2]);
	printf("pv = %f %f %f\n",p[0],p[1],p[2]);
	printf("ratio = %f\n",*ratio);
	printf("image[0] = %d\n",image[0]);
	printf("set->d[0] = %d\n",set->d[0]);
#endif

/* image space variables */
	ptr = 0;
	yh = (dy)/2.0;
	xh = (dx)/2.0;
/* volume space variables */
	ddx = 1;
	ddy = (set->d[0]);
	ddz = (set->d[0])*(set->d[1]);
	hdx = (set->d[0])/2.0;
	hdy = (set->d[1])/2.0;
	hdz = (set->d[2])/2.0;
/* get the (possibly compressed or expanded) X,Y plane vectors */
	for(i=0;i<3;i++) {
		xt[i] = x[i] * (*ratio);
		yt[i] = y[i] * (*ratio);
	}

/* sample the image */
	for(y1=(-yh);y1<(yh); y1=y1+1.0) {
/* compute scan baseline point for an X scan */
		samp[0] = p[0] + (y1)*yt[0] + (-xh)*xt[0];
		samp[1] = p[1] + (y1)*yt[1] + (-xh)*xt[1];
		samp[2] = p[2] + (y1)*yt[2] + (-xh)*xt[2];
/* do an X scan */
		for(x1=(-xh);x1<xh;x1=x1+1.0) {
/* bump into volume coordinates */
/* NOTE: divizion of Z value by squeeze_factor */
			s[0] = samp[0] + hdx + 0.49;
			s[1] = samp[1] + hdy + 0.49;
			s[2] = (samp[2]/(set->squeeze_factor)) + hdz + 0.49;
/* is it in the volume */
			if (withinthevolume(s,set) == 1) {
/* get the point */
				ptr2 = s[0] + s[1]*ddy + s[2]*ddz;
/* store the data */
				image[ptr++] = data[ptr2]+256;
			} else {
				image[ptr++] = 0+256;
			}
/* bump along X vector */
			samp[0] = samp[0] + xt[0];
			samp[1] = samp[1] + xt[1];
			samp[2] = samp[2] + xt[2];
		}
	}
}

void vl_mpfit_uplane_(long int dx,long int dy,float *x,float *y,float *n,
	float *p,vset *set,long int solid)
{
	double	vd1[3],vd2[3],vd3[3],vd4[3];
	long int	v1[3],v2[3],v3[3],v4[3];
	long int	i;
	double	hd[3];
	double xh,yh;

/* center of volume */
	hd[0] = (set->d[0])/2.0;
	hd[1] = (set->d[1])/2.0;
	hd[2] = ((set->d[2])/2.0)*(set->squeeze_factor);

/* get the vectors */
	yh = (dy)/2.0;
	xh = (dx)/2.0;
	for(i=0; i<3; i++) v1[i] = p[i] + (-yh)*y[i] + (-xh)*x[i] + hd[i];
	for(i=0; i<3; i++) v2[i] = p[i] + (-yh)*y[i] + (xh)*x[i] + hd[i];
	for(i=0; i<3; i++) v3[i] = p[i] + (yh)*y[i] + (xh)*x[i] + hd[i];
	for(i=0; i<3; i++) v4[i] = p[i] + (yh)*y[i] + (-xh)*x[i] + hd[i];

/* Since vl_dopoint does Z axis compression we must unsqueeze the point
	passed so that vl_dopoint can UNDO this */
	v1[2] = v1[2] / (set->squeeze_factor);
	v2[2] = v2[2] / (set->squeeze_factor);
	v3[2] = v3[2] / (set->squeeze_factor);
	v4[2] = v4[2] / (set->squeeze_factor);

/* xform them */
	(void) vl_dopoint(v1,vd1,set);
	(void) vl_dopoint(v2,vd2,set);
	(void) vl_dopoint(v3,vd3,set);
	(void) vl_dopoint(v4,vd4,set);

/* draw the central plane */
	if (solid) {
		(void) bgnpolygon();
		(void) V2D(vd1);
		(void) V2D(vd2);
		(void) V2D(vd3);
		(void) V2D(vd4);
		(void) endpolygon();
	} else {
		(void) bgnline();
		(void) V2D(vd1);
		(void) V2D(vd2);
		(void) V2D(vd3);
		(void) V2D(vd4);
		(void) V2D(vd1);
		(void) endline();
	}
	return;
}
void VL_MPFIT_UPLANE(long int dx,long int dy,float *x,float *y,float *n,
	float *p,vset *set,long int solid)
{
vl_mpfit_uplane_(dx,dy,x,y,n,p,set,solid);
}

void vl_mpfit_fplane_(float *points,vset *set)
{
	double	vd1[3],vd2[3],vd3[3],vd4[3];
	long int	v1[3],v2[3],v3[3],v4[3];
	long int	i;
	double	hd[3];

/* center of volume */
	hd[0] = (set->d[0])/2.0;
	hd[1] = (set->d[1])/2.0;
	hd[2] = ((set->d[2])/2.0)*(set->squeeze_factor); 

/* draw the fixed plane */
	for(i=0;i<3;i++) v1[i] = points[i] + hd[i];
	for(i=0;i<3;i++) v2[i] = points[i+3] + hd[i];
	for(i=0;i<3;i++) v3[i] = points[i+6] + hd[i];
	for(i=0;i<3;i++) v4[i] = points[i+9] + hd[i];

/* Since vl_dopoint does Z axis compression we must unsqueeze the point
	passed so that vl_dopoint can UNDO this */
	v1[2] = v1[2] / (set->squeeze_factor);
	v2[2] = v2[2] / (set->squeeze_factor);
	v3[2] = v3[2] / (set->squeeze_factor);
	v4[2] = v4[2] / (set->squeeze_factor);

/* xform them */
	(void) vl_dopoint(v1,vd1,set);
	(void) vl_dopoint(v2,vd2,set);
	(void) vl_dopoint(v3,vd3,set);
	(void) vl_dopoint(v4,vd4,set);

/* draw it */
	(void) bgnline();
	(void) V2D(vd1);
	(void) V2D(vd2);
	(void) V2D(vd4);
	(void) V2D(vd3);
	(void) V2D(vd1);
	(void) endline();
}
void VL_MPFIT_FPLANE(float *points,vset *set)
{
vl_mpfit_fplane_(points,set);
}

void vl_draw_pset_line_(long int *points,long int *n,vset *set)
{
	double          vd1[3],vd2[3];
	long int	i;
	
	for(i=0;i<(*n);i++) {
/* xform them */
		(void) vl_dopoint(&(points[(i*6)+0]),vd1,set);
		(void) vl_dopoint(&(points[(i*6)+3]),vd2,set);

/* draw it */
		(void) bgnline();
		(void) V2D(vd1);
		(void) V2D(vd2);
		(void) endline();
	}

	return;
}
void VL_DRAW_PSET_LINE(long int *points,long int *n,vset *set)
{
vl_draw_pset_line_(points,n,set);
}

void vl_draw_pset_poly_(long int *points,long int *n,vset *set)
{
	double          vd1[3];
	long int	i;
	
	(void) bgnpolygon();
	for(i=0;i<(*n);i++) {
/* xform vert */
		(void) vl_dopoint(&(points[i*3]),vd1,set);
/* draw it */
		(void) V2D(vd1);
	}
	(void) endpolygon();

	return;
}
void VL_DRAW_PSET_POLY(long int *points,long int *n,vset *set)
{
vl_draw_pset_poly_(points,n,set);
}


void vl_draw_tal_plane_(long int *points,long int *n,long int *n2,vset *set)
{
	double		vd1[3],vd2[3],d0[3],d1[3];
	long int	vi1[3],vi2[3],i,j;

/* if n ==0 then a single line will do */
	if ((*n) == 0) {
		setlinestyle(0);
/* xform them */
		(void) vl_dopoint(&(points[0]),vd1,set);
		(void) vl_dopoint(&(points[3]),vd2,set);

/* draw it */
		(void) bgnline();
		(void) V2D(vd1);
		(void) V2D(vd2);
		(void) endline();
	} else {
		setlinestyle(0);
/* xform them */
		(void) vl_dopoint(&(points[0]),vd1,set);
		(void) vl_dopoint(&(points[3]),vd2,set);

/* draw it */
		(void) bgnline();
		(void) V2D(vd1);
		(void) V2D(vd2);
		(void) endline();
/* xform them */
		(void) vl_dopoint(&(points[6]),vd1,set);
		(void) vl_dopoint(&(points[9]),vd2,set);

/* draw it */
		(void) bgnline();
		(void) V2D(vd1);
		(void) V2D(vd2);
		(void) endline();
/* xform them */
		(void) vl_dopoint(&(points[0]),vd1,set);
		(void) vl_dopoint(&(points[6]),vd2,set);

/* draw it */
		(void) bgnline();
		(void) V2D(vd1);
		(void) V2D(vd2);
		(void) endline();
/* xform them */
		(void) vl_dopoint(&(points[3]),vd1,set);
		(void) vl_dopoint(&(points[9]),vd2,set);

/* draw it */
		(void) bgnline();
		(void) V2D(vd1);
		(void) V2D(vd2);
		(void) endline();
/* dashed lines */
		setlinestyle(2768);
/* grid in parallel */
		for(i=0;i<3;i++) {
			d0[0+i]=(double)(points[6+i]-points[0+i])/(double)(*n);
			d1[0+i]=(double)(points[9+i]-points[3+i])/(double)(*n);
		}
		for(i=1;i<(*n);i++) {
			for(j=0;j<3;j++) vi1[j]=points[0+j] + d0[j]*(double)(i);
			for(j=0;j<3;j++) vi2[j]=points[3+j] + d1[j]*(double)(i);
/* xform them */
			(void) vl_dopoint(vi1,vd1,set);
			(void) vl_dopoint(vi2,vd2,set);

/* draw it */
			(void) bgnline();
			(void) V2D(vd1);
			(void) V2D(vd2);
			(void) endline();
		}
		setlinestyle(0);
/* grid in perpendicular */
		if ((*n2) < 2) return;
/* dashed lines */
		setlinestyle(2768);
		for(i=0;i<3;i++) {
			d0[0+i]=(double)(points[3+i]-points[0+i])/(double)(*n2);
			d1[0+i]=(double)(points[9+i]-points[6+i])/(double)(*n2);
		}
		for(i=1;i<(*n2);i++) {
			for(j=0;j<3;j++) vi1[j]=points[0+j] + d0[j]*(double)(i);
			for(j=0;j<3;j++) vi2[j]=points[6+j] + d1[j]*(double)(i);
/* xform them */
			(void) vl_dopoint(vi1,vd1,set);
			(void) vl_dopoint(vi2,vd2,set);

/* draw it */
			(void) bgnline();
			(void) V2D(vd1);
			(void) V2D(vd2);
			(void) endline();
		}
		setlinestyle(0);
	}
	return;
}
void VL_DRAW_TAL_PLANE(long int *points,long int *n,long int *n2,vset *set)
{
vl_draw_tal_plane_(points,n,n2,set);
}

void vl_draw_tal_cutplane_(long int *points,long int type,long int *offset,
	long int zoom)
{
	double		vd1[3],vd2[3],vout[12],vv[3],q;
	long int	i,j,k,iz;
	double		hd[3],G,t,temp[3];

/* get Q for the plane equation */
	q = 0.0;
	for(i=0;i<3;i++) q += (held_data.p[i]*held_data.n[i]);
/* get dataset params */
	for(i=0;i<3;i++) hd[i] = (held_data.theset->d[i])/2.0;

/* clean up 'N' */
        for(i=0;i<3;i++) if (fabs(held_data.n[i]) < 0.0001) held_data.n[i]=0.0;

/* get the first point (wraparound) */
	vd2[0] =(double)(points[9] - hd[0]);
	vd2[1] =(double)(points[10] - hd[1]);
	vd2[2] =((double)(points[11]-hd[2]))*(held_data.theset->squeeze_factor);

	k = 0; /* accomulates output */
	j = 0; /* points to input */
	for(i=0;i<4;i++) {

/* get one of the vectors */
		vd1[0] = vd2[0];  vd1[1] = vd2[1];  vd1[2] = vd2[2];
		vd2[0] =(double)(points[j+0] - hd[0]);
		vd2[1] =(double)(points[j+1] - hd[1]);
		vd2[2] =((double)(points[j+2] - hd[2]))*
				(held_data.theset->squeeze_factor);

/* intersect vd1-vd2 with cutting plane */
/* get the vector */
		vv[0] = (vd2[0] - vd1[0]);
		vv[1] = (vd2[1] - vd1[1]);
		vv[2] = (vd2[2] - vd1[2]);

/* compute demoninator of intersection equation */
		G = (held_data.n[0]*vv[0]);
		G += (held_data.n[1]*vv[1]);
		G += (held_data.n[2]*vv[2]);
#ifdef DEBUG
printf("RJF= %f %f %f G= %f\n",held_data.n[0],held_data.n[1],held_data.n[2],G);
#endif

/* if not coplanar */
		if (fabs(G) > 0.0001) {
			t = (q-((held_data.n[0]*vd1[0])+
			(held_data.n[1]*vd1[1])+(held_data.n[2]*vd1[2])))/G;
/* if valid T get the coordinates */
#ifdef DEBUG
			printf("T=%lf\n",t);
#endif
			if ((t >= 0.0) && (t <= 1.0)) {
/* get the 3D coords (pixel space) */
				vout[k+0] = vd1[0]+ (t*vv[0]);
				vout[k+1] = vd1[1]+ (t*vv[1]);
				vout[k+2] = vd1[2]+ (t*vv[2]);
/* vector from P to the point */
				temp[0] = vout[k+0] - held_data.p[0];
				temp[1] = vout[k+1] - held_data.p[1];
				temp[2] = vout[k+2] - held_data.p[2];
#ifdef DEBUG
				printf("Vout: %lf %lf %lf   temp: %lf %lf %lf\n",vout[k+0],vout[k+1],vout[k+2],temp[0],temp[1],temp[2]);
#endif
/* project onto cutting plane */
				vout[k+0] = (temp[0]*held_data.x[0]) +
						(temp[1]*held_data.x[1]) +
						(temp[2]*held_data.x[2]);

				vout[k+1] = (temp[0]*held_data.y[0]) +
						(temp[1]*held_data.y[1]) +
						(temp[2]*held_data.y[2]);

				vout[k+2] = (temp[0]*held_data.n[0]) +
						(temp[1]*held_data.n[1]) +
						(temp[2]*held_data.n[2]);
/* correct to 0-imagesize realm */
				vout[k+0] += (held_data.imdx/2.0);
				vout[k+1] += (held_data.imdy/2.0);
#ifdef DEBUG
				printf("Vout++: %d %lf %lf %lf\n",k,vout[k+0],vout[k+1],vout[k+2]);
#endif
				k = k + 3;
			}
		}
		j = j + 3;
	}
/* draw the intersection line (if any) */
	if (type) {
		setlinestyle(0);
	} else {
		setlinestyle(2768);
	}
	(void) bgnline();
	for(i=0;i<k;i=i+3) {
		vout[i] = (vout[i]-(double)(offset[0]))*(double)(zoom);
		vout[i+1] = (vout[i+1]-(double)(offset[1]))*(double)(zoom);
#ifdef DEBUG
		printf("Drawing: %lf %lf\n",vout[i],vout[i+1]);
#endif
		(void) v2d(&(vout[i]));
	}
	(void) endline();
	setlinestyle(0);

	return;
}
void VL_DRAW_TAL_CUTPLANE(long int *points,long int type,long int *offset,
	long int zoom)
{
vl_draw_tal_cutplane_(points,type,offset,zoom);
}

void vl_mpfit_line_(float *pnt1,float *pnt2,vset *set)
{
	double		vd1[3],vd2[3];
	long int	v1[3],v2[3];
	long int	i;
	double		hd[3];

/* center of volume */
	hd[0] = (set->d[0])/2.0;
	hd[1] = (set->d[1])/2.0;
	hd[2] = ((set->d[2])/2.0)*(set->squeeze_factor); 

/* draw the fixed plane */
	for(i=0;i<3;i++) v1[i] = pnt1[i] + hd[i];
	for(i=0;i<3;i++) v2[i] = pnt2[i] + hd[i];

/* Since vl_dopoint does Z axis compression we must unsqueeze the point
	passed so that vl_dopoint can UNDO this */
	v1[2] = v1[2] / (set->squeeze_factor);
	v2[2] = v2[2] / (set->squeeze_factor);

/* xform them */
	(void) vl_dopoint(v1,vd1,set);
	(void) vl_dopoint(v2,vd2,set);

/* draw it */
	(void) bgnline();
	(void) V2D(vd1);
	(void) V2D(vd2);
	(void) endline();
}
void VL_MPFIT_LINE(float *pnt1,float *pnt2,vset *set)
{
vl_mpfit_line_(pnt1,pnt2,set);
}
