/* Copyright 1996-2001 Roger P. Woods, M.D. */
/* Modified 5/22/01 */

/*
 * This implements chirp-z interpolation.
 *
 * The general method of chirp-z interpolation is described in:
 *
 * Rabiner LR, Schafer RW, Rader CM. The Bell System Technical
 * Journal 1969;48:1249-1292
 *
 * but note that this implementation differs from the one described there.
 *
 */


#include "AIR.h"

void AIR_chirpterp(const unsigned int m, double *in, const unsigned int k, double *out, const unsigned int nfft, double *work1, double *work2, double *work3, double *ch, double *wa2, double *wa, unsigned int *ifac2, unsigned int *ifac, const double w, const double a, const unsigned int oversamp, AIR_Boolean *inited)

/* m=length of input array */
/* in=input array */
/* k=length of output array */
/* out=output array */
/* nfft=power of 2 (or highly composite number) >= m+k-1 */
/* work1 work array for me of size nfft */
/* work2 another array for me of size nfft */
/* work3 yet another array for me of size nfft */
/* ch work array for fft of size nfft */
/* wa2 work array for fft of size m */
/* wa work array for fft of size nfft */
/* ifac2=work array of length>=15 inited for size m */
/* ifac=work array of length>=15 inited for size nfft*/
/* w=spacing angle (radians) */
/* a=shifting angle (radians) */
/* oversamp=factor by which data is being oversampled */
/* *inited=0 if this function has not been called before */

{
	/* Do fft of input array */
	if(!(*inited)) AIR_cffti1(m,wa2,ifac2);
	AIR_cfftf1(m,in,ch,wa2,ifac2);


	/* Apply the shift */
	{
		double *inptr=in+2;
		double *inpts=in+3;
		double *outptr=in+2*m-2;
		double *outpts=in+2*m-1;
		unsigned int i=1;
		
		while(inptr<outptr){
		
			double tempr=cos(a*i);
			double tempi=sin(a*i);
			{
				double temp2r=*inptr;
				double temp2i=*inpts;
				*inptr=temp2r*tempr-temp2i*tempi;
				*inpts=temp2r*tempi+temp2i*tempr;
			}
			tempi*=-1;
			{
				double temp2r=*outptr;
				double temp2i=*outpts;
				*outptr=temp2r*tempr-temp2i*tempi;
				*outpts=temp2r*tempi+temp2i*tempr;
			}
			i++;
			inptr+=2;
			inpts+=2;
			outptr-=2;
			outpts-=2;
		}
		if(inptr==outptr){
			double tempr=cos(a*i);
			*inptr*=tempr;
			*inpts*=tempr;
		}
	}
	{
		unsigned int index=m/2;

		{
			double *outptr;
			
			if(2*(m/2)==m){
				double *inptr=in+m;
				outptr=work2;

				*outptr++=*inptr++/2.0;	/* Real */
				*outptr++=*inptr++/2.0;	/* Imaginary */

				while(inptr<in+2*m) *outptr++=*inptr++;	/* Real and Imaginary */
				inptr=in;
				while(inptr<in+m) *outptr++=*inptr++;	/* Real and Imaginary */

				*outptr++=*inptr++/2.0;	/* Real */
				*outptr++=*inptr++/2.0;	/* Imaginary */
			}
			else{
				double *inptr=in+m+1;
				outptr=work2;

				while(inptr<in+2*m) *outptr++=*inptr++;	/* Real and Imaginary */
				inptr=in;
				while(inptr<in+m+1) *outptr++=*inptr++;	/* Real and Imaginary */
			}
			while(outptr<work2+2*nfft) *outptr++=0.0;	/* Real and Imaginary */
		}

		/* Create the chirp filter in work1 */
		if(!(*inited)){
			{
				double *outptr;
			
				for(outptr=work1;outptr<work1+2*nfft;outptr++) *outptr=0.0; /* You really do need this line */
			}
			{
				double *outptr,*outpts;
				unsigned int i,j;
				
				for(i=index,j=0,outptr=work1,outpts=work1+1;j<k*oversamp;i++,j++,outptr+=2,outpts+=2){
					double temp=(-1.0)*i*i/2.0;
					*outptr=cos(w*temp);
					*outpts=sin(w*temp);
				}
			}
			if(index!=0){
				double *outptr=work1+2*nfft-2;
				double *outpts=work1+2*nfft-1;
				unsigned int i,j;
				
				for(i=index-1,j=0; i!=0;i--,j++,outptr-=2,outpts-=2){
					double temp=(-1.0)*i*i/2.0;
					*outptr=cos(w*temp);
					*outpts=sin(w*temp);
				}
				
				for(;j<2*index;i++,j++,outptr-=2,outpts-=2){
					double temp=(-1.0)*i*i/2.0;
					*outptr=cos(w*temp);
					*outpts=sin(w*temp);
				}
			}

			/* Create the chirp adjustment in work3 */
			{
				double *outptr,*outpts;
				unsigned int i=index;
				
				for(outptr=work3,outpts=work3+1;i!=0;i--,outptr+=2,outpts+=2){
					double temp=(double)i*i/2.0;
					*outptr=cos(w*temp);
					*outpts=sin(w*temp);
				}
				
				for(;i<=index;i++,outptr+=2,outpts+=2){
					double temp=(double)i*i/2.0;
					*outptr=cos(w*temp);
					*outpts=sin(w*temp);
				}
			}
		}

		/* Adjust values in work2 using required filter */
		{
			double *outptr,*outpts,*inptr,*inpts;
			unsigned int i=index;
			
			for(outptr=work2,outpts=work2+1,inptr=work3,inpts=work3+1;i!=0;i--,outptr+=2,outpts+=2,inptr+=2,inpts+=2){
				double tempr=*outptr;
				double tempi=*outpts;
				*outptr=*inptr*tempr-*inpts*tempi;
				*outpts=*inptr*tempi+*inpts*tempr;
			}
			
			for(;i<=index;i++,outptr+=2,outpts+=2,inptr+=2,inpts+=2){
				double tempr=*outptr;
				double tempi=*outpts;
				*outptr=*inptr*tempr-*inpts*tempi;
				*outpts=*inptr*tempi+*inpts*tempr;
			}
		}
	}

	/* Do FFT's */
	if(!(*inited)){
		AIR_cffti1(nfft,wa,ifac);
		AIR_cfftf1(nfft,work1,ch,wa,ifac);
	}
	AIR_cfftf1(nfft,work2,ch,wa,ifac);

	/* Multiply FFT's to convolve */
	{
		double *inptr, *inpts, *outptr, *outpts;
		unsigned int i;
		
		for(i=0,inptr=work1,inpts=work1+1,outptr=work2,outpts=work2+1;i<nfft;i++,inptr+=2,inpts+=2,outptr+=2,outpts+=2){
			double tempr=*outptr;
			double tempi=*outpts;
			*outptr=tempr**inptr-tempi**inpts;
			*outpts=tempr**inpts+tempi**inptr;
		}
	}

	/* Invert the FFT */
	AIR_cfftb1(nfft,work2,ch,wa,ifac);
	{
		double *outptr;
		
		for(outptr=work2;outptr<work2+2*nfft;outptr++) *outptr/=nfft;
	}

	/* Compute output */
	{
		double *inptr=work2;
		double *inpts=work2+1;
		double *outptr=out;
		double *outpts=out+1;
		unsigned int i;
		
		for(i=0;i<k*oversamp;i+=oversamp,inptr+=2*oversamp,inpts+=2*oversamp,outptr+=2,outpts+=2){
			double temp=(double)i*i/2.0;
			double tempr=cos(w*temp);
			double tempi=sin(w*temp);
			*outptr=(*inptr*tempr-*inpts*tempi)/m;
			*outpts=(*inptr*tempi+*inpts*tempr)/m;
		}
	}
	*inited=TRUE;
}
