/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/25/01 */

/*
 * This program will set the global maximum in a header file
 * 8 bit files cannot have their global maximum adjusted
 */

#include "AIRmain.h"

int main(int argc, char *argv[])

{
	if (argc!=2 && argc!=3){
		printf("Usage: %s filename [max]\n",argv[0]);
		exit(EXIT_FAILURE);
	}
	{
		int dummy[8];
		struct AIR_Key_info dimensions;
		
		{
			struct AIR_Fptrs fps;
			
			(void)AIR_open_header(argv[1],&fps,&dimensions,dummy);
			if(fps.errcode!=0){
				AIR_Error errcode=fps.errcode;
				AIR_close_header(&fps);
				AIR_report_error(errcode);
				exit(EXIT_FAILURE);
			}
			AIR_close_header(&fps);
			if(fps.errcode!=0){
				AIR_report_error(fps.errcode);
				exit(EXIT_FAILURE);
			}
		}
		if (dimensions.bits!=16){
			printf("%s only modifies files with 16 bits/pixel; this file has %u bits/pixel\n",argv[0],dimensions.bits);
			exit(EXIT_FAILURE);
		}

		if(argc==2){

			printf("File types:\n");
			printf("\t1  16 bits/pixel unsigned short ints (not ANALYZE compatible)(0 to 65535)\n");;
			printf("\t2  16 bits/pixel short ints (0 to 32767)\n");
			printf("\t3  16 bits/pixel short ints (-32768 to +32767)\n");


			if(dummy[1]>65535){
				printf("this file's type cannot be evaluated\n");
				exit(EXIT_FAILURE);
			}
			if(dummy[1]>32767){
				if(dummy[0]<0){
					printf("this file's type cannot be evaluated\n");
					exit(EXIT_FAILURE);
				}
				else{
					printf("This is a type 1 file with a header global maximum of %i\n",dummy[1]);
					exit(EXIT_FAILURE);
				}
			}
			if(dummy[1]<0){
				if(dummy[0]<-32768 || dummy[0]>=0){
					printf("this file's type cannot be evaluated\n");
					exit(EXIT_FAILURE);
				}
				else{
					printf("This is a type 3 file with a header global maximum of %i\n",dummy[1]);
					exit(EXIT_FAILURE);
				}
			}
			if(dummy[0]<-32768){
				printf("this file's type cannot be evaluated\n");
				exit(EXIT_FAILURE);
			}
			if (dummy[0]<0){
				printf("This is a type 3 file with a header global maximum of %i\n",dummy[1]);
				exit(EXIT_FAILURE);
			}
			if (dummy[0]>32767){
				printf("this file's type cannot be evaluated\n");
				exit(EXIT_FAILURE);
			}
			printf("This is a type 2 file with a header global maximum of %i\n",dummy[1]);
			exit(EXIT_FAILURE);
		}
		{
			int value;
			{
				AIR_Error errcode;
				value=AIR_parse_int(argv[2],&errcode);
				if(errcode!=0){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("max must be an integer\n");
					exit(EXIT_FAILURE);
				}
			}
			if(value>65535){
				printf("value %i is out of possible range for all header types\n",value);
				exit(EXIT_FAILURE);
			}
			if(value>32767 && dummy[0]<0){
				printf("Type 3 files cannot have maxima greater than 32767\n");
				printf("Use makeaheader to create a type 1 header\n");
				printf("(Any values currently <0 will become values >32767 with a type 1 header)\n");
				printf("(True zero will be redefined as 0 with a type 1 header)\n");
				exit(EXIT_FAILURE);
			}
			if(value<0 && dummy[1]>32768){
				printf("Type 1 files cannot have negative maxima\n");
				printf("Use makeaheader to create a type 3 header\n");
				printf("(Any values currently >32767 will become negative numbers with a type 3 header)\n");
				printf("(True zero will be redefined as -32768 with a type 3 header)\n");
				exit(EXIT_FAILURE);
			}
			if(value<dummy[0]){
				printf("Type 2 files cannot have negative maxima\n");
				printf("Use makeaheader to create a type 3 header\n");
				printf("(True zero will be redefined as -32768 with a type 3 header)\n");
				exit(EXIT_FAILURE);
			}
			if(value<32768 && dummy[1]>32767){
				printf("WARNING: You have converted a type 1 file to a type 2 file\n");
				printf("If there are no type 1 values >32767, this conversion will have no effect\n");
				printf("However, if there are values >32767, they will be converted to negative numbers and ignored\n");
				printf("You can convert back to a type 1 file by resetting the global maximum to a number >32767\n");
			}
			if(value>32767 && dummy[1]<32768){
				printf("WARNING: You have converted a type 2 file to a type 1 file\n");
				printf("If there are no negative (undefined) type 2 values, this conversion will have no effect\n");
				printf("However, if there are negative values, they will be converted to positive numbers>32767\n");
				printf("You can convert back to a type 2 file by resetting the global maximum to a number <32768\n");
			}
			dummy[1]=value;
		}

		{
			AIR_Error errcode=AIR_write_header(argv[1],&dimensions,"units specified",dummy);
			if(errcode!=0){
				AIR_report_error(errcode);
				exit(EXIT_FAILURE);
			}
		}
	}
	return EXIT_SUCCESS;
}
