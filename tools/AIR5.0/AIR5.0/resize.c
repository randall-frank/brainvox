/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 5/28/01 */

/*
 * This program will resize the matrix of a file
 * shifting the data as specified in the process
 *
 */

#include "AIRmain.h"

int main(int argc, char *argv[])

{
	if (argc<9 || argc>11){
		printf("Resizes 3D volume with offset\n");
		printf("Usage: %s input new_x_dim new_y_dim new_z_dim x_shift y_shift z_shift output [overwrite_permission(y/n)] [.air_file_name]\n",argv[0]);
		exit(EXIT_FAILURE);
	}
	{
		AIR_Boolean ow=FALSE;
		unsigned int new_x_dim, new_y_dim, new_z_dim;
		signed int x_shift, y_shift, z_shift;
		{
			AIR_Error errcode;
			new_x_dim=AIR_parse_uint(argv[2],&errcode);
			if(errcode!=0 || new_x_dim==0){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("new_x_dim must be a positive integer\n");
				exit(EXIT_FAILURE);
			}
		}
		{
			AIR_Error errcode;
			new_y_dim=AIR_parse_uint(argv[3],&errcode);
			if(errcode!=0 || new_y_dim==0){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("new_y_dim must be a positive integer\n");
				exit(EXIT_FAILURE);
			}
		}
		{
			AIR_Error errcode;
			new_z_dim=AIR_parse_uint(argv[4],&errcode);
			if(errcode!=0 || new_z_dim==0){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("new_z_dim must be a positive integer\n");
				exit(EXIT_FAILURE);
			}
		}
		{
			AIR_Error errcode;
			x_shift=AIR_parse_int(argv[5],&errcode);
			if(errcode!=0){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("x_shift must be an integer\n");
				exit(EXIT_FAILURE);
			}
		}
		{
			AIR_Error errcode;
			y_shift=AIR_parse_int(argv[6],&errcode);
			if(errcode!=0){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("y_shift must be an integer\n");
				exit(EXIT_FAILURE);
			}
		}
		{
			AIR_Error errcode;
			z_shift=AIR_parse_int(argv[7],&errcode);
			if(errcode!=0){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("z_shift must be an integer\n");
				exit(EXIT_FAILURE);
			}
		}
		if(argc>9) ow=argv[9][0]=='y';
		{
			char *air_file=NULL;
			if(argc>10) air_file=argv[10];

			{
				AIR_Error errcode=AIR_do_resize(argv[0], argv[1], argv[8], air_file, new_x_dim, new_y_dim, new_z_dim, x_shift, y_shift, z_shift, ow);
				if(errcode!=0){
					AIR_report_error(errcode);
					exit(EXIT_FAILURE);
				}
			}
		}
	}
	return EXIT_SUCCESS;
}
