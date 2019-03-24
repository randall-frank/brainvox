#include "voxel.h"
#include "vl_util.h"
#include <math.h>
#include <stdio.h>

long int withinthevolume(long int *p,vset *set);

typedef struct {
	double x[3],y[3],p[3],n[3];
	vset *theset;
	long int imdx,imdy;
} sample_common;

sample_common held_data;

void vl_get_held_data_(float *p,float *x,float *y,float *z,float *h,float *squ)
{
	long int	i;

	for(i=0;i<3;i++) {
		p[i] = held_data.p[i];
		x[i] = held_data.x[i];
		y[i] = held_data.y[i];
		z[i] = held_data.n[i];
		h[i] = (held_data.theset->d[i])/2.0;
	}
	*squ = held_data.theset->squeeze_factor;

	return;
}
void VL_GET_HELD_DATA(float *p,float *x,float *y,float *z,float *h,float *squ)
{
vl_get_held_data_(p,x,y,z,h,squ);
}

void vl_sample_vectors_(long int dx,long int dy,float *rots,float *dist,
	vset *set)
{
	double xform[3][3];
	double drots[3];
	double temp[3];
	long int i;

/* copy the rotations */
	for(i=0;i<3;i++) drots[i] = rots[i];

/* initial point is 0,0,0 */
	held_data.p[0] = 0.0;
	held_data.p[1] = 0.0;
	held_data.p[2] = 0.0;

/* compute matrix */
	vl_calcmatrix(drots,xform);

/* initial normal is 0,0,1 */
	temp[0] = 0.0;
	temp[1] = 0.0;
	temp[2] = 1.0;
/* mult N by matrix */
	vl_matmult(temp,held_data.n,xform);

/* intial y vector is 0,1,0 */
	temp[0] = 0.0;
	temp[1] = 1.0;
	temp[2] = 0.0;
/* mult Y by matrix */
	vl_matmult(temp,held_data.y,xform);

/* compute X by Y x N */
	held_data.x[0] = (held_data.y[1]*held_data.n[2])
			-(held_data.n[1]*held_data.y[2]);
	held_data.x[1] = (held_data.y[2]*held_data.n[0])
			-(held_data.n[2]*held_data.y[0]);
	held_data.x[2] = (held_data.y[0]*held_data.n[1])
			-(held_data.n[0]*held_data.y[1]);

/* compute p = p + Dn */
	held_data.p[0] = held_data.p[0] + (*dist)*held_data.n[0];
	held_data.p[1] = held_data.p[1] + (*dist)*held_data.n[1];
	held_data.p[2] = held_data.p[2] + (*dist)*held_data.n[2];

/* get the other data */
	held_data.theset = set;
	held_data.imdx = dx;
	held_data.imdy = dy;

#ifdef DEBUG
	printf("Sample vectors Held: %f %f %f \n %f %f %f\n %f %f %f \n %f %f %f\n",
		held_data.x[0],held_data.x[1],held_data.x[2],
		held_data.y[0],held_data.y[1],held_data.y[2],
		held_data.p[0],held_data.p[1],held_data.p[2],
		held_data.n[0],held_data.n[1],held_data.n[2]);
	printf("imdx = %ld imdy = %ld\n",held_data.imdx,held_data.imdy);
#endif
/* done */
	return;
}
void VL_SAMPLE_VECTORS(long int dx,long int dy,float *rots,float *dist,
	vset *set)
{
vl_sample_vectors_(dx,dy,rots,dist,set);
}

void vl_mpsample_vectors_(long int dx,long int dy,float *x,float *y,float *n,
	float *p,vset *set)
{
	long int i;

/* initial point is passed */
	for(i=0;i<3;i++) {
		held_data.p[i] = p[i];
		held_data.x[i] = x[i];
		held_data.y[i] = y[i];
		held_data.n[i] = n[i];
	}
/*
	held_data.p[2] = held_data.p[2] / (set->squeeze_factor);
*/

/* get the other data */
	held_data.theset = set;
	held_data.imdx = dx;
	held_data.imdy = dy;

/* done */
	return;
}
void VL_MPSAMPLE_VECTORS(long int dx,long int dy,float *x,float *y,float *n,
	float *p,vset *set)
{
vl_mpsample_vectors_(dx,dy,x,y,n,p,set);
}

void vl_3dtocutplane_(long int *point,long int *out,float *dist,long int *error)
{
	double temp[3],cdist,fpt[3];
	double hdx,hdy,hdz;

	*error = 1;
/* volume space variables */
	hdx = (held_data.theset->d[0])/2.0;
	hdy = (held_data.theset->d[1])/2.0;
	hdz = (held_data.theset->d[2])/2.0;
	fpt[0] = (double)(point[0]-hdx);
	fpt[1] = (double)(point[1]-hdy);
	fpt[2] = ((double)(point[2]-hdz))*(held_data.theset->squeeze_factor);
/* compute distance from point to plane */
	temp[0] = fpt[0] - held_data.p[0];
	temp[1] = fpt[1] - held_data.p[1];
	temp[2] = fpt[2] - held_data.p[2];
/* temp dot N */
	cdist = temp[0]*held_data.n[0] +
			temp[1]*held_data.n[1] + temp[2]*held_data.n[2];
#ifdef DEBUG
	printf("vl_3dtocutplane : Input : %d %d %d \n",
			point[0],point[1],point[2]);
	printf("Cdist = %lf dist = %f\n",cdist,*dist);
#endif
/* if the distance exceeds dist then error out */
	if (fabs(cdist) > (*dist)) return;

/* compute the point on the plane */
/* convert this volume point to a 2d point on the cutting plane */
/* by computing the projection of the temp vector onto the x and y vectors */
	fpt[0] = temp[0]*held_data.x[0] +
			temp[1]*held_data.x[1] + temp[2]*held_data.x[2];
	fpt[1] = temp[0]*held_data.y[0] +
			temp[1]*held_data.y[1] + temp[2]*held_data.y[2];
	fpt[2] = cdist;

	out[0] = fpt[0] + (held_data.imdx/2.0) + 0.49;
	out[1] = fpt[1] + (held_data.imdy/2.0) + 0.49;
	out[2] = fpt[2] + 0.49;
#ifdef DEBUG
	printf("Output Point : %d %d %d\n",out[0],out[1],out[2]);
#endif

/* if the point is within bounds then return positive */
	if ((out[0] >=0) && (out[0] < held_data.imdx) &&
		(out[1] >=0) && (out[1] < held_data.imdy)) {
		*error = 0;
	}
	return;
}
void VL_3DTOCUTPLANE(long int *point,long int *out,float *dist,long int *error)
{
vl_3dtocutplane_(point,out,dist,error);
}

void vl_cutplaneto3d_(long int *point,long int *out,long int *error)
{
	long int i;
	double hdx,hdy,hdz;
	double samp[3];
	double xh,yh,x1,y1;
	long int s[3];

#ifdef DEBUG
	printf("cutplaneto3d Held: %f %f %f \n %f %f %f\n %f %f %f \n",
		held_data.x[0],held_data.x[1],held_data.x[2],
		held_data.y[0],held_data.y[1],held_data.y[2],
		held_data.p[0],held_data.p[1],held_data.p[2]);
#endif

/* image space stuff */
	xh = (held_data.imdx)/2.0;
	yh = (held_data.imdy)/2.0;
/* volume space variables */
	hdx = (held_data.theset->d[0])/2.0;
	hdy = (held_data.theset->d[1])/2.0;
	hdz = (held_data.theset->d[2])/2.0;
/* convert point to image centered coords */
	x1 = point[0] - xh;
	y1 = point[1] - yh;
#ifdef DEBUG
	printf("xh,yh,hdx,hdy,hdz,x1,y1= %lf %lf %lf %lf %lf\n",xh,yh,hdx,hdy,hdz,x1,y1);
#endif
/* get the 3d point */
	samp[0] = held_data.p[0] + (y1)*held_data.y[0] + (x1)*held_data.x[0];
	samp[1] = held_data.p[1] + (y1)*held_data.y[1] + (x1)*held_data.x[1];
	samp[2] = held_data.p[2] + (y1)*held_data.y[2] + (x1)*held_data.x[2];
	s[0] = samp[0] + hdx + 0.49;
	s[1] = samp[1] + hdy + 0.49;
	s[2] = (samp[2]/(held_data.theset->squeeze_factor)) + hdz + 0.49;
#ifdef DEBUG
	printf("Samp=%lf %lf %lf S=%ld %ld %ld  squ=%f\n",
		 samp[0],samp[1],samp[2],s[0],s[1],s[2],(held_data.theset->squeeze_factor));
#endif
/* is it in the volume */
	if (withinthevolume(s,held_data.theset) == 1) {
		*error = 0;
		out[0] = s[0];
		out[1] = s[1];
		out[2] = s[2];
	} else {
		*error = 1;
	}
}
void VL_CUTPLANETO3D(long int *point,long int *out,long int *error)
{
vl_cutplaneto3d_(point,out,error);
}

void vl_3dpttomrpet_(long int dx,long int dy,long int dz,float *x,float *y,
	float *n,float *p,vset *set,float *ratio,float *mp_squ,long *in,
	long *out,long *error)
{
        double temp[3],fpt[3],xt[3],yt[3],nt[3];
        double hdx,hdy,hdz;
        int i;

#ifdef DEBUG
	printf("vl_3dpttomrpet : ratio=%f  pet_squ=%f  mri_squ=%f\n",
		(*ratio),(*mp_squ),(set->squeeze_factor));
#endif

        *error = 1;
/* volume space variables */
        hdx = (set->d[0])/2.0;
        hdy = (set->d[1])/2.0;
        hdz = (set->d[2])/2.0;
        fpt[0] = (double)(in[0]-hdx);
        fpt[1] = (double)(in[1]-hdy);
        fpt[2] = ((double)(in[2]-hdz))*(set->squeeze_factor);
/* compute vector from point to mp-fit centroid */
        temp[0] = fpt[0] - p[0];
        temp[1] = fpt[1] - p[1];
        temp[2] = fpt[2] - p[2];
/* MP_FIT normals are scaled versions of the up_* normals */
	for(i=0;i<3;i++) {
		xt[i] = x[i] / (*ratio);
		yt[i] = y[i] / (*ratio);
		nt[i] = n[i];  /* this SHOULD be / (*ratio) */
/* It is NOT currently because Brainvox distorts the slices a bit when
	computing the squeeze factor.  */      
	}
/* project (dot product) vector onto MP_FIT normals */
	fpt[0] = temp[0]*xt[0] + temp[1]*xt[1] + temp[2]*xt[2];
	fpt[1] = temp[0]*yt[0] + temp[1]*yt[1] + temp[2]*yt[2];
	fpt[2] = temp[0]*nt[0] + temp[1]*nt[1] + temp[2]*nt[2];
/* add MP_FIT centroids */
	out[0] = fpt[0] + (dx/2.0) + 0.49;
	out[1] = fpt[1] + (dy/2.0) + 0.49;
	out[2] = -(fpt[2]/(*mp_squ)) + 0.49;
/* if the point is within bounds then return positive */
	if ((out[0] >=0) && (out[0] < dx) &&
		(out[2] >=0) && (out[2] < dz) &&
		(out[1] >=0) && (out[1] < dy)) {
		*error = 0;
	}
	return;
}
void VL_3DPTTOMRPET(long int dx,long int dy,long int dz,float *x,float *y,
	float *n,float *p,vset *set,float *ratio,float *mp_squ,long *in,
	long *out,long *error)
{
vl_3dpttomrpet_(dx,dy,dz,x,y,n,p,set,ratio,mp_squ,in,out,error);
}

void vl_3dpttomrpetcut_(long int dx,long int dy,float *x,float *y,float *n,
        float *p,vset *set,float *ratio,long *in,long *out,float *dist,
        long *error)
{
	double temp[3],cdist,fpt[3],xt[3],yt[3];
	double hdx,hdy,hdz;
	int i;	

	*error = 1;
/* volume space variables */
	hdx = (set->d[0])/2.0;
	hdy = (set->d[1])/2.0;
	hdz = (set->d[2])/2.0;
	fpt[0] = (double)(in[0]-hdx);
	fpt[1] = (double)(in[1]-hdy);
	fpt[2] = ((double)(in[2]-hdz))*(set->squeeze_factor);
/* compute distance from point to plane */
	temp[0] = fpt[0] - p[0];
	temp[1] = fpt[1] - p[1];
	temp[2] = fpt[2] - p[2];
/* temp dot N */
	cdist = temp[0]*n[0] + temp[1]*n[1] + temp[2]*n[2];
#ifdef DEBUG
	printf("vl_3dtocutplane : Input : %d %d %d \n",
			in[0],in[1],in[2]);
	printf("Cdist = %lf dist = %f\n",cdist,*dist);
#endif
	cdist = cdist / (*ratio);
/* if the distance exceeds dist then error out */
	if (fabs(cdist) > (*dist)) return;
/* get the (possibly compressed or expanded) X,Y plane vectors */
        for(i=0;i<3;i++) {
                xt[i] = x[i] / (*ratio);
                yt[i] = y[i] / (*ratio);
        }
/* compute the point on the plane */
/* convert this volume point to a 2d point on the cutting plane */
/* by computing the projection of the temp vector onto the x and y vectors */
	fpt[0] = temp[0]*xt[0] + temp[1]*xt[1] + temp[2]*xt[2];
	fpt[1] = temp[0]*yt[0] + temp[1]*yt[1] + temp[2]*yt[2];
	fpt[2] = cdist;
#ifdef DEBUG
	printf("Output Point : %d %d %d\n",out[0],out[1],out[2]);
#endif
	out[0] = fpt[0] + (dx/2.0) + 0.49;
	out[1] = fpt[1] + (dy/2.0) + 0.49;
	out[2] = fpt[2] + 0.49;

/* if the point is within bounds then return positive */
	if ((out[0] >=0) && (out[0] < dx) &&
		(out[1] >=0) && (out[1] < dy)) {
		*error = 0;
	}
}
void VL_3DPTTOMRPETCUT(long int dx,long int dy,float *x,float *y,float *n,
        float *p,vset *set,float *ratio,long *in,long *out,float *dist,
        long *error)
{
vl_3dpttomrpetcut_(dx,dy,x,y,n, p,set,ratio,in,out,dist,error);
}
