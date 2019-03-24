/* Copyright 1997-2001 Roger P. Woods, M.D. */
/* Modified: 5/23/01 */


/* This program will compute the hash value of a volume	*/


#include "AIRmain.h"

int main(int argc, char *argv[])

{
	if(argc!=2){
		printf("Usage: %s filename\n",argv[0]);
		exit(EXIT_FAILURE);
	}
	{
		AIR_Error errcode=AIR_do_identify(argv[1]);
		if(errcode!=0){
			AIR_report_error(errcode);
			exit(EXIT_FAILURE);
		}
	}
	return EXIT_SUCCESS;
}

