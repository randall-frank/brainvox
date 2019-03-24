/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 12/18/01 */

/*
 * This program will take an input image and create a corresponding
 *  output image that has been interpolated to cubic voxels.
 *
 * It is implemented to parallel the interpolation to cubic voxels
 *  that takes place with the reslice program.
 *
 */

#include "AIRmain.h"

int main(int argc, char *argv[])

{
	if (argc<3){
		printf("Usage: %s input output [options]\n",argv[0]);
		printf("\toptions:\n\n");
		printf("\t\t[-f output_air_file_name [overwrite?(y/n)]] (save a .air file describing the transformation)\n");
		printf("\t\t[-o] overwrite permission granted\n");
		printf("\t\t[-s] suppress image file generation\n");
		exit(EXIT_FAILURE);
	}
	{
		AIR_Boolean ow=FALSE;
		AIR_Boolean airow=FALSE;
		AIR_Boolean imageout=TRUE;
		
		char *air_file=NULL;
		{
			int n=3;
			while(argc>n){
				if(n==3 && argv[n][0]=='y'){		/* Maintain compatibility with AIR 3.x */
					n++;
					ow=TRUE;
				
					if(argc>n){
						printf("arguments to %s cannot intermix old and new style syntax\n",argv[0]);
						exit(EXIT_FAILURE);
					}
					continue;
				}
				else if(n==3 && argv[n][0]!='-'){	/* Maintain compatibility with AIR 3.x */
					n++;
					ow=0;
					if(argc>n){
						printf("arguments to %s cannot intermix old and new style syntax\n",argv[0]);
						exit(EXIT_FAILURE);
					}
					continue;
				}
				else if(argv[n][0]!='-'){
					printf("unable to parse argument %i which should have started with a '-'\n",n);
					exit(EXIT_FAILURE);
				}
				if(argv[n][1]=='o'){
					n++;
					ow=TRUE;
				}
				else if(argv[n][1]=='f'){
					n++;
					if(argc<=n){
						printf("argument -f should have been followed by a file name\n");
						exit(EXIT_FAILURE);
					}
					air_file=argv[n++];
					if(argc>n){
						if(argv[n][0]!='-'){
							if(argv[n][0]=='y') airow=TRUE;
							n++;
						}
					}
				}
				else if(argv[n][1]=='s'){
					n++;
					imageout=0;
				}
				else{
					printf("Cannot parse argument %s\n",argv[n]);
					exit(EXIT_FAILURE);
				}
			}
		}

		if(!imageout && !air_file){
			printf("If you use the -s flag with %s, you must use the -f flag\n",argv[0]);
			exit(EXIT_FAILURE);
		}
		
		{
			AIR_Error errcode=AIR_do_zoomer(argv[0], argv[1], argv[2], air_file, imageout, ow, airow);
			if(errcode!=0){
				AIR_report_error(errcode);
				exit(EXIT_FAILURE);
			}
		}
	}
	return EXIT_SUCCESS;
}
