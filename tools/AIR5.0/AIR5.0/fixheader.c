/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/28/01 */

/*
 * This program will change the size of a voxel as defined
 *  in the header file.
 */

#include "AIRmain.h"

int main(int argc, char *argv[])

{
	if (argc!=5 && argc!=2){
		printf("Usage: %s filename [x_size y_size z_size]\n",argv[0]);
		exit(EXIT_FAILURE);
	}
	{
		if(argc==2){
			AIR_Error errcode=AIR_do_fixheader(argv[1], TRUE, 0, 0, 0);
			if(errcode!=0){
				AIR_report_error(errcode);
				exit(EXIT_FAILURE);
			}
		}
		else{
			double x_size, y_size, z_size;
			{
				AIR_Error errcode;
				x_size=AIR_parse_double(argv[2],&errcode);
				if(errcode!=0 || x_size<=0.0){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("x_size must be a positive number\n");
					exit(EXIT_FAILURE);
				}
			}
			{
				AIR_Error errcode;
				y_size=AIR_parse_double(argv[3],&errcode);
				if(errcode!=0 || y_size<=0.0){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("y_size must be a positive number\n");
					exit(EXIT_FAILURE);
				}
			}
			{
				AIR_Error errcode;
				z_size=AIR_parse_double(argv[4],&errcode);
				if(errcode!=0 || z_size<=0.0){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("z_size must be a positive number\n");
					exit(EXIT_FAILURE);
				}
			}
			{
				AIR_Error errcode=AIR_do_fixheader(argv[1], FALSE, x_size, y_size, z_size);
				if(errcode!=0){
					AIR_report_error(errcode);
					exit(EXIT_FAILURE);
				}
			}
		}
	}
	return EXIT_SUCCESS;
}
