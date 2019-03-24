/*
======================================================================
Copyright (C) 1994. All Rights Reserved. 
Image Analysis Facility, University of Iowa

THIS SOFTWARE IS PROPERTY OF:
Image Analysis Facility
The University of Iowa, Iowa City, Iowa

-- OVERVIEW --------------

MODULE NAME: iblast
FILE NAME: loadsave.c
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
	Routines for reading and writing early VoxBlast volume definition files


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
#include <sys/types.h>
#include <sys/stat.h>
#include "iblast.h"


int	tloaddefs(char *filename,float *ipixel,float *islice,int *start,int *end
	,int *step,int *dx, int *dy, char *tname)
{
	FILE *mfile;
	char units[256];
	float ver;

	mfile = fopen(filename,"r");
	if (mfile == NULL) {
		printf("Unable to open the file:%s\n",filename);
		return(-1);
	}
	fscanf(mfile,"%s %f\n",units,&ver);
	if (strcmp(units,"FBLAST") != 0) {
		printf("Invalid format in the file:%s\n",filename);
		return(-1);
	}
	fscanf(mfile,"%s\n",tname);
	fscanf(mfile,"%d\n",dx);
	fscanf(mfile,"%d\n",dy);
	fscanf(mfile,"%d\n",start);
	fscanf(mfile,"%d\n",end);
	fscanf(mfile,"%d\n",step);
	fscanf(mfile,"%f\n",ipixel);
	fscanf(mfile,"%f\n",islice);
	fscanf(mfile,"%s\n",units);
	fscanf(mfile,"%s\n",opacityf);
	fscanf(mfile,"%s\n",lutf);
	fclose(mfile);
	return(0);
}

int	tsavedefs(char *filename,float *ipixel,float *islice,int *start,int *end
	,int *step,int *dx, int *dy, char *tname)
{
	FILE *mfile;

	mfile = fopen(filename,"w");
	if (mfile == NULL) {
		printf("Unable to open the file:%s\n",filename);
		return(-1);
	}
	fprintf(mfile,"FBLAST 1.0\n");
	fprintf(mfile,"%s\n",tname);
	fprintf(mfile,"%d\n",*dx);
	fprintf(mfile,"%d\n",*dy);
	fprintf(mfile,"%d\n",*start);
	fprintf(mfile,"%d\n",*end);
	fprintf(mfile,"%d\n",*step);
	fprintf(mfile,"%f\n",*ipixel);
	fprintf(mfile,"%f\n",*islice);
	fprintf(mfile,"%s\n","MM");
	fprintf(mfile,"%s\n",opacityf);
	fprintf(mfile,"%s\n",lutf);
	fclose(mfile);
	return(0);
}
