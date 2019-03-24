/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 5/18/01 */

/*
 * reads a linear AIR reslice parameter file or 1st order warp file into an air struct
 *
 * returns:
 *	0 if successful
 *	error code if unsuccessful
 */

#include "AIR.h"

AIR_Error AIR_read_air16via_airw(const char *filename, struct AIR_Air16 *air1)

{
	double *e[4];
	
	e[0]=air1->e[0];
	e[1]=air1->e[1];
	e[2]=air1->e[2];
	e[3]=air1->e[3];
	
	/* Use read_airw to emulate read_air16 */
	{
		struct AIR_Warp airnon;
		double **f;
		{
			AIR_Error errcode;
			f=AIR_read_airw(filename,&airnon,&errcode);
			if(!f){
				return(errcode);
			}
		}
		if(airnon.order!=1){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("Only first order warp files can be converted to rigid-body approximations\n");
			AIR_free_2(f);
			return(AIR_WARP_NOT_FIRST_ORDER_ERROR);
		}
		if(airnon.coord<2 || airnon.coord>3){
			AIR_free_2(f);
			return(AIR_POLYNOMIAL_DIMENSIONS_ERROR);
		}
			
		/* Copy strings */
		strncpy(air1->r_file,airnon.r_file,(size_t)AIR_CONFIG_MAX_PATH_LENGTH-1);
		air1->r_file[AIR_CONFIG_MAX_PATH_LENGTH-1]='\0';

		strncpy(air1->s_file,airnon.s_file,(size_t)AIR_CONFIG_MAX_PATH_LENGTH-1);
		air1->s_file[AIR_CONFIG_MAX_PATH_LENGTH-1]='\0';
		
		strncpy(air1->comment,airnon.comment,(size_t)AIR_CONFIG_MAX_COMMENT_LENGTH-1);
		air1->comment[AIR_CONFIG_MAX_COMMENT_LENGTH-1]='\0';
		
		strncpy(air1->reserved,airnon.reserved,(size_t)AIR_CONFIG_RESERVED_LENGTH-1);
		air1->reserved[AIR_CONFIG_RESERVED_LENGTH-1]='\0';
		
		air1->s.bits=airnon.s.bits;
		air1->s.x_dim=airnon.s.x_dim;
		air1->s.y_dim=airnon.s.y_dim;
		air1->s.z_dim=airnon.s.z_dim;
		air1->s.x_size=airnon.s.x_size;
		air1->s.y_size=airnon.s.y_size;
		air1->s.z_size=airnon.s.z_size;
		
		air1->r.bits=airnon.r.bits;
		air1->r.x_dim=airnon.r.x_dim;
		air1->r.y_dim=airnon.r.y_dim;
		air1->r.z_dim=airnon.r.z_dim;
		air1->r.x_size=airnon.r.x_size;
		air1->r.y_size=airnon.r.y_size;
		air1->r.z_size=airnon.r.z_size;
		
		air1->s_hash=airnon.s_hash;
		air1->r_hash=airnon.r_hash;
		
		air1->s_volume=airnon.s_volume;
		air1->r_volume=airnon.r_volume;
					
		/* Adjust elements to the values they would have in a .air file */
		{
			double pixel_size_s;

			pixel_size_s=air1->s.x_size;
			if(air1->s.y_size<pixel_size_s) pixel_size_s=air1->s.y_size;
			if(air1->s.z_size<pixel_size_s) pixel_size_s=air1->s.z_size;

			e[3][0]=f[0][0];
			e[3][1]=f[1][0];
			e[0][0]=f[0][1]/(air1->s.x_size/pixel_size_s);
			e[0][1]=f[1][1]/(air1->s.x_size/pixel_size_s);
			e[1][0]=f[0][2]/(air1->s.y_size/pixel_size_s);
			e[1][1]=f[1][2]/(air1->s.y_size/pixel_size_s);
			e[2][0]=f[0][3]/(air1->s.z_size/pixel_size_s);
			e[2][1]=f[1][3]/(air1->s.z_size/pixel_size_s);
			
			if(airnon.coord==3){
				/* 3D */
				e[3][2]=f[2][0];
				e[0][2]=f[2][1]/(air1->s.x_size/pixel_size_s);
				e[1][2]=f[2][2]/(air1->s.y_size/pixel_size_s);
				e[2][2]=f[2][3]/(air1->s.z_size/pixel_size_s);
			}
			else{
				/* 2D */
				e[3][2]=0.0;
				e[0][2]=0.0;
				e[1][2]=0.0;
				e[2][2]=1.0/(air1->s.z_size/pixel_size_s);
			}
		}
		
		e[0][3]=0.0;
		e[1][3]=0.0;
		e[2][3]=0.0;
		e[3][3]=1.0;
		
		AIR_free_2(f);
	}

	return 0;
}
