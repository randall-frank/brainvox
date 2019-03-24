/*
 * vect -
 *	Various functions to support operations on vectors.
 *
 * David M. Ciemiewicz, Mark Grossman, Henry Moreton, and Paul Haeberli
 *
 * Modified for my own nefarious purposes-- Gavin Bell
 */
#include <stdio.h>
#include "vect.h"

float *vnew()
{
	register float *v;

	v = (float *) malloc(sizeof(float)*3);
	return v;
}

float *vclone(float *v)
{
	register float *c;

	c = vnew();
	vcopy(v, c);
	return c;
}

void vcopy(float *v1,float *v2)
{
	register int i;
	for (i = 0 ; i < 3 ; i++)
		v2[i] = v1[i];
}

void vprint(float *v)
{
	printf("x: %f y: %f z: %f\n",v[0],v[1],v[2]);
}

void vset(float *v,float x,float y,float z)
{
	v[0] = x;
	v[1] = y;
	v[2] = z;
}

void vzero(float *v)
{
	v[0] = 0.0;
	v[1] = 0.0;
	v[2] = 0.0;
}

void vnormal(float *v)
{
	vscale(v,1.0/vlength(v));
}

float vlength(float *v)
{
	return sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

void vscale(float *v,float div)
{
	v[0] *= div;
	v[1] *= div;
	v[2] *= div;
}

void vmult(float *src1,float *src2,float *dst)
{
	dst[0] = src1[0] * src2[0];
	dst[1] = src1[1] * src2[1];
	dst[2] = src1[2] * src2[2];
}

void vadd(float *src1,float *src2,float *dst)
{
	dst[0] = src1[0] + src2[0];
	dst[1] = src1[1] + src2[1];
	dst[2] = src1[2] + src2[2];
}

void vsub(float *src1,float *src2,float *dst)
{
	dst[0] = src1[0] - src2[0];
	dst[1] = src1[1] - src2[1];
	dst[2] = src1[2] - src2[2];
}

void vhalf(float *v1,float *v2,float *half)
{
	float len;

	vadd(v2,v1,half);
	len = vlength(half);
	if(len>0.0001)
		vscale(half,1.0/len);
	else
		*half = *v1;
}

float vdot(float *v1,float *v2)
{
	return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
}

void vcross(float *v1, float *v2, float *cross)
{
	float temp[3];

	temp[0] = (v1[1] * v2[2]) - (v1[2] * v2[1]);
	temp[1] = (v1[2] * v2[0]) - (v1[0] * v2[2]);
	temp[2] = (v1[0] * v2[1]) - (v1[1] * v2[0]);
	vcopy(temp, cross);
}

void vdirection(float *v1, float *dir)
{
	*dir = *v1;
	vnormal(dir);
}

void vreflect(float *in,float *mirror,float *out)
{
	float temp[3];

	vcopy(mirror, temp);
	vscale(temp,vdot(mirror,in));
	vsub(temp,in,out);
	vadd(temp,out,out);
}

void vmultmatrix(float m1[4][4],float m2[4][4],float prod[4][4])
{
	register int row, col;
	float temp[4][4];

	for(row=0 ; row<4 ; row++) 
		for(col=0 ; col<4 ; col++)
			temp[row][col] = m1[row][0] * m2[0][col]
						   + m1[row][1] * m2[1][col]
						   + m1[row][2] * m2[2][col]
						   + m1[row][3] * m2[3][col];
	for(row=0 ; row<4 ; row++) 
		for(col=0 ; col<4 ; col++)
		prod[row][col] = temp[row][col];
}

void vtransform(float *v,float mat[4][4],float *vt)
{
	float t[3];

	t[0] = v[0]*mat[0][0] + v[1]*mat[1][0] + v[2]*mat[2][0] + mat[3][0];
	t[1] = v[0]*mat[0][1] + v[1]*mat[1][1] + v[2]*mat[2][1] + mat[3][1];
	t[2] = v[0]*mat[0][2] + v[1]*mat[1][2] + v[2]*mat[2][2] + mat[3][2];
	vcopy(t, vt);
}
