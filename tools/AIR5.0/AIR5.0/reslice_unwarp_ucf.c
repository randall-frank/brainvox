/* Copyright 1998-2002 Roger P. Woods, M.D. */
/* Modified: 1/16/02 */

/* 
 */

#include "AIRmain.h"

int main(int argc, char *argv[]){
	
	if(argc<4){
		printf("Usage: %s .warp_file ucf_file output_ucf_file [options]\n",argv[0]);
		printf("\n\toptions:\n");
		printf("\t[-i] spatial inversion is deliberate\n");
		printf("\t[-o] (grants overwrite permission)\n");
		printf("\t[-s] silently omit unmappable points\n");
		exit(EXIT_FAILURE);
	}
	{
		char *warpfile=argv[1];
		char *ucf_in=argv[2];
		char *ucf_out=argv[3];
		AIR_Boolean ow=FALSE;
		AIR_Boolean strict=TRUE;
		int parity=1;
		
		char **current_arg=argv+4;
		char **end_of_args=argv+argc;

		while(current_arg<end_of_args){
		
			char *current_char=*current_arg++;
			
			if(*current_char++!='-'){
				current_arg--;
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("Unable to parse argument %s, which was expected to begin with a '-'\n", *current_arg++);
				exit(EXIT_FAILURE);
			}
			
			switch(*current_char++){
										
				case 'o':
					ow=TRUE;
					break;
					
				case 'i':
					parity=-1;
					break;
					
				case 's':
					strict=FALSE;
					break;

				default:
					current_arg--;
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("Unable to parse %s\n",*current_arg++);
					exit(EXIT_FAILURE);
			} /* end argument parsing switch statement */
		} /* end of argument parsing */
		{
			AIR_Error errcode=AIR_do_reslice_unwarp_ucf(warpfile, ucf_in, ucf_out, parity, strict, ow);
			if(errcode!=0){
				AIR_report_error(errcode);
				exit(EXIT_FAILURE);
			}
		}
	}
	return EXIT_SUCCESS;
}

