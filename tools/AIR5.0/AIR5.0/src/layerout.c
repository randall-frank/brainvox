/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 5/22/01 */

/*
 * AIR_Pixels ***layerout()
 *
 * This routine will produce a large screen layout
 * consisting of n x m images with designated start z
 * and z_step
 *
 * new_stats can point to the same location as stats
 */

#include "AIR.h"

AIR_Pixels ***AIR_layerout(AIR_Pixels ***volume, const struct AIR_Key_info *stats, struct AIR_Key_info *new_stats, const unsigned int n, const unsigned int m, const unsigned int start_z, const signed int z_step, AIR_Error *errcode)

{
	if(n<1 || m<1){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("layout must have positive dimensions, i.e. not %u by %u\n",n,m);
		*errcode=AIR_INVALID_LAYOUT_ERROR;
		return NULL;
	}
	{
		const unsigned int x_dim=stats->x_dim;
		const unsigned int y_dim=stats->y_dim;
		const unsigned int z_dim=stats->z_dim;
		
		AIR_Pixels ***new_volume=AIR_create_vol3(x_dim*n,y_dim*m,1);
		if(!new_volume){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("memory allocation failure\n");
			*errcode=AIR_MEMORY_ALLOCATION_ERROR;
			return NULL;
		}
		{
			/* Don't use new_stats yet, just in case new_stats==stats */
			struct AIR_Key_info temp_stats;
			
			temp_stats.bits=stats->bits;
			temp_stats.x_dim=stats->x_dim*n;
			temp_stats.y_dim=stats->y_dim*m;
			temp_stats.z_dim=1;
			temp_stats.x_size=stats->x_size;
			temp_stats.y_size=stats->y_size;
			temp_stats.z_size=stats->z_size*abs(z_step);

			AIR_zeros(new_volume,&temp_stats);

			{				
				AIR_Pixels ***temp_volume=AIR_flip_y(volume,stats,errcode);
				if(!temp_volume){
					AIR_free_vol3(new_volume);
					return NULL;
				}
				{
					unsigned int k=start_z;
					AIR_Pixels ***k1=temp_volume+start_z;
					unsigned int region=0;
					
					for (;region<n*m;k+=z_step,k1+=z_step,region++){
						if (k<z_dim){
						
							unsigned int y_offset=region/n;
							unsigned int x_offset=region-y_offset*n;
							
							y_offset*=y_dim;
							x_offset*=x_dim;
							{
								unsigned int j=0;
								AIR_Pixels **j1=*k1;
								AIR_Pixels **j2=*new_volume+y_offset;
								
								for (;j<y_dim;j++,j1++,j2++){
									unsigned int i=0;
									AIR_Pixels *i1=*j1;
									AIR_Pixels *i2=*j2+x_offset;
									
									for (;i<x_dim;i++,i1++,i2++){
										*i2=*i1;
									}
								}
							}
						}
						if(k==0 && z_step<0) break;
					}
				}
				AIR_free_vol3(temp_volume);
			}
			/* Now that we are done with stats, it's ok to set new_stats */
			*new_stats=temp_stats;
		}

		{
			AIR_Pixels ***temp_volume=AIR_flip_y(new_volume,new_stats,errcode);
			if(!temp_volume){
				AIR_free_vol3(new_volume);
				return NULL;
			}
			AIR_free_vol3(new_volume);
			new_volume=temp_volume;
		}
		return new_volume;
	}
}
