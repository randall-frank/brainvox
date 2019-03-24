/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/10/01 */

/*
 * void dposl()
 *
 * This routine will solve a*x=b using factors computed by
 *  DGECO or DGEFA
 *
 * On entry
 *	a	the output from DGECO or DGEFA
 *	n	the order of the matrix A
 *	b	the right hand side vector (replaced by soln vector)
 *
 * On return
 *	b	the solution vector x
 *
 */

#include "AIR.h"

void AIR_dposl(double **a, const unsigned int n, double *b)

{
	double	t;

	/*  Solve TRANS(R)*y=b  */
	{
		unsigned int k;
		
		for(k=0;k<n;k++){
			t=AIR_ddot(k,a[k],b);
			b[k]=(b[k]-t)/a[k][k];
		}
	}

	/*Solve R*x=y */
	if(n!=0){
		unsigned int k;
		
		for(k=n;(k--)!=0;){
			b[k]/=a[k][k];
			t=-b[k];
			AIR_daxpy(k,t,a[k],b);
		}
	}
}
