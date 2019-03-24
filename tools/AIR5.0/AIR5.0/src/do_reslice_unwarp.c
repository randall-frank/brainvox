/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 7/29/01 */

/*
 * This routine will reslice a volume using the information
 *  contained in the specified .warp file, reversing the roles of reslice and standard files
 */

#include "AIR.h"

static void free_function(/*@null@*/ /*@out@*/ /*@only@*/ double **e, /*@null@*/ /*@out@*/ /*@only@*/ double **g, /*@null@*/ /*@out@*/ /*@only@*/ AIR_Pixels ***datain, /*@null@*/ /*@out@*/ /*@only@*/ AIR_Pixels ***dataout)

{
	if(e) AIR_free_2(e);
	if(g) AIR_free_2(g);
	if(datain) AIR_free_vol3(datain);
	if(dataout) AIR_free_vol3(dataout);
}

AIR_Error AIR_do_reslice_unwarp(const char *program, const char *outfile, const char *alternate_reslice_file, const char *warpfile, const unsigned int interp, const unsigned int *window, double scale, const char *mult_scale_file, const char *div_scale_file, const signed int parity, double accuracy, const char *linear_estimate_file, const AIR_Boolean ow)

{
	if(accuracy<0){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("User interface allowed negative accuracy\n");
		return AIR_USER_INTERFACE_ERROR;
	}
	
	if(scale<0){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("User interface allowed negative scale\n");
		return AIR_USER_INTERFACE_ERROR;
	}
	
	/* Verify the alternate reslice file */
	if(alternate_reslice_file){
		if(strlen(alternate_reslice_file)>=(size_t)AIR_CONFIG_MAX_PATH_LENGTH){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("File name %s is too long to incorporate into a .warp file\n",alternate_reslice_file);
			return AIR_PATH_TOO_LONG_ERROR;
		}
		{
			AIR_Error errcode=AIR_load_probr(alternate_reslice_file,TRUE);
			if(errcode!=0) return(errcode);
		}
	}
	
	/* Verify that warpfile can be read*/
	if(warpfile){
		AIR_Error errcode=AIR_fprobr(warpfile);
		
		if(errcode!=0){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("Problem with file %s\n",warpfile);
			return(errcode);
		}
	}	
	
	/* Verify that output can be saved */
	{
		AIR_Error errcode=AIR_save_probw(outfile,ow);
		
		if(errcode!=0) return errcode;
	}
	
	/* Verify that linear_estimate_file can be read */
	if(linear_estimate_file){
		AIR_Error errcode=AIR_fprobr(linear_estimate_file);
		
		if(errcode!=0){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("Problem with file %s\n",linear_estimate_file);
			return(errcode);
		}
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
		double **e=NULL;
		double **g=NULL;
		AIR_Pixels ***datain=NULL;
		AIR_Pixels ***dataout=NULL;
		
		struct AIR_Warp airwarp1;
		struct AIR_Warp airwarp2;
		
		AIR_Pixels (*interp3D)(const struct AIR_Key_info *, AIR_Pixels ***, const double, const double, const double, const double, const unsigned int *, const double)=NULL;
		AIR_Pixels (*interp2D)(const struct AIR_Key_info *, AIR_Pixels ***, const double, const double, const unsigned int, const double, const unsigned int *, const double)=NULL;

		struct AIR_Key_info dimensions3;
		
		{
			AIR_Error errcode;
			
			e=AIR_read_airw(warpfile,&airwarp1,&errcode);
			if(!e){
				free_function(e,g,datain,dataout);
				return(errcode);
			}
		}
		
		if(alternate_reslice_file) strcpy(airwarp1.r_file, alternate_reslice_file);
			
		/* Validate interpolation model */
		{			
			switch(interp){
			
				case 0:
					if(airwarp1.coord==3) interp3D=AIR_interp_nn_3D;
					else interp2D=AIR_interp_nn_2D;
					break;
				case 1:
					if(airwarp1.coord==3) interp3D=AIR_interp_lin_3D;
					else interp2D=AIR_interp_lin_2D;
					break;
				case 2:
					if(window[0]==0 || window[1]==0){
						printf("%s: %d: ",__FILE__,__LINE__);
						printf("User interface allowed invalid interpolation window widths\n");
						free_function(e,g,datain,dataout);
						return AIR_USER_INTERFACE_ERROR;
					}
					if(airwarp1.coord==3) interp3D=AIR_interp_wsinc_xy;
					else interp2D=AIR_interp_wsinc_2D;
					break;
				case 3:
					if(airwarp1.coord==3){
						if(window[0]==0 || window[1]==0){
							printf("%s: %d: ",__FILE__,__LINE__);
							printf("User interface allowed invalid interpolation window widths\n");
							free_function(e,g,datain,dataout);
							return AIR_USER_INTERFACE_ERROR;
						}
						interp3D=AIR_interp_wsinc_xz;
					}
					else{
						printf("%s: %d: ",__FILE__,__LINE__);
						printf("A 2D warp is not compatible with the selected interpolation model\n");
						free_function(e,g,datain,dataout);
						return AIR_INTERP_CANT_2D_ERROR;
					}
					break;
				case 4:
					if(airwarp1.coord==3){
						if(window[0]==0 || window[1]==0){
							printf("%s: %d: ",__FILE__,__LINE__);
							printf("User interface allowed invalid interpolation window widths\n");
							free_function(e,g,datain,dataout);					
							return AIR_USER_INTERFACE_ERROR;
						}
						interp3D=AIR_interp_wsinc_yz;
					}
					else{
						printf("%s: %d: ",__FILE__,__LINE__);
						printf("A 2D warp is not compatible with the selected interpolation model\n");
						free_function(e,g,datain,dataout);					
						return AIR_INTERP_CANT_2D_ERROR;
					}					
					break;	
				case 5:
					if(airwarp1.coord==3){
						if(window[0]==0 || window[1]==0 || window[2]==0){
							printf("%s: %d: ",__FILE__,__LINE__);
							printf("User interface allowed invalid interpolation window widths\n");
							free_function(e,g,datain,dataout);						
							return AIR_USER_INTERFACE_ERROR;
						}
						interp3D=AIR_interp_wsinc_3D;
					}
					else{
						if(window[0]==0 || window[1]==0){
							printf("%s: %d: ",__FILE__,__LINE__);
							printf("User interface allowed invalid interpolation window widths\n");
							free_function(e,g,datain,dataout);						
							return AIR_USER_INTERFACE_ERROR;
						}
						interp2D=AIR_interp_wsinc_2D;
					}
					break;	
				default:
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("User interface provided invalid interpolation model\n");
					free_function(e,g,datain,dataout);				
					return AIR_USER_INTERFACE_ERROR;
			}
		}
		if( !interp2D && !interp3D){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("An appropriate interpolation model could not be assigned\n");
			free_function(e,g,datain,dataout);				
			return AIR_USER_INTERFACE_ERROR;
		}
		
		/* Load and verify the linear estimate file or create estimate */
		if(linear_estimate_file){
			{
				AIR_Error errcode;
				
				g=AIR_read_airw(linear_estimate_file,&airwarp2,&errcode);
				if(!g){
					free_function(e,g,datain,dataout);
					return(errcode);
				}
			}
			if(airwarp2.order!=1){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("Estimate file %s is not linear\n",linear_estimate_file);
				free_function(e,g,datain,dataout);
				return AIR_WARP_NOT_FIRST_ORDER_ERROR;
			}
			{
				AIR_Error errcode=AIR_same_dim(&airwarp1.s,&airwarp2.r);
				
				if(errcode!=0){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("The linear estimate file %s has incorrect dimensions to estimate the inverse of the nonlinear .warp file %s\n",linear_estimate_file,warpfile);
					free_function(e,g,datain,dataout);
					return(errcode);
				}
			}
			{
				AIR_Error errcode=AIR_same_size(&airwarp1.s,&airwarp2.r);
				
				if(errcode!=0){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("The linear estimate file %s has incorrect voxel sizes to estimate the inverse of the nonlinear .warp file %s\n",linear_estimate_file,warpfile);
					free_function(e,g,datain,dataout);
					return(errcode);
				}
			}
			{
				AIR_Error errcode=AIR_same_dim(&airwarp1.r,&airwarp2.s);
				
				if(errcode!=0){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("The linear estimate file %s has incorrect dimensions to estimate the inverse of the nonlinear .warp file %s\n",linear_estimate_file,warpfile);
					free_function(e,g,datain,dataout);
					return(errcode);
				}
			}

			{
				AIR_Error errcode=AIR_same_size(&airwarp1.r,&airwarp2.s);
				
				if(errcode!=0){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("The linear estimate file %s has incorrect voxel sizes to estimate the inverse of the nonlinear .warp file %s\n",linear_estimate_file,warpfile);
					free_function(e,g,datain,dataout);
					return(errcode);
				}
			}
		}			
		else{
			g=AIR_matrix2(4,3);
			if(!g){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("memory allocation failure\n");
				free_function(e,g,datain,dataout);
				return AIR_MEMORY_ALLOCATION_ERROR;
			}
			g[0][0]=g[1][0]=g[2][0]=0.0;
			g[0][1]=g[1][2]=g[2][3]=1.0;
			g[0][2]=g[0][3]=e[1][1]=e[1][3]=e[2][1]=e[2][2]=0.0;
		}	
		{
			AIR_Pixels ***datain=NULL;
			AIR_Pixels ***dataout=NULL;
			
			/*Load the reslice file*/
			{
				AIR_Error errcode;
				/* Only allow nearest neighbor interpolation of binary files */
				datain=AIR_load(airwarp1.r_file, &dimensions3, interp==0, &errcode);
				if(!datain){
					free_function(e,g,datain,dataout);
					return(errcode);
				}
			}

			/*Check the reslice file against the .air reslice file specifications*/
			{
				AIR_Error errcode=AIR_same_dim(&airwarp1.s,&dimensions3);
				
				if(errcode!=0){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("File '%s' not created because of matrix size incompatibility\n",outfile);
					printf("Dimensions expected based on warp file: %u %u %u\n",airwarp1.s.x_dim,airwarp1.s.y_dim,airwarp1.s.z_dim);
					printf("Dimensions of matrix in image file '%s': %u %u %u\n",airwarp1.s_file,dimensions3.x_dim,dimensions3.y_dim,dimensions3.z_dim);
					free_function(e,g,datain,dataout);
					return(errcode);
				}
			}
			{
				AIR_Error errcode=AIR_same_size(&airwarp1.s,&dimensions3);
				
				if(errcode!=0){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("File '%s' not created because of voxel size discrepancy\n",outfile);
					printf("Voxel dimensions expected based on warp file: %.4f %.4f %.4f\n",airwarp1.s.x_size,airwarp1.s.y_size,airwarp1.s.z_size);
					printf("Voxel dimensions in image file '%s': %.4f %.4f %.4f\n",airwarp1.s_file,dimensions3.x_size,dimensions3.y_size,dimensions3.z_size);
					free_function(e,g,datain,dataout);
					return(errcode);
				}
			}
			
			/* Reslice the data */
			{
				AIR_Error errcode;
				
				if(airwarp1.coord==3){
					dataout=AIR_r_uwrp_3D(datain,&airwarp1.s,&airwarp1.r,e,scale,airwarp1.order,g,parity,accuracy,window,interp3D,&errcode);		
				}
				else{
					dataout=AIR_r_uwrp_2D(datain,&airwarp1.s,&airwarp1.r,e,scale,airwarp1.order,g,parity,accuracy,window,interp2D,&errcode);		
				}
				if(!dataout){
					free_function(e,g,datain,dataout);
					return(errcode);
				}
			}

			/*Save the data*/
			airwarp1.r.bits=8*sizeof(AIR_Pixels);
			{
				AIR_Error errcode=AIR_save(dataout,&airwarp1.r,outfile,ow,program);
				
				if(errcode!=0){
					free_function(e,g,datain,dataout);
					return(errcode);
				}
			}
			free_function(e,g,datain,dataout);
		}
	}
	return 0;
}
