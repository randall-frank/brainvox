/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/30/01 */


/* AIR_Pixels interp_lin_3D()
 *
 * Computes the interpolated value
 *
 */

#include "AIR.h"

AIR_Pixels AIR_interp_lin_3D(/*@unused@*/ const struct AIR_Key_info *stats, AIR_Pixels ***volume, const double x_p, const double y_p, const double z_p, const double scale, /*@unused@*/ const unsigned int *window, /*@unused@*/ const double pi)

{
	/* Find interpolated value */
	double a,b,c,d,e,f;

	unsigned int x_up=(unsigned int)ceil(x_p);
	unsigned int x_down=(unsigned int)floor(x_p);
	unsigned int y_up=(unsigned int)ceil(y_p);
	unsigned int y_down=(unsigned int)floor(y_p);
	unsigned int z_up=(unsigned int)ceil(z_p);
	unsigned int z_down=(unsigned int)floor(z_p);
	if(x_up==x_down){
		a=0;
		d=1;
	}
	else{
		a=x_p-x_down;
		d=x_up-x_p;
	}
	if(y_up==y_down){
		b=0;
		e=1;
	}
	else{
		b=y_p-y_down;
		e=y_up-y_p;
	}
	if(z_up==z_down){
		c=0;
		f=1;
	}
	else{
		c=z_p-z_down;
		f=z_up-z_p;
	}

	/*Interpolate */
	{
		AIR_Pixels *pi2, **pj2, ***pk2;
		double total;

		pk2=volume+z_down;
		pj2=*pk2+y_down;
		pi2=*pj2+x_down;
		total=*pi2*d*e*f;

		if (x_up!=x_down){
			pi2++;
			total+=*pi2*a*e*f;
		}

		if (y_up!=y_down){
			pj2++;
			pi2=*pj2+x_down;
			total+=*pi2*d*b*f;

			if (x_up!=x_down){
				pi2++;
				total+=*pi2*a*b*f;
			}
		}

		if (z_up!=z_down){
			pk2++;
			pj2=*pk2+y_down;
			pi2=*pj2+x_down;
			total+=*pi2*d*e*c;

			if (x_up!=x_down){
				pi2++;
				total+=*pi2*a*e*c;
			}

			if (y_up!=y_down){
				pj2++;
				pi2=*pj2+x_down;
				total+=*pi2*d*b*c;

				if (x_up!=x_down){
					pi2++;
					total+=*pi2*a*b*c;
				}
			}
		}
		total*=scale;
		total+=.5;
		if(total>(double)AIR_CONFIG_MAX_POSS_VALUE){
			 return AIR_CONFIG_MAX_POSS_VALUE;
		}
		else return (AIR_Pixels)total;
	}
}
