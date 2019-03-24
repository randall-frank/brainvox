/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 5/23/01 */

/*
 * Does orthogonal Fourier smoothing with a Gaussian filter
 *
 * The filter widths should be provided in real world units
 * (i.e., not in terms of pixels unless the real world pixels
 * are exactly one real world unit wide).
 *
 * Areas that have a value of zero in the mask will have zero
 *  values on output, and these areas will not be smoothed into
 *  adjacent non-zero areas.
 *
 * It is allowed for the mask to be a NULL pointer, in which
 * case all points within the original data matrix are included
 * and the routine will execute almost twice as fast.		
 */

#include "AIR.h"

#define LIMIT 100
#define EPSABS 1.0e-14
#define EPSREL 1.0e-11

static double bell(const double x)

{
	return exp(-x*x/2.0);
}


static void free_function(/*@null@*/ /*@out@*/ /*@only@*/ double *data1, /*@null@*/ /*@out@*/ /*@only@*/ double *data2, /*@null@*/ /*@out@*/ /*@only@*/ double *data3, /*@null@*/ /*@out@*/ /*@only@*/ double *ch, /*@null@*/ /*@out@*/ /*@only@*/ double *wa)

{
	if(data1) AIR_free_1(data1);
	if(data2) AIR_free_1(data2);
	if(data3) AIR_free_1(data3);
	if(ch) AIR_free_1(ch);
	if(wa) AIR_free_1(wa);
}

AIR_Error AIR_gausssmaller(AIR_Pixels ***data, const struct AIR_Key_info *datastats, float kx, float ky, float kz)

{
	double *data1=NULL;
	double *data2=NULL;
	double *data3=NULL;
	double *ch=NULL;
	double *wa=NULL;
	
	const unsigned int x_dim=datastats->x_dim;
	const unsigned int y_dim=datastats->y_dim;
	const unsigned int z_dim=datastats->z_dim;
	
	unsigned int nnx,nny,nnz;



	if(kx<0||ky<0||kz<0){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("Smoothing kernels must all be nonnegative numbers\n");
		return AIR_NEGATIVE_SMOOTHING_ERROR;
	}
	
	/*Don't smooth along a dimension that is only 1 pixel wide*/
	if(x_dim==1) kx=0;
	if(y_dim==1) ky=0;
	if(z_dim==1) kz=0;

	

	/*Compute size of array needed to pad for wraparound protection and efficient fft */

	nnx=2*x_dim;
	/* Extra padding to speed up fft */
	if(AIR_pfactor(nnx)>5){
		do{
			nnx++;
		}while(AIR_pfactor(nnx)>5);
	}
	nny=2*y_dim;
	/* Extra padding to speed up fft */
	if(AIR_pfactor(nny)>5){
		do{
			nny++;
		}while(AIR_pfactor(nny)>5);
	}
	nnz=2*z_dim;
	/* Extra padding to speed up fft */
	if(AIR_pfactor(nnz)>5){
		do{
			nnz++;
		}while(AIR_pfactor(nnz)>5);
	}
	{
		unsigned int nnmax=0;
		if(nnx>nnmax && kx!=0.0) nnmax=nnx;
		if(nny>nnmax && ky!=0.0) nnmax=nny;
		if(nnz>nnmax && kz!=0.0) nnmax=nnz;

		/*Allocate the data array*/
		data1=AIR_matrix1(nnmax);
		if(data1){
			data2=AIR_matrix1(nnmax);
			if(data2){
				data3=AIR_matrix1(nnmax);
				if(data3){
					ch=AIR_matrix1(nnmax);
					if(ch){
						wa=AIR_matrix1(nnmax);
					}
				}
			}
		}
		if(!wa){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("memory allocation failure\n");
			free_function(data1,data2,data3,ch,wa);
			return AIR_MEMORY_ALLOCATION_ERROR;
		}
	}
	if(kx!=0.0){
	
		unsigned int ifac[20];
		/* Smooth along x */

		/*Initialize arrays*/
		{
			unsigned int ii;
			
			for(ii=0;ii<nnx;ii++){
				data1[ii]=0;
				data2[ii]=0;
				data3[ii]=0; /*Zero unless 'null' mask chosen*/
				ch[ii]=0;
				wa[ii]=0;
			}
		}
		kx/=(2.0*datastats->x_size);
		kx=sqrt(-2.0*log(.5))/kx;

		/*Compute the gaussian for the x dimension*/
		{
			unsigned int ii;
			
			for(ii=0;ii<x_dim/2;ii++){
				double alist[LIMIT],blist[LIMIT],rlist[LIMIT],elist[LIMIT];
				unsigned int iord[LIMIT];
				unsigned int last;
				unsigned int neval;
				unsigned int ier;
				double abserr;

				data2[ii]=AIR_dqage(bell,(ii-0.5)*kx,(ii+0.5)*kx,EPSABS,EPSREL,LIMIT,&abserr,&neval,&ier,alist,blist,rlist,elist,iord,&last);
				if(ier!=0){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("Failed to integrate gaussian to required accuracy\n");
					free_function(data1,data2,data3,ch,wa);
					return AIR_DQAGE_INTEGRATION_ERROR;
				}
			}
		}
		if(data2[0]==0.0||data2[1]==0.0){
			data2[0]=1.0;
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("WARNING: x dimension of filter was too small, no x smoothing applied\n");
		}
		{
			unsigned int ii;
			
			/* Exploit symmetry */
			for(ii=1;ii<x_dim/2;ii++){
				data2[nnx-ii]=data2[ii];
			}
		}

		/*Initialize fourier transform arrays for this dimension*/
		AIR_rffti1(nnx,wa,ifac);

		/*Compute forward transform of gaussian*/
		AIR_rfftf1(nnx,data2,ch,wa,ifac);
		
		/*Since there is no mask, compute normalization now*/

		/*Initialize arrays*/
		{
			unsigned int ii;
			
			for(ii=x_dim;ii<nnx;ii++){
				data3[ii]=0;
			}
		}
		/*Prepare to do fft*/
		{	
			unsigned int ii;
			

			for(ii=0;ii<x_dim;ii++){
				data3[ii]=1.0;
			}
		}	
		/*Do fft */
		AIR_rfftf1(nnx,data3,ch,wa,ifac);
			
		/*Convolve with gaussian*/
		data3[0]*=data2[0];
		if(nnx%2==0){
			data3[nnx-1]*=data2[nnx-1];
		}
		{
			double *p2=data2+1;
			double *p3=data3+1;
			unsigned int ii;
			
			for(ii=0;ii<nnx-(nnx/2)-1;ii++,p2+=2,p3+=2){
				double tempr=p3[0]*p2[0]-p3[1]*p2[1];
				double tempi=p3[0]*p2[1]+p3[1]*p2[0];
				p3[0]=tempr;
				p3[1]=tempi;
			}
		}
		/* Do ifft */
		AIR_rfftb1(nnx,data3,ch,wa,ifac);

		/*For each row in the dataset*/
		{
			unsigned int kk;
			
			for(kk=0;kk<z_dim;kk++){
			
				unsigned int jj;
				
				for(jj=0;jj<y_dim;jj++){

					/*Initialize array*/
					{
						unsigned int ii;
						
						for(ii=x_dim;ii<nnx;ii++){
							data1[ii]=0;
						}
					}
					/*Prepare to do fft of that row*/
					{
						unsigned int ii;
						
						for(ii=0;ii<x_dim;ii++){
							data1[ii]=(double)data[kk][jj][ii];
						}
					}
					/*Do fft */
					AIR_rfftf1(nnx,data1,ch,wa,ifac);

					/*Convolve with gaussian*/
					data1[0]*=data2[0];

					if(nnx%2==0){
						data1[nnx-1]*=data2[nnx-1];
					}
					{
						double *p1=data1+1;
						double *p2=data2+1;
						unsigned int ii;
						
						for(ii=0;ii<nnx-(nnx/2)-1;ii++,p1+=2,p2+=2){
							double tempr=p1[0]*p2[0]-p1[1]*p2[1];
							double tempi=p1[0]*p2[1]+p1[1]*p2[0];
							p1[0]=tempr;
							p1[1]=tempi;
						}
					}
					/* Do ifft */
					AIR_rfftb1(nnx,data1,ch,wa,ifac);


					/*Put results back into data */
					{
						unsigned int ii;
						
						for(ii=0;ii<x_dim;ii++){
							if(data3[ii]==0.0){
								data[kk][jj][ii]=(AIR_Pixels)0.0;
							}
							else{
								data[kk][jj][ii]=(AIR_Pixels)(data1[ii]/data3[ii]+.5);
							}
						}
					}
				}
			}
		}
	}
	if(ky!=0.0){
	
		unsigned int ifac[20];
		/* Smooth along y */

		/*Initialize arrays*/
		{
			unsigned int jj;
			
			for(jj=0;jj<nny;jj++){
				data1[jj]=0;
				data2[jj]=0;
				data3[jj]=0; /*Zero unless 'null' mask chosen*/
				ch[jj]=0;
				wa[jj]=0;
			}
		}
		ky/=(2.0*datastats->y_size);
		ky=sqrt(-2.0*log(.5))/ky;

		/*Compute the gaussian for the y dimension*/
		{
			unsigned int jj;
			
			for(jj=0;jj<y_dim/2;jj++){

				double alist[LIMIT],blist[LIMIT],rlist[LIMIT],elist[LIMIT];
				unsigned int iord[LIMIT];
				unsigned int last;
				unsigned int neval;
				unsigned int ier;
				double abserr;
				
				data2[jj]=AIR_dqage(bell,(jj-0.5)*ky,(jj+0.5)*ky,EPSABS,EPSREL,LIMIT,&abserr,&neval,&ier,alist,blist,rlist,elist,iord,&last);
				if(ier!=0){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("Failed to integrate gaussian to required accuracy\n");
					free_function(data1,data2,data3,ch,wa);
					return AIR_DQAGE_INTEGRATION_ERROR;
				}
			}
		}
		if(data2[0]==0.0||data2[1]==0.0){
			data2[0]=1.0;
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("WARNING: y dimension of filter was too small, no y smoothing applied\n");
		}
		/* Exploit symmetry */
		{
			unsigned int jj;
			
			for(jj=1;jj<y_dim/2;jj++){
				data2[nny-jj]=data2[jj];
			}
		}
		/*Initialize fourier transform arrays for this dimension*/
		AIR_rffti1(nny,wa,ifac);

		/*Compute forward transform of gaussian*/
		AIR_rfftf1(nny,data2,ch,wa,ifac);
		
		/*Since there is no mask, compute normalization now*/
		/*Initialize arrays*/
		{
			unsigned int jj;
			
			for(jj=y_dim;jj<nny;jj++){
				data3[jj]=0;
			}
		}
		/*Prepare to do fft*/
		{
			unsigned int jj;
			
			for(jj=0;jj<y_dim;jj++){
				data3[jj]=1.0;
			}
		}
		/*Do fft */
		AIR_rfftf1(nny,data3,ch,wa,ifac);
			
		/*Convolve with gaussian*/
		data3[0]*=data2[0];
		if(nny%2==0){
			data3[nny-1]*=data2[nny-1];
		}
		{
			double *p2=data2+1;
			double *p3=data3+1;
			unsigned int jj;
			
			for(jj=0;jj<nny-(nny/2)-1;jj++,p2+=2,p3+=2){
				double tempr=p3[0]*p2[0]-p3[1]*p2[1];
				double tempi=p3[0]*p2[1]+p3[1]*p2[0];
				p3[0]=tempr;
				p3[1]=tempi;
			}
		}	
		/* Do ifft */
		AIR_rfftb1(nny,data3,ch,wa,ifac);


		/*For each row in the dataset*/
		{
			unsigned int kk;
			
			for(kk=0;kk<z_dim;kk++){
			
				unsigned int ii;
				
				for(ii=0;ii<x_dim;ii++){

					/*Initialize array*/
					{
						unsigned int jj;
						
						for(jj=y_dim;jj<nny;jj++){
							data1[jj]=0;
						}
					}
					/*Prepare to do fft of that row*/
					{
						unsigned int jj;
						
						for(jj=0;jj<y_dim;jj++){
							data1[jj]=(double)data[kk][jj][ii];
						}
					}
					/*Do fft */
					AIR_rfftf1(nny,data1,ch,wa,ifac);

					/*Convolve with gaussian*/
					data1[0]*=data2[0];

					if(nny%2==0){
						data1[nny-1]*=data2[nny-1];
					}
					{
						double *p1=data1+1;
						double *p2=data2+1;
						unsigned int jj;
						
						for(jj=0;jj<nny-(nny/2)-1;jj++,p1+=2,p2+=2){
							double tempr=p1[0]*p2[0]-p1[1]*p2[1];
							double tempi=p1[0]*p2[1]+p1[1]*p2[0];
							p1[0]=tempr;
							p1[1]=tempi;
						}
					}
					/* Do ifft */
					AIR_rfftb1(nny,data1,ch,wa,ifac);


					/*Put results back into data */
					{
						unsigned int jj;
						
						for(jj=0;jj<y_dim;jj++){
							if(data3[jj]==0.0){
								data[kk][jj][ii]=(AIR_Pixels)0.0;
							}
							else{
								data[kk][jj][ii]=(AIR_Pixels)(data1[jj]/data3[jj]+.5);
							}
						}
					}
				}
			}
		}
	}
	if(kz!=0.0){
	
		unsigned int ifac[20];
		/* Smooth along z */

		/*Initialize arrays*/
		{
			unsigned int kk;
			
			for(kk=0;kk<nnz;kk++){
				data1[kk]=0;
				data2[kk]=0;
				data3[kk]=0; /*Zero unless 'null' mask chosen*/
				ch[kk]=0;
				wa[kk]=0;
			}
		}
		kz/=(2.0*datastats->z_size);
		kz=sqrt(-2.0*log(.5))/kz;

		/*Compute the gaussian for the z dimension*/
		{
			unsigned int kk;
			
			for(kk=0;kk<z_dim/2;kk++){
			
				double alist[LIMIT],blist[LIMIT],rlist[LIMIT],elist[LIMIT];
				unsigned int iord[LIMIT];
				unsigned int last;
				unsigned int neval;
				unsigned int ier;
				double abserr;
				
				data2[kk]=AIR_dqage(bell,(kk-0.5)*kz,(kk+0.5)*kz,EPSABS,EPSREL,LIMIT,&abserr,&neval,&ier,alist,blist,rlist,elist,iord,&last);
				if(ier!=0){
						printf("%s: %d: ",__FILE__,__LINE__);
						printf("Failed to integrate gaussian to required accuracy\n");
						free_function(data1,data2,data3,ch,wa);
						return AIR_DQAGE_INTEGRATION_ERROR;
				}
			}
		}
		if(data2[0]==0.0||data2[1]==0.0){
			data2[0]=1.0;
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("WARNING: z dimension of filter was too small, no z smoothing applied\n");
		}
		/* Exploit symmetry */
		{
			unsigned int kk;
			
			for(kk=1;kk<z_dim/2;kk++){
				data2[nnz-kk]=data2[kk];
			}
		}
		/*Initialize fourier transform arrays for this dimension*/
		AIR_rffti1(nnz,wa,ifac);

		/*Compute forward transform of gaussian*/
		AIR_rfftf1(nnz,data2,ch,wa,ifac);
		
		/*Since there is no mask, compute normalization now*/

		/*Initialize arrays*/
		{
			unsigned int kk;
			
			for(kk=z_dim;kk<nnz;kk++){
				data3[kk]=0;
			}
		}
		/*Prepare to do fft*/
		{
			unsigned int kk;
			
			for(kk=0;kk<z_dim;kk++){
				data3[kk]=1.0;
			}
		}
		/*Do fft */
		AIR_rfftf1(nnz,data3,ch,wa,ifac);
			
		/*Convolve with gaussian*/
		data3[0]*=data2[0];
		if(nnz%2==0){
			data3[nnz-1]*=data2[nnz-1];
		}
		{
			double *p2=data2+1;
			double *p3=data3+1;
			unsigned int kk;
			
			for(kk=0;kk<nnz-(nnz/2)-1;kk++,p2+=2,p3+=2){
				double tempr=p3[0]*p2[0]-p3[1]*p2[1];
				double tempi=p3[0]*p2[1]+p3[1]*p2[0];
				p3[0]=tempr;
				p3[1]=tempi;
			}
		}	
		/* Do ifft */
		AIR_rfftb1(nnz,data3,ch,wa,ifac);
		

		/*For each row in the dataset*/
		{
			unsigned int jj;
			
			for(jj=0;jj<y_dim;jj++){
			
				unsigned int ii;
				
				for(ii=0;ii<x_dim;ii++){

					/*Initialize array*/
					{
						unsigned int kk;
						
						for(kk=z_dim;kk<nnz;kk++){
							data1[kk]=0;
						}
					}
					/*Prepare to do fft of that row*/
					{
						unsigned int kk;
						
						for(kk=0;kk<z_dim;kk++){
							data1[kk]=(double)data[kk][jj][ii];
						}
					}
					/*Do fft */
					AIR_rfftf1(nnz,data1,ch,wa,ifac);

					/*Convolve with gaussian*/
					data1[0]*=data2[0];

					if(nnz%2==0){
						data1[nnz-1]*=data2[nnz-1];
					}
					{
						double *p1=data1+1;
						double *p2=data2+1;
						unsigned int kk;
						
						for(kk=0;kk<nnz-(nnz/2)-1;kk++,p1+=2,p2+=2){
							double tempr=p1[0]*p2[0]-p1[1]*p2[1];
							double tempi=p1[0]*p2[1]+p1[1]*p2[0];
							p1[0]=tempr;
							p1[1]=tempi;
						}
					}
					/* Do ifft */
					AIR_rfftb1(nnz,data1,ch,wa,ifac);


					/*Put results back into data */
					{
						unsigned int kk;
						
						for(kk=0;kk<z_dim;kk++){
							if(data3[kk]==0.0){
								data[kk][jj][ii]=(AIR_Pixels)0.0;
							}
							else{
								data[kk][jj][ii]=(AIR_Pixels)(data1[kk]/data3[kk]+.5);
							}
						}
					}
				}
			}
		}
	}
		
	free_function(data1,data2,data3,ch,wa);
	return 0;
}

