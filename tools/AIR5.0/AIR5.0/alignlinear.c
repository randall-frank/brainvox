/* Copyright 1995-2002 Roger P. Woods, M.D. */
/* Modified  1/16/02  */

/* 	This program will apply a linear registration model to align two studies.
 *	It incorporates the essential elements of all prior linear registration
 *	programs (e.g., alignpettopet, alignmritopet, etc.)
 */


#include "AIRmain.h"

#define SF 81			/*Default initial sampling interval*/
#define SF2 1			/*Default final sampling interval*/
#define SFF 3			/*Default sampling interval decrement ratio*/
#define CONVERGE .00001		/*Default convergence criteria*/
#define ITERATE 25		/*Default iterations before forced convergence*/
#define NOPROGTRY 5		/*Default iterations without improved sd to force convergence*/
#define ZOOM FALSE			/* FALSE turns off zooming*/

#define PARTITIONS1 1 		/*number of separate partitions (usually 1 or 256)*/
				/*use 1 for pet to pet, use 256 for mri to pet*/
#define PARTITIONS2 1
#define DYNAMIC1 TRUE		/*Default (TRUE) is to partition dynamically to the max_actual_value*/
				/* Static partitioning is based on MAX_POSS_VALUE */
#define DYNAMIC2 TRUE
#define LOCALVERBOSITY 0	/*Governs printing of interim values on the screen*/
#define COSTFXN 1			/* Default cost function */
#define INTERACTION TRUE		/* FALSE assumes non-interaction of spatial parameters */
#define POSITIVE_DEFINITE_REQUIRED TRUE


int main(int argc, char *argv[])

{
	long int threshold1=(AIR_CONFIG_THRESHOLD1);		/*ignore all pixels in the standard file less than this*/
	long int threshold2=(AIR_CONFIG_THRESHOLD2);		/*ignore all pixels in the reslice file less than this*/
	unsigned int samplefactor=SF;	/*initial sampling interval*/
	unsigned int samplefactor2=SF2;	/*final sampling interval*/
	unsigned int sffactor=SFF;		/*sampling interval decrement ratio*/
	AIR_Boolean zooming=ZOOM;		/*0 turns off zooming, 1 turns on zooming*/

	unsigned int model=0;		/*menu identified model for registration*/
	unsigned int partitions1=PARTITIONS1;	/*number of partitions used to segment standard file*/
	unsigned int partitions2=PARTITIONS2;	/*number of partitions used to segment reslice file*/
	AIR_Boolean dynamic1=DYNAMIC1;		/*dynamic or static partitioning of standard file*/
	AIR_Boolean dynamic2=DYNAMIC2;		/*dynamic or static partitioning of reslice file*/
	AIR_Boolean verbosity=LOCALVERBOSITY;		/*Controls printing of interim information*/

	float precision=CONVERGE;	/*value that must exceed predicted cost function change for convergence*/
	unsigned int iterations=ITERATE;		/*number of iterations before forced termination*/
	unsigned int noprogtries=NOPROGTRY;	/*number of iterations without sd improvement before forced termination*/
	
	unsigned int	costfxn=COSTFXN;
	AIR_Boolean posdefreq=POSITIVE_DEFINITE_REQUIRED;
	
	float kx1=0.0;
	float kx2=0.0;
	float ky1=0.0;
	float ky2=0.0;
	float kz1=0.0;
	float kz2=0.0;

	AIR_Boolean interaction=INTERACTION;

	if (argc<6){
		printf("\nUsage:%s standard_file reslice_file air_out -m model_menu_number [options]\n",argv[0]);
		printf("\tModel Menu:\n");
		printf("\t\t3-D models:\n");
		printf("\t\t\t 6. rigid body 6 parameter model\n");
		printf("\t\t\t 7. global rescale 7 parameter model\n");
		printf("\t\t\t 9. traditional 9 parameter model (std must be on AC-PC line)\n");
		printf("\t\t\t12. affine 12 parameter model\n");
		printf("\t\t\t15. perspective 15 parameter model\n");
		printf("\n\t\t2-D models (constrained to in-plane, no interpolation):\n");
		printf("\t\t\t23. 2-D rigid body 3 parameter model\n");
		printf("\t\t\t24. 2-D global rescale 4 parameter model\n");
		printf("\t\t\t25. 2-D affine/fixed determinant 5 parameter model\n");
		printf("\t\t\t26. 2-D affine 6 parameter model\n");
		printf("\t\t\t28. 2-D perspective 8 parameter model\n");
		printf("\n");
		printf("\n\toptions: (defaults in parentheses)\n");
		printf("\t[-b1 FWHM_x(%e) FWHM_y(%e) FWHM_z(%e)] (standard file)\n",kx1,ky1,kz1);
		printf("\t[-b2 FWHM_x(%e) FWHM_y(%e) FWHM_z(%e)] (reslice file)\n",kx2,ky2,kz2);
		printf("\t[-c convergence_threshold(%f)]\n",precision);
		printf("\t[-d] use static partitioning (like AIR3.0 and earlier)\n");
		printf("\t[-e1 standard_file_mask]\n");
		printf("\t[-e2 reslice_file_mask]\n");
		printf("\t[-f initialization_file]\n");
		printf("\t[-fs scaling_initialization_file]\n");
		printf("\t[-g termination_file [overwrite?(y/n)]]\n");
		printf("\t[-gs scaling_termination_file [overwrite?(y/n)]\n");
		printf("\t[-h halt_after_(%u)_iterations_without_improvement]\n",noprogtries);
		printf("\t[-j] use non-positive definite Hessian matrices (not validated, use at your own risk)\n");
		printf("\t[-p1 segment_standard_file_into_(%u)_partitions]\n",partitions1);
		printf("\t[-p2 segment_reslice_file_into_(%u)_partitions]\n",partitions2);		
		printf("\t[-q] assume non-interaction of spatial parameter derivatives\n");
		printf("\t[-r repeated_iterations(%u)]\n",iterations);
		printf("\t[-s initial_sampling(%u) final_sampling(%u) sampling_decrement_ratio(%u)]\n",samplefactor,samplefactor2,sffactor);
		printf("\t[-t1 threshold_standard_file(%li)]\n",threshold1);
		printf("\t[-t2 threshold_reslice_file(%li)]\n",threshold2);
		printf("\t[-v] verbose reporting of interim results\n");
		printf("\t[-x cost_function(%u)]\n",costfxn);
		printf("\t\t1. standard deviation of ratio image\n");
		printf("\t\t2. least squares\n");
		printf("\t\t3. least squares with intensity rescaling\n");
		printf("\t[-z] (no additional argument; turns on pre-alignment interpolation)\n");

		exit(EXIT_FAILURE);
	}
	
	{
		char *program=argv[0];
		char *standard_file=argv[1];
		char *reslice_file=argv[2];
		char *air_file=argv[3];
		
		char *standard_file_mask=NULL;
		char *reslice_file_mask=NULL;
		
		char *initialization_file=NULL;
		char *termination_file=NULL;
		
		char *scaling_initialization_file=NULL;
		char *scaling_termination_file=NULL;
		
		AIR_Boolean overwrite_termination_file=FALSE;
		AIR_Boolean overwrite_scaling_termination_file=FALSE;
		
		/* Make sure .air file does not end with header or image suffix */
		if(strstr(argv[3],AIR_CONFIG_IMG_SUFF)!=0||strstr(argv[3],AIR_CONFIG_HDR_SUFF)!=0){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("\nName of output .air file cannot contain %s or %s\n",AIR_CONFIG_IMG_SUFF,AIR_CONFIG_HDR_SUFF);
			exit(EXIT_FAILURE);
		}	
		
		{
			char **current_arg=argv+4;
			char **end_of_args=argv+argc;
			
			while(current_arg<end_of_args){
			
				char *current_char=*current_arg++;
			
				if(*current_char++!='-'){
					current_arg--;
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("Unable to parse argument %s, which was expected to begin with a '-'\n", *current_arg);
					current_arg++;
					exit(EXIT_FAILURE);
				}
				
				switch(*current_char++){
				
					case 't':
						if(current_arg>=end_of_args){
							printf("%s: %d: ",__FILE__,__LINE__);
							printf("-t%c must be followed by an integer\n",*current_char++);
							exit(EXIT_FAILURE);
						}
						switch(*current_char++){
						
							case '1':
								{
									AIR_Error errcode;
									threshold1=AIR_parse_long(*current_arg,&errcode);
									current_arg++;
									if(errcode!=0){
										printf("%s: %d: ",__FILE__,__LINE__);
										printf("-t1 must be followed by an integer\n");
										exit(EXIT_FAILURE);
									}
								}
								break;
								
							case '2':
								{
									AIR_Error errcode;
									threshold2=AIR_parse_long(*current_arg,&errcode);
									current_arg++;
									if(errcode!=0){
										printf("%s: %d: ",__FILE__,__LINE__);
										printf("-t2 must be followed by an integer\n");
										exit(EXIT_FAILURE);
									}
								}
								break;
								
							default:
								current_arg--;
								printf("%s: %d: ",__FILE__,__LINE__);
								printf("Unable to parse %s\n",*current_arg);
								current_arg++;
								exit(EXIT_FAILURE);
						}
						break;
						
					case 'b':
						if(current_arg+2>=end_of_args){
							printf("%s: %d: ",__FILE__,__LINE__);
							printf("-b%c must be followed by three non-negative numbers\n",*current_char++);
							exit(EXIT_FAILURE);
						}
						switch(*current_char++){
						
							case '1':
								{
									AIR_Error errcode;
									kx1=AIR_parse_float(*current_arg, &errcode);
									current_arg++;
									if(errcode!=0 || kx1<0.0){
										printf("%s: %d: ",__FILE__,__LINE__);
										printf("-b1 must be followed by three non-negative numbers\n");
										exit(EXIT_FAILURE);
									}
								}
								{
									AIR_Error errcode;
									ky1=AIR_parse_float(*current_arg, &errcode);
									current_arg++;
									if(errcode!=0 || ky1<0.0){
										printf("%s: %d: ",__FILE__,__LINE__);
										printf("-b1 must be followed by three non-negative numbers\n");
										exit(EXIT_FAILURE);
									}
								}								
								{
									AIR_Error errcode;
									kz1=AIR_parse_float(*current_arg, &errcode);
									current_arg++;
									if(errcode!=0 || kz1<0.0){
										printf("%s: %d: ",__FILE__,__LINE__);
										printf("-b1 must be followed by three non-negative numbers\n");
										exit(EXIT_FAILURE);
									}
								}
								break;
								
							case '2':
								{
									AIR_Error errcode;
									kx2=AIR_parse_float(*current_arg, &errcode);
									current_arg++;
									if(errcode!=0 || kx2<0.0){
										printf("%s: %d: ",__FILE__,__LINE__);
										printf("-b2 must be followed by three non-negative numbers\n");
										exit(EXIT_FAILURE);
									}
								}
								{
									AIR_Error errcode;
									ky2=AIR_parse_float(*current_arg, &errcode);
									current_arg++;
									if(errcode!=0 || ky2<0.0){
										printf("%s: %d: ",__FILE__,__LINE__);
										printf("-b2 must be followed by three non-negative numbers\n");
										exit(EXIT_FAILURE);
									}
								}								
								{
									AIR_Error errcode;
									kz2=AIR_parse_float(*current_arg, &errcode);
									current_arg++;
									if(errcode!=0 || kz2<0.0){
										printf("%s: %d: ",__FILE__,__LINE__);
										printf("-b2 must be followed by three non-negative numbers\n");
										exit(EXIT_FAILURE);
									}
								}
								break;
								
							default:
								current_arg--;
								printf("%s: %d: ",__FILE__,__LINE__);
								printf("Unable to parse %s\n",*current_arg);
								current_arg++;
								exit(EXIT_FAILURE);
						}
						break;
						
					case 'p':
						if(current_arg>=end_of_args){
							printf("%s: %d: ",__FILE__,__LINE__);
							printf("-p%c must be followed by a non-negative integer\n",*current_char++);
							exit(EXIT_FAILURE);
						}
						switch(*current_char++){
						
							case '1':
								{
									AIR_Error errcode;
									partitions1=AIR_parse_uint(*current_arg, &errcode);
									current_arg++;
									if(errcode!=0){
										printf("%s: %d: ",__FILE__,__LINE__);
										printf("-p1 must be followed by a non-negative integer\n");
										exit(EXIT_FAILURE);
									}
								}
								break;
								
							case '2':
								{
									AIR_Error errcode;
									partitions2=AIR_parse_uint(*current_arg, &errcode);
									current_arg++;
									if(errcode!=0){
										printf("%s: %d: ",__FILE__,__LINE__);
										printf("-p2 must be followed by a non-negative integer\n");
										exit(EXIT_FAILURE);
									}
								}
								break;

							default:
								current_arg--;
								printf("%s: %d: ",__FILE__,__LINE__);
								printf("Unable to parse %s\n",*current_arg);
								current_arg++;
								exit(EXIT_FAILURE);
						}
						break;
						
					case 'e':
						if(current_arg>=end_of_args){
							printf("%s: %d: ",__FILE__,__LINE__);
							printf("An image file name must follow -e1 or -e2\n");
							exit(EXIT_FAILURE);
						}
						switch(*current_char++){
							case '1':
								standard_file_mask=*current_arg++;
								break;
								
							case '2':
								reslice_file_mask=*current_arg++;
								break;
								
							default:
								current_arg--;
								printf("%s: %d: ",__FILE__,__LINE__);
								printf("Unable to parse %s\n",*current_arg);
								current_arg++;
								exit(EXIT_FAILURE);
						}
						break;
						
					case 'f':
						if(current_arg>=end_of_args){
							printf("%s: %d: ",__FILE__,__LINE__);
							printf("An initialization parameter file name must follow -f or -fs\n");
							exit(EXIT_FAILURE);
						}
						switch(*current_char++){
							case '\0':
								initialization_file=*current_arg++;
								break;
							case 's':
								scaling_initialization_file=*current_arg++;
								break;
							default:
								current_arg--;
								printf("%s: %d: ",__FILE__,__LINE__);
								printf("Unable to parse %s\n",*current_arg);
								current_arg++;
								exit(EXIT_FAILURE);
						}
						break;

					case 'g':
						if(current_arg>=end_of_args){
							printf("%s: %d: ",__FILE__,__LINE__);
							printf("A termination parameter file name must follow -g or -gs\n");
							exit(EXIT_FAILURE);
						}
						switch(*current_char++){
							case '\0':
								termination_file=*current_arg++;
								if(current_arg<end_of_args){
									if(**current_arg!='-'){
										if(**current_arg++=='y') overwrite_termination_file=TRUE;
									}
								}
								break;
							case 's':
								scaling_termination_file=*current_arg++;
								if(current_arg<end_of_args){
									if(**current_arg!='-'){
										if(**current_arg++=='y') overwrite_scaling_termination_file=TRUE;
									}
								}
								break;
							default:
								current_arg--;
								printf("%s: %d: ",__FILE__,__LINE__);
								printf("Unable to parse %s\n",*current_arg);
								current_arg++;
								exit(EXIT_FAILURE);
						}
						break;
	
					
					case 's':
						if(current_arg+2>=end_of_args){
							printf("%s: %d: ",__FILE__,__LINE__);
							printf("-s must be followed by three positive integers\n");
							exit(EXIT_FAILURE);
						}
						{
							AIR_Error errcode;
							samplefactor=AIR_parse_uint(*current_arg, &errcode);
							current_arg++;
							if(errcode!=0 || samplefactor==0){
								printf("%s: %d: ",__FILE__,__LINE__);
								printf("-s must be followed by three positive integers\n");
								exit(EXIT_FAILURE);
							}
						}
						{
							AIR_Error errcode;
							samplefactor2=AIR_parse_uint(*current_arg, &errcode);
							current_arg++;
							if(errcode!=0 || samplefactor2==0){
								printf("%s: %d: ",__FILE__,__LINE__);
								printf("-s must be followed by three positive integers\n");
								exit(EXIT_FAILURE);
							}
						}
						{
							AIR_Error errcode;
							sffactor=AIR_parse_uint(*current_arg, &errcode);
							current_arg++;
							if(errcode!=0 || sffactor==0){
								printf("%s: %d: ",__FILE__,__LINE__);
								printf("-s must be followed by three positive integers\n");
								exit(EXIT_FAILURE);
							}
						}
						
						if(samplefactor2>samplefactor){
							printf("%s: %d: ",__FILE__,__LINE__);
							printf("Final sampling (2nd argument after -s) cannot be larger than initial sampling (1st argument after -s)\n");
							exit(EXIT_FAILURE);
						}
						if(sffactor==1 && samplefactor!=samplefactor2){
							printf("%s: %d: ",__FILE__,__LINE__);
							printf("Sampling decrement ratio (3rd argument after -s) must be larger than 1\n");
							exit(EXIT_FAILURE);
						}
						break;
				
					case 'c':
						if(current_arg>=end_of_args){
							printf("%s: %d: ",__FILE__,__LINE__);
							printf("-c must be followed by a positive number\n");
							exit(EXIT_FAILURE);
						}
						{
							AIR_Error errcode;
							precision=AIR_parse_float(*current_arg, &errcode);
							current_arg++;
							if(errcode!=0 || precision<=0.0){
								printf("%s: %d: ",__FILE__,__LINE__);
								printf("-c must be followed by a positive number\n");
								exit(EXIT_FAILURE);
							}
						}
						break;
						
					case 'r':
						if(current_arg>=end_of_args){
							printf("%s: %d: ",__FILE__,__LINE__);
							printf("-r must be followed by an unsigned integer\n");
							exit(EXIT_FAILURE);
						}
						{
							AIR_Error errcode;
							iterations=AIR_parse_uint(*current_arg, &errcode);
							current_arg++;
							if(errcode!=0){
								printf("%s: %d: ",__FILE__,__LINE__);
								printf("-r must be followed by an unsigned integer\n");
								exit(EXIT_FAILURE);
							}
						}
						break;
						
					case 'h':
						if(current_arg>=end_of_args){
							printf("%s: %d: ",__FILE__,__LINE__);
							printf("-h must be followed by a non-negative integer\n");
							exit(EXIT_FAILURE);
						}
						{
							AIR_Error errcode;
							noprogtries=AIR_parse_uint(*current_arg, &errcode);
							current_arg++;
							if(errcode!=0){
								printf("%s: %d: ",__FILE__,__LINE__);
								printf("-h must be followed by a non-negative integer\n");
								exit(EXIT_FAILURE);
							}
						}
						break;
						
					case 'v':
						verbosity=TRUE;
						break;
						
					case 'd':
						dynamic1=FALSE;
						dynamic2=FALSE;
						break;
						
					case 'j':
						posdefreq=FALSE;
						break;
						
					case 'q':
						interaction=FALSE;
						break;
						
					case 'z':
						zooming=TRUE;
						break;
						
					case 'x':
						if(current_arg>=end_of_args){
							printf("%s: %d: ",__FILE__,__LINE__);
							printf("-x must be followed by a valid cost function number from the menu\n");
							exit(EXIT_FAILURE);
						}
						{
							AIR_Error errcode;
							costfxn=AIR_parse_uint(*current_arg, &errcode);
							current_arg++;
							if(errcode!=0){
								printf("%s: %d: ",__FILE__,__LINE__);
								printf("-x must be followed by a valid cost function number from the menu\n");
								exit(EXIT_FAILURE);
							}
						}
						if(costfxn<1 || costfxn>3){
							printf("%s: %d: ",__FILE__,__LINE__);
							printf("-x must be followed by a valid cost function number from the menu\n");
							exit(EXIT_FAILURE);
						}
						break;
						
					case 'm':
						if(current_arg>=end_of_args){
							printf("%s: %d: ",__FILE__,__LINE__);
							printf("-m must be followed by a spatial transformation model from the menu\n");
							exit(EXIT_FAILURE);
						}
						{
							AIR_Error errcode;
							model=AIR_parse_uint(*current_arg, &errcode);
							current_arg++;
							if(errcode!=0){
								printf("%s: %d: ",__FILE__,__LINE__);
								printf("-m must be followed by a spatial transformation model from the menu\n");
								exit(EXIT_FAILURE);
							}
						}

						switch(model){
							case 6:
							case 7:
							case 9:
							case 12:
							case 15:
							case 23:
							case 24:
							case 25:
							case 26:
							case 28:
							break;
							default:
								printf("%s: %d: ",__FILE__,__LINE__);
								printf("-m must be followed by a spatial transformation model from the menu\n");
								exit(EXIT_FAILURE);
						}
						break;
						
					default:
						current_arg--;
						printf("%s: %d: ",__FILE__,__LINE__);
						printf("Unable to parse %s\n",*current_arg);
						current_arg++;
						exit(EXIT_FAILURE);
				}
			}
		}

		if(model==0){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("You must specify a spatial transformation model using the -m argument\n");
			printf("Type %s to see a list of models and usage\n",argv[0]);
			exit(EXIT_FAILURE);
		}

		if(zooming){
			if(standard_file_mask!=NULL || reslice_file_mask!=NULL){
				printf("WARNING: Using a mask file together with the -z option may not be a good idea\n");
			}
		}

		if(partitions1==0 && standard_file_mask!=NULL){
			printf("WARNING: The standard file mask %s will be ignored because you have set the standard file's number of partitions to zero\n", standard_file_mask);
			standard_file_mask=NULL;
		}
		
		if(partitions2==0 && reslice_file_mask!=NULL){
			printf("WARNING: The reslice file mask %s will be ignored because you have set the reslice file's number of partitions to zero\n", reslice_file_mask);
			reslice_file_mask=NULL;
		}
		if(costfxn!=1 && (partitions1>1 || partitions2>1)){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("Only the ratio image uniformity cost function allows use of more than one partition per file\n");
			exit(EXIT_FAILURE);
		}
		
		if(costfxn!=3 && scaling_initialization_file!=NULL){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("A scaling initialization file (%s) can only be used with the scaled least squares cost function (-x 3)\n", scaling_initialization_file);
			scaling_initialization_file=NULL;
			exit(EXIT_FAILURE);
		}
		
		if(costfxn!=3 && scaling_termination_file!=NULL){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("A scaling termination file (%s) can only be created with the scaled least squares cost function (-x 3)\n", scaling_termination_file);
			exit(EXIT_FAILURE);
		}
		
		if(partitions1+partitions2<1){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("Either the standard file or the reslice file must have at least 1 partition\n");
			exit(EXIT_FAILURE);
		}
		if(termination_file){
			if(strcmp(air_file, termination_file)==0){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("Cannot have two output files with the name: %s\n",termination_file);
				exit(EXIT_FAILURE);
			}
		}
		if(scaling_termination_file){
			if(strcmp(air_file, scaling_termination_file)==0){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("Cannot have two output files with the name: %s\n", scaling_termination_file);
				exit(EXIT_FAILURE);
			}
			if(termination_file){
				if(strcmp(termination_file, scaling_termination_file)==0){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("Cannot have two output files with the name: %s\n", scaling_termination_file);
					exit(EXIT_FAILURE);
				}
			}
		}

	
#if(AIR_CONFIG_VERBOSITY)
		printf("\tspatial transformation model: ");
		switch(model){
			case 6:
				printf("3D 6 parameter rigid-body model\n");
				break;
			case 7:
				printf("3D 7 parameter global rescaling model\n");
				break;
			case 9:
				printf("3D 9 parameter traditional model\n");
				break;
			case 12:
				printf("3D 12 parameter affine model\n");
				break;
			case 15:
				printf("3D 15 parameter perspective model\n");
				break;
			case 23:
				printf("2D 3 parameter rigid-body model\n");
				break;
			case 24:
				printf("2D 4 parameter global rescaling model\n");
				break;
			case 25:
				printf("2D 5 parameter affine model with fixed determinant\n");
				break;
			case 26:
				printf("2D 6 parameter affine model\n");
				break;
			case 28:
				printf("2D 8 parameter perspective model\n");
				break;
		}
		printf("\tstandard file: %s\n",standard_file);
		if(threshold1!=(AIR_CONFIG_THRESHOLD1)){
			printf("\t\tthreshold: %li\n",threshold1);
		}
		if(kx1!=0 || ky1!=0 || kz1!=0){
			printf("\t\tsmoothing kernel: FWHM_x=%e mm, FWMH_y=%e mm, FWMH_z=%e mm\n",kx1,ky1,kz1);
		}
		if(partitions1!=PARTITIONS1){
			printf("\t\tpartitions: %u\n",partitions1);
		}
		if(dynamic1!=DYNAMIC1){
			printf("\t\t\tstatic partitioning like AIR3.0 and earlier\n");
		}
		if(standard_file_mask){
			printf("\t\tmask file: %s\n",standard_file_mask);
		}
		printf("\treslice file: %s\n",reslice_file);
		if(threshold2!=(AIR_CONFIG_THRESHOLD2)){
			printf("\t\tthreshold: %li\n",threshold2);
		}
		if(kx2!=0 || ky2!=0 || kz2!=0){
			printf("\t\tsmoothing kernel: FWHM_x=%e mm, FWMH_y=%e mm, FWMH_z=%e mm\n",kx2,ky2,kz2);
		}
		if(partitions2!=PARTITIONS2){
			printf("\t\tpartitions: %u\n",partitions2);
		}
		if(dynamic2!=DYNAMIC2){
			printf("\t\t\tstatic partitioning like AIR3.0 and earlier\n");
		}
		if(reslice_file_mask){
			printf("\t\tmask file: %s\n",reslice_file_mask);
		}
		if(initialization_file){
			printf("\tinitialization file: %s\n",initialization_file);
		}
		if(scaling_initialization_file){
			printf("\tscaling initialization file: %s\n",scaling_initialization_file);
		}
		if(termination_file){
			printf("\ttermination file: %s\n",termination_file);
			if(overwrite_termination_file){
				printf("\t\toverwrite permission granted\n");
			}
		}
		if(scaling_termination_file){
			printf("\tscaling termination file: %s\n",scaling_termination_file);
			if(overwrite_scaling_termination_file){
				printf("\t\toverwrite permission granted\n");
			}
		}
		if(samplefactor!=SF || samplefactor2!=SF2 || sffactor!=SFF){
			printf("\tsampling:\n\t\tinitial: %u\n\t\tfinal: %u\n\t\tdecrement ratio: %u\n",samplefactor,samplefactor2,sffactor);
		}
		if(precision!=(float)CONVERGE){
			printf("\tconverge when predicted cost function change is less than %e\n",precision);
		}
		if(iterations!=ITERATE){
			printf("\tforce convergence after %u repeat iterations\n",iterations);
		}
		if(noprogtries!=NOPROGTRY){
			printf("\thalt after %u iterations with no improvement in cost function\n",noprogtries);
		}
		if(interaction!=INTERACTION){
			printf("\tassume noninteraction of spatial parameter derivatives\n");
		}
		if(verbosity!=LOCALVERBOSITY){
			printf("\tverbose mode selected\n");
		}
		
		if(zooming) printf("\tzooming enabled\n");
		
		if(costfxn!=COSTFXN){
			printf("\tcost function: ");
			switch(costfxn){
				case 1:
					printf("standard deviation of ratio image\n");
					break;
				case 2:
					printf("least squares\n");
					break;
				case 3:
					printf("least squares with intensity rescaling\n");
					break;
			}
		}
#endif
		{

			AIR_Error errcode=AIR_do_alignlinear(program, model, air_file, TRUE, standard_file, threshold1, kx1, ky1, kz1, partitions1, dynamic1,  standard_file_mask, reslice_file, threshold2, kx2, ky2, kz2, partitions2, dynamic2, reslice_file_mask, initialization_file, scaling_initialization_file, termination_file, overwrite_termination_file, scaling_termination_file, overwrite_scaling_termination_file, samplefactor, samplefactor2, sffactor, precision, iterations, noprogtries, interaction, verbosity, zooming, costfxn, posdefreq);

			if(errcode!=0){
				AIR_report_error(errcode);
				exit(EXIT_FAILURE);
			}

		}

	}
	return EXIT_SUCCESS;
}

