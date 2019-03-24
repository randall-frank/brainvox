/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/30/01 */


/* AIR_Pixels ***r_uwrp_2D
 *
 * This routine will reslice a file based on the unit vectors es
 *
 * returns:
 *	pointer to resliced data if successful
 *	NULL pointer if unsuccessful
 */

#include "AIR.h"

static void free_function(/*@null@*/ /*@out@*/ /*@only@*/ double *dx, /*@null@*/ /*@out@*/ /*@only@*/ double *dx_pi, /*@null@*/ /*@out@*/ /*@only@*/ double *dx_pj)

{
	if(dx) free(dx);
	if(dx_pi) free(dx_pi);
	if(dx_pj) free(dx_pj);
}

AIR_Pixels ***AIR_r_uwrp_2D(AIR_Pixels ***volume, const struct AIR_Key_info *stats, const struct AIR_Key_info *new_stats, double **es, const double scale, const unsigned int order, double **e_guess, const signed int parity, const double accuracy, const unsigned int *window, AIR_Pixels (*interp2D)(const struct AIR_Key_info *, AIR_Pixels ***, const double, const double, const unsigned int, const double, const unsigned int *, const double), AIR_Error *errcode)

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
			y_max1=stats->y_dim-1.0;
			
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
			{
				unsigned int coeffp=(order+1)*(order+2)/2;
				dx=AIR_matrix1(coeffp);
				if(!dx || !dx_pi || !dx_pj){
					AIR_free_vol3(new_volume);
					free_function(dx,dx_pi,dx_pj);
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("unable to allocate memory to reslice file\n");
					*errcode=AIR_MEMORY_ALLOCATION_ERROR;
					return NULL;
				}
			}
			{
				double *ed[3];		/* Must be 3 even for 2D */
				double ed0[9];		/* Must be 9 even for 2D */
			
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
			
								double x_p, y_p;
			
								/* Make initial guess */
								if(AIR_warp2D(e_guess,(double)i,(double)j,&x_p,&y_p,1,dx)!=0){
									continue;
								}
			
								/* Iteratively improve initial guess to get final solution */
								if(AIR_antiwarp2D(es,(double)i,(double)j,&x_p,&y_p,order,dx_pi,dx_pj,dx,accuracy)!=0){
									continue;
								}
			
								/* Make sure that determinant is positive at identified solution */
								{
									if(AIR_diffg2D(es,x_p,y_p,ed,order,dx_pi,dx_pj)!=0){
										continue;
									}
									{
										unsigned int ipvt[3];	/* Must be 3 even for 2D */
										
										if(AIR_dgefa(ed,3,ipvt)!=3) continue;
										if(parity*AIR_ddet(ed,3,ipvt)<=0) continue;
									}
								}
			
								if(x_p<0 || x_p>x_max1) continue;
								if(y_p<0 || y_p>y_max1) continue;
			
								*pi3=(*interp2D)(stats,volume,x_p,y_p,k,scale,window,pi);
							}
						}
					}
				}
			}
			free_function(dx,dx_pi,dx_pj);
		}
		*errcode=0;
		return new_volume;
	}
}
