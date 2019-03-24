/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/21/01 */


/*
 *
 * This routine will compute the inverse of the mapping usually used to reslice
 * Returns 0 if successful, error code otherwise
 *
 */

#include "AIR.h"

AIR_Error AIR_antiwarp3D(double **es, const double x, const double y, const double z, double *i_new, double *j_new, double *k_new, const unsigned int order, double *dx_pi, double *dx_pj, double *dx_pk, double *dx, const double accuracy)

/* (*i_new,*j_new,*k_new) should contain an initial guess */

{
	double		cost;

	double		b[3];
	double		*esd2[3];
	double		esd20[9];
	double		*ed[3];
	double		ed0[9];

	unsigned int iters=0;
	AIR_Error errcode=0;
	
	double i1=*i_new;
	double j1=*j_new;
	double k1=*k_new;
	
	esd2[0]=esd20;
	esd2[1]=esd20+3;
	esd2[2]=esd20+6;

	ed[0]=ed0;
	ed[1]=ed0+3;
	ed[2]=ed0+6;

	do{
		double x_p, y_p, z_p;
		/* Find the coordinate that corresponds to i1,j1,k1 */
		errcode=AIR_warp3D(es,i1,j1,k1,&x_p,&y_p,&z_p,order,dx);
		if(errcode!=0){
			iters=31;
			break;
		}
		errcode=AIR_diffg3D(es,i1,j1,k1,ed,order,dx_pi,dx_pj,dx_pk);
		if(errcode!=0){
			iters=31;
			break;
		}
		{
			double dx_i=ed[0][0];
			double dx_j=ed[1][0];
			double dx_k=ed[2][0];
			double dy_i=ed[0][1];
			double dy_j=ed[1][1];
			double dy_k=ed[2][1];
			double dz_i=ed[0][2];
			double dz_j=ed[1][2];
			double dz_k=ed[2][2];

			cost=(x_p-x)*(x_p-x)+(y_p-y)*(y_p-y)+(z_p-z)*(z_p-z);
			{
				double dcosti=2*(x_p-x)*dx_i+2*(y_p-y)*dy_i+2*(z_p-z)*dz_i;
				double dcostj=2*(x_p-x)*dx_j+2*(y_p-y)*dy_j+2*(z_p-z)*dz_j;
				double dcostk=2*(x_p-x)*dx_k+2*(y_p-y)*dy_k+2*(z_p-z)*dz_k;

				double ecostii=2*(dx_i*dx_i+dy_i*dy_i+dz_i*dz_i);
				double ecostij=2*(dx_j*dx_i+dy_j*dy_i+dz_j*dz_i);
				double ecostik=2*(dx_k*dx_i+dy_k*dy_i+dz_k*dz_i);

				double ecostjj=2*(dx_j*dx_j+dy_j*dy_j+dz_j*dz_j);
				double ecostjk=2*(dx_k*dx_j+dy_k*dy_j+dz_k*dz_j);

				double ecostkk=2*(dx_k*dx_k+dy_k*dy_k+dz_k*dz_k);

				b[0]=dcosti;
				b[1]=dcostj;
				b[2]=dcostk;

				esd2[0][0]=ecostii;
				esd2[0][1]=ecostij;
				esd2[0][2]=ecostik;
				esd2[1][0]=ecostij;
				esd2[1][1]=ecostjj;
				esd2[1][2]=ecostjk;
				esd2[2][0]=ecostik;
				esd2[2][1]=ecostjk;
				esd2[2][2]=ecostkk;
			}
		}
		if(AIR_dpofa(esd2,3)!=3){
			iters=31;
			break;
		}
		AIR_dposl(esd2,3,b);

		i1-=b[0];
		j1-=b[1];
		k1-=b[2];

		iters++;

	}while(cost>accuracy && iters<30);

	if(iters<30){
		*i_new=i1;
		*j_new=j1;
		*k_new=k1;
		return 0;
	}
	else{
		*i_new=-10000;
		*j_new=-10000;
		*k_new=-10000;
		if(errcode!=0) return errcode;
		return AIR_ANTIWARP_ERROR;
	}
}
