/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 5/22/01 */

/*
 * This program will resize the matrix of a file
 * shifting the data as specified in the process
 *
 */

#include "AIR.h"

AIR_Error AIR_do_resize(const char *program, const char *input, const char *output, const char *air_file, const unsigned int x_dim, const unsigned int y_dim, const unsigned int z_dim, const signed int x_shift, const signed int y_shift, const signed int z_shift, const AIR_Boolean ow)

{
	if(x_dim==0 || y_dim==0 || z_dim==0){
		return(AIR_USER_INTERFACE_ERROR);
	}
	if(strlen(input)>=(size_t)AIR_CONFIG_MAX_PATH_LENGTH){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("File name %s is too long to incorporate into a .air file\n",input);
		return(AIR_PATH_TOO_LONG_ERROR);
	}
	if(strlen(output)>=(size_t)AIR_CONFIG_MAX_PATH_LENGTH){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("File name %s is too long to incorporate into a .air file\n",output);
		return(AIR_PATH_TOO_LONG_ERROR);
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
		AIR_Pixels ***datain;
		struct AIR_Air16 air1;

		strcpy(air1.r_file,input);
		{
			AIR_Error errcode;

			datain=AIR_load(air1.r_file, &air1.r, TRUE, &errcode);
			if (!datain){
				return(errcode);
			}
		}

		air1.s.bits=air1.r.bits;
		air1.s.x_dim=air1.r.x_dim;
		air1.s.y_dim=air1.r.y_dim;
		air1.s.z_dim=air1.r.z_dim;
		air1.s.x_size=air1.r.x_size;
		air1.s.y_size=air1.r.y_size;
		air1.s.z_size=air1.r.z_size;

		{
			AIR_Error errcode;
			
			AIR_Pixels ***temp=AIR_resizer(datain,&air1.s,&air1.s,x_dim,y_dim,z_dim,x_shift,y_shift,z_shift,&errcode);
			if(!temp){
				AIR_free_vol3(datain);
				return(errcode);
			}
			AIR_free_vol3(datain);
			datain=temp;
		}

		strcpy(air1.s_file,output);
		{
			AIR_Error errcode=AIR_save(datain,&air1.s,air1.s_file,ow,program);
			
			if(errcode!=0){
				AIR_free_vol3(datain);
				return(errcode);
			}
		}
		AIR_free_vol3(datain);

		if(air_file){
		
			double *e[4];
			
			air1.r_hash=1;
			air1.s_hash=1;

			e[0]=air1.e[0];
			e[1]=air1.e[1];
			e[2]=air1.e[2];
			e[3]=air1.e[3];

			e[0][0]=e[1][1]=e[2][2]=e[3][3]=1.0;
			e[1][0]=e[2][0]=0.0;
			e[0][1]=e[2][1]=0.0;
			e[0][2]=e[1][2]=0.0;
			e[0][3]=e[1][3]=e[2][3]=0.0;

			e[3][0]=(double)x_shift;
			e[3][1]=(double)y_shift;
			e[3][2]=(double)z_shift;

			{
				double pixel_size_s=air1.s.x_size;
				if(air1.s.y_size<pixel_size_s) pixel_size_s=air1.s.y_size;
				if(air1.s.z_size<pixel_size_s) pixel_size_s=air1.s.z_size;

				{
					unsigned int i;
					
					for(i=0;i<4;i++){
						e[0][i]/=(air1.s.x_size/pixel_size_s);
						e[1][i]/=(air1.s.y_size/pixel_size_s);
						e[2][i]/=(air1.s.z_size/pixel_size_s);
					}
				}
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
				AIR_Error errcode=AIR_write_air16(air_file,TRUE,e,TRUE,&air1);
				
				if(errcode!=0){
					return(errcode);
				}
			}
		}
	}
	return 0;
}
