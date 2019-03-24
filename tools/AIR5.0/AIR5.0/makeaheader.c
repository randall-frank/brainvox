/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/24/01 */

/* This program will generate a header for an image file */

#include "AIRmain.h"

int main(int argc, char *argv[])

{
	if (argc!=9){
		printf("Usage: %s filename filetype x_dim y_dim z_dim x_size y_size z_size\n",argv[0]);
		printf("File types (just use the corresponding number):\n");
		printf("\t0  8 bits/pixel (0 to 255)\n");
		printf("\t1  16 bits/pixel unsigned short ints (not ANALYZE 2.0 compatible)(0 to 65535)\n");;
		printf("\t2  16 bits/pixel short ints (0 to 32767)\n");
		printf("\t3  16 bits/pixel short ints (-32768 to +32767)\n");
		exit(EXIT_FAILURE);
	}
	
	{
		unsigned int filetype;
		{
			AIR_Error errcode;
			filetype=AIR_parse_uint(argv[2],&errcode);
			if(errcode!=0 || filetype>3){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("filetype must be 0, 1, 2 or 3\n");
				exit(EXIT_FAILURE);
			}
		}
		
		{
			unsigned int x_dim, y_dim, z_dim;
			{
				AIR_Error errcode;
				x_dim=AIR_parse_uint(argv[3],&errcode);
				if(errcode!=0 || x_dim==0){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("x_dim must be a positive integer\n");
					exit(EXIT_FAILURE);
				}
			}
			{
				AIR_Error errcode;
				y_dim=AIR_parse_uint(argv[4],&errcode);
				if(errcode!=0 || y_dim==0){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("y_dim must be a positive integer\n");
					exit(EXIT_FAILURE);
				}
			}
			{
				AIR_Error errcode;
				z_dim=AIR_parse_uint(argv[5],&errcode);
				if(errcode!=0 || z_dim==0){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("z_dim must be a positive integer\n");
					exit(EXIT_FAILURE);
				}
			}
			{
				double x_size, y_size, z_size;
				{
					AIR_Error errcode;
					x_size=AIR_parse_double(argv[6],&errcode);
					if(errcode!=0 || x_size<=0.0){
						printf("%s: %d: ",__FILE__,__LINE__);
						printf("x_size must be a positive number\n");
						exit(EXIT_FAILURE);
					}
				}
				{
					AIR_Error errcode;
					y_size=AIR_parse_double(argv[7],&errcode);
					if(errcode!=0 || y_size<=0.0){
						printf("%s: %d: ",__FILE__,__LINE__);
						printf("y_size must be a positive number\n");
						exit(EXIT_FAILURE);
					}
				}
				{
					AIR_Error errcode;
					z_size=AIR_parse_double(argv[8],&errcode);
					if(errcode!=0 || z_size<=0.0){
						printf("%s: %d: ",__FILE__,__LINE__);
						printf("z_size must be a positive number\n");
						exit(EXIT_FAILURE);
					}
				}
				{
					AIR_Error errcode=AIR_do_makeaheader(argv[1], filetype, x_dim, y_dim, z_dim, x_size, y_size, z_size, TRUE);
					if(errcode!=0){
						AIR_report_error(errcode);
						exit(EXIT_FAILURE);
					}
				}
			}
		}
	}
	return EXIT_SUCCESS;
}
