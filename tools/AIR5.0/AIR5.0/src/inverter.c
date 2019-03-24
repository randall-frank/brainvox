/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/8/01 */

/*
 * Inverts 4x4 matrix ef to generate er
 *
 * Returns:
 *	0 if inverted successfully
 * 	errcode if error
 *
 */

#include "AIR.h"

AIR_Error AIR_inverter(double **ef, double **er, const AIR_Boolean zooming, const struct AIR_Key_info *pixel1_stats, const struct AIR_Key_info *pixel2_stats)

{
	double 	e[4][4];
	double 	*f[4];

	f[0]=&e[0][0];
	f[1]=&e[1][0];
	f[2]=&e[2][0];
	f[3]=&e[3][0];

	{
		unsigned int j;

		for(j=0;j<4;j++){
			
			unsigned int i;

			for(i=0;i<4;i++){
				e[j][i]=ef[j][i];
			}
		}
	}
	{
		AIR_Error errcode=AIR_gael(f);
		if(errcode!=0) return errcode;
	}
	{
		unsigned int j;

		for(j=0;j<4;j++){

			unsigned int i;

			for(i=0;i<4;i++){
				er[j][i]=e[j][i];
			}
        }
	}

	if (zooming){

		{
			double pixel_size2=pixel2_stats->x_size;
			if(pixel2_stats->y_size<pixel_size2) pixel_size2=pixel2_stats->y_size;
			if(pixel2_stats->z_size<pixel_size2) pixel_size2=pixel2_stats->z_size;

			if(fabs(pixel2_stats->x_size-pixel_size2)>AIR_CONFIG_PIX_SIZE_ERR){
			
				unsigned int mm;
				
				for (mm=0;mm<4;mm++){
					er[0][mm]/=(pixel2_stats->x_size/pixel_size2);
				}
			}

			if(fabs(pixel2_stats->y_size-pixel_size2)>AIR_CONFIG_PIX_SIZE_ERR){
			
				unsigned int mm;
				
				for (mm=0;mm<4;mm++){
					er[1][mm]/=(pixel2_stats->y_size/pixel_size2);
				}
			}

			if(fabs(pixel2_stats->z_size-pixel_size2)>AIR_CONFIG_PIX_SIZE_ERR){
			
				unsigned int mm;
				
				for (mm=0;mm<4;mm++){
					er[2][mm]/=(pixel2_stats->z_size/pixel_size2);
				}
			}
		}
		{
			double pixel_size1=pixel1_stats->x_size;
			if(pixel1_stats->y_size<pixel_size1) pixel_size1=pixel1_stats->y_size;
			if(pixel1_stats->z_size<pixel_size1) pixel_size1=pixel1_stats->z_size;

			if(fabs(pixel1_stats->x_size-pixel_size1)>AIR_CONFIG_PIX_SIZE_ERR){
			
				unsigned int mm;
				
				for (mm=0;mm<4;mm++){
					er[mm][0]/=(pixel1_stats->x_size/pixel_size1);
				}
			}

			if(fabs(pixel1_stats->y_size-pixel_size1)>AIR_CONFIG_PIX_SIZE_ERR){
			
				unsigned int mm;
				
				for (mm=0;mm<4;mm++){
					er[mm][1]/=(pixel1_stats->y_size/pixel_size1);
				}
			}

			if(fabs(pixel1_stats->z_size-pixel_size1)>AIR_CONFIG_PIX_SIZE_ERR){
			
				unsigned int mm;
				
				for (mm=0;mm<4;mm++){
					er[mm][2]/=(pixel1_stats->z_size/pixel_size1);
				}
			}
		}
	}
	return 0;
}
