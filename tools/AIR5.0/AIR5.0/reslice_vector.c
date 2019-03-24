/* Copyright 1997-2001 Roger P. Woods, M.D. */
/* Modified: 12/15/01 */

/*
 * This program reslices using a vector-field to define the target space
 *
 * The vector field can be combined with an optional warp file
 *
 */


#include "AIRmain.h"

int main(int argc, char *argv[])

{
	if (argc<3){
		printf("Usage: %s vector_field output [options]\n",argv[0]);
		printf("\t\toptions:\n");
		printf("\t\t[-a alternate_reslice_file]\n");
		printf("\t\t[-d divide_by_intensity_scale_factor]\n");
		printf("\t\t[-df divide_by_intensity_scale_factor_file]\n");
		printf("\t\t[-n menu_model_number [x_half_window_width y_half_window_width z_half_window_width]]\n");
		printf("\t\t[-o] (grants overwrite permission)\n");
		printf("\t\t[-s multiplicative_intensity_scale_factor]\n");
		printf("\t\t[-sf multiplicative_intensity_scale_factor_file]\n");
		printf("\t\t[-w .warp_file]\n");
		printf("\n");
		printf("\t\tInterpolation models (trilinear is default interpolation model)\n");
		printf("\t\t\t0. nearest neighbor\n");
		printf("\t\t\t1. trilinear\n");
		printf("\t\t\t2. windowed sinc in original xy plane, linear along z\n");
		printf("\t\t\t3. windowed sinc in original xz plane, linear along y\n");
		printf("\t\t\t4. windowed sinc in original yz plane, linear along x\n");
		printf("\t\t\t5. 3D windowed sinc\n");
		exit(EXIT_FAILURE);
	}
	{
		char 		*program=argv[0];
		char 		*vecfile=argv[1];
		char 		*outfile=argv[2];
		
		char 		*alternate_reslice_file=NULL;
		char		*warpfile=NULL;
		char		*mult_scale_file=NULL;
		char 		*div_scale_file=NULL;
		AIR_Boolean	ow=FALSE;		/*Overwrite permission flag*/	
		unsigned int interp=1;		/*Interpolation model*/
		unsigned int window[]={0,0,0};
		float		scale=1.0;		
		
		char 		**current_arg=argv+3;
		char 		**end_of_args=argv+argc;
		
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
					
				case 'w':
					if(current_arg>=end_of_args){
						printf("%s: %d: ",__FILE__,__LINE__);
						printf("-w must be followed by a .warp file name\n");
						exit(EXIT_FAILURE);
					}
					warpfile=*current_arg++;
					break;
					
				case 'o':
					ow=TRUE;
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
			AIR_Error errcode=AIR_do_reslice_vector(program,vecfile,outfile,alternate_reslice_file,warpfile,interp,window,scale,mult_scale_file,div_scale_file,ow);
			if(errcode!=0) AIR_report_error(errcode);
		}
	}
	return EXIT_SUCCESS;
}	
