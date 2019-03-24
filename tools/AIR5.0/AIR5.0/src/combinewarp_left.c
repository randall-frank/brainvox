/* Copyright 1997-2001 Roger P. Woods, M.D. */
/* Modified: 5/22/01 */

/* 
 * Combines an affine airwarp with a nonlinear airwarp, retaining the standard space of the affine airwarp
 * The affine airwarp and its coefficients e are unchanged upon return
 */

#include "AIR.h"

AIR_Error AIR_combinewarp_left(const struct AIR_Warp airlinear, double **e, struct AIR_Warp *airnonlinear, double ***f)
{
	if(airlinear.coord<2 || airlinear.coord>3){
		return(AIR_POLYNOMIAL_DIMENSIONS_ERROR);
	}
	if(airlinear.order>1){
		return(AIR_WARP_NOT_FIRST_ORDER_ERROR);
	}
	if(airnonlinear->coord<2 || airnonlinear->coord>3){
		return(AIR_POLYNOMIAL_DIMENSIONS_ERROR);
	}
	if(airnonlinear->order>AIR_CONFIG_MAXORDER){
		return(AIR_POLYNOMIAL_ORDER_ERROR);
	}
	{
		AIR_Error errcode=AIR_same_dim(&airlinear.r,&airnonlinear->s);
		if(errcode!=0){
			return(errcode);
		}
	}
	{
		AIR_Error errcode=AIR_same_size(&airlinear.r,&airnonlinear->s);
		if(errcode!=0){
			return(errcode);
		}
	}
	
	/* Change standard file in airnonlinear to that in airlinear */
	strcpy(airnonlinear->s_file, airlinear.s_file);
	airnonlinear->s.x_dim=airlinear.s.x_dim;
	airnonlinear->s.y_dim=airlinear.s.y_dim;
	airnonlinear->s.z_dim=airlinear.s.z_dim;
	airnonlinear->s.x_size=airlinear.s.x_size;
	airnonlinear->s.y_size=airlinear.s.y_size;
	airnonlinear->s.z_size=airlinear.s.z_size;
	airnonlinear->s.bits=airlinear.s.bits;

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
			AIR_Error errcode=AIR_blat100(*f,e,airnonlinear->order);
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
			AIR_Error errcode=AIR_blat1000(*f,g,airnonlinear->order);
			if(errcode!=0){
				AIR_free_2(g);
				return(errcode);
			}
		}
		AIR_free_2(g);
	}
	else{
		/* Combine 3D using e */
		AIR_Error errcode=AIR_blat1000(*f,e,airnonlinear->order);
		if(errcode!=0){
			return(errcode);
		}
	}
	return 0;
}

