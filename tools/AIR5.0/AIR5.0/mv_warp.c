/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 5/25/01 */

/*
 * This program will change the file to be resliced in a .wrp
 *  file. The full path name is replaced			
 */

#include "AIRmain.h"

int main(int argc, char *argv[])

{
	/* Deprecated argv[3] 'verbose' */
	if (argc<3 || argc>4){
		printf("Usage: %s .warp_file new_reslice_file\n",argv[0]);
		exit(EXIT_FAILURE);
	}
	{
		AIR_Error errcode=AIR_do_mv_warp(argv[1], argv[2]);
		if(errcode!=0){
			AIR_report_error(errcode);
		}
	}
	return EXIT_SUCCESS;
}
