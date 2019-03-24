/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/30/01 */


/* AIR_Pixels interp_wsinc_3D()
 *
 * returns the interpolated value
 */

#include "AIR.h"

static double wsinc(const double alpha, const double pi, const unsigned int kern)

{
	if(alpha==0) return 1.0;
	return 0.5*(1.0+cos(pi*alpha/kern))*sin(pi*alpha)/(pi*alpha);
}

AIR_Pixels AIR_interp_wsinc_3D(const struct AIR_Key_info *stats, AIR_Pixels ***volume, const double x_p, const double y_p, const double z_p, const double scale, const unsigned int *window, const double pi)

{
	const unsigned int xkern=window[0];
	const unsigned int ykern=window[1];
	const unsigned int zkern=window[2];
	
	const unsigned int x_dim=stats->x_dim;
	const unsigned int y_dim=stats->y_dim;
	const unsigned int z_dim=stats->z_dim;
	
	unsigned int x_bot, x_top;
	unsigned int y_bot, y_top;
	unsigned int z_bot, z_top;
	
	x_bot=(unsigned int)floor(x_p);
	if(x_bot>=xkern) x_bot-=(xkern-1);
	else x_bot=0;

	x_top=(unsigned int)ceil(x_p)+xkern;
	if(x_dim<x_top) x_top=x_dim;
	
	y_bot=(unsigned int)floor(y_p);
	if(y_bot>=ykern) y_bot-=(ykern-1);
	else y_bot=0;
	
	y_top=(unsigned int)ceil(y_p)+ykern;
	if(y_dim<y_top) y_top=y_dim;
	
	z_bot=(unsigned int)floor(z_p);
	if(z_bot>=zkern) z_bot-=(zkern-1);
	else z_bot=0;
	
	z_top=ceil(z_p)+zkern;
	if(z_dim<z_top) z_top=z_dim;
	
	/* Find interpolated value */
	{
		double total=0.0;

		{
			AIR_Pixels ***pk2=volume+z_bot;
			unsigned int kx4=z_bot;

			for(;kx4<z_top;kx4++,pk2++){

				double sincz=wsinc((z_p-kx4),pi,zkern);

				AIR_Pixels **pj2=*pk2+y_bot;
				unsigned int jx4=y_bot;

				for(;jx4<y_top;jx4++,pj2++){

					double sincyz=wsinc((y_p-jx4),pi,ykern)*sincz;

					AIR_Pixels *pi2=*pj2+x_bot;
					unsigned int ix4=x_bot;

					for(;ix4<x_top;ix4++,pi2++){

						double sincxyz=wsinc((x_p-ix4),pi,xkern)*sincyz;

						total+=*pi2*sincxyz;
					}
				}
			}
		}
		total*=scale;
		if(total<0.0) return 0;
		else{
			total+=.5;
			if(total>(double)AIR_CONFIG_MAX_POSS_VALUE) return AIR_CONFIG_MAX_POSS_VALUE;
			else return (AIR_Pixels)total;
		}
	}
}
