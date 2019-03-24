/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 5/27/01 */

/*
 * This program will project a volume using the information
 *  contained in the specified .air file.
 *
 * Trilinear interpolation is used.
 *
 * Cubic voxels are generated			
 */

#include "AIR.h"

static void free_function(/*@null@*/ /*@out@*/ /*@only@*/ AIR_Pixels ***datain, /*@null@*/ /*@out@*/ /*@only@*/ AIR_Pixels ***datain2, /*@null@*/ /*@out@*/ /*@only@*/ AIR_Pixels ***dataout, /*@null@*/ /*@out@*/ /*@only@*/ AIR_Pixels ***class, /*@null@*/ /*@out@*/ /*@only@*/ AIR_Pixels ***final)
{
	if(datain) AIR_free_vol3(datain);
	if(datain2) AIR_free_vol3(datain2);
	if(dataout) AIR_free_vol3(dataout);
	if(class) AIR_free_vol3(class);
	if(final) AIR_free_vol3(final);
}

AIR_Error AIR_do_stereoproject(const char *program, const char *output, const char *alternate_reslice_file, const char *mask_file ,const char *air_file, const long int threshold, const double roll, const AIR_Boolean ow)
{
	/* Verify that output can be saved*/
	{
		AIR_Error errcode=AIR_save_probw(output,ow);
		if(errcode!=0) return errcode;
	}
	/* Verify alternate reslice file length is legal */
	if(alternate_reslice_file){
		if(strlen(alternate_reslice_file)>=(size_t)AIR_CONFIG_MAX_PATH_LENGTH){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("File name %s is too long to incorporate into a .air file\n",alternate_reslice_file);
			return AIR_PATH_TOO_LONG_ERROR;
		}
		{
			AIR_Error errcode=AIR_load_probr(alternate_reslice_file,FALSE);
			if(errcode!=0) return(errcode);
		}
	}
	/*Load the .air file*/
	{
		struct AIR_Air16 air1,air2;
		double *e[4],*e2[4], *e3[4];
		double e30[16];
		AIR_Pixels mapped_threshold;
		{
			AIR_Error errcode=AIR_read_air16(air_file,&air1);
			if(errcode!=0) return errcode;
		}
		
		if(alternate_reslice_file){
			strcpy(air1.r_file,alternate_reslice_file);
		}
		
		/* Adjust the .air matrix */
		e[0]=air1.e[0];
		e[1]=air1.e[1];
		e[2]=air1.e[2];
		e[3]=air1.e[3];
	
		e2[0]=air2.e[0];
		e2[1]=air2.e[1];
		e2[2]=air2.e[2];
		e2[3]=air2.e[3];
		
		e3[0]=e30;
		e3[1]=e30+4;
		e3[2]=e30+8;
		e3[3]=e30+12;
		
		/* Adjust it to generate cubic voxels */
		{
			double pixel_size_s=air1.s.x_size;
			if(air1.s.y_size<pixel_size_s) pixel_size_s=air1.s.y_size;
			if(air1.s.z_size<pixel_size_s) pixel_size_s=air1.s.z_size;
	
			if(fabs(air1.s.x_size-pixel_size_s)>AIR_CONFIG_PIX_SIZE_ERR){
				double xoom1=air1.s.x_size/pixel_size_s;
				air1.s.x_dim=(air1.s.x_dim-1)*xoom1+1;
				air1.s.x_size=pixel_size_s;
			}
			if(fabs(air1.s.y_size-pixel_size_s)>AIR_CONFIG_PIX_SIZE_ERR){
				double yoom1=air1.s.y_size/pixel_size_s;
				air1.s.y_dim=(air1.s.y_dim-1)*yoom1+1;
				air1.s.y_size=pixel_size_s;
			}
			if(fabs(air1.s.z_size-pixel_size_s)>AIR_CONFIG_PIX_SIZE_ERR){
				double zoom1=air1.s.z_size/pixel_size_s;
				air1.s.z_dim=(air1.s.z_dim-1)*zoom1+1;
				air1.s.z_size=pixel_size_s;
			}
	
			/*Adjust for modified voxel sizes*/
			if(fabs(air1.s.x_size-pixel_size_s)>AIR_CONFIG_PIX_SIZE_ERR){
				e[0][0]*=(air1.s.x_size/pixel_size_s);
				e[0][1]*=(air1.s.x_size/pixel_size_s);
				e[0][2]*=(air1.s.x_size/pixel_size_s);
				e[0][3]*=(air1.s.x_size/pixel_size_s);
			}
	
			if(fabs(air1.s.y_size-pixel_size_s)>AIR_CONFIG_PIX_SIZE_ERR){
				e[1][0]*=(air1.s.y_size/pixel_size_s);
				e[1][1]*=(air1.s.y_size/pixel_size_s);
				e[1][2]*=(air1.s.y_size/pixel_size_s);
				e[1][3]*=(air1.s.y_size/pixel_size_s);
			}
	
			if(fabs(air1.s.z_size-pixel_size_s)>AIR_CONFIG_PIX_SIZE_ERR){
				e[2][0]*=(air1.s.z_size/pixel_size_s);
				e[2][1]*=(air1.s.z_size/pixel_size_s);
				e[2][2]*=(air1.s.z_size/pixel_size_s);
				e[2][3]*=(air1.s.z_size/pixel_size_s);
			}
		}
		
		{
			/* Load the reslice file */
			struct AIR_Key_info		dimensions;
			struct AIR_Key_info		dimensions2;
			struct AIR_Key_info		dimensionsf;
			
			AIR_Pixels 		***datain=NULL;
			AIR_Pixels		***datain2=NULL;
			AIR_Pixels 		***dataout=NULL;
			AIR_Pixels		***class=NULL;
			AIR_Pixels		***final=NULL;
			
			{
				AIR_Error errcode;
	
				datain=AIR_load(air1.r_file, &dimensions, FALSE, &errcode);
				if(!datain){
					free_function(datain,datain2,dataout,class,final);
					return errcode;
				}
			}
	
			/*Check the reslice file against the .air reslice file specifications*/
			{
				AIR_Error errcode=AIR_same_dim(&air1.r,&dimensions);
				
				if(errcode!=0){
					printf("File '%s' not created because of matrix size incompatibility\n",output);
					printf("Dimensions expected based on file '%s': %u %u %u\n",air_file,air1.r.x_dim,air1.r.y_dim,air1.r.z_dim);
					printf("Dimensions of matrix in image file '%s': %u %u %u\n",air1.r_file,dimensions.x_dim,dimensions.y_dim,dimensions.z_dim);
					free_function(datain,datain2,dataout,class,final);
					return errcode;
				}
			}
			{
				AIR_Error errcode=AIR_same_size(&air1.r,&dimensions);
				
				if(errcode!=0){
					printf("File '%s' not created because of voxel size discrepancy\n",output);
					printf("Voxel dimensions expected based on file '%s': %.4f %.4f %.4f\n",air_file,air1.r.x_size,air1.r.y_size,air1.r.z_size);
					printf("Voxel dimensions in image file '%s': %.4f %.4f %.4f\n",air1.r_file,dimensions.x_size,dimensions.y_size,dimensions.z_size);
					free_function(datain,datain2,dataout,class,final);
					return errcode;
				}
			}
			
			/* Convert the threshold */
			{
				AIR_Error errcode;
				mapped_threshold=AIR_map_value(air1.r_file,threshold,&errcode);
				if(errcode!=0){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("Unable to map threshold to internal representation value\n");
					free_function(datain,datain2,dataout,class,final);
					return(errcode);
				}
			}
			
			/*Classify the data based on the threshold*/
			{
				AIR_Error errcode;
				
				class=AIR_classify(datain,&dimensions,mapped_threshold,&errcode);
				if(!class){
					printf("Failed to pre-classify data for rendering\n");
					free_function(datain,datain2,dataout,class,final);
					return errcode;
				}
			}
	
			/*Mask the input volume if mask is available*/
	
			if(mask_file){
				/* Load mask*/
				{
					AIR_Error errcode;
					
					datain2=AIR_load(mask_file, &dimensions2, TRUE, &errcode);
					if(!datain2){
						free_function(datain,datain2,dataout,class,final);
						return errcode;
					}
				}
	
				/*Verify dimension compatibility*/
				{
					AIR_Error errcode=AIR_same_dim(&dimensions,&dimensions2);
					
					if(errcode!=0){
						printf("%s and %s have incompatible matrix sizes\n",air1.r_file,mask_file);
						printf("%s: %u %u %u\n",air1.r_file,dimensions.x_dim,dimensions.y_dim,dimensions.z_dim);
						printf("%s: %u %u %u\n",mask_file,dimensions2.x_dim,dimensions2.y_dim,dimensions2.z_dim);
						free_function(datain,datain2,dataout,class,final);
						return errcode;
					}
				}
				{
					AIR_Error errcode=AIR_mask(datain,&dimensions,datain2,&dimensions2,0);
					
					if(errcode!=0){
						printf("failed to created masked version of file\n");
						free_function(datain,datain2,dataout,class,final);
						return errcode;
					}
				}
				AIR_free_vol3(datain2);
				datain2=NULL;
			}
			

	
			if(roll!=0.0) dimensionsf.x_dim=air1.s.x_dim*2;
			else dimensionsf.x_dim=air1.s.x_dim;
			dimensionsf.y_dim=air1.s.y_dim;
			dimensionsf.z_dim=1;
	
			dimensionsf.x_size=air1.s.x_size;
			dimensionsf.y_size=air1.s.y_size;
			dimensionsf.z_size=air1.s.z_size;
	
			dimensionsf.bits=8*sizeof(AIR_Pixels);
	
			final=AIR_create_vol3(dimensionsf.x_dim,dimensionsf.y_dim,dimensionsf.z_dim);
			if(!final){
				printf("Failed to allocate memory for output volume\n");
				free_function(datain,datain2,dataout,class,final);
				return AIR_MEMORY_ALLOCATION_ERROR;
			}
			{
				AIR_Boolean flag2=FALSE;
				unsigned int flag=0;
	
				while(!flag2){
					if(flag==0){
						/*Because the projection routine is  more efficient working along x, 	*/
						/* we will rotate the input image by an additional -90 degrees around	*/
						/* the y axis (so that projecting along x becomes projecting along z)	*/
						/* 									*/
						/*We don't use a matrix swap routine because  the whole	volume hasn't	*/
						/* been calculated (we don't want to compute the whole volume		*/
						/* just that part until we reach the threshold)				*/
						{
							double temp2=e[0][0];
							e[0][0]=e[2][0];
							e[2][0]=temp2;
						}
						{
							double temp2=e[0][1];
							e[0][1]=e[2][1];
							e[2][1]=temp2;
						}
						{
							double temp2=e[0][2];
							e[0][2]=e[2][2];
							e[2][2]=temp2;
						}
						{
							double temp2=e[0][3];
							e[0][3]=e[2][3];
							e[2][3]=temp2;
						}
						/* The x and z dimensions and sizes of air1.s must be exchanged */
						{
							unsigned int temp=air1.s.x_dim;
							air1.s.x_dim=air1.s.z_dim;
							air1.s.z_dim=temp;
						}
						{
							double temp2=air1.s.x_size;
							air1.s.x_size=air1.s.z_size;
							air1.s.z_size=temp2;
						}
					}
					else{
						if(roll!=0.0){
							/*Set up a rotation matrix that will reorient standard space*/
							/* this reorientation is done as orthonormal transform of	*/
							/* the unrotated parameters in order to assure that is looks	*/
							/* correct as rendered (even if rendering itself is not orthonormal)*/
							double thisroll=2*acos((double)0.0)*(roll)/180;
							double sinroll=sin(thisroll);
							double cosroll=cos(thisroll);
	
	
							e2[0][0]=cosroll;
							e2[0][1]=0.0;
							e2[0][2]=-sinroll;
							e2[0][3]=0.0;
	
							e2[1][0]=0.0;
							e2[1][1]=1.0;
							e2[1][2]=0.0;
							e2[1][3]=0.0;
	
							e2[2][0]=sinroll;
							e2[2][1]=0.0;
							e2[2][2]=cosroll;
							e2[2][3]=0.0;
	
							e2[3][0]=(air1.s.x_dim*(1-cosroll)-air1.s.z_dim*sinroll)/2;
							e2[3][1]=0.0;
							e2[3][2]=(air1.s.z_dim*(1-cosroll)+air1.s.x_dim*sinroll)/2;
							e2[3][3]=1.0;
	
							AIR_matmul(e,e2,e3);
							{
								unsigned int j;
								
								for(j=0;j<4;j++){
								
									unsigned int i;
									
									for(i=0;i<3;i++){
										e[j][i]=e3[j][i];
									}
								}
							}
						}
					}
					if(flag==0|| roll!=0.0){
	
						/*Project the data*/
						air2.s.x_dim=air1.s.x_dim;
						air2.s.y_dim=air1.s.y_dim;
						air2.s.z_dim=air1.s.z_dim;
						air2.s.x_size=air1.s.x_size;
						air2.s.y_size=air1.s.y_size;
						air2.s.z_size=air1.s.z_size;
						air2.s.bits=air1.s.bits;
	
						{
							AIR_Error errcode;
							
							dataout=AIR_projslicex(datain,&air1.r,&air2.s,e,mapped_threshold,class,&errcode);
							if(!dataout){
								free_function(datain,datain2,dataout,class,final);
								return errcode;
							}
						}
	
						/*Swap x and z of the data back*/
						{
							AIR_Error errcode;
							AIR_Pixels ***temp=AIR_swap_x_and_z(dataout,&air2.s,&air2.s,&errcode);
							if(!temp){
								free_function(datain,datain2,dataout,class,final);
								return errcode;
							}
							AIR_free_vol3(dataout);
							dataout=temp;
						}
						{
							unsigned int j;
							
							for(j=0;j<air2.s.y_dim;j++){
							
								unsigned int i;
								
								for(i=0;i<air2.s.x_dim;i++){
									final[0][j][i+flag*air2.s.x_dim]=dataout[0][j][i];
								}
							}
						}
						AIR_free_vol3(dataout);
						dataout=NULL;
					}
	
					if(flag==0) flag=1;
					else{
						flag=0;
						flag2=TRUE;
					}
				}
			}
	
			/*Save the data*/
			{
				AIR_Error errcode=AIR_save(final,&dimensionsf,output,ow,program);
				
				if(errcode!=0){
					free_function(datain,datain2,dataout,class,final);
					return errcode;
				}
			}
			free_function(datain,datain2,dataout,class,final);
		}
		return 0;
	}
}
