/* Copyright 1996-2001 Roger P. Woods, M.D. */
/* Modified 5/30/01 */

/* AIR_Pixels ***r_scan_chirp_xy()
 *
 * This routine will reslice a file based on the unit vectors es
 *  chirp z interpolation is used
 *
 * returns:
 *	pointer to resliced data if successful
 *	NULL pointer if unsuccessful
 *	error code in *errcode
 */

#include "AIR.h"

#define BIGY 2.0	/* Must be greater than or equal to 1.0 */

static void free_function(/*@null@*/ /*@out@*/ /*@only@*/ double *new, /*@null@*/ /*@out@*/ /*@only@*/ double *wa, /*@null@*/ /*@out@*/ /*@only@*/ double *wa2, /*@null@*/ /*@out@*/ /*@only@*/ double *work1, /*@null@*/ /*@out@*/ /*@only@*/ double *work2, /*@null@*/ /*@out@*/ /*@only@*/ double *work3, /*@null@*/ /*@out@*/ /*@only@*/ double *ch, /*@null@*/ /*@out@*/ /*@only@*/ double *old, /*@null@*/ /*@out@*/ /*@only@*/ float ***temp_volume1, /*@null@*/ /*@out@*/ /*@only@*/ float **temp_plane1)

{
	if(new) free(new);
	if(wa) free(wa);
	if(wa2) free(wa2);
	if(work1) free(work1);
	if(work2) free(work2);
	if(work3) free(work3);
	if(ch) free(ch);
	if(old) free(old);
	if(temp_volume1) AIR_free_3f(temp_volume1);
	if(temp_plane1) AIR_free_2f(temp_plane1);
}


AIR_Pixels ***AIR_r_scan_chirp_xy(AIR_Pixels ***volume, const struct AIR_Key_info *stats, struct AIR_Key_info *new_stats, double **es, const double scale, AIR_Error *errcode)

{
	if(es[0][3]!=0.0||es[1][3]!=0.0||es[2][3]!=0.0||es[3][3]!=1.0){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("Sorry, scanline routines cannot do perspective transformations\n");
		*errcode=AIR_NO_PERSPECTIVE_ERROR;
		return NULL;
	}
	{
		double *new=NULL;
		double *wa=NULL;
		double *wa2=NULL;
		double *work1=NULL;
		double *work2=NULL;
		double *work3=NULL;
		double *ch=NULL;
		double *old=NULL;
		float ***temp_volume1=NULL;
		float **temp_plane1=NULL;
	
		AIR_Pixels ***new_volume=NULL;
	
		double e00,e01,e02,e10,e11,e12,e20,e21,e22,e30,e31,e32;
		double e00i,e10i,e20i,e30i;
		double pi=2*acos((double)0.0);
	
		unsigned int
			x_dim2=new_stats->x_dim,
			y_dim2=new_stats->y_dim,
			z_dim2=new_stats->z_dim;
	
		unsigned int
			x_dim1=stats->x_dim,
			y_dim1,
			z_dim1;
	
		if(stats->y_dim>1) y_dim1=(unsigned int)ceil((double)BIGY*stats->y_dim);
		else y_dim1=1;
	
		z_dim1=stats->z_dim;
	
		/* Decompose the transformation matrix */
		{	
			double
				e00t=es[0][0],
				e01t=((double)BIGY)*es[0][1],
				e02t=es[0][2],
				e10t=es[1][0],
				e11t=((double)BIGY)*es[1][1],
				e12t=es[1][2],
				e20t=es[2][0],
				e21t=((double)BIGY)*es[2][1],
				e22t=es[2][2],
				e30t=es[3][0],
				e31t=((double)BIGY)*es[3][1],
				e32t=es[3][2];
	
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
	
				/*Need to allocate enough float memory to accomodate larger of two x sizes, old z size and */
				/* larger of original (before prepass) and final y sizes */
	
				unsigned int x_dim3, y_dim4;
	
				if(x_dim2>x_dim1) x_dim3=x_dim2;
				else x_dim3=x_dim1;
	
				if(y_dim2>stats->y_dim) y_dim4=y_dim2;
				else (y_dim4=stats->y_dim);
	
				temp_volume1=AIR_matrix3f(x_dim3,y_dim4,z_dim1);
	
				/* Need to have a plane that is larger of x_sizes by old y size */
				temp_plane1=AIR_matrix2f(x_dim3,y_dim1);
				{
					/*Allocate memory to interpolate*/

					unsigned int dim1max=x_dim1;
					if(y_dim1>dim1max) dim1max=y_dim1;

					old=AIR_matrix1(2*dim1max);	
					wa2=AIR_matrix1(2*dim1max);
					{

						unsigned int dim2max=x_dim2;
						if(y_dim2>dim2max) dim2max=y_dim2;
						if(y_dim1>dim2max) dim2max=y_dim1;	/* For prepass */

						new=AIR_matrix1(2*dim2max);
						{

							unsigned int maxmem=x_dim1+x_dim2*ceil(fabs(e00));
							if(maxmem<y_dim1+y_dim2*ceil(fabs(e11))) maxmem=y_dim1+y_dim2*ceil(fabs(e11));
							if(maxmem<stats->y_dim+y_dim1) maxmem=stats->y_dim+y_dim1;	/* For prepass */
							
							{
								unsigned int nfft=1;
								
								while(nfft<maxmem) nfft*=2;
	
								ch=AIR_matrix1(2*nfft);
								work1=AIR_matrix1(2*nfft);
								work2=AIR_matrix1(2*nfft);
								work3=AIR_matrix1(2*nfft);
								wa=AIR_matrix1(2*nfft);
							}
						}
					}
				}
			}
			if(!new_volume||!temp_volume1||!temp_plane1||!old||!wa2||!new||!ch||!work1||!work2||!work3||!wa){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("chirperxy() memory allocation failure\n");
				free_function(new,wa,wa2,work1,work2,work3,ch,old,temp_volume1,temp_plane1);
				if(new_volume) AIR_free_vol3(new_volume);
				*errcode=AIR_MEMORY_ALLOCATION_ERROR;
				return NULL;
			}
		}
	
		/* Now, for each plane, we prepass interpolate along y and then interpolate x and y */
		{
			double
				x_k=e30,
				x_k2=e30i,
				y_k=e31;
			unsigned int k;
	
			for(k=0;k<z_dim1;k++,x_k+=e20,x_k2+=e20i,y_k+=e21){
	
				/* Prepass interpolate along y*/
				{
					unsigned int oversamp=1;
					double w=2.0*pi/(stats->y_dim*(double)BIGY);
					double a=0.0;
					AIR_Boolean inited=0;
					unsigned int ifac[20],ifac2[20];
					unsigned int nfft=1;

					while(nfft<=stats->y_dim+y_dim1-1) nfft*=2;
	
					if(stats->y_dim>1 && (double)BIGY!=1.0){
	
						unsigned int i;
	
						for(i=0;i<stats->x_dim;i++){
	
							/* Copy */
							{
								unsigned int j;
	
								for(j=0;j<stats->y_dim;j++){
									old[2*j]=(double)volume[k][j][i];
									old[2*j+1]=0.0;
								}
							}
							/* Interpolate */
							{
								AIR_chirpterp(stats->y_dim,old,y_dim1,new,nfft,work1,work2,work3,ch,wa2,wa,ifac2,ifac,w,a,oversamp,&inited);
							}
							/* Copy back */
							{
								unsigned int j;
	
								for(j=0;j<y_dim1;j++) temp_plane1[j][i]=(float)new[2*j];
							}
						}
					}
					else{
						/* Have to move data into temp_plane1 regardless */
	
						unsigned int j;
	
						for(j=0;j<stats->y_dim;j++){
	
							unsigned int i;
	
							for(i=0;i<stats->x_dim;i++){
								temp_plane1[j][i]=(float)volume[k][j][i];
							}
						}
					}
				}
	
				/* Interpolate from plane along x */
				if(x_dim1>1){
					unsigned int oversamp=(unsigned int)ceil(fabs(e00));
					double w=2.0*pi/(x_dim1*oversamp/e00);
					AIR_Boolean inited=FALSE;
					unsigned int ifac[20],ifac2[20];
					unsigned int nfft=1;

					while(nfft<=x_dim1+x_dim2*oversamp-1) nfft*=2;
					{
						double x_j=x_k;
						unsigned int j;
						
						for(j=0;j<y_dim1;j++,x_j+=e10){
		
							/* Copy */
							{
								unsigned int i;
		
								for(i=0;i<x_dim1;i++){
									old[2*i]=temp_plane1[j][i];
									old[2*i+1]=0.0;
								}
							}
							/* Interpolate */
							{
								double a=2.0*pi*(e30+k*e20+j*e10)/x_dim1;
								AIR_chirpterp(x_dim1,old,x_dim2,new,nfft,work1,work2,work3,ch,wa2,wa,ifac2,ifac,w,a,oversamp,&inited);
							}
							/* Copy back */
							{
								double x_i=x_j;
								unsigned int i;
		
								for(i=0;i<x_dim2;i++,x_i+=e00){
									if(x_i>=0 && x_i<=x_dim1-1){
										temp_plane1[j][i]=(float)new[2*i];
									}
									else temp_plane1[j][i]=0.0;
								}
							}
						}
					}
				}
	
				/* Interpolate from plane along y */
				{
					unsigned int oversamp=(unsigned int)ceil(fabs(e11));
					double w=2.0*pi/(y_dim1*oversamp/e11);
					AIR_Boolean inited=FALSE;
					unsigned int ifac[20],ifac2[20];
	
					unsigned int nfft=1;

					while(nfft<=y_dim1+y_dim2*oversamp-1) nfft*=2;
					{
						double
							x_i=x_k2,
							y_i=y_k;
						unsigned int i;
						
						for(i=0;i<x_dim2;i++,x_i+=e00i,y_i+=e01){
		
							/*Copy */
							{
								unsigned int j;
		
								for(j=0;j<y_dim1;j++){
									old[2*j]=temp_plane1[j][i];
									old[2*j+1]=0;
								}
							}
							/* Interpolate */
							if(y_dim1>1){
		
								double a=2.0*pi*(e31+k*e21+i*e01)/y_dim1;
								AIR_chirpterp(y_dim1,old,y_dim2,new,nfft,work1,work2,work3,ch,wa2,wa,ifac2,ifac,w,a,oversamp,&inited);
							}
							else new[0]=old[0];
		
							/* Copy back */
							{
								double
									x_j=x_i,
									y_j=y_i;
								unsigned int j;
		
								for(j=0;j<y_dim2;j++,x_j+=e10i,y_j+=e11){
									if(y_j>=0 && y_j<=y_dim1-1 && x_j>=0 && x_j<=x_dim1-1){
										temp_volume1[k][j][i]=(float)new[2*j];
									}
									else temp_volume1[k][j][i]=0.0;
								}
							}
						}
					}
				}
			}
		}
	
		/*Interpolate from float volume along z*/
		{
			double z_j=e32;
			unsigned int j;
	
			for(j=0;j<y_dim2;j++,z_j+=e12){
	
				double z_i=z_j;
				unsigned int i;
	
				for(i=0;i<x_dim2;i++,z_i+=e02){
	
					double z_k=z_i;
					unsigned int k;
	
					for(k=0;k<z_dim2;k++,z_k+=e22){
						if(z_k>=0 && z_k<=z_dim1-1){
	
							unsigned int
								z_up=(unsigned int)ceil(z_k),
								z_down=(unsigned int)floor(z_k);
							double c,f;

							if(z_up==z_down){
								c=0;
								f=1;
							}
							else{
								c=z_k-z_down;
								f=z_up-z_k;
							}
							new[0]=temp_volume1[z_down][j][i]*f+temp_volume1[z_up][j][i]*c;
							new[0]*=scale;
							if(new[0]<0.0) new_volume[k][j][i]=0;
							else{
								new[0]+=.5;
								if(new[0]>(double)AIR_CONFIG_MAX_POSS_VALUE) new_volume[k][j][i]=AIR_CONFIG_MAX_POSS_VALUE;
								else new_volume[k][j][i]=(AIR_Pixels)new[0];
							}
						}
						else new_volume[k][j][i]=0;
					}
				}
			}
		}
	
		free_function(new,wa,wa2,work1,work2,work3,ch,old,temp_volume1,temp_plane1);
		*errcode=0;
		return new_volume;
	}
}
