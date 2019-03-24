/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 11/12/01 */


/* This program will apply a binary mask to a data file	*/


#include "AIRmain.h"

int main(int argc, char *argv[])

{
	if (argc<4 || argc>5){
		printf("Usage: %s input maskfile output [overwrite?(y/n)]\n",argv[0]);
		exit(EXIT_FAILURE);
	}
	{
		AIR_Boolean ow=FALSE;
		
		if(argc>4) ow=argv[4][0]=='y';
		{
			AIR_Error errcode=AIR_do_binarymask(argv[0], argv[1], argv[2], argv[3], ow);
			if(errcode){
				AIR_report_error(errcode);
				exit(EXIT_FAILURE);
			}
		}
	}
	return EXIT_SUCCESS;
}
