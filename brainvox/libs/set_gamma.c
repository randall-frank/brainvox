#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "gl.h"

void	brainvox_gamma_(float *gamma)
{
	float           framp[256];
	short           ramp[256];
	long int        i;

	for(i=0;i<256;i++) framp[i] = pow(i/255., 1./(*gamma));
	for(i=0;i<256;i++) ramp[i] = 255.0*framp[i];

	gammaramp(ramp, ramp, ramp);

	return;
}
