/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 12/18/01 */

/*
 *
 * This routine is intended to provide orthogonal Fourier
 *  smoothing that will take masking into account.
 *
 * Areas that have a value of zero in the mask will have zero
 *  values on output, and these areas will not be smoothed into
 *  adjacent non-zero areas. Likewise, the zero-padding	used
 *  to protect against wraparound effects will not be smoothed
 *  into the image matrix areas.
 *
 */

#include "AIRmain.h"

int main(int argc, char *argv[])

{
	if(argc<6||argc>12){
		printf("Usage: %s input FWHM_x FWHM_y FWHM_z output [options]\n\n",argv[0]);
		printf("\toptions:\n");
		printf("\t[-m maskfile] (smoothing restricted to non-zero mask values)\n");
		printf("\t[-o] (grants overwrite permission)\n");
		printf("\t[-p] (prevents wraparound padding for periodic data)\n");
		printf("\t[-r] (prevents rescaling of data)\n");
		printf("\t[-s] (smears any wraparound padding into data)\n");
		printf("\t[-v] (suppress notification of magnitude of rescaling)\n");
		exit(EXIT_FAILURE);
	}
	{
		AIR_Boolean ow=FALSE;
		AIR_Boolean rescale=TRUE;
		AIR_Boolean smoothpad=FALSE;
		AIR_Boolean pad=TRUE;
		AIR_Boolean flag=FALSE;
		AIR_Boolean verbose=TRUE;

		float kx, ky, kz;
		
		char *maskfile=NULL;
		{
			AIR_Error errcode;
			kx=AIR_parse_float(argv[2],&errcode);
			if(errcode!=0 || kx<0.0){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("FWHM_x must be a nonnegative number\n");
				exit(EXIT_FAILURE);
			}
		}
		{
			AIR_Error errcode;
			ky=AIR_parse_float(argv[3],&errcode);
			if(errcode!=0 || ky<0.0){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("FWHM_y must be a nonnegative number\n");
				exit(EXIT_FAILURE);
			}
		}		
		{
			AIR_Error errcode;
			kz=AIR_parse_float(argv[4],&errcode);
			if(errcode!=0 || kz<0.0){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("FWHM_z must be a nonnegative number\n");
				exit(EXIT_FAILURE);
			}
		}
		
		/*Make sure - flag is not used at input file or output file position*/
		if(argv[1][0]=='-'||argv[5][0]=='-'){
			printf("the - flag cannot be used at the start of the input or output file name\n");
			exit(EXIT_FAILURE);
		}
		
		/*Parse arguments*/
		{
			int n=6;
			while(argc>n){
				if(argv[n][0]!='-' || argv[n][2]!='\0'){
					printf("unable to parse arguments, argument %i was expected to consist of a - followed by a single character\n",n);
					exit(EXIT_FAILURE);
				}
				if(argv[n][1]=='o'){
					n++;
					ow=FALSE;
#if(AIR_CONFIG_VERBOSITY!=0)
					printf("\toverwrite enabled\n");
#endif
				}
				else if(argv[n][1]=='r'){
					n++;
					rescale=FALSE;
#if(AIR_CONFIG_VERBOSITY!=0)
					printf("\tRescaling disabled\n");
#endif
				}
				else if(argv[n][1]=='p'){
					n++;
					pad=FALSE;
#if(AIR_CONFIG_VERBOSITY!=0)
					printf("\tPeriodic data: no zero padding\n");
#endif
				}
				else if(argv[n][1]=='s'){
					n++;
					smoothpad=TRUE;
#if(AIR_CONFIG_VERBOSITY!=0)
					printf("\tSmear zero padding (if present) into data\n");
#endif
				}
				else if(argv[n][1]=='v'){
					n++;
					verbose=FALSE;
#if(AIR_CONFIG_VERBOSITY!=0)
					printf("\tSupress reports of rescaling magnitude\n");
#endif
				}

				else if(argv[n][1]=='m'){
					n++;
					if(flag){
						printf("The -m flag should only be used once\n");
						exit(EXIT_FAILURE);
					}
					flag=TRUE;
					if(argc<=n){
						printf("A file name must follow -m\n");
						exit(EXIT_FAILURE);
					}
					if(argv[n][0]=='-'){
						printf("File name that follows -m cannot start with - \n");
						exit(EXIT_FAILURE);
					}
					maskfile=argv[n++];
					
#if(AIR_CONFIG_VERBOSITY!=0)
					printf("\tApply mask file %s\n",maskfile);
#endif
				}
				else{
					printf("Sorry, flag -%c is not defined for this program\n",argv[n][1]);
					exit(EXIT_FAILURE);
				}
			}
		}
		{
		
			AIR_Error errcode=AIR_do_gsmooth(argv[0], argv[5], argv[1], pad, smoothpad, kx, ky, kz, maskfile, rescale, verbose, ow);
			if(errcode!=0){
				AIR_report_error(errcode);
				exit(EXIT_FAILURE);
			}
		}
		return EXIT_SUCCESS;
	}
}

