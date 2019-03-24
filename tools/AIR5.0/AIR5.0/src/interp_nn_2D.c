/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/30/01 */


/* AIR_Pixels interp_nn_2D()
 *
 * returns the interpolated value
 */

#include "AIR.h"

AIR_Pixels AIR_interp_nn_2D(/*@unused@*/ const struct AIR_Key_info *stats, AIR_Pixels ***volume, const double x_p, const double y_p, const unsigned int k, const double scale, /*@unused@*/ const unsigned int *window, /*@unused@*/ const double pi)

{
	/* Find interpolated value */
	unsigned int x_up=(unsigned int)floor(x_p+.5);
	unsigned int y_up=(unsigned int)floor(y_p+.5);

	/* Interpolate */
	{
		double total;

		total=(double)volume[k][y_up][x_up];

		total*=scale;
		total+=.5;
		if(total>(double)AIR_CONFIG_MAX_POSS_VALUE){
			 return AIR_CONFIG_MAX_POSS_VALUE;
		}
		else return (AIR_Pixels)total;
	}
}
