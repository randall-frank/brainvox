/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 5/23/01 */

/*
 * This program will create a single plane layout based on the input data.
 */


#include "AIR.h"

AIR_Error AIR_do_layout(const char *program, const char *input, const char *output, const char orientation, const unsigned int width, const unsigned int height, const unsigned int z_start, const signed int z_step, const AIR_Boolean ow)

{
	/* Make sure orientation is valid */
	if(orientation!='t' && orientation!='c' && orientation!='s'){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("%c is not a valid choice for orientation\n",orientation);
		return(AIR_USER_INTERFACE_ERROR);
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
		struct AIR_Key_info dimensions;
		AIR_Pixels ***datain;
		
		{
			AIR_Error errcode;

			datain=AIR_load(input, &dimensions, TRUE, &errcode);
			if (!datain){
				return(errcode);
			}
		}

		/*Reorient data for sagittal or coronal sections*/
		if(orientation=='s'){
			{
				AIR_Error errcode;
				
				AIR_Pixels ***temp=AIR_swap_x_and_z(datain,&dimensions,&dimensions,&errcode);
				if(!temp){
					AIR_free_vol3(datain);
					return(errcode);
				}
				AIR_free_vol3(datain);
				datain=temp;
			}
			{
				AIR_Error errcode;
				
				AIR_Pixels ***temp=AIR_flip_x(datain,&dimensions,&errcode);
				if(!temp){
					AIR_free_vol3(datain);
					AIR_report_error(errcode);
					exit(EXIT_FAILURE);
				}
				AIR_free_vol3(datain);
				datain=temp;
			}
			{
				AIR_Error errcode;
				
				AIR_Pixels ***temp=AIR_swap_x_and_y(datain,&dimensions,&dimensions,&errcode);
				if(!temp){
					AIR_free_vol3(datain);
					return(errcode);
				}
				AIR_free_vol3(datain);
				datain=temp;
			}
			{
				AIR_Error errcode;
				
				AIR_Pixels ***temp=AIR_flip_y(datain,&dimensions,&errcode);
				if(!temp){
					AIR_free_vol3(datain);
					return(errcode);
				}
				AIR_free_vol3(datain);
				datain=temp;
			}
		}
		else if(orientation=='c'){
			{
				AIR_Error errcode;
				
				AIR_Pixels ***temp=AIR_swap_y_and_z(datain,&dimensions,&dimensions,&errcode);
				if(!temp){
					AIR_free_vol3(datain);
					return(errcode);
				}
				AIR_free_vol3(datain);
				datain=temp;
			}
		}

		{
			AIR_Error errcode;
			
			AIR_Pixels ***temp=AIR_layerout(datain,&dimensions,&dimensions,width,height,z_start,z_step,&errcode);
			if(!temp){
				AIR_free_vol3(datain);
				return(errcode);
			}
			AIR_free_vol3(datain);
			datain=temp;
		}
		{
			AIR_Error errcode=AIR_save(datain,&dimensions,output,ow,program);
			
			if(errcode!=0){
				AIR_free_vol3(datain);
				return(errcode);
			}
		}
		AIR_free_vol3(datain);
	}
	return 0;
}
