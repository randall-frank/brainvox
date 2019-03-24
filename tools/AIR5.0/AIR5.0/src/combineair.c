/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 5/27/01 */

/*
 * combines two .air files after performing compatibility testing
 * AIR files are 4x4
 *
 * returns:
 *	0 if successful
 *	error code if unsuccessful (compatibility testing failed)
 *
 * Values in air1 and air2 will not be modified at return time from
 *  this routine, unless air_comb is the same address as air1 or air2
 *  in which case the corresponding input will be overwritten by the
 *  output. The routine is constructed such that the output will be
 *  correct when used to overwrite an input.
 */

#include "AIR.h"

AIR_Error AIR_combineair(const struct AIR_Air16 *air1, const struct AIR_Air16 *air2orig, struct AIR_Air16 *air_comb)

{
	struct AIR_Air16 air2=*air2orig;

	{
		double pixel_size_s=air2.s.x_size;
		if(air2.s.y_size<pixel_size_s) pixel_size_s=air2.s.y_size;
		if(air2.s.z_size<pixel_size_s) pixel_size_s=air2.s.z_size;


		/*If a dimension of air1 reslice file does not match corresponding dimension of air2 standard file,*/
		/* try interpolating air2 standard file to cubic voxels */

		if(air2.s.x_dim!=air1->r.x_dim){
			double xoom2=air2.s.x_size/pixel_size_s;
			air2.s.x_dim=(air2.s.x_dim-1)*xoom2+1;
			air2.s.x_size=pixel_size_s;
		}

		if(air2.s.y_dim!=air1->r.y_dim){
			double yoom2=air2.s.y_size/pixel_size_s;
			air2.s.y_dim=(air2.s.y_dim-1)*yoom2+1;
			air2.s.y_size=pixel_size_s;
		}

		if(air2.s.z_dim!=air1->r.z_dim){
			double zoom2=air2.s.z_size/pixel_size_s;
			air2.s.z_dim=(air2.s.z_dim-1)*zoom2+1;
			air2.s.z_size=pixel_size_s;
		}


		/*Test x,y and z dimension and size compatibility of air1 reslice file and air2 standard file*/

		/*Dimension*/
		{
			AIR_Error errcode=AIR_same_dim(&air1->r,&air2.s);
			if(errcode!=0){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("Cannot combine because matrix dimensions are not equivalent\n");

				return errcode;
			}
		}

		/*Size*/
		{
			AIR_Error errcode=AIR_same_size(&air1->r,&air2.s);
			if(errcode!=0){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("Cannot combine because sizes are not equivalent\n");

				return errcode;
			}
		}

		/* Modify to account for storage format of .air files */

		if(fabs(air2.s.x_size-pixel_size_s)>AIR_CONFIG_PIX_SIZE_ERR){
			air2.e[0][0]*=(air2.s.x_size/pixel_size_s);
			air2.e[0][1]*=(air2.s.x_size/pixel_size_s);	
			air2.e[0][2]*=(air2.s.x_size/pixel_size_s);
			air2.e[0][3]*=(air2.s.x_size/pixel_size_s);
		}

		if(fabs(air2.s.y_size-pixel_size_s)>AIR_CONFIG_PIX_SIZE_ERR){
			air2.e[1][0]*=(air2.s.y_size/pixel_size_s);
			air2.e[1][1]*=(air2.s.y_size/pixel_size_s);	
			air2.e[1][2]*=(air2.s.y_size/pixel_size_s);
			air2.e[1][3]*=(air2.s.y_size/pixel_size_s);
		}

		if(fabs(air2.s.z_size-pixel_size_s)>AIR_CONFIG_PIX_SIZE_ERR){
			air2.e[2][0]*=(air2.s.z_size/pixel_size_s);
			air2.e[2][1]*=(air2.s.z_size/pixel_size_s);	
			air2.e[2][2]*=(air2.s.z_size/pixel_size_s);
			air2.e[2][3]*=(air2.s.z_size/pixel_size_s);
		}
	}
	{
		double e[4][4];
		
		/*Compute new AIR file values*/
		e[0][0]=air2.e[0][0]*air1->e[0][0]+air2.e[1][0]*air1->e[0][1]+air2.e[2][0]*air1->e[0][2]+air2.e[3][0]*air1->e[0][3];
		e[1][0]=air2.e[0][0]*air1->e[1][0]+air2.e[1][0]*air1->e[1][1]+air2.e[2][0]*air1->e[1][2]+air2.e[3][0]*air1->e[1][3];
		e[2][0]=air2.e[0][0]*air1->e[2][0]+air2.e[1][0]*air1->e[2][1]+air2.e[2][0]*air1->e[2][2]+air2.e[3][0]*air1->e[2][3];
		e[3][0]=air2.e[0][0]*air1->e[3][0]+air2.e[1][0]*air1->e[3][1]+air2.e[2][0]*air1->e[3][2]+air2.e[3][0]*air1->e[3][3];


		e[0][1]=air2.e[0][1]*air1->e[0][0]+air2.e[1][1]*air1->e[0][1]+air2.e[2][1]*air1->e[0][2]+air2.e[3][1]*air1->e[0][3];
		e[1][1]=air2.e[0][1]*air1->e[1][0]+air2.e[1][1]*air1->e[1][1]+air2.e[2][1]*air1->e[1][2]+air2.e[3][1]*air1->e[1][3];
		e[2][1]=air2.e[0][1]*air1->e[2][0]+air2.e[1][1]*air1->e[2][1]+air2.e[2][1]*air1->e[2][2]+air2.e[3][1]*air1->e[2][3];
		e[3][1]=air2.e[0][1]*air1->e[3][0]+air2.e[1][1]*air1->e[3][1]+air2.e[2][1]*air1->e[3][2]+air2.e[3][1]*air1->e[3][3];


		e[0][2]=air2.e[0][2]*air1->e[0][0]+air2.e[1][2]*air1->e[0][1]+air2.e[2][2]*air1->e[0][2]+air2.e[3][2]*air1->e[0][3];
		e[1][2]=air2.e[0][2]*air1->e[1][0]+air2.e[1][2]*air1->e[1][1]+air2.e[2][2]*air1->e[1][2]+air2.e[3][2]*air1->e[1][3];
		e[2][2]=air2.e[0][2]*air1->e[2][0]+air2.e[1][2]*air1->e[2][1]+air2.e[2][2]*air1->e[2][2]+air2.e[3][2]*air1->e[2][3];
		e[3][2]=air2.e[0][2]*air1->e[3][0]+air2.e[1][2]*air1->e[3][1]+air2.e[2][2]*air1->e[3][2]+air2.e[3][2]*air1->e[3][3];


		e[0][3]=air2.e[0][3]*air1->e[0][0]+air2.e[1][3]*air1->e[0][1]+air2.e[2][3]*air1->e[0][2]+air2.e[3][3]*air1->e[0][3];
		e[1][3]=air2.e[0][3]*air1->e[1][0]+air2.e[1][3]*air1->e[1][1]+air2.e[2][3]*air1->e[1][2]+air2.e[3][3]*air1->e[1][3];
		e[2][3]=air2.e[0][3]*air1->e[2][0]+air2.e[1][3]*air1->e[2][1]+air2.e[2][3]*air1->e[2][2]+air2.e[3][3]*air1->e[2][3];
		e[3][3]=air2.e[0][3]*air1->e[3][0]+air2.e[1][3]*air1->e[3][1]+air2.e[2][3]*air1->e[3][2]+air2.e[3][3]*air1->e[3][3];



		/*Fill in the information in the combined struct*/
		{
			unsigned int i;
			
			for(i=0;i<4;i++){
			
				unsigned int j;
				
				for(j=0;j<4;j++){
					air_comb->e[i][j]=e[i][j];
				}
			}
		}
	}

	strcpy(air_comb->s_file,air1->s_file);
	air_comb->s.bits=air1->s.bits;
	air_comb->s.x_dim=air1->s.x_dim;
	air_comb->s.y_dim=air1->s.y_dim;
	air_comb->s.z_dim=air1->s.z_dim;
	air_comb->s.x_size=air1->s.x_size;
	air_comb->s.y_size=air1->s.y_size;
	air_comb->s.z_size=air1->s.z_size;

	strcpy(air_comb->r_file,air2.r_file);
	air_comb->r.bits=air2.r.bits;
	air_comb->r.x_dim=air2.r.x_dim;
	air_comb->r.y_dim=air2.r.y_dim;
	air_comb->r.z_dim=air2.r.z_dim;
	air_comb->r.x_size=air2.r.x_size;
	air_comb->r.y_size=air2.r.y_size;
	air_comb->r.z_size=air2.r.z_size;

	return 0;
}
