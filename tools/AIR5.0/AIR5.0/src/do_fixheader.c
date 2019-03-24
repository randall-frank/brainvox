/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/29/01 */

/*
 * This program will change the size of a voxel as defined
 *  in the header file.
 */

#include "AIR.h"

AIR_Error AIR_do_fixheader(const char *filename, const AIR_Boolean keepsizes, const double xsize, const double ysize, const double zsize)

{
	if(!keepsizes){
		if(xsize<=0.0 || ysize<=0.0 || zsize<=0.0){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("Voxel sizes must be positive\n");
			return(AIR_INFO_SIZE_ERROR);
		}
	}
	{
		int dummy[8];
		struct AIR_Key_info dimensions;

		{
			struct AIR_Fptrs fps;
			
			(void)AIR_open_header(filename,&fps,&dimensions,dummy);
			if(fps.errcode!=0){
				if(fps.errcode==AIR_INFO_DIM_ERROR){
					printf("Fixing negative values in %s\n",filename);
				}
				else if(fps.errcode==AIR_INFO_SIZE_ERROR){
					printf("Fixing negative values in %s\n",filename);
				}
				else{
					AIR_Error errcode=fps.errcode;
					AIR_close_header(&fps);
					return(errcode);
				}
			}
			AIR_close_header(&fps);
			if(fps.errcode!=0){
				return(fps.errcode);
			}
		}
		if(!keepsizes){
			dimensions.x_size=xsize;
			dimensions.y_size=ysize;
			dimensions.z_size=zsize;
		}
		{
			AIR_Error errcode=AIR_write_header(filename,&dimensions,"units specified",dummy);
			if(errcode!=0){
				return(errcode);
			}
		}
	}
	return 0;
}
