/* Copyright 1997-2001 Roger P. Woods, M.D. */
/* Modified: 5/30/01 */

/*
 *	This will reslice using the vector file and warp file
 */


#include "AIR.h"
#include <float.h>

AIR_Pixels ***AIR_r_vec_warp_3D(AIR_Pixels ***volume, const struct AIR_Key_info *stats, const struct AIR_Key_info *new_stats, float ****field, double **es, const double scale, const unsigned int order, const unsigned int *window, AIR_Pixels (*interp3D)(const struct AIR_Key_info *, AIR_Pixels ***, const double, const double, const double, const double, const unsigned int *, const double), AIR_Error *errcode)

{
	if(order>AIR_CONFIG_MAXORDER){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("Can't reslice polynomial of order %u\n",order);
		*errcode=AIR_POLYNOMIAL_ORDER_ERROR;
		return NULL;
	}
	{
		const double pi=2*acos((double)0.0);
	
		const double
			x_max1=stats->x_dim-1.0,
			y_max1=stats->y_dim-1.0,
			z_max1=stats->z_dim-1.0;
	
		const unsigned int
			x_dim2=new_stats->x_dim,
			y_dim2=new_stats->y_dim,
			z_dim2=new_stats->z_dim;
		
		AIR_Pixels ***new_volume=AIR_create_vol3(x_dim2, y_dim2, z_dim2);
		if(!new_volume){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("unable to allocate memory to reslice file\n");
			*errcode=AIR_MEMORY_ALLOCATION_ERROR;
			return NULL;
		}
		AIR_zeros(new_volume,new_stats);	/* This must be called to use the continue statements */
		{
			double *dx;
			{
				unsigned int coeffp=(order+1)*(order+2)*(order+3)/6;
				dx=AIR_matrix1(coeffp);
				if(!dx){
					AIR_free_vol3(new_volume);
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("unable to allocate memory to reslice file\n");
					*errcode=AIR_MEMORY_ALLOCATION_ERROR;
					return NULL;
				}
			}
		
			/* For each point */
			{
				unsigned int k=0;
				AIR_Pixels ***pk3=new_volume;
				float ****fk=field;
							
				for(;k<z_dim2; k++, pk3++, fk++){
				
					unsigned int j=0;
					AIR_Pixels **pj3=*pk3;
					float ***fj=*fk;
					
					for(;j<y_dim2;j++, pj3++, fj++){
					
						unsigned int i=0;
						AIR_Pixels *pi3=*pj3;
						float **fi=*fj;
									
						for(;i<x_dim2;i++, pi3++,fi++){
						
							float *fd=*fi;
						
							double x=*fd++;
							double y=*fd++;
							double z=*fd++;
							
							double x_p, y_p, z_p;
		
							if(x==FLT_MAX) continue; /* FLT_MAX is an error flag */
							
							if(AIR_warp3D(es,x,y,z,&x_p,&y_p,&z_p,order,dx)!=0){
								continue;
							}
							
							if(x_p<0 || x_p>x_max1) continue;
							if(y_p<0 || y_p>y_max1) continue;
							if(z_p<0 || z_p>z_max1) continue;
							
							*pi3=(*interp3D)(stats,volume,x_p,y_p,z_p,scale,window,pi);
						}
					}
				}
			}
			free(dx);
		}
		*errcode=0;
		return new_volume;
	}
}
