/*
 * trackball.h
 * A virtual trackball implementation
 * Written by Gavin Bell for Silicon Graphics, November 1988.
 */
#ifndef GLDEF
typedef float Matrix[4][4];
#endif

#include "vect.h"

/*
 *	Pass the x and y coordinates of the last and current positions of
 * the mouse, scaled so they are from (-1.0 ... 1.0).
 *
 * if ox,oy is the window's center and sizex,sizey is its size, then
 * the proper transformation from screen coordinates (sc) to world
 * coordinates (wc) is:
 * wcx = (2.0 * (scx-ox)) / (float)sizex - 1.0
 * wcy = (2.0 * (scy-oy)) / (float)sizey - 1.0
 *
 * For a really easy interface to this see 'ui.h'.
 */
void
trackball(float *, float, float, float, float);

/*
 *	Given two sets of Euler paramaters, add them together to get an
 * equivalent third set.  When incrementally adding them, the first
 * argument here should be the new rotation, the secon and third the
 * total rotation (which will be over-written with the resulting new
 * total rotation).
 */
void
add_eulers(float *, float *, float *);

/*
 *	A useful function, builds a rotation matrix in Matrix based on
 * given Euler paramaters.
 */
void
build_rotmatrix(Matrix, float *);

/*
 * This function computes the Euler paramaters given an xyz axis (the
 * first argument, 3 floats) and angle (expressed in radians, the
 * second argument).  The result is put into the third argument, which
 * must be an array of 4 floats.
 */
void
axis_to_euler(float *, float, float *);
