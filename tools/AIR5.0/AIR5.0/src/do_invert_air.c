/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 5/23/01 */

/*
 * This program will invert a .air file. In other words
 *  if the input .air file provides instructions for reslicing
 *  file A to match file B, the output of this program
 *  will be a file with instructions for reslicing file B
 *  to match file A
 */

#include "AIR.h"

AIR_Error AIR_do_invert_air(const char *program, const char *input, const char *output, const AIR_Boolean ow)

{
	if(strstr(output,AIR_CONFIG_IMG_SUFF)!=NULL||strstr(output,AIR_CONFIG_HDR_SUFF)!=NULL){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("\nName of output .air file cannot contain %s or %s\n",AIR_CONFIG_IMG_SUFF,AIR_CONFIG_HDR_SUFF);
		return(AIR_USER_INTERFACE_ERROR);
	}

	/* Precheck of output permission */
	{
		AIR_Error errcode=AIR_save_probw(output,ow);
		if(errcode!=0){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("Computations not performed due to anticipated output problems with %s\n",output);
			return(errcode);
		}
	}
	{
		/* Load air file */
		struct AIR_Air16 air1;
		{
			AIR_Error errcode=AIR_read_air16(input,&air1);
			if(errcode!=0){
				return(errcode);
			}
		}
		
		{
			double f0[16];
			double *f[4];
			double *e[4];
			
			e[0]=air1.e[0];
			e[1]=air1.e[1];
			e[2]=air1.e[2];
			e[3]=air1.e[3];

			f[0]=f0;
			f[1]=f0+4;
			f[2]=f0+8;
			f[3]=f0+12;

			/* Mathematical inversion of e into f */
			{
				AIR_Error errcode=AIR_inverter(e,f,TRUE,&air1.s,&air1.r);
				if(errcode!=0){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("Inversion failed\n");
					return(errcode);
				}
			}
			/* Bookkeeping inversion */
			{
				char temp[AIR_CONFIG_MAX_PATH_LENGTH];
				air1.r_file[AIR_CONFIG_MAX_PATH_LENGTH-1]='\0';
				air1.s_file[AIR_CONFIG_MAX_PATH_LENGTH-1]='\0';
				strcpy(temp,air1.r_file);
				strcpy(air1.r_file,air1.s_file);
				strcpy(air1.s_file,temp);
			}
			{
				struct AIR_Key_info tempinfo=air1.r;
				air1.r=air1.s;
				air1.s=tempinfo;
			}
			{
				unsigned long int templong=air1.r_hash;
				air1.r_hash=air1.s_hash;
				air1.s_hash=templong;
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
				AIR_Error errcode=AIR_write_air16(output,ow,f,TRUE,&air1);
				if(errcode!=0){
					return(errcode);
				}
			}
		}
	}
	return(0);
}
