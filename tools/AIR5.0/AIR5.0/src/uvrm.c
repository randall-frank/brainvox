/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 5/27/01 */

/*
 * Inverts matrix and calculates derivatives
 * (first and second) of inversion
 *
 * It expects ef and er to be 4 x 4 matrices
 *
 * Returns:
 *	0 if successful
 *	errcode if unsuccessful
 */

#include "AIR.h"

AIR_Error AIR_uvrm(const unsigned int parameters, double **ef, double **er, double ***def, double ***der, double ****eef, double ****eer, const AIR_Boolean zooming, const struct AIR_Key_info *pixel1_stats, const struct AIR_Key_info *pixel2_stats)

/* ef=Input forward transform*/
/* er=Output reverse transform*/

/* def=Input 1st derivatives of forward transform*/
/* der=Output 1st derivatives of reverse transform*/

/* eef=Input 2nd derivatives of forward transform*/
/* eer=Output 2nd derivatives of reverse transform*/

{
	double	dfv0[16];
	double	*dfv[4];
	double	dfw0[16];
	double	*dfw[4];
	double	drv0[16];
	double	*drv[4];
	double	drw0[16];
	double 	*drw[4];
	double	drvw0[16];
	double	*drvw[4];
	double	drwv0[16];
	double	*drwv[4];
	double	efvw0[16];
	double	*efvw[4];
	double	err0[16];
	double	*err[4];
	
	{
		unsigned int i;
		
		for(i=0;i<4;i++){
			dfv[i]=dfv0+4*i;
			dfw[i]=dfw0+4*i;
			drv[i]=drv0+4*i;
			drw[i]=drw0+4*i;
			drvw[i]=drvw0+4*i;
			drwv[i]=drwv0+4*i;
			efvw[i]=efvw0+4*i;
			err[i]=err0+4*i;
		}
	}

	/*Copy parameters to be inverted into matrix er*/
	{
		unsigned int j;
		
		for(j=0;j<4;j++){
		
			unsigned int i;
			
			for(i=0;i<4;i++){
				er[j][i]=ef[j][i];
			}
		}
	}

	/* Invert spatially */
	{
		AIR_Error errcode=AIR_gael(er);
		if(errcode!=0) return errcode;
	}
	{
		unsigned int j;
		
		for(j=0;j<4;j++){
		
			unsigned int i;
			
			for(i=0;i<4;i++){
				err[j][i]=er[j][i];
			}
		}
	}

	/*Compute first derivatives of inverse with respect to each parameter*/
	{
		unsigned int dv;
		
		for (dv=0;dv<parameters;dv++){
			{
				unsigned int j;
				
				for(j=0;j<4;j++){
				
					unsigned int i;
					
					for(i=0;i<4;i++){
						dfv[j][i]=def[j][i][dv];
					}
				}
			}
			AIR_matmul(dfv,err,drv);	/* dfv*err=drv */
			AIR_matmul(err,drv,drv);	/* err*drv=drv=err*dfv*err */
			{
				unsigned int j;
				
				for(j=0;j<4;j++){
				
					unsigned int i;
					
					for(i=0;i<4;i++){
						der[j][i][dv]= -drv[j][i];	/* der=-err*dfv*err */
					}
				}
			}
		}
	}

	/*Compute second partial derivatives of inverse with respect to each pair of parameters*/
	{
		unsigned int dv;
		
		for(dv=0;dv<parameters;dv++){
		
			unsigned int dw;
			
			for(dw=0;dw<=dv;dw++){
				{
					unsigned int j;
					
					for(j=0;j<4;j++){
						
						unsigned int i;
						
						for(i=0;i<4;i++){
							dfv[j][i]=def[j][i][dv];
							dfw[j][i]=def[j][i][dw];
							efvw[j][i]=eef[j][i][dv][dw];
						}
					}
				}

				AIR_matmul(err,dfv,drv);		/* err*dfv=drv */
				AIR_matmul(err,dfw,drw);		/* err*dfw=drw */
				AIR_matmul(drw,drv,drwv);		/* drw*drv=drwv=err*dfw*err*dfv */
				AIR_matmul(drv,drw,drvw);		/* drv*drw=drvw=err*dfv*err*dfw */
				AIR_matmul(drwv,err,drwv);		/* drwv*err=drwv=err*dfv*err*dfw*err */
				AIR_matmul(drvw,err,drvw);		/* drvw*err=drvw=err*dfw*err*dfv*err */

				AIR_matmul(efvw,err,efvw);		/* efvw*err=efvw */
				AIR_matmul(err,efvw,efvw);		/* err*efvw=efvw=err*efvw*err */

				{
					unsigned int j;
					
					for(j=0;j<4;j++){

						unsigned int i;
						
						for(i=0;i<4;i++){
							eer[j][i][dv][dw]=drvw[j][i]+drwv[j][i]-efvw[j][i];
							/* eer= err*dfv*err*dfw*err+err*dfw*err*dfv*err-err*efvw*err */
						}
					}
				}
			}
		}
	}

	/*Adjust all values to account for zooming*/

	if (zooming){
		{
			double pixel_size2=pixel2_stats->x_size;
			if(pixel2_stats->y_size<pixel_size2) pixel_size2=pixel2_stats->y_size;
			if(pixel2_stats->z_size<pixel_size2) pixel_size2=pixel2_stats->z_size;
			{
				unsigned int i;
				
				for (i=0;i<4;i++){
					er[0][i]/=(pixel2_stats->x_size/pixel_size2);
					er[1][i]/=(pixel2_stats->y_size/pixel_size2);
					er[2][i]/=(pixel2_stats->z_size/pixel_size2);
					{
						unsigned int t;
						
						for (t=0;t<parameters;t++){
							der[0][i][t]/=(pixel2_stats->x_size/pixel_size2);
							der[1][i][t]/=(pixel2_stats->y_size/pixel_size2);
							der[2][i][t]/=(pixel2_stats->z_size/pixel_size2);
							{
								unsigned int s;
								
								for (s=0;s<=t;s++){
									eer[0][i][t][s]/=(pixel2_stats->x_size/pixel_size2);
									eer[1][i][t][s]/=(pixel2_stats->y_size/pixel_size2);
									eer[2][i][t][s]/=(pixel2_stats->z_size/pixel_size2);
								}
							}
						}
					}
				}
			}
		}
		{
			double pixel_size1=pixel1_stats->x_size;
			if(pixel1_stats->y_size<pixel_size1) pixel_size1=pixel1_stats->y_size;
			if(pixel1_stats->z_size<pixel_size1) pixel_size1=pixel1_stats->z_size;
			{
				unsigned int j;
				
				for (j=0;j<4;j++){
					er[j][0]/=(pixel1_stats->x_size/pixel_size1);
					er[j][1]/=(pixel1_stats->y_size/pixel_size1);
					er[j][2]/=(pixel1_stats->z_size/pixel_size1);
					{
						unsigned int t;
						
						for (t=0;t<parameters;t++){
							der[j][0][t]/=(pixel1_stats->x_size/pixel_size1);
							der[j][1][t]/=(pixel1_stats->y_size/pixel_size1);
							der[j][2][t]/=(pixel1_stats->z_size/pixel_size1);
							{
								unsigned int s;
								
								for (s=0;s<=t;s++){
									eer[j][0][t][s]/=(pixel1_stats->x_size/pixel_size1);
									eer[j][1][t][s]/=(pixel1_stats->y_size/pixel_size1);
									eer[j][2][t][s]/=(pixel1_stats->z_size/pixel_size1);
								}
							}
						}
					}
				}
			}
		}		
	}

	/* Divide everything by er[3][3] per homogenous coordinate protocol  */	
	{
		unsigned int j;
	
		for(j=0;j<4;j++){
	
			unsigned int i;
		
			for(i=0;i<4;i++){
			
				if(!(i==3 && j==3)){
			
					er[j][i]/=er[3][3];
					
					{
						unsigned int t;
						
						for(t=0;t<parameters;t++){
													
							der[j][i][t]-=er[j][i]*der[3][3][t];
							der[j][i][t]/=er[3][3];

							
							{
								unsigned int s;
								
								for(s=0;s<=t;s++){
								
									eer[j][i][t][s]-=er[j][i]*eer[3][3][t][s];
									eer[j][i][t][s]-=der[j][i][s]*der[3][3][t];
									eer[j][i][t][s]-=der[j][i][t]*der[3][3][s];
									eer[j][i][t][s]/=er[3][3];
									
								}
							}
						}
					}
				}
				else{
					/* Included only as a safeguard */
					er[3][3]=1;
					{
						unsigned int t;
						
						for(t=0;t<parameters;t++){
						
							der[3][3][t]=0;
							
							{
								unsigned int s;
								
								for(s=0;s<=t;s++){
								
									eer[3][3][t][s]=0;
								}
							}
						}
					}
				}
			}
		}
	}
	return 0;
}
