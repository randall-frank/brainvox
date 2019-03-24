/* Copyright 1997-2001 Roger P. Woods, M.D. */
/* Modified: 5/22/01 */

/*
 * This program will combine a nonlinear .warp file
 * with affine .air files to produce a single nonlinear .warp file that will
 * have the same net spatial result.
 *
 * This avoids the need to repeatedly reslice a file to
 *  various locations, and also prevents the accumulation
 *  of interpolation errors.
 *
 */

#include "AIR.h"

AIR_Error AIR_do_combine_warp(const char *program, const char *outfile, const unsigned int n, char **warp_files, const AIR_Boolean ow)
{
	if(strstr(outfile,AIR_CONFIG_IMG_SUFF)!=NULL||strstr(outfile,AIR_CONFIG_HDR_SUFF)!=NULL){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("\nName of output .air file cannot contain %s or %s\n",AIR_CONFIG_IMG_SUFF,AIR_CONFIG_HDR_SUFF);
		return(AIR_USER_INTERFACE_ERROR);
	}
	if(n<1){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("At least one file must be specified\n");
		return(AIR_USER_INTERFACE_ERROR);
	}
	/* Precheck of output permissions */
	{
		AIR_Error errcode=AIR_save_probw(outfile,ow);
		
		if(errcode!=0){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("Computations not performed due to anticipated output problems with %s\n",outfile);
			return(errcode);
		}
	}
	{
		struct AIR_Warp air1;		
		double **e;

		/* Get the first file */
		{
			AIR_Error errcode;
			
			e=AIR_read_airw(warp_files[0],&air1,&errcode);
			if(!e){
				return(errcode);
			}
		}
		{
			unsigned int i;
			for(i=1;i<n;i++){
			
				struct AIR_Warp air2;
				double **f;
				{
					AIR_Error errcode;
					
					f=AIR_read_airw(warp_files[i],&air2,&errcode);
					if(!f){
						AIR_free_2(e);
						return(errcode);
					}
				}
				if(air2.order==1){
					AIR_Error errcode=AIR_combinewarp_right(&air1, &e, air2, f);
					if(errcode!=0){
						AIR_free_2(e);
						AIR_free_2(f);
						return(errcode);
					}
					AIR_free_2(f);
				}
				else{
					if(air1.order==1){
						AIR_Error errcode=AIR_combinewarp_left(air1,e,&air2,&f);
						if(errcode!=0){
							AIR_free_2(e);
							AIR_free_2(f);
							return(errcode);
						}
						/* Move result to the left */
						air1=air2;
						AIR_free_2(e);
						e=f;
					}
					else{
						printf("Two or more nonlinear .warp files cannot be combined\n");
						AIR_free_2(e);
						AIR_free_2(f);
						return(1);
					}
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
			AIR_Error errcode=AIR_write_airw(outfile,ow,e,&air1);
			if(errcode!=0){
				AIR_free_2(e);
				return(errcode);
			}
		}
		AIR_free_2(e);
	}
	return 0;
}

