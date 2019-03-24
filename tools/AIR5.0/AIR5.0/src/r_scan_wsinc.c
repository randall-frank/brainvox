/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/30/01 */


/* AIR_Pixels ***r_scan_wsinc()
 *
 * This routine will reslice a file based on the unit vectors es
 *  sinc interpolation is used
 *
 * returns:
 *	pointer to resliced data if successful
 *	NULL pointer if unsuccessful
 *	error code in *errcode				
 */

#include "AIR.h"

#define BIG 2		/* BIG must be greater than or equal to SMALL */
#define SMALL 1

static double wsinc(const double alpha, const double pi, const unsigned int kern)

{
	if(alpha==0) return 1.0;
	return 0.5*(1.0+cos(pi*alpha/kern))*sin(pi*alpha)/(pi*alpha);
}

static void free_function(/*@null@*/ /*@out@*/ /*@only@*/ float *new, /*@null@*/ /*@out@*/ /*@only@*/ float *old, /*@null@*/ /*@out@*/ /*@only@*/ double *wa, /*@null@*/ /*@out@*/ /*@only@*/ double *ch, /*@null@*/ /*@out@*/ /*@only@*/ double *oldd, /*@null@*/ /*@out@*/ /*@only@*/ float ***temp_volume1)

{
	if(new) free(new);
	if(wa) free(wa);
	if(ch) free(ch);
	if(oldd) free(oldd);
	if(old) free(old);
	if(temp_volume1) AIR_free_3f(temp_volume1);
}


AIR_Pixels ***AIR_r_scan_wsinc(AIR_Pixels ***volume, const struct AIR_Key_info *stats, struct AIR_Key_info *new_stats, double **es, const double scale, const unsigned int xkern, const unsigned int ykern, const unsigned int zkern, AIR_Error *errcode)

{
	if(es[0][3]!=0.0||es[1][3]!=0.0||es[2][3]!=0.0||es[3][3]!=1.0){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("Sorry, scanline routines cannot do perspective transformations\n");
		*errcode=AIR_NO_PERSPECTIVE_ERROR;
		return NULL;
	}
	{
		float *new=NULL;
		float *old=NULL;
		double *wa=NULL;
		double *ch=NULL;
		double *oldd=NULL;
		float ***temp_volume1=NULL;
	
		AIR_Pixels 	***new_volume=NULL;
		double		e00,e01,e02,e10,e11,e12,e20,e21,e22,e30,e31,e32;
		double		e00i,e10i,e20i,e30i;
		double		pi=2*acos((double)0.0);
		unsigned int ykern2, zkern2;
	
		unsigned int
			x_dim2=new_stats->x_dim,
			y_dim2=new_stats->y_dim,
			z_dim2=new_stats->z_dim;
			
		unsigned int
			x_dim1=stats->x_dim,
			y_dim1=stats->y_dim,
			z_dim1=stats->z_dim;
		
		if(y_dim1>1){
			y_dim1*=BIG;
			y_dim1/=SMALL;
		}
		
		if(z_dim1>1){
			z_dim1*=BIG;
			z_dim1/=SMALL;
		}
	

	
		/* Decompose the transformation matrix */
		{	
			double
				e00t=es[0][0],
				e01t=((double)BIG/(double)SMALL)*es[0][1],
				e02t=((double)BIG/(double)SMALL)*es[0][2],
				e10t=es[1][0],
				e11t=((double)BIG/(double)SMALL)*es[1][1],
				e12t=((double)BIG/(double)SMALL)*es[1][2],
				e20t=es[2][0],
				e21t=((double)BIG/(double)SMALL)*es[2][1],
				e22t=((double)BIG/(double)SMALL)*es[2][2],
				e30t=es[3][0],
				e31t=((double)BIG/(double)SMALL)*es[3][1],
				e32t=((double)BIG/(double)SMALL)*es[3][2];
	
			e02=e02t;			/*i*/
			e12=e12t;			/*j*/
			e22=e22t;			/*k*/
	
			if(e22==0.0){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("Sorry, scanline routines cannot handle this transformation\n");
				*errcode=AIR_TRANSFORM_BREAKS_SCANLINE_ERROR;
				return NULL;
			}
	
			e32=e32t;			/*l*/
			e21=e21t/e22;			/*g*/
			e01=e01t-e21*e02;		/*e*/
			e11=e11t-e21*e12;		/*f*/
	
			if(e11==0.0){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("Sorry, scanline routines cannot handle this transformation\n");
				*errcode=AIR_TRANSFORM_BREAKS_SCANLINE_ERROR;
				return NULL;
			}
	
			e31=e31t-e21*e32;		/*h*/
			{
				double cbg=e20t/e22;
				double bf=e10t-e12*cbg;
				e10=bf/e11;			/*b*/
				e20=cbg-e10*e21;		/*c*/
				e00=e00t-e10*e01-e02*cbg;	/*a*/
				e30=e30t-e10*e31-e32*cbg;	/*d*/
		
				e00i=e00+e10*e01;		/*a+be*/
				e10i=bf;			/*bf*/
			}
			e20i=e20+e10*e21;		/*c+bg*/
			e30i=e30+e10*e31;		/*d+bh*/
		}
	
		/*Allocate memory*/
		{
			new_volume=AIR_create_vol3(x_dim2,y_dim2,z_dim2);
			{
				unsigned int x_dim3, y_dim3, z_dim3;
	
				/*Need to allocate enough float memory to accomodate larger of two x sizes, old z size and larger of two y sizes*/
	
				if(x_dim2>x_dim1) x_dim3=x_dim2;
				else x_dim3=x_dim1;
	
				if(y_dim2>y_dim1) y_dim3=y_dim2;
				else y_dim3=y_dim1;
	
				z_dim3=z_dim1;
	
				temp_volume1=AIR_matrix3f(x_dim3,y_dim3,z_dim3);
	
				{
					/*Allocate memory to interpolate*/
	
					unsigned int dim1max=x_dim1;
					if(y_dim1>dim1max) dim1max=y_dim1;
					if(z_dim1>dim1max) dim1max=z_dim1;
					
					old=(float *)malloc(dim1max*sizeof(float));
					oldd=AIR_matrix1(dim1max);
					ch=AIR_matrix1(dim1max);
					wa=AIR_matrix1(dim1max);
					{
						unsigned int dim2max=x_dim2;
						if(y_dim2>dim2max) dim2max=y_dim2;
						if(z_dim2>dim2max) dim2max=z_dim2;
						
						new=(float *)malloc(dim2max*sizeof(float));
					}
				}
			}
			if(!new_volume||!temp_volume1||!old||!oldd||!ch||!wa||!new){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("unable to allocate memory for interpolation\n");
				free_function(new,old,wa,ch,oldd,temp_volume1);
				if(new_volume) AIR_free_vol3(new_volume);
				*errcode=AIR_MEMORY_ALLOCATION_ERROR;
				return NULL;
			}
		}
	
		/*Prepass interpolate along y*/
		if(stats->y_dim>1 && BIG!=SMALL){
	
			unsigned int k;
	
			for(k=0;k<stats->z_dim;k++){
	
				unsigned int i;
	
				for(i=0;i<stats->x_dim;i++){
	
					/* Copy */
					{
						unsigned int j;
	
						for(j=0;j<stats->y_dim;j++){
							oldd[j]=(double)volume[k][j][i];
						}
					}
					/* Interpolate */
					{
						unsigned int ifac[20];
	
						/*Initialize fourier transform arrays for this dimension*/
						AIR_rffti1(stats->y_dim,wa,ifac);
	
						/*Compute forward transform*/
						AIR_rfftf1(stats->y_dim,oldd,ch,wa,ifac);
	
						if(2*(stats->y_dim/2)==stats->y_dim) oldd[stats->y_dim-1]/=2;
						{
							unsigned int j;
							
							for(j=stats->y_dim;j<(BIG*stats->y_dim)/SMALL;j++){
								oldd[j]=0.0;
							}
						}
						/*Initialize fourier transform arrays for new dimension*/
						AIR_rffti1((BIG*stats->y_dim)/SMALL,wa,ifac);
	
						/*Compute inverse transform*/
						AIR_rfftb1((BIG*stats->y_dim)/SMALL,oldd,ch,wa,ifac);
					}
					/* Copy back */
					{
						unsigned int j;
	
						for(j=0;j<y_dim1;j++){
							temp_volume1[k][j][i]=oldd[j]/stats->y_dim;
						}
					}
				}
			}
		}
		else{
			/* Have to move data into temp_volume1 regardless */
			unsigned int k;
	
			for(k=0;k<stats->z_dim;k++){
	
				unsigned int j;
	
				for(j=0;j<stats->y_dim;j++){
	
					unsigned int i;
	
					for(i=0;i<stats->x_dim;i++){
						temp_volume1[k][j][i]=(float)volume[k][j][i];
					}
				}
			}
		}
	
		/*Prepass interpolate along z */
		if(stats->z_dim>1 && BIG!=SMALL){
	
			unsigned int j;
	
			for(j=0;j<y_dim1;j++){
	
				unsigned int i;
	
				for(i=0;i<stats->x_dim;i++){
	
					/* Copy */
					{
						unsigned int k;
	
						for(k=0;k<stats->z_dim;k++){
							oldd[k]=temp_volume1[k][j][i];
						}
					}
					/* Interpolate */
					{
						unsigned int ifac[20];
	
						/*Initialize fourier transform arrays for this dimension*/
						AIR_rffti1(stats->z_dim,wa,ifac);
	
						/*Compute forward transform*/
						AIR_rfftf1(stats->z_dim,oldd,ch,wa,ifac);
	
						if(2*(stats->z_dim/2)==stats->z_dim) oldd[stats->z_dim-1]/=2;
		
						{
							unsigned int k;
							
							for(k=stats->z_dim;k<(BIG*stats->z_dim)/SMALL;k++){
								oldd[k]=0.0;
							}
						}
	
						/*Initialize fourier transform arrays for new dimension*/
						AIR_rffti1((BIG*stats->z_dim)/SMALL,wa,ifac);
	
						/*Compute inverse transform*/
						AIR_rfftb1((BIG*stats->z_dim)/SMALL,oldd,ch,wa,ifac);
					}
					/* Copy back */
					{
						unsigned int k;
	
						for(k=0;k<z_dim1;k++){
							temp_volume1[k][j][i]=oldd[k]/stats->z_dim;
						}
					}
				}
			}
		}
				
		/*Interpolate from float volume along x*/
		{
			double x_k=e30;
			unsigned int k;
	
			for(k=0;k<z_dim1;k++,x_k+=e20){
	
				double x_j=x_k;
				unsigned int j;
	
				for(j=0;j<y_dim1;j++,x_j+=e10){
	
					/* Copy */
					{
						unsigned int i;
	
						for(i=0;i<x_dim1;i++){
							old[i]=temp_volume1[k][j][i];
						}
					}
					/* Interpolate */
					{
						double x_i=x_j;
						float *i3=new;
						
						unsigned int i;
	
						for(i=0;i<x_dim2;i++,i3++,x_i+=e00){
							if(x_i>=0 && x_i<=x_dim1-1){
							
								unsigned int x_bot, x_top;
														
								x_bot=(unsigned int)floor(x_i);
								if(x_bot>=xkern) x_bot-=(xkern-1);
								else x_bot=0;
								
								x_top=(unsigned int)ceil(x_i)+xkern;
								if(x_dim1<x_top) x_top=x_dim1;
								{
									float *i2=old+x_bot;
									double total=0.0;
									
									unsigned int i4=x_bot;
		
									for(;i4<x_top;i4++,i2++){
										total+=*i2*wsinc(x_i-i4,pi,xkern);
									}
									*i3=(float)total;
								}
							}
							else *i3=0;
						}
					}
					/* Copy back */
					{
						unsigned int i;
	
						for(i=0;i<x_dim2;i++){
							temp_volume1[k][j][i]=new[i];
						}
					}
				}
			}
		}
	
		ykern2=(unsigned int)ceil((BIG*(float)ykern)/SMALL-.00000001);
	
	
		/*Interpolate from float volume along y*/
		{
			double
				x_k=e30i,
				y_k=e31;
				
			unsigned int k;
	
			for(k=0;k<z_dim1;k++,x_k+=e20i,y_k+=e21){
	
				double
					x_i=x_k,
					y_i=y_k;
					
				unsigned int i;
	
				for(i=0;i<x_dim2;i++,x_i+=e00i,y_i+=e01){
	
					/* Copy */
					{
						unsigned int j;
	
						for(j=0;j<y_dim1;j++){
							old[j]=temp_volume1[k][j][i];
						}
					}
					/* Interpolate */
					{
						double
							x_j=x_i,
							y_j=y_i;
							
						float *j3=new;
							
						unsigned int j;
	
						for(j=0;j<y_dim2;j++,j3++,x_j+=e10i,y_j+=e11){
							if(y_j>=0 && y_j<=y_dim1-1 && x_j>=0 && x_j<=x_dim1-1){
							
								unsigned int y_bot, y_top;
							
								y_bot=(unsigned int)floor(y_j);
								if(y_bot>=ykern2) y_bot-=(ykern2-1);
								else y_bot=0;
	
								y_top=(unsigned int)ceil(y_j)+ykern2;
								if(y_dim1<y_top) y_top=y_dim1;
								{
									float *j2=old+y_bot;
									double total=0.0;
									
									unsigned int j4=y_bot;
	
									for(;j4<y_top;j4++,j2++){
										total+=*j2*wsinc(y_j-j4,pi,ykern2);
									}
									*j3=(float)total;
								}
							}
							else *j3=0;
						}
					}
					/* Copy back */
					{
						unsigned int j;
	
						for(j=0;j<y_dim2;j++){
							temp_volume1[k][j][i]=new[j];
						}
					}
				}
			}
		}
	
		zkern2=(unsigned int)ceil((BIG*(float)zkern)/SMALL-.00000001);
	
		/*Interpolate from float volume along z*/
		{
			double z_j=e32;
			unsigned int j;
	
			for(j=0;j<y_dim2;j++,z_j+=e12){
	
				double z_i=z_j;
				unsigned int i;
	
				for(i=0;i<x_dim2;i++,z_i+=e02){
	
					/* Copy */
					{
						unsigned int k;
	
						for(k=0;k<z_dim1;k++){
							old[k]=temp_volume1[k][j][i];
						}
					}
					/* Interpolate */
					{
						double z_k=z_i;
						float *k3=new;
						unsigned int k;
	
						for(k=0;k<z_dim2;k++,k3++,z_k+=e22){
							if(z_k>=0 && z_k<=z_dim1-1){
							
								unsigned int z_bot, z_top;
						
								z_bot=(unsigned int)floor(z_k);
								if(z_bot>=zkern2) z_bot-=(zkern2-1);
								else z_bot=0;
								
								z_top=ceil(z_k)+zkern2;
								if(z_dim1<z_top) z_top=z_dim1;
								{
									float *k2=old+z_bot;
									double total=0.0;
									
									unsigned int k4=z_bot;
		
									for(;k4<z_top;k4++,k2++){
										total+=*k2*wsinc(z_k-k4,pi,zkern2);
									}
									*k3=(float)total;
								}
							}
							else *k3=0;
						}
					}
					/* Copy back, rounding */
					{
						unsigned int k;
	
						for(k=0;k<z_dim2;k++){
						
							new[k]*=scale;
							if(new[k]<0.0) new_volume[k][j][i]=0;
							else{
								new[k]+=.5;
								if(new[k]>(float)AIR_CONFIG_MAX_POSS_VALUE) new_volume[k][j][i]=AIR_CONFIG_MAX_POSS_VALUE;
								else new_volume[k][j][i]=(AIR_Pixels)new[k];
							}
						}
					}
				}
			}
		}
	
	
		e00=es[0][0];
		e01=es[0][1];
		e02=es[0][2];
		e10=es[1][0];
		e11=es[1][1];
		e12=es[1][2];
		e20=es[2][0];
		e21=es[2][1];
		e22=es[2][2];
		e30=es[3][0];
		e31=es[3][1];
		e32=es[3][2];
	
		/*Zero any points whose 8 neighbors are not within the original image boundaries*/
		{
			double
				x_k=e30,
				y_k=e31,
				z_k=e32;
	
			AIR_Pixels ***k3=new_volume;
			AIR_Pixels ***k4=k3+z_dim2;
	
			for (; k3<k4; k3++,x_k+=e20,y_k+=e21,z_k+=e22){
				
				double
					x_j=x_k,
					y_j=y_k,
					z_j=z_k;
				AIR_Pixels **j3=*k3;
				AIR_Pixels **j4=j3+y_dim2;
	
				for (; j3<j4; j3++,x_j+=e10,y_j+=e11,z_j+=e12){
	
					double
						x_i=x_j,
						y_i=y_j,
						z_i=z_j;
	
					AIR_Pixels *i3=*j3;
					AIR_Pixels *i4=i3+x_dim2;
	
					for (; i3<i4; i3++,x_i+=e00,y_i+=e01,z_i+=e02){
						if(x_i>=0 && x_i<=stats->x_dim-1){
							if(y_i>=0 && y_i<=stats->y_dim-1){
								if(!(z_i>=0 && z_i<=stats->z_dim-1)) *i3=0;
							}
							else *i3=0;
						}
						else *i3=0;
					}
				}
			}
		}
	
		free_function(new,old,wa,ch,oldd,temp_volume1);
		*errcode=0;	
		return new_volume;
	}
}

