#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "voxel.h"

void vl_arb_rotate_(double *angle, double *axis, vset *set);

void	cube_rot_(long int *dx,long int *dy,long int *dz,vset *set,float *ret)
{
	double fp[4];

	fp[0] = (*dx);
	fp[1] = 0;
	fp[2] = 1.0;
	fp[3] = 0;
	vl_arb_rotate_(&(fp[0]),&(fp[1]),set);
	fp[0] = -(*dy);
	fp[1] = 1.0;
	fp[2] = 0;
	fp[3] = 0;
	vl_arb_rotate_(&(fp[0]),&(fp[1]),set);
	fp[0] = -(*dz);
	fp[1] = 0;
	fp[2] = 0;
	fp[3] = 1.0;
	vl_arb_rotate_(&(fp[0]),&(fp[1]),set);

	ret[0] = set->rots[0];
	ret[1] = set->rots[1];
	ret[2] = set->rots[2];

	return;
}
void	CUBE_ROT(long int *dx,long int *dy,long int *dz,vset *set,float *ret)
{
cube_rot_(dx,dy,dz,set,ret);
}
