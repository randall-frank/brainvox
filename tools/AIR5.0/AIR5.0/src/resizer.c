/* Copyright 1993-2001 Roger P. Woods, M.D. */
/* Modified: 5/7/01 */

/*
 * AIR_Pixels ***resizer()
 *
 * This routine will resize the matrix of a 3D volume
 *
 */

#include "AIR.h"

AIR_Pixels ***AIR_resizer(AIR_Pixels ***volume, const struct AIR_Key_info *stats, struct AIR_Key_info *new_stats, const unsigned int new_x_dim, const unsigned int new_y_dim, const unsigned int new_z_dim, const signed int x_shift, const signed int y_shift, const signed int z_shift, AIR_Error *errcode)

{
	AIR_Pixels 	***new_volume;
	
	const unsigned int x_dim=stats->x_dim;
	const unsigned int y_dim=stats->y_dim;
	const unsigned int z_dim=stats->z_dim;

	/*Check for negative file dimensions*/
	if (new_x_dim==0 || new_y_dim==0 || new_z_dim==0){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("file dimensions <1 are not allowed\n");
		*errcode=AIR_INVALID_RESIZING_ERROR;
		return NULL;
	}
	
	new_volume=AIR_create_vol3(new_x_dim,new_y_dim,new_z_dim);
	if(!new_volume){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("unable to allocate memory for resized file\n");
		*errcode=AIR_MEMORY_ALLOCATION_ERROR;
		return NULL;
	}
	
	*new_stats=*stats;

	new_stats->x_dim=new_x_dim;
	new_stats->y_dim=new_y_dim;
	new_stats->z_dim=new_z_dim;

	AIR_zeros(new_volume,new_stats);
	{
		AIR_Pixels ***k1, ***k2, ***k1_end, ***k2_end;
		if(z_shift>0){
			k1=volume+z_shift;
			k2=new_volume;
		}
		else{
			k1=volume;
			k2=new_volume-z_shift;
		}
		k1_end=volume+z_dim;
		k2_end=new_volume+new_z_dim;
		
		for(;k1<k1_end && k2<k2_end;k1++,k2++){
		
			AIR_Pixels **j1, **j2, **j1_end, **j2_end;
			if(y_shift>0){
				j1=*k1+y_shift;
				j2=*k2;
			}
			else{
				j1=*k1;
				j2=*k2-y_shift;
			}
			j1_end=*k1+y_dim;
			j2_end=*k2+new_y_dim;
			for(;j1<j1_end && j2<j2_end;j1++,j2++){
			
				AIR_Pixels *i1, *i2, *i1_end, *i2_end;
				if(x_shift>0){
					i1=*j1+x_shift;
					i2=*j2;
				}
				else{
					i1=*j1;
					i2=*j2-x_shift;
				}
				i1_end=*j1+x_dim;
				i2_end=*j2+new_x_dim;
				for(;i1<i1_end && i2<i2_end;i1++,i2++){
					*i2=*i1;
				}
			}
		}
	}
	*errcode=0;
	return new_volume;
}
