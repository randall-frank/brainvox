/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 12/4/01 */

/*
 * This program will change the directory path of the file
 *  that is designated to be resliced in a .air file.
 *
 * The name of the file itself will not be changed
 */

#include "AIRmain.h"

int main(int argc, char *argv[])

{
	if (argc<3 || argc>4){
		printf("Usage: %s .air_file new_directory [foreign_path_separator]\n",argv[0]);
		exit(EXIT_FAILURE);
	}
	{
		const char *foreign_path_separator=NULL;
		if(argc>3) foreign_path_separator=argv[3];
		{
			AIR_Error errcode=AIR_do_cd_air(argv[1], argv[2],foreign_path_separator);
			if(errcode!=0){
				AIR_report_error(errcode);
				exit(EXIT_FAILURE);
			}
		}
	}
	return EXIT_SUCCESS;
}
