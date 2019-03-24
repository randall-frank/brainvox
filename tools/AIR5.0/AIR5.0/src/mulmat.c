/* Copyright 1997-2001 Roger P. Woods, M.D. */
/* Modified 5/5/01 */

/* void mulmat()
 *
 * Multiplies matrices: c=a*b
 *  	where a is m by n
 *	and
 *	where b is n by r
 *
 * 	so that c is m by r
 */

#include "AIR.h"

void AIR_mulmat(double **a, const unsigned int m, const unsigned int n, double **b, const unsigned int r, double **c)

{
	unsigned int y;
	double *out;
	double **btemp;
	
	for(y=0,out=*c,btemp=b;y<r;y++,btemp++){
		
		unsigned int x;
		
		for(x=0;x<m;x++,out++){
			*out=0.0;
			{
				unsigned int i;
				double *ina;
				double *inb;
				
				for(i=0,ina=(*a)+x,inb=*btemp;i<n;i++,ina+=m,inb++){
					/* c[y][x]=a[i][x]*b[y][i]; */
					*out+=*ina**inb;
				}
			}
		}
	}
}
