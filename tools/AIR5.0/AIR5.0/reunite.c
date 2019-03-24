/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 5/25/01 */

/*
 * This program will combine multiple 2D files into a single 3D file					
 */

#include "AIRmain.h"

int main(int argc, char *argv[])

{
	if (argc<4){
		printf("Reunites single planes into a volume\n");
		printf("Usage #1: %s output overwrite?(y/n) inputs\n",argv[0]);
		printf("Usage #2: %s output overwrite?(y/n) -f filelist_file\n",argv[0]);
		exit(EXIT_FAILURE);
	}
	{
		char **list=argv+3;
		unsigned int n=(unsigned int)argc-3;
		AIR_Boolean need_to_free=FALSE;
		if(strcmp(list[0],"-f")==0){
			if(argc<5){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("The -f flag must be followed by an ASCII file name\n");
				exit(EXIT_FAILURE);
			}
			{
				AIR_Error errcode;
				list=AIR_parse_file_list(argv[4],&n,&errcode);
				if(errcode!=0){
					AIR_report_error(errcode);
					exit(EXIT_FAILURE);
				}
			}
			need_to_free=TRUE;
		}
		{
			AIR_Error errcode=AIR_do_reunite(argv[0], argv[1], n, list, argv[2][0]=='y');
			if(errcode!=0){
				if(need_to_free) AIR_free_2c(list);
				AIR_report_error(errcode);
				exit(EXIT_FAILURE);
			}
		}
		if(need_to_free) AIR_free_2c(list);
	}
	return EXIT_SUCCESS;
}
