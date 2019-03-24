/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 5/25/01 */

/*
 * This program is designed to allow arbitrary reorientation
 *  of the axes of a file.
 */


#include "AIRmain.h"

int main(int argc, char *argv[])

{
	if (argc<3){
		printf("Usage: %s input output [options]\n",argv[0]);
		printf("\noptions:\n\n");
		printf("\t[o] (overwrite permission granted)\n");
		printf("\t[f output_air_file_name] (save a .air file describing the transformation)\n\n");
		printf("\t[x] (mirror along x axis)\n");
		printf("\t[y] (mirror along y axis)\n");
		printf("\t[z] (mirror along z axis)\n\n");
		printf("\t[xy] (rotate +90 degrees to interchange x and y axes)\n");
		printf("\t[yx] (rotate -90 degrees to interchange y and x axes)\n");
		printf("\t[xz] (rotate +90 degrees to interchange x and z axes)\n");
		printf("\t[zx] (rotate -90 degrees to interchange z and x axes)\n");
		printf("\t[yz] (rotate +90 degrees to interchange y and z axes)\n");
		printf("\t[zy] (rotate -90 degrees to interchange z and y axes)\n\n");
		printf("\t[xx] (rotate 180 degrees around x axis)\n");
		printf("\t[yy] (rotate 180 degrees around y axis)\n");
		printf("\t[zz] (rotate 180 degrees around z axis)\n");
		exit(EXIT_FAILURE);
	}
	{
		AIR_Boolean ow=FALSE;
		char *airfile=NULL;
		
		/*Check for use of reserved symbol as output file name*/
		if(argv[2][1]=='\0'){
			if(argv[2][0]=='o'||argv[2][0]=='f'||argv[2][0]=='x'||argv[2][0]=='y'||argv[2][0]=='z'){
				printf("output file cannot be named %s\n",argv[2]);
				exit(EXIT_FAILURE);
			}
		}
		else if(argv[2][2]=='\0'){
			if(argv[2][0]=='x'||argv[2][0]=='y'||argv[2][0]=='z'){
				if(argv[2][1]=='x'||argv[2][1]=='y'||argv[2][1]=='z'){
					printf("output file cannot be named %s\n",argv[2]);
					exit(EXIT_FAILURE);
				}
			}
		}
		{
			unsigned int index=0;
			unsigned int *transforms;
			{
				int n=3;
				
				transforms=(unsigned int *)malloc((argc-3)*sizeof(unsigned int));
				if(!transforms){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("memory allocation error\n");
					exit(EXIT_FAILURE);
				}
				
				while (argc>n){
					if(argv[n][0]=='o'){
						ow=TRUE;
					}
					else if(argv[n][0]=='f'){
						n++;
						if(n==argc){
							printf("%s: %d: ",__FILE__,__LINE__);
							printf("A file name for saving the .air file must follow argument 'f'\n");
							free(transforms);;
							exit(EXIT_FAILURE);
						}
						airfile=argv[n];
					}	
					else if(argv[n][0]=='-'){
						printf("Don't use minus sign (-) with this program\n");
						free(transforms);;
						exit(EXIT_FAILURE);
					}
					else if(argv[n][0]=='x'){
						if(argv[n][1]=='\0'){
							transforms[index++]=1;
						}
						else if(argv[n][1]=='x'){
							transforms[index++]=4;
						}
						else if(argv[n][1]=='y'){
							transforms[index++]=5;
						}
						else if(argv[n][1]=='z'){
							transforms[index++]=6;
						}
						else{
							printf("Cannot parse argument %s\n",argv[n]);
							free(transforms);
							exit(EXIT_FAILURE);
						}
					}
					else if(argv[n][0]=='y'){
						if(argv[n][1]=='\0'){
							transforms[index++]=2;
						}
						else if(argv[n][1]=='x'){
							transforms[index++]=7;
						}
						else if(argv[n][1]=='y'){
							transforms[index++]=8;
						}
						else if(argv[n][1]=='z'){
							transforms[index++]=9;
						}
						else{
							printf("Cannot parse argument %s\n",argv[n]);
							free(transforms);
							exit(EXIT_FAILURE);
						}
					}
					else if(argv[n][0]=='z'){
						if(argv[n][1]=='\0'){
							transforms[index++]=3;
						}
						else if(argv[n][1]=='x'){
							transforms[index++]=10;
						}
						else if(argv[n][1]=='y'){
							transforms[index++]=11;

						}
						else if(argv[n][1]=='z'){
							transforms[index++]=12;
						}
						else{
							printf("Cannot parse argument %s\n",argv[n]);
							free(transforms);
							exit(EXIT_FAILURE);
						}
					}
					else{
						printf("Cannot parse argument %s\n",argv[n]);
						free(transforms);
						exit(EXIT_FAILURE);
					}
					n++;
				}
			}
			{
				AIR_Error errcode=AIR_do_reorient(argv[0], argv[1], argv[2], index, transforms, airfile, ow);
				if(errcode!=0){
					AIR_report_error(errcode);
					free(transforms);
					exit(EXIT_FAILURE);
				}
			}
			free(transforms);
		}
	}
	return EXIT_SUCCESS;
}
