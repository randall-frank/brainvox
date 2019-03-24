/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 5/19/01 */

/*
 * This program will combine multiple 2D files into a single 3D file					
 */

#include "AIR.h"

AIR_Error AIR_do_reunite(const char *program, const char *output, const unsigned int n, char **inputs, const AIR_Boolean ow)

{
	if(n==0){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("At least one input file must be specified\n");
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

		/*Make sure that all of the files are of compatible dimension and consist of single planes*/
		/* Get the first file header */
		{
			struct AIR_Fptrs fps;
			{
				int dummy[8];
				(void)AIR_open_header(inputs[0],&fps,&dimensions,dummy);
			}
			if(fps.errcode!=0){
				AIR_Error errcode=fps.errcode;
				AIR_close_header(&fps);
				return(errcode);
			}
			AIR_close_header(&fps);
			if(fps.errcode!=0){
				return(fps.errcode);
			}
		}
		if(dimensions.z_dim!=1){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("File %s has has more than one plane\n",inputs[0]);
			return(AIR_MULTI_PLANE_IMAGE_ERROR);
		}
		/* Compare subsequent headers to the first */
		{
			unsigned int k;
						
			for(k=1;k<n;k++){
			
				struct AIR_Key_info dimensions2;
				{
					struct AIR_Fptrs fps;
					{
						int dummy[8];
						(void)AIR_open_header(inputs[k],&fps,&dimensions2,dummy);
					}
					if(fps.errcode!=0){
						AIR_Error errcode=fps.errcode;
						AIR_close_header(&fps);
						return(errcode);
					}
					AIR_close_header(&fps);
					if(fps.errcode!=0){
						return(fps.errcode);
					}
				}
				{
					AIR_Error errcode=AIR_same_dim(&dimensions,&dimensions2);
					if(errcode!=0){
						printf("%s: %d: ",__FILE__,__LINE__);
						printf("dimensions incompatibility for file %s:%u %u %u instead of %u %u %u\n",inputs[k],dimensions2.x_dim,dimensions2.y_dim,dimensions2.z_dim,dimensions.x_dim,dimensions.y_dim,dimensions.z_dim);
						return(errcode);
					}
				}
			}
		}

		dimensions.z_dim=n;
		{
			AIR_Pixels ***datain=AIR_create_vol3(dimensions.x_dim,dimensions.y_dim,dimensions.z_dim);
			if(!datain){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("Memory allocation error\n");
				return(AIR_MEMORY_ALLOCATION_ERROR);
			}
			{
				unsigned int k;
				
				for(k=0;k<n;k++){

					AIR_Error errcode=AIR_quickread(datain[k][0],inputs[k],0);
					if(errcode!=0){
						AIR_free_vol3(datain);
						return(errcode);
					}
				}
			}
			/*Following required to allow different input and output bits*/
			/* because quickread cannot be allowed to change the bits */
			dimensions.bits=8*sizeof(AIR_Pixels); 
			{
				AIR_Error errcode=AIR_save(datain,&dimensions,output,ow,program);
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

