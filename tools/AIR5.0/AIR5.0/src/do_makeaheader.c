/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/19/01 */

/* This program will generate a header for an image file */

#include "AIR.h"

AIR_Error AIR_do_makeaheader(const char *output, const unsigned int filetype, const unsigned int x_dim, const unsigned int y_dim, const unsigned int z_dim, const double x_size, const double y_size, const double z_size, const AIR_Boolean ow)

{
	if(x_size <=0.0 || y_size <=0.0 || z_size <=0.0){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("Sizes must be greater than zero\n");
		return(AIR_INFO_SIZE_ERROR);
	}
	/* Precheck of output permissions */
	{
		int errcode=AIR_save_probw(output,ow);
		if(errcode!=0){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("Computations not performed due to anticipated output problems with %s\n",output);
			return(errcode);
		}
	}
	{
		struct AIR_Key_info dimensions;
		int dummy[8];
		
		if(filetype==0){
			dimensions.bits=8;
			dummy[0]=0;
			dummy[1]=255;
		}
		else if(filetype==1){
			dimensions.bits=16;
			dummy[0]=0;
			dummy[1]=65535;
		}
		else if(filetype==2){
			dimensions.bits=16;
			dummy[0]=0;
			dummy[1]=32767;
		}
		else if(filetype==3){
			dimensions.bits=16;
			dummy[0]=-32767;
			dummy[1]=32767;
		}
		else{
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("file type %u not defined\n",filetype);
			return(AIR_USER_INTERFACE_ERROR);
		}

		dimensions.x_dim=x_dim;
		dimensions.y_dim=y_dim;
		dimensions.z_dim=z_dim;

		dimensions.x_size=x_size;
		dimensions.y_size=y_size;
		dimensions.z_size=z_size;

		{
			AIR_Error errcode=AIR_write_header(output,&dimensions,"units specified",dummy);
			if(errcode!=0){
				AIR_report_error(errcode);
				exit(errcode);
			}
		}
	}
	return 0;
}
