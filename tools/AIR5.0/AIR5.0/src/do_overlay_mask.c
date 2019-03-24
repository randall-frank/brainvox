/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 5/23/01 */

/*
 * This routine will set voxels at or above threshold in mask to the maximum possible value in the base
 */


#include "AIR.h"

static void free_function(/*@null@*/ /*@out@*/ /*@only@*/ AIR_Pixels ***datain, /*@null@*/ /*@out@*/ /*@only@*/ AIR_Pixels ***datain2)

{
	if(datain) AIR_free_vol3(datain);
	if(datain2) AIR_free_vol3(datain2);
}

AIR_Error AIR_do_overlay_mask(const char *program, const char *mask_file, const char *base_file, const char *output, const long int threshold, const AIR_Boolean ow)
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
		AIR_Pixels mapped_threshold;
		
		if(threshold==(AIR_CONFIG_IMPOSSIBLE_THRESHOLD)){
			mapped_threshold=1;
		}
		else{
			AIR_Error errcode;
			mapped_threshold=AIR_map_value(mask_file,threshold,&errcode);
			if(errcode!=0){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("Unable to map threshold to internal representation value\n");
				return(errcode);		
			}
		}
		
		{
			AIR_Pixels ***datain=NULL;
			AIR_Pixels ***datain2=NULL;
	
			struct AIR_Key_info dimensions;
			struct AIR_Key_info dimensions2;
	
			/* Load the file to overlay */
			{
				AIR_Error errcode;
	
				datain=AIR_load(mask_file, &dimensions, TRUE, &errcode);
				if(!datain){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("Unable to load file %s\n",mask_file);
					free_function(datain,datain2);
					return(errcode);
				}
			}
	
			/* Load the file on which to overlay */
			{
				AIR_Error errcode;
	
				datain2=AIR_load(base_file, &dimensions2, TRUE, &errcode);
				if(!datain2){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("Unable to load file %s\n",base_file);
					free_function(datain,datain2);
					return(errcode);
				}
			}
	
			/* Verify compatibility */
			{
				AIR_Error errcode=AIR_same_dim(&dimensions,&dimensions2);
				
				if(errcode!=0){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("input files %s and %s have different dimensions\n",mask_file,base_file);
					free_function(datain,datain2);
					return(errcode);
				}
			}
	
			if(AIR_same_size(&dimensions,&dimensions2)!=0){
				printf("WARNING: input files %s and %s have different voxel sizes\n",mask_file,base_file);
			}
	
			{
				unsigned int k;
				
				for(k=0;k<dimensions.z_dim;k++){
				
					unsigned int j;
					
					for(j=0;j<dimensions.y_dim;j++){
					
						unsigned int i;
						
						for(i=0;i<dimensions.x_dim;i++){
							if(datain[k][j][i]>=mapped_threshold) datain2[k][j][i]=AIR_CONFIG_MAX_POSS_VALUE;
						}
					}
				}
			}
			{
				AIR_Error errcode=AIR_save(datain2,&dimensions2,output,ow,program);
				
				if(errcode!=0){
					free_function(datain,datain2);
					return(errcode);
				}
			}
			free_function(datain,datain2);
		}
	}
	return 0;
}

