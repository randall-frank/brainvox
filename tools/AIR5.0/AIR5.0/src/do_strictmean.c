/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 5/22/01 */

#include "AIR.h"

#define STRICTNESS 1	/*STRICTNESS=0 computes mean of all non-zero pixels at a given location*/
			/*STRICTNESS>0 outputs zero where any input pixel is zero*/
			/*STRICTNESS<0 computes mean of all pixels at given location*/

#define WEIGHTING 0	/*WEIGHTING=1 results in a weighted average based on quantitation files*/
			/*WEIGHTING=0 results in a unweighted (i.e., normal) average*/
			/*If the above doesn't make sense, use WEIGHTING 0 */
			/*Note that input values are rescaled in either case*/

#define RESCALE TRUE	/*RESCALE=TRUE results in output being rescaled with maximum of MAX_POSS_VALUE*/
			/*RESCALE=FALSE will not rescale input data (largest scaling factor of inputs */
			/* will apply to output */

AIR_Error AIR_do_strictmean(const char *program, const char *output, const unsigned int n, char **inputs, const char *suffix, const AIR_Boolean ow)

{
	/* Precheck of output permissions */
	{
		AIR_Error errcode=AIR_save_probw(output,ow);
		
		if(errcode!=0){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("Computations not performed due to anticipated output problems with %s\n",output);
			return(errcode);
		}
	}

	printf("%u files to average\n",n);
	
	{
		double scalingmax;
		{
			AIR_Pixels ***means;
			struct AIR_Key_info means_stats;
			{
				AIR_Error errcode;
				
				means=AIR_meaner(&means_stats,&scalingmax,STRICTNESS,WEIGHTING,suffix,RESCALE,inputs,n,&errcode);
				if(!means){
					return(errcode);
				}
			}
			{
				AIR_Error errcode=AIR_save(means,&means_stats,output,ow,program);
				
				if(errcode!=0){
					AIR_free_vol3(means);
					return(errcode);
				}
			}
			AIR_free_vol3(means);
		}

		/*Write out rescaling value if a suffix is given*/
		/* Otherwise, print out rescaling factor if values have been rescaled */
		
		if(suffix){
			{
				AIR_Error errcode=AIR_write_value(scalingmax,output,suffix,ow);
				
				if(errcode!=0){
					return(errcode);
				}
			}
		}
		else{
			if(scalingmax!=1.0){
				printf("Values have been rescaled by a factor of %e\n",1/scalingmax);
			}
		}
	}
	return 0;
}

