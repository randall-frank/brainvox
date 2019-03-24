/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 5/23/01 */

/*
 * Uses Fourier interpolation to magnify files
 */


#include "AIR.h"

static void free_function(/*@null@*/ /*@out@*/ /*@only@*/ AIR_Pixels ***datain, /*@null@*/ /*@out@*/ /*@only@*/ AIR_Pixels ***dataout)

{
	if(datain) AIR_free_vol3(datain);
	if(dataout) AIR_free_vol3(dataout);
}

AIR_Error AIR_do_magnify(const char *program, const char *input, const char *output, const unsigned int bigx, const unsigned int bigy, const unsigned int bigz, const unsigned int smallx, const unsigned int smally, const unsigned int smallz, const AIR_Boolean ow)

{
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
		struct AIR_Key_info dimensions;
		struct AIR_Key_info new_dimensions;
		AIR_Pixels ***datain=NULL;
		AIR_Pixels ***dataout=NULL;
		
		{
			AIR_Error errcode;

			datain=AIR_load(input, &dimensions, 0, &errcode);
			if(!datain){
				free_function(datain,dataout);
				return(errcode);
			}
		}
		
		if(smallx*(bigx*dimensions.x_dim/smallx)!=bigx*dimensions.x_dim){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("x shrinkage must be a factor of the x dimension (%u)\n",dimensions.x_dim);
			free_function(datain,dataout);
			return(AIR_MAGNIFY_MOD_ERROR);
		}
		if(smally*(bigy*dimensions.y_dim/smally)!=bigy*dimensions.y_dim){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("y shrinkage must be a factor of the y dimension (%u)\n",dimensions.y_dim);
			free_function(datain,dataout);
			return(AIR_MAGNIFY_MOD_ERROR);
		}
		if(smallz*(bigz*dimensions.z_dim/smallz)!=bigz*dimensions.z_dim){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("z shrinkage must be a factor of the z dimension (%u)\n",dimensions.z_dim);
			free_function(datain,dataout);
			return(AIR_MAGNIFY_MOD_ERROR);
		}

		{
			AIR_Error errcode;
			
			dataout=AIR_magnifier(datain,&dimensions,&new_dimensions,bigx,bigy,bigz,smallx,smally,smallz, &errcode);
			if(!dataout){
				free_function(datain,dataout);
				return(errcode);
			}
		}
		{
			AIR_Error errcode=AIR_save(dataout,&new_dimensions,output,ow,program);
			if(errcode!=0){
				free_function(datain,dataout);
				return(errcode);
			}
		}
		free_function(datain,dataout);
	}
	return 0;
}
