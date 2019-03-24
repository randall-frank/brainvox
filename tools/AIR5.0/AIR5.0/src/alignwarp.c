/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 5/22/01 */

/*
 * double **alignwarp()
 *
 * This routine orchestrates the process of aligning two images
 *  for a nonlinear N parameter fit.
 * It calls all of the associated subroutines and is the routine
 *  that should be called by main programs when nonlinear
 *  registration is needed.
 *
 * Returns:
 *	Pointer to unit vectors for performing registration
 *	NULL pointer if unsuccessful
 *								
 */

#include "AIR.h"
#include <float.h>

static void freealignwarp(/*@null@*/ /*@out@*/ /*@only@*/ double **ecf2, /*@null@*/ /*@out@*/ /*@only@*/ double *delta, /*@null@*/ /*@out@*/ /*@only@*/ double *i_best, /*@null@*/ /*@out@*/ /*@only@*/ unsigned int *kpvt, /*@null@*/ /*@out@*/ /*@only@*/ double *dcf, /*@null@*/ /*@out@*/ /*@only@*/ double **ecf, /*@null@*/ /*@out@*/ /*@only@*/ double *dx, /*@null@*/ /*@out@*/ /*@only@*/ double *gersch)

{
	if(ecf2) AIR_free_2(ecf2);
	if(delta) free(delta);
	if(i_best) free(i_best);
	if(kpvt) free(kpvt);
	if(dcf) free(dcf);
	if(ecf) AIR_free_2(ecf);
	if(dx) free(dx);
	if(gersch) free(gersch);
}


double **AIR_alignwarp(double *i, double (*uvderivsN)(const unsigned int, double **, const unsigned int, AIR_Pixels ***, const struct AIR_Key_info *, const AIR_Pixels, AIR_Pixels ***, const struct AIR_Key_info *, const AIR_Pixels, double *, double **, double *, const double, unsigned int *), AIR_Pixels ***pixel1, const struct AIR_Key_info *pixel1_info, const AIR_Pixels threshold1, AIR_Pixels ***pixel2, const struct AIR_Key_info *pixel2_info, const AIR_Pixels threshold2, unsigned int samplefactor, const unsigned int samplefactor2, unsigned int sffactor, const double precision, const unsigned int iterations, const unsigned int noprogtries, const unsigned int coords, const unsigned int coeffp, const AIR_Boolean posdefreq, const AIR_Boolean verbosity, AIR_Error *errcode)

/* i=Parameters*/
/* uvderivsN=Routine to compute derivatives*/
/* pixel1=standard file*/
/* thresholds ignore pixel <= this value*/
/* pixel2=reslice file*/
/* samplefactor=initial sampling interval*/
/* samplefactor2=final sampling interval*/
/* sffactor=sampling interval decrement ratio*/
/* precision=first derivative convergence criteria*/
/* iterations=maximum iterations at a given sampling density*/
/* noprogtries=maximum iterations without improvement in cf*/
/* coords=the number of coordinates referenced in the AIR file*/
/* coeffp=the number of coefficients referenced in the AIR file*/

{
	double		**ecf2=NULL;			/*copy of ecf*/
	double		*delta=NULL;
	double		*i_best=NULL;		/*Best values of i at current samplingfactor*/
	unsigned int 		*kpvt=NULL;
	double		*dcf=NULL;		/* Subroutine storage */	
	double		**ecf=NULL;		/* Storage for use by subroutine, placed here to keep subroutines smaller */
	double		*dx=NULL;			/* Subroutine storage */
	double 		*gersch=NULL;
	
	double 		**es=NULL;	
		
	double cf_best=DBL_MAX;	/* best value of cost function at current sampling */

	const unsigned int 	parameters=coords*coeffp+1;	/* Spatial parameters plus rescaling parameter */

	double 	*scale;
	
	*errcode=0;


	/*Make sure that samplefactors can't take crazy values*/
	if (samplefactor==0){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("samplefactor cannot be zero\n");
		*errcode=AIR_ALIGNWARP_BAD_PARAMETER_ERROR;
		return NULL;
	}
	if (samplefactor==samplefactor2) {
		sffactor=2;
	}

	/*Allocate matrices*/

	ecf2=AIR_tritrix2(parameters);
	delta=AIR_matrix1(parameters);
	i_best=AIR_matrix1(parameters);
	kpvt=(unsigned int *)malloc(parameters*sizeof(unsigned int));
	dcf=AIR_matrix1(parameters);
	ecf=AIR_tritrix2(parameters);
	dx=AIR_matrix1(coeffp);
	gersch=AIR_matrix1(parameters);

	es=(double **)malloc(coords*sizeof(double));

	/*Verify that allocations were all successful*/

	if(!ecf2||!es||!delta||!i_best||!kpvt||!dcf||!ecf||!dx||!gersch){
		freealignwarp(ecf2,delta,i_best,kpvt,dcf,ecf,dx,gersch);
		if(es) free(es);
		*errcode=AIR_MEMORY_ALLOCATION_ERROR;
		return NULL;
	}

	{
		unsigned int jj;
		
		for(jj=0;jj<coords;jj++){
			es[jj]=&i[coeffp*jj];
		}
	}
	scale=&i[coeffp*coords];

	/*Make sure the two files aren't identical--skip registration if they are*/
	/* and return the appropriately adjusted identity matrix */
	if (AIR_identical(pixel1,pixel1_info,pixel2,pixel2_info)){
		
		unsigned int j;
		
		for(j=0;j<coords;j++){
			{
				unsigned int jj;
				
				for (jj=0;jj<coeffp;jj++){
					es[j][jj]=0.0;
				}
			}
			es[j][j+1]=1.0;
		}
		freealignwarp(ecf2,delta,i_best,kpvt,dcf,ecf,dx,gersch);
		return es;
	}

	if(verbosity!=0){
		
			{
				unsigned int j;
				
				for(j=0;j<coords;j++){
					printf("START ");
					{
						unsigned int jj;
						
						for(jj=0;jj<coeffp;jj++){
							printf("%e ",es[j][jj]);
						}
					}
					printf("\n");
				}
			}
			printf("scale=%e\n",*scale);
			printf("\n");
	}

	{
		double *i_bestt=i_best;
		double *it=i;
		unsigned int t;
		
		for(t=0;t<parameters;t++, i_bestt++, it++){
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
		
			/*Calculate the cost function and derivatives*/
			double cf;
		
			{
				unsigned int error=0;

				cf=(*uvderivsN)(parameters-1,es,samplefactor,pixel2,pixel2_info,threshold2,pixel1,pixel1_info,threshold1,dcf,ecf,dx,*scale,&error);

				if(error!=0) break;
			}

			{
				double *dcft=dcf;
				unsigned int t;
				
				for (t=0;t<parameters;t++, dcft++){
					(*dcft)*=-1;
				}
			}


			if(cf<cf_best){
				cf_best=cf;
				{
					double *i_bestt=i_best;
					double *it=i;
					unsigned int t;
					
					for(t=0; t<parameters; t++, i_bestt++, it++){
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
				
				for (t=0; t<parameters; t++, deltat++, dcft++){
				
					*deltat=*dcft;
					
					{
						unsigned int s;
						
						for(s=0; s<=t; s++, ecf2ts++, ecfts++){
							
							*ecf2ts=*ecfts;
						}
					}
				}
			}


			/* Now we solve the system of equations Hessian*delta=gradient*/
			{
				if(AIR_dpofa(ecf2,parameters)==parameters){
					AIR_dposl(ecf2,parameters,delta);
				}
				else{
					/* Hessian is not positive definite */
					if(posdefreq){
						/* If possible, we will sample more densely, otherwise, we will terminate */
						if(samplefactor/sffactor>=samplefactor2){
							if(verbosity!=0){
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
						
						for (t=0; t<parameters; t++){
													
							{
								unsigned int s;
								
								for(s=0; s<=t; s++, ecf2ts++, ecfts++){
									
									*ecf2ts=*ecfts;
								}
							}
						}
					}
					/* Now use modchol and dmodposl which do not require a positive definite Hessian */
					(void)AIR_modchol(ecf2,parameters,kpvt,gersch);
					if(verbosity!=0){
							printf("Using revised modified Cholesky factorization\n");
					}
					AIR_dmodposl(ecf2,parameters,delta,kpvt);
				}
			}
			
			{
				double *it=i;
				double *deltat=delta;
				unsigned int t;
				
				for(t=0; t<parameters; t++, it++, deltat++){
				
					*it+=*deltat;
				}
			}

			deltacf=AIR_sdpred(delta,dcf,ecf,parameters);

			if(verbosity!=0){
			
				printf("cost function=%g\n",cf);
				printf("predicted cost function change=%e\n",deltacf);
				printf("iterations completed: %u sampling: %u parameters: %u\n",iters,samplefactor,parameters);
/*
				printf("\n");
				{
					unsigned int j;
					
					for(j=0;j<coords;j++){
						printf("NEW ");
						{
							unsigned int jj;
							
							for(jj=0;jj<coeffp;jj++){
								printf("%e ",es[j][jj]);
							}
						}
						printf("\n");
					}
				}
*/
				printf("scale=%e\n",*scale);
				printf("\n");
			}

		}while(fabs(deltacf)>precision);
		
		{
			double *it=i;
			double *i_bestt=i_best;
			unsigned int t;
			
			for(t=0;t<parameters;t++, it++, i_bestt++){
				*it=*i_bestt;
			}
		}
		if(verbosity!=0){
			printf("\n");
			printf("BEST VALUES AT SAMPLEFACTOR=%u:\n",samplefactor);
			printf("cost function=%g\n",cf_best);

			printf("\n");
			{
				unsigned int j;
				
				for(j=0;j<coords;j++){
					printf("BEST ");
					{
						unsigned int jj;
						
						for(jj=0;jj<coeffp;jj++){
							printf("%e ",es[j][jj]);
						}
					}
					printf("\n");
				}
			}
			printf("scale=%e\n",*scale);
			printf("\n");
		}

	/*Calculate next sample factor for next iteration*/
	samplefactor=samplefactor/sffactor;
	}  /*until samplefactor<samplefactor2*/		

	if(verbosity!=0){
		/*Print out final results*/
		printf("cost function=%g\n",cf_best);
	}

	/*Clean up*/
	freealignwarp(ecf2,delta,i_best,kpvt,dcf,ecf,dx,gersch);

	return es;
}	
	
	
