#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "../AIR5.0/AIR.h"

#include "air_interface.h"

/* ------------------- Start of AIR 5 interface code -------------- */

#define COORDS 4	/*Number of coordinates referenced in AIR file*/
#define COEFFS 4	/*Number of coefficients referenced in AIR file*/
#define MAX_PARAMS 16	/*Maximum number of parameters in any implemented model including rescaling*/

static void free_function(AIR_Pixels ***pixel1, AIR_Pixels ***pixel2, AIR_Pixels ***pixel1e, AIR_Pixels ***pixel2e, double **es)
{
	if(pixel1) AIR_free_vol3(pixel1);
	if(pixel2) AIR_free_vol3(pixel2);
	if(pixel1e) AIR_free_vol3(pixel1e);
	if(pixel2e) AIR_free_vol3(pixel2e);
	if(es) AIR_free_2(es);
}

static void free_function2(AIR_Pixels ***datain, AIR_Pixels ***dataout)
{
	if(datain) AIR_free_vol3(datain);
	if(dataout) AIR_free_vol3(dataout);
}

void print_params(AlignParams *p)
{
	printf("verb=%d\n",p->verbosity);
	printf("model=%d\n",p->model);
	printf("costfxn=%d\n",p->costfxn);
	printf("Samplefactor=%d\n",p->samplefactor);
	printf("SAM2=%d\n",p->samplefactor2);
	printf("SAM3=%d\n",p->sffactor);
	printf("P1=%d\n",p->partitions1);
	printf("P2=%d\n",p->partitions2);
	printf("perc=%f\n",p->precision);
	printf("interp=%d\n",p->interp);
	printf("T1=%d\n",p->threshold1);
	printf("T2=%d\n",p->threshold2);
}

void align_param_init(AlignParams *p)
{
	p->verbosity = 0;
	p->model = 23;
	p->costfxn = 2;
	p->precision = 0.00001;
	p->samplefactor = 81;
	p->samplefactor2 = 1;
	p->sffactor = 3;
	p->partitions1 = 1; 
	p->partitions2 = 1;
	p->interp = 1;
	p->threshold1 = 7000;
	p->threshold2 = 7000;
}

/* ---- original AIR5 code from AIR_do_alignlinear begins here ---- */

int align_images(char *fixed,char *in,char *out,int *dims,
		AlignParams *params, double *fit,double *fitparams, int fitonly)
{
	int iterations = 25;
	int noprogtries = 5;
	int posdefreq = 1;
	int zooming = 0;
	float kx1 = 0.0, ky1 = 0.0, kz1 = 0.0;
	float kx2 = 0.0, ky2 = 0.0, kz2 = 0.0;
	int threshold1 = params->threshold1;
	int threshold2 = params->threshold2;
	int dynamic1 = 1, dynamic2 = 1;
	int interaction = 1;

	int 	i,j,k;
	double	saved[4][4];

	/* pull out the parameters */
	int verbosity = params->verbosity;
	int model = params->model;
	int costfxn = params->costfxn;
	float precision = params->precision;
	int samplefactor = params->samplefactor;
	int samplefactor2 = params->samplefactor2;
	int sffactor = params->sffactor;
	int partitions1 = params->partitions1;
	int partitions2 = params->partitions2;
	int interp = params->interp;

/* ---- original AIR5 code from AIR_do_alignlinear begins here ---- */
/* Copyright 1995-2000 Roger P. Woods, M.D. */

	struct AIR_Air16	air1;	/*AIR file to be output including file names, dimensions,reslice parameters*/
	int parameters;		/*number of formal parameters in specified model*/
	int param_int;		/*number of parameters in derivative routine uvfN*/
	int scales=(costfxn==3);		/* number of rescaling parameters */
	int memory=(costfxn==1);		/* Tells align routine whether to allocate extra memory needed for RIU cost function */

	void	(*uvfN)(const double *, double **, double ***, double ****, const struct AIR_Key_info *, const struct AIR_Key_info *, const AIR_Boolean);

	double	(*uvderivsN)(const unsigned int, double **, double ***, double ****, const unsigned int, 
			     AIR_Pixels ***, const struct AIR_Key_info *, AIR_Pixels ***,
			     const struct AIR_Key_info *, const AIR_Pixels, double *, double **,
			     unsigned int *, double *, double *, double **, double **, double ***,
			     double ***, const unsigned int, AIR_Pixels, double, const AIR_Boolean,
			     unsigned int *)=NULL;

	double tps[MAX_PARAMS];

	/* a lot of unnecessary (for this use) error checking removed */

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
	/* we just fill in the arrays */
	{
		air1.s.bits = 16;
		air1.s.x_dim = dims[0];
		air1.s.y_dim = dims[1];
		air1.s.z_dim = dims[2];
		air1.s.x_size = 1.0;
		air1.s.y_size = 1.0;
		air1.s.z_size = 1.0;
		strcpy(air1.s_file,"Standard");
	}
	{
		air1.r.bits = 16;
		air1.r.x_dim = dims[0];
		air1.r.y_dim = dims[1];
		air1.r.z_dim = dims[2];
		air1.r.x_size = 1.0;
		air1.r.y_size = 1.0;
		air1.r.z_size = 1.0;
		strcpy(air1.r_file,"Reslice");
	}

	/* Read the intensity parameter initialization */
	tps[parameters]=1.0;
	
	/* Read the spatial parameter initializations */
	{

		switch(model){
		
			case 9:
				tps[7]=1;
				tps[8]=1;
				/* note deliberate omission of break */
			case 7:
				tps[6]=1;
				/* note deliberate omission of break */
			case 6:
				tps[0]=tps[1]=tps[2]=tps[3]=tps[4]=tps[5]=0;
				break;
				
			case 15:
				tps[12]=0;
				tps[13]=0;
				tps[14]=0;
				/* note deliberate omission of break */
			case 12:
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
				/* note deliberate omission of break */
			case 23:
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
				/* note deliberate omission of break */
			case 26:
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
			printf("Standard file (%s) has %i planes and reslice file (%s) has %i planes\n",air1.s_file,air1.s.z_dim,air1.r_file,air1.r.z_dim);
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

	/* Load and process the image data */
	{
		AIR_Pixels ***pixel1=NULL;	/*pointer to original pixel values of standard file*/
		AIR_Pixels ***pixel2=NULL;	/*pointer to original pixel values of file to reslice*/

		AIR_Pixels ***pixel1e=NULL;	/* editing mask for standard file or NULL */
		AIR_Pixels ***pixel2e=NULL;	/* editing mask for reslice file or NULL */
		
		double **es=NULL;

	
		/* Load the input files */
		pixel1=AIR_create_vol3(dims[0],dims[1],dims[2]);
		memcpy(pixel1[0][0],fixed,dims[0]*dims[1]*dims[2]*2);

		pixel2=AIR_create_vol3(dims[0],dims[1],dims[2]);
		memcpy(pixel2[0][0],in,dims[0]*dims[1]*dims[2]*2);

		air1.s_hash=1; /* AIR_hash(pixel1,&air1.s); */
		air1.r_hash=2; /*AIR_hash(pixel2,&air1.r); */
		
		/* Smooth if requested--must be done before any masking is applied */
		if(kx1!=0.0||ky1!=0.0||kz1!=0.0){
		
			int errcode=AIR_gausssmaller(pixel1,&air1.s,kx1,ky1,kz1);
			
			if(errcode){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("Failure in smoothing routine\n");
				free_function(pixel1,pixel2,pixel1e,pixel2e,es);
				return(errcode);
			}
		}
		if(kx2!=0.0||ky2!=0.0||kz2!=0.0){
		
			int errcode=AIR_gausssmaller(pixel2,&air1.r,kx2,ky2,kz2);
			
			if(errcode){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("Failure in smoothing routine\n");
				free_function(pixel1,pixel2,pixel1e,pixel2e,es);
				return(errcode);
			}
		}

		/*Perform alignment*/
		{
			int errcode;
			
			es=AIR_align(parameters,tps,uvfN,param_int,uvderivsN,pixel1,pixel1e,
			             &air1.s,(int)threshold1,pixel2,pixel2e,&air1.r,(int)threshold2,
				     samplefactor,samplefactor2,sffactor,precision,iterations,
			             noprogtries,zooming,partitions1,partitions2,dynamic1,dynamic2,
				     scales,COORDS,COEFFS,memory,posdefreq,verbosity,&errcode);

			if(!es){
				*fit = -1;
				free_function(pixel1,pixel2,pixel1e,pixel2e,es);
				return(errcode);
			}
			*fit = AIR_align_last_costfunction();
		}

		for(i=0;i<4;i++) for(j=0;j<4;j++)
			saved[i][j] = es[i][j];
		free_function(pixel1,pixel2,pixel1e,pixel2e,es);
	}
	if (fitonly) {
		k = 0;
		for(i=0;i<4;i++) for(j=0;j<4;j++) {
			if (fitparams) fitparams[k++] = saved[i][j];
		}
		return 1;
	}

/* ---- original AIR5 code from do_reslice begins here ---- */
/* Copyright 1995-2000 Roger P. Woods, M.D. */

	/* removed unnecessary error checks */

	{
		struct AIR_Air16	air1;
		double			*e[4]; 
		struct AIR_Key_info	dimensions3;
		
		AIR_Pixels		***(*reslicerN)(AIR_Pixels ***, const struct AIR_Key_info *, struct AIR_Key_info *, double **, const double, AIR_Error *)=NULL;

		float scale = 1.0;

		e[0]=air1.e[0];
		e[1]=air1.e[1];
		e[2]=air1.e[2];
		e[3]=air1.e[3];

		k = 0;
		for(i=0;i<4;i++) for(j=0;j<4;j++) {
			e[i][j] = saved[i][j];
			if (fitparams) fitparams[k++] = e[i][j];
		}


		/* Validate interpolation model */
		{
			int affine=(air1.e[0][3]==0 && air1.e[1][3]==0 && air1.e[2][3]==0 && air1.e[3][3]==1);
			
			dimensions3.bits=0;	/*Disable binary file loading by default*/
			
			switch(interp){
				case 0:
					dimensions3.bits=1;
					reslicerN=AIR_r_persp_nn;
					break;
				case 1:
					if(affine) reslicerN=AIR_r_affine_lin;
					else reslicerN=AIR_r_persp_lin;
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
			AIR_Pixels 		***datain=NULL;
			AIR_Pixels 		***dataout=NULL;
	
			datain=AIR_create_vol3(dims[0],dims[1],dims[2]);
			memcpy(datain[0][0],in,dims[0]*dims[1]*dims[2]*2);

			air1.r.bits = 16;
			air1.r.x_dim = dims[0];
			air1.r.y_dim = dims[1];
			air1.r.z_dim = dims[2];
			air1.r.x_size = 1.0;
			air1.r.y_size = 1.0;
			air1.r.z_size = 1.0;

			air1.s.bits = 16;
			air1.s.x_dim = dims[0];
			air1.s.y_dim = dims[1];
			air1.s.z_dim = dims[2];
			air1.s.x_size = 1.0;
			air1.s.y_size = 1.0;
			air1.s.z_size = 1.0;

			{	
				double pixel_size_s=air1.s.x_size;
				if(air1.s.y_size<pixel_size_s) pixel_size_s=air1.s.y_size;
				if(air1.s.z_size<pixel_size_s) pixel_size_s=air1.s.z_size;

				/*Sort out how to interpolate*/
				{
					if(fabs(air1.s.x_size-pixel_size_s)>AIR_CONFIG_PIX_SIZE_ERR){
						float xoom1=air1.s.x_size/pixel_size_s;
						air1.s.x_dim=(air1.s.x_dim-1)*xoom1+1;
						air1.s.x_size=pixel_size_s;
					}
					if(fabs(air1.s.y_size-pixel_size_s)>AIR_CONFIG_PIX_SIZE_ERR){
						float yoom1=air1.s.y_size/pixel_size_s;
						air1.s.y_dim=(air1.s.y_dim-1)*yoom1+1;
						air1.s.y_size=pixel_size_s;
					}
					if(fabs(air1.s.z_size-pixel_size_s)>AIR_CONFIG_PIX_SIZE_ERR){
						float zoom1=air1.s.z_size/pixel_size_s;
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
			}


			/*Reslice the data*/
			{
				int errcode;
				
				if(reslicerN) dataout=(reslicerN)(datain,&air1.r,&air1.s,e,scale,&errcode);
				if(!dataout){
					free_function2(datain,dataout);
					return(errcode);
				}
			}

			/*Save the data*/
			memcpy(out,dataout[0][0],dims[0]*dims[1]*dims[2]*2);
			free_function2(datain,dataout);
		}
	}
	return 0;
}

void get_xyz(long int *xyz,char *str)
{
        char    t[256];
        char    *p;

        strcpy(t,str);

        p = t;
        sscanf(p,"%ld",&(xyz[0]));

        p = strchr(p,':');
        if (p) {
                p++;
                if (*p == '\0') return;
        } else {
                return;
        }
        sscanf(p,"%ld",&(xyz[1]));

        p = strchr(p,':');
        if (p) {
                p++;
                if (*p == '\0') return;
        } else {
                return;
        }
        sscanf(p,"%ld",&(xyz[2]));

        return;
}

