/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 12/15/01 */

/* 
 *This program displays the contents of a .warp file		
 */


#include "AIRmain.h"

int main(int argc, char *argv[])

{
	if (argc!=2){
		printf("Usage: %s .warp_file\n",argv[0]);
		exit(EXIT_FAILURE);
	}
	{
		struct AIR_Warp air1;
		double **e;
		{
			AIR_Error errcode;
			
			e=AIR_read_airw(argv[1],&air1,&errcode);
			if(!e){
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

		printf("Equations refer to voxel coordinates\n");
		{
			unsigned int xs=0;
			unsigned int ys=0;
			unsigned int zs=0;

			unsigned int order=0;
			unsigned int i=0;

			if(air1.coord==3){
				printf("x,y, and z are coordinates in standard file\n");
				printf("x',y', and z' are coordinates in reslice file\n");
				printf("\tx'=\t\ty'=\t\tz'=\n");
				printf("\t%e\t%e\t%e\n",e[0][i],e[1][i],e[2][i]);

				while(order<air1.order){
					order++;
					xs=order+1;
					ys=0;
					zs=0;

					while(zs<=order){
						for(;xs--!=0;ys++){
							i++;
							if(e[0][i]!=0.0||e[1][i]!=0.0||e[2][i]!=0.0){
								printf("+\t%e\t%e\t%e\t",e[0][i],e[1][i],e[2][i]);
								if(xs!=0){
									printf("x");
									if(xs>1) printf("^%u",xs);
									if(xs!=order) printf("*");
								}
								if(ys!=0){
									printf("y");
									if(ys>1) printf("^%u",ys);
									if(zs!=0) printf("*");
								}
								if(zs!=0){
									printf("z");
									if(zs>1) printf("^%u",zs);
								}
								printf("\n");
							}
						}
						zs++;
						xs=order-zs+1;
						ys=0;
					}
				}
			}
			else if(air1.coord==2){
				printf("x and y are coordinates in standard file\n");
				printf("x' and y' are coordinates in reslice file\n");
				printf("\tx'=\t\ty'=\n");
				printf("\t%e\t%e\n",e[0][i],e[1][i]);

				while(order<air1.order){
					order++;
					xs=order+1;
					ys=0;

					for(;xs--!=0;ys++){
						i++;
						if(e[0][i]!=0.0||e[1][i]!=0.0){
							printf("+\t%e\t%e\t",e[0][i],e[1][i]);
							if(xs!=0){
								printf("x");
								if(xs>1) printf("^%u",xs);
								if(xs!=order) printf("*");
							}
							if(ys!=0){
								printf("y");
								if(ys>1) printf("^%u",ys);
							}
							printf("\n");
						}
					}
				}
			}
			else{
				printf("file %s claims to have %u coordinates--cannot parse this number of coordinates\n",argv[1],air1.coord);
			}
		}
		printf("\nComment: %s\n",air1.comment);
		AIR_free_2(e);
	}
	return EXIT_SUCCESS;
}
