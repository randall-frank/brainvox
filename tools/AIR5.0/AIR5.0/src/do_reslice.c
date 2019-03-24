/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 5/30/01 */

/* This program will reslice a volume using the information
 *  contained in the specified .air file.
 *
 * Trilinear interpolation is default, but nearest neighbor or sinc
 *  can be explicitly requested.
 *
 * The default is to interpolate the output to cubic voxels,
 *  and not to overwrite existing files.
 *
 * Defaults can be overridden and other options utilized by
 *  various flags.
 *
 * Flags can be displayed by typing the name of the program
 *  without additional arguments.
 */

#include "AIR.h"

static void free_function(/*@null@*/ /*@out@*/ /*@only@*/ AIR_Pixels ***datain, /*@null@*/ /*@out@*/ /*@only@*/ AIR_Pixels ***dataout)

{
	if(datain) AIR_free_vol3(datain);
	if(dataout) AIR_free_vol3(dataout);
}

AIR_Error AIR_do_reslice(const char *program, const char *airfile, const char *outfile, const char *alternate_reslice_file, const unsigned int interp, const unsigned int *window, const AIR_Boolean cubic, double scale, const char *mult_scale_file, const char *div_scale_file, const AIR_Boolean ow, const unsigned int x_dimout, const float x_distance, const float x_shift, const unsigned int y_dimout, const float y_distance, const float y_shift, const unsigned int z_dimout, const float z_distance, const float z_shift)

{	
	if(scale<=0){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("User interface allowed negative scale\n");
		return AIR_USER_INTERFACE_ERROR;
	}
	if(x_dimout!=0 || x_distance!=0.0 || x_shift!=0.0){
		if(x_dimout==0){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("User interface allowed negative or zero x dimension\n");
			return AIR_USER_INTERFACE_ERROR;
		}
		if(x_distance<=0.0){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("User interface allowed negative or zero voxel x size\n");
				return AIR_USER_INTERFACE_ERROR;
		}
		if(!cubic){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("User interface allowed non-cubic voxel request and explicit dimensioning\n");
			return AIR_USER_INTERFACE_ERROR;
		}
	}
	if(y_dimout!=0 || y_distance!=0.0 || y_shift!=0.0){
		if(y_dimout==0){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("User interface allowed negative or zero y dimension\n");
			return AIR_USER_INTERFACE_ERROR;
		}
		if(y_distance<=0.0){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("User interface allowed negative or zero voxel y size\n");
				return AIR_USER_INTERFACE_ERROR;
		}
		if(!cubic){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("User interface allowed non-cubic voxel request and explicit dimensioning\n");
			return AIR_USER_INTERFACE_ERROR;
		}
	}
	if(z_dimout!=0 || z_distance!=0.0 || z_shift!=0.0){
		if(z_dimout==0){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("User interface allowed negative or zero z dimension\n");
			return AIR_USER_INTERFACE_ERROR;
		}
		if(z_distance<=0.0){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("User interface allowed negative or zero voxel z size\n");
			return AIR_USER_INTERFACE_ERROR;
		}
		if(!cubic){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("User interface allowed non-cubic voxel request and explicit dimensioning\n");
			return AIR_USER_INTERFACE_ERROR;
		}
	}
	
	/* Verify the alternate reslice file */
	if(alternate_reslice_file){
		if(strlen(alternate_reslice_file)>=(size_t)AIR_CONFIG_MAX_PATH_LENGTH){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("File name %s is too long to incorporate into a .air file\n",alternate_reslice_file);
			return AIR_PATH_TOO_LONG_ERROR;
		}
		{
			AIR_Error errcode=AIR_load_probr(alternate_reslice_file,TRUE);
			if(errcode!=0) return(errcode);
		}
	}
	
	/* Verify that air file can be read */
	{
		AIR_Error errcode=AIR_fprobr(airfile);
		
		if(errcode!=0){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("Problem with file %s\n",airfile);
			return(errcode);
		}
	}
	
	/* Verify that output can be saved */
	{
		AIR_Error errcode=AIR_save_probw(outfile,ow);
		
		if(errcode!=0) return errcode;
	}
	
	/* Verify that scaling_files can be read */
	if(mult_scale_file){
		AIR_Error errcode=AIR_fprobr(mult_scale_file);
		
		if(errcode!=0){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("Problem with file %s\n",mult_scale_file);
			return(errcode);
		}
	}
	if(div_scale_file){
		AIR_Error errcode=AIR_fprobr(div_scale_file);
		
		if(errcode!=0){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("Problem with file %s\n",div_scale_file);
			return(errcode);
		}
	}
	
	/* Apply the scaling files */
	if(mult_scale_file){
	
		FILE *fp=fopen(mult_scale_file,"rb");
		if (!fp){
					
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("\nunable to open 'multiply-by' scaling file '%s'\n",mult_scale_file);
			{
				AIR_Error errcode=AIR_fprobr(mult_scale_file);
				if(errcode==0) errcode=AIR_UNSPECIFIED_FILE_READ_ERROR;
				return(errcode);
			}
		}
		{
			double temp;
			
			if(fscanf(fp,"%le",&temp)==EOF){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("unable to read value from 'multiply-by' scaling file '%s'\n",mult_scale_file);
				(void)fclose(fp);
				return AIR_READ_SCALING_FILE_ERROR;
			}
			scale*=temp;
		}
		if(fclose(fp)!=0){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("failed to close file %s\n",mult_scale_file);
			return AIR_CANT_CLOSE_READ_ERROR;
		}
	}
	if(div_scale_file){
	
		FILE *fp=fopen(div_scale_file,"rb");
		if (!fp){
					
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("\nunable to open 'divide-by' scaling file '%s'\n",div_scale_file);
			{
				AIR_Error errcode=AIR_fprobr(div_scale_file);
				if(errcode==0) errcode=AIR_UNSPECIFIED_FILE_READ_ERROR;
				return(errcode);
			}
		}
		{
			double temp;
			
			if(fscanf(fp,"%le",&temp)==EOF){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("unable to read value from 'divide-by' scaling file '%s'\n",div_scale_file);
				(void)fclose(fp);
				return AIR_READ_SCALING_FILE_ERROR;
			}
			scale/=temp;
		}
		if(fclose(fp)!=0){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("failed to close file %s\n",div_scale_file);
			return AIR_CANT_CLOSE_READ_ERROR;
		}
	}

	{
		struct AIR_Air16 air1;
		double *e[4];
		struct AIR_Key_info dimensions3;
		
		AIR_Pixels ***(*reslicerN)(AIR_Pixels ***, const struct AIR_Key_info *, struct AIR_Key_info *, double **, const double, AIR_Error *)=NULL;
		AIR_Pixels ***(*reslicerNW2)(AIR_Pixels ***, const struct AIR_Key_info *, struct AIR_Key_info *, double **, const double, const unsigned int, const unsigned int, AIR_Error *)=NULL;
		AIR_Pixels ***(*reslicerNW3)(AIR_Pixels ***, const struct AIR_Key_info *, struct AIR_Key_info *, double **, const double, const unsigned int, const unsigned int, const unsigned int, AIR_Error *)=NULL;

		e[0]=air1.e[0];
		e[1]=air1.e[1];
		e[2]=air1.e[2];
		e[3]=air1.e[3];
		
		/*Get the reslice parameters*/
		{
			AIR_Error errcode=AIR_read_air16((airfile),&air1);
			
			if(errcode!=0) return(errcode);
		}
		
		if(alternate_reslice_file) strcpy(air1.r_file, alternate_reslice_file);
		
		/* Validate interpolation model */
		{
			AIR_Boolean affine=(air1.e[0][3]==0 && air1.e[1][3]==0 && air1.e[2][3]==0 && air1.e[3][3]==1);

			switch(interp){
				case 0:
					reslicerN=AIR_r_persp_nn;
					break;
				case 1:
					if(affine) reslicerN=AIR_r_affine_lin;
					else reslicerN=AIR_r_persp_lin;
					break;
				case 2:
					if(window[0]==0 || window[1]==0){
						printf("%s: %d: ",__FILE__,__LINE__);
						printf("User interface allowed invalid interpolation window widths\n");
						return AIR_USER_INTERFACE_ERROR;
					}
					reslicerNW2=AIR_r_persp_wsinc_xy;
					break;
				case 3:
					if(window[0]==0 || window[1]==0){
						printf("%s: %d: ",__FILE__,__LINE__);
						printf("User interface allowed invalid interpolation window widths\n");
						return AIR_USER_INTERFACE_ERROR;
					}
					reslicerNW2=AIR_r_persp_wsinc_xz;
					break;
				case 4:
					if(window[0]==0 || window[1]==0){
						printf("%s: %d: ",__FILE__,__LINE__);
						printf("User interface allowed invalid interpolation window widths\n");
						return AIR_USER_INTERFACE_ERROR;
					}
					reslicerNW2=AIR_r_persp_wsinc_yz;
					break;
				case 5:
					if(window[0]==0 || window[1]==0 || window[2]==0){
						printf("%s: %d: ",__FILE__,__LINE__);
						printf("User interface allowed invalid interpolation window widths\n");
						return AIR_USER_INTERFACE_ERROR;
					}
					reslicerNW3=AIR_r_persp_wsinc;
					break;
				case 6:
					if(window[0]==0 || window[1]==0 || window[2]==0){
						printf("%s: %d: ",__FILE__,__LINE__);
						printf("User interface allowed invalid interpolation window widths\n");
						return AIR_USER_INTERFACE_ERROR;
					}
					if(!affine){
						printf("%s: %d: ",__FILE__,__LINE__);
						printf("Scanline interpolation model cannot be used with perspective distortions\n");
						return AIR_NO_PERSPECTIVE_ERROR;
					}
					reslicerNW3=AIR_r_scan_wsinc;
					break;
				case 7:
					if(!affine){
						printf("%s: %d: ",__FILE__,__LINE__);
						printf("Scanline interpolation model cannot be used with perspective distortions\n");
						return AIR_NO_PERSPECTIVE_ERROR;
					}
					reslicerN=AIR_r_scan_sinc;
					break;
				case 10:

					reslicerN=AIR_r_scan_chirp;
					break;
				case 11:
					if(!affine){
						printf("%s: %d: ",__FILE__,__LINE__);
						printf("Scanline interpolation model cannot be used with perspective distortions\n");
						return AIR_NO_PERSPECTIVE_ERROR;
					}
					reslicerN=AIR_r_scan_chirp_xy;
					break;
				case 12:
					if(!affine){
						printf("%s: %d: ",__FILE__,__LINE__);
						printf("Scanline interpolation model cannot be used with perspective distortions\n");
						return AIR_NO_PERSPECTIVE_ERROR;
					}
					reslicerN=AIR_r_scan_chirp_xz;
					break;
				case 13:
					if(!affine){
						printf("%s: %d: ",__FILE__,__LINE__);
						printf("Scanline interpolation model cannot be used with perspective distortions\n");
						return AIR_NO_PERSPECTIVE_ERROR;
					}
					reslicerN=AIR_r_scan_chirp_yz;
					break;
				default:
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("User interface provided invalid interpolation model\n");
					return AIR_USER_INTERFACE_ERROR;
			}
		}

		{
			AIR_Pixels ***datain=NULL;
			AIR_Pixels ***dataout=NULL;
		
			/*Load the reslice file*/
			{
				AIR_Error errcode;
				/* Only nearest neighbor interpolation should be applied to binary files */
				datain=AIR_load(air1.r_file, &dimensions3, interp==0, &errcode);
				if(!datain){
					free_function(datain,dataout);
					return(errcode);
				}
			}

			/*Check the reslice file against the .air reslice file specifications*/
			{
				AIR_Error errcode=AIR_same_dim(&air1.r,&dimensions3);
				
				if(errcode!=0){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("File '%s' not created because of matrix size incompatibility\n",outfile);
					printf("Dimensions expected based on file '%s': %u %u %u\n",airfile,air1.r.x_dim,air1.r.y_dim,air1.r.z_dim);
					printf("Dimensions of matrix in image file '%s': %u %u %u\n",air1.r_file,dimensions3.x_dim,dimensions3.y_dim,dimensions3.z_dim);
					free_function(datain,dataout);
					return(errcode);
				}
			}
			{
				AIR_Error errcode=AIR_same_size(&air1.r,&dimensions3);
				
				if(errcode!=0){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("File '%s' not created because of voxel size discrepancy\n",outfile);
					printf("Voxel dimensions expected based on file '%s': %.4f %.4f %.4f\n",airfile,air1.r.x_size,air1.r.y_size,air1.r.z_size);
					printf("Voxel dimensions in image file '%s': %.4f %.4f %.4f\n",air1.r_file,dimensions3.x_size,dimensions3.y_size,dimensions3.z_size);
					free_function(datain,dataout);
					return(errcode);
				}
			}
			
			{	
				double pixel_size_s=air1.s.x_size;
				if(air1.s.y_size<pixel_size_s) pixel_size_s=air1.s.y_size;
				if(air1.s.z_size<pixel_size_s) pixel_size_s=air1.s.z_size;

				/*Sort out how to interpolate*/
				if (!cubic){
					/* If z_dimout==0, keep the same z_dim as the standard file*/
					if(x_dimout!=0){
						air1.s.x_dim=x_dimout;
						air1.s.x_size=x_distance;
					}
					if(y_dimout!=0){
						air1.s.y_dim=y_dimout;
						air1.s.y_size=y_distance;
					}
					if(z_dimout!=0){
						air1.s.z_dim=z_dimout;
						air1.s.z_size=z_distance;
					}
				}
				else{
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

				/*Adjust for shifts*/
				e[3][0]+=e[0][0]*x_shift+e[1][0]*y_shift+e[2][0]*z_shift;
				e[3][1]+=e[0][1]*x_shift+e[1][1]*y_shift+e[2][1]*z_shift;
				e[3][2]+=e[0][2]*x_shift+e[1][2]*y_shift+e[2][2]*z_shift;
				e[3][3]+=e[0][3]*x_shift+e[1][3]*y_shift+e[2][3]*z_shift;
			}


			/*Reslice the data*/
			{
				AIR_Error errcode;
				
				if(reslicerN) dataout=(reslicerN)(datain,&air1.r,&air1.s,e,scale,&errcode);
				else if(reslicerNW2) dataout=(reslicerNW2)(datain,&air1.r,&air1.s,e,scale,window[0],window[1],&errcode);
				else if(reslicerNW3) dataout=(reslicerNW3)(datain,&air1.r,&air1.s,e,scale,window[0],window[1],window[2],&errcode);
				
				if(!dataout){
					free_function(datain,dataout);
					return(errcode);
				}
			}

			/*Save the data*/
			air1.s.bits=8*sizeof(AIR_Pixels);
			{
				AIR_Error errcode=AIR_save(dataout,&air1.s,outfile,ow,program);
				
				if(errcode!=0){
					free_function(datain,dataout);
					return(errcode);
				}
			}
			free_function(datain,dataout);
		}
	}
	return 0;
}

