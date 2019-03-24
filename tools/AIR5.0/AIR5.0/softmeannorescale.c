/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 5/25/01 */

#include "AIRmain.h"

int main(int argc, char *argv[])

{
	if (argc<5){
		printf("Usage: %s output overwrite?(y/n) scaling_suffix input_file1 [...input_fileN]\n",argv[0]);
		printf("Use 'null' for scaling_suffix if inputs are all to same scale and no scaling files are available\n");
		exit(EXIT_FAILURE);
	}
	{
		const char *suffix=NULL;
		
		if(strcmp(argv[3],"null")!=0) suffix=argv[3];
		
		{
			AIR_Error errcode=AIR_do_softmeannorescale(argv[0], argv[1], (unsigned int)argc-4, argv+4, suffix, argv[2][0]=='y');
			if(errcode!=0){
				AIR_report_error(errcode);
				exit(EXIT_FAILURE);
			}
		}
	}
	return EXIT_SUCCESS;
}

