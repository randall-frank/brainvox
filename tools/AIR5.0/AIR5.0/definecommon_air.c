/* Copyright 1997-2001 Roger P. Woods, M.D. */
/* Modified: 5/24/01 */

/*
 * This defines a new average standard space that is the
 *  average of the reslice spaces of a set of .air files that
 *  share a common standard space.
 *
 * It does error checking for compatibility			
 */

#include "AIRmain.h"

int main(const int argc, char *argv[])

{
	if (argc<7){
		printf("Usage: %s input_prefix input_suffix output_prefix output_suffix overwrite?(y/n) id1 [..idN]\n",argv[0]);
		exit(EXIT_FAILURE);
	}
	{
		const char empty[1]={'\0'};
		const char *outprefix;
		const char *outsuffix;
		const char *inprefix;
		const char *insuffix;
		
		if(strcmp(argv[1],"null")==0) inprefix=empty;
		else inprefix=argv[1];

		if(strcmp(argv[2],"null")==0) insuffix=empty;
		else insuffix=argv[2];

		if(strcmp(argv[3],"null")==0) outprefix=empty;
		else outprefix=argv[3];

		if(strcmp(argv[4],"null")==0) outsuffix=empty;
		else outsuffix=argv[4];
		
		{
			AIR_Error errcode=AIR_do_definecommon_air(argv[0], inprefix, insuffix, outprefix, outsuffix, (unsigned int)argc-6, argv+6, argv[5][0]=='y');
			if(errcode!=0){
				AIR_report_error(errcode);
				exit(EXIT_FAILURE);
			}
		}
	}
	return EXIT_SUCCESS;
}
