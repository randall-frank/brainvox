/* Copyright 1995-2000 Roger P. Woods, M.D. */
/* Modified: 12/8/01 */

/* 
 * This program will perform binary operations on two binary files 	
 */

#include "AIRmain.h"

int main(int argc, char *argv[])

{
	if (argc<5 || argc>6){
		printf("Usage: %s binary_file_1 operation binary_file_2 output [overwrite?(y/n)]\n",argv[0]);
		printf("\t\toperation options:\n\t\t\tintersect\n\t\t\tadd\n\t\t\tcontrast\n\t\t\tsubtract\n\t\t\tinvert (binary_file_2 is ignored but must be specified)\n");
		exit(EXIT_FAILURE);
	}
	if(strcmp(argv[2],"intersect")!=0){
		if(strcmp(argv[2],"add")!=0){
			if(strcmp(argv[2],"contrast")!=0){
				if(strcmp(argv[2],"subtract")!=0){
					if(strcmp(argv[2],"invert")!=0){
						printf("Cannot understand operation %s\n",argv[2]);
						exit(EXIT_FAILURE);
					}
				}
			}
		}
	}

	{
		AIR_Boolean ow=FALSE;
		if(argc>5) ow=argv[5][0]=='y';
		{
			AIR_Error errcode=AIR_do_binarymath(argv[0], argv[1], argv[2], argv[3], argv[4], ow);
			if(errcode!=0){
				AIR_report_error(errcode);
				exit(EXIT_FAILURE);
			}
		}
	}
	
	return EXIT_SUCCESS;
}
