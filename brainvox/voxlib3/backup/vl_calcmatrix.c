#include "voxel.h"
#include "vl_util.h"
#include <stdio.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159
#endif

/* matrix calculation routines */

void	vl_calcmatrix(double *rots,double xform[3][3])
{
	long int i,j;
	double b1,b2,b3;
	double mat[3][3];

/* convert degress to radians */
	b1 = ((rots[0])/180.0)*M_PI;
	b2 = ((rots[1])/180.0)*M_PI;
	b3 = ((rots[2])/180.0)*M_PI;
/* calculate the matrix */
	mat[0][0] = cos(b3)*cos(b2);
	mat[0][1] = (-1.0*sin(b3)*cos(b1))+(sin(b1)*sin(b2)*cos(b3));
	mat[0][2] = (sin(b3)*sin(b1))+(sin(b2)*cos(b3)*cos(b1));
	mat[1][0] = cos(b2)*sin(b3);
	mat[1][1] = (cos(b1)*cos(b3))+(sin(b1)*sin(b3)*sin(b2));
	mat[1][2] = (-1.0*cos(b3)*sin(b1))+(cos(b1)*sin(b2)*sin(b3));
	mat[2][0] = -1.0*sin(b2);
	mat[2][1] = cos(b2)*sin(b1);
	mat[2][2] = cos(b1)*cos(b2);
/* copy it into the array */
	for(i=0;i<3;i++) {
		for(j=0;j<3;j++) {
			xform[i][j] = mat[i][j];
		}
	}
}

void vl_matmult(double *in,double *out,double mat[3][3])
{
	out[0] = (in[0]*mat[0][0] + in[1]*mat[0][1] +
		 in[2]*mat[0][2]);
	out[1] = (in[0]*mat[1][0] + in[1]*mat[1][1] +
		 in[2]*mat[1][2]);
	out[2] = (in[0]*mat[2][0] + in[1]*mat[2][1] +
		 in[2]*mat[2][2]);
}

void vl_imatmult(long int *in,double *out,double mat[3][3])
{
	out[0] = (in[0]*mat[0][0] + in[1]*mat[0][1] +
		 in[2]*mat[0][2]);
	out[1] = (in[0]*mat[1][0] + in[1]*mat[1][1] +
		 in[2]*mat[1][2]);
	out[2] = (in[0]*mat[2][0] + in[1]*mat[2][1] +
		 in[2]*mat[2][2]);
}


void axisorder(long int *order, long int *dir, vset *set)
{
	double xa[3],ya[3],za[3];
	double value[3],f;
	long int i,j,k;
	static double vx[3] = {1.0, 0.0, 0.0};
	static double vy[3] = {0.0, 1.0, 0.0};
	static double vz[3] = {0.0, 0.0, 1.0};

/* find the axis order */

	(void) vl_matmult(vx,xa,set->xform);
	(void) vl_matmult(vy,ya,set->xform);
	(void) vl_matmult(vz,za,set->xform);
/* assume x,y,z */
	for(i=0; i<3; i++) {
		order[i] = i;
	};
/* get the Z values */
	value[0] = xa[2];
	value[1] = ya[2];
	value[2] = za[2];
/* set the dir return array */
	for (k=0; k<3; k++) {
		if (value[k] < 0) {
			dir[k] = -1;
		} else {
			dir[k] = 1;
		};
	};
/* if we have a unity squeeze factor, we have no need to sort the array */
/* keeps the unity case fast! */
	if (set->squeeze_factor == 1.0) return;
/* bubble sort by Value */
	for(k = 0; k<9; k++) {
		for(i=1; i<3; i++) {
			if (fabs(value[i]) > fabs(value[i-1])) {
				f = value[i];
				value[i] = value[i-1];
				value[i-1] = f;
				SWAP(order[i],order[i-1]);
			};
		};
	};
	
}
