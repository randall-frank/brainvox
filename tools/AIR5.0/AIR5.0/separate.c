/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 12/8/01 */

/*
 * This program will separate a 3D file into multiple 2D files
 *
 * Output files are numbered automatically
 *
 */

#include "AIRmain.h"

int main(int argc, char *argv[])

{
	if (argc<3 || argc>4){
		printf("Usage: %s input output [overwrite_permission(y/n)]\n",argv[0]);
		exit(EXIT_FAILURE);
	}
	{
		AIR_Boolean ow=FALSE;
		
		if(argc>3) ow=argv[3][0]=='y';

		{
			AIR_Error errcode=AIR_do_separate(argv[0], argv[1], argv[2], ow);
			if(errcode!=0){
				AIR_report_error(errcode);
				exit(EXIT_FAILURE);
			}
		}
	}
	return EXIT_SUCCESS;
}
