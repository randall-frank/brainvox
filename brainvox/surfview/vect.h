/*
 *	Definition for vector math.  Vectors are just arrays of 3 floats.
 */

#ifndef VECTDEF
#define VECTDEF

#include <math.h>

float *vnew();
float *vclone();
void vcopy(float *, float *);
void vprint(float *);
void vset(float *, float, float, float);
void vzero(float *);
void vnormal(float *);
float vlength(float *);
void vscale(float *, float);
void vmult(float *, float *, float *);
void vadd(float *, float *, float *);
void vsub(float *, float *, float *);
void vhalf(float *, float *, float *);
float vdot(float *, float *);
void vcross(float *, float *, float *);
void vdirection(float *, float *);
void vreflect(float *, float *, float *);
void vmultmatrix(float [4][4], float [4][4], float [4][4]);

#endif
