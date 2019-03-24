/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/22/01 */

/*
 * void dqpsrt()
 *
 * This routine maintains the descending ordering in the list of the
 * local error estimated resulting from the interval subdivision
 * process. At each call, two error estimates are inserted using the
 * sequential search method, top-down for the largest error estimate
 * and bottom-up for the smallest error estimate.
 *
 * Ported from the SLATEC Fortran routine which was written by Robert
 * Piessens and Elise de Doncker.
 *
 * limit is the maximum number of estimates the list can contain
 * last is the index of the last error estimate currently on the list
 * maxerr points to address of the nrmax-th largest error estimate currently on list
 * ermax=elist[maxerr] (nrmax-th largest error estimate)
 * elist is a vector of dimension last containing the error estimates
 * iord is a vector of dimension LAST, the first K elements of which contain
 *  pointers to the error estimates, such that iord is a vector of dimension last,
 *  the first k elements of which contain elist[iord[0]],...,elist[iord[k]] to
 *  form a decreasing sequence with k=last-1 if last<=(limit/2+2) and
 *  k=limit-last otherwise
 * maxerr=iord[nrmax]
 */

#include "AIR.h"

void AIR_dqpsrt(const unsigned int limit, const unsigned int last, unsigned int *maxerr, double *ermax, const double *elist, unsigned int *iord, unsigned int *nrmax)

{		
	/*Check whether the list contains more than two error estimates*/
	if(last<=1){
		iord[0]=0;
		iord[1]=1;
		*maxerr=iord[*nrmax];
		*ermax=elist[*maxerr];
		return;
	}
	
	{
		double errmax=elist[*maxerr];
		
		/* If a difficult integrand caused subdivision to increase the error estimate */
		if(*nrmax!=0){
		
			unsigned int ido=*nrmax-1;
			unsigned int i;
			
			for(i=0;i<=ido;i++){
			
				unsigned int isucc=iord[*nrmax-1];
				
				if(errmax<=elist[isucc]) break;
				iord[*nrmax]=isucc;
				(*nrmax)--;
			}
		}/*30*/

		{
			unsigned int jupbn; /* Number of elements in list to maintain in descending order */
			
			if(last>(limit/2+1)) jupbn=limit+1-last;
			else jupbn=last;
			{
				double errmin=elist[last];
				
				/*Insert errmax by traversing the list top-down, starting comparison*/
				/* from the element elist[iord[*nrmax+1]]*/
				{
					unsigned int i;
					unsigned int jbnd=jupbn-1;
					{
						AIR_Boolean flag=FALSE;
                                                
						for(i=*nrmax+1;i<=jbnd;i++){
						
								unsigned int isucc=iord[i];
								
								if(errmax>=elist[isucc]){
										flag=TRUE;
										break;
								}
								iord[i-1]=isucc;
						}
						if(!flag){
							iord[jbnd]=*maxerr;
							iord[jupbn]=last;
							*maxerr=iord[*nrmax];
							*ermax=elist[*maxerr];
							return;
						}
					}
					iord[i-1]=*maxerr;
					{
						unsigned int k=jbnd;
						unsigned int j;
						
						for(j=i;j<=jbnd;j++){
						
							unsigned int isucc=iord[k];
							
							if(errmin<elist[isucc]){
								iord[k+1]=last;
								*maxerr=iord[*nrmax];
								*ermax=elist[*maxerr];
								return;
							}
							iord[k+1]=isucc;
							k--;
						}
					}
					iord[i]=last;
				}
			}
		}
	}
	*maxerr=iord[*nrmax];
	*ermax=elist[*maxerr];
	return;	
}
