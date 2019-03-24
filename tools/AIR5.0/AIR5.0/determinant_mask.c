/* Copyright 1995-2000 Roger P. Woods, M.D. */
/* Modified: 5/24/01 */

/*
 * Creates an image of the positive determinant regions of a .warp file
 */

#include "AIRmain.h"

int main(int argc, char *argv[])

{
	AIR_Boolean ow=FALSE;		/*Overwrite permission flag*/

	if(argc<3 || argc>4){
		printf("Usage: %s .warp_file output [options]\n\n",argv[0]);
		printf("\toptions:\n");
		printf("\t[-o] (grants overwrite permission)\n");
		exit(EXIT_FAILURE);
	}

	/*Make sure - flag is not used as reslice_parameter file or output file position*/
	if(argv[1][0]=='-'||argv[2][0]=='-'){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("the - flag cannot be used at the start of the reslice_parameter_file or output file name\n");
		exit(EXIT_FAILURE);
	}

	/*Parse arguments*/
	{
		int n=3;
		while(argc>n){
			if(argv[n][0]!='-' || argv[n][2]!='\0'){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("unable to parse arguments, argument %i was expected to consist of a - followed by a single character\n",n);
				exit(EXIT_FAILURE);
			}
			if(argv[n][1]=='o'){
				n++;
				ow=TRUE;
#if(AIR_CONFIG_VERBOSITY!=0)
				printf("\toverwrite enabled\n");
#endif
			}
			else{
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("Cannot parse argument %s\n",argv[n]);
				exit(EXIT_FAILURE);
			}
		}
	}
	{
		AIR_Error errcode=AIR_do_determinant_mask(argv[0], argv[1], argv[2], ow);
		if(errcode!=0){
			AIR_report_error(errcode);
			exit(EXIT_FAILURE);
		}
	}
	return EXIT_SUCCESS;
}

