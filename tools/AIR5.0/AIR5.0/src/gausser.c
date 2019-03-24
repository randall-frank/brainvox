/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 5/23/01 */

/*
 * AIR_Pixels ***gausser()
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

static void free_function(/*@null@*/ /*@out@*/ /*@only@*/ float ***datain2, /*@null@*/ /*@out@*/ /*@only@*/ float ***datain3, /*@null@*/ /*@out@*/ /*@only@*/ double *data1, /*@null@*/ /*@out@*/ /*@only@*/ double *data2, /*@null@*/ /*@out@*/ /*@only@*/ double *data3, /*@null@*/ /*@out@*/ /*@only@*/ double *ch, /*@null@*/ /*@out@*/ /*@only@*/ double *wa)

{
	if(datain2) AIR_free_3f(datain2);
	if(datain3) AIR_free_3f(datain3);
	if(data1) AIR_free_1(data1);
	if(data2) AIR_free_1(data2);
	if(data3) AIR_free_1(data3);
	if(ch) AIR_free_1(ch);
	if(wa) AIR_free_1(wa);
}

AIR_Pixels ***AIR_gausser(AIR_Pixels ***data, const struct AIR_Key_info *datastats, AIR_Pixels ***datamask, float kx, float ky, float kz, const AIR_Boolean rescale, float *scaling, const AIR_Boolean pad, const AIR_Boolean smoothpad, const AIR_Boolean replace, AIR_Error *errcode)

/* if rescale is nonzero, data will be rescaled so hottest pixel=MAX_POSS_VALUE */
/* if pad is nonzero, data will be padded for wraparound protection */
/* if smooth pad is nonzero, padding zeros will be smoothed into the data */
/* if replace is nonzero, input data will be replaced by the smoothed data */
/*  and the returned pointer will be the same as the pointer passed as data */
/*  (make sure not to free data and the returned value if you use this) */

{
	float ***datain2=NULL;
	float ***datain3=NULL;
	double *data1=NULL;
	double *data2=NULL;
	double *data3=NULL;
	double *ch=NULL;
	double *wa=NULL;
	
	AIR_Pixels		***datanew=NULL;

	const unsigned int x_dim=datastats->x_dim;
	const unsigned int y_dim=datastats->y_dim;
	const unsigned int z_dim=datastats->z_dim;
	
	unsigned int nnx,nny,nnz;


	if(kx<0||ky<0||kz<0){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("Smoothing kernels must all be nonnegative numbers\n");
		*errcode=AIR_NEGATIVE_SMOOTHING_ERROR;
		return NULL;
	}
	
	/*Don't smooth along a dimension that is only 1 pixel wide*/
	if(x_dim==1) kx=0;
	if(y_dim==1) ky=0;
	if(z_dim==1) kz=0;

	/*Copy the file to be smoothed into a float volume*/
	datain2=AIR_matrix3f(x_dim,y_dim,z_dim);
	if(!datain2){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("Memory allocation failure\n");
		*errcode=AIR_MEMORY_ALLOCATION_ERROR;
		return NULL;
	}
	{
		unsigned int kk;
		
		for(kk=0;kk<z_dim;kk++){
		
			unsigned int jj;
			
			for(jj=0;jj<y_dim;jj++){
			
				unsigned int ii;
				
				for(ii=0;ii<x_dim;ii++){
					datain2[kk][jj][ii]=(float)data[kk][jj][ii];
				}
			}
		}
	}
	if(datamask){
		/* Create float volume to hold mask */
		datain3=AIR_matrix3f(x_dim,y_dim,z_dim);
		if(!datain3){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("memory allocation failure\n");
			free_function(datain2,datain3,data1,data2,data3,ch,wa);
			*errcode=AIR_MEMORY_ALLOCATION_ERROR;
			return NULL;
		}
	}

	/*Compute size of array needed to pad for wraparound protection and efficient fft */
	if(pad){
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
	}
	else{
		nnx=x_dim;
		nny=y_dim;
		nnz=z_dim;
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
			free_function(datain2,datain3,data1,data2,data3,ch,wa);
			*errcode=AIR_MEMORY_ALLOCATION_ERROR;
			return NULL;
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
				data3[ii]=(double)smoothpad; /*Zero unless 'null' mask chosen*/
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
					free_function(datain2,datain3,data1,data2,data3,ch,wa);
					*errcode=AIR_DQAGE_INTEGRATION_ERROR;
					return NULL;
				}
			}
		}
		if(data2[0]==0.0||data2[1]==0.0){
			data2[0]=1.0;
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("WARNING: x dimension of filter was too small, no x smoothing applied\n");
		}
		/* Exploit symmetry */
		{
			unsigned int ii;
			
			for(ii=1;ii<x_dim/2;ii++){
				data2[nnx-ii]=data2[ii];
			}
		}

		/*Initialize fourier transform arrays for this dimension*/
		AIR_rffti1(nnx,wa,ifac);

		/*Compute forward transform of gaussian*/
		AIR_rfftf1(nnx,data2,ch,wa,ifac);
		
		/*If there is no mask, compute normalization now*/
		if(!datamask){
			/*Initialize arrays*/
			{
				unsigned int ii;
				
				for(ii=x_dim;ii<nnx;ii++){
					data3[ii]=(double)smoothpad;
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
		}

		/*For each row in the dataset*/
		{
			unsigned int kk;
			
			for(kk=0;kk<z_dim;kk++){
			
				unsigned int jj;
				
				for(jj=0;jj<y_dim;jj++){
					if(datamask){
						/*Initialize arrays*/
						{
							unsigned int ii;
							
							for(ii=x_dim;ii<nnx;ii++){
								data1[ii]=0;
								data3[ii]=(double)smoothpad; /*Zero unless 'null' mask chosen*/
							}
						}
						/*Prepare to do fft of that row*/
						{
							unsigned int ii;
							
							for(ii=0;ii<x_dim;ii++){
								if(datamask[kk][jj][ii]==0){
									data1[ii]=0.0;
									data3[ii]=0.0;
								}
								else{
									data1[ii]=datain2[kk][jj][ii];
									data3[ii]=1.0;
								}
							}
						}
		
						/*Do fft */
						AIR_rfftf1(nnx,data1,ch,wa,ifac);
						AIR_rfftf1(nnx,data3,ch,wa,ifac);

						/*Convolve with gaussian*/
						data1[0]*=data2[0];
						data3[0]*=data2[0];

						if(nnx-2*(nnx/2)==0){
							data1[nnx-1]*=data2[nnx-1];
							data3[nnx-1]*=data2[nnx-1];
						}
						{
							double *p1=data1+1;
							double *p2=data2+1;
							double *p3=data3+1;
							unsigned int ii;
							
							for(ii=0;ii<nnx-(nnx/2)-1;ii++,p1+=2,p2+=2,p3+=2){
								{
									double tempr=p1[0]*p2[0]-p1[1]*p2[1];
									double tempi=p1[0]*p2[1]+p1[1]*p2[0];
									p1[0]=tempr;
									p1[1]=tempi;
								}
								{
									double tempr=p3[0]*p2[0]-p3[1]*p2[1];
									double tempi=p3[0]*p2[1]+p3[1]*p2[0];
									p3[0]=tempr;
									p3[1]=tempi;
								}
							}
						}
						/* Do ifft */
						AIR_rfftb1(nnx,data1,ch,wa,ifac);
						AIR_rfftb1(nnx,data3,ch,wa,ifac);
					}
					else{
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
								data1[ii]=datain2[kk][jj][ii];
							}
						}
						/*Do fft */
						AIR_rfftf1(nnx,data1,ch,wa,ifac);

						/*Convolve with gaussian*/
						data1[0]*=data2[0];

						if(nnx-2*(nnx/2)==0){
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
					}

					/*Put results back into datain2 */
					if(datamask){
						
						unsigned int ii;
						
						for(ii=0;ii<x_dim;ii++){
							datain2[kk][jj][ii]=data1[ii];
							datain3[kk][jj][ii]=data3[ii];
						}
					}
					else{
					
						unsigned int ii;
						
						for(ii=0;ii<x_dim;ii++){
							if(data3[ii]==0.0){
								datain2[kk][jj][ii]=0.0;
							}
							else{
								datain2[kk][jj][ii]=data1[ii]/data3[ii];
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
				data3[jj]=(double)smoothpad; /*Zero unless 'null' mask chosen*/
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
					free_function(datain2,datain3,data1,data2,data3,ch,wa);
					*errcode=AIR_DQAGE_INTEGRATION_ERROR;					
					return NULL;
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
		
		/*If there is no mask, compute normalization now*/
		if(!datamask){
			/*Initialize arrays*/
			{
				unsigned int jj;
				
				for(jj=y_dim;jj<nny;jj++){
					data3[jj]=(double)smoothpad;
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
		}

		/*For each row in the dataset*/
		{
			unsigned int kk;
			
			for(kk=0;kk<z_dim;kk++){
			
				unsigned int ii;
				
				for(ii=0;ii<x_dim;ii++){
					if(datamask){
						/*Initialize arrays*/
						{
							unsigned int jj;
							
							for(jj=y_dim;jj<nny;jj++){
								data1[jj]=0;
								data3[jj]=(double)smoothpad; /*Zero unless 'null' mask chosen*/
							}
						}
						/*Prepare to do fft of that row*/
						{
							unsigned int jj;
							
							for(jj=0;jj<y_dim;jj++){
								data1[jj]=datain2[kk][jj][ii];
								data3[jj]=datain3[kk][jj][ii];
							}
						}
						/*Do fft */
						AIR_rfftf1(nny,data1,ch,wa,ifac);
						AIR_rfftf1(nny,data3,ch,wa,ifac);

						/*Convolve with gaussian*/
						data1[0]*=data2[0];
						data3[0]*=data2[0];

						if(nny%2==0){
							data1[nny-1]*=data2[nny-1];
							data3[nny-1]*=data2[nny-1];
						}
						{
							double *p1=data1+1;
							double *p2=data2+1;
							double *p3=data3+1;
							unsigned int jj;
							
							for(jj=0;jj<nny-(nny/2)-1;jj++,p1+=2,p2+=2,p3+=2){
								{
									double tempr=p1[0]*p2[0]-p1[1]*p2[1];
									double tempi=p1[0]*p2[1]+p1[1]*p2[0];
									p1[0]=tempr;
									p1[1]=tempi;
								}
								{
									double tempr=p3[0]*p2[0]-p3[1]*p2[1];
									double tempi=p3[0]*p2[1]+p3[1]*p2[0];
									p3[0]=tempr;
									p3[1]=tempi;
								}
							}
						}
						/* Do ifft */
						AIR_rfftb1(nny,data1,ch,wa,ifac);
						AIR_rfftb1(nny,data3,ch,wa,ifac);
					}
					else{
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
								data1[jj]=datain2[kk][jj][ii];
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
					}

					/*Put results back into datain2 */
					if(datamask){
						unsigned int jj;
						
						for(jj=0;jj<y_dim;jj++){
							datain2[kk][jj][ii]=data1[jj];
							datain3[kk][jj][ii]=data3[jj];
						}
					}
					else{
					
						unsigned int jj;
						
						for(jj=0;jj<y_dim;jj++){
							if(data3[jj]==0.0){
								datain2[kk][jj][ii]=0.0;
							}
							else{
								datain2[kk][jj][ii]=data1[jj]/data3[jj];
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
				data3[kk]=(double)smoothpad; /*Zero unless 'null' mask chosen*/
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
					free_function(datain2,datain3,data1,data2,data3,ch,wa);
					*errcode=AIR_DQAGE_INTEGRATION_ERROR;
					return NULL;
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
		
		/*If there is no mask, compute normalization now*/
		if(!datamask){
			/*Initialize arrays*/
			{
				unsigned int kk;
				
				for(kk=z_dim;kk<nnz;kk++){
					data3[kk]=(double)smoothpad;
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
				{
					unsigned int kk;
					
					for(kk=0;kk<nnz-(nnz/2)-1;kk++,p2+=2,p3+=2){
						double tempr=p3[0]*p2[0]-p3[1]*p2[1];
						double tempi=p3[0]*p2[1]+p3[1]*p2[0];
						p3[0]=tempr;
						p3[1]=tempi;
					}
				}
			}
			/* Do ifft */
			AIR_rfftb1(nnz,data3,ch,wa,ifac);
		}

		/*For each row in the dataset*/
		{
			unsigned int jj;
			
			for(jj=0;jj<y_dim;jj++){
			
				unsigned int ii;
				
				for(ii=0;ii<x_dim;ii++){
					if(datamask){
						/*Initialize arrays*/
						{
							unsigned int kk;
							
							for(kk=z_dim;kk<nnz;kk++){
								data1[kk]=0;
								data3[kk]=(double)smoothpad; /*Zero unless 'null' mask chosen*/
							}
						}
						/*Prepare to do fft of that row*/
						{
							unsigned int kk;
							
							for(kk=0;kk<z_dim;kk++){
								data1[kk]=datain2[kk][jj][ii];
								data3[kk]=datain3[kk][jj][ii];
							}
						}
						/*Do fft */
						AIR_rfftf1(nnz,data1,ch,wa,ifac);
						AIR_rfftf1(nnz,data3,ch,wa,ifac);

						/*Convolve with gaussian*/
						data1[0]*=data2[0];
						data3[0]*=data2[0];

						if(nnz%2==0){
							data1[nnz-1]*=data2[nnz-1];
							data3[nnz-1]*=data2[nnz-1];
						}
						{
							double *p1=data1+1;
							double *p2=data2+1;
							double *p3=data3+1;
							unsigned int kk;
							
							for(kk=0;kk<nnz-(nnz/2)-1;kk++,p1+=2,p2+=2,p3+=2){
								{
									double tempr=p1[0]*p2[0]-p1[1]*p2[1];
									double tempi=p1[0]*p2[1]+p1[1]*p2[0];
									p1[0]=tempr;
									p1[1]=tempi;
								}
								{
									double tempr=p3[0]*p2[0]-p3[1]*p2[1];
									double tempi=p3[0]*p2[1]+p3[1]*p2[0];
									p3[0]=tempr;
									p3[1]=tempi;
								}
							}
						}
						/* Do ifft */
						AIR_rfftb1(nnz,data1,ch,wa,ifac);
						AIR_rfftb1(nnz,data3,ch,wa,ifac);
					}
					else{
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
								data1[kk]=datain2[kk][jj][ii];
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
					}

					/*Put results back into datain2 */
					if(datamask){
					
						unsigned int kk;
						
						for(kk=0;kk<z_dim;kk++){
							if(datamask[kk][jj][ii]==0){
								datain2[kk][jj][ii]=0.0;
							}
							else{
								if(data3[kk]==0.0){
									datain2[kk][jj][ii]=0.0;
								}
								else{
									datain2[kk][jj][ii]=data1[kk]/data3[kk];
								}
							}
						}
					}
					else{

						unsigned int kk;
						
						for(kk=0;kk<z_dim;kk++){
							if(data3[kk]==0.0){
								datain2[kk][jj][ii]=0.0;
							}
							else{
								datain2[kk][jj][ii]=data1[kk]/data3[kk];
							}
						}
					}
				}
			}
		}
	}
	{
		float max;
		
		/* Deal with scaling issues */
		if(rescale){
			max=0.0;
			{
				unsigned int kk;
				
				for(kk=0;kk<z_dim;kk++){
				
					unsigned int jj;
					
					for(jj=0;jj<y_dim;jj++){
					
						unsigned int ii;
						
						for(ii=0;ii<x_dim;ii++){
							if(datain2[kk][jj][ii]>max) max=datain2[kk][jj][ii];
						}
					}
				}
			}
			if(scaling){
				*scaling=(float)AIR_CONFIG_MAX_POSS_VALUE/max;
			}
		}
		else{
			max=(float)AIR_CONFIG_MAX_POSS_VALUE;
			if(scaling){
				*scaling=1.0;
			}
		}
		
		/*Copy results into return volume*/
		if(replace){
			{
				unsigned int kk;
				
				for(kk=0;kk<z_dim;kk++){
				
					unsigned int jj;
					
					for(jj=0;jj<y_dim;jj++){
					
						unsigned int ii;
						
						for(ii=0;ii<x_dim;ii++){
							if(datain2[kk][jj][ii]<0.0){
								data[kk][jj][ii]=0;
							}
							else{
								float temp=AIR_CONFIG_MAX_POSS_VALUE*(datain2[kk][jj][ii]/max)+.5;
								if(temp>(float)AIR_CONFIG_MAX_POSS_VALUE){
									data[kk][jj][ii]=AIR_CONFIG_MAX_POSS_VALUE;
								}
								else data[kk][jj][ii]=(AIR_Pixels)temp;
							}
						}
					}
				}
			}
			free_function(datain2,datain3,data1,data2,data3,ch,wa);
			return data;
		}
		else{
			datanew=AIR_create_vol3(x_dim,y_dim,z_dim);
			if(!datanew){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("memory allocation failure\n");
				free_function(datain2,datain3,data1,data2,data3,ch,wa);
				*errcode=AIR_MEMORY_ALLOCATION_ERROR;
				return NULL;
			}
			{
				unsigned int kk;
				
				for(kk=0;kk<z_dim;kk++){
				
					unsigned int jj;
					
					for(jj=0;jj<y_dim;jj++){
					
						unsigned int ii;
						
						for(ii=0;ii<x_dim;ii++){
							if(datain2[kk][jj][ii]<0.0){
								datanew[kk][jj][ii]=0;
							}
							else{
								float temp=AIR_CONFIG_MAX_POSS_VALUE*(datain2[kk][jj][ii]/max)+.5;
								if(temp>(float)AIR_CONFIG_MAX_POSS_VALUE){
									datanew[kk][jj][ii]=AIR_CONFIG_MAX_POSS_VALUE;
								}
								else{
									datanew[kk][jj][ii]=(AIR_Pixels)temp;
								}
							}
						}
					}
				}
			}
			free_function(datain2,datain3,data1,data2,data3,ch,wa);
			return datanew;
		}
	}
}

