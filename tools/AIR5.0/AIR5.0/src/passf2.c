/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/4/01 */

#include "AIR.h"

/* Calculate the fast Fourier transform of complex subvectors of length two */

void AIR_passf2(const unsigned int ido, const unsigned int l1, double *cc, double *ch, const double *wa1)

{
	const unsigned int ccz=2*ido;
	const unsigned int chz=l1*ido;

	if(ido<=2){
		
		unsigned int k;
		double *ch0k,*ch1k,*cck0,*cck1;
		
		for(k=0,ch0k=ch,ch1k=ch0k+chz,cck0=cc,cck1=cck0+ido;k<l1;k++,ch0k+=ido,ch1k+=ido,cck0+=ccz,cck1+=ccz){
			ch0k[0]=cck0[0]+cck1[0];
			ch1k[0]=cck0[0]-cck1[0];
			ch0k[1]=cck0[1]+cck1[1];
			ch1k[1]=cck0[1]-cck1[1];
		}
		return;
	}

	if(ido/2>=l1){
	
		unsigned int k;
		double *ch0k,*ch1k,*cck0,*cck1;
		
		for(k=0,ch0k=ch,ch1k=ch0k+chz,cck0=cc,cck1=cck0+ido;k<l1;k++,ch0k+=ido,ch1k+=ido,cck0+=ccz,cck1+=ccz){
			
			unsigned int i;
			
			for(i=1;i<ido;i+=2){
			
				ch0k[i-1]=cck0[i-1]+cck1[i-1];
				{
					double tr2=cck0[i-1]-cck1[i-1];
					ch0k[i]=cck0[i]+cck1[i];
					{
						double ti2=cck0[i]-cck1[i];
						
						ch1k[i]=wa1[i-1]*ti2-wa1[i]*tr2;
						ch1k[i-1]=wa1[i-1]*tr2+wa1[i]*ti2;
					}
				}
			}
		}
		return;
	}
	{
		unsigned int i;
		
		for(i=1;i<ido;i+=2){
		
			unsigned int k;
			double *ch0k,*ch1k,*cck0,*cck1;
			
			for(k=0,ch0k=ch,ch1k=ch0k+chz,cck0=cc,cck1=cck0+ido;k<l1;k++,ch0k+=ido,ch1k+=ido,cck0+=ccz,cck1+=ccz){
				ch0k[i-1]=cck0[i-1]+cck1[i-1];
				{
					double tr2=cck0[i-1]-cck1[i-1];
					ch0k[i]=cck0[i]+cck1[i];
					{
						double ti2=cck0[i]-cck1[i];
						
						ch1k[i]=wa1[i-1]*ti2-wa1[i]*tr2;
						ch1k[i-1]=wa1[i-1]*tr2+wa1[i]*ti2;
					}
				}
			}
		}
	}
	return;
}













