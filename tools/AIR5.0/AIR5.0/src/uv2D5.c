/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 5/8/01 */

/*
 * In-plane affine rescaling model with fixed determinant
 */

#include "AIR.h"

#define PARAMETERS 5

void AIR_uv2D5(const double *i, double **e, double ***de, double ****ee, const struct AIR_Key_info *reslice_info, const struct AIR_Key_info *standard_info, const AIR_Boolean zooming)

{
	double	determinant;
	double	pixel_size_s;

	pixel_size_s=standard_info->x_size;
	if(standard_info->y_size<pixel_size_s) pixel_size_s=standard_info->y_size;
	if(standard_info->z_size<pixel_size_s) pixel_size_s=standard_info->z_size;

	e[0][0]=i[0];
	e[0][1]=i[3];
	e[0][2]=0;

	e[1][0]=i[1];
	e[1][2]=0;

	e[2][0]=0;
	e[2][1]=0;
	e[2][2]=1;

	e[3][0]=i[2];
	e[3][1]=i[4];
	e[3][2]=0;

	/*And the constant row*/
	e[0][3]=0;
	e[1][3]=0;
	e[2][3]=0;
	e[3][3]=1;

	if(!zooming){
		determinant=(standard_info->x_size)*(standard_info->y_size)/((reslice_info->x_size)*(reslice_info->y_size));
	}
	else{
		determinant=(pixel_size_s)*(pixel_size_s)/((reslice_info->x_size)*(reslice_info->y_size));
	}

	e[1][1]=(determinant+i[1]*i[3])/i[0];
	{
		unsigned int t;
		
		for(t=0;t<5;t++){
			de[0][0][t]=0;
			de[0][1][t]=0;
			de[0][2][t]=0;
			de[0][3][t]=0;
			de[1][0][t]=0;
			de[1][1][t]=0;
			de[1][2][t]=0;
			de[1][3][t]=0;
			de[2][0][t]=0;
			de[2][1][t]=0;
			de[2][2][t]=0;
			de[2][3][t]=0;
			de[3][0][t]=0;
			de[3][1][t]=0;
			de[3][2][t]=0;
			de[3][3][t]=0;
			{
				unsigned int s;
				
				for(s=0;s<=t;s++){
					ee[0][0][t][s]=0;
					ee[0][1][t][s]=0;
					ee[0][2][t][s]=0;
					ee[0][3][t][s]=0;
					ee[1][0][t][s]=0;
					ee[1][1][t][s]=0;
					ee[1][2][t][s]=0;
					ee[1][3][t][s]=0;
					ee[2][0][t][s]=0;
					ee[2][1][t][s]=0;
					ee[2][2][t][s]=0;
					ee[2][3][t][s]=0;
					ee[3][0][t][s]=0;
					ee[3][1][t][s]=0;
					ee[3][2][t][s]=0;
					ee[3][3][t][s]=0;
				}
			}
		}
	}
	de[0][0][0]=1;
	de[1][0][1]=1;
	de[3][0][2]=1;
	de[0][1][3]=1;
	de[3][1][4]=1;
	de[1][1][0]=-(determinant+i[1]*i[3])/(i[0]*i[0]);
	de[1][1][1]=i[3]/i[0];
	de[1][1][3]=i[1]/i[0];

	ee[1][1][0][0]=(determinant+i[1]*i[3])*2.0/(i[0]*i[0]*i[0]);
	ee[1][1][1][0]=-i[3]/(i[0]*i[0]);
	ee[1][1][3][0]=-i[1]/(i[0]*i[0]);
	ee[1][1][3][1]=1.0/i[0];

	return;
}
