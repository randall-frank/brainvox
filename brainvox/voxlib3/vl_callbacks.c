#include <stdio.h>
#include "voxel.h"

void vl_puts(VLCHAR *text)
{
	printf("%s",text);
}

#ifndef NOCALLBACKS

#ifdef GLCODE

#include <gl.h>
#include <device.h>
#include <sys/types.h>

#ifndef WIN32
#include <sys/time.h>
#endif

static VLFLOAT64 lasttime = 0.0;

VLINT32 vl_rcallback(VLINT32 type,VLINT32 inter, vset *set)
{
#ifndef WIN32
        struct timeval after;
        struct timezone zone;
#endif
	VLFLOAT64 time2;

        if (inter == 0) return(0L);

	switch (type) {
		case VL_FOREGROUND:
#ifndef WIN32
                        (void) gettimeofday(&after,&zone);
                        time2 = ((VLFLOAT64)after.tv_usec/1000000.0);
                        time2 = time2 + (VLFLOAT64)(after.tv_sec);
			if ((time2 - lasttime) > 1.5) {   
				lasttime = time2;
			/* redisplay the image if desired */
                        	(void) lrectwrite(0,0,set->imagex-1,
				set->imagey-1,(VLUINT32 *)set->image);
			}
#endif
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
VLINT32 vl_rcallback(VLINT32 type,VLINT32 inter, vset *set)
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
