/* Copyright 1998-2002 Roger P. Woods, M.D. */
/* Modified: 1/16/02 */

/* 
 */

#include "AIRmain.h"


int main(int argc, char *argv[]){
	if(argc<4 || argc>5){
		printf("Usage: %s .air_file ucf_file output_ucf_file [ow?(y/n)]\n",argv[0]);
		exit(EXIT_FAILURE);
	}
	{
		char *airfile=argv[1];
		char *ucf_in=argv[2];
		char *ucf_out=argv[3];
		AIR_Boolean ow=FALSE;
		
		if(argc>4) ow=argv[4][0]=='y';
		
		{
			AIR_Error errcode=AIR_do_reslice_ucf(airfile, ucf_in, ucf_out, ow);
			if(errcode!=0){
				AIR_report_error(errcode);
				exit(EXIT_FAILURE);
			}
		}
	}
	return EXIT_SUCCESS;
}

