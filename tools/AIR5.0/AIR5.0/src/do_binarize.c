/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 5/9/01 */

/*
 * This program will create a binary mask of the input
 *  data with all values between min and max inclusive
 *  converted to 1's and all other values converted to 0's
 *
 * Note that user supplied values for max and min are
 * 	processed using map_value() within this subroutine.
 *
 * It is legal to have max<min, but the result is a binary file of zeros
 *
 */

#include "AIR.h"

AIR_Error AIR_do_binarize(const char *program, const char *input, const char *output, const AIR_Boolean use_default_min, long int min, AIR_Boolean use_default_max, long int max, const AIR_Boolean ow)

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
		AIR_Pixels mapped_min, mapped_max;
		
		if(use_default_min) mapped_min=1;
		else{
			AIR_Error errcode;
			mapped_min=AIR_map_value(input,min,&errcode);
			if(errcode!=0){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("Unable to map min to internal representation value\n");
				return(errcode);
			}		
		}
		
		if(use_default_max) mapped_max=AIR_CONFIG_MAX_POSS_VALUE;
		else{
			AIR_Error errcode;
			mapped_max=AIR_map_value(input,max,&errcode);
			if(errcode!=0){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("Unable to map max to internal representation value\n");
				return(errcode);
			}
		}
		{
			AIR_Pixels ***datain;
			struct AIR_Key_info dimensions;
			
			{
				AIR_Error errcode;
				
				datain=AIR_load(input, &dimensions, 0, &errcode);
				if(!datain){
					return(errcode);
				}
			}
			{
				AIR_Error errcode=AIR_saveas1(datain, &dimensions, output, ow, mapped_min, mapped_max, program);
				
				if(errcode!=0){
					AIR_free_vol3(datain);
					return(errcode);
				}
			}
			AIR_free_vol3(datain);
		}
	}
	return 0;
}
