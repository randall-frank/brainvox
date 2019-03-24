/*
======================================================================
Copyright (C) 1994. All Rights Reserved. 
Image Analysis Facility, University of Iowa

THIS SOFTWARE IS PROPERTY OF:
Image Analysis Facility
The University of Iowa, Iowa City, Iowa

-- OVERVIEW --------------

MODULE NAME: Voxlib/Polygons/Text
FILE NAME:
DATE:  16 Jun 94
AUTHOR(S): RJF

========================================================================
PROJECT: Voxlib
STATUS: (Development, Production, Obsolete)
PLATFORM: UNIX/MAC/PC  
LANGUAGE: C/C++
DESCRIPTION: This file contains routines that support the rendering
 of general pixmaps into the output image buffer.  This is used by
the font rendering code for high quality/platform specific fonts.
DEPENDENCIES: 
LIMITATIONS:
NOTES:

-- REVISION HISTORY --------------
DATE:
REASON:
REQUESTED BY:
AUTHOR:

 ======================================================================
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "voxel.h"
#include "vl_util.h"
#include "vl_polygon.h"
#include "vl_putil.h"
/*
    ======================================================================
    Function/Method 
    ======================================================================
    PROTOTYPE		: vl_render_pixmap_(void *map,short *val,
			long int dx,long int dy,long int ox,long int oy,
			long int flag,long int color,vset *set);
    DESCRIPTION/USAGE	: This function renders the pixel map pointed to
			  by MAP into the SET image buffer.  The upper
			  left corner of the pixmap is located at ox,oy
			  in the image buffer (0,0=center of buffer).
			  The pixel map is dx,dy pixels in size.  FLAG
			  controls the format of MAP and the usage of
			  VAL and the RGB color passed in COLOR.
    SYNTAX/PARAMETERS	: FLAG consists of the following bitmasks:
			  VL_P_NOZWRITE: disable writing to ouput zbuffer
			  VL_P_NOZCOMP: disable zbuffer compares while rendering
			  VL_P_PMAP_32: assume MAP is an array of 32 bit RGBA 
				pixels instead of an array of 8bit A pixels.
				If VL_P_PMAP_32 is set, the COLOR paramter is
				ignored.
			  VL_P_PMAP_ZB: assume VAL is a pointer to an array of
				Z values (dx,dy) at each pixel.  By default,
				VAL is assumed to point to a single Z value
				for all pixels.
    PRE-CONDITIONS	:
    POST-CONDITIONS	:
    SIDE EFFECTS   	:
    LIMITATIONS 	:
	Never been optimized!  Could be broken into a number of special
	cases for MUCH higher speed.

    NOTES		:  The pixels are alpha blended into the output
				buffer.  (255=opaque 0=transparent)

    ======================================================================
*/
void vl_render_pixmap_(void *map,short *val,long int dx,long int dy,
	long int ox,long int oy,long int flag,long int color,vset *set)
{
	long int	i,j,k,t,offset;
	short		zval;
	unsigned long int	b2,im_val,col,Icolor,RB,MSA,SA;
	unsigned long int 	*l_in = (unsigned long int *)map;
	unsigned char		*c_in = (unsigned char *)map;
	unsigned long int	*image = (unsigned long int *)set->image;

/* check for valid output */
	if (set->image == 0L) return;
	if (set->zbuffer == 0L) return;

/* get the pixmap index */
	k = 0;
	for(j=0;j<dy;j++) {
/* get offset into ouput map, and clip */
		offset = j + oy + (set->imagey/2);
		if ((offset >= 0) && (offset < set->imagey)) {
		offset *= (set->imagex);
		offset += (ox + (set->imagex/2));
		for(i=0;i<dx;i++) {
			t = i + ox + (set->imagex/2);
			if ((t >= 0) && (t < set->imagex)) {
/* get color and alpha */
				if (flag & VL_P_PMAP_32) {
					col = (l_in[k] & 0x00ffffff);
					SA = ((l_in[k] >> 24) & 0xff);
				} else {
					col = color & 0x00ffffff;
					SA = c_in[k];
				}
/* get z value */
				if (flag & VL_P_PMAP_ZB) {
					zval = val[k];
				} else {
					zval = *val;
				}
/* render (zclip first) */
				if ((set->zbuffer[offset] < (zval*8)) || 
					(flag & VL_P_NOZCOMP)) {
/* build the pixel color */
					MSA = 255 - SA;
					im_val = image[offset];
					b2 = im_val & 0x00ff00ff;
					Icolor = col & 0x00ff00ff;
					RB = ((Icolor*SA) + (b2*MSA)) 
						& 0xff00ff00;
					b2 = im_val & 0x00ff00;
					Icolor = col & 0x00ff00;
					RB = RB | (((Icolor*SA) + (b2*MSA)) 
						& 0x00ff0000);
/* place it in the buffer */
					image[offset] = RB >> 8;
/* update the zbuffer (if WRITE is enabled and the Alpha is != 0) */
					if (((flag & VL_P_NOZWRITE) == 0)
				 && (SA != 0)) set->zbuffer[offset] = (zval*8);
				} 
			}
			offset++;
			k++;
		}
		}
	}

	return;
}


