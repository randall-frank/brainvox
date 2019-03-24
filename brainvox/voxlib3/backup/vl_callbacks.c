#include <stdio.h>
#include "voxel.h"

void vl_puts(char *text)
{
	printf("%s",text);
}

#ifndef NOCALLBACKS

#ifdef GLCODE

#include <gl.h>
#include <device.h>
#include <sys/types.h>
#include <sys/time.h>

static double lasttime = 0.0;

long int vl_rcallback(long type,vset *set)
{
        struct timeval after;
        struct timezone zone;
	double time2;

	switch (type) {
		case VL_FOREGROUND:
                        (void) gettimeofday(&after,&zone);
                        time2 = ((double)after.tv_usec/1000000.0);
                        time2 = time2 + (double)(after.tv_sec);
			if ((time2 - lasttime) > 1.5) {   
				lasttime = time2;
			/* redisplay the image if desired */
                        	(void) lrectwrite(0,0,set->imagex-1,
				set->imagey-1,(unsigned long *)set->image);
			}
			/* if abort requested return true */
                        if (getbutton(LEFTMOUSE)) { 
				qreset();
				lasttime = 0.0;
				return(1L);
                        };
			return(0L);
			break;

		case VL_BACKGROUND:
			return(0L);
			break;
	}
	return(0L);
}
#else
long int vl_rcallback(long type,vset *set)
{
	switch (type) {
		case VL_FOREGROUND:
			/* if abort requested return true */
			/* redisplay the image if desired */
			return(0L);
			break;

		case VL_BACKGROUND:
			return(0L);
			break;
	}
	return(0L);
}
#endif

#endif
