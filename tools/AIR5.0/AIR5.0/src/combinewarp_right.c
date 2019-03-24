/* Copyright 1997-2001 Roger P. Woods, M.D. */
/* Modified: 5/27/01 */

/* 
 * Combines an affine airwarp with a nonlinear airwarp, retaining the standard space of the nonlinear airwarp
 * The affine airwarp and its coefficients e are unchanged upon return
 */

#include "AIR.h"

AIR_Error AIR_combinewarp_right(struct AIR_Warp *airnonlinear, double ***f, const struct AIR_Warp airlinear_orig, double **e_orig)
{
	if(airlinear_orig.coord<2 || airlinear_orig.coord>3){
		return(AIR_POLYNOMIAL_DIMENSIONS_ERROR);
	}
	if(airlinear_orig.order>1){
		return(AIR_WARP_NOT_FIRST_ORDER_ERROR);
	}
	if(airnonlinear->coord<2 || airnonlinear->coord>3){
		return(AIR_POLYNOMIAL_DIMENSIONS_ERROR);
	}
	if(airnonlinear->order>AIR_CONFIG_MAXORDER){
		return(AIR_POLYNOMIAL_ORDER_ERROR);
	}
	{
		struct AIR_Warp airlinear=airlinear_orig;
		double *e[4];
		double e_items[16];
				
		e[0]=e_items;
		e[1]=e_items+4;
		e[2]=e_items+8;
		e[3]=e_items+12;
		{
			unsigned int j;
			for(j=0;j<=airlinear.coord-1;j++){
				unsigned int i;
				for(i=0;i<=airlinear.coord;i++){
					e[j][i]=e_orig[j][i];
				}
			}
		}		
		{
			double pixel_size_s=airlinear.s.x_size;
			if(airlinear.s.y_size<pixel_size_s) pixel_size_s=airlinear.s.y_size;
			if(airlinear.s.z_size<pixel_size_s) pixel_size_s=airlinear.s.z_size;

			/*If dimensions don't match try interpolating linear air file*/
			/* to cubic voxels */
			if(airlinear.s.x_dim!=airnonlinear->r.x_dim){
				double xoom2=airlinear.s.x_size/pixel_size_s;
				airlinear.s.x_dim=(airlinear.s.x_dim-1)*xoom2+1;
				e[0][1]*=(pixel_size_s/airlinear.s.x_size);
				e[1][1]*=(pixel_size_s/airlinear.s.x_size);
				e[2][1]*=(pixel_size_s/airlinear.s.x_size);
				airlinear.s.x_size=pixel_size_s;
			}

			if(airlinear.s.y_dim!=airnonlinear->r.y_dim){
				double yoom2=airlinear.s.y_size/pixel_size_s;
				airlinear.s.y_dim=(airlinear.s.y_dim-1)*yoom2+1;
				e[0][2]*=(pixel_size_s/airlinear.s.y_size);
				e[1][2]*=(pixel_size_s/airlinear.s.y_size);
				e[2][2]*=(pixel_size_s/airlinear.s.y_size);
				airlinear.s.y_size=pixel_size_s;
			}

			if(airlinear.s.z_dim!=airnonlinear->r.z_dim){
				double zoom2=airlinear.s.z_size/pixel_size_s;
				airlinear.s.z_dim=(airlinear.s.z_dim-1)*zoom2+1;
				e[0][3]*=(pixel_size_s/airlinear.s.z_size);
				e[1][3]*=(pixel_size_s/airlinear.s.z_size);
				e[2][3]*=(pixel_size_s/airlinear.s.z_size);
				airlinear.s.z_size=pixel_size_s;
			}
		}
		{
			AIR_Error errcode=AIR_same_dim(&airnonlinear->r,&airlinear.s);
			if(errcode!=0){
				return(errcode);
			}
		}
		{
			AIR_Error errcode=AIR_same_size(&airnonlinear->r,&airlinear.s);
			if(errcode!=0){
				return(errcode);
			}
		}
		
		/* Change reslice file in airnonlinear to that in airlinear */
		strcpy(airnonlinear->r_file,airlinear.r_file);
		airnonlinear->r.x_dim=airlinear.r.x_dim;
		airnonlinear->r.y_dim=airlinear.r.y_dim;
		airnonlinear->r.z_dim=airlinear.r.z_dim;
		airnonlinear->r.x_size=airlinear.r.x_size;
		airnonlinear->r.y_size=airlinear.r.y_size;
		airnonlinear->r.z_size=airlinear.r.z_size;
		airnonlinear->r.bits=airlinear.r.bits;
				
		/* Handle 2D airnonlinear */
		if(airnonlinear->coord==2){
		
			AIR_Boolean promoting=FALSE;
			
			if(airlinear.coord==3){
				/* See if 3D parameters are irrelevant--if not, set to promote */
				if(airlinear.order==0 && e[2][0]!=0.0) promoting=TRUE;
				else if(e[0][3]!=0.0||e[1][3]!=0.0||e[2][3]!=1.0||e[2][0]!=0.0||e[2][1]!=0.0||e[2][2]!=0.0){
					promoting=TRUE;
				}
			}
			if(!promoting){
			
				/* Combine 2D */
				AIR_Error errcode=AIR_blat40(*f,e,airnonlinear->order);
				if(errcode!=0){
					return(errcode);
				}
				return 0;
			}
			else{
				/* Promote airnonlinear to 3D */
				airnonlinear->coord=3;
				airnonlinear->coeffp=(airnonlinear->order+1)*(airnonlinear->order+2)*(airnonlinear->order+3)/6;
				{
					AIR_Error errcode;

					double **g=AIR_promote(airnonlinear->order,*f,&errcode);
					if(!g){
						return(errcode);
					}
					AIR_free_2(*f);
					*f=g;
				}
			}
		}
		/* Now we are dealing with 3D airnonlinear */
		if(airlinear.coord==2){
		
			/* Promote coefficients of airlinear to 3D */
			double **g;
			{
				AIR_Error errcode;

				g=AIR_promote(1,e,&errcode);
				if(!g){
					return(errcode);
				}
			}
			/* Combine 3D using g */
			{
				AIR_Error errcode=AIR_blat400(*f,g,airnonlinear->order);
				if(errcode!=0){
					AIR_free_2(g);
					return(errcode);
				}
			}
			AIR_free_2(g);
		}
		else{
			/* Combine 3D using e */
			AIR_Error errcode=AIR_blat400(*f,e,airnonlinear->order);
			if(errcode!=0){
				return(errcode);
			}
		}
		return 0;
	}
}
