/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 12/17/01 */

/*
 * This program will resize the matrix of a file
 * shifting the data as specified in the process
 *
 */

#include "AIRmain.h"

int main(int argc, char *argv[])

{
	if (argc<4 || argc>6){
		printf("Usage: %s input pad output [overwrite?(y/n)] [.air_file_name]\n",argv[0]);
		exit(EXIT_FAILURE);
	}
	{
		unsigned int pad;
		{
			AIR_Error errcode;
			pad=AIR_parse_uint(argv[2],&errcode);
			if(errcode!=0){
				printf("%s:  %d: ",__FILE__,__LINE__);
				printf("pad must be a non-negative integer\n");
				exit(EXIT_FAILURE);
			}
		}
		{
			AIR_Boolean ow=FALSE;
			if(argc>4) ow=argv[4][0]=='y';
			{
				char *air_file=NULL;
				if(argc>5) air_file=argv[5];
	
				{
					AIR_Error errcode=AIR_do_crop(argv[0], argv[1], argv[3], air_file, pad, ow);
	
					if(errcode!=0){
						AIR_report_error(errcode);
						exit(EXIT_FAILURE);
					}
				}
			}
		}
	}
	return EXIT_SUCCESS;
}
