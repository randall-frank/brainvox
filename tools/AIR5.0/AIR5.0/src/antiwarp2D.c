/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/21/01 */


/*
 *
 * This routine will compute the inverse of the mapping usually used to reslice
 * Returns 0 if successful, error code otherwise
 *
 */

#include "AIR.h"

AIR_Error AIR_antiwarp2D(double **es, const double x, const double y, double *i_new, double *j_new, const unsigned int order, double *dx_pi, double *dx_pj, double *dx, const double accuracy)

/* (i_new,j_new) should contain an initial guess */

{
	double		cost;

	double		b[2];
	double		*esd2[2];
	double		esd20[4];
	double		*ed[3];			/* Must be 3 for diffg2D() */
	double		ed0[9];			/* Must be 9 for diffg2D() */

	unsigned int iters=0;
	AIR_Error errcode=0;
	
	double i1=*i_new;
	double j1=*j_new;

	esd2[0]=esd20;
	esd2[1]=esd20+2;

	ed[0]=ed0;
	ed[1]=ed0+3;
	ed[2]=ed0+6;	

	do{
		double x_p,y_p;
		
		/* Find the coordinate that corresponds to i1,j1 */
		errcode=AIR_warp2D(es,i1,j1,&x_p,&y_p,order,dx);
		if(errcode!=0){
			iters=31;
			break;
		}

		errcode=AIR_diffg2D(es,i1,j1,ed,order,dx_pi,dx_pj);
		if(errcode!=0){
			iters=31;
			break;
		}
		{
			double dx_i=ed[0][0];
			double dx_j=ed[1][0];
			double dy_i=ed[0][1];
			double dy_j=ed[1][1];

			cost=(x_p-x)*(x_p-x)+(y_p-y)*(y_p-y);
			{
				double dcosti=2*(x_p-x)*dx_i+2*(y_p-y)*dy_i;
				double dcostj=2*(x_p-x)*dx_j+2*(y_p-y)*dy_j;

				double ecostii=2*(dx_i*dx_i+dy_i*dy_i);
				double ecostij=2*(dx_j*dx_i+dy_j*dy_i);

				double ecostjj=2*(dx_j*dx_j+dy_j*dy_j);

				b[0]=dcosti;
				b[1]=dcostj;

				esd2[0][0]=ecostii;
				esd2[0][1]=ecostij;
				esd2[1][0]=ecostij;
				esd2[1][1]=ecostjj;
			}
		}
		if(AIR_dpofa(esd2,2)!=2){
			iters=31;
			break;
		}
		AIR_dposl(esd2,2,b);
		
		i1-=b[0];
		j1-=b[1];

		iters++;

	}while(cost>accuracy && iters<30);

	if(iters<30){
		*i_new=i1;
		*j_new=j1;
		return 0;
	}
	else{
		*i_new=-10000;
		*j_new=-10000;
		if(errcode!=0) return errcode;
		return AIR_ANTIWARP_ERROR;
	}
}
