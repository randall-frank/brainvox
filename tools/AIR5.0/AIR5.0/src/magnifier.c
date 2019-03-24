/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/23/01 */

/* 
 * AIR_Pixels ***magnifier()
 *
 * This routine will magnify a file
 *  fourier interpolation is used
 *
 * returns:
 *	pointer to resliced data if successful
 *	NULL pointer if unsuccessful
 *									
 */

#include "AIR.h"

static void free_function(/*@null@*/ /*@out@*/ /*@only@*/ float *new, /*@null@*/ /*@out@*/ /*@only@*/ float *old, /*@null@*/ /*@out@*/ /*@only@*/ double *wa, /*@null@*/ /*@out@*/ /*@only@*/ double *ch, /*@null@*/ /*@out@*/ /*@only@*/ double *oldd, /*@null@*/ /*@out@*/ /*@only@*/ float ***temp_volume1)

{
	if(new) free(new);
	if(wa) free(wa);
	if(ch) free(ch);
	if(oldd) free(oldd);
	if(old) free(old);
	if(temp_volume1) AIR_free_3f(temp_volume1);
}


AIR_Pixels ***AIR_magnifier(AIR_Pixels ***volume, const struct AIR_Key_info *stats, struct AIR_Key_info *new_stats, const unsigned int bigx, const unsigned int bigy, const unsigned int bigz, const unsigned int smallx, const unsigned int smally, const unsigned int smallz, AIR_Error *errcode)

{
	float *new=NULL;
	float *old=NULL;
	double *wa=NULL;
	double *ch=NULL;
	double *oldd=NULL;
	float ***temp_volume1=NULL;
	
	AIR_Pixels 	***new_volume=NULL;
	
	const unsigned int x_dim=stats->x_dim;
	const unsigned int y_dim=stats->y_dim;
	const unsigned int z_dim=stats->z_dim;
	
	unsigned int x_dim1,y_dim1,z_dim1;

	if(bigx<smallx || bigy<smally || bigz<smallz){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("coding error when calling subroutine magnifier\n");
		*errcode=AIR_INVALID_MAGNIFICATION_ERROR;
		return NULL;
	}
	
	*new_stats=*stats;
	new_stats->x_size=smallx*new_stats->x_size/bigx;
	new_stats->y_size=smally*new_stats->y_size/bigy;
	new_stats->z_size=smallz*new_stats->z_size/bigz;

	if(x_dim>1){
		x_dim1=(bigx*x_dim)/smallx;
		new_stats->x_dim=(bigx*x_dim)/smallx;
	}
	else{
		x_dim1=x_dim;
		new_stats->x_dim=x_dim;
	}
	if(y_dim>1){
		y_dim1=(bigy*y_dim)/smally;
		new_stats->y_dim=(bigy*y_dim)/smally;
	}
	else{
		y_dim1=y_dim;
		new_stats->y_dim=y_dim;
	}
	if(z_dim>1){
		z_dim1=(bigz*z_dim)/smallz;
		new_stats->z_dim=(bigz*z_dim)/smallz;
	}
	else{
		z_dim1=z_dim;
		new_stats->z_dim=z_dim;
	}

	/*Allocate memory for return volume*/

	new_volume=AIR_create_vol3(x_dim1,y_dim1,z_dim1);
	if(!new_volume){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("unable to allocate memory to reslice file\n");
		*errcode=AIR_MEMORY_ALLOCATION_ERROR;
		return NULL;
	}

	/*Need to allocate enough float memory to accomodate new x and y sizes, old z size*/
	{
		unsigned int x_dim3=x_dim1;
		unsigned int y_dim3=y_dim1;
		unsigned int z_dim3=z_dim;

		temp_volume1=AIR_matrix3f(x_dim3,y_dim3,z_dim3);
		if(temp_volume1){

			/*Allocate memory to interpolate*/
			unsigned int dim1max=x_dim1;
			if(y_dim1>dim1max) dim1max=y_dim1;
			if(z_dim1>dim1max) dim1max=z_dim1;

			old=(float *)malloc(dim1max*sizeof(float));
			if(old){
				oldd=AIR_matrix1(dim1max);
				if(oldd){
					ch=AIR_matrix1(dim1max);
					if(ch){
						wa=AIR_matrix1(dim1max);
						if(wa){
							new=(float *)malloc(dim1max*sizeof(float));
						}
					}
				}
			}
		}
	}
	/* Verify success of allocations */

	if(!new){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("memory allocation failure\n");
		free_function(new,old,wa,ch,oldd,temp_volume1);
		AIR_free_vol3(new_volume);
		*errcode=AIR_MEMORY_ALLOCATION_ERROR;
		return NULL;
	}

	/*Prepass interpolate along x*/
	if(x_dim>1 && bigx!=smallx){
	
		unsigned int k;
		
		for(k=0;k<z_dim;k++){
		
			unsigned int j;
			
			for(j=0;j<y_dim;j++){
				{
					unsigned int i;
					
					for(i=0;i<x_dim;i++){
						oldd[i]=(double)volume[k][j][i];
					}
				}
			
				{
					unsigned int ifac[20];
					/*Initialize fourier transform arrays for this dimension*/
					AIR_rffti1(x_dim,wa,ifac);

					/*Compute forward transform*/
					AIR_rfftf1(x_dim,oldd,ch,wa,ifac);
				}

				if(2*(x_dim/2)==x_dim) oldd[x_dim-1]/=2;

				{
					unsigned int i;
					
					for(i=x_dim;i<(bigx*x_dim)/smallx;i++){
						oldd[i]=0.0;
					}
				}
				{
					unsigned int ifac[20];
					/*Initialize fourier transform arrays for new dimension*/
					AIR_rffti1((bigx*x_dim)/smallx,wa,ifac);

					/*Compute inverse transform*/
					AIR_rfftb1((bigx*x_dim)/smallx,oldd,ch,wa,ifac);
				}
			
				{
					unsigned int i;
					
					for(i=0;i<x_dim1;i++){
						temp_volume1[k][j][i]=oldd[i]/x_dim;
					}
				}
			}
		}
	}
	else{
	
		unsigned int k;
		
		for(k=0;k<z_dim;k++){
		
			unsigned int j;
			
			for(j=0;j<y_dim;j++){
			
				unsigned int i;
				
				for(i=0;i<x_dim;i++){
					temp_volume1[k][j][i]=(float)volume[k][j][i];
				}
			}
		}
	}



	/*Prepass interpolate along y*/
	if(y_dim>1 && bigy!=smally){
	
		unsigned int k;
		
		for(k=0;k<z_dim;k++){
		
			unsigned int i;
			
			for(i=0;i<x_dim1;i++){
				{
					unsigned int j;
					
					for(j=0;j<y_dim;j++){
						oldd[j]=temp_volume1[k][j][i];
					}
				}
			
				{
					unsigned int ifac[20];
					/*Initialize fourier transform arrays for this dimension*/
					AIR_rffti1(y_dim,wa,ifac);

					/*Compute forward transform*/
					AIR_rfftf1(y_dim,oldd,ch,wa,ifac);
				}
				if(2*(y_dim/2)==y_dim) oldd[y_dim-1]/=2;

				{
					unsigned int j;
					
					for(j=y_dim;j<(bigy*y_dim)/smally;j++){
						oldd[j]=0.0;
					}
				}
				{
					unsigned int ifac[20];
					/*Initialize fourier transform arrays for new dimension*/
					AIR_rffti1((bigy*y_dim)/smally,wa,ifac);

					/*Compute inverse transform*/
					AIR_rfftb1((bigy*y_dim)/smally,oldd,ch,wa,ifac);
				}

				{
					unsigned int j;
					
					for(j=0;j<y_dim1;j++){
						temp_volume1[k][j][i]=oldd[j]/y_dim;
					}
				}
			}
		}
	}


	/*Prepass interpolate along z */
	if(z_dim>1 && bigz!=smallz){
	
		unsigned int j;
		
		for(j=0;j<y_dim1;j++){
		
			unsigned int i;
			
			for(i=0;i<x_dim1;i++){
				{
					unsigned int k;
					
					for(k=0;k<z_dim;k++){
						oldd[k]=temp_volume1[k][j][i];
					}
				}
		
				{
					unsigned int ifac[20];
					/*Initialize fourier transform arrays for this dimension*/
					AIR_rffti1(z_dim,wa,ifac);

					/*Compute forward transform*/
					AIR_rfftf1(z_dim,oldd,ch,wa,ifac);
				}

				if(2*(z_dim/2)==z_dim) oldd[z_dim-1]/=2;

				{
					unsigned int k;
					
					for(k=z_dim;k<(bigz*z_dim)/smallz;k++){
						oldd[k]=0.0;
					}
				}
				{
					unsigned int ifac[20];
					
					/*Initialize fourier transform arrays for new dimension*/
					AIR_rffti1((bigz*z_dim)/smallz,wa,ifac);

					/*Compute inverse transform*/
					AIR_rfftb1((bigz*z_dim)/smallz,oldd,ch,wa,ifac);
				}
				{
					unsigned int k;
					
					for(k=0;k<z_dim1;k++){
						oldd[k]/=z_dim;
						oldd[k]+=.5;
						if(oldd[k]<0){
							new_volume[k][j][i]=0;
						}
						else if(oldd[k]>(double)AIR_CONFIG_MAX_POSS_VALUE){
							new_volume[k][j][i]=AIR_CONFIG_MAX_POSS_VALUE;
						}
						else{
							new_volume[k][j][i]=(AIR_Pixels)oldd[k];
						}
					}
				}
			}
		}
	}
	else{
		unsigned int k;
		
		for(k=0;k<z_dim1;k++){
		
			unsigned int j;
			
			for(j=0;j<y_dim1;j++){
			
				unsigned int i;
				
				for(i=0;i<x_dim1;i++){
				
					temp_volume1[k][j][i]+=.5;
					if(temp_volume1[k][j][i]<0){
						new_volume[k][j][i]=0;
					}
					else if(temp_volume1[k][j][i]>(float)AIR_CONFIG_MAX_POSS_VALUE){
						new_volume[k][j][i]=AIR_CONFIG_MAX_POSS_VALUE;
					}
					else{
						new_volume[k][j][i]=(AIR_Pixels)temp_volume1[k][j][i];
					}
				}
			}
		}
	}

	free_function(new,old,wa,ch,oldd,temp_volume1);
	*errcode=0;	
	return new_volume;
}

