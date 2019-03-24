/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 5/23/01 */

/*
 * AIR_Pixels ***meaner()
 *
 * Routine orchestrates computation of a mean image by the
 *  subroutine meancalc().
 *
 * Multiplicative weighting factors are read from files with
 *  the suffix "scale_fac_suff" unless "scale_fac_suff" is a null
 *  character string, in which case all files are weighted
 *  equally. A NULL pointer is returned if weighting factors
 *  cannot be read.
 *
 * The files to be averaged are checked for matrix dimension
 *  compatibility, but not for voxel size compatibility. The
 *  function returns NULL in the event of matrix dimension
 *  incompatibility. The voxel dimensions and matrix dimensions
 *  and bits/pixel of the first input file is assigned to the
 *  returned image (via struct AIR_Key_info *mean_stats).
 *
 * A NULL pointer is returned if the bits/pixel is not
 *  consistent with the declaration of AIR_Pixels in the header
 *  file.
 *
 * If rescale!=0, a second pass is made through all of the data
 *  to recalculate a rescaled image such that the maximum
 *  mean pixel value anywhere in the returned file is exactly
 *  equal to the MAX_POSS_VALUE declared in the AIR.h header.
 *
 * Returns:
 *	Pointer to mean image if successful
 *	NULL pointer if unsuccessful
 *  *errcode will contain error code
 *
 * A value equivalent to that read from the suffix file is
 *  returned at *scalingmax
 */

#include "AIR.h"

static void free_function(/*@null@*/ /*@out@*/ /*@only@*/ AIR_Pixels ***data, /*@null@*/ /*@out@*/ /*@only@*/ double *scaling)
{
	if(data) AIR_free_vol3(data);
	if(scaling) free(scaling);
}


AIR_Pixels ***AIR_meaner(struct AIR_Key_info *mean_stats, double *scalingmax, const signed int strictness, const AIR_Boolean weighting, const char *scale_fac_suff, const AIR_Boolean rescale, char **inputs, const unsigned int quantity, AIR_Error *errcode)
/*strictness >0 requires nonzero pixel in all input files*/
/*strictness ==0 averages only non-zero values*/
/*strictness <0 results in true average including zeros*/
/*Real world value of brightest output pixel is returned in scalingmax */
/*weighting ==0 results in equal weighting of all files*/

/* scale_fac_suff=='\0' causes normalization factor of 1 */
/*rescale==0 yields no rescaling of image brightness*/
/* inputs is input file names*/
/* quantity is total number of input files*/

{
	AIR_Pixels		***data=NULL;
	double			*scaling=NULL;	/*multiplicative factors relating pixel values to real world*/

	AIR_Pixels 		***mean=NULL;

	unsigned int			bits;
	unsigned int			x_dim,y_dim,z_dim;



	scaling=(double *)malloc(quantity*sizeof(double));
	if(!scaling){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("Memory allocation error in subroutine meaner()\n");
		*errcode=AIR_MEMORY_ALLOCATION_ERROR;
		return NULL;
	}

	*scalingmax=0.0;

	{
		unsigned int n;
		
		for(n=0;n<quantity;n++){
			if(!scale_fac_suff){
				scaling[n]=1.0;
			}
			else{
				scaling[n]=AIR_read_value(inputs[n],scale_fac_suff,errcode);
				if(scaling[n]<=0.0){
					free_function(data,scaling);
					return NULL;
				}
			}
			if(scaling[n]>*scalingmax) *scalingmax=scaling[n];
		}
	}
	{
		unsigned int n;
		
		for(n=0;n<quantity;n++){
			scaling[n]/=*scalingmax;
		}
	}

	/*Find out size of files*/
	{
		struct AIR_Fptrs fps;
		int dummy[8];

		(void)AIR_open_header(inputs[0],&fps,mean_stats,dummy);
		if(fps.errcode!=0){
			*errcode=fps.errcode;
			AIR_close_header(&fps);
			free_function(data,scaling);
			return NULL;
		}
		AIR_close_header(&fps);
		if(fps.errcode!=0){
			*errcode=fps.errcode;
			free_function(data,scaling);
			return NULL;
		}

		bits=mean_stats->bits;

		x_dim=mean_stats->x_dim;
		y_dim=mean_stats->y_dim;
		z_dim=mean_stats->z_dim;
	}


	/*Make sure all of the files are compatible*/
	{
		unsigned int i;
		
		for (i=0;i<quantity;i++){
			struct AIR_Key_info temp_info;
			struct AIR_Fptrs fps;
			int dummy[8];

			float globalscale=AIR_open_header(inputs[i],&fps,&temp_info,dummy);
			if(fps.errcode!=0){
				*errcode=fps.errcode;
				AIR_close_header(&fps);
				free_function(data,scaling);
				return NULL;
			}
			AIR_close_header(&fps);
			if(fps.errcode!=0){
				*errcode=fps.errcode;
				free_function(data,scaling);
				return NULL;
			}
			*errcode=AIR_same_dim(mean_stats,&temp_info);
			if(*errcode!=0){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("file %s is not compatible with file %s\n",inputs[i],inputs[0]);
				printf("%u x_dim, %u y_dim ,%u z_dim %s\n",temp_info.x_dim,temp_info.y_dim,temp_info.z_dim,inputs[i]);
				printf("%u x_dim, %u y_dim ,%u z_dim %s\n",x_dim,y_dim,z_dim,inputs[0]);
				free_function(data,scaling);
				return NULL;
			}
			if(bits!=temp_info.bits){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("WARNING: Files being averaged have different numbers of bits/pixel\n");
				printf("values will be bit shifted\n");
				printf("%u bits %s\n",temp_info.bits,inputs[i]);
				printf("%u bits %s\n",bits,inputs[0]);
			}
			if(globalscale!=1.0){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("WARNING: File %s rescaled during loading by multiplicative factor of %e\n",inputs[i],globalscale);
			}
		}
	}
	
	/*Set the bits/pixel*/
	mean_stats->bits=8*sizeof(AIR_Pixels);

	/*Allocate memory for mean image output*/
	mean=AIR_create_vol3(x_dim,y_dim,z_dim);
	if(mean){
		/* Allocate memory for a single plane from each of the studies */
		data=AIR_create_vol3(x_dim,y_dim,quantity);
	}
	if(!data){
		if(mean) AIR_free_vol3(mean);
		free_function(data,scaling);
		*errcode=AIR_MEMORY_ALLOCATION_ERROR;
		return NULL;
	}

	/*max is tracked by meancalc across planes to find the overall maximum*/
	{
		double max=0.0;

		/*For every plane*/
		{
			unsigned int j;
			
			for(j=0;j<z_dim;j++){
				/*Read the data for each study*/
				{
					unsigned int i;
					
					for(i=0;i<quantity;i++){
						*errcode=AIR_quickread(data[i][0],inputs[i],j);
						if(*errcode!=0){
							free_function(data,scaling);
							AIR_free_vol3(mean);
							return NULL;
						}
					}
				}
				/*Analyze this plane*/

				*errcode=AIR_meancalc(mean[j],data,scaling,quantity,strictness,weighting,x_dim,y_dim,(double) 1,&max);
				if(*errcode!=0){
					free_function(data,scaling);
					AIR_free_vol3(mean);
					return NULL;
				}
			}
		}
		{
			double brightness=1.0;

			if(rescale){
				brightness=(double)AIR_CONFIG_MAX_POSS_VALUE/max;
				*scalingmax/=brightness;

				/*For every plane*/
				{
					unsigned int j;
					
					for(j=0;j<z_dim;j++){
						/*Read the data for each study*/
						{
							unsigned int i;
							
							for(i=0;i<quantity;i++){
								*errcode=AIR_quickread(data[i][0],inputs[i],j);
								if(*errcode!=0){
									free_function(data,scaling);
									AIR_free_vol3(mean);
									return NULL;
								}
							}
						}
						/*Analyze this plane*/

						*errcode=AIR_meancalc(mean[j],data,scaling,quantity,strictness,weighting,x_dim,y_dim,brightness,&max);
						if(*errcode!=0){
							free_function(data,scaling);
							AIR_free_vol3(mean);
							return NULL;
						}
					}
				}
			}
		}

		if(max>AIR_CONFIG_MAX_POSS_VALUE+.5){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("WARNING: Values as high as %u were encountered but were set to %u\n",(unsigned int)max,AIR_CONFIG_MAX_POSS_VALUE);
		}
	}
	free_function(data,scaling);	
	return mean;

}
	
		
