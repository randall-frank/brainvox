/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 12/8/01 */

/*
 * This program will invert a .air file. In other words
 *  if the input .air file provides instructions for reslicing
 *  file A to match file B, the output of this program
 *  will be a file with instructions for reslicing file B
 *  to match file A
 */

#include "AIRmain.h"

int main(int argc, char *argv[])

{
	AIR_Boolean ow=FALSE;
	
	if (argc<3||argc>4){
		printf("\nUsage: %s input output [overwrite?(y/n)]\n",argv[0]);
		exit(EXIT_FAILURE);
	}
	if(argc>3) ow=argv[3][0]=='y';

	if(strstr(argv[2],AIR_CONFIG_IMG_SUFF)!=0||strstr(argv[2],AIR_CONFIG_HDR_SUFF)!=0){
		printf("\nName of output .air file cannot contain %s or %s\n",AIR_CONFIG_IMG_SUFF,AIR_CONFIG_HDR_SUFF);
		exit(EXIT_FAILURE);
	}
	{
		AIR_Error errcode=AIR_do_invert_air(argv[0], argv[1], argv[2], ow);
		if(errcode!=0){
			AIR_report_error(errcode);
			exit(EXIT_FAILURE);
		}
	}
	return EXIT_SUCCESS;
}
