/* Copyright 1995-2002 Roger P. Woods, M.D. */
/* Modified 1/16/02 */

/* 	This program will apply a nonlinear registration model to align two studies.
 *	It incorporates the essential elements of all prior linear registration
 *	programs (e.g., alignpettopet, alignmritopet, etc.)
 */


#include "AIR.h"

static void free_function(/*@null@*/ /*@out@*/ /*@only@*/ AIR_Pixels ***pixel1, /*@null@*/ /*@out@*/ /*@only@*/ AIR_Pixels ***pixel2, /*@null@*/ /*@out@*/ /*@only@*/ AIR_Pixels ***pixel1e, /*@null@*/ /*@out@*/ /*@only@*/ AIR_Pixels ***pixel2e, /*@null@*/ /*@out@*/ /*@only@*/ double *tps, /*@null@*/ /*@out@*/ /*@only@*/ double *tps_old, /*@null@*/ /*@out@*/ /*@only@*/ char *i_wrp_file, /*@null@*/ /*@out@*/ /*@only@*/ char *i_wrp_files, /*@null@*/ /*@out@*/ /*@only@*/ double **es)
{
	if(pixel1) AIR_free_vol3(pixel1);
	if(pixel2) AIR_free_vol3(pixel2);
	if(pixel1e) AIR_free_vol3(pixel1e);
	if(pixel2e) AIR_free_vol3(pixel2e);
	if(tps) free(tps);
	if(tps_old) free(tps_old);
	if(i_wrp_file) free(i_wrp_file);
	if(i_wrp_files) free(i_wrp_files);
	if(es) free(es);					/* es is set to point to tps */
}

AIR_Error AIR_do_align_warp(const char *program, const unsigned int imodel, const unsigned int fmodel, const char *warp_file, const AIR_Boolean overwrite, const char *standard_file, long int threshold1, const float kx1, const float ky1, const float kz1, const char *standard_file_mask, const char *reslice_file, long int threshold2, const float kx2, const float ky2, const float kz2, const char *reslice_file_mask, const char *initialization_file, const char *scaling_initialization_file, const char *scaling_termination_file, const AIR_Boolean overwrite_scaling_termination_file, const unsigned int samplefactor, const unsigned int samplefactor2, const unsigned int sffactor, const float precision, const unsigned int iterations, const unsigned int noprogtries, const AIR_Boolean interaction, const AIR_Boolean verbosity, const AIR_Boolean intermediate, const AIR_Boolean posdefreq)

{
	struct AIR_Warp
		air1,	/*AIR file to be output including file names, dimensions,reslice parameters*/
		air1e;	/*pseudo AIR file for edited images*/
	
	AIR_Pixels
		mapped_threshold1,
		mapped_threshold2;

	double	(*uvderivsN)(const unsigned int, double **, const unsigned int, AIR_Pixels ***, const struct AIR_Key_info *, const AIR_Pixels, AIR_Pixels ***, const struct AIR_Key_info *, const AIR_Pixels, double *, double **, double *, const double, unsigned int *);

	unsigned int coord;
	unsigned int coeffp;
	const unsigned int d3[14]={1,4,10,20,35,56,84,120,165,220,286,364,455,455};
	const unsigned int d2[14]={1,3,6,10,15,21,28,36,45,55,66,78,91,91};


	/* Warn of plan to overwrite existing .warp file */
	if(overwrite){
		FILE *fp=fopen(warp_file,"rb");
		
		if(fp){
			if(fclose(fp)!=0){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("failed to close file %s\n",warp_file);
				return AIR_CANT_CLOSE_READ_ERROR;
			}
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("\nWARNING: File '%s' will be overwritten by the output of this program\n",warp_file);
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
	if(intermediate && imodel==fmodel){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("User interface has requested intermediate warp files, but initial and final models are identical\n");
		return AIR_USER_INTERFACE_ERROR;
	}
	if(fmodel<1 ||fmodel>32||(fmodel>12 && fmodel<21)){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("User interface supplied an invalid final spatial transformation model (%u)\n",fmodel);
		return AIR_USER_INTERFACE_ERROR;
	}
	if(imodel<1 ||imodel>32||(imodel>12 && imodel<21)){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("User interface supplied an invalid initial spatial transformation model (%u)\n",imodel);
		return AIR_USER_INTERFACE_ERROR;
	}
	if((imodel<21 && fmodel>20) || (fmodel<21 && imodel>20)){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("User interface supplied initial spatial transformation model %u which is incompatible with the final spatial transformation model of %u\n",imodel,fmodel);
		return AIR_USER_INTERFACE_ERROR;
	}
	if(fmodel<imodel){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("User interface supplied initial spatial transformation model %u which is larger than the final spatial transformation model of %u\n",imodel,fmodel);
		return AIR_USER_INTERFACE_ERROR;
	}
	if(scaling_termination_file){
		if(strcmp(warp_file, scaling_termination_file)==0){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("User interface allowed output files with identical names\n");
			return AIR_USER_INTERFACE_ERROR;
		}
	}

	/* Bail out if the file names are too long to store in a .warp file */
	if(strlen(standard_file)>=(size_t)AIR_CONFIG_MAX_PATH_LENGTH){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("File name %s is too long to incorporate into a .warp file\n",standard_file);
		return AIR_PATH_TOO_LONG_ERROR;
	}
	if(strlen(reslice_file)>=(size_t)AIR_CONFIG_MAX_PATH_LENGTH){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("File name %s is too long to incorporate into a .warp file\n",reslice_file);
		return AIR_PATH_TOO_LONG_ERROR;
	}
	if(standard_file_mask){
		if(strlen(standard_file_mask)>=(size_t)AIR_CONFIG_MAX_PATH_LENGTH){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("File name %s is too long to incorporate into a .warp file\n",standard_file_mask);
			return AIR_PATH_TOO_LONG_ERROR;
		}
	}
	if(reslice_file_mask){
		if(strlen(reslice_file_mask)>=(size_t)AIR_CONFIG_MAX_PATH_LENGTH){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("File name %s is too long to incorporate into a .warp file\n",reslice_file_mask);
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
		AIR_Error errcode=AIR_fprobw(warp_file,overwrite);
		
		if(errcode!=0){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("problem with file %s\n",warp_file);
			return(errcode);
		}
	}
	if(intermediate){		
		char *i_wrp_file=malloc((strlen(warp_file)+4)*sizeof(char *));
		if(!i_wrp_file){
			printf("memory allocation failure\n");
			return AIR_MEMORY_ALLOCATION_ERROR;
		}
		{
			unsigned int model;
			
			for(model=imodel;model<=fmodel;model++){

				sprintf(i_wrp_file,"%s_%u",warp_file,model);
				{
					AIR_Error errcode=AIR_fprobw(i_wrp_file,TRUE);
					
					if(errcode!=0){
						printf("%s: %d: ",__FILE__,__LINE__);
						printf("problem with file %s\n",i_wrp_file);
						free(i_wrp_file);
						return(errcode);
					}
				}
			}
		}
		free(i_wrp_file);
		if(scaling_termination_file){
			char *i_wrp_files=malloc((strlen(scaling_termination_file)+4)*sizeof(char *));
			if(!i_wrp_files){
				printf("memory allocation failure\n");
				return AIR_MEMORY_ALLOCATION_ERROR;
			}
			{
				unsigned int model;
				
				for(model=imodel;model<=fmodel;model++){

					sprintf(i_wrp_files,"%s_%u",scaling_termination_file,model);
					{
						AIR_Error errcode=AIR_fprobw(i_wrp_files,TRUE);
						
						if(errcode!=0){
							printf("%s: %d: ",__FILE__,__LINE__);
							printf("problem with file %s\n",i_wrp_files);
							free(i_wrp_files);
							return(errcode);
						}
					}
				}
			}
			free(i_wrp_files);
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
	
	/* Select subroutine for computing derivatives */
	if(fmodel<21){
		if(interaction) uvderivsN=AIR_uvderiv3Dwarp;
		else uvderivsN=AIR_qvderiv3Dwarp;
	}
	else{
		if(interaction) uvderivsN=AIR_uvderiv2Dwarp;
		else uvderivsN=AIR_qvderiv2Dwarp;
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
	/* Compute the internal representation of the thresholds */
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
	{
		AIR_Pixels
			***pixel1=NULL,	/*pointer to original pixel values of standard file*/
			***pixel2=NULL,	/*pointer to original pixel values of file to reslice*/

			***pixel1e=NULL,	/* editing mask for standard file or NULL */
			***pixel2e=NULL;	/* editing mask for reslice file or NULL */
		
		double **es=NULL;
			
		double
			*tps=NULL,
			*tps_old=NULL;
		
		char
			*i_wrp_file=NULL,
			*i_wrp_files=NULL;
		
		unsigned int model=imodel;
		
		unsigned int extrazeros;
		
		if(model<21) extrazeros=d3[model+1]-d3[model];
		else extrazeros=d2[model+1]-d2[model];
		
		{
			unsigned int fcoeffp=0;
			
			if(fmodel<21){
				fcoeffp=d3[fmodel];
				coord=3;
			}
			else{
				fcoeffp=d2[fmodel-20];
				coord=2;
			}

			/* Allocate memory for parameters including scaling */
			tps=AIR_matrix1(coord*fcoeffp+1);
			if(!tps){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("unable to allocate memory for parameters\n");
				free_function(pixel1,pixel2,pixel1e,pixel2e,tps,tps_old,i_wrp_file,i_wrp_files,es);
				return AIR_MEMORY_ALLOCATION_ERROR;
			}
			tps_old=AIR_matrix1(coord*fcoeffp+1);
			if(!tps_old){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("unable to allocate memory for parameters copy\n");
				free_function(pixel1,pixel2,pixel1e,pixel2e,tps,tps_old,i_wrp_file,i_wrp_files,es);
				return AIR_MEMORY_ALLOCATION_ERROR;
			}
			{
				unsigned int i;
				
				for(i=0;i<coord*fcoeffp+1;i++) tps_old[i]=0.0;
			}
		}

		/*For 3D models, verify that both files don't have only a single plane*/
		if(coord>2){
			if(air1.s.z_dim==1 && air1.r.z_dim==1){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("Standard (%s) and reslice (%s) files both have only a single plane of data\n",air1.s_file,air1.r_file);
				printf("You must use a 2D model in this situation\n");
				free_function(pixel1,pixel2,pixel1e,pixel2e,tps,tps_old,i_wrp_file,i_wrp_files,es);
				return AIR_CANT_3D_SINGLE_PLANE_ERROR;
			}
		}
		else{
			if(air1.s.z_dim!=air1.r.z_dim){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("For 2D models, standard and reslice files must have the same number of planes\n");
				printf("File %s has %u planes; file %s has %u\n",air1.s_file,air1.s.z_dim,air1.r_file,air1.r.z_dim);
				free_function(pixel1,pixel2,pixel1e,pixel2e,tps,tps_old,i_wrp_file,i_wrp_files,es);
				return AIR_CANT_2D_UNMATCHED_ERROR;
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
					free_function(pixel1,pixel2,pixel1e,pixel2e,tps,tps_old,i_wrp_file,i_wrp_files,es);
					return(fps.errcode);
				}
				AIR_close_header(&fps);
				if(fps.errcode!=0){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("Failed to close file %s\n",air1e.s_file);
					free_function(pixel1,pixel2,pixel1e,pixel2e,tps,tps_old,i_wrp_file,i_wrp_files,es);
					return(fps.errcode);
				}
			}
			/* Verify that mask has same dimensions as data */
			{
				AIR_Error errcode=AIR_same_size(&air1.s, &air1e.s);
				
				if(errcode!=0){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("Dimension mismatch: %s: %u %u %u, %s: %u %u %u\n",air1.s_file,air1.s.x_dim,air1.s.y_dim,air1.s.z_dim,air1e.s_file,air1e.s.x_dim,air1e.s.y_dim,air1e.s.z_dim);
					free_function(pixel1,pixel2,pixel1e,pixel2e,tps,tps_old,i_wrp_file,i_wrp_files,es);
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
					free_function(pixel1,pixel2,pixel1e,pixel2e,tps,tps_old,i_wrp_file,i_wrp_files,es);
					return(fps.errcode);
				}
				AIR_close_header(&fps);
				if(fps.errcode!=0){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("Failed to close file %s\n",air1e.r_file);
					free_function(pixel1,pixel2,pixel1e,pixel2e,tps,tps_old,i_wrp_file,i_wrp_files,es);
					return(fps.errcode);
				}
			}
			/* Verify that mask has same dimensions as data */
			{
				AIR_Error errcode=AIR_same_size(&air1.r, &air1e.r);
				
				if(errcode!=0){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("Dimension mismatch: %s: %u %u %u, %s: %u %u %u\n",air1.r_file,air1.r.x_dim,air1.r.y_dim,air1.r.z_dim,air1e.r_file,air1e.r.x_dim,air1e.r.y_dim,air1e.r.z_dim);
					free_function(pixel1,pixel2,pixel1e,pixel2e,tps,tps_old,i_wrp_file,i_wrp_files,es);					
					return(errcode);
				}
			}
		}
	
		if(model<21) coeffp=d3[model];
		else coeffp=d2[model-20];
		
				
		if(scaling_initialization_file){
		
			FILE *fp=fopen(scaling_initialization_file,"rb");
			
			if (!fp){
				printf("%s: %d: ",__FILE__,__LINE__);				
				printf("unable to open scaling initialization file '%s'\n",scaling_initialization_file);
				free_function(pixel1,pixel2,pixel1e,pixel2e,tps,tps_old,i_wrp_file,i_wrp_files,es);
				{
					AIR_Error errcode=AIR_fprobr(scaling_initialization_file);
					if(errcode==0) errcode=AIR_UNSPECIFIED_FILE_READ_ERROR;
					return(errcode);
				}
			}
			if(fscanf(fp,"%le",&tps[coord*coeffp])==EOF){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("unable to read parameter from initialization file '%s'\n",scaling_initialization_file);
				free_function(pixel1,pixel2,pixel1e,pixel2e,tps,tps_old,i_wrp_file,i_wrp_files,es);
				(void)fclose(fp);
				return AIR_READ_SCALING_FILE_ERROR;
			}
			{
				double junk;
				
				if(fscanf(fp,"%le",&junk)!=EOF){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("initialization file '%s' contains more than the one expected parameter\n",scaling_initialization_file);
					free_function(pixel1,pixel2,pixel1e,pixel2e,tps,tps_old,i_wrp_file,i_wrp_files,es);
					(void)fclose(fp);
					return AIR_READ_SCALING_FILE_ERROR;
				}
			}
			if(fclose(fp)!=0){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("failed to close file %s\n",scaling_initialization_file);
				free_function(pixel1,pixel2,pixel1e,pixel2e,tps,tps_old,i_wrp_file,i_wrp_files,es);
				return AIR_CANT_CLOSE_READ_ERROR;
			}
		}
		else tps[coord*coeffp]=1.0;
		
		/* Zero tps spatial parameters */
		{
			unsigned int i;
			
			for(i=0;i<coord*coeffp;i++){
				tps[i]=0;
			}
		}

		if(initialization_file){
		
			struct AIR_Warp airini;
			unsigned int expectedcoeffs;
			
			if(model<21) expectedcoeffs=d3[model-1];
			else expectedcoeffs=d2[model-20-1];
			
			{
				double **inif;
				{
					AIR_Error errcode;
					
					inif=AIR_read_airw(initialization_file,&airini,&errcode);
					if(!inif){
						printf("%s: %d: ",__FILE__,__LINE__);
						printf("failed to read initialization file '%s'\n",initialization_file);
						free_function(pixel1,pixel2,pixel1e,pixel2e,tps,tps_old,i_wrp_file,i_wrp_files,es);
						return(errcode);
					}
				}
				/* Verify that number of coeffs is appropriate */
				if(airini.coeffp!=expectedcoeffs){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("unable to obtain expected number of parameters from initialization file '%s'\n",initialization_file);
					AIR_free_2(inif);
					free_function(pixel1,pixel2,pixel1e,pixel2e,tps,tps_old,i_wrp_file,i_wrp_files,es);
					return AIR_READ_INIT_FILE_ERROR;
				}
				/* Verify that number of coords is appropriate */
				if(coord != airini.coord){
					/* Allow .air file to initialize 2D warp if it describes pure 2D affine transform */
					if(!(airini.coord==3 && coord==2 && airini.coeffp==4 && inif[2][0]==0.0 && inif[2][1]==0.0 && inif[2][2]-1.0<AIR_CONFIG_PIX_SIZE_ERR && inif[2][3]==0.0)){
						printf("%s: %d: ",__FILE__,__LINE__);
						printf("unable to obtain expected number of parameters from initialization file '%s'\n",initialization_file);
						AIR_free_2(inif);
						free_function(pixel1,pixel2,pixel1e,pixel2e,tps,tps_old,i_wrp_file,i_wrp_files,es);
						return AIR_READ_INIT_FILE_ERROR;
					}
				}
				/* Copy initialization into fps */
				{
					unsigned int j;
					
					for(j=0;j<coord;j++){
					
						unsigned int i;
						
						for(i=0;i<expectedcoeffs;i++){
						
							tps[j*coeffp+i]=inif[j][i];
						}
					}
				}
				AIR_free_2(inif);
			}
		}
		else{
		
			tps[0]=(air1.r.x_dim-air1.s.x_dim*(air1.s.x_size/air1.r.x_size))/2;
			tps[1]=air1.s.x_size/air1.r.x_size;
			tps[coeffp]=(air1.r.y_dim-air1.s.y_dim*(air1.s.y_size/air1.r.y_size))/2;
			tps[coeffp+2]=air1.s.y_size/air1.r.y_size;
			if(coord>2){
				tps[2*coeffp]=(air1.r.z_dim-air1.s.z_dim*(air1.s.z_size/air1.r.z_size))/2;
				tps[2*coeffp+3]=air1.s.z_size/air1.r.z_size;
			}

			/* Advise against default initialization */
			if(model!=1 && model!=21){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("WARNING: It is recommended that you use initialization files with nonlinear transformations\n");
			}
		}
		if(intermediate){
			i_wrp_file=malloc((strlen(warp_file)+4)*sizeof(char *));
			if(!i_wrp_file){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("memory allocation failure\n");
				free_function(pixel1,pixel2,pixel1e,pixel2e,tps,tps_old,i_wrp_file,i_wrp_files,es);
				return AIR_MEMORY_ALLOCATION_ERROR;
			}
			if(scaling_termination_file){
				i_wrp_files=malloc((strlen(scaling_termination_file)+4)*sizeof(char*));
				if(!i_wrp_files){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("memory allocation failure\n");
					free_function(pixel1,pixel2,pixel1e,pixel2e,tps,tps_old,i_wrp_file,i_wrp_files,es);
					return AIR_MEMORY_ALLOCATION_ERROR;
				}
			}
		}

		/* Load the input files */
		{
			AIR_Error errcode; 
			pixel1=AIR_load(air1.s_file, &air1.s, 0, &errcode);
			if(!pixel1){
				free_function(pixel1,pixel2,pixel1e,pixel2e,tps,tps_old,i_wrp_file,i_wrp_files,es);
				return(errcode);
			}
		}
		air1.s_hash=AIR_hash(pixel1,&air1.s);

		{
			AIR_Error errcode;

			pixel2=AIR_load(air1.r_file, &air1.r, 0, &errcode);
			if(!pixel2){
				free_function(pixel1,pixel2,pixel1e,pixel2e,tps,tps_old,i_wrp_file,i_wrp_files,es);
				return(errcode);
			}
		}
		air1.r_hash=AIR_hash(pixel2,&air1.r);
		
		/* Load the masks */
		if(standard_file_mask){
			{
				AIR_Error errcode;

				pixel1e=AIR_load(air1e.s_file, &air1e.s , TRUE, &errcode);
				if(!pixel1e){
					free_function(pixel1,pixel2,pixel1e,pixel2e,tps,tps_old,i_wrp_file,i_wrp_files,es);
					return(errcode);
				}
			}
		}
		if(reslice_file_mask){
			{
				AIR_Error errcode;

				pixel2e=AIR_load(air1e.r_file, &air1e.r , TRUE, &errcode);
				if(!pixel2e){
					free_function(pixel1,pixel2,pixel1e,pixel2e,tps,tps_old,i_wrp_file,i_wrp_files,es);
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
				free_function(pixel1,pixel2,pixel1e,pixel2e,tps,tps_old,i_wrp_file,i_wrp_files,es);
				return(errcode);
			}
		}
		if(kx2!=0.0||ky2!=0.0||kz2!=0.0){
		
			AIR_Error errcode=AIR_gausssmaller(pixel2,&air1.r,kx2,ky2,kz2);
			
			if(errcode!=0){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("Failure in smoothing routine\n");
				free_function(pixel1,pixel2,pixel1e,pixel2e,tps,tps_old,i_wrp_file,i_wrp_files,es);
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
		
		/* Make record of old tps */
		
		{
			double
				*tpst=tps,
				*tps_oldt=tps_old;
			unsigned int i;
			
			for(i=0;i<=coord*coeffp;i++, tpst++, tps_oldt++){
				*tps_oldt=*tpst;
			}				
		}
		
		/*Perform alignment*/
		{
			AIR_Error errcode;
			
			es=AIR_alignwarp(tps,uvderivsN,pixel1,&air1.s,mapped_threshold1,pixel2,&air1.r,mapped_threshold2,samplefactor,samplefactor2,sffactor,precision,iterations,noprogtries,coord,coeffp,posdefreq,verbosity,&errcode);
			if(!es){
				free_function(pixel1,pixel2,pixel1e,pixel2e,tps,tps_old,i_wrp_file,i_wrp_files,es);
				return(errcode);
			}
		}
		
		/* More alignment if multiple models invoked */
		while(model<fmodel){
		
			/* Verify that parameters were altered with model just completed */
			
			{
				double
					*tpst=tps,
					*tps_oldt=tps_old;
				unsigned int i;
				
				for(i=0;i<=coord*coeffp;i++, tpst++, tps_oldt++){
					if(*tpst!=*tps_oldt) break;
				}
				
				if(i>coord*coeffp) break;
					
			}

			if(intermediate){
				/* Assemble comment for warp file */
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
						char comment[256];

						sprintf(comment,"%s t=(%li %li) sf=(%u %u %u) prec=%.4f its=%u %u model=%u %u %u",lprogram,threshold1,threshold2,samplefactor,samplefactor2,sffactor,precision,iterations,noprogtries,imodel,model,fmodel);

						comment[AIR_CONFIG_MAX_COMMENT_LENGTH-1]='\0';
						strcpy(air1.comment,comment);
					}
				}

				/* Sort out what order representation is really needed */
				{
					unsigned int coeff_max=0;
					unsigned int j;
					
					for(j=0;j<coord;j++){
					
						unsigned int i;
						
						for(i=0;i<coeffp;i++){
							if(es[j][i]!=0.0){
								if(i>coeff_max) coeff_max=i;
							}
						}
					}
					air1.coeffp=coeff_max+1;	/* Because of zero indexing */
				}
				air1.coord=coord;
				if(air1.coord==2){

					unsigned int i;
					
					for(i=12;i!=0;i--){
						if(air1.coeffp>d2[i-1]){
							air1.order=i;
							air1.coeffp=d2[i];
							break;
						}
						if(i==1){
							air1.order=0;
							air1.coeffp=1;
						}
					}
				}
				else{

					unsigned int i;
					
					for(i=12;i!=0;i--){
						if(air1.coeffp>d3[i-1]){
							air1.order=i;
							air1.coeffp=d3[i];
							break;
						}
						if(i==1){
							air1.order=0;
							air1.coeffp=1;
						}
					}
				}
				{					
					sprintf(i_wrp_file,"%s_%u",warp_file,model);
					/*Write out air file*/
					if(AIR_write_airw(i_wrp_file,TRUE,es,&air1)!=0){
						printf("%s: %d: ",__FILE__,__LINE__);
						printf("WARNING: Failed to save intermediate .warp file %s\n",i_wrp_file);
					}
				}
				
				/* Overwrite the most recent version of the scaling termination file*/
				if(scaling_termination_file){
				
					sprintf(i_wrp_files,"%s_%u",scaling_termination_file,model);
					{
						FILE *fp=fopen(i_wrp_files,"w");
						
						if(!fp){

							printf("%s: %d: ",__FILE__,__LINE__);
							printf("\nunable to create intermediate scaling termination file '%s'\n",i_wrp_files);
							{
								AIR_Error errcode=AIR_fprobw(i_wrp_files,TRUE);
								free_function(pixel1,pixel2,pixel1e,pixel2e,tps,tps_old,i_wrp_file,i_wrp_files,es);
								if(errcode==0) errcode=AIR_UNSPECIFIED_FILE_WRITE_ERROR;
								return(errcode);
							}
						}
						if(fprintf(fp,"%e\n",tps[coord*coeffp])<1){
							if(fclose(fp)!=0){
								printf("%s: %d: ",__FILE__,__LINE__);
								printf("failed to close file %s\n",i_wrp_files);
								free_function(pixel1,pixel2,pixel1e,pixel2e,tps,tps_old,i_wrp_file,i_wrp_files,es);
								return AIR_CANT_CLOSE_WRITE_ERROR;
							}
							printf("%s: %d: ",__FILE__,__LINE__);
							printf("Failed to write data into intermediate scaling termination file '%s'\n",i_wrp_files);
							free_function(pixel1,pixel2,pixel1e,pixel2e,tps,tps_old,i_wrp_file,i_wrp_files,es);
							return AIR_WRITE_SCALING_FILE_ERROR;
						}
						if(fclose(fp)!=0){
							printf("%s: %d: ",__FILE__,__LINE__);
							printf("failed to close file %s\n",i_wrp_files);
							free_function(pixel1,pixel2,pixel1e,pixel2e,tps,tps_old,i_wrp_file,i_wrp_files,es);
							return AIR_CANT_CLOSE_WRITE_ERROR;
						}
					}
				}
			}

			/* Copy parameters into tps_old, reformatting for next model along the way*/
			{
				double
					*tpst=tps,
					*tps_oldt=tps_old;
				{
					unsigned int j;
					
					for(j=0;j<coord;j++){
						{
							unsigned int i;
						
							for(i=0;i<coeffp;i++, tpst++, tps_oldt++){
								*tps_oldt=*tpst;
							}
						}
						{
							unsigned int i;
							
							for(i=0;i<extrazeros;i++, tps_oldt++){
								*tps_oldt=0;
							}
						}
					}
				}
				*tps_oldt=*tpst;
			}


			/* Increment model and reset model dependent values */
			model++;

			if(model<21){
				coeffp=d3[model];
				extrazeros=d3[model+1]-d3[model];
#if(AIR_CONFIG_VERBOSITY!=0)
				printf("\t%i parameter 3D model selected\n",coord*coeffp);
#endif
			}
			else{
				coeffp=d2[model-20];
				extrazeros=d2[model-20+1]-d2[model-20];
#if(AIR_CONFIG_VERBOSITY!=0)
				printf("\t%i parameter 2D model selected\n",coord*coeffp);
#endif
			}


			/* Copy reformatted parameters back into i */
			{
				double
					*tpst=tps,
					*tps_oldt=tps_old;
				unsigned int i;
				
				for(i=0;i<=coord*coeffp;i++, tpst++, tps_oldt++){
					*tpst=*tps_oldt;
				}				
			}

			/* Free memory allocated by prior model */
			if(es){
				free(es);	/* es points to tps */
				es=NULL;
			}
			
			{
				AIR_Error errcode;
				
				es=AIR_alignwarp(tps,uvderivsN,pixel1,&air1.s,mapped_threshold1,pixel2,&air1.r,mapped_threshold2,samplefactor,samplefactor2,sffactor,precision,iterations,noprogtries,coord,coeffp,posdefreq,verbosity,&errcode);

				if(!es){
					free_function(pixel1,pixel2,pixel1e,pixel2e,tps,tps_old,i_wrp_file,i_wrp_files,es);
					return(errcode);
				}
			}
		}
	
#if(AIR_CONFIG_VERBOSITY!=0)
		if(model<fmodel) printf("Last model applied was %i because of failure to improve cost function with that model\n",model);
#endif


		/* Assemble comment for warp file */
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
				char comment[256];
				
				sprintf(comment,"%s t=(%li %li) sf=(%u %u %u) prec=%.4f its=%u %u model=%u %u %u",lprogram,threshold1,threshold2,samplefactor,samplefactor2,sffactor,precision,iterations,noprogtries,imodel,model,fmodel);

				comment[AIR_CONFIG_MAX_COMMENT_LENGTH-1]='\0';
				strcpy(air1.comment,comment);
			}
		}

		/* Sort out what order representation is really needed */
		{
			unsigned int coeff_max=0;
			{
				unsigned int j;
				
				for(j=0;j<coord;j++){
				
					unsigned int i;
					
					for(i=0;i<coeffp;i++){
					
						if(es[j][i]!=0.0){
							if(i>coeff_max) coeff_max=i;
						}
					}
				}
			}
			air1.coeffp=coeff_max+1;	/* Because of zero indexing */
		}

		air1.coord=coord;
		if(air1.coord==2){
			
			unsigned int i;
			
			for(i=12;i!=0;i--){
				if(air1.coeffp>d2[i-1]){
					air1.order=i;
					air1.coeffp=d2[i];
					break;
				}
				if(i==1){
					air1.order=0;
					air1.coeffp=1;
				}
			}
		}
		else{
		
			unsigned int i;
			
			for(i=12;i!=0;i--){
				if(air1.coeffp>d3[i-1]){
					air1.order=i;
					air1.coeffp=d3[i];
					break;
				}
				if(i==1){
					air1.order=0;
					air1.coeffp=1;
				}
			}
		}

		/*Write out warp file*/
		{
			AIR_Error errcode=AIR_write_airw(warp_file,TRUE,es,&air1);
			
			if(errcode!=0){
				free_function(pixel1,pixel2,pixel1e,pixel2e,tps,tps_old,i_wrp_file,i_wrp_files,es);
				return(errcode);
			}
		}


		/*Write out termination file*/

		if(scaling_termination_file){
		
			FILE *fp=fopen(scaling_termination_file,"w");
			
			if(!fp){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("\nunable to create termination file '%s'\n",scaling_termination_file);
				free_function(pixel1,pixel2,pixel1e,pixel2e,tps,tps_old,i_wrp_file,i_wrp_files,es);
				{
					AIR_Error errcode=AIR_fprobw(scaling_termination_file,TRUE);
					if(errcode==0) errcode=AIR_UNSPECIFIED_FILE_WRITE_ERROR;
					return(errcode);
				}
			}
			if(fprintf(fp,"%e\n",tps[coord*coeffp])<1){
				free_function(pixel1,pixel2,pixel1e,pixel2e,tps,tps_old,i_wrp_file,i_wrp_files,es);
				if(fclose(fp)!=0){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("failed to close file %s\n",scaling_termination_file);
					return AIR_CANT_CLOSE_WRITE_ERROR;
				}
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("Failed to write data into scale termination file '%s'\n",scaling_termination_file);
				return AIR_WRITE_SCALING_FILE_ERROR;
			}
			if(fclose(fp)!=0){
				free_function(pixel1,pixel2,pixel1e,pixel2e,tps,tps_old,i_wrp_file,i_wrp_files,es);
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("failed to close file %s\n",scaling_termination_file);
				return AIR_CANT_CLOSE_WRITE_ERROR;
			}
		}
		free_function(pixel1,pixel2,pixel1e,pixel2e,tps,tps_old,i_wrp_file,i_wrp_files,es);
	}
	return 0;
}

