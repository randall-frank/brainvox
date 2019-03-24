/*
======================================================================
Copyright (C) 1994. All Rights Reserved. 
Image Analysis Facility, University of Iowa

THIS SOFTWARE IS PROPERTY OF:
Image Analysis Facility
The University of Iowa, Iowa City, Iowa

-- OVERVIEW --------------

MODULE NAME: iblast
FILE NAME: svimg.c
DATE:  
AUTHOR(S): 

========================================================================
PROJECT: Voxlib
STATUS: Development
PLATFORM: UNIX  
LANGUAGE: C
DESCRIPTION: Command line Voxlib function exerciser
DEPENDENCIES: 
LIMITATIONS:
NOTES:
	Routine to save a Voxlib buffer to disk as a 3 byte RGB or 1 byte 
grayscale image.


-- REVISION HISTORY --------------
DATE:
REASON:
REQUESTED BY:
AUTHOR:

-- GENERAL --------------



 ======================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* routine to save an RGB or Grayscale image to disk */

void svimg_(char *filename,long int *array,long int xsize,long int ysize,
	long int zsize,long int zoom,long int llx,long int lly,
	long int dx,long int dy,long int *err)
{
    long int y,ptr,x,bcount,planar;
    long int oy,ox,z,plane;
    FILE *image;

unsigned char rbuf[4096];

    *err = 0;
    image = fopen(filename,"w");        
    if (image == 0){
        *err = -1;
        return;
    }
    planar = 0;
    if (getenv("RGB_FORMAT")) {
	if (strcmp(getenv("RGB_FORMAT"),"PLANAR") == 0) planar = 1;
    }
    if (planar == 0) {
/* pixel interleaved */
    	oy = 0;
    	for(y=0; y<dy; y++) {
/* copy the line into the bufs from $aabbggrr */
                ptr = ((y + lly)*xsize) + llx;
                ox = 0;
                for(x=0; x<dx; x++) {
                        for(z=0; z<zoom; z++) {
                                rbuf[ox++] = (array[ptr]) & 0xff;  /* for
$rrrrrrrr */
                                if (zsize != 1) {  /* for $rrggbbrrggbbrrggbb */
                                        rbuf[ox++] = (array[ptr] >> 8) & 0xff;
                                        rbuf[ox++] = (array[ptr] >> 16) & 0xff;
                                }
                        }
                        ptr++;
                }
                /*
                        fill rbuf, gbuf, and bbuf with pixel values
                */
                for(z=0; z<zoom; z++) {
                        bcount = dx*zoom*zsize;
			fwrite(rbuf,bcount,1,image);
                        oy = oy + 1;
                }
    	}
    } else {
/* planar image format */
	for(plane=0;plane<zsize;plane++) {
		for(y=0;y<dy;y++) {
			ox = 0;
			ptr = ((y + lly)*xsize) + llx;
/* build up the scan line */
                	for(x=0; x<dx; x++) {
                        	for(z=0; z<zoom; z++) {
                                	rbuf[ox++] = 
					    ((array[ptr]) >> (8*plane)) & 0xff;
                                }
                        	ptr++;
                        }
/* write the scanline ZOOM times */
                	for(z=0; z<zoom; z++) {
                        	bcount = dx*zoom*1;
				fwrite(rbuf,bcount,1,image);
                	}
		}
	}
    }
    fclose(image);
    return;
}  
