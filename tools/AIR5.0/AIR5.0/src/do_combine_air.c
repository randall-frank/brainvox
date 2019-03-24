/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 5/22/01 */

/*
 * This program will combine multiple sequential .air files
 *  to produce a single .air file that will have the same net
 *  spatial result.
 *
 * This avoids the need to repeatedly reslice a file to
 *  various locations, and also prevents the accumulation
 *  of interpolation errors.
 *
 * The program does some error checking to catch blatantly
 *  obvious incorrect usage.
 */

#include "AIR.h"

AIR_Error AIR_do_combine_air(const char *program, const unsigned int n, char **inputs, const char *output, const AIR_Boolean ow)

{
	if(strstr(output,AIR_CONFIG_IMG_SUFF)!=NULL||strstr(output,AIR_CONFIG_HDR_SUFF)!=NULL){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("\nName of output .air file cannot contain %s or %s\n",AIR_CONFIG_IMG_SUFF,AIR_CONFIG_HDR_SUFF);
		return(AIR_USER_INTERFACE_ERROR);
	}
	if(n<1){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("No files were provided to combine\n");
		return(AIR_USER_INTERFACE_ERROR);
	}

	{
		struct AIR_Air16	air1;
		
		/*Get the first .air file*/
		{
			AIR_Error errcode=AIR_read_air16(inputs[0],&air1);
			if(errcode!=0){
				return(errcode);
			}
		}
		{
			unsigned int i;
			
			for(i=1;i<n;i++){
			
				struct AIR_Air16 air2;

				/*Get the next .air file*/
				{
					AIR_Error errcode=AIR_read_air16(inputs[i],&air2);
					if(errcode!=0){
						return(errcode);
					}
				}

				/*Replace the first .air file with combination*/
				{
					AIR_Error errcode=AIR_combineair(&air1,&air2,&air1);
					if(errcode!=0){
						return(errcode);
					}
				}
				air1.r_hash=air2.r_hash;
			}
		}

		/* Copy this program name into air1.comment */
		if(strlen(program)<(size_t)AIR_CONFIG_MAX_COMMENT_LENGTH){
			strcpy(air1.comment,program);
		}
		else{
			const char *shortname=AIR_file_from_path(program);
			strncpy(air1.comment,shortname,(size_t)AIR_CONFIG_MAX_COMMENT_LENGTH-1);
			air1.comment[AIR_CONFIG_MAX_COMMENT_LENGTH-1]='\0';
		}
		/* Save the output */
		{
			double *e[4];
			
			e[0]=air1.e[0];
			e[1]=air1.e[1];
			e[2]=air1.e[2];
			e[3]=air1.e[3];
			{
				AIR_Error errcode=AIR_write_air16(output,ow,e,TRUE,&air1);
				if(errcode!=0){
					return(errcode);
				}
			}
		}
	}
	return 0;
}

