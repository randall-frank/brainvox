/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 5/23/01 */

/*
 * This program will create multiplane layouts that display slices from
 * multiple files (with identical dimensions) simultaneously side-by-side.
 */


#include "AIR.h"

static void free_function(/*@null@*/ /*@out@*/ /*@only@*/ AIR_Pixels ***datain, /*@null@*/ /*@out@*/ /*@only@*/ AIR_Pixels ***dataout)

{
	if(datain) AIR_free_vol3(datain);
	if(dataout) AIR_free_vol3(dataout);
}

AIR_Error AIR_do_fuse(const char *program, const char *output, const unsigned int width, const unsigned int height, const unsigned int n, char **ids, const AIR_Boolean ow)

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
		/*Read header of first file*/
		struct AIR_Key_info dimensions1;
		{
			struct AIR_Fptrs fps;
			{
				AIR_Error errcode;
				int dummy[8];
				
				(void)AIR_open_header(ids[0],&fps,&dimensions1,dummy);
				if(fps.errcode!=0){
					errcode=fps.errcode;
					AIR_close_header(&fps);
					return(errcode);
				}
			}

			AIR_close_header(&fps);
			if(fps.errcode!=0){
				return(fps.errcode);
			}
		}
		
		{
			struct AIR_Key_info dimensionsout;

			/* Allocate output file */
			dimensionsout.bits=8*sizeof(AIR_Pixels);
			dimensionsout.x_dim=dimensions1.x_dim*width;
			dimensionsout.y_dim=dimensions1.y_dim*height;
			dimensionsout.z_dim=dimensions1.z_dim;
			dimensionsout.x_size=dimensions1.x_size;
			dimensionsout.y_size=dimensions1.y_size;
			dimensionsout.z_size=dimensions1.z_size;
			
			{
				AIR_Pixels ***dataout=NULL;
							
				dataout=AIR_create_vol3(dimensionsout.x_dim,dimensionsout.y_dim,dimensionsout.z_dim);
				if(!dataout){
					return(AIR_MEMORY_ALLOCATION_ERROR);
				}
				AIR_zeros(dataout,&dimensionsout);

				{
					unsigned int w=0;
					unsigned int h=height-1;
					unsigned int k;

					for(k=0;k<n;k++){
					
						if(ids[k][0]!=NULL){
						
							struct AIR_Key_info dimensionsN;
							AIR_Pixels ***datain=NULL;
							
							{
								AIR_Error errcode;

								datain=AIR_load(ids[k], &dimensionsN, TRUE, &errcode);
								if(!datain){
									free_function(datain,dataout);
									return(errcode);
								}
							}
							{
								AIR_Error errcode=AIR_same_dim(&dimensions1,&dimensionsN);
								if(errcode!=0){
									printf("%s: %d: ",__FILE__,__LINE__);
									printf("Dimensions of file %s do not match dimensions of earlier files\n",ids[k]);
									free_function(datain,dataout);
									return(errcode);
								}
							}
							{
								AIR_Error errcode=AIR_same_size(&dimensions1,&dimensionsN);
								if(errcode!=0){
									printf("WARNING: Voxel sizes of file %s do not match sizes of earlier files\n",ids[k]);
								}
							}
							{
								unsigned int kk;
								
								for(kk=0;kk<dimensions1.z_dim;kk++){
								
									unsigned int j;
									
									for(j=0;j<dimensions1.y_dim;j++){
									
										unsigned int i;
										
										for(i=0;i<dimensions1.x_dim;i++){
											dataout[kk][j+h*dimensions1.y_dim][i+w*dimensions1.x_dim]=datain[kk][j][i];
										}
									}
								}
							}
							AIR_free_vol3(datain);
							datain=NULL;
						}
						w++;
						if(w==width){
							w=0;
							if(h!=0) h--;
							else break;
						}
					}
				}
				{
					AIR_Error errcode=AIR_save(dataout,&dimensionsout,output,ow,program);
					if(errcode!=0){
						AIR_free_vol3(dataout);
						return(errcode);
					}
				}
				AIR_free_vol3(dataout);
				
				if(n>width*height){
					printf("WARNING: number of input files exceeded the capacity of the specified layout\n");
				}
			}
		}
	}
	return 0;
}
