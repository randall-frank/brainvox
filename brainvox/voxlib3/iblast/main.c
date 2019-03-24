/*
======================================================================
Copyright (C) 1994. All Rights Reserved. 
Image Analysis Facility, University of Iowa

THIS SOFTWARE IS PROPERTY OF:
Image Analysis Facility
The University of Iowa, Iowa City, Iowa

-- OVERVIEW --------------

MODULE NAME: iblast
FILE NAME: main.c
DATE:  
AUTHOR(S): 

========================================================================
PROJECT: Voxlib
STATUS: Development
PLATFORM: Unix  
LANGUAGE: C
DESCRIPTION: Voxlib interactive command line excerciser
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
#include <string.h>
#include <stdlib.h>

#include "iblast.h"

float rx = 0.0;
float ry = 0.0;
float rz = 0.0;
int speed = 1;
vset set;
char opacityf[256];
char lutf[256];
int imagex,imagey;
int vwid,dirty;
int bcolor;

/* set up some globals and call the commandline parser */

int main(int argc,char **argv)
{
	bcolor = 0;

	strcpy(opacityf,"_opacity");
	strcpy(lutf,"_vlut");

	cmdline();

	exit(0);
}
