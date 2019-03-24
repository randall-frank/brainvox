/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 5/23/01 */

/*
 * Creates an image of the positive determinant regions of a .warp file
 */

#include "AIR.h"


static void free_function(/*@null@*/ /*@out@*/ /*@only@*/ double **e, /*@null@*/ /*@out@*/ /*@only@*/ double **f, /*@null@*/ /*@out@*/ /*@only@*/ double *dx_pi, /*@null@*/ /*@out@*/ /*@only@*/ double *dy_pi, /*@null@*/ /*@out@*/ /*@only@*/ double *dz_pi, /*@null@*/ /*@out@*/ /*@only@*/ AIR_Pixels ***datain)

{
	if(e) AIR_free_2(e);
	if(f) AIR_free_2(f);

	if(dx_pi) free(dx_pi);
	if(dy_pi) free(dy_pi);
	if(dz_pi) free(dz_pi);

	if(datain) AIR_free_vol3(datain);
}

AIR_Error AIR_do_determinant_mask(const char *program, const char *warp_file, const char *output, const AIR_Boolean ow)

{
	/* Precheck of output permissions */
	{
		AIR_Error errcode=AIR_save_probw(output,ow);
		if(errcode!=0){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("Computations not performed due to anticipated output problems with %s\n",output);
			return(errcode);
		}
	}
	
	{
		struct AIR_Warp air1;
		double **e=NULL;
		double **f=NULL;
		double *dx_pi=NULL;
		double *dy_pi=NULL;
		double *dz_pi=NULL;
		AIR_Pixels ***datain=NULL;
		
		/*Get the reslice parameters*/
		{
			AIR_Error errcode;
			
			e=AIR_read_airw(warp_file,&air1,&errcode);
			if(!e){
				return(errcode);
			}
		}
		
		if(air1.coord<2 || air1.coord>3){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("This program does not support %u spatial dimensions\n",air1.coord);
			free_function(e,f,dx_pi,dy_pi,dz_pi,datain);
			return(AIR_POLYNOMIAL_DIMENSIONS_ERROR);
		}

		/* Allocate memory */
		dx_pi=AIR_matrix1(air1.coeffp);
		dy_pi=AIR_matrix1(air1.coeffp);
		f=AIR_matrix2(3,3);
		datain=AIR_create_vol3(air1.s.x_dim,air1.s.y_dim,air1.s.z_dim);
		if(!dx_pi || !dy_pi || !f || !datain){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("Memory allocation failed\n");
			free_function(e,f,dx_pi,dy_pi,dz_pi,datain);
			return(AIR_MEMORY_ALLOCATION_ERROR);
		}
		if(air1.coord>2){
			dz_pi=AIR_matrix1(air1.coeffp);
			if(!dz_pi){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("Memory allocation failed\n");
				free_function(e,f,dx_pi,dy_pi,dz_pi,datain);
				return(AIR_MEMORY_ALLOCATION_ERROR);
			}
		}
		{
			double pixel_size_s=air1.s.x_size;
			if(air1.s.y_size<pixel_size_s) pixel_size_s=air1.s.y_size;
			if(air1.s.z_size<pixel_size_s) pixel_size_s=air1.s.z_size;

			{
				unsigned int k;
				
				for(k=0;k<air1.s.z_dim;k++){
				
					unsigned int j;
					
					for(j=0;j<air1.s.y_dim;j++){
					
						unsigned int i;
						
						for(i=0;i<air1.s.x_dim;i++){
							if(air1.coord==3){
								if(AIR_diffg3D(e,(double)i,(double)j,(double)k,f,air1.order,dx_pi,dy_pi,dz_pi)!=0){
									datain[k][j][i]=0;
									continue;
								}
							}
							else if(air1.coord==2){
								if(AIR_diffg2D(e,(double)i,(double)j,f,air1.order,dx_pi,dy_pi)!=0){
									datain[k][j][i]=0;
									continue;
								}
							}
							else{
								printf("%s: %d: ",__FILE__,__LINE__);
								printf("Coding error in %s\n",program);
							}
							{
								unsigned int ii;
								
								for(ii=0;ii<3;ii++){
									f[0][ii]/=(air1.s.x_size/pixel_size_s);
									f[1][ii]/=(air1.s.y_size/pixel_size_s);
									f[2][ii]/=(air1.s.z_size/pixel_size_s);
								}
							}
							{
								unsigned int jj;
								
								for(jj=0;jj<3;jj++){	/*Note: j<3 instead of j<4 is not an error*/
								
									unsigned int ii;
									
									for(ii=0;ii<3;ii++){
										f[jj][ii]/=pixel_size_s;
									}
								}
							}
							{
								unsigned int jj;
								
								for(jj=0;jj<3;jj++){
									f[jj][0]*=air1.r.x_size;
									f[jj][1]*=air1.r.y_size;
									f[jj][2]*=air1.r.z_size;
								}
							}
							{
								double determinant;
								{
									unsigned int ipvt[3];
									if(AIR_dgefa(f,3,ipvt)!=3){
										datain[k][j][i]=0;
										continue;
									}
									determinant=AIR_ddet(f,3,ipvt);
								}
								if(determinant<=0.0) datain[k][j][i]=0;
								else datain[k][j][i]=1;
							}
						}
					}
				}
			}
		}

		/*Save the data*/
		{
			AIR_Error errcode=AIR_saveas1(datain,&air1.s,output,ow,(AIR_Pixels)1,(AIR_Pixels)2,program);
			if(errcode!=0){
				free_function(e,f,dx_pi,dy_pi,dz_pi,datain);
				return(errcode);
			}
		}
		free_function(e,f,dx_pi,dy_pi,dz_pi,datain);
	}
	return 0;
}

