/* Copyright 1997-2001 Roger P. Woods, M.D. */
/* Modified: 5/24/01 */

/*
 * This program will combine a nonlinear .warp file
 * with affine .air files to produce a single nonlinear .warp file that will
 * have the same net spatial result.
 *
 * This avoids the need to repeatedly reslice a file to
 *  various locations, and also prevents the accumulation
 *  of interpolation errors.
 *
 */

#include "AIRmain.h"

int main(int argc, char *argv[])

{
	if(argc<4){
		printf("\nUsage: %s output overwrite?(y/n) [.air_file1....airfileN1] .warp_file [.air_fileN1+1....air_fileN2\n",argv[0]);
		printf("\n\treslice_file in each air_file should be spatially equivalent to standard_file in the air_file that follows it\n");
		printf("\n\toutput standard_file will be the same as that in the first file named\n\n");
		printf("\toutput reslice_file will be the same as that in the last file named\n\n");
		printf("\t.air files must be affine (i.e., not perspective)\n");
		exit(EXIT_FAILURE);
	}

	if(strstr(argv[1],AIR_CONFIG_IMG_SUFF)!=0||strstr(argv[1],AIR_CONFIG_HDR_SUFF)!=0){
		printf("\nName of output .air file cannot contain %s or %s\n",AIR_CONFIG_IMG_SUFF,AIR_CONFIG_HDR_SUFF);
		exit(EXIT_FAILURE);
	}
	{
		AIR_Error errcode=AIR_do_combine_warp(argv[0], argv[1], (unsigned int)argc-3, argv+3, argv[2][0]=='y');

		if(errcode!=0){
			AIR_report_error(errcode);
			exit(EXIT_FAILURE);
		}
	}
	return EXIT_SUCCESS;
}

