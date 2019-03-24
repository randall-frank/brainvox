/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 12/15/01 */

/* 
 *This program displays the contents of a vector field file		
 */


#include "AIRmain.h"

int main(int argc, char *argv[])

{
	if (argc!=2){
		printf("Usage: %s vector_field_file\n",argv[0]);
		exit(EXIT_FAILURE);
	}
	{
		struct AIR_Field air1;
		float ****field3=NULL;
		float ***field2=NULL;
		{
			AIR_Error errcode=AIR_read_airfield(argv[1],&air1,&field3, &field2);
			if(errcode!=0){
				if(field3) AIR_free_4f(field3);
				if(field2) AIR_free_3f(field2);
				AIR_report_error(errcode);
				exit(EXIT_FAILURE);
			}
		}

		printf("\n");
		printf("standard file: %s\n",air1.s_file);
		printf("identifier: %010lu\n",air1.s_hash);
		printf("file dimensions: %u by %u by %u pixels (x,y,z)\n",air1.s.x_dim,air1.s.y_dim,air1.s.z_dim);
		printf("voxel dimensions: %e by %e by %e (x,y,z)\n",air1.s.x_size,air1.s.y_size,air1.s.z_size);
		printf("\n");
		printf("reslice file: %s\n",air1.r_file);
		printf("identifier: %010lu\n",air1.r_hash);
		printf("file dimensions: %u by %u by %u pixels (x,y,z)\n",air1.r.x_dim,air1.r.y_dim,air1.r.z_dim);
		printf("voxel dimensions: %e by %e by %e (x,y,z)\n",air1.r.x_size,air1.r.y_size,air1.r.z_size);
		printf("\n");
		printf("This program does not print the vectors contained in the vector field\n");

		printf("\nComment: %s\n",air1.comment);
		
		if(field3) AIR_free_4f(field3);
		if(field2) AIR_free_3f(field2);
	}
	return EXIT_SUCCESS;
}
