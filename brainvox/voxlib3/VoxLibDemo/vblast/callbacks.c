/*
======================================================================
Copyright (C) 1994. All Rights Reserved. 
Image Analysis Facility, University of Iowa

THIS SOFTWARE IS PROPERTY OF:
Image Analysis Facility
The University of Iowa, Iowa City, Iowa

-- OVERVIEW --------------

MODULE NAME: voxlib
FILE NAME: callbacks.c
DATE:  
AUTHOR(S): 

========================================================================
PROJECT: vblast
STATUS: Development
PLATFORM: UNIX/IrisGL 
LANGUAGE: C
DESCRIPTION: Simple IrisGL viewer attached to voxlib
DEPENDENCIES: 
LIMITATIONS:
NOTES:
	Routines for background redisplay of the rendered image...


-- REVISION HISTORY --------------
DATE:
REASON:
REQUESTED BY:
AUTHOR:

-- GENERAL --------------



 ======================================================================
*/


#include <gl.h>
#include <device.h>
#include <sys/types.h>
#include <sys/time.h>
#include "voxel.h"

/* time between redraws (every 1/2 sec)  Rendering will be faster if 
	this number is increased */
#define REFRESH_TIME 0.5

static double lasttime = 0.0;

long int vl_rcallback(long type,vset *set)
{
        struct timeval after;
        struct timezone zone;
	double time2;

	switch (type) {
		case VL_FOREGROUND:
		case VL_BACKGROUND:
                        (void) gettimeofday(&after,&zone);
                        time2 = ((double)after.tv_usec/1000000.0);
                        time2 = time2 + (double)(after.tv_sec);
			if ((time2 - lasttime) > REFRESH_TIME) {   
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
	}
	return(0L);
}
