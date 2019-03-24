/*
======================================================================
Copyright (C) 1994. All Rights Reserved. 
Image Analysis Facility, University of Iowa

THIS SOFTWARE IS PROPERTY OF:
Image Analysis Facility
The University of Iowa, Iowa City, Iowa

-- OVERVIEW --------------

MODULE NAME: iblast
FILE NAME: palettes.c
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
	Palette binary I/O routines.


-- REVISION HISTORY --------------
DATE:
REASON:
REQUESTED BY:
AUTHOR:

-- GENERAL --------------



 ======================================================================
*/

#include <stdio.h>
#include <string.h>

#include "iblast.h"

void	sread_palette(char *lutf,char *opacityf,long *rlut)
{
	unsigned char lut[768],opacity[768];	
	int i;

        (void) vl_read_pal_(opacityf,opacity);
        (void) vl_read_pal_(lutf,lut);
        for (i=0; i<256; i++ ) {
/* now the rlut */
                rlut[i] = 1L*lut[i] + 256L*lut[i+256] + 65536L*lut[i+512];
		rlut[i] = rlut[i] | ((opacity[i] << 24) & 0xff000000);
	};
}

void	read_palettes()
{
        long rlut[256],llut[256];

/* read lookup tables from disk and */

	(void) sread_palette(lutf,opacityf,rlut);
	(void) sread_palette(lutf,opacityf,llut);
        (void) vl_lookups_(rlut,llut,bcolor,&set);
}
