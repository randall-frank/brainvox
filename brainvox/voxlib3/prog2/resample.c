#include "../voxel.h"
#include "../vl_util.h"
#include <stdio.h>

long int withinthevolume(long int *p,vset *set);

void vl_resample_(int dx,int dy,char *image,float *rots,float *dist,vset *set)
{
	double xform[3][3];
	double drots[3];
	double p[3],temp[3],n[3],y[3],x[3];
	int xh,yh,x1,y1;
	long int ptr,ptr2,s[3];
	int ddx,ddy,ddz,hdx,hdy,hdz,i;
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
	for(y1=(-yh);y1<yh;y1++) {
/* compute scan baseline point for an X scan */
		samp[0] = p[0] + (y1)*y[0] + (-xh)*x[0];
		samp[1] = p[1] + (y1)*y[1] + (-xh)*x[1];
		samp[2] = p[2] + (y1)*y[2] + (-xh)*x[2];
/* do an X scan */
		for(x1=(-xh);x1<xh;x1++) {
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
				image[ptr++] = set->data[ptr2];
			} else {
				image[ptr++] = 0;
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
	int i;
	for (i=0; i<3; i++) {
		if ((p[i] < 0) || (p[i] >= set->d[i])) return(0);
	}
	return(1);
}


void vl_splane_(int dx,int dy,float *rots,float *dist,vset *set)
{
	double	vd1[3],vd2[3],vd3[3],vd4[3];
	long int	v1[3],v2[3],v3[3],v4[3];
	int	i;

	double xform[3][3];
	double drots[3];
	double p[3],temp[3],n[3],y[3],x[3];
	int xh,yh;

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

/* get the vectors */
	yh = (dy)/2.0;
	xh = (dx)/2.0;
	for(i=0; i<3; i++) v1[i] = p[i] + (-yh)*y[i] + (-xh)*x[i];
	for(i=0; i<3; i++) v2[i] = p[i] + (-yh)*y[i] + (xh)*x[i];
	for(i=0; i<3; i++) v3[i] = p[i] + (yh)*y[i] + (xh)*x[i];
	for(i=0; i<3; i++) v4[i] = p[i] + (yh)*y[i] + (-xh)*x[i];

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

