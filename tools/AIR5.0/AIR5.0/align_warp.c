/* Copyright 1995-2002 Roger P. Woods, M.D. */
/* Modified 1/16/02 */

/* 	This program will apply a nonlinear registration model to align two studies.
 *	It incorporates the essential elements of all prior linear registration
 *	programs (e.g., alignpettopet, alignmritopet, etc.)
 */


#include "AIRmain.h"

#define SF 81			/*Default initial sampling interval*/
#define SF2 9			/*Default final sampling interval*/
#define SFF 3			/*Default sampling interval decrement ratio*/
#define CONVERGE .5		/*Default convergence criteria*/
#define ITERATE 50		/*Default iterations before forced convergence*/
#define NOPROGTRY 2		/*Default iterations without improved sd to force convergence*/

#define LOCALVERBOSITY FALSE	/*Governs printing of interim values on the screen*/
#define INTERACTION TRUE		/* 0 assumes non-interaction of spatial parameters */
#define POSITIVE_DEFINITE_REQUIRED TRUE


int main(int argc, char *argv[])

{
	long int threshold1=(AIR_CONFIG_THRESHOLD1);		/*ignore all pixels in the standard file less than this*/
	long int threshold2=(AIR_CONFIG_THRESHOLD2);		/*ignore all pixels in the reslice file less than this*/
	unsigned int samplefactor=SF;	/*initial sampling interval*/
	unsigned int samplefactor2=SF2;	/*final sampling interval*/
	unsigned int sffactor=SFF;		/*sampling interval decrement ratio*/
	unsigned int imodel=0;		/*menu identified model for registration*/
	unsigned int fmodel=0;		/*final model*/
	AIR_Boolean verbosity=LOCALVERBOSITY;		/*Controls printing of interim information*/
	float precision=CONVERGE;	/*value that must exceed predicted cost function change for convergence*/
	unsigned int iterations=ITERATE;		/*number of iterations before forced termination*/
	unsigned int noprogtries=NOPROGTRY;	/*number of iterations without sd improvement before forced termination*/
	AIR_Boolean posdefreq=POSITIVE_DEFINITE_REQUIRED;

	float kx1=0.0;
	float kx2=0.0;
	float ky1=0.0;
	float ky2=0.0;
	float kz1=0.0;
	float kz2=0.0;
	
	AIR_Boolean interaction=INTERACTION;
	AIR_Boolean intermediate=0;	/* flag whether to generate intermediate .warp files */

	if (argc<6){
		printf("\nUsage:%s standard_file reslice_file .warp_out -m model_menu_number [final_model_menu_number] [options]\n",argv[0]);
		printf("\tModel Menu:\n");
		printf("\t\t3-D models:\n");
		printf("\t\t\t 1. first order linear 12 parameter model\n");
		printf("\t\t\t 2. second order nonlinear 30 parameter model\n");
		printf("\t\t\t 3. third order nonlinear 60 parameter model\n");
		printf("\t\t\t 4. fourth order nonlinear 105 parameter model\n");
		printf("\t\t\t 5. fifth order nonlinear 168 parameter model\n");
		printf("\t\t\t 6. sixth order nonlinear 252 parameter model\n");
		printf("\t\t\t 7. seventh order nonlinear 360 parameter model\n");
		printf("\t\t\t 8. eighth order nonlinear 495 parameter model\n");
		printf("\t\t\t 9. ninth order nonlinear 660 parameter model\n");
		printf("\t\t\t 10. tenth order nonlinear 858 parameter model\n");
		printf("\t\t\t 11. eleventh order nonlinear 1092 parameter model\n");
		printf("\t\t\t 12. twelfth order nonlinear 1365 parameter model\n");
		printf("\n");
		printf("\t\t2-D models:\n");
		printf("\t\t\t 21. first order linear 6 parameter model\n");
		printf("\t\t\t 22. second order nonlinear 12 parameter model\n");
		printf("\t\t\t 23. third order nonlinear 20 parameter model\n");
		printf("\t\t\t 24. fourth order nonlinear 30 parameter model\n");
		printf("\t\t\t 25. fifth order nonlinear 42 parameter model\n");
		printf("\t\t\t 26. sixth order nonlinear 56 parameter model\n");
		printf("\t\t\t 27. seventh order nonlinear 72 parameter model\n");
		printf("\t\t\t 28. eighth order nonlinear 90 parameter model\n");
		printf("\t\t\t 29. ninth order nonlinear 110 parameter model\n");
		printf("\t\t\t 30. tenth order nonlinear 132 parameter model\n");
		printf("\t\t\t 31. eleventh order nonlinear 156 parameter model\n");
		printf("\t\t\t 32. twelfth order nonlinear 182 parameter model\n");
		printf("\n");
		printf("\n\toptions: (defaults in parentheses)\n");
		printf("\t[-b1 FWHM_x(%e) FWHM_y(%e) FWHM_z(%e)] (standard file)\n",kx1,ky1,kz1);
		printf("\t[-b2 FWHM_x(%e) FWHM_y(%e) FWHM_z(%e)] (reslice file)\n",kx2,ky2,kz2);
		printf("\t[-c convergence_threshold(%f)]\n",precision);
		printf("\t[-e1 standard_file_mask]\n");
		printf("\t[-e2 reslice_file_mask]\n");
		printf("\t[-f initialization_warp_file]\n");
		printf("\t[-fs scaling_initialization_file]\n");
		printf("\t[-gs scaling_termination_file [overwrite?(y/n)]\n");
		printf("\t[-h halt_after_(%u)_iterations_without_improvement]\n",noprogtries);
		printf("\t[-i] write out intermediate .warp files for each incremental model (append _model_number and allow overwriting)\n");
		printf("\t[-j] use non-positive definite Hessian matrices (not validated, use at your own risk)\n");
		printf("\t[-q] assume non-interaction of spatial parameter derivatives\n");
		printf("\t[-r repeated_iterations(%u)]\n",iterations);
		printf("\t[-s initial_sampling(%u) final_sampling(%u) sampling_decrement_ratio(%u)]\n",samplefactor,samplefactor2,sffactor);
		printf("\t[-t1 threshold_standard_file(%li)]\n",threshold1);
		printf("\t[-t2 threshold_reslice_file(%li)]\n",threshold2);
		printf("\t[-v] verbose reporting of interim results\n");
		exit(EXIT_FAILURE);
	}
	{
		char *program=argv[0];
		char *standard_file=argv[1];
		char *reslice_file=argv[2];
		char *warp_file=argv[3];
		
		char *standard_file_mask=NULL;
		char *reslice_file_mask=NULL;
		
		char *initialization_file=NULL;
		
		char *scaling_initialization_file=NULL;
		char *scaling_termination_file=NULL;
		
		AIR_Boolean overwrite_scaling_termination_file=0;
		
		/* Make sure .warp file does not end with header or image suffix */
		if(strstr(argv[3],AIR_CONFIG_IMG_SUFF)!=0||strstr(argv[3],AIR_CONFIG_HDR_SUFF)!=0){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("\nName of output .warp file cannot contain %s or %s\n",AIR_CONFIG_IMG_SUFF,AIR_CONFIG_HDR_SUFF);
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
					printf("Unable to parse argument %s, which was expected to being with a '-'\n", *current_arg);
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
									threshold1=AIR_parse_long(*current_arg, &errcode);
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
									threshold2=AIR_parse_long(*current_arg, &errcode);
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
							printf("-b%c must be followed by three non-negative numbers\n",*current_char);
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
							printf("A termination parameter file name must follow -gs\n");
							exit(EXIT_FAILURE);
						}
						switch(*current_char++){
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
							samplefactor=AIR_parse_uint(*current_arg,&errcode);
							current_arg++;
							if(errcode!=0 || samplefactor==0){
								printf("%s: %d: ",__FILE__,__LINE__);
								printf("-s must be followed by three positive integers\n");
								exit(EXIT_FAILURE);
							}
						}
						{
							AIR_Error errcode;
							samplefactor2=AIR_parse_uint(*current_arg,&errcode);
							current_arg++;
							if(errcode!=0 || samplefactor2==0){
								printf("%s: %d: ",__FILE__,__LINE__);
								printf("-s must be followed by three positive integers\n");
								exit(EXIT_FAILURE);
							}
						}
						{
							AIR_Error errcode;
							sffactor=AIR_parse_uint(*current_arg,&errcode);
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
							if(errcode!=0 || precision<=0){
								printf("%s: %d: ",__FILE__,__LINE__);
								printf("-c must be followed by a positive number\n");
								exit(EXIT_FAILURE);
							}
						}
						break;
						
					case 'r':
						if(current_arg>=end_of_args){
							printf("%s: %d: ",__FILE__,__LINE__);
							printf("-r must be followed by a non-negative integer\n");
							exit(EXIT_FAILURE);
						}
						{
							AIR_Error errcode;
							iterations=AIR_parse_uint(*current_arg,&errcode);
							current_arg++;
							if(errcode!=0){
								printf("%s: %d: ",__FILE__,__LINE__);
								printf("-r must be followed by a non-negative integer\n");
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
							noprogtries=AIR_parse_uint(*current_arg,&errcode);
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

					case 'q':
						interaction=FALSE;
						break;
						
					case 'j':
						posdefreq=FALSE;
						break;
						
					case 'i':
						intermediate=TRUE;
						break;
						
					case 'm':
											
						if(current_arg>=end_of_args){
							printf("%s: %d: ",__FILE__,__LINE__);
							printf("-m must be followed by a spatial transformation model number from the menu\n");
							exit(EXIT_FAILURE);
						}
						{
							AIR_Error errcode;
							fmodel=AIR_parse_uint(*current_arg, &errcode);
							current_arg++;
							if(errcode!=0){
								printf("%s: %d: ",__FILE__,__LINE__);
								printf("-m must be followed by a spatial transformation model number from the menu\n");
								exit(EXIT_FAILURE);
							}
						}
						if(fmodel<1 ||fmodel>32||(fmodel>12 && fmodel<21)){
								printf("%s: %d: ",__FILE__,__LINE__);
								printf("-m must be followed by a spatial transformation model number from the menu\n");
								exit(EXIT_FAILURE);
						}
						if(fmodel<21) imodel=1;
						else imodel=21;
						if(current_arg<end_of_args){
						
							if(**current_arg!='-'){
								imodel=fmodel;
								{
									AIR_Error errcode;
									fmodel=AIR_parse_uint(*current_arg, &errcode);
									current_arg++;
									if(errcode!=0){
										printf("%s: %d: ",__FILE__,__LINE__);
										printf("when two arguments not preceeded by '-' follow -m, both arguments must be spatial transformation model numbers from the menu\n");
										exit(EXIT_FAILURE);
									}
								}
								if(fmodel<1 ||fmodel>32||(fmodel>12 && fmodel<21)){
          								printf("%s: %d: ",__FILE__,__LINE__);
										printf("when two arguments not preceeded by '-' follow -m, both arguments must be spatial transformation model numbers from the menu\n");
										exit(EXIT_FAILURE);
								}
								if((imodel<21 && fmodel>20) || (fmodel<21 && imodel>20)){
									printf("%s: %d: ",__FILE__,__LINE__);
									printf("Initial model %u is incompatible with a final model of %u\n",imodel,fmodel);
									exit(EXIT_FAILURE);
								}
								if(fmodel<imodel){
									printf("%s: %d: ",__FILE__,__LINE__);
									printf("Final model %u should not be lower than initial model %u\n",fmodel,imodel);
									exit(EXIT_FAILURE);
								}
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
			}
		}

		if(imodel==0 || fmodel==0){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("You must specify a spatial transformation model using the -m argument\n");
			printf("Type %s to see a list of models and usage\n",argv[0]);
			exit(EXIT_FAILURE);
		}

		if(scaling_termination_file){
			if(strcmp(warp_file, scaling_termination_file)==0){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("Cannot have two output files with the name: %s\n", scaling_termination_file);
				exit(EXIT_FAILURE);
			}
		}
		if(intermediate && imodel==fmodel){
			printf("WARNING: -i flag has no effect when the initial and final models are identical\n");
			intermediate=0;
			exit(EXIT_FAILURE);
		}
#if(AIR_CONFIG_VERBOSITY)
		printf("\tspatial transformation model:\n");
		if(imodel<21){
			printf("\t\tinitial 3D warp order: %u\n",imodel);
			printf("\t\tfinal 3D warp order: %u\n",fmodel);
		}
		else{
			printf("\t\tinitial 2D warp order: %u\n",imodel-20);
			printf("\t\tfinal 2D warp order: %u\n",fmodel-20);
		}
		printf("\tstandard file: %s\n",standard_file);
		if(threshold1!=(AIR_CONFIG_THRESHOLD1)){
			printf("\t\tthreshold: %li\n",threshold1);
		}
		if(kx1!=0 || ky1!=0 || kz1!=0){
			printf("\t\tsmoothing kernel: FWHM_x=%e mm, FWMH_y=%e mm, FWMH_z=%e mm\n",kx1,ky1,kz1);
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
		if(reslice_file_mask){
			printf("\t\tmask file: %s\n",reslice_file_mask);
		}
		if(initialization_file){
			printf("\tinitialization file: %s\n",initialization_file);
		}
		if(scaling_initialization_file){
			printf("\tscaling initialization file: %s\n",scaling_initialization_file);
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
		if(intermediate){
			printf("\tgenerate intermediate .warp files\n");
		}
#endif

		{
			AIR_Error errcode=AIR_do_align_warp(program, imodel, fmodel, warp_file, TRUE, standard_file, threshold1, kx1, ky1, kz1, standard_file_mask, reslice_file, threshold2, kx2, ky2, kz2, reslice_file_mask, initialization_file, scaling_initialization_file, scaling_termination_file, overwrite_scaling_termination_file, samplefactor, samplefactor2, sffactor, precision, iterations, noprogtries, interaction, verbosity, intermediate, posdefreq);

			if(errcode!=0){
				AIR_report_error(errcode);
				exit(EXIT_FAILURE);
			}

		}
	}

	return(EXIT_SUCCESS);
}
