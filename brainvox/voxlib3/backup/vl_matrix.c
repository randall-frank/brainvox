#include "voxel.h"
#include "vl_util.h"
#include <stdio.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159
#endif

/* routine to convert the matrix in the set into a set of rotations */
void 	vl_reverse_matrix_(vset *set)
{
	double x,y,z,temp[3][3],error;
	long i,j,k;

	y = -(set->xform[2][0]);
	if (y > 1.0) y = 1.0;
	if (y < -1.0) y = -1.0;
	y = asin(y);
	if (fabs(cos(y)) < 0.00001) {  /* if the cos is VERY SMALL */
/* AMBIGUITIES EXIST HERE!!!  we ARBITRARILY set z to 0 */
		z = 0.0;
		x = atan2(-(set->xform[1][2]),(set->xform[1][1]));
	} else {
		x = atan2(((set->xform[2][1])/cos(y)),
			((set->xform[2][2])/cos(y)));
		z = atan2(((set->xform[1][0])/cos(y)),
			((set->xform[0][0])/cos(y)));
	}

/* put the angles in degrees */
	x = (x*180.0)/M_PI;
	y = (y*180.0)/M_PI;
	z = (z*180.0)/M_PI;

/* set their rotations (the matrix should remain unchanged) */
	set->rots[0] = x;
	set->rots[1] = y;
	set->rots[2] = z;

/* check the calculations */
 	(void) vl_calcmatrix(set->rots,temp);
	error = 0.0;
	for(j=0;j<3;j++) {
	    for(k=0;k<3;k++) {
		error = error + fabs(temp[j][k] - (set->xform[j][k]));
	    }
	}
	if (error > 0.001) vl_puts("Internal matrix consistancy error detected.");
#ifdef DEBUG
	if (error > 0.001) {
		printf("Internal error:%lf %lf %lf = %lf\n",x,y,z,error);
#define ZZZZ set->xform
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
	return;
}

void	vl_arb_rotate_(double *angle, double *axis, vset *set)
{
	double temp2[3][3],temp[3][3],dang,u[3],l;
	long int	i,j,k;

/* convert angle to radians */
	dang = -((*angle)/180.0)*M_PI;

/* make sure it is a UNIT vector */
	l = (axis[0]*axis[0]) + (axis[1]*axis[1]) + (axis[2]*axis[2]);
	if (l == 0.0) {
		vl_puts("Error: Zero length unit axis specified.");
		return;
	}
	l = sqrt(l);
	u[0] = axis[0] / l;
	u[1] = axis[1] / l;
	u[2] = axis[2] / l;

/* build the matrix for rotation about an arbitrary axis (u) */
	temp2[0][0] = u[0]*u[0] + cos(dang)*(1.0 - (u[0]*u[0]));
	temp2[1][0] = u[0]*u[1]*(1.0 - cos(dang)) - u[2]*sin(dang);
	temp2[2][0] = u[2]*u[0]*(1.0 - cos(dang)) + u[1]*sin(dang);

	temp2[0][1] = u[0]*u[1]*(1.0 - cos(dang)) + u[2]*sin(dang);
	temp2[1][1] = u[1]*u[1] + cos(dang)*(1.0 - (u[1]*u[1]));
	temp2[2][1] = u[1]*u[2]*(1.0 - cos(dang)) - u[0]*sin(dang);

	temp2[0][2] = u[2]*u[0]*(1.0 - cos(dang)) - u[1]*sin(dang);
	temp2[1][2] = u[1]*u[2]*(1.0 - cos(dang)) + u[0]*sin(dang);
	temp2[2][2] = u[2]*u[2] + cos(dang)*(1.0 - (u[2]*u[2]));

#ifdef DEBUG
	printf("Original Matrix:\n");
	printf(" %fl %fl %fl \n",temp2[0][0],temp2[0][1],temp2[0][2]);
	printf(" %fl %fl %fl \n",temp2[1][0],temp2[1][1],temp2[1][2]);
	printf(" %fl %fl %fl \n",temp2[2][0],temp2[2][1],temp2[2][2]);
#endif
/* perform the matrix multiply */
        for(i=0;i<3;i++) {
                for(j=0;j<3;j++) {
			temp[i][j] = 0.0;
			for(k=0; k<3;k++) {
			    temp[i][j] += ((temp2[i][k]) * (set->xform[k][j]));
			}
                }
        }
#ifdef DEBUG
	printf("Output Matrix:\n");
	printf(" %fl %fl %fl \n",temp[0][0],temp[0][1],temp[0][2]);
	printf(" %fl %fl %fl \n",temp[1][0],temp[1][1],temp[1][2]);
	printf(" %fl %fl %fl \n",temp[2][0],temp[2][1],temp[2][2]);
#endif

/* copy the output array to the vset */
        for(i=0;i<3;i++) {
                for(j=0;j<3;j++) {
                        set->xform[i][j] = temp[i][j];
                }
        }

/* get the new absolute rotations from the matrix */
	(void) vl_reverse_matrix_(set);

/* and we're done... */
	return;

}
