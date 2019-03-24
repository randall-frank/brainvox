/*
======================================================================
Copyright (C) 1994. All Rights Reserved. 
Image Analysis Facility, University of Iowa

THIS SOFTWARE IS PROPERTY OF:
Image Analysis Facility
The University of Iowa, Iowa City, Iowa

-- OVERVIEW --------------

MODULE NAME: voxlib
FILE NAME: main.c
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
#include <malloc.h>
#include <math.h>


int main(int argc,char **argv)
{
    char *opacity,*lut,*opacity2,*lut2;
    char nfile[2];

    strcpy(nfile,"-");

    if (argc < 2) {
	fprintf(stderr,
    "usage: %s fbfile {{{{palfile1} opacityfile1} palfile2} opacityfile2}\n",
		argv[0]);
	exit(1);
    } 

    printf("\n");
    printf("Left mouse - drag to change cube orientation\n");
    printf("Right mouse - drag to view cutting plane\n");
    printf("F1 - quality toggle\n");
    printf("F2 - reread palettes\n");
    printf("F3 - apply postlighting\n");
    printf("F4 - cutting plane toggle\n");
    printf("\n");

    opacity = nfile;
    lut = nfile;
    opacity2 = nfile;
    lut2 = nfile;

/* read colors/opacity */
    if (argc >= 3) {
	lut = argv[2];
        lut2 = lut;
    }
    if (argc >= 4) {
	opacity = argv[3];
        opacity2 = opacity;
    }
    if (argc >= 5) {
	lut2 = argv[4];
    }
    if (argc >= 6) {
	opacity2 = argv[5];
    }

    render(argv[1],lut,opacity,lut2,opacity2); 

    exit(0);
}

