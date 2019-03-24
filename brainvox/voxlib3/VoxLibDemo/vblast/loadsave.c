/*
======================================================================
Copyright (C) 1994. All Rights Reserved. 
Image Analysis Facility, University of Iowa

THIS SOFTWARE IS PROPERTY OF:
Image Analysis Facility
The University of Iowa, Iowa City, Iowa

-- OVERVIEW --------------

MODULE NAME: vblast
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
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "voxel.h"

int	tsavedefs(char *filename,float *ipixel,float *islice,int *start,int *end
	,int *step,int *dx, int *dy, char *tname);
int	tloaddefs(char *filename,float *ipixel,float *islice,int *start,int *end
	,int *step,int *dx, int *dy, char *tname);


int	tloaddefs(char *filename,float *ipixel,float *islice,int *start,int *end
	,int *step,int *dx, int *dy, char *tname)
{
	FILE *mfile;
	char units[256];
	float ver;
	char	opacityf[256];
	char	lutf[256];

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
	FILE 	*mfile;

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
	fprintf(mfile,"%s\n","_opacity");
	fprintf(mfile,"%s\n","_vlut");
	fclose(mfile);
	return(0);
}

/* read a dataset specified via a Voxblast .fb file */
extern	vset	set;

void    loadset(char *fname,long int *lims)
{
        int start,end,step,dx,dy,interp;
        float ipixel,islice,squf;
        char tname[256];
        long int count,i;
        unsigned char *data;
        unsigned long hist[256];

        if (tloaddefs(fname,&ipixel,&islice,&start,&end,&step,&dx,&dy,
                tname)== -1) {
                fprintf(stderr,"Unable to read file:%s\n",fname);
                return;
        }

/* free old memory if needed */
        if (set.data != 0) {
                free((set.data));
        }
/* count the images */
        count =0;
        for(i=start; i<=end; i = i + step) count=count+1;
/* get the squeeze factor */
        squf = 10.0;
        interp = -1;
        while ((squf > 1.00) && (interp < 20)) {
                interp = interp + 1;
                squf = (((float)count)*islice)/
                        ((float)(count+ (interp*(count-1)))*ipixel);
        }
        printf("Interp = %d  squf = %f \n",interp,squf);

/* handle interpolations */
        count = count + (interp * (count-1));

/* allocate memory for the data x*y*count */
        (void) printf("Allocating %d bytes of memory.\n",count*dx*dy);
        data = malloc((count*dx*dy));
        if (data == NULL) {
                (void) printf("Unable to allocate image memory.\n");
                exit(1);
        }

/* read the images */
        (void) vl_read_images_(tname,dx,dy,start,end,step,interp,data);

/* set the dataset & squeeze factor */
        (void) vl_data_(dx,dy,count,data,&set);

/* report back the limits */
        lims[0] = 0;
        lims[1] = dx-1;
        lims[2] = 0;
        lims[3] = dy-1;
        lims[4] = 0;
        lims[5] = count-1;

        (void) vl_setsqu_(&squf,&set);

        return;
}

