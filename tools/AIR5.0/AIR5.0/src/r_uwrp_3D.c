/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/30/01 */


/* AIR_Pixels ***r_uwrp_3D()
 *
 * This routine will reslice a file based on the unit vectors es
 *
 * returns:
 *	pointer to resliced data if successful
 *	NULL pointer if unsuccessful
 */

#include "AIR.h"

static void free_function(/*@null@*/ /*@out@*/ /*@only@*/ double *dx, /*@null@*/ /*@out@*/ /*@only@*/ double *dx_pi, /*@null@*/ /*@out@*/ /*@only@*/ double *dx_pj, /*@null@*/ /*@out@*/ /*@only@*/ double *dx_pk)

{
	if(dx) free(dx);
	if(dx_pi) free(dx_pi);
	if(dx_pj) free(dx_pj);
	if(dx_pk) free(dx_pk);
}

AIR_Pixels ***AIR_r_uwrp_3D(AIR_Pixels ***volume, const struct AIR_Key_info *stats, const struct AIR_Key_info *new_stats, double **es, const double scale, const unsigned int order, double **e_guess, const signed int parity, const double accuracy, const unsigned int *window, AIR_Pixels (*interp3D)(const struct AIR_Key_info *, AIR_Pixels ***, const double, const double, const double, const double, const unsigned int *, const double), AIR_Error *errcode)

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
			double *dx=NULL;
			double *dx_pi=NULL;
			double *dx_pj=NULL;
			double *dx_pk=NULL;
			{
				unsigned int coeffp=(order+1)*(order+2)*(order+3)/6;
				dx=AIR_matrix1(coeffp);
				dx_pi=AIR_matrix1(coeffp);
				dx_pj=AIR_matrix1(coeffp);
				dx_pk=AIR_matrix1(coeffp);
				
				if(!dx || !dx_pi || !dx_pj || !dx_pk){
					AIR_free_vol3(new_volume);
					free_function(dx, dx_pi, dx_pj, dx_pk);
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("unable to allocate memory to reslice file\n");
					*errcode=AIR_MEMORY_ALLOCATION_ERROR;
					return NULL;
				}
			}
			{
				double *ed[3];
				double ed0[9];

				ed[0]=ed0;
				ed[1]=ed0+3;
				ed[2]=ed0+6;

				/* Reslice */
				{
					unsigned int k=0;
					AIR_Pixels ***pk3=new_volume;
			
					for (;k<z_dim2;k++,pk3++){
			
						unsigned int j=0;
						AIR_Pixels **pj3=*pk3;
			
						for (;j<y_dim2;j++,pj3++){
			
							unsigned int i=0;
							AIR_Pixels *pi3=*pj3;
			
							for (;i<x_dim2;i++,pi3++){
			
								double x_p, y_p, z_p;
			
								/* Make initial guess */
								if(AIR_warp3D(e_guess,(double)i,(double)j,(double)k,&x_p,&y_p,&z_p,1,dx)!=0){
									continue;
								}
			
								/* Iteratively improve initial guess to get final solution */
								if(AIR_antiwarp3D(es,(double)i,(double)j,(double)k,&x_p,&y_p,&z_p,order,dx_pi,dx_pj,dx_pk,dx,accuracy)!=0){
									continue;
								}
			
								/* Make sure that determinant is positive at identified solution */
								{
									if(AIR_diffg3D(es,x_p,y_p,z_p,ed,order,dx_pi,dx_pj,dx_pk)!=0){
										continue;
									}
									{
										unsigned int ipvt[3];
										if(AIR_dgefa(ed,3,ipvt)!=3) continue;
										if(parity*AIR_ddet(ed,3,ipvt)<=0) continue;
									}
								}
			
								if(x_p<0 || x_p>x_max1) continue;
								if(y_p<0 || y_p>y_max1) continue;
								if(z_p<0 || z_p>z_max1) continue;
			
								*pi3=(*interp3D)(stats,volume,x_p,y_p,z_p,scale,window,pi);
							}
						}
					}
				}
			}
			free_function(dx,dx_pi,dx_pj,dx_pk);
		}
		*errcode=0;
		return new_volume;
	}
}
