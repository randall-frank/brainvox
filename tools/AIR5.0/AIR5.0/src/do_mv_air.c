/* Copyright 1997-2001 Roger P. Woods, M.D. */
/* Modified: 5/22/01 */

/*
 * This program will change the file to be resliced in a .air
 *  file. The full path name is replaced			
 */

#include "AIR.h"

AIR_Error AIR_do_mv_air(const char *air_file, const char *new_reslice_file)

{
	if(strlen(new_reslice_file)>=(size_t)AIR_CONFIG_MAX_PATH_LENGTH){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("File name %s is too long to incorporate into a .air file\n",new_reslice_file);
		return(AIR_PATH_TOO_LONG_ERROR);
	}
	{
		struct AIR_Air16 air1;
		{
			AIR_Error errcode=AIR_read_air16(air_file,&air1);
			if(errcode!=0){
				return(errcode);
			}
		}

		/* Preserve record of prior reslice file setting in comment */
		strncpy(air1.comment,air1.r_file,(size_t)AIR_CONFIG_MAX_COMMENT_LENGTH-1);
		air1.comment[AIR_CONFIG_MAX_COMMENT_LENGTH-1]='\0';

		strcpy(air1.r_file,new_reslice_file);

		/*Test to see if reslice file exists, if not, print warning*/
		/* If it exists check compatibility */
		{
			struct AIR_Key_info new_info;
			struct AIR_Fptrs fps;
			{
				int flag[8];
				
				(void)AIR_open_header(air1.r_file,&fps,&new_info,flag);
			}
			if(fps.errcode!=0){
				AIR_close_header(&fps);
				if(fps.errcode!=0){
					return(fps.errcode);
				}
				printf("WARNING: .air file '%s' now targets a nonexistent or unreadable reslice file: '%s'\n",air_file,air1.r_file);
			}
			else{
				AIR_close_header(&fps);
				if(fps.errcode!=0){
					return(fps.errcode);
				}

				{
					AIR_Error errcode=AIR_same_dim(&air1.r,&new_info);
					if(errcode!=0){
						printf("%s: %d: ",__FILE__,__LINE__);
						printf("%s not modified due to matrix size discrepancy\n",air_file);
						printf("Dimensions of reslice matrix in .air file %s: %u %u %u\n",air_file,air1.r.x_dim,air1.r.y_dim,air1.r.z_dim);
						printf("Dimensions of matrix in image file %s: %u %u %u\n",air1.r_file,new_info.x_dim,new_info.y_dim,new_info.z_dim);
						return(errcode);
					}
				}
				{
					AIR_Error errcode=AIR_same_size(&air1.r,&new_info);
					
					if(errcode!=0){
						printf("%s: %d: ",__FILE__,__LINE__);
						printf("%s not modified due to voxel size discrepancy\n",air_file);
						printf("Voxel dimensions in .air file %s: %.4f %.4f %.4f\n",air_file,air1.r.x_size,air1.r.y_size,air1.r.z_size);
						printf("Voxel dimensions in image file %s: %.4f %.4f %.4f\n",air1.r_file,new_info.x_size,new_info.y_size,new_info.z_size);
						return(errcode);
					}
				}
			}
		}

		{
			double *e[4];
			
			e[0]=air1.e[0];
			e[1]=air1.e[1];
			e[2]=air1.e[2];
			e[3]=air1.e[3];
			
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
