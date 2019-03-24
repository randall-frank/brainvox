/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 12/8/01 */

/*
 * This program will create a binary mask of the input
 *  data with all values between min and max inclusive
 *  converted to 1's and all other values converted to 0's
 */

#include "AIRmain.h"

int main(const int argc, char *argv[])

{
	if (argc<3 || argc>6){
		printf("Usage: %s input output [overwrite?(y/n)][min][max]\n",argv[0]);
		printf("\tdefault maps all non-black voxels to one\n");
		printf("\totherwise, max to min inclusive (i.e., [max,min]) is mapped to one\n");
		exit(EXIT_FAILURE);
	}
	{
		AIR_Boolean use_default_min=TRUE;
		AIR_Boolean use_default_max=TRUE;
		
		long int min=(AIR_CONFIG_IMPOSSIBLE_THRESHOLD);
		long int max=(AIR_CONFIG_IMPOSSIBLE_THRESHOLD);
		
		if(argc>4){
			use_default_min=FALSE;
			{
				AIR_Error errcode;
				
				min=AIR_parse_long(argv[4], &errcode);
				if(errcode!=0){
					AIR_report_error(errcode);
					exit(EXIT_FAILURE);
				}
			}
			if(argc>5){
				use_default_max=FALSE;
				{
					AIR_Error errcode;
					
					max=AIR_parse_long(argv[5], &errcode);
					if(errcode!=0){
						AIR_report_error(errcode);
						exit(EXIT_FAILURE);
					}
				}
			}
		}
		{
			AIR_Boolean ow=FALSE;
			if(argc>3) ow=argv[3][0]=='y';
			{
				AIR_Error errcode=AIR_do_binarize(argv[0], argv[1], argv[2], use_default_min, min, use_default_max, max, ow);
				if(errcode!=0){
					AIR_report_error(errcode);
					exit(EXIT_FAILURE);
				}
			}
		}
	}
	return EXIT_SUCCESS;
}
