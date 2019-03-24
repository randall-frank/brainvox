/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 5/22/01 */

/*
 * This program will separate a 3D file into multiple 2D files
 *
 * Output files are numbered automatically
 *
 */

#include "AIR.h"

AIR_Error AIR_do_separate(const char *program, const char *input, const char *outprefix, const AIR_Boolean ow)

{
	struct AIR_Key_info 	dimensions;
	AIR_Pixels 		***datain;
	
	{
		AIR_Error errcode;

		datain=AIR_load(input, &dimensions, TRUE, &errcode);
		if(!datain){
			return(errcode);
		}
	}
	{
		/* Figure out how many digits are needed to number the files */
		unsigned int digits=1;
		{
			unsigned int number=dimensions.z_dim;
			while(number>9){
				number/=10;
				digits++;
			}
		}
		{
			char *filename=malloc(strlen(outprefix)+digits+1);
			if(!filename){
				AIR_free_vol3(datain);
				return(AIR_MEMORY_ALLOCATION_ERROR);
			}
			{
				AIR_Pixels ***k2=datain+dimensions.z_dim;
				dimensions.z_dim=1;
				{
					AIR_Pixels ***k1;
					unsigned int k;
					
					for (k=0,k1=datain;k1<k2;k++,k1++){

						sprintf(filename,"%s%0*u",outprefix,(int)digits,k+1);
						{
							AIR_Error errcode=AIR_save(k1,&dimensions,filename,ow,program);
							
							if(errcode!=0){
								printf("%s: %d: ",__FILE__,__LINE__);
								printf("%s failed to save file %s\n",program,filename);
								AIR_free_vol3(datain);
								free(filename);
								return(errcode);
							}
						}
					}
				}
			}
			free(filename);
		}
	}
	AIR_free_vol3(datain);
	return 0;
}
