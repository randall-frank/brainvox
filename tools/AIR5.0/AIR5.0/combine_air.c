/* Copyright 1995-2000 Roger P. Woods, M.D. */
/* Modified: 11/30/00 */

/*
 * This program will combine multiple sequential .air files
 *  to produce a single .air file that will have the same net
 *  spatial result.
 *
 * This avoids the need to repeatedly reslice a file to
 *  various locations, and also prevents the accumulation
 *  of interpolation errors.
 *
 * The program does some error checking to catch blatantly
 *  obvious incorrect usage.
 */

#include "AIRmain.h"

int main(int argc, char *argv[])

{
	if(argc<5){
		printf("\nUsage: %s output overwrite?(y/n) air_file1 air_file2 [... air_file_n]\n",argv[0]);
		printf("\n\treslice_file in each air_file should be spatially equivalent to standard_file in the air_file that follows it\n");
		printf("\n\toutput standard_file will be the same as that in air_file1\n");
		printf("\toutput reslice_file will be the same as that in air_file_n\n");
		exit(EXIT_FAILURE);
	}

	if(strstr(argv[1],AIR_CONFIG_IMG_SUFF)!=0||strstr(argv[1],AIR_CONFIG_HDR_SUFF)!=0){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("\nName of output .air file cannot contain %s or %s\n",AIR_CONFIG_IMG_SUFF,AIR_CONFIG_HDR_SUFF);
		exit(EXIT_FAILURE);
	}
	{
		AIR_Error errcode=AIR_do_combine_air(argv[0], (unsigned int)argc-3, argv+3, argv[1], argv[2][0]=='y');
		if(errcode!=0){
			AIR_report_error(errcode);
			exit(EXIT_FAILURE);
		}
	}
	return EXIT_SUCCESS;
}

