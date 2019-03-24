/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 5/24/01 */

/*
 * Uses Fourier interpolation to magnify files
 */


#include "AIRmain.h"

int main(int argc, char *argv[])

{
	if (argc!=7 && argc!=10){
		printf("Usage: %s input output overwrite?(y/n) magnify_x magnify_y magnify_z [shrink_x shrink_y shrink_z]\n",argv[0]);
		exit(EXIT_FAILURE);
	}

	{
		unsigned int bigx, bigy, bigz;
		unsigned int
			smallx=1,
			smally=1,
			smallz=1;
			
		{
			AIR_Error errcode;
			bigx=AIR_parse_uint(argv[4],&errcode);
			if(errcode!=0 || bigx==0){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("magnify_x must be a positive integer\n");
				exit(EXIT_FAILURE);
			}
		}
		{
			AIR_Error errcode;
			bigy=AIR_parse_uint(argv[5],&errcode);
			if(errcode!=0 || bigy==0){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("magnify_y must be a positive integer\n");
				exit(EXIT_FAILURE);
			}
		}
		{
			AIR_Error errcode;
			bigz=AIR_parse_uint(argv[6],&errcode);
			if(errcode!=0 || bigz==0){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("magnify_z must be a positive integer\n");
				exit(EXIT_FAILURE);
			}
		}

		if(argc>7){
		
			{
				AIR_Error errcode;
				smallx=AIR_parse_uint(argv[7],&errcode);
				if(errcode!=0 || smallx==0){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("shrink_x must be a positive integer\n");
					exit(EXIT_FAILURE);
				}
			}
			{
				AIR_Error errcode;
				smally=AIR_parse_uint(argv[8],&errcode);
				if(errcode!=0 || smally==0){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("shrink_y must be a positive integer\n");
					exit(EXIT_FAILURE);
				}
			}
			{
				AIR_Error errcode;
				smallz=AIR_parse_uint(argv[9],&errcode);
				if(errcode!=0 || smallz==0){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("shrink_z must be a positive integer\n");
					exit(EXIT_FAILURE);
				}
			}

			if(bigx<smallx || bigy<smally || bigz<smallz){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("Magnifications must be larger than corresponding shrinkages\n");
				exit(EXIT_FAILURE);
			}

		}
		{
			AIR_Error errcode=AIR_do_magnify(argv[0], argv[1], argv[2], bigx, bigy, bigz, smallx, smally, smallz, argv[3][0]=='y');
			if(errcode!=0){
				AIR_report_error(errcode);
				exit(EXIT_FAILURE);
			}
		}
	}
	return EXIT_SUCCESS;
}
