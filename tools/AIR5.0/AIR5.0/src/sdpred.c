/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/27/01 */


/*
 * This routine estimates the predicted standard deviation
 *  based on first and second derivatives. Since it is
 *  derivative based, it leaves out a constant additive factor
 *  and is therefore only useful in estimating changes in sd.
 *  not the true value of sd
 *
 * Returns:
 *	the value of the predicted sd
 */


#include "AIR.h"

double AIR_sdpred(const double *delta, const double *dsd, double **esd, const unsigned int parameters)

/*First derivs in dsd have previously been multiplied by -1*/
{
	double total=0.0;
	unsigned int jj;

	/* Exploit symmetry of esd */
	for (jj=0;jj<parameters;jj++){
		total+=-delta[jj]*dsd[jj];
		{
			unsigned int ii;
			
			for(ii=0;ii<jj;ii++){
				total+=delta[jj]*esd[jj][ii]*delta[ii];
			}
		}
		total+=delta[jj]*esd[jj][jj]*delta[jj]/2;
	}
	return total;
}
