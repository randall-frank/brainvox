/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 11/4/01 */

/* 
 * This defines a new average standard space that is the
 *  average of the reslice spaces of a set of .air files that
 *  share a common standard space.
 * 
 * Routine iterates until all means are less than TINY, then
 *  iterates until failure to improve further.
 *
 * Storage2 needs to be allocated matrix3(2,2,20)
 * and storage4 needs to be allocated matrix3(4,4,14)
 *
 * Returns:
 *	0 if successful
 *	errcode if unsuccessful						
 */

#include "AIR.h"
#define TINY 1.0e-10
#define MAXITERS 100

AIR_Error AIR_comloger(const AIR_Boolean affinesafe, double ***en, const unsigned int count, const double *toproot, const double *botroot, double ***storage2, double ***storage4)

{

	double **e=*storage4++;
	double **t=*storage4++;
	double **temp=*storage4++;
	double **temp3=*storage4++;

	/*Do the averaging*/
	{
		unsigned int j;

		for(j=0;j<4;j++){

			unsigned int i;

			for(i=0;i<4;i++){
				/* Start with first file as the estimated average */
				temp[j][i]=t[j][i]=en[0][j][i];
			}
		}
	}
	{
		unsigned int its;
		double fitnew=0.0;
		double fitold=3.0;
		AIR_Boolean small=FALSE;
		unsigned int which=0;

		for(its=0;its<MAXITERS;its++){

			unsigned int ipvt[4];

			/*Minimize log*/

			/* Define estimated average as standard space */

			if(AIR_dgefa(t,4,ipvt)!=4) return AIR_SINGULAR_COMLOGER_ERROR;
			{
				unsigned int j;

				for(j=0;j<4;j++){

					unsigned int i;

					for(i=0;i<4;i++){
						temp3[j][i]=0.0;
					}
				}
			}

			{
				unsigned int k;

				for(k=0;k<count;k++){

					{
						unsigned int j;
	
						for(j=0;j<4;j++){
						
							unsigned int i;
						
							for(i=0;i<4;i++){
								
								e[j][i]=en[k][j][i];
							}
							AIR_dgesl(t,4,ipvt,e[j],FALSE);
							
						}
					}
					/*Find log of e*/
					{
						AIR_Error errcode;
						
						if(affinesafe){
							errcode=AIR_eloger5(e,toproot,botroot,storage2,storage4);
						}
						else{
							errcode=AIR_eloger4(4,e,toproot,botroot,storage2,storage4);
						}

						if(errcode!=0){
							which++;
							if(which<count){
								{
									unsigned int j;

									for(j=0;j<4;j++){

										unsigned int i;

										for(i=0;i<4;i++){
											/* Restart with next file as the estimated average */
											temp[j][i]=t[j][i]=en[which][j][i];
											temp3[j][i]=0.0;
										}
									}
								}
								if(AIR_dgefa(t,4,ipvt)!=4) return AIR_SINGULAR_COMLOGER_ERROR;
								k=0;
								its=0;
								continue;
							}
							else return errcode;
						}
					}
					{
						unsigned int j;

						for(j=0;j<4;j++){

							unsigned int i;

							for(i=0;i<4;i++){
								temp3[j][i]+=e[j][i];
							}
						}
					}
				}
			}
			{
				unsigned int j;

				for(j=0;j<4;j++){

					unsigned int i;

					for(i=0;i<4;i++){
						temp3[j][i]/=count;
						e[j][i]=temp3[j][i];
					}
				}
			}
			
			/*Find exp e*/
			{
				AIR_Error errcode=AIR_eexper4(4,e,toproot,botroot,storage2,storage4);

				if(errcode!=0) return errcode;
			}

			AIR_mulmat(temp,4,4,e,4,t);

			/*Reset for next iteration*/
			{
				unsigned int j;

				for(j=0;j<4;j++){

					unsigned int i;

					for(i=0;i<4;i++){
						temp[j][i]=t[j][i];
					}
				}
			}

			if(!small){
				small=TRUE;
				{
					unsigned int j;

					for(j=0;j<4;j++){

						unsigned int i;

						for(i=0;i<4;i++){
							if(temp3[j][i]>TINY) small=FALSE;
						}
					}
				}
			}
			else{
				fitnew=0.0;
				{
					unsigned int j;

					for(j=0;j<4;j++){

						unsigned int i;

						for(i=0;i<4;i++){
							fitnew+=fabs(temp3[j][i]);
						}
					}
				}
			}

			if(fitnew>=fitold) break;
			else if(small) fitold=fitnew;

		} /* End of loop */

		if(its==MAXITERS && !small) return AIR_COMLOGER_CONVERGE_ERROR;
	}
	/*Define new standard file based on mean of reslice files*/
	{
		unsigned int ipvt[4];

		if(AIR_dgefa(t,4,ipvt)!=4) return AIR_SINGULAR_COMLOGER_ERROR;
		{

			unsigned int k;

			for(k=0;k<count;k++){

				unsigned int j;

				for(j=0;j<4;j++){
					/*Copy row j of en[k] into work*/

					double work[4];

					{
						unsigned int i;

						for(i=0;i<4;i++){
							work[i]=en[k][i][j];
						}
					}
					/*Compute row j of output using dgesl in transpose mode*/
					AIR_dgesl(t,4,ipvt,work,TRUE);
					/*Copy this back into en*/
					{
						unsigned int i;

						for(i=0;i<4;i++){
							en[k][i][j]=work[i];
						}
					}
				}
			}
		}
	}

	return 0;
}
