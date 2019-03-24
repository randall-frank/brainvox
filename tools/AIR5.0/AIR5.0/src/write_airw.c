/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/10/01 */

/*
 * writes an nonlinear AIR4 reslice parameter file
 *
 * returns:
 *	0 if successful
 *	error code if unsuccessful
 */

#include "AIR.h"

AIR_Error AIR_write_airw(const char *filename, const AIR_Boolean ow, double **e, const struct AIR_Warp *air1)

{
	/*Open output file if permitted to do so*/
	if(!ow){
		AIR_Error errcode=AIR_fprobw(filename,ow);
		if(errcode!=0){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("Failed to write output .warp file %s\n",filename);
			return errcode;
		}
	}
	{
		FILE *fp=fopen(filename,"wb");
		if(!fp){

			printf("%s: %d: ",__FILE__,__LINE__);
			printf("Failed to write output .warp file %s\n",filename);
			{
				AIR_Error errcode=AIR_fprobw(filename,ow);
				if(errcode==0) return AIR_UNSPECIFIED_FILE_WRITE_ERROR;
				return errcode;
			}
		}
	 
		/*Write out warp struct*/
		if(fwrite(air1,sizeof(struct AIR_Warp),1,fp)!=1){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("error writing warp file %s\n",filename);
			if(fclose(fp)!=0){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("failed to close warp file %s\n",filename);
				return AIR_CANT_CLOSE_WRITE_ERROR;
			}			
			return AIR_WRITE_WARP_FILE_ERROR;
		}
		/* Write out the matrix */
		{
			unsigned int j;
			
			for(j=0;j<air1->coord;j++){
			
				if(fwrite(e[j],sizeof(double),(size_t)air1->coeffp,fp)!=(size_t)air1->coeffp){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("error writing warp file %s\n",filename);
					if(fclose(fp)!=0){
						printf("%s: %d: ",__FILE__,__LINE__);
						printf("failed to close warp file %s\n",filename);
						return AIR_CANT_CLOSE_WRITE_ERROR;
					}					
					return AIR_WRITE_WARP_FILE_ERROR;
				}
			}
		}		
		
		if(fclose(fp)!=0){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("failed to close warp file %s\n",filename);
			return AIR_CANT_CLOSE_WRITE_ERROR;
		}
	}
	return 0;
}
