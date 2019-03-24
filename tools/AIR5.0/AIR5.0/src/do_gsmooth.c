/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 5/23/01 */

/*
 *
 * This routine is intended to provide orthogonal Fourier
 *  smoothing that will take masking into account.
 *
 * Areas that have a value of zero in the mask will have zero
 *  values on output, and these areas will not be smoothed into
 *  adjacent non-zero areas. Likewise, the zero-padding	used
 *  to protect against wraparound effects will not be smoothed
 *  into the image matrix areas.
 *
 */

#include "AIR.h"

#define REPLACE TRUE		/* if nonzero, smoothing routine replaces input array with output */

static void free_function(/*@null@*/ /*@out@*/ /*@only@*/ AIR_Pixels ***datain, /*@null@*/ /*@out@*/ /*@only@*/ AIR_Pixels ***datamask)

{
	if(datain) AIR_free_vol3(datain);
	if(datamask) AIR_free_vol3(datamask);
}

AIR_Error AIR_do_gsmooth(const char *program, const char *output, const char *input, const AIR_Boolean pad, const AIR_Boolean smoothpad, const float kx, const float ky, const float kz, const char *maskfile, const AIR_Boolean rescale, const AIR_Boolean verbose, const AIR_Boolean ow)

{
	if(kx<0||ky<0||kz<0){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("Smoothing kernels must all be nonnegative numbers\n");
		return(AIR_NEGATIVE_SMOOTHING_ERROR);
	}
	
	/* Precheck of output permissions */
	{
		AIR_Error errcode=AIR_save_probw(output,ow);
		if(errcode!=0){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("Computations not performed due to anticipated output problems with %s\n",output);
			return(errcode);
		}
	}

	{
		AIR_Pixels ***datain=NULL;
		AIR_Pixels ***datamask=NULL;
		
		struct AIR_Key_info dimensions;
		struct AIR_Key_info dimensionsmask;
		
		/*Load the file to be smoothed*/
		{
			AIR_Error errcode;

			datain=AIR_load(input, &dimensions, FALSE, &errcode);
			if(!datain){
				free_function(datain,datamask);
				return(errcode);
			}
		}
		if(maskfile){
		
			/* Load the mask */
			{
				AIR_Error errcode;

				datamask=AIR_load(maskfile, &dimensionsmask, TRUE, &errcode);
				if(!datamask){
					free_function(datain,datamask);
					return(errcode);
				}
			}
			/*Verify compatibility of data and mask*/
			{
				AIR_Error errcode=AIR_same_dim(&dimensions,&dimensionsmask);
				if(errcode!=0){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("Files %s and %s have incompatible dimensions\n",input,maskfile);
					free_function(datain,datamask);
					return(errcode);
				}
			}				
		}
		
		{
			float scaling;
			
			/* Smooth data back into datain */
			{
				AIR_Error errcode;
				
				if((AIR_Pixels ***)AIR_gausser(datain,&dimensions,datamask,kx,ky,kz,rescale,&scaling,pad,smoothpad,REPLACE,&errcode)==NULL){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("Smoothing failed\n");
					free_function(datain,datamask);
					return(errcode);
				}
			}
			
			/*Save the data*/
			{
				AIR_Error errcode=AIR_save(datain,&dimensions,output,ow,program);
				if(errcode!=0){
					free_function(datain,datamask);
					return(errcode);
				}
			}

			if(verbose){
				if(scaling!=1.0){
					printf("Values in output file %s rescaled by multiplicative factor of %e\n",output,scaling);
				}
			}
		}
		free_function(datain,datamask);
	}
	return 0;
}

