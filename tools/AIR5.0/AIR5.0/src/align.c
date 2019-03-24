/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 5/22/01 */

/*
 * double **align()
 *
 * This routine orchestrates the process of aligning two images
 *  for a linear N parameter fit.
 *
 * It calls all of the associated subroutines and is the routine
 *  that should be called by main programs when linear intersubject
 *  registration is needed.
 *
 * This variant of the routine allows for a rescaling parameter if needed.
 *
 * Edited versions of the files are accepted, but ignored if NULL
 * 
 * Note that when zooming!=0, an edited version of the file gets interpolated
 *  It's best to avoid this since editing is a nonlinear operator
 *
 * Returns:
 *	Pointer to unit vectors for performing registration
 *	NULL pointer if unsuccessful
 *								
 */

#include "AIR.h"
#include <float.h>

#define FORWARD 1
#define REVERSE 0

/* Hack to get the cost function back to the caller */
double AIR_align_cf_best = 0;
double AIR_align_last_costfunction(void)
{
	return(AIR_align_cf_best);
}

static void free_function(/*@null@*/ /*@out@*/ /*@only@*/ double **ecf, /*@null@*/ /*@out@*/ /*@only@*/ double **ecf2, /*@null@*/ /*@out@*/ /*@only@*/ double **er, /*@null@*/ /*@out@*/ /*@only@*/ double ***des, /*@null@*/ /*@out@*/ /*@only@*/ double ***der, /*@null@*/ /*@out@*/ /*@only@*/ double ****ees, /*@null@*/ /*@out@*/ /*@only@*/ double ****eer, /*@null@*/ /*@out@*/ /*@only@*/ double *dcff, /*@null@*/ /*@out@*/ /*@only@*/ double *dcfr, /*@null@*/ /*@out@*/ /*@only@*/ double *dcf, /*@null@*/ /*@out@*/ /*@only@*/ double **ecff, /*@null@*/ /*@out@*/ /*@only@*/ double **ecfr, /*@null@*/ /*@out@*/ /*@only@*/ unsigned int *q1, /*@null@*/ /*@out@*/ /*@only@*/ double *q2, /*@null@*/ /*@out@*/ /*@only@*/ double *q3, /*@null@*/ /*@out@*/ /*@only@*/ double **q4, /*@null@*/ /*@out@*/ /*@only@*/ double **q5, /*@null@*/ /*@out@*/ /*@only@*/ double ***q6, /*@null@*/ /*@out@*/ /*@only@*/ double ***q7, /*@null@*/ /*@out@*/ /*@only@*/ AIR_Pixels ***pixel5a, /*@null@*/ /*@out@*/ /*@only@*/ AIR_Pixels ***pixel6a, /*@null@*/ /*@out@*/ /*@only@*/ double *delta, /*@null@*/ /*@out@*/ /*@only@*/ double *i_best, /*@null@*/ /*@out@*/ /*@only@*/ unsigned int *kpvt, /*@null@*/ /*@out@*/ /*@only@*/ double *gersch)

{
	if(ecf) AIR_free_2(ecf);
	if(ecf2) AIR_free_2(ecf2);
	if(er) AIR_free_2(er);
	if(des) AIR_free_3(des);
	if(der) AIR_free_3(der);
	if(ees) AIR_free_4(ees);
	if(eer) AIR_free_4(eer);
	if(dcff) free(dcff);
	if(dcfr) free(dcfr);
	if(dcf) free(dcf);
	if(ecff) AIR_free_2(ecff);
	if(ecfr) AIR_free_2(ecfr);
	if(q1) free(q1);
	if(q2) free(q2);
	if(q3) free(q3);
	if(q4) AIR_free_2(q4);
	if(q5) AIR_free_2(q5);
	if(q6) AIR_free_3(q6);
	if(q7) AIR_free_3(q7);
	if(pixel5a) AIR_free_vol3(pixel5a);
	if(pixel6a) AIR_free_vol3(pixel6a);
	if(delta) free(delta);
	if(i_best) free(i_best);
	if(kpvt) free(kpvt);
	if(gersch) free(gersch);
}

double **AIR_align(
	const unsigned int spatial_parameters, 	/* Number of parameters in model*/
	double *i, 								/* Parameters*/
	void (*uvfN)(const double *, double **, double ***, double ****, const struct AIR_Key_info *, const struct AIR_Key_info *, const AIR_Boolean), 
	const unsigned int param_int, 			/* Internal parameters in uvderivsN */
	double (*uvderivsN)(const unsigned int, double **, double ***, double ****, const unsigned int, AIR_Pixels ***, const struct AIR_Key_info *, AIR_Pixels ***, const struct AIR_Key_info *, const AIR_Pixels, double *, double **, unsigned int *, double *, double *, double **, double **, double ***, double ***, const unsigned int, AIR_Pixels, double, const AIR_Boolean, unsigned int *), 
	AIR_Pixels ***pixel1, 					/* Standard file */
	AIR_Pixels ***pixel1e, 					/* Edited standard file */
	const struct AIR_Key_info *pixel1_info, 
	const AIR_Pixels threshold1, 			/* ignore pixels <=threshold */
	AIR_Pixels ***pixel2, 					/* Reslice file */
	AIR_Pixels ***pixel2e, 					/* Edited reslice file */
	const struct AIR_Key_info *pixel2_info, 
	const AIR_Pixels threshold2, 			/* Ignore pixels <=threshold */
	unsigned int samplefactor, 				/* Initial sampling interval */
	const unsigned int samplefactor2, 		/* Final sampling interval */
	unsigned int sffactor, 					/* Sampling interval decrement ratio */
	const double precision, 				/* Cost function predicted changed convergence criteria */
	const unsigned int iterations, 			/* Maximum iterations at given sampling */
	const unsigned int noprogtries, 		/* Maximum iterations without cost function improvement */
	const AIR_Boolean zooming, 				/* Interpolate to cubic voxels if TRUE */
	const unsigned int partitions1, 		/* Standard file partitions (1 for intramodality, 256 for cross modality */
	const unsigned int partitions2, 		/* Reslice file partitions (1 for intramodaility, 256 for cross modality */
	const AIR_Boolean dynamic1, 			/* Standard partitions extend to MAX_POSS_VALUE if FALSE, max_actual_value if TRUE */
	const AIR_Boolean dynamic2, 			/* Reslice partitions extend to MAX_POSS_VALUE if FALSE, max_actual_value if TRUE */
	const unsigned int scales, 				/* Number of scaling parameters (currently 0 or 1) */
	const unsigned int coords, 				/* Number of coordinates referenced in AIR file */
	const unsigned int coeffs, 				/* Number of coefficients referenced in AIR file */
	const AIR_Boolean memory, 				/* Allocate extra memory if TRUE */
	const AIR_Boolean posdefreq, 			/* Positive definite Hessian required if TRUE */
	const AIR_Boolean verbosity, 			/* Verbose messages if TRUE */
	AIR_Error *errcode						/* Error code return */
)
{
	unsigned int *q1=NULL;
	double *q2=NULL;
	double *q3=NULL;
	double **q4=NULL;
	double **q5=NULL;
	double ***q6=NULL;
	double ***q7=NULL;
	AIR_Pixels ***pixel5a=NULL;		/*pointer to interpolated pixel values of standard file if not NULL*/
	AIR_Pixels ***pixel6a=NULL;		/*pointer to interpolated pixel values of file to reslice if not NULL*/

	AIR_Pixels 	***pixel5=NULL;		/*pointer to possibly interpolated pixel values of standard file*/
	AIR_Pixels 	***pixel6=NULL;		/*pointer to possibly interpolated pixel values of file to reslice*/

	signed int direction;

	struct AIR_Key_info pixel5_info;
	struct AIR_Key_info pixel6_info;

	AIR_Pixels	max5;			/* Maximum value in edited, interpolated standard file */
	AIR_Pixels	max6;			/* Maximum value in edited, interpolated reslice file */ 

	double cf_best=DBL_MAX;				/* best value of cost function at the current sampling */

	double *scaleptr;
	double dummy=0.0;

	AIR_align_cf_best = 0.0;

	if(scales!=0) scaleptr=i+spatial_parameters;
	else scaleptr=&dummy;

	*errcode=0;

	/*Make sure that samplefactors can't take crazy values*/
	if (samplefactor==0){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("samplefactor cannot be zero\n");
		*errcode=AIR_ALIGN_BAD_PARAMETER_ERROR;
		return NULL;
	}
	if (samplefactor==samplefactor2) sffactor=2;


	/*Set flags,etc*/
	if(partitions1!=0 && partitions2!=0) direction=0;
	else if (partitions1!=0) direction=1;
	else if (partitions2!=0) direction=-1;
	else{
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("you have not requested any partitions for either file, cannot align without at least one partition\n");
		*errcode=AIR_ALIGN_BAD_PARAMETER_ERROR;
		return NULL;
	}
	{

		/*Allocate matrices*/
	
		double
			**es=AIR_matrix2(coords,coeffs),
			**er=AIR_matrix2(coords,coeffs);
		
		double
			***des=AIR_matrix3(spatial_parameters,coords,coeffs),		/* these being the origin and relative*/
			***der=AIR_matrix3(spatial_parameters,coords,coeffs);		/* these being the origin and relative*/
		
		double
			****ees=AIR_tritrix4(spatial_parameters,coords,coeffs),		/* positions of the 3 unit vectors */
			****eer=AIR_tritrix4(spatial_parameters,coords,coeffs);		/* positions of the 3 unit vectors */
		
		double
			*dcf=AIR_matrix1(spatial_parameters+scales),
			*dcff=AIR_matrix1(spatial_parameters+scales),
			*dcfr=AIR_matrix1(spatial_parameters+scales);
		
		double
			**ecf=AIR_tritrix2(spatial_parameters+scales),
			**ecf2=AIR_tritrix2(spatial_parameters+scales),
			**ecff=AIR_tritrix2(spatial_parameters+scales),
			**ecfr=AIR_tritrix2(spatial_parameters+scales);
		
		double *delta=AIR_matrix1(spatial_parameters+scales);
		double *i_best=AIR_matrix1(spatial_parameters+scales);
		double *gersch=AIR_matrix1(spatial_parameters+scales);
		unsigned int *kpvt=(unsigned int *)malloc((spatial_parameters+scales)*sizeof(unsigned int));

		if(memory){
	
			unsigned int partitions=partitions1;
			if(partitions2>partitions) partitions=partitions2;
	
			q1=(unsigned int *)malloc(partitions*sizeof(unsigned int));
			q2=AIR_matrix1(partitions);
			q3=AIR_matrix1(partitions);
			q4=AIR_matrix2(param_int,partitions);
			q5=AIR_matrix2(param_int,partitions);
			q6=AIR_tritrix3(param_int,partitions);
			q7=AIR_tritrix3(param_int,partitions);
		}
	
	
		/*Verify that allocations were all successful*/
	
		if(!ecf||!ecf2||!es||!er||!des||!der||!ees||!eer||!dcff||!dcfr||!dcf||!ecff||!ecfr||!delta||!i_best||!kpvt||!gersch){
			free_function(ecf,ecf2,er,des,der,ees,eer,dcff,dcfr,dcf,ecff,ecfr,q1,q2,q3,q4,q5,q6,q7,pixel5a,pixel6a,delta,i_best,kpvt,gersch);
			if(es) AIR_free_2(es);
			*errcode=AIR_MEMORY_ALLOCATION_ERROR;
			return NULL;
		}
		if(memory){
			if(!q1||!q2||!q3||!q4||!q5||!q6||!q7){
				free_function(ecf,ecf2,er,des,der,ees,eer,dcff,dcfr,dcf,ecff,ecfr,q1,q2,q3,q4,q5,q6,q7,pixel5a,pixel6a,delta,i_best,kpvt,gersch);
				if(es) AIR_free_2(es);
				*errcode=AIR_MEMORY_ALLOCATION_ERROR;
				return NULL;
			}
		}
	
		max5=max6=AIR_CONFIG_MAX_POSS_VALUE;
		if (direction>-1){
			if (!zooming){
				/*Pretend to make a copy of the standard file*/
				if(!pixel1e) pixel5=pixel1;
				else pixel5=pixel1e;
				pixel5_info=*pixel1_info;
			}
			else{
				/*Zoom up the standard file*/
				if(!pixel1e) pixel5=AIR_zoom(pixel1,pixel1_info,&pixel5_info,errcode);
				else pixel5=AIR_zoom(pixel1e,pixel1_info,&pixel5_info,errcode);
				if(!pixel5){
					free_function(ecf,ecf2,er,des,der,ees,eer,dcff,dcfr,dcf,ecff,ecfr,q1,q2,q3,q4,q5,q6,q7,pixel5a,pixel6a,delta,i_best,kpvt,gersch);
					AIR_free_2(es);
					return NULL;
				}
				pixel5a=pixel5;
			}
			if(dynamic1) max5=AIR_maxvoxel(pixel5,&pixel5_info);
		}
	
		if (direction<1){
			if (!zooming){
				/*Pretend to make a copy of the reslice file*/
				if(!pixel2e) pixel6=pixel2;
				else pixel6=pixel2e;
				pixel6_info=*pixel2_info;
			}
			else{
				/*Zoom up the reslice file*/
				if(!pixel2e) pixel6=AIR_zoom(pixel2,pixel2_info,&pixel6_info,errcode);
				else pixel6=AIR_zoom(pixel2e,pixel2_info,&pixel6_info,errcode);
				if(!pixel6){
					free_function(ecf,ecf2,er,des,der,ees,eer,dcff,dcfr,dcf,ecff,ecfr,q1,q2,q3,q4,q5,q6,q7,pixel5a,pixel6a,delta,i_best,kpvt,gersch);
					AIR_free_2(es);
					return NULL;
				}
				pixel6a=pixel6;
			}
			if(dynamic2) max6=AIR_maxvoxel(pixel6,&pixel6_info);
		}
	
		/*Make sure the two files aren't identical--skip registration if they are*/
		/* and return the appropriately adjusted identity matrix */
		if (AIR_identical(pixel1,pixel1_info,pixel2,pixel2_info)){
			{
				unsigned int jj;
	
				for (jj=0;jj<coeffs;jj++){
	
					unsigned int j;
	
					for (j=0;j<coords;j++){
						es[jj][j]=0;
					}
				}
			}
			es[0][0]=1,es[1][1]=1,es[2][2]=1,es[3][3]=1;
			if(zooming){
	
				double pixel_size1=pixel1_info->x_size;
				if(pixel1_info->y_size<pixel_size1) pixel_size1=pixel1_info->y_size;
				if(pixel1_info->z_size<pixel_size1) pixel_size1=pixel1_info->z_size;
				es[0][0]=pixel_size1/pixel1_info->x_size;
				es[1][1]=pixel_size1/pixel1_info->y_size;
				es[2][2]=pixel_size1/pixel1_info->z_size;
			}
			free_function(ecf,ecf2,er,des,der,ees,eer,dcff,dcfr,dcf,ecff,ecfr,q1,q2,q3,q4,q5,q6,q7,pixel5a,pixel6a,delta,i_best,kpvt,gersch);
			return es;
		}
	
		if(verbosity){
				(*uvfN)(i,es,des,ees,pixel2_info,pixel1_info,zooming);
	
				{
					unsigned int j;
	
					for(j=0;j<coords;j++){
	
						unsigned int jj;
	
						printf("START ");
						for(jj=0;jj<coeffs;jj++){
							printf("%e ",es[jj][j]);
						}
						printf("\n");
					}
				}
				if(scales!=0) printf("scale=%e\n",*scaleptr);
				printf("\n");
		}
	
		{
			double *i_bestt=i_best;
			double *it=i;
			unsigned int t;
	
			for(t=0;t<spatial_parameters+scales;t++, i_bestt++, it++){
				*i_bestt=*it;
			}
		}
	
		/*Iterate until the final sample factor is reached*/
		while (samplefactor>=samplefactor2){
		
			unsigned int iters=0;
			unsigned int tries=0;
			double deltacf=DBL_MAX;
			cf_best=DBL_MAX;
	
			do{				/*while(fabs(deltacf>precision)*/
			
				double cf;
			
				/*Calculate the standard deviation, mean and derivatives*/
				if (direction==0){
	
					unsigned int error=0;
	
					(*uvfN)(i,es,des,ees,pixel2_info,pixel1_info,zooming);
	
					cf=(*uvderivsN)(spatial_parameters,es,des,ees,samplefactor,pixel2,pixel2_info,pixel5,&pixel5_info,threshold1,dcff,ecff,q1,q2,q3,q4,q5,q6,q7,partitions1,max5,*scaleptr,TRUE,&error);
	
					{
						*errcode=AIR_uvrm(spatial_parameters,es,er,des,der,ees,eer,zooming,pixel1_info,pixel2_info);
						if(*errcode!=0){
							free_function(ecf,ecf2,er,des,der,ees,eer,dcff,dcfr,dcf,ecff,ecfr,q1,q2,q3,q4,q5,q6,q7,pixel5a,pixel6a,delta,i_best,kpvt,gersch);
							AIR_free_2(es);
							return NULL;
						}
					}
	
					cf+=(*uvderivsN)(spatial_parameters,er,der,eer,samplefactor,pixel1,pixel1_info,pixel6,&pixel6_info,threshold2,dcfr,ecfr,q1,q2,q3,q4,q5,q6,q7,partitions2,max6,*scaleptr,FALSE,&error);
					if(error==2) break;	/* No pixels above threshold that stayed in bounds */
				
					cf/=(2.0-error);
					{
						double *dcft=dcf;
						double *dcfft=dcff;
						double *dcfrt=dcfr;
						double *ecfts=*ecf;
						double *ecffts=*ecff;
						double *ecfrts=*ecfr;
						
						unsigned int t;
	
						for (t=0; t<spatial_parameters+scales; t++, dcft++, dcfft++, dcfrt++){
			
							*dcft=(-*dcfft-*dcfrt)/(2.0-error);
							
							{
								unsigned int s;
								
								for (s=0; s<=t; s++, ecfts++, ecffts++, ecfrts++){
	
									*ecfts=(*ecffts+*ecfrts)/(2.0-error);
	
								}
							}
						}
					}
				}
				else if (direction==-1){
	
					unsigned int error=0;
	
					(*uvfN)(i,es,des,ees,pixel2_info,pixel1_info,zooming);
					{
						*errcode=AIR_uvrm(spatial_parameters,es,er,des,der,ees,eer,zooming,pixel1_info,pixel2_info);
						if(*errcode!=0){
							free_function(ecf,ecf2,er,des,der,ees,eer,dcff,dcfr,dcf,ecff,ecfr,q1,q2,q3,q4,q5,q6,q7,pixel5a,pixel6a,delta,i_best,kpvt,gersch);
							AIR_free_2(es);
							return NULL;
						}
					}
					cf=(*uvderivsN)(spatial_parameters,er,der,eer,samplefactor,pixel1,pixel1_info,pixel6,&pixel6_info,threshold2,dcfr,ecfr,q1,q2,q3,q4,q5,q6,q7,partitions2,max6,*scaleptr,FALSE,&error);
					if(error!=0) break;	/* No pixels above threshold stayed in bounds */
					{
						double *dcft=dcf;
						double *dcfrt=dcfr;
						double *ecfts=*ecf;
						double *ecfrts=*ecfr;
						unsigned int t;
	
						for (t=0; t<spatial_parameters+scales; t++, dcft++, dcfrt++){
	
							*dcft=-*dcfrt;
							{
								unsigned int s;
								
								for (s=0; s<=t; s++, ecfts++, ecfrts++){
								
									*ecfts=*ecfrts;
								}
							}
						}
					}
				}
				else{
	
					unsigned int error=0;
	
					(*uvfN)(i,es,des,ees,pixel2_info,pixel1_info,zooming);
					cf=(*uvderivsN)(spatial_parameters,es,des,ees,samplefactor,pixel2,pixel2_info,pixel5,&pixel5_info,threshold1,dcff,ecff,q1,q2,q3,q4,q5,q6,q7,partitions1,max5,*scaleptr,TRUE,&error);
					if(error!=0) break;	/* No pixels above threshold stayed in bounds */
					{
						double *dcft=dcf;
						double *dcfft=dcff;
						double *ecfts=*ecf;
						double *ecffts=*ecff;
						unsigned int t;
	
						for (t=0;t<spatial_parameters+scales;t++, dcft++, dcfft++){
	
							*dcft=-*dcfft;
							{
								unsigned int s;
								
								for (s=0; s<=t; s++, ecfts++, ecffts++){
								
									*ecfts=*ecffts;
								}
							}
						}
					}
				}
	
	
				if(cf<cf_best){
					cf_best=cf;
					{
						double *i_bestt=i_best;
						double *it=i;
						unsigned int t;
	
						for(t=0;t<spatial_parameters+scales;t++, i_bestt++, it++){
							*i_bestt=*it;
						}
					}
					tries=0;
				}
				else tries++;
	
				if(tries>noprogtries) break;
	
				iters++;
				if (iters>iterations) break;
	
				{
					double *deltat=delta;
					double *dcft=dcf;
					double *ecf2ts=*ecf2;
					double *ecfts=*ecf;
					unsigned int t;
	
					for (t=0;t<spatial_parameters+scales;t++, deltat++, dcft++){
					
						*deltat=*dcft;
						{
							unsigned int s;
							
							for(s=0;s<=t;s++, ecf2ts++, ecfts++){
							
								*ecf2ts=*ecfts;
							}
						}
					}
				}
	
				/* Now we solve the system of equations Hessian*delta=gradient*/
				{
					if(AIR_dpofa(ecf2,spatial_parameters+scales)==spatial_parameters+scales){
						AIR_dposl(ecf2,spatial_parameters+scales,delta);
					}
					else{
						/* Hessian is not positive definite */
						if(posdefreq){
							/* If possible, we will sample more densely, otherwise, we will terminate */
							if(samplefactor/sffactor>=samplefactor2){
								if(verbosity){
									printf("%s: %d: ",__FILE__,__LINE__);
									printf("Increasing sampling due to non-positive definite Hessian matrix\n");
								}
							}
							else{
								printf("%s: %d: ",__FILE__,__LINE__);
								printf("WARNING: Registration terminated due to a Hessian matrix that was not positive definite\n");
								printf("Inspection of results is advised\n");
							}
							break;
						}
						/* Reload the Hessian for revised modified Cholesky factorization */
						{
							double *ecf2ts=*ecf2;
							double *ecfts=*ecf;
							unsigned int t;
	
							for (t=0;t<spatial_parameters+scales;t++){
							
								{
									unsigned int s;
									
									for(s=0;s<=t;s++, ecf2ts++, ecfts++){
									
										*ecf2ts=*ecfts;
									}
								}
							}
						}
						/* We use modchol and dmodposl which do not require a positive definite Hessian */
						(void)AIR_modchol(ecf2,spatial_parameters+scales,kpvt,gersch);
						if(verbosity){
								printf("Using revised modified Cholesky factorization\n");
						}	
						AIR_dmodposl(ecf2,spatial_parameters+scales,delta,kpvt);
					}
				}
	
				{
					double *it=i;
					double *deltat=delta;
					unsigned int t;
	
					for(t=0;t<spatial_parameters+scales;t++, it++, deltat++){
						*it+=*deltat;
					}
				}
	
				deltacf=AIR_sdpred(delta,dcf,ecf,spatial_parameters+scales);
	
				if(verbosity){
					(*uvfN)(i,es,des,ees,pixel2_info,pixel1_info,zooming);
	
					printf("cost function=%g\n",cf);
					printf("predicted cost function change=%e\n",deltacf);
					printf("iterations completed: %u sampling: %u\n",iters,samplefactor);
	
					printf("\n");
					{
						unsigned int j;
	
						for(j=0;j<coords;j++){
							printf("NEW ");
							{
								unsigned int jj;
	
								for(jj=0;jj<coeffs;jj++){
									printf("%e ",es[jj][j]);
								}
							}
							printf("\n");
						}
					}
					if(scales!=0) printf("scale=%e\n",*scaleptr);
					printf("\n");
				}
	
			}while(fabs(deltacf)>precision);
			{
				double *it=i;
				double *i_bestt=i_best;
				unsigned int t;
	
				for(t=0;t<spatial_parameters+scales;t++, it++, i_bestt++){
					*it=*i_bestt;
				}
			}
			if(verbosity){
				printf("\n");
				printf("BEST VALUES AT SAMPLEFACTOR=%u:\n",samplefactor);
				printf("cost function=%g\n",cf_best);
				(*uvfN)(i,es,des,ees,pixel2_info,pixel1_info,zooming);
	
				printf("\n");
				{
					unsigned int j;
	
					for(j=0;j<coords;j++){
						printf("BEST ");
						{
							unsigned int jj;
	
							for(jj=0;jj<coeffs;jj++){
								printf("%e ",es[jj][j]);
							}
						}
						printf("\n");
					}
				}
				if(scales!=0) printf("scale=%e\n",*scaleptr);
				printf("\n");
			}
	
			/*Calculate next sample factor for next iteration*/
			samplefactor=samplefactor/sffactor;
		}  /*until samplefactor<samplefactor2*/		
	
		if(verbosity){
			/*Print out final results*/
			printf("cost function=%g\n",cf_best);
		}
		
		/*Convert optimal parameters into transformation matrix once again*/
		(*uvfN)(i,es,des,ees,pixel2_info,pixel1_info,zooming);
	
	
		/*Clean up*/
		free_function(ecf,ecf2,er,des,der,ees,eer,dcff,dcfr,dcf,ecff,ecfr,q1,q2,q3,q4,q5,q6,q7,pixel5a,pixel6a,delta,i_best,kpvt,gersch);
	
		AIR_align_cf_best = cf_best;
		return es;
	}
}	
	
	
