/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 5/23/01 */

/* This program will apply a binary mask to a data file	*/

#include "AIR.h"

static void free_function(/*@null@*/ /*@out@*/ /*@only@*/ AIR_Pixels ***datain1, /*@null@*/ /*@out@*/ /*@only@*/ AIR_Pixels ***datain2)

{
	if(datain1) AIR_free_vol3(datain1);
	if(datain2) AIR_free_vol3(datain2);
}

AIR_Error AIR_do_binarymask(const char *program, const char *input, const char *maskfile, const char *output, const AIR_Boolean ow)

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
		struct AIR_Key_info dimensions1;
		struct AIR_Key_info dimensions2;
		
		AIR_Pixels ***datain1=NULL;
		AIR_Pixels ***datain2=NULL;
		
		{
			AIR_Error errcode;

			datain1=AIR_load(input, &dimensions1, TRUE, &errcode);
			if (!datain1){
				free_function(datain1,datain2);
				return(errcode);
			}
		}
		{
			AIR_Error errcode;
			
			datain2=AIR_load1(maskfile,&dimensions2, &errcode);
			if (!datain2){
				free_function(datain1,datain2);
				return(errcode);
			}
		}

		/*Note that trucation permission is granted in call to function mask() */
		{
			AIR_Error errcode=AIR_mask(datain1,&dimensions1,datain2,&dimensions2, TRUE);
			if(errcode!=0){
				free_function(datain1,datain2);
				return(errcode);
			}
		}
		{
			AIR_Error errcode=AIR_save(datain1,&dimensions1,output,ow,program);
			if(errcode!=0){
				free_function(datain1,datain2);
				return(errcode);
			}
		}

		free_function(datain1,datain2);
	}
	return 0;
}
