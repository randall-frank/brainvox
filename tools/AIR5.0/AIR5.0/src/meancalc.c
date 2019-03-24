/* Copyright 1994-2001 Roger P. Woods, M.D. */
/* Modified 5/22/01 */

/* void meancalc()
 *
 * Routine computes a mean or weighted mean of the planes
 *  pointed to by data and returns this value
 *  (multiplied by rescale) at the
 *  corresponding locations pointed to by mean.
 *
 * If strictness>0, the mean at a point will be
 *  defined as zero if any of the planes are zero at
 *  that point.
 *
 * If strictness==0, any plane with a zero value at
 *  a point will be excluded when computing the mean
 *  at that point.
 *
 * If strictness<0, the true mean of all studies (even if zero)
 *   will be computed.
 *
 * If the hottest pixel on the plane exceeds the value stored
 *  at max, max is reset to this new hottest value
 *
 * Scaling is now applied invariably. If weighting==0, the
 *  scaling factors are viewed as multiplicative scaing factors
 *  but if weighting!=0, the scaling factors are viewed as
 *  weights in a weighted average. In the former case, the
 *  mean is calculated by dividing by the number of acceptable
 *  datapoints (where acceptable may be modified base on
 *  strictness). In the latter case, the mean is calculated
 *  by dividing by the sum of the weightings. The combination
 *  weighting!=0, strictness==0 is not allowed (11/1/94)
 *
 * In the interest of improved quantitation, values are now
 *  rounded off rather than truncated (11/1/94)
 */


#include "AIR.h"

AIR_Error AIR_meancalc(AIR_Pixels **mean, AIR_Pixels ***data, const double *scaling, const unsigned int quantity, const signed int strictness, const AIR_Boolean weighting, const unsigned int x_dim, const unsigned int y_dim, const double rescale, double *max)
/*Output=mean[y_dim][x_dim]*/
/*Input=data[quantity][y_dim][x_dim]*/
/*Weighting factor for each study=scaling[quantity]*/
/*quantity=Total number of studies to average*/
/*strictness>0 requires nonzero pixel in all input files to get nonzero mean pixel*/
/*weighting==0 results in equal weighting of all files*/
/*True weighted mean is multiplied by rescale to calculate mean*/
/* max is the value of the hottest pixel*/

{
	if(weighting && strictness==0){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("Coding error when calling meancalc(), weighted means should not use strictness of zero\n");
		return AIR_INVALID_MEANCALC_ERROR;
	}

	/*For every pixel*/
	{
		unsigned int j;
		
		for(j=0;j<y_dim;j++){
		
			unsigned int i;
			
			for(i=0;i<x_dim;i++){

				/*initialize values*/
				double cpix=0.0;
				unsigned int count=0;
				double scaler=0.0;
				unsigned int k;

				/*For each study*/
				for(k=0;k<quantity;k++){

					/*Add value of current pixel to cumulated total and increment count*/
					/*Don't include a study if the value of this pixel is zero unless*/
					/* strictness is negative */
					if((strictness<0)||data[k][j][i]!=0){
						if(weighting){
							cpix+=data[k][j][i]*scaling[k];
							count++;
							scaler+=scaling[k];
						}
						else{
							cpix+=data[k][j][i]*scaling[k];
							count++;
						}
					}
				}

				/*Find the mean value*/
				if(strictness>0){
					if(count!=quantity){
						mean[j][i]=0;
						continue;
					}
				}
				if(weighting){
					if(scaler==0){
						mean[j][i]=0;
						continue;
					}
					cpix=rescale*cpix/scaler;
					if(cpix>*max) *max=cpix;
					cpix+=.5;
					if(cpix>(double)AIR_CONFIG_MAX_POSS_VALUE){
						mean[j][i]=AIR_CONFIG_MAX_POSS_VALUE;
					}
					else mean[j][i]=(AIR_Pixels)cpix;

					continue;
				}
				if(count==0){
					mean[j][i]=0;
					continue;
				}
				cpix=rescale*cpix/count;
				if(cpix>*max) *max=cpix;
				cpix+=.5;
				if(cpix>(double)AIR_CONFIG_MAX_POSS_VALUE){
					mean[j][i]=AIR_CONFIG_MAX_POSS_VALUE;
				}
				else mean[j][i]=(AIR_Pixels)cpix;
			}
		}
	}
	return 0;
}
	
		
