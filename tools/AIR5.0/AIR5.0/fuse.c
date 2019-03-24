/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 12/18/01 */

/*
 * This program will create multiplane layouts that display slices from
 * multiple files (with identical dimensions) simultaneously side-by-side.
 */

#include "AIRmain.h"

int main(int argc, char *argv[])

{
	if (argc<6){
		printf("Usage: %s output overwrite?(y/n) width height input_file1 [...input_fileN]\n",argv[0]);
		printf("\tuse 'null' as an input_file to control blank space\n");
		exit(EXIT_FAILURE);
	}
	{
		unsigned int width;
		{
			AIR_Error errcode;
			width=AIR_parse_uint(argv[3],&errcode);
			if(errcode!=0){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("width must be a positive integer, not %s\n",argv[3]);
				AIR_report_error(errcode);
				exit(EXIT_FAILURE);
			}
		}
		if(width==0){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("width cannot be zero\n");
			exit(EXIT_FAILURE);
		}
		{
			unsigned int height;
			{
				AIR_Error errcode;
				height=AIR_parse_uint(argv[4],&errcode);
				if(errcode!=0){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("height must be a positive integer, not %s\n",argv[4]);
					AIR_report_error(errcode);
					exit(EXIT_FAILURE);
				}
			}
			if(height==0){
				printf("height cannot be zero\n");
				exit(EXIT_FAILURE);
			}
			/* Convert any "null" to '\0' */
			{
				int k;
				
				for(k=5;k<argc;k++){
					if(strcmp(argv[k],"null")==0){
						argv[k][0]='\0';
					}
				}
					
			}
			{
				AIR_Error errcode=AIR_do_fuse(argv[0], argv[1], width, height, (unsigned int)argc-5, argv+5, argv[2][0]=='y');
				if(errcode!=0){
					AIR_report_error(errcode);
					exit(EXIT_FAILURE);
				}
			}
		}
	}
	return EXIT_SUCCESS;
}
