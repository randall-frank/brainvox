#include "voxel.h"
#include "vl_util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef MALLOC
#undef MALLOC
#endif
#ifdef FREE
#undef FREE
#endif

#define MALLOC malloc
#define FREE free

double vb_closest_plane(double *verts,long int nverts,double *p,double *n,
	double *X,double *Y,double *area);

int vb_find_rots_(double xform[3][3],double *rots);

static double local_dot(double *x,double *y);
static void local_cross(double *x,double *y,double *z);
static void local_norm(double *n);

double vb_closest_plane(double *verts,long int nverts,double *p,double *n,
	double *X,double *Y,double *area)
{
	double		err = -1.0; /* assume error */
	double		*u,*v,d;
	double		t[3];
	long int 	i,j;
	
	*area = 0.0;
/* must be at least three points */	
	if (nverts < 3) return(err);	
/* zero the accumulators */	
	p[0] = 0.0; p[1] = 0.0; p[2] = 0.0;
	n[0] = 0.0; n[1] = 0.0; n[2] = 0.0;
/* init the vects */
	X[0] = 1.0; X[1] = 0.0; X[2] = 0.0;
	Y[0] = 0.0; Y[1] = 1.0; Y[2] = 0.0;
/* use Newell's method to compute the average normal */	
	for(i=0;i<nverts;i++) {
		j = i + 1;
		if (j >= nverts) j -= nverts;
		u = &(verts[i*3]);
		v = &(verts[j*3]);
		n[0] += ((u[1] - v[1]) * (u[2] + v[2]));
		n[1] += ((u[2] - v[2]) * (u[0] + v[0]));
		n[2] += ((u[0] - v[0]) * (u[1] + v[1]));
/* 	accumulate the verts themselves for the average point on the plane */
		p[0] += u[0];
		p[1] += u[1];
		p[2] += u[2];
	}
/* normalize the normal */
	local_norm(n);
/* finish the average point computation */
	p[0] = p[0] / (double)(nverts);
	p[1] = p[1] / (double)(nverts);
	p[2] = p[2] / (double)(nverts);
/* compute the average error of planar fit */
	err = 0.0;
	for(i=0;i<nverts;i++) {
/* use distance from point (u) to the plane (p&n). (vector p->u dot normal)*/
		u = &(verts[i*3]);
		t[0] = u[0] - p[0];
		t[1] = u[1] - p[1];
		t[2] = u[2] - p[2];
		d = local_dot(t,n);
		err += fabs(d);
/* project points onto the plane yielding X,Y,err coordinates */
/* bump point back onto plane */
		t[0] = u[0] - (d*n[0]);
		t[1] = u[1] - (d*n[1]);
		t[2] = u[2] - (d*n[2]);
/* if this is the first point in the list, make it the X direction */
		if (i == 0) {
			X[0] = t[0] - p[0];
			X[1] = t[1] - p[1];
			X[2] = t[2] - p[2];
			local_norm(X);
			local_cross(n,X,Y);   /* Z cross X = Y */
			local_norm(Y);
		}
/* final points are just (t dot X),(t dot Y),d */
		u[0] = local_dot(t,X);
		u[1] = local_dot(t,Y);
		u[2] = d;
	}
/* average abs error */
	err = err / (double)(nverts);
/* compute area of the resultant planar polygon */
	d = 0.0;
	for(i=0;i<nverts;i++) {
		j = i + 1;
		if (j >= nverts) j -= nverts;
		u = &(verts[i*3]);
		v = &(verts[j*3]);
		d += (u[0]*v[1] - v[0]*u[1]);
	}
/* area is 1/2 the abs value */
	*area = fabs(d)/2.0;
	return(err);
}

static void local_cross(double *x,double *y,double *z)
{
	z[0] = x[1]*y[2] - y[1]*x[2];
	z[1] = x[2]*y[0] - y[2]*x[0];
	z[2] = x[0]*y[1] - y[0]*x[1];
	return;
}

static double local_dot(double *x,double *y)
{
	return(x[0]*y[0] + x[1]*y[1] + x[2]*y[2]);
}

static void local_norm(double *n)
{
	double d;
	
	d = (n[0]*n[0]) + (n[1]*n[1]) + (n[2]*n[2]);
	if (d <= 0.0) d = 1.0;
	d = sqrt(d);
	n[0] = n[0] / d;
	n[1] = n[1] / d;
	n[2] = n[2] / d;
	return;
}

void match_plane_(long int *rots,float *p,float *trans)
{
	double 	xform[3][3];
	double 	drots[3];
	double	n[3],temp[3];
	long int	i;

/* copy the rotations */
	for(i=0;i<3;i++) drots[i] = rots[i];

/* compute matrix */
        vl_calcmatrix(drots,xform);

/* initial normal is 0,0,1 */
        temp[0] = 0.0;
        temp[1] = 0.0;
        temp[2] = 1.0;

/* mult N by matrix */
        vl_matmult(temp,n,xform);

/* get distance along axis via dot product */
	*trans = (n[0]*p[0])+(n[1]*p[1])+(n[2]*p[2]);

	return;
}
void MATCH_PLANE(long int *rots,float *p,float *trans)
{
match_plane_(rots,p,trans);
}

void fit_plane_(double *verts,long int nverts,
	double *error,double *params,double *ret)
{
        double  area,n[3],p[3],x[3],y[3],err,off[3],t[3],xform[3][3];
        long int i;

        area = 0.0;
        err = -1.0;
        
#ifdef DEBUG
	for(i=0;i<nverts;i++) printf("in %ld: %lf %lf %lf\n",i,
		verts[i*3],verts[i*3+1],verts[i*3+2]);
#endif
        err = vb_closest_plane(verts,nverts,p,n,x,y,&area);
        if (err >= 0.0) {
/* compute the rotations which will setup the slicing plane to match the fit */
/* params[0-2] = rots params[3] = D */
/* -D is the distance from the origin (0,0,0) to the plane ((p->orig) dot (n)) */
                t[0] = 0 - p[0];
                t[1] = 0 - p[1];
                t[2] = 0 - p[2];
                params[3] = -(t[0]*n[0] + t[1]*n[1] + t[2]*n[2]);
/* build matrix from x,y,n vectors and compute rots to make that matrix */
                for(i=0;i<3;i++) {
                        xform[i][0] = x[i];
                        xform[i][1] = y[i];
                        xform[i][2] = n[i];
                }
                vb_find_rots_(xform,params);
#ifdef DEBUG
		printf("%lf %lf %lf %lf\n",params[0],params[1],params[2],
			params[3]);
#endif
        }
errout: 
        *error = err;
	*ret = area;
        return;
}
void FIT_PLANE(double *verts,long int nverts,
	double *error,double *params,double *ret)
{
fit_plane_(verts,nverts,error,params,ret);
}

#ifndef M_PI
#define M_PI 3.14159
#endif

void    vl_calcmatrix(double *rots,double xform[3][3]);

/* routine to convert the matrix in the set into a set of rotations */

int    vb_find_rots_(double xform[3][3],double *rots)
{
        double x,y,z,temp[3][3],error;
        long i,j,k;
        
        y = -(xform[2][0]);
        if (y < -1.0) y = -1.0;
        if (y > 1.0) y = 1.0;
        
        y = asin(y);
        if (fabs(cos(y)) < 0.00001) {  /* if the cos is VERY SMALL */
/* AMBIGUITIES EXIST HERE!!!  we ARBITRARILY set z to 0 */
                z = 0.0;
                x = atan2(-(xform[1][2]),(xform[1][1]));
        } else {
                x = atan2(((xform[2][1])/cos(y)),
                        ((xform[2][2])/cos(y)));
                z = atan2(((xform[1][0])/cos(y)),
                        ((xform[0][0])/cos(y)));
        }

/* put the angles in degrees */
        x = (x*180.0)/M_PI;
        y = (y*180.0)/M_PI;
        z = (z*180.0)/M_PI;

/* set their rotations (the matrix should remain unchanged) */
        rots[0] = x;
        rots[1] = y;
        rots[2] = z;

/* check the calculations */
        (void) vl_calcmatrix(rots,temp);
        error = 0.0;
        for(j=0;j<3;j++) {
            for(k=0;k<3;k++) {
                error = error + fabs(temp[j][k] - (xform[j][k]));
            }
        }
        if (error > 0.001) {
/* make them match */
		vl_calcmatrix(rots,xform);
/* printf("Warning: matrix consistancy error detected.\n"); */
	}
#ifdef DEBUG
        if (error > 0.001) {
                printf("Internal error:%lf %lf %lf = %lf\n",x,y,z,error);
#define ZZZZ xform
        printf("Input Matrix:\n");
        printf(" %fl %fl %fl \n",ZZZZ[0][0],ZZZZ[0][1],ZZZZ[0][2]);
        printf(" %fl %fl %fl \n",ZZZZ[1][0],ZZZZ[1][1],ZZZZ[1][2]);
        printf(" %fl %fl %fl \n",ZZZZ[2][0],ZZZZ[2][1],ZZZZ[2][2]);
        printf("Output Matrix:\n");
        printf(" %fl %fl %fl \n",temp[0][0],temp[0][1],temp[0][2]);
        printf(" %fl %fl %fl \n",temp[1][0],temp[1][1],temp[1][2]);
        printf(" %fl %fl %fl \n",temp[2][0],temp[2][1],temp[2][2]);
        }
#endif
        while (rots[0] >= 360.0) rots[0] -= 360.0;
        while (rots[0] < 0.0) rots[0] += 360.0;
        while (rots[1] >= 360.0) rots[1] -= 360.0;
        while (rots[1] < 0.0) rots[1] += 360.0;
        while (rots[2] >= 360.0) rots[2] -= 360.0;
        while (rots[2] < 0.0) rots[2] += 360.0;
        
        return(error > 0.001);
}

int    VB_FIND_ROTS(double xform[3][3],double *rots)
{
vb_find_rots_( xform,rots);
}

