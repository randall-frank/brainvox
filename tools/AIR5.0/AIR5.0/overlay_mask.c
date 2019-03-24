/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 5/25/01 */

/*
 * This program will set voxels at or above threshold in mask to the maximum possible value in the base
 */


#include "AIRmain.h"

int main(int argc, char *argv[])

{
	if (argc<4){
		printf("Usage: %s mask base output [options]\n",argv[0]);
		printf("\tOptions:\n");
		printf("\t\t[-o] overwrite permission granted\n");
		printf("\t\t[-t threshold] overlay voxels from mask that are greater than or equal to threshold\n");
		exit(EXIT_FAILURE);
	}
	{
		AIR_Boolean ow=FALSE;
		long int threshold=(AIR_CONFIG_IMPOSSIBLE_THRESHOLD);
		{
			int n=4;

			/* Parse arguments */
			while(argc>n){
				if(argv[n][0]!='-'){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("unable to parse arguments, expected argument %i to start with a '-'\n",n);
					exit(EXIT_FAILURE);
				}
				if(argv[n][1]=='o'){
					n++;
					ow=TRUE;
#if(AIR_CONFIG_VERBOSITY!=0)
					printf("\toverwrite enabled\n");
#endif
				}
				else if(argv[n][1]=='t'){
					n++;
					if(argc<n){
						printf("%s: %d: ",__FILE__,__LINE__);
						printf("A threshold value must follow -t\n");
						exit(EXIT_FAILURE);
					}
					threshold=atol(argv[n++]);
#if(AIR_CONFIG_VERBOSITY!=0)
					printf("Overlay values in %s that are greater than %le\n",overlay_file,threshold));
#endif
					/* Don't pass magic value to subroutine */
					if(threshold==((long int)AIR_CONFIG_IMPOSSIBLE_THRESHOLD)){
						threshold--;
					}
				}
				else{
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("Sorry, flag -%c is not defined for this program\n",argv[n][1]);
					exit(EXIT_FAILURE);
				}
			}
		}

		{
			AIR_Error errcode=AIR_do_overlay_mask(argv[0], argv[1], argv[2], argv[3], threshold, ow);
			if(errcode!=0){
				AIR_report_error(errcode);
				exit(EXIT_FAILURE);
			}
		}
	}
	return EXIT_SUCCESS;
}
