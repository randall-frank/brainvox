/* Copyright 1997-2001 Roger P. Woods, M.D. */
/* Modified: 5/30/01 */ 

/*
 * This reslices a file based on a vector field and optional warp field
 */


#include "AIR.h"

static void free_function(/*@null@*/ /*@out@*/ /*@only@*/ float ****field3, /*@null@*/ /*@out@*/ /*@only@*/ float ***field2, /*@null@*/ /*@out@*/ /*@only@*/ double **e, /*@null@*/ /*@out@*/ /*@only@*/ AIR_Pixels ***datain, /*@null@*/ /*@out@*/ /*@only@*/ AIR_Pixels ***dataout)

{	
	if(field3) AIR_free_4f(field3);
	if(field2) AIR_free_3f(field2);
	if(e) AIR_free_2(e);
	if(datain) AIR_free_vol3(datain);
	if(dataout) AIR_free_vol3(dataout);
}

AIR_Error AIR_do_reslice_vector(const char *program, const char *vecfile, const char *outfile, const char *alternate_reslice_file, const char *warpfile, const unsigned int interp, const unsigned int *window, double scale, const char *mult_scale_file, const char *div_scale_file, const AIR_Boolean ow)

{
	if(scale<=0){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("User interface allowed negative scale\n");
		return AIR_USER_INTERFACE_ERROR;
	}
	
	/* Verify the alternate reslice file */
	if(alternate_reslice_file){
		if(strlen(alternate_reslice_file)>=(size_t)AIR_CONFIG_MAX_PATH_LENGTH){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("File name %s is too long to incorporate into a vector field file\n",alternate_reslice_file);
			return AIR_PATH_TOO_LONG_ERROR;
		}
		{
			AIR_Error errcode=AIR_load_probr(alternate_reslice_file,TRUE);
			if(errcode!=0) return(errcode);
		}
	}
	
	/* Verify that vecfile can be read */
	{
		AIR_Error errcode=AIR_fprobr(vecfile);
		
		if(errcode!=0){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("Problem with file %s\n",vecfile);
			return(errcode);
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
		
		if(errcode!=0){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("Anticipated problem saving file %s\n",outfile);
			return errcode;
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
		float ****field3=NULL;
		float ***field2=NULL;
		double **e=NULL;
		AIR_Pixels ***datain=NULL;
		AIR_Pixels ***dataout=NULL;
		
		struct AIR_Field airfield1;
		struct AIR_Warp airwarp1;
		
		AIR_Pixels (*interp3D)(const struct AIR_Key_info *, AIR_Pixels ***, const double, const double, const double, const double, const unsigned int *, const double)=NULL;
		AIR_Pixels (*interp2D)(const struct AIR_Key_info *, AIR_Pixels ***, const double, const double, const unsigned int, const double, const unsigned int *, const double)=NULL;

		struct AIR_Key_info dimensions3;
			
		/* Read in the vector field */
		{
			AIR_Error errcode=AIR_read_airfield(vecfile, &airfield1, &field3, &field2);
			if(errcode!=0) return(errcode);
		}
		/* Read in the warping field */
		if(warpfile){
		
			AIR_Error errcode;
			
			e=AIR_read_airw(warpfile,&airwarp1,&errcode);
			if(!e){
				free_function(field3,field2,e,datain,dataout);
				return(errcode);
			}
			
			if(airfield1.coord!=airwarp1.coord){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("file %s was not created because vector file %s did not have the same number of coordinates as warp file %s\n",outfile,vecfile,warpfile);
				free_function(field3,field2,e,datain,dataout);
				return AIR_VECTOR_FIELD_MISMATCH_ERROR;
			}
			
			/* Verify that the standard file matches the reslice file of the vector field */
			{
				AIR_Error errcode=AIR_same_dim(&airwarp1.s,&airfield1.r);
				
				if(errcode!=0){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("file %s was not created because vector file %s did not match dimensions of warp file %s\n",outfile,vecfile,warpfile);
					free_function(field3,field2,e,datain,dataout);
					return(errcode);
				}
			}
			{
				AIR_Error errcode=AIR_same_size(&airwarp1.s,&airfield1.r);
				
				if(errcode!=0){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("file %s was not created because vector file %s did not match voxel sizes of warp file %s\n",outfile,vecfile,warpfile);
					free_function(field3,field2,e,datain,dataout);
					return(errcode);
				}
			}
		}
		else{
			airwarp1.s=airfield1.r;
			airwarp1.r=airfield1.r;
			strcpy(airwarp1.r_file,airfield1.r_file);
			strcpy(airwarp1.s_file,airfield1.r_file);
			airwarp1.coord=airfield1.coord;
			airwarp1.order=1;
			
			/* Create e suitable for 2D or 3D order */
			e=AIR_matrix2(4,3);
			if(!e){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("memory allocation failure\n");
				free_function(field3,field2,e,datain,dataout);
				return AIR_MEMORY_ALLOCATION_ERROR;
			}
			{
				unsigned int j;
				
				for(j=0;j<3;j++){
				
					unsigned int i;
					
					for(i=0;i<4;i++){
						if(i==j+1) e[j][i]=1;
						else e[j][i]=0;
					}
				}
			}
		}
		if(alternate_reslice_file) strncpy(airwarp1.r_file,alternate_reslice_file,(size_t)AIR_CONFIG_MAX_PATH_LENGTH-1);
		
		/* Validate interpolation model */
		{			
			switch(interp){
			
				case 0:
					if(airfield1.coord==3) interp3D=AIR_interp_nn_3D;
					else interp2D=AIR_interp_nn_2D;
					break;
				case 1:
					if(airfield1.coord==3) interp3D=AIR_interp_lin_3D;
					else interp2D=AIR_interp_lin_2D;
					break;
				case 2:
					if(window[0]==0 || window[1]==0){
						printf("%s: %d: ",__FILE__,__LINE__);
						printf("User interface allowed invalid interpolation window widths\n");
						free_function(field3,field2,e,datain,dataout);
						return AIR_USER_INTERFACE_ERROR;
					}
					if(airfield1.coord==3) interp3D=AIR_interp_wsinc_xy;
					else interp2D=AIR_interp_wsinc_2D;
					break;
				case 3:
					if(airfield1.coord==3){
						if(window[0]==0 || window[1]==0){
							printf("%s: %d: ",__FILE__,__LINE__);
							printf("User interface allowed invalid interpolation window widths\n");
							free_function(field3,field2,e,datain,dataout);
							return AIR_USER_INTERFACE_ERROR;
						}
						interp3D=AIR_interp_wsinc_xz;
					}
					else{
						printf("%s: %d: ",__FILE__,__LINE__);
						printf("A 2D warp is not compatible with the selected interpolation model\n");
						free_function(field3,field2,e,datain,dataout);
						return AIR_INTERP_CANT_2D_ERROR;
					}
					break;
				case 4:
					if(airfield1.coord==3){
						if(window[0]==0 || window[1]==0){
							printf("%s: %d: ",__FILE__,__LINE__);
							printf("User interface allowed invalid interpolation window widths\n");
							free_function(field3,field2,e,datain,dataout);					
							return AIR_USER_INTERFACE_ERROR;
						}
						interp3D=AIR_interp_wsinc_yz;
					}
					else{
						printf("%s: %d: ",__FILE__,__LINE__);
						printf("A 2D warp is not compatible with the selected interpolation model\n");
						free_function(field3,field2,e,datain,dataout);					
						return AIR_INTERP_CANT_2D_ERROR;
					}					
					break;	
				case 5:
					if(airfield1.coord==3){
						if(window[0]==0 || window[1]==0 || window[2]==0){
							printf("%s: %d: ",__FILE__,__LINE__);
							printf("User interface allowed invalid interpolation window widths\n");
							free_function(field3,field2,e,datain,dataout);						
							return AIR_USER_INTERFACE_ERROR;
						}
						interp3D=AIR_interp_wsinc_3D;
					}
					else{
						if(window[0]==0 || window[1]==0){
							printf("%s: %d: ",__FILE__,__LINE__);
							printf("User interface allowed invalid interpolation window widths\n");
							free_function(field3,field2,e,datain,dataout);						
							return AIR_USER_INTERFACE_ERROR;
						}
						interp2D=AIR_interp_wsinc_2D;
					}
					break;	
				default:
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("User interface provided invalid interpolation model\n");
					free_function(field3,field2,e,datain,dataout);				
					return AIR_USER_INTERFACE_ERROR;
			}
		}
		if( !interp2D && !interp3D){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("An appropriate interpolation model could not be assigned\n");
			free_function(field3,field2,e,datain,dataout);				
			return AIR_USER_INTERFACE_ERROR;
		}
		{
			AIR_Pixels ***datain=NULL;
			AIR_Pixels ***dataout=NULL;
			
			/*Load the reslice file*/
			{
				AIR_Error errcode;
				/* Allow only nearest neighbor interpolation of binary files */
				datain=AIR_load(airwarp1.r_file, &dimensions3, interp==0, &errcode);
				if(!datain){
					free_function(field3,field2,e,datain,dataout);
					return(errcode);
				}
			}

			/*Check the reslice file against the .air reslice file specifications*/
			{
				AIR_Error errcode=AIR_same_dim(&airwarp1.r,&dimensions3);
				
				if(errcode!=0){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("File '%s' not created because of matrix size incompatibility\n",outfile);
					printf("Dimensions expected based on warp file: %u %u %u\n",airwarp1.r.x_dim,airwarp1.r.y_dim,airwarp1.r.z_dim);
					printf("Dimensions of matrix in image file '%s': %u %u %u\n",airwarp1.r_file,dimensions3.x_dim,dimensions3.y_dim,dimensions3.z_dim);
					free_function(field3,field2,e,datain,dataout);
					return(errcode);
				}
			}
			{
				AIR_Error errcode=AIR_same_size(&airwarp1.r,&dimensions3);
				
				if(errcode!=0){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("File '%s' not created because of voxel size discrepancy\n",outfile);
					printf("Voxel dimensions expected based on warp file: %.4f %.4f %.4f\n",airwarp1.r.x_size,airwarp1.r.y_size,airwarp1.r.z_size);
					printf("Voxel dimensions in image file '%s': %.4f %.4f %.4f\n",airwarp1.r_file,dimensions3.x_size,dimensions3.y_size,dimensions3.z_size);
					free_function(field3,field2,e,datain,dataout);
					return(errcode);
				}
			}
			
			/* Reslice the data */
			{
				AIR_Error errcode;
				
				if(airfield1.coord==3){
					dataout=AIR_r_vec_warp_3D(datain,&airwarp1.r,&airfield1.s,field3,e,scale,airwarp1.order,window,interp3D,&errcode);		
				}
				else{
					dataout=AIR_r_vec_warp_2D(datain,&airwarp1.r,&airfield1.s,field2,e,scale,airwarp1.order,window,interp2D,&errcode);		
				}
				if(!dataout){
					free_function(field3,field2,e,datain,dataout);
					return(errcode);
				}
			}
			
			/*Save the data*/
			airwarp1.s.bits=8*sizeof(AIR_Pixels);
			{
				AIR_Error errcode=AIR_save(dataout,&airfield1.s,outfile,ow,program);
				
				if(errcode!=0){
					free_function(field3,field2,e,datain,dataout);
					return(errcode);
				}
			}
			free_function(field3,field2,e,datain,dataout);
		}
	}

	return 0;
}
