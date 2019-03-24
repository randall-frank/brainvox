#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#if defined(GFX_IGL_NONE)

#include <gl/gl.h>

#define NOSUID

int main(int argc,char **argv)
{
#ifdef NOSUID
	double		gamma;
	float		framp[256];
	short		ramp[256];
	long int	i;

	
	if (argc == 2) {

		foreground();
		noport();
		winopen("");

		gamma = atof(argv[1]);
		for(i=0;i<256;i++) framp[i] = pow(i/255., 1./gamma);
		for(i=0;i<256;i++) ramp[i] = 255.0*framp[i];
		gammaramp(ramp, ramp, ramp);

	} else {
/* note: the value reported here is not really correct as we do not change
	it above */
		execl("/usr/sbin/gamma",argv[0],(char *)0);
	}
#else
	exit(execl("/usr/sbin/gamma",argv[0],argv[1],(char *)0));
#endif
	exit(0);
}

#else
int main(int argc,char **argv)
{
	fprintf(stderr,"gamma control not supported on this platform\n");
	exit(0);
}
#endif
