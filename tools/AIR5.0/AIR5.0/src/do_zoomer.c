/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 5/23/01 */

/*
 * This program will take an input image and create a corresponding
 *  output image that has been interpolated to cubic voxels.
 *
 * It is implemented to parallel the interpolation to cubic voxels
 *  that takes place with the reslice program.
 *
 */

#include "AIR.h"

static void free_function(/*@null@*/ /*@out@*/ /*@only@*/ AIR_Pixels ***datain, /*@null@*/ /*@out@*/ /*@only@*/ AIR_Pixels ***dataout)

{
	if(datain) AIR_free_vol3(datain);
	if(dataout) AIR_free_vol3(dataout);
}

AIR_Error AIR_do_zoomer(const char *program, const char *input, const char *output, const char *air_file, const AIR_Boolean imageout, const AIR_Boolean ow, const AIR_Boolean airow)

{
	struct AIR_Air16 air1;
	
	if(air_file){
		if(strlen(input)>=(size_t)AIR_CONFIG_MAX_PATH_LENGTH){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("File name %s is too long to incorporate into a .air file\n",input);
			return(AIR_PATH_TOO_LONG_ERROR);
		}
		else strcpy(air1.r_file,input);
		if(strlen(output)>=(size_t)AIR_CONFIG_MAX_PATH_LENGTH){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("File name %s is too long to incorporate into a .air file\n",output);
			return(AIR_PATH_TOO_LONG_ERROR);
		}
		else strcpy(air1.s_file,output);
	}

	if(!imageout && !air_file){
		return(AIR_USER_INTERFACE_ERROR);
	}


	if(imageout){
		AIR_Pixels 	***datain=NULL;
		AIR_Pixels	***dataout=NULL;

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
			AIR_Error errcode;

			datain=AIR_load(input, &air1.r, 0, &errcode);
			if (!datain){
				free_function(datain,dataout);
				return(errcode);
			}
		}
		{
			AIR_Error errcode;
			
			dataout=AIR_zoom(datain,&air1.r,&air1.s,&errcode);
			if (!dataout){
				free_function(datain,dataout);
				return(errcode);
			}
		}
		{
			AIR_Error errcode=AIR_save(dataout,&air1.s,output,ow,program);
			
			if(errcode!=0){
				free_function(datain,dataout);
				return(errcode);
			}
		}
		free_function(datain,dataout);
	}
	else{
		struct AIR_Fptrs fps;
		int dummy[8];

		{
			AIR_Error errcode;
			
			(void)AIR_open_header(input,&fps,&air1.s,dummy);
			if(fps.errcode!=0){
				errcode=fps.errcode;
				AIR_close_header(&fps);
				return(errcode);
			}
		}
		AIR_close_header(&fps);
		if(fps.errcode!=0){
			return(fps.errcode);
		}
		{
			AIR_Error errcode;
			
			(void)AIR_zoom((AIR_Pixels ***)NULL,&air1.r,&air1.s, &errcode);
			/* Zoom does not generate a relevant error in this context */
		}
	}
	if(air_file){
		double *e[4];
		
		{
			unsigned int j;
			
			for(j=0;j<4;j++){
				e[j]=air1.e[j];
				{
					unsigned int i;
					
					for(i=0;i<4;i++){
						e[j][i]=0.0;
					}
				}
				e[j][j]=1.0;
			}
		}

		air1.r_hash=1;
		air1.s_hash=1;
		{
			double pixel_size_s=air1.r.x_size;
			if(air1.r.y_size<pixel_size_s) pixel_size_s=air1.r.y_size;
			if(air1.r.z_size<pixel_size_s) pixel_size_s=air1.r.z_size;
			if(fabs(air1.r.x_size-pixel_size_s)>AIR_CONFIG_PIX_SIZE_ERR) e[0][0]=pixel_size_s/air1.r.x_size;
			if(fabs(air1.r.y_size-pixel_size_s)>AIR_CONFIG_PIX_SIZE_ERR) e[1][1]=pixel_size_s/air1.r.y_size;
			if(fabs(air1.r.z_size-pixel_size_s)>AIR_CONFIG_PIX_SIZE_ERR) e[2][2]=pixel_size_s/air1.r.z_size;
		}
		if(strlen(program)<(size_t)AIR_CONFIG_MAX_COMMENT_LENGTH){
			strcpy(air1.comment,program);
		}
		else{
			const char *shortname=AIR_file_from_path(program);
			strncpy(air1.comment,shortname,(size_t)AIR_CONFIG_MAX_COMMENT_LENGTH-1);
			air1.comment[AIR_CONFIG_MAX_COMMENT_LENGTH-1]='\0';
		}
		{
			AIR_Error errcode=AIR_write_air16(air_file,airow,e,TRUE,&air1);
			
			if(errcode!=0){
				return(errcode);
			}
		}
	}
	return 0;
}
