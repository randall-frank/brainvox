/* Copyright 1995-2002 Roger P. Woods, M.D. */
/* Modified  1/16/02  */

/*
 * This function receives file names and parameter values from
 * a user interface and then performs the appropriate file I/O
 * operations before and after calling align().
 *
 * Resource utilization is deferred until as much error checking
 * as possible has been completed.
 */


#include "AIR.h"

#define COORDS 4		/*Number of coordinates referenced in AIR file*/
#define COEFFS 4		/*Number of coefficients referenced in AIR file*/
#define MAX_PARAMS 16		/*Maximum number of parameters in any implemented model including rescaling*/


static void free_function(/*@null@*/ /*@out@*/ /*@only@*/ AIR_Pixels ***pixel1, /*@null@*/ /*@out@*/ /*@only@*/ AIR_Pixels ***pixel2, /*@null@*/ /*@out@*/ /*@only@*/ AIR_Pixels ***pixel1e, /*@null@*/ /*@out@*/ /*@only@*/ AIR_Pixels ***pixel2e, /*@null@*/ /*@out@*/ /*@only@*/ double **es)

{
	if(pixel1) AIR_free_vol3(pixel1);
	if(pixel2) AIR_free_vol3(pixel2);
	if(pixel1e) AIR_free_vol3(pixel1e);
	if(pixel2e) AIR_free_vol3(pixel2e);
	if(es) AIR_free_2(es);
}


AIR_Error AIR_do_alignlinear(const char *program, const unsigned int model, const char *air_file, const AIR_Boolean overwrite, const char *standard_file, long int threshold1, const float kx1, const float ky1, const float kz1, const unsigned int partitions1, const AIR_Boolean dynamic1, const char *standard_file_mask, const char *reslice_file, long int threshold2, const float kx2, const float ky2, const float kz2, const unsigned int partitions2, const AIR_Boolean dynamic2, const char *reslice_file_mask, const char *initialization_file, const char *scaling_initialization_file, const char *termination_file, const AIR_Boolean overwrite_termination_file, const char *scaling_termination_file, const AIR_Boolean overwrite_scaling_termination_file, const unsigned int samplefactor, const unsigned int samplefactor2, const unsigned int sffactor, const float precision, const unsigned int iterations, const unsigned int noprogtries, const AIR_Boolean interaction, const AIR_Boolean verbosity, const AIR_Boolean zooming, const unsigned int costfxn, const AIR_Boolean posdefreq)

{
	struct AIR_Air16	air1;	/*AIR file to be output including file names, dimensions,reslice parameters*/
	struct AIR_Air16	air1e;	/*pseudo AIR file for edited images*/

	unsigned int parameters;		/*number of formal parameters in specified model*/
	unsigned int param_int;		/*number of parameters in derivative routine uvfN*/
	unsigned int scales=(unsigned int)(costfxn==3);		/* number of rescaling parameters */
	AIR_Boolean memory=(costfxn==1);		/* Tells align routine whether to allocate extra memory needed for RIU cost function */
	unsigned int index=0;		/*index of current parameter being read from initialization file*/

	AIR_Pixels
		mapped_threshold1,
		mapped_threshold2;
	
	void	(*uvfN)(const double *, double **, double ***, double ****, const struct AIR_Key_info *, const struct AIR_Key_info *, const AIR_Boolean);

	double	(*uvderivsN)(const unsigned int, double **, double ***, double ****, const unsigned int, AIR_Pixels ***, const struct AIR_Key_info *, AIR_Pixels ***, const struct AIR_Key_info *, const AIR_Pixels, double *, double **, unsigned int *, double *, double *, double **, double **, double ***, double ***, const unsigned int, AIR_Pixels, double, const AIR_Boolean, unsigned int *)=NULL;

	double tps[MAX_PARAMS];

	/* Warn of plan to overwrite existing .air file */
	if(overwrite){
		FILE *fp=fopen(air_file,"rb");
		
		if(fp){
			if(fclose(fp)!=0){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("failed to close file %s\n",air_file);
				return AIR_CANT_CLOSE_READ_ERROR;
			}
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("\nWARNING: File '%s' will be overwritten by the output of this program\n",air_file);
		}
	}
	/* Look for fatal conflicts that should have been caught by the user interface */
	
	if(kx1<0 || ky1<0 || kz1<0 || kx2<0 || ky2<0 || kz2<0){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("User interface allowed negative FWHM for smoothing kernel\n");
		return AIR_USER_INTERFACE_ERROR;
	}
	if(samplefactor==0 || samplefactor2==0 ||sffactor==0){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("User interface allowed zero sampling\n");
		return AIR_USER_INTERFACE_ERROR;
	}
	if(samplefactor2>samplefactor){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("User interface allowed final sampling sparser than initial sampling\n");
		return AIR_USER_INTERFACE_ERROR;
	}
	if(sffactor==1 && samplefactor!=samplefactor2){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("User interface allowed sampling that will never converge\n");
		return AIR_USER_INTERFACE_ERROR;
	}
	if(precision<=0){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("User interface allowed zero or negative precision\n");
		return AIR_USER_INTERFACE_ERROR;
	}

	/* Invalid numbers for model and costfxn are handled later in switch statements */

	if(costfxn!=1 && (partitions1>1 || partitions2>1)){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("User interface allowed use of more than one partition with cost function other than ratio image uniformity\n");
		return AIR_USER_INTERFACE_ERROR;
	}
	if(scaling_initialization_file!=NULL && costfxn!=3){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("User interface allowed use of scaling initialization file with a cost function that does not use scaling\n");
		return AIR_USER_INTERFACE_ERROR;
	}
	if(scaling_termination_file!=NULL && costfxn!=3){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("User interface allowed use of scaling termination file with a cost function that does not use scaling\n");
		return AIR_USER_INTERFACE_ERROR;
	}
	if(partitions1+partitions2<1){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("User interface allowed allocation of no partitions for either standard or reslice file\n");
		return AIR_USER_INTERFACE_ERROR;
	}
	if(termination_file){
		if(strcmp(air_file, termination_file)==0){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("User interface allowed output files with identical names\n");
			return AIR_USER_INTERFACE_ERROR;
		}
	}
	if(scaling_termination_file){
		if(strcmp(air_file, scaling_termination_file)==0){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("User interface allowed output files with identical names\n");
			return AIR_USER_INTERFACE_ERROR;
		}
		if(termination_file){
			if(strcmp(termination_file, scaling_termination_file)==0){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("User interface allowed output files with identical names\n");
				return AIR_USER_INTERFACE_ERROR;
			}
		}
	}

	/* Bail out if the file names are too long to store in a .air file */	
	if(strlen(standard_file)>=(size_t)AIR_CONFIG_MAX_PATH_LENGTH){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("File name %s is too long to incorporate into a .air file\n",standard_file);
		return AIR_PATH_TOO_LONG_ERROR;
	}
	if(strlen(reslice_file)>=(size_t)AIR_CONFIG_MAX_PATH_LENGTH){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("File name %s is too long to incorporate into a .air file\n",reslice_file);
		return AIR_PATH_TOO_LONG_ERROR;
	}
	if(standard_file_mask){
		if(strlen(standard_file_mask)>=(size_t)AIR_CONFIG_MAX_PATH_LENGTH){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("File name %s is too long to incorporate into a .air file\n",standard_file_mask);
			return AIR_PATH_TOO_LONG_ERROR;
		}
	}
	if(reslice_file_mask){
		if(strlen(reslice_file_mask)>=(size_t)AIR_CONFIG_MAX_PATH_LENGTH){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("File name %s is too long to incorporate into a .air file\n",reslice_file_mask);
			return AIR_PATH_TOO_LONG_ERROR;
		}
	}
			
	/* Bail out if it won't be possible to read the input files */
	{
		AIR_Error errcode=AIR_load_probr(standard_file,TRUE);
		
		if(errcode!=0){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("problem with file %s\n",standard_file);
			return(errcode);
		}
	}
	if(standard_file_mask){
		AIR_Error errcode=AIR_load_probr(standard_file_mask,TRUE);
		
		if(errcode!=0){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("problem with file %s\n",standard_file_mask);
			return(errcode);
		}
	}
	{
		AIR_Error errcode=AIR_load_probr(reslice_file,TRUE);
		
		if(errcode!=0){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("problem with file %s\n",reslice_file);
			return(errcode);
		}
	}
	if(reslice_file_mask){
		AIR_Error errcode=AIR_load_probr(reslice_file_mask,TRUE);
		
		if(errcode!=0){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("problem with file %s\n",reslice_file_mask);
			return(errcode);
		}
	}
	if(initialization_file){

		AIR_Error errcode=AIR_fprobr(initialization_file);
		
		if(errcode!=0){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("\nProblem with file %s\n", initialization_file);
			return(errcode);
		}
	}
	if(scaling_initialization_file){

		AIR_Error errcode=AIR_fprobr(scaling_initialization_file);
		
		if(errcode!=0){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("\nProblem with file %s\n", scaling_initialization_file);
			return(errcode);
		}
	}
	/* Bail out if it won't be possible to write the output files */
	{
		AIR_Error errcode=AIR_fprobw(air_file,overwrite);
		
		if(errcode!=0){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("problem with file %s\n",air_file);
			return(errcode);
		}
	}
	if(termination_file){

		AIR_Error errcode=AIR_fprobw(termination_file,overwrite_termination_file);
		
		if(errcode!=0){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("\nProblem with file %s\n", termination_file);
			return(errcode);
		}
	}
	if(scaling_termination_file){

		AIR_Error errcode=AIR_fprobw(scaling_termination_file,overwrite_scaling_termination_file);
		
		if(errcode!=0){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("\nProblem with file %s\n", scaling_termination_file);
			return(errcode);
		}
	}

	switch(model){
		case 6:
			parameters=6;
			param_int=12;
			uvfN=AIR_uv3D6;
			break;
		case 7:
			parameters=7;
			param_int=12;
			uvfN=AIR_uv3D7;
			break;
		case 9:
			parameters=9;
			param_int=12;
			uvfN=AIR_uv3D9;
			break;
		case 12:
			parameters=12;
			param_int=12;
			uvfN=AIR_uv3D12;
			break;
		case 15:
			parameters=15;
			param_int=15;
			uvfN=AIR_uv3D15;
			break;
		case 23:
			parameters=3;
			param_int=6;
			uvfN=AIR_uv2D3;
			break;
		case 24:
			parameters=4;
			param_int=6;
			uvfN=AIR_uv2D4;
			break;
		case 25:
			parameters=5;
			param_int=6;
			uvfN=AIR_uv2D5;
			break;
		case 26:
			parameters=6;
			param_int=6;
			uvfN=AIR_uv2D6;
			break;
		case 28:
			parameters=8;
			param_int=8;
			uvfN=AIR_uv2D8;
			break;
		default:
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("User interface passed invalid model number\n");
			return AIR_USER_INTERFACE_ERROR;
	}
	
	/* Get the data from the input file headers */
	{
		struct AIR_Fptrs fps;
		int flag[8];
		
		strcpy(air1.s_file,standard_file);

		(void)AIR_open_header(air1.s_file, &fps, &air1.s, flag);

		if(fps.errcode!=0){
			AIR_close_header(&fps);
			return(fps.errcode);
		}

		AIR_close_header(&fps);
		if(fps.errcode!=0){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("Failed to close file %s\n",air1.s_file);
			return(fps.errcode);
		}
	}
	{
		struct AIR_Fptrs fps;
		int flag[8];
		
		strcpy(air1.r_file,reslice_file);

		(void)AIR_open_header(air1.r_file, &fps, &air1.r, flag);
		if(fps.errcode!=0){
			AIR_close_header(&fps);
			return(fps.errcode);
		}

		AIR_close_header(&fps);
		if(fps.errcode!=0){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("Failed to close file %s\n",air1.r_file);
			return(fps.errcode);
		}
	}
	/* Compute and validate the internal representation of the default thresholds */
	{
		AIR_Error errcode;
		mapped_threshold1=AIR_map_value(air1.s_file,threshold1,&errcode);
		
		if(errcode!=0){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("Unable to map threshold1 to internal representation value\n");
			return(errcode);
		}	
	}
	{
		AIR_Error errcode;
		mapped_threshold2=AIR_map_value(air1.r_file,threshold2,&errcode);
		
		if(errcode!=0){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("Unable to map threshold2 to internal representation value\n");
			return(errcode);
		}
	}

	/* Read the intensity parameter initialization */
	if(scaling_initialization_file){
	
		FILE *fp=fopen(scaling_initialization_file,"rb");

		if (!fp){
			
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("\nunable to open initialization file '%s'\n",scaling_initialization_file);
			{
				AIR_Error errcode=AIR_fprobr(scaling_initialization_file);
				if(errcode==0) errcode=AIR_UNSPECIFIED_FILE_READ_ERROR;
				return(errcode);
			}
		}
			
		if(fscanf(fp,"%le",tps+parameters)==EOF){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("unable to read value from initialization file '%s'\n",scaling_initialization_file);
			return AIR_READ_SCALING_FILE_ERROR;
		}
		if(fclose(fp)!=0){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("failed to close file %s\n",scaling_initialization_file);
			return AIR_CANT_CLOSE_READ_ERROR;
		}
	}
	else tps[parameters]=1.0;
	
	/* Read the spatial parameter initializations */
	if(initialization_file){
			
		FILE *fp=fopen(initialization_file,"rb");

		if (!fp){
					
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("\nunable to open initialization file '%s'\n",initialization_file);
			{
				AIR_Error errcode=AIR_fprobr(initialization_file);
				if(errcode==0) errcode=AIR_UNSPECIFIED_FILE_READ_ERROR;
				return(errcode);
			}
		}
		for(index=0;index<parameters;index++){
			if(fscanf(fp,"%le",&tps[index])==EOF){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("The initialization file %s provided fewer than the expected %u parameters\n",initialization_file, parameters);
				if(fclose(fp)!=0){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("failed to close file %s\n",initialization_file);
				}
				return AIR_READ_INIT_FILE_ERROR;
			}
		}
		{
			double junk;
			
			if(fscanf(fp,"%le",&junk)!=EOF){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("The initialization file %s provided more than the expected %u parameters\n",initialization_file, parameters);
				if(fclose(fp)!=0){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("failed to close file %s\n",initialization_file);
				}
				return AIR_READ_INIT_FILE_ERROR;
			}
		}
		if(fclose(fp)!=0){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("failed to close file %s\n",initialization_file);
			return AIR_CANT_CLOSE_READ_ERROR;
		}
	}
	else{

		switch(model){
		
			case 9:
				tps[7]=1;
				tps[8]=1;
			/*@fallthrough@*/case 7:
				tps[6]=1;
			/*@fallthrought@*/case 6:
				tps[0]=tps[1]=tps[2]=tps[3]=tps[4]=tps[5]=0;
				break;
				
			case 15:
				tps[12]=0;
				tps[13]=0;
				tps[14]=0;
			/*@fallthrough@*/case 12:
				tps[0]=air1.s.x_size/air1.r.x_size;
				tps[1]=0;
				tps[2]=0;
				tps[3]=(air1.r.x_dim-air1.s.x_dim*(air1.s.x_size/air1.r.x_size))/2;

				tps[4]=0;
				tps[5]=air1.s.y_size/air1.r.y_size;
				tps[6]=0;
				tps[7]=(air1.r.y_dim-air1.s.y_dim*(air1.s.y_size/air1.r.y_size))/2;

				tps[8]=0;
				tps[9]=0;
				tps[10]=air1.s.z_size/air1.r.z_size;
				tps[11]=(air1.r.z_dim-air1.s.z_dim*(air1.s.z_size/air1.r.z_size))/2;
				break;
				
			case 24:
				tps[3]=1;
			/*@fallthrough@*/case 23:
				tps[0]=0;
				tps[1]=0;
				tps[2]=0;
				break;
				
			case 25:
				tps[0]=air1.s.x_size/air1.r.x_size;
				tps[1]=0;
				tps[2]=(air1.r.x_dim-air1.s.x_dim*(air1.s.x_size/air1.r.x_size))/2;
				tps[3]=0;
				tps[4]=(air1.r.y_dim-air1.s.y_dim*(air1.s.y_size/air1.r.y_size))/2;
				break;
				
			case 28:
				tps[6]=0;
				tps[7]=0;
			/*@fallthrough@*/case 26:
				tps[0]=air1.s.x_size/air1.r.x_size;
				tps[1]=0;
				tps[2]=(air1.r.x_dim-air1.s.x_dim*(air1.s.x_size/air1.r.x_size))/2;
				tps[3]=0;
				tps[4]=air1.s.y_size/air1.r.y_size;
				tps[5]=(air1.r.y_dim-air1.s.y_dim*(air1.s.y_size/air1.r.y_size))/2;
				break;
		}
	}
	switch(costfxn){
		case 1:
			if(interaction){
				switch(param_int){
					case 15:
						uvderivsN=AIR_uvderivsN15;
						break;
					case 12:
						uvderivsN=AIR_uvderivsN12;
						break;
					case 8:
						uvderivsN=AIR_uvderivsN8;
						break;
					case 6:
						uvderivsN=AIR_uvderivsN6;
						break;
				}
			}
			else{
				switch(param_int){
					case 15:
						uvderivsN=AIR_qvderivsN15;
						break;
					case 12:
						uvderivsN=AIR_qvderivsN12;
						break;
					case 8:
						uvderivsN=AIR_qvderivsN8;
						break;
					case 6:
						uvderivsN=AIR_qvderivsN6;
						break;
				}
			}
			break;
		case 2:
			if(interaction){
				switch(param_int){
					case 15:
						uvderivsN=AIR_uvderivsLS15;
						break;
					case 12:
						uvderivsN=AIR_uvderivsLS12;
						break;
					case 8:
						uvderivsN=AIR_uvderivsLS8;
						break;
					case 6:
						uvderivsN=AIR_uvderivsLS6;
						break;
				}
			}
			else{
				switch(param_int){
					case 15:
						uvderivsN=AIR_qvderivsLS15;
						break;
					case 12:
						uvderivsN=AIR_qvderivsLS12;
						break;
					case 8:
						uvderivsN=AIR_qvderivsLS8;
						break;
					case 6:
						uvderivsN=AIR_qvderivsLS6;
						break;
				}
			}
			break;
		case 3:
			if(interaction){
				switch(param_int){
					case 15:
						uvderivsN=AIR_uvderivsRS15;
						break;
					case 12:
						uvderivsN=AIR_uvderivsRS12;
						break;
					case 8:
						uvderivsN=AIR_uvderivsRS8;
						break;
					case 6:
						uvderivsN=AIR_uvderivsRS6;
						break;
				}
			}
			else{
				switch(param_int){
					case 15:
						uvderivsN=AIR_qvderivsRS15;
						break;
					case 12:
						uvderivsN=AIR_qvderivsRS12;
						break;
					case 8:
						uvderivsN=AIR_qvderivsRS8;
						break;
					case 6:
						uvderivsN=AIR_qvderivsRS6;
						break;
				}
			}
			param_int++; /* This allows allocation of space for the scaling parameter */
			break;
		default:
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("User interface allowed specification of an unimplemented cost function\n");
			return AIR_USER_INTERFACE_ERROR;
	}
	if(!uvderivsN){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("User interface allowed specification of an invalid number of internal parameters\n");
		return AIR_USER_INTERFACE_ERROR;
	}

	/*For 2D models, verify equivalent z dimensions*/
	/*For 3D models, verify that both files don't have only a single plane*/
	if(model==23||model==24||model==25||model==26||model==28){
		if(air1.s.z_dim!=air1.r.z_dim){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("The number of planes in the standard and reslice files must be identical for 2D alignment models\n");
			printf("Standard file (%s) has %u planes and reslice file (%s) has %u planes\n",air1.s_file,air1.s.z_dim,air1.r_file,air1.r.z_dim);
			return AIR_CANT_2D_UNMATCHED_ERROR;
		}
		if(air1.s.z_size!=air1.r.z_size){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("WARNING: the voxel z_size differs for the two files that you are aligning using a 2D in-plane model\n");
			printf("standard (%s) z_size=%e, reslice (%s) z_size=%e\n",air1.s_file,air1.s.z_size,air1.r_file,air1.r.z_size);
		}
	}
	else{	/* 3D model*/
		if(air1.s.z_dim==1 && air1.r.z_dim==1){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("Standard (%s) and reslice (%s) files both have only a single plane of data\n",air1.s_file,air1.r_file);
			printf("You must use a 2D model in this situation\n");
			return AIR_CANT_3D_SINGLE_PLANE_ERROR;
		}
	}
	if(zooming){
		/*Adjust initialization of certain models for zooming as needed*/
		if(model==25){
		
			double pixel_size_s=air1.s.x_size;
			if(air1.s.y_size<pixel_size_s) pixel_size_s=air1.s.y_size;
			if(air1.s.z_size<pixel_size_s) pixel_size_s=air1.s.z_size;

			tps[0]*=pixel_size_s/air1.s.x_size;
			tps[1]*=pixel_size_s/air1.s.y_size;
			tps[3]*=pixel_size_s/air1.s.x_size;
		}

		if(model==26||model==28){
		
			double pixel_size_s=air1.s.x_size;
			if(air1.s.y_size<pixel_size_s) pixel_size_s=air1.s.y_size;
			if(air1.s.z_size<pixel_size_s) pixel_size_s=air1.s.z_size;

			tps[0]*=pixel_size_s/air1.s.x_size;
			tps[1]*=pixel_size_s/air1.s.y_size;
			tps[3]*=pixel_size_s/air1.s.x_size;
			tps[4]*=pixel_size_s/air1.s.y_size;
			if(model==28){
				tps[6]*=pixel_size_s/air1.s.x_size;
				tps[7]*=pixel_size_s/air1.s.y_size;
			}
		}

		if(model==12||model==15){

			double pixel_size_s=air1.s.x_size;
			if(air1.s.y_size<pixel_size_s) pixel_size_s=air1.s.y_size;
			if(air1.s.z_size<pixel_size_s) pixel_size_s=air1.s.z_size;

			tps[0]*=pixel_size_s/air1.s.x_size;
			tps[1]*=pixel_size_s/air1.s.y_size;
			tps[2]*=pixel_size_s/air1.s.z_size;
			tps[4]*=pixel_size_s/air1.s.x_size;
			tps[5]*=pixel_size_s/air1.s.y_size;
			tps[6]*=pixel_size_s/air1.s.z_size;
			tps[8]*=pixel_size_s/air1.s.x_size;
			tps[9]*=pixel_size_s/air1.s.y_size;
			tps[10]*=pixel_size_s/air1.s.z_size;
			if(model==15){
				tps[12]*=pixel_size_s/air1.s.x_size;
				tps[13]*=pixel_size_s/air1.s.y_size;
				tps[14]*=pixel_size_s/air1.s.z_size;
			}
		}
	}

	if(standard_file_mask){
		{
			struct AIR_Fptrs fps;
			int flag[8];
			
			strcpy(air1e.s_file,standard_file_mask);

			(void)AIR_open_header(air1e.s_file, &fps, &air1e.s, flag);
			if(fps.errcode!=0){
				AIR_close_header(&fps);
				return(fps.errcode);
			}
			AIR_close_header(&fps);
			if(fps.errcode!=0){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("Failed to close file %s\n",air1e.s_file);
				return(fps.errcode);
			}
		}
		/* Verify that mask has same dimensions as data */
		{
			AIR_Error errcode=AIR_same_size(&air1.s, &air1e.s);
			
			if(errcode!=0){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("Dimension mismatch: %s: %u %u %u, %s: %u %u %u\n",air1.s_file,air1.s.x_dim,air1.s.y_dim,air1.s.z_dim,air1e.s_file,air1e.s.x_dim,air1e.s.y_dim,air1e.s.z_dim);
				return(errcode);
			}
		}
	}

	if(reslice_file_mask){
		{
			struct AIR_Fptrs fps;
			int flag[8];
			
			strcpy(air1e.r_file,reslice_file_mask);

			(void)AIR_open_header(air1e.r_file, &fps, &air1e.r, flag);
			if(fps.errcode!=0){
				AIR_close_header(&fps);
				return(fps.errcode);
			}
			AIR_close_header(&fps);
			if(fps.errcode!=0){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("Failed to close file %s\n",air1e.r_file);
				return(fps.errcode);
			}
		}
		/* Verify that mask has same dimensions as data */
		{
			AIR_Error errcode=AIR_same_size(&air1.r, &air1e.r);
			
			if(errcode!=0){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("Dimension mismatch: %s: %u %u %u, %s: %u %u %u\n",air1.r_file,air1.r.x_dim,air1.r.y_dim,air1.r.z_dim,air1e.r_file,air1e.r.x_dim,air1e.r.y_dim,air1e.r.z_dim);
				return(errcode);
			}
		}
	}
	
	/* Load and process the image data */
	{
		AIR_Pixels ***pixel1=NULL;	/*pointer to original pixel values of standard file*/
		AIR_Pixels ***pixel2=NULL;	/*pointer to original pixel values of file to reslice*/

		AIR_Pixels ***pixel1e=NULL;	/* editing mask for standard file or NULL */
		AIR_Pixels ***pixel2e=NULL;	/* editing mask for reslice file or NULL */
		
		double **es=NULL;

	
		/* Load the input files */
		{
			AIR_Error errcode; 
			
			pixel1=AIR_load(air1.s_file, &air1.s, 0, &errcode);
			if(!pixel1){
				free_function(pixel1,pixel2,pixel1e,pixel2e,es);
				return(errcode);
			}
		}
		air1.s_hash=AIR_hash(pixel1,&air1.s);

		{
			AIR_Error errcode;

			pixel2=AIR_load(air1.r_file, &air1.r, 0, &errcode);
			if(!pixel2){
				free_function(pixel1,pixel2,pixel1e,pixel2e,es);
				return(errcode);
			}
		}
		air1.r_hash=AIR_hash(pixel2,&air1.r);
		
		/* Load the masks */
		if(standard_file_mask){
			{
				AIR_Error errcode;

				pixel1e=AIR_load(air1e.s_file, &air1e.s, TRUE, &errcode);
				if(!pixel1e){
					free_function(pixel1,pixel2,pixel1e,pixel2e,es);
					return(errcode);
				}
			}
		}
		if(reslice_file_mask){
			{
				AIR_Error errcode;

				pixel2e=AIR_load(air1e.r_file, &air1e.r, TRUE, &errcode);
				if(!pixel2e){
					free_function(pixel1,pixel2,pixel1e,pixel2e,es);
					return(errcode);
				}
			}
		}

		/* Smooth if requested--must be done before any masking is applied */
		if(kx1!=0.0||ky1!=0.0||kz1!=0.0){
		
			AIR_Error errcode=AIR_gausssmaller(pixel1,&air1.s,kx1,ky1,kz1);
			
			if(errcode!=0){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("Failure in smoothing routine\n");
				free_function(pixel1,pixel2,pixel1e,pixel2e,es);
				return(errcode);
			}
		}
		if(kx2!=0.0||ky2!=0.0||kz2!=0.0){
		
			AIR_Error errcode=AIR_gausssmaller(pixel2,&air1.r,kx2,ky2,kz2);
			
			if(errcode!=0){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("Failure in smoothing routine\n");
				free_function(pixel1,pixel2,pixel1e,pixel2e,es);
				return(errcode);
			}
		}

		/* Copy data wherever masks apply */

		if(pixel1e){
			{
				unsigned int k;
				
				AIR_Pixels *pixel1ekji=**pixel1e;
				AIR_Pixels *pixel1kji=**pixel1;
				
				for(k=0;k<air1.s.z_dim;k++){
				
					unsigned int j;
					
					for(j=0;j<air1.s.y_dim;j++){
					
						unsigned int i;
						
						for(i=0;i<air1.s.x_dim;i++, pixel1ekji++, pixel1kji++){
						
							if(*pixel1ekji!=0) *pixel1ekji=*pixel1kji;
						}
					}
				}
			}
		}
		if(pixel2e){

			{
				unsigned int k;
				
				AIR_Pixels *pixel2ekji=**pixel2e;
				AIR_Pixels *pixel2kji=**pixel2;
				
				for(k=0;k<air1.r.z_dim;k++){
				
					unsigned int j;
					
					for(j=0;j<air1.r.y_dim;j++){
					
						unsigned int i;
						
						for(i=0;i<air1.r.x_dim;i++, pixel2ekji++, pixel2kji++){
						
							if(*pixel2ekji!=0) *pixel2ekji=*pixel2kji;
						}
					}
				}
			}
		}


		/*Perform alignment*/

		{
			AIR_Error errcode;
			
			es=AIR_align(parameters,tps,uvfN,param_int,uvderivsN,pixel1,pixel1e,&air1.s,mapped_threshold1,pixel2,pixel2e,&air1.r,mapped_threshold2,samplefactor,samplefactor2,sffactor,precision,iterations,noprogtries,zooming,partitions1,partitions2,dynamic1,dynamic2,scales,COORDS,COEFFS,memory,posdefreq,verbosity,&errcode);

			if(!es){
				free_function(pixel1,pixel2,pixel1e,pixel2e,es);
				return(errcode);
			}
		}

		/*Assemble comment for air file*/
		{
			char lprogram[16];
			{
				const char *progname=strrchr(program,AIR_CONFIG_PATH_SEP);

				if(!progname) progname=program;
				else progname++;
				{
					size_t n=strlen(progname);
					
					if(n>15) n=15;
					strncpy(lprogram,progname,n);
					lprogram[n]='\0';
				}
			}
			{
				char comment[2*AIR_CONFIG_MAX_COMMENT_LENGTH];
				
				sprintf(comment,"%s t=(%li %li) sf=(%u %u %u) prec=%.4f its=%u %u zm=%i pts=%u  %u model=%u",lprogram,threshold1,threshold2,samplefactor,samplefactor2,sffactor,precision,iterations,noprogtries,(int)zooming,partitions1,partitions2,model);

				comment[AIR_CONFIG_MAX_COMMENT_LENGTH-1]='\0';
				strcpy(air1.comment,comment);
			}
		}

		/*Write out air file*/
		{
			AIR_Error errcode=AIR_write_air16(air_file,TRUE,es,zooming,&air1);
			
			if(errcode!=0){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("\nAttempt to save .air file %s failed.\n",air_file);
				free_function(pixel1,pixel2,pixel1e,pixel2e,es);
				return(errcode);
			}
		}
		free_function(pixel1,pixel2,pixel1e,pixel2e,es);
	}

	/*Write out termination files*/

	if(scaling_termination_file){
	
		/* Make sure that file hasn't been created or permissions changed while we worked */
		{
			AIR_Error errcode=AIR_fprobw(scaling_termination_file,overwrite_scaling_termination_file);
			
			if(errcode!=0){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("\nProblem with file %s\n", scaling_termination_file);
				return(errcode);
			}
		}
		{
			FILE *fp=fopen(scaling_termination_file,"w");
			
			if(!fp){
							
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("\nunable to create termination file '%s'\n",scaling_termination_file);
				{
					AIR_Error errcode=AIR_fprobw(scaling_termination_file,TRUE);
					if(errcode==0) errcode=AIR_UNSPECIFIED_FILE_WRITE_ERROR;
					return(errcode);
				}
			}
			if(fprintf(fp,"%e\n",tps[parameters])<1){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("Failed to write data into scale termination file '%s'\n",scaling_termination_file);
				if(fclose(fp)!=0){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("failed to close file %s\n",scaling_termination_file);
					return AIR_CANT_CLOSE_WRITE_ERROR;
				}
				return AIR_WRITE_SCALING_FILE_ERROR;
			}
			if(fclose(fp)!=0){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("failed to close file %s\n",scaling_termination_file);
				return AIR_CANT_CLOSE_WRITE_ERROR;
			}
		}
	}
		
	if(termination_file){
		/*Adjust termination of certain models for zooming as needed*/
		if(zooming){
			if(model==25){
					double pixel_size_s=air1.s.x_size;
					if(air1.s.y_size<pixel_size_s) pixel_size_s=air1.s.y_size;
					if(air1.s.z_size<pixel_size_s) pixel_size_s=air1.s.z_size;

					tps[0]/=(pixel_size_s/air1.s.x_size);
					tps[1]/=(pixel_size_s/air1.s.y_size);
					tps[3]/=(pixel_size_s/air1.s.x_size);
			}

			if(model==26||model==28){
				double pixel_size_s=air1.s.x_size;
				if(air1.s.y_size<pixel_size_s) pixel_size_s=air1.s.y_size;
				if(air1.s.z_size<pixel_size_s) pixel_size_s=air1.s.z_size;

				tps[0]/=(pixel_size_s/air1.s.x_size);
				tps[1]/=(pixel_size_s/air1.s.y_size);
				tps[3]/=(pixel_size_s/air1.s.x_size);
				tps[4]/=(pixel_size_s/air1.s.y_size);
				if(model==28){
					tps[6]/=(pixel_size_s/air1.s.x_size);
					tps[7]/=(pixel_size_s/air1.s.y_size);
				}
			}

			if(model==12||model==15){
				double pixel_size_s=air1.s.x_size;
				if(air1.s.y_size<pixel_size_s) pixel_size_s=air1.s.y_size;
				if(air1.s.z_size<pixel_size_s) pixel_size_s=air1.s.z_size;

				tps[0]/=(pixel_size_s/air1.s.x_size);
				tps[1]/=(pixel_size_s/air1.s.y_size);
				tps[2]/=(pixel_size_s/air1.s.z_size);
				tps[4]/=(pixel_size_s/air1.s.x_size);
				tps[5]/=(pixel_size_s/air1.s.y_size);
				tps[6]/=(pixel_size_s/air1.s.z_size);
				tps[8]/=(pixel_size_s/air1.s.x_size);
				tps[9]/=(pixel_size_s/air1.s.y_size);
				tps[10]/=(pixel_size_s/air1.s.z_size);
				if(model==15){
					tps[12]/=(pixel_size_s/air1.s.x_size);
					tps[13]/=(pixel_size_s/air1.s.y_size);
					tps[14]/=(pixel_size_s/air1.s.z_size);
				}
			}
		}
		
		/* Make sure that file hasn't been created or permissions changed while we worked */
		{
			AIR_Error errcode=AIR_fprobw(termination_file,overwrite_termination_file);
			
			if(errcode!=0){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("\nProblem with file %s\n", termination_file);
				return(errcode);
			}
		}
		{
			FILE *fp=fopen(termination_file,"w");
			
			if(!fp){
							
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("\nunable to create termination file '%s'\n",termination_file);
				{
					AIR_Error errcode=AIR_fprobw(termination_file,TRUE);
					if(errcode==0) return AIR_UNSPECIFIED_FILE_WRITE_ERROR;
					return(errcode);
				}
			}
			{
				unsigned int ii;
				
				for(ii=0;ii<parameters;ii++){
					if(fprintf(fp,"%e\n",tps[ii])<1){
						if(fclose(fp)!=0){
							printf("%s: %d: ",__FILE__,__LINE__);
							printf("failed to close file %s\n",termination_file);
							return AIR_CANT_CLOSE_WRITE_ERROR;							
						}
						printf("%s: %d: ",__FILE__,__LINE__);
						printf("Failed to write data into termination file '%s'\n",termination_file);
						return AIR_WRITE_INIT_FILE_ERROR;
					}
				}
			}
			if(fclose(fp)!=0){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("failed to close file %s\n",termination_file);
				return AIR_CANT_CLOSE_WRITE_ERROR;
			}
		}
	}		
	return 0;
}

