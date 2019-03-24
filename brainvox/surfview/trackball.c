/*
 *	Implementation of a virtual trackball.  See trackball.h for the
 * interface to these routines.
 *	Implemented by Gavin Bell, lots of ideas from Thant Tessman and
 * the August '88 issue of Siggraph's "Computer Graphics," pp. 121-129.
 */
#include <stdio.h>
#include <math.h>
#include "trackball.h"

/* Size of virtual trackball, in relation to window size */
float trackballsize = 0.4;

/* Function prototypes for local functions */
float tb_project_to_sphere(float, float, float);
void normalize_euler(float *);

/*
 * Ok, simulate a track-ball.  Project the points onto the virtual
 * trackball, then figure out the axis of rotation, which is the cross
 * product of P1 P2 and O P1 (O is the center of the ball, 0,0,0)
 * Note:  This is a deformed trackball-- is a trackball in the center,
 * but is deformed into a hyperbolic solid of rotation away from the
 * center.
 * 
 * It is assumed that the arguments to this routine are in the range
 * (-1.0 ... 1.0)
 */
void
trackball(float *e, float p1x, float p1y, float p2x, float p2y)
{
	float p1[3];
	float p2[3];
	float d[3];
	float phi;	/* how much to rotate about axis */
	float a[3];	/* Axis of rotation */

	vzero(a);

	if (p1x == p2x && p1y == p2y)
	{
		vzero(e);	/* Zero rotation */
		e[3] = 1.0;
		return ;
	}

/*
 * First, figure out z-coordinates for projection of P1 and P2 to
 * deformed sphere
 */
	vset(p1, p1x, p1y, tb_project_to_sphere(trackballsize, p1x, p1y));
	vset(p2, p2x, p2y, tb_project_to_sphere(trackballsize, p2x, p2y));

/*
 *	Now, we want the cross product of P1 and P2
 *  (Or cross product of p2 and p1... this was determined by trial and
 * error, so there may be a compensating mathematical boo-boo
 * somewhere else).
 */
	vcross(p2, p1, a);

/*
 *	Figure out how much to rotate around that axis.
 */
	vsub(p1, p2, d);
	phi = sin(vlength(d) / (2.0 * trackballsize));

	axis_to_euler(a, phi, e);
}

/*
 *	Given an axis and angle, compute euler paramaters
 */
void
axis_to_euler(float *a, float phi, float *e)
{
	vnormal(a);	/* Normalize axis */
	vcopy(a, e);
	vscale(e, sin(phi/2.0));
	e[3] = cos(phi/2.0);
}

/*
 * Project an x,y pair onto a sphere of radius r OR a hyperbolic sheet
 * if we are away from the center of the sphere.
 */
float
tb_project_to_sphere(float r, float x, float y)
{
	float d, t, z;

	d = sqrt(x*x + y*y);
	if (d < r*M_SQRT1_2)	/* Inside sphere */
	{
		z = sqrt(r*r - d*d);
	}
	else	/* On hyperbola */
	{
		t = r / M_SQRT2;
		z = t*t / d;
	}
	return z;
}

/*
 *	Given two rotations, e1 and e2, expressed as Euler paramaters,
 * figure out the equivalent single rotation and stuff it into dest.
 * 
 * This routine also normalizes the result every COUNT times it is
 * called, to keep error from creeping in.
 */
#define COUNT 100
void
add_eulers(float *e1, float *e2, float *dest)
{
	static int count=0;
	register int i;
	float t1[3], t2[3], t3[3];
	float tf[4];

	vcopy(e1, t1); vscale(t1, e2[3]);
	vcopy(e2, t2); vscale(t2, e1[3]);
	vcross(e2, e1, t3);
	vadd(t1, t2, tf);
	vadd(t3, tf, tf);
	tf[3] = e1[3] * e2[3] - vdot(e1, e2);

	for (i = 0 ; i < 4 ;i++)
	{
		dest[i] = tf[i];
	}

	if (++count > COUNT)
	{
		count = 0;
		normalize_euler(dest);
	}
}

/*
 * Euler paramaters always obey:  a^2 + b^2 + c^2 + d^2 = 1.0
 * We'll normalize based on this formula.  Also, normalize greatest
 * component, to avoid problems that occur when the component we're
 * normalizing gets close to zero (and the other components may add up
 * to more than 1.0 because of rounding error).
 */
void
normalize_euler(float *e)
{	/* Normalize result */
	int which, i;
	float gr;

	which = 0;
	gr = e[which];
	for (i = 1 ; i < 4 ; i++)
	{
		if (fabs(e[i]) > fabs(gr))
		{
			gr = e[i];
			which = i;
		}
	}

	e[which] = 0.0;

	e[which] = sqrt(1.0 - (e[0]*e[0] + e[1]*e[1] +
		e[2]*e[2] + e[3]*e[3]));

	/* Check to see if we need negative square root */
	if (gr < 0.0)
		e[which] = -e[which];

#define EQ(a, b) (fabs((a)-(b)) < 0.0001)

	if (!(EQ(e[which],gr)))
	{
		fprintf(stderr, "Whoa: e goes from %f to %f\n",
			gr, e[0]);
	}
}

/*
 * Build a rotation matrix, given Euler paramaters.
 */
void
build_rotmatrix(Matrix m, float *e)
{
	m[0][0] = 1 - 2.0 * (e[1] * e[1] + e[2] * e[2]);
	m[0][1] = 2.0 * (e[0] * e[1] - e[2] * e[3]);
	m[0][2] = 2.0 * (e[2] * e[0] + e[1] * e[3]);
	m[0][3] = 0.0;

	m[1][0] = 2.0 * (e[0] * e[1] + e[2] * e[3]);
	m[1][1] = 1 - 2.0 * (e[2] * e[2] + e[0] * e[0]);
	m[1][2] = 2.0 * (e[1] * e[2] - e[0] * e[3]);
	m[1][3] = 0.0;

	m[2][0] = 2.0 * (e[2] * e[0] - e[1] * e[3]);
	m[2][1] = 2.0 * (e[1] * e[2] + e[0] * e[3]);
	m[2][2] = 1 - 2.0 * (e[1] * e[1] + e[0] * e[0]);
	m[2][3] = 0.0;

	m[3][0] = 0.0;
	m[3][1] = 0.0;
	m[3][2] = 0.0;
	m[3][3] = 1.0;
}
