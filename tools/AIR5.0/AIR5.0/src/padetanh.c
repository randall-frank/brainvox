/* Copyright 2000-2001 Roger P. Woods, M.D. */
/* Modified 5/18/01 */

/*
 * Solves for Pade roots needed to computed matrix log and matrix exponential
 *
 * toproot and botroot must have a dimension of 8 (or more).
 *
 */

#include "AIR.h"
#define PADE 8 		/* Note that it is not sufficient to simply change this variable without other code changes */

AIR_Error AIR_padetanh(double *toproot, double *botroot)

{
	double cmdata[PADE*PADE];
	double *cm[PADE];

	/* Set up array cm */
	{
		double **cmi=cm;
		double **cmiend=cm+PADE;
		double *cmdata8=cmdata;
		
		for(;cmi<cmiend;cmi++, cmdata8+=PADE){
		
			*cmi=cmdata8;
		}
	}
	/* Zero array cm */
	{
		double *cmdatai=cmdata;
		double *cmdataend=cmdata+PADE*PADE;
		
		for(;cmdatai<cmdataend;cmdatai++){
		
			*cmdatai=0;
		}
	}
	/* Place ones along first subdiagonal */
	{
		unsigned int i;
		
		for(i=0;i<(unsigned int)PADE-1;i++){
			cm[i][i+1]=1;
		}
	}
	/* Load the polynomial to be solved */
	cm[1][0]=-990;
	cm[3][0]=-135135;
	cm[5][0]=-4729725;
	cm[7][0]=-34459425;
	/* Find the roots of the polynomial */
	{
		unsigned int is1, is2;
		double iv1[PADE];
		double fv1[PADE];
		double wr[PADE];
		
		AIR_balanc(PADE,cm,&is1,&is2,fv1);
		AIR_orthes(PADE,is1,is2,cm,iv1);
		{
			unsigned int ierr;
			
			AIR_hqr(PADE,is1,is2,cm,&wr[0],toproot,&ierr);
			if(ierr!=PADE) return AIR_HQR_FAILURE_ERROR;
		}
	}
	/* Rezero cm */
	{
		double *cmdatai=cmdata;
		double *cmdataend=cmdata+PADE*PADE;
		
		for(;cmdatai<cmdataend;cmdatai++){
		
			*cmdatai=0;
		}
	}
	/* Reset subdiagonal to ones */		
	{
		unsigned int i;
		
		for(i=0;i<(unsigned int)PADE-1;i++){
			cm[i][i+1]=1;
		}
	}
	/* Load the second polynomial */
	cm[1][0]=-308.0;
	cm[3][0]=-21021.0;
	cm[5][0]=-360360.0;
	cm[7][0]=-765765.0;
	/* Find the roots of the polynomial */
	{
		unsigned int is1, is2;
		double iv1[PADE];
		double fv1[PADE];
		double wr[PADE];
		
		AIR_balanc(PADE,cm,&is1,&is2,fv1);
		AIR_orthes(PADE,is1,is2,cm,iv1);
		{
			unsigned int ierr;
			
			AIR_hqr(PADE,is1,is2,cm,&wr[0],botroot,&ierr);
			if(ierr!=PADE) return AIR_HQR_FAILURE_ERROR;
		}
	}
	return 0;
}
