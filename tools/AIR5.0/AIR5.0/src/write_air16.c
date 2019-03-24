/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/30/01 */

/*
 * writes an AIR reslice parameter file
 * adjusts e matrix to cubic voxel default when zooming==0
 * assumes adjustment has already been made when zooming!=0
 *
 * returns:
 *	0 if successful
 *	error code if unsuccessful
 */

#include "AIR.h"

AIR_Error AIR_write_air16(const char *filename, const AIR_Boolean ow, double **e, const AIR_Boolean zooming, const struct AIR_Air16 *air1)

{
	struct AIR_Air16	air2=*air1;

	/* Fill in the matrix */
	{
		unsigned int j;

		for (j=0;j<4;j++){
		
			unsigned int i;

			for(i=0;i<4;i++){
				air2.e[j][i]=e[j][i];
			}
		}
	}

	if(!zooming){
		/* Adjust matrix for storage */

		double pixel_size_s=air2.s.x_size;
		if(air2.s.y_size<pixel_size_s) pixel_size_s=air2.s.y_size;
		if(air2.s.z_size<pixel_size_s) pixel_size_s=air2.s.z_size;

		if(fabs(air2.s.x_size-pixel_size_s)>AIR_CONFIG_PIX_SIZE_ERR){
			air2.e[0][0]/=(air2.s.x_size/pixel_size_s);
			air2.e[0][1]/=(air2.s.x_size/pixel_size_s);
			air2.e[0][2]/=(air2.s.x_size/pixel_size_s);
			air2.e[0][3]/=(air2.s.x_size/pixel_size_s);
		}

		if(fabs(air2.s.y_size-pixel_size_s)>AIR_CONFIG_PIX_SIZE_ERR){
			air2.e[1][0]/=(air2.s.y_size/pixel_size_s);
			air2.e[1][1]/=(air2.s.y_size/pixel_size_s);
			air2.e[1][2]/=(air2.s.y_size/pixel_size_s);
			air2.e[1][3]/=(air2.s.y_size/pixel_size_s);
		}

		if(fabs(air2.s.z_size-pixel_size_s)>AIR_CONFIG_PIX_SIZE_ERR){
			air2.e[2][0]/=(air2.s.z_size/pixel_size_s);
			air2.e[2][1]/=(air2.s.z_size/pixel_size_s);
			air2.e[2][2]/=(air2.s.z_size/pixel_size_s);
			air2.e[2][3]/=(air2.s.z_size/pixel_size_s);
		}
	}

	/* Write the .air file */
	{
		/*Open output file if permitted to do so*/
		if(!ow){
			AIR_Error errcode=AIR_fprobw(filename,ow);
			if(errcode!=0){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("Failed to write output .air file %s\n",filename);
				return errcode;
			}
		}
		{
			FILE *fp=fopen(filename,"wb");
			if(!fp){

				printf("%s: %d: ",__FILE__,__LINE__);
				printf("Failed to write output .air file %s\n",filename);
				{
					AIR_Error errcode=AIR_fprobw(filename,ow);
					if(errcode==0) return AIR_UNSPECIFIED_FILE_WRITE_ERROR;
					return errcode;
				}
			}
	 
			/*Write out air struct*/
			if(fwrite(&air2,1,sizeof(struct AIR_Air16),fp)!=sizeof(struct AIR_Air16)){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("error writing .air file %s\n",filename);
				if(fclose(fp)!=0){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("failed to close .air file %s\n",filename);
					return AIR_CANT_CLOSE_WRITE_ERROR;
				}				
				return AIR_WRITE_AIR_FILE_ERROR;
			}
			if(fclose(fp)!=0){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("failed to close .air file %s\n",filename);
				return AIR_CANT_CLOSE_WRITE_ERROR;
			}
		}
	}
	return 0;
}
