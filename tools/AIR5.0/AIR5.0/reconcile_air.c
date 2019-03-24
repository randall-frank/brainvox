/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 12/22/01 */

/*
 * This reconciles a net of .air files
 * It does error checking for compatibility
 */


#include "AIRmain.h"

int main(int argc, char *argv[])

{
	if (argc<10){
		printf("Usage: %s iterations prefix intron suffix output_prefix output_suffix overwrite?(y/n) id1 id2 [...idN]\n",argv[0]);
		exit(EXIT_FAILURE);
	}
	{
		if(strcmp(argv[2],"null")==0) argv[2][0]='\0';

		if(strcmp(argv[3],"null")==0) argv[3][0]='\0';

		if(strcmp(argv[4],"null")==0) argv[4][0]='\0';

		if(strcmp(argv[5],"null")==0) argv[5][0]='\0';

		if(strcmp(argv[6],"null")==0) argv[6][0]='\0';
		{
			unsigned int iterations;
			{
				AIR_Error errcode;
				iterations=AIR_parse_uint(argv[1],&errcode);
				if(errcode!=0){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("iterations must be a non-negative integer\n");
					exit(EXIT_FAILURE);
				}
			}			
			{
				AIR_Error errcode=AIR_do_reconcile_air(argv[0], iterations, argv[2], argv[3], argv[4], argv[5], argv[6], (unsigned int)argc-8, argv+8, argv[7][0]=='y');
				if(errcode!=0){
					AIR_report_error(errcode);
					exit(EXIT_FAILURE);
				}
			}
		}
	}
	return EXIT_SUCCESS;
}
