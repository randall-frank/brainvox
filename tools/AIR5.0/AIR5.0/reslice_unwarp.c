/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 12/15/01 */

/*
 * This program will reslice a volume using the information
 *  contained in the specified .air file.
 *
 * Trilinear interpolation is the default.
 *
 * The default is to interpolate the output to cubic voxels,
 *  and not to overwrite existing files.
 *
 * Defaults can be overridden and other options utilized by
 *  various flags.
 *
 * Flags can be displayed by typing the name of the program
 *  without additional arguments.
 */

#include "AIRmain.h"

int main(int argc, char *argv[])

{

	double accuracy=(double)1.0e-7;

	if(argc<3){
		printf("Note: program will not interpolate to create cubic voxels\n");
		printf("Usage: %s .warp_file output [options]\n\n",argv[0]);
		printf("\toptions:\n");
		printf("\t[-a alternate_reslice_file]\n");
		printf("\t[-c accuracy(%e)] (in voxels)\n",accuracy);
		printf("\t[-d divide_by_intensity_scale_factor]\n");
		printf("\t[-df divide_by_intensity_scale_factor_file]\n");
		printf("\t[-i] spatial inversion is deliberate\n");
		printf("\t[-l linear_estimate_air_or_warp_file]\n");
		printf("\t[-n menu_model_number {x_half_window_width y_half_window_width z_half_window_width}]\n");
		printf("\t[-o] (grants overwrite permission)\n");
		printf("\t[-s multiplicative_intensity_scale_factor]\n");
		printf("\t[-sf multiplicative_intensity_scale_factor_file]\n");
				
		printf("\n\tInterpolation models (trilinear is default interpolation model)\n");
		printf("\t\t0. nearest neighbor\n");
		printf("\t\t1. trilinear\n");
		printf("\t\t2. windowed sinc in original xy plane, linear along z\n");
		printf("\t\t3. windowed sinc in original xz plane. linear along y\n");
		printf("\t\t4. windowed sinc in original yz plane, linear along x\n");
		printf("\t\t5. 3D windowed sinc\n");
		exit(EXIT_FAILURE);
	}
	{
		char *program=argv[0];
		char *warpfile=argv[1];
		char *outfile=argv[2];
		
		char *alternate_reslice_file=NULL;
		char *linear_estimate_file=NULL;
		char *mult_scale_file=NULL;
		char *div_scale_file=NULL;
		AIR_Boolean ow=FALSE;
		unsigned int interp=1;
		unsigned int window[]={0,0,0};
		float scale=1.0;
		int parity=1;
		

		char **current_arg=argv+3;
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
			
				case 'a':
					if(current_arg>=end_of_args){
						printf("%s: %d: ",__FILE__,__LINE__);
						printf("-a must be followed by an image file name\n");
						exit(EXIT_FAILURE);
					}
					alternate_reslice_file=*current_arg++;
					break;
					
				case 'l':
					if(current_arg>=end_of_args){
						printf("%s: %d: ",__FILE__,__LINE__);
						printf("-l must be followed by an affine air file or linear warp file\n");
						exit(EXIT_FAILURE);
					}
					linear_estimate_file=*current_arg++;
					break;
					
				case 'o':
					ow=TRUE;
					break;
					
				case 'c':
					if(current_arg>=end_of_args){
						printf("%s: %d: ",__FILE__,__LINE__);
						printf("-c must be followed by a positive number\n");
						exit(EXIT_FAILURE);
					}
					{
						AIR_Error errcode;
						accuracy=AIR_parse_float(*current_arg, &errcode);
						current_arg++;
						if(errcode!=0 || accuracy<=0.0){
							printf("%s: %d: ",__FILE__,__LINE__);
							printf("-c must be followed by a positive number\n");
							exit(EXIT_FAILURE);
						}
					}
					break;
				case 'i':
					parity=-1;
					break;
					
				case 's':
					switch(*current_char++){
						case '\0':
							if(current_arg>=end_of_args){
								printf("%s: %d: ",__FILE__,__LINE__);
								printf("\n-s must be followed by a positive number\n");
								exit(EXIT_FAILURE);
							}
							{
								AIR_Error errcode;
								scale=AIR_parse_float(*current_arg, &errcode);
								current_arg++;
								if(errcode!=0 || scale<=0){
									printf("%s: %d: ",__FILE__,__LINE__);
									printf("-s must be followed by a positive number\n");
									exit(EXIT_FAILURE);
								}
							}
							break;
						case 'f':
							if(current_arg>=end_of_args){
								printf("%s: %d: ",__FILE__,__LINE__);
								printf("-sf must be followed by a file name\n");
								exit(EXIT_FAILURE);
							}
							mult_scale_file=*current_arg++;
							break;
						default:
							current_arg--;
							printf("%s: %d: ",__FILE__,__LINE__);
							printf("Unable to parse %s\n",*current_arg++);
							exit(EXIT_FAILURE);
					}
					break;
							
				case 'd':
					switch(*current_char++){
						case '\0':
							if(current_arg>=end_of_args){
								printf("%s: %d: ",__FILE__,__LINE__);
								printf("\n-d must be followed by a positive number\n");
								exit(EXIT_FAILURE);
							}
							{
								AIR_Error errcode;
								scale=AIR_parse_float(*current_arg, &errcode);
								current_arg++;
								if(errcode!=0 || scale<=0.0){
									printf("%s: %d: ",__FILE__,__LINE__);
									printf("-d must be followed by a positive number\n");
									exit(EXIT_FAILURE);
								}
							}
							scale=(float)1.0/scale;
							break;
						case 'f':
							if(current_arg>=end_of_args){
								printf("%s: %d: ",__FILE__,__LINE__);
								printf("\n-df must be followed by a file name\n");
								exit(EXIT_FAILURE);
							}
							div_scale_file=*current_arg++;
							break;
						default:
							current_arg--;
							printf("%s: %d: ",__FILE__,__LINE__);
							printf("Unable to parse %s\n",*current_arg++);
							exit(EXIT_FAILURE);
					}
					break;
					

				case 'n':
					if(current_arg>=end_of_args){
						printf("%s: %d: ",__FILE__,__LINE__);
						printf("-n must be followed by a valid interpolation model number\n");
						exit(EXIT_FAILURE);
					}
					{
						AIR_Error errcode;
						interp=AIR_parse_uint(*current_arg, &errcode);
						current_arg++;
						if(errcode!=0){
							printf("%s: %d: ",__FILE__,__LINE__);
							printf("-n must be followed by a valid interpolation model number\n");
							exit(EXIT_FAILURE);
						}
					}
					switch(interp){
						case 0:
						case 1:
						case 7:
						case 10:
						case 11:
						case 12:
						case 13:
						break;
						case 2:
						case 3:
						case 4:
							if(current_arg+1>=end_of_args){
								printf("%s: %d: ",__FILE__,__LINE__);
								printf("-n %u must be followed by two positive integers\n",interp);
								exit(EXIT_FAILURE);
							}
							{
								AIR_Error errcode;
								window[0]=AIR_parse_uint(*current_arg, &errcode);
								current_arg++;
								if(errcode!=0 || window[0]==0){
									printf("%s: %d: ",__FILE__,__LINE__);
									printf("-n %u must be followed by two positive integers\n", interp);
									exit(EXIT_FAILURE);
								}
							}
							{
								AIR_Error errcode;
								window[1]=AIR_parse_uint(*current_arg, &errcode);
								current_arg++;
								if(errcode!=0 || window[1]==0){
									printf("%s: %d: ",__FILE__,__LINE__);
									printf("-n %u must be followed by two positive integers\n", interp);
									exit(EXIT_FAILURE);
								}
							}
							break;
						case 5:
						case 6:
							if(current_arg+2>=end_of_args){
								printf("%s: %d: ",__FILE__,__LINE__);
								printf("-n %u must be followed by three positive integers\n",interp);
								exit(EXIT_FAILURE);
							}
							{
								AIR_Error errcode;
								window[0]=AIR_parse_uint(*current_arg, &errcode);
								current_arg++;
								if(errcode!=0 || window[0]==0){
									printf("%s: %d: ",__FILE__,__LINE__);
									printf("-n %u must be followed by three positive integers\n", interp);
									exit(EXIT_FAILURE);
								}
							}
							{
								AIR_Error errcode;
								window[1]=AIR_parse_uint(*current_arg, &errcode);
								current_arg++;
								if(errcode!=0 || window[1]==0){
									printf("%s: %d: ",__FILE__,__LINE__);
									printf("-n %u must be followed by three positive integers\n", interp);
									exit(EXIT_FAILURE);
								}
							}
							{
								AIR_Error errcode;
								window[2]=AIR_parse_uint(*current_arg, &errcode);
								current_arg++;
								if(errcode!=0 || window[2]==0){
									printf("%s: %d: ",__FILE__,__LINE__);
									printf("-n %u must be followed by three positive integers\n", interp);
									exit(EXIT_FAILURE);
								}
							}
							break;
						default:
							printf("%s: %d: ",__FILE__,__LINE__);
							printf("-n %u does not define a valid interpolation model\n",interp);
							exit(EXIT_FAILURE);
					}
					break;								
				
				default:
					current_arg--;
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("Unable to parse %s\n",*current_arg++);
					exit(EXIT_FAILURE);
			} /* end argument parsing switch statement */
		} /* end of argument parsing */
#if(AIR_CONFIG_VERBOSITY!=0)
		if(alternate_reslice_file){
			printf("Reslice %s instead of the original target\n",alternate_reslice_file);
		}
		if(ow){
			printf("Overwrite any existing file %s\n",outfile);
		}
		if(div_scale_file){
			printf("Obtain 'divide-by' intensity scaling factor from file %s\n",div_scale_file);
		}
		if(mult_scale_file){
			printf("Obtain 'multiply-by' intensity scaling factor from file %s\n",mult_scale_file);
		}
		if(scale!=1.0){
			printf("Multiply intensities by %e\n",scale);
		}
		if(interp!=1){
			printf("Interpolation model: ");
			switch(interp){
				case 0:
					printf("nearest neighbor\n");
					break;
				case 1:
					printf("trilinear\n");
					break;
				case 2:
					printf("windowed sinc (%i by %i) in original xy plane, linear along z\n",2*window[0],2*window[1]);
					break;
				case 3:
					printf("windowed sinc (%i by %i) in original xz plane, linear along y\n",2*window[0],2*window[1]);
					break;				
				case 4:
					printf("windowed sinc (%i by %i) in original yz plane, linear along x\n",2*window[0],2*window[1]);
					break;	
				case 5:
					printf("3D windowed sinc (%i by %i by %i)\n",2*window[0],2*window[1],2*window[2]);
					break;
			}
		}
		
#endif		

		{
			AIR_Error errcode=AIR_do_reslice_unwarp(program,outfile,alternate_reslice_file,warpfile,interp,window,scale,mult_scale_file,div_scale_file,parity,accuracy,linear_estimate_file,ow);

			if(errcode!=0){
				AIR_report_error(errcode);
				exit(EXIT_FAILURE);
			}
		}
	}
	return EXIT_SUCCESS;
}
