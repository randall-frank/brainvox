/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 12/8/01 */

/*
 * This program will create a single plane layout based on the input data.
 */


#include "AIRmain.h"

int main(int argc, char *argv[])

{
	if (argc<8 || argc>9){
		printf("Converts 3D volume to n by m layout\n");
		printf("Data can be oriented transversely, coronally, or sagitally\n");
		printf("Plane numbering begins with plane 1\n");
		printf("Usage: %s input orient(t/c/s) width height z_start z_step output [overwrite?(y/n)]\n",argv[0]);
		exit(EXIT_FAILURE);
	}
	{
		/*Note that input index is decremented by 1 to generate zero-offset value*/

		unsigned int width, height, z_start;
		signed int z_step;
		char orientation;
		
		{
			AIR_Error errcode;
			width=AIR_parse_uint(argv[3],&errcode);
			if(errcode!=0 || width==0){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("width must be a positive integer\n");
				exit(EXIT_FAILURE);
			}
		}
		{
			AIR_Error errcode;
			height=AIR_parse_uint(argv[4],&errcode);
			if(errcode!=0 || height==0){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("height must be a positive integer\n");
				exit(EXIT_FAILURE);
			}
		}
		{
			AIR_Error errcode;
			z_start=AIR_parse_uint(argv[5],&errcode);
			if(errcode!=0 || z_start==0){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("z_start must be a positive integer\n");
				exit(EXIT_FAILURE);
			}
			z_start--;
		}
		{
			AIR_Error errcode;
			z_step=AIR_parse_int(argv[6],&errcode);
			if(errcode!=0){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("z_step must be an integer\n");
				exit(EXIT_FAILURE);
			}
		}
		
		orientation=argv[2][0];
		if(orientation != 't' && orientation != 'c' && orientation != 's'){
			printf("%c is not a valid choice for orientation\n",orientation);
			exit(EXIT_FAILURE);
		}

		{
			AIR_Boolean ow=FALSE;
			if(argc>8) ow=argv[8][0]=='y';
			{
				AIR_Error errcode=AIR_do_layout(argv[0], argv[1], argv[7], orientation, width, height, z_start, z_step, ow);
				if(errcode!=0){
					AIR_report_error(errcode);
					exit(EXIT_FAILURE);
				}
			}
		}
	}
	return EXIT_SUCCESS;
}
