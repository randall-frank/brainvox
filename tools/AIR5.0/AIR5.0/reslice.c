/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 12/13/01 */

/* This program will reslice a volume using the information
 *  contained in the specified .air file.
 *
 * Trilinear interpolation is default, but nearest neighbor or sinc
 *  can be explicitly requested.
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
	if(argc<3){
		printf("Note: program interpolates output to create cubic voxels unless overridden by -k, -x, -y or -z options.\n");
		printf("Usage: %s reslice_parameter_file output [options]\n\n",argv[0]);
		printf("\toptions:\n");
		printf("\t[-a alternate_reslice_file]\n");
		printf("\t[-d divide_by_intensity_scale_factor]\n");
		printf("\t[-df divide_by_intensity_scale_factor_file]\n");
		printf("\t[-k] (keeps voxel dimensions same as standard file's)\n");
		printf("\t[-n model {x_half-window_width y_half-window_width z_half-window_width}]\n");
		printf("\t[-o] (grants overwrite permission)\n");
		printf("\t[-s multiplicative_intensity_scale_factor]\n");
		printf("\t[-sf multiplicative_intensity_scale_factor_file]\n");
		printf("\t[-x x_dim x_size [x_shift]]\n");
		printf("\t[-y y_dim y_size [y_shift]]\n");
		printf("\t[-z z_dim z_size [z_shift]]\n");
		
		printf("\n\tInterpolation models (trilinear is default interpolation model)\n");
		printf("\t\t0. nearest neighbor\n");
		printf("\t\t1. trilinear\n");
		printf("\n\t\t2. windowed sinc in original xy plane, linear along z\n");
		printf("\t\t3. windowed sinc in original xz plane. linear along y\n");
		printf("\t\t4. windowed sinc in original yz plane, linear along x\n");
		printf("\t\t5. 3D windowed sinc\n");
		printf("\n\t\t6. 3D windowed scanline sinc\n");
		printf("\t\t7. 3D unwindowed scanline sinc\n");
		printf("\n\t\t10. 3D scanline chirp-z\n");
		printf("\t\t11. scanline chirp-z in original xy plane, linear along z\n");
		printf("\t\t12. scanline chirp-z in original xz plane, linear along y\n");
		printf("\t\t13. scanline chirp-z in original yz plane, linear along x\n");
		
		printf("\n\tScanline models are only valid for moderate angles and don't allow perspective distortions\n");
		printf("\n\tWindowed models require appropriate half-window widths\n");
		exit(EXIT_FAILURE);
	}
	{
		char *program=argv[0];
		char *airfile=argv[1];
		char *outfile=argv[2];
		
		char *alternate_reslice_file=NULL;
		char *mult_scale_file=NULL;
		char *div_scale_file=NULL;
		AIR_Boolean ow=FALSE;		/*Overwrite permission flag*/
		unsigned int
			x_dimout=0,
			y_dimout=0,
			z_dimout=0;	
		float
			x_distance=0.0,	/*User specified x_size*/
			y_distance=0.0,	/*User specified y_size*/
			z_distance=0.0;	/*User specified z_size*/
		float
			x_shift=0.0,
			y_shift=0.0,
			z_shift=0.0;
		AIR_Boolean cubic=TRUE;		/*Flag for whether output voxels must be cubic*/
		unsigned int interp=1;		/*Interpolation model*/
		unsigned int window[]={0,0,0};
		float scale=1.0;
		
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
					
				case 'o':
					ow=TRUE;
					break;
					
				case 'k':
					cubic=FALSE;
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
					
				case 'x':
					if(current_arg+1>=end_of_args){
						printf("%s: %d: ",__FILE__,__LINE__);
						printf("-x must be followed by a positive integer and a positive number\n");
						exit(EXIT_FAILURE);
					}
					{
						AIR_Error errcode;
						x_dimout=AIR_parse_uint(*current_arg, &errcode);
						current_arg++;
						if(errcode!=0 || x_dimout==0){
							printf("%s: %d: ",__FILE__,__LINE__);
							printf("-x must be followed by a positive integer and a positive number\n");
							exit(EXIT_FAILURE);
						}
					}
					{
						AIR_Error errcode;
						x_distance=AIR_parse_float(*current_arg, &errcode);
						current_arg++;
						if(errcode!=0 || x_distance<=0.0){
							printf("%s: %d: ",__FILE__,__LINE__);
							printf("-x must be followed by a positive integer and a positive number\n");
							exit(EXIT_FAILURE);
						}
					}

					if(current_arg<end_of_args){
                                        
						AIR_Error errcode;
						x_shift=AIR_parse_float(*current_arg, &errcode);
						if(errcode!=0) x_shift=0.0;
						else current_arg++;
					}
					break;
				case 'y':
					if(current_arg+1>=end_of_args){
						printf("%s: %d: ",__FILE__,__LINE__);
						printf("-y must be followed by a positive integer and a positive number\n");
						exit(EXIT_FAILURE);
					}
					{
						AIR_Error errcode;
						y_dimout=AIR_parse_uint(*current_arg, &errcode);
						current_arg++;
						if(errcode!=0 || x_dimout==0){
							printf("%s: %d: ",__FILE__,__LINE__);
							printf("-y must be followed by a positive integer and a positive number\n");
							exit(EXIT_FAILURE);
						}
					}
					{
						AIR_Error errcode;
						y_distance=AIR_parse_float(*current_arg, &errcode);
						current_arg++;
						if(errcode!=0 || y_distance<=0.0){
							printf("%s: %d: ",__FILE__,__LINE__);
							printf("-y must be followed by a positive integer and a positive number\n");
							exit(EXIT_FAILURE);
						}
					}
					if(current_arg<end_of_args){
						AIR_Error errcode;
						y_shift=AIR_parse_float(*current_arg, &errcode);
						if(errcode!=0) y_shift=0.0;
						else current_arg++;
					}
					break;
				case 'z':
					if(current_arg+1>=end_of_args){
						printf("%s: %d: ",__FILE__,__LINE__);
						printf("-z must be followed by a positive integer and a positive number\n");
						exit(EXIT_FAILURE);
					}
					{
						AIR_Error errcode;
						z_dimout=AIR_parse_uint(*current_arg, &errcode);
						current_arg++;
						if(errcode!=0 || x_dimout==0){
							printf("%s: %d: ",__FILE__,__LINE__);
							printf("-z must be followed by a positive integer and a positive number\n");
							exit(EXIT_FAILURE);
						}
					}
					{
						AIR_Error errcode;
						z_distance=AIR_parse_float(*current_arg, &errcode);
						current_arg++;
						if(errcode!=0 || z_distance<=0.0){
							printf("%s: %d: ",__FILE__,__LINE__);
							printf("-z must be followed by a positive integer and a positive number\n");
							exit(EXIT_FAILURE);
						}
					}
					if(current_arg<end_of_args){
						AIR_Error errcode;
						z_shift=AIR_parse_float(*current_arg, &errcode);
						if(errcode!=0) z_shift=0.0;
						else current_arg++;
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
		
		if(x_dimout!=0 || y_dimout!=0 || z_dimout!=0){
			if(!cubic){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("-k flag cannot be used with the -x, -y or -z flag\n");
				exit(EXIT_FAILURE);
			}
		}

#if(AIR_CONFIG_VERBOSITY!=0)
		if(alternate_reslice_file){
			printf("Reslice %s instead of the original target\n",alternate_reslice_file);
		}
		if(ow){
			printf("Overwrite any existing file %s\n",outfile);
		}
		if(!cubic){
			printf("Reslice to match voxel sizes of standard file without interpolation to cubic voxels\n");
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
		if(x_dimout){
			printf("Reslice to generate file x-dimension of %i and voxel x-size of %f\n",x_dimout, x_distance);
			if(x_shift){
				printf("Shift along x by %f voxels\n",x_shift);
			}
		}
		if(y_dimout){
			printf("Reslice to generate file y-dimension of %i and voxel y-size of %f\n",y_dimout, y_distance);
			if(y_shift){
				printf("Shift along y by %f voxels\n",y_shift);
			}
		}
		if(z_dimout){
			printf("Reslice to generate file z-dimension of %i and voxel z-size of %f\n",z_dimout, z_distance);
			if(z_shift){
				printf("Shift along z by %f voxels\n",z_shift);
			}
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
				case 6:
					printf("3D windowed scanline sinc (%i by %i by %i)\n",2*window[0],2*window[1],2*window[2]);
					break;
				case 7:
					printf("3D unwindowed scanline sinc\n");
					break;
				case 10:
					printf("3D scanline chirp-z\n");
					break;
				case 11:
					printf("scanline chirp-z in original xy plane, linear along z\n");
					break;
				case 12:
					printf("scanline chirp-z in original xz plane, linear along y\n");
					break;
				case 13:
					printf("scanline chirp-z in original yz plane, linear along x\n");
			}
		}
#endif
		{
			AIR_Error errcode=AIR_do_reslice(program,airfile,outfile,alternate_reslice_file,interp,window,cubic,scale,mult_scale_file,div_scale_file,ow,x_dimout,x_distance,x_shift,y_dimout,y_distance,y_shift,z_dimout,z_distance,z_shift);
			if(errcode!=0){
				AIR_report_error(errcode);
				exit(EXIT_FAILURE);
			}
		}
	}
	return EXIT_SUCCESS;
}	
