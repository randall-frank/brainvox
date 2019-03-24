/* Copyright 1996-2001 Roger P. Woods, M.D. */
/* Modified: 6/1/01 */

/* 
 *This program displays the contents of a .air file		
 */


#include "AIRmain.h"

int main(int argc, char *argv[])

{
	if (argc<2 || argc>3){
		printf("Usage: %s filename [options]\n",argv[0]);
		printf("options:\n");
		printf("\t-l (Matrix logarithm of real world transformation matrix)\n");
		printf("\t-r (Real world transformation matrix in millimeters)\n");
		printf("\t-v (Voxel based transformation matrix)\n");
		exit(EXIT_FAILURE);
	}
	{
		struct AIR_Air16 air1;
		{
			AIR_Error errcode=AIR_read_air16(argv[1],&air1);
			
			if(errcode!=0){
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
		{
			double *e[4];
			
			e[0]=air1.e[0];
			e[1]=air1.e[1];
			e[2]=air1.e[2];
			e[3]=air1.e[3];

			{
				double pixel_size_s=air1.s.x_size;
				if(air1.s.y_size<pixel_size_s) pixel_size_s=air1.s.y_size;
				if(air1.s.z_size<pixel_size_s) pixel_size_s=air1.s.z_size;

				if(argc==2){
					printf("E*cubic standard coordinates=reslice coordinates\n");
					printf("E=\n");
				}
				else if(argv[2][0]=='-' && argv[2][1]=='l'){
					{
						unsigned int j;
						
						for(j=0;j<3;j++){	/*Note: j<3 instead of j<4 is not an error*/
						
							unsigned int i;
							
							for(i=0;i<4;i++){
								e[j][i]/=pixel_size_s;
							}
						}
					}
					{
						unsigned int j;
						
						for(j=0;j<4;j++){
							e[j][0]*=air1.r.x_size;
							e[j][1]*=air1.r.y_size;
							e[j][2]*=air1.r.z_size;
						}
					}
					{
						double toproot[8];
						double botroot[8];
						{
							AIR_Error errcode=AIR_padetanh(&toproot[0],&botroot[0]);
							if(errcode!=0){
								AIR_report_error(errcode);
								exit(EXIT_FAILURE);
							}
						}
			
						{
							double ***storage2=AIR_matrix3(2,2,20);
					
							if(!storage2){
								AIR_report_error(AIR_MEMORY_ALLOCATION_ERROR);
								exit(EXIT_FAILURE);
							}
							{
								double ***storage4=AIR_matrix3(4,4,10);
								
								if(!storage4){
									AIR_free_3(storage2);
									AIR_report_error(AIR_MEMORY_ALLOCATION_ERROR);
									exit(EXIT_FAILURE);
								}
								{
									AIR_Error errcode=AIR_eloger4(4,e,toproot,botroot,storage2,storage4);
									if(errcode!=0){
										AIR_free_3(storage2);
										AIR_free_3(storage4);
										AIR_report_error(errcode);
										exit(EXIT_FAILURE);
									}
								}
								AIR_free_3(storage4);
							}
							AIR_free_3(storage2);
						}
					}
					printf("expm(R)*standard spatial location=reslice spatial location\n");
					printf("R=\n");
				}
				else if(argv[2][0]=='-' && argv[2][1]=='r'){
					{
						unsigned int j;
						
						for(j=0;j<3;j++){	/*Note: j<3 instead of j<4 is not an error*/
						
							unsigned int i;
							
							for(i=0;i<4;i++){
								e[j][i]/=pixel_size_s;
							}
						}
					}
					{
						unsigned int j;
						
						for(j=0;j<4;j++){
							e[j][0]*=air1.r.x_size;
							e[j][1]*=air1.r.y_size;
							e[j][2]*=air1.r.z_size;
						}
					}
					printf("R*standard spatial location=reslice spatial location\n");
					printf("R=\n");
				}
				else if(argv[2][0]=='-' && argv[2][1]=='v'){
					{
						unsigned int i;
						
						for(i=0;i<4;i++){
							e[0][i]*=(air1.s.x_size/pixel_size_s);
							e[1][i]*=(air1.s.y_size/pixel_size_s);
							e[2][i]*=(air1.s.z_size/pixel_size_s);
						}
					}
					printf("V*standard coordinates=reslice coordinates\n");
					printf("V=\n");
				}
				else{
					printf("Can't parse argument %s\n",argv[2]);
					exit(EXIT_FAILURE);
				}
			}
			printf("[%e %e %e %e\n",e[0][0],e[1][0],e[2][0],e[3][0]);
			printf("%e %e %e %e\n",e[0][1],e[1][1],e[2][1],e[3][1]);
			printf("%e %e %e %e\n",e[0][2],e[1][2],e[2][2],e[3][2]);
			printf("%e %e %e %e]\n",e[0][3],e[1][3],e[2][3],e[3][3]);
			printf("\nComment: %s\n",air1.comment);
		}
	}
	return EXIT_SUCCESS;
}
