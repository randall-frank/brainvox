/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 5/25/01 */

/*
* This program displays the information stored in the header file
 * that is relevant and accessible to AIR.				
*/

#include "AIRmain.h"

int main(int argc, char *argv[])

{
	if (argc<2 || argc>2){
		printf("Usage: %s filename\n",argv[0]);
		exit(EXIT_FAILURE);
	}
	{
		struct AIR_Key_info dimensions;
		int dummy[8];
		AIR_Error errcode=0;
		{
			struct AIR_Fptrs fps;
			(void)AIR_open_header(argv[1],&fps,&dimensions,dummy);
			if(fps.errcode!=0){
				errcode=fps.errcode;
				fps.errcode=0;
			}
			AIR_close_header(&fps);
			if(fps.errcode!=0){
				AIR_report_error(fps.errcode);
				exit(EXIT_FAILURE);
			}
		}
		if(errcode!=0){
			switch(errcode){
				case AIR_INFO_DIM_ERROR:
					printf("WARNING: Illegal dimension\n");
					break;
				case AIR_INFO_SIZE_ERROR:
					printf("WARNING: Illegal voxel size\n");
					break;
				default:
					AIR_report_error(errcode);
					exit(EXIT_FAILURE);
			}
		}	
		printf("bits/pixel=%u\n",dimensions.bits);
		printf("x_dim=%u\n",dimensions.x_dim);
		printf("y_dim=%u\n",dimensions.y_dim);
		printf("z_dim=%u\n",dimensions.z_dim);
		printf("x_size=%e\n",dimensions.x_size);
		printf("y_size=%e\n",dimensions.y_size);
		printf("z_size=%e\n",dimensions.z_size);
		if(dimensions.bits==16){
			printf("\nglobal maximum=%i\n",dummy[1]);
			printf("global minimum=%i\n",dummy[0]);
		}
		if(errcode!=0){
			printf("WARNING: Note illegal values\n");
		}
	}
	return EXIT_SUCCESS;
}
