/* Copyright 1997-2001 Roger P. Woods, M.D. */
/* Modified: 5/24/01 */

/*
 * This program will change the file to be resliced in a .air
 *  file. The full path name is replaced			
 */

#include "AIRmain.h"

int main(int argc, char *argv[])

{
	/* argv[3] is deprecated 'verbose' */
	if (argc<3 || argc>4){
		printf("Usage: %s .air_file new_reslice_file\n",argv[0]);
		exit(EXIT_FAILURE);
	}
	{
		AIR_Error errcode=AIR_do_mv_air(argv[1], argv[2]);
		if(errcode!=0){
			AIR_report_error(errcode);
			exit(EXIT_FAILURE);
		}
	}
	return EXIT_SUCCESS;
}
