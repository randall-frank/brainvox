/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/5/01 */

#include "AIR.h"

/* Calculate the fast Fourier transform of subvectors of length two */

void AIR_radf2(const unsigned int ido, const unsigned int l1, double *cc, double *ch, const double *wa1)

{
	const unsigned int ccz=l1*ido;
	const unsigned int chz=2*ido;
	
	{
		unsigned int k;
		double *cc0k,*cc1k,*chk0,*chk1;
		 
		for(k=0,cc0k=cc,cc1k=cc0k+ccz,chk0=ch,chk1=chk0+ido;k<l1;k++,cc0k+=ido,cc1k+=ido,chk0+=chz,chk1+=chz){
			*chk0=*cc0k+*cc1k;
			chk1[ido-1]=*cc0k-*cc1k;
		}
	}
	if(ido<2) return;
	if(ido==2){
	
		unsigned int k;
		double *cc0k,*cc1k,*chk0,*chk1;
		
		for(k=0,cc0k=cc,cc1k=cc0k+ccz,chk0=ch,chk1=chk0+ido;k<l1;k++,cc0k+=ido,cc1k+=ido,chk0+=chz,chk1+=chz){
			*chk1=-cc1k[1];
			chk0[1]=cc0k[1];
		}
		return;
	}
	if((ido-1)/2>=l1){
	
		unsigned int k;
		double *cc0k,*cc1k,*chk0,*chk1;
		
		for(k=0,cc0k=cc,cc1k=cc0k+ccz,chk0=ch,chk1=chk0+ido;k<l1;k++,cc0k+=ido,cc1k+=ido,chk0+=chz,chk1+=chz){
		
			unsigned int i;
			
			for(i=2;i<ido;i+=2){
			
				unsigned int ic=ido-i;
				double tr2=wa1[i-2]*cc1k[i-1]+wa1[i-1]*cc1k[i];
				double ti2=wa1[i-2]*cc1k[i]-wa1[i-1]*cc1k[i-1];
				
				chk0[i]=cc0k[i]+ti2;
				chk1[ic]=ti2-cc0k[i];
				chk0[i-1]=cc0k[i-1]+tr2;
				chk1[ic-1]=cc0k[i-1]-tr2;
			}
		}
	}
	else{
	
		unsigned int i;
		
		for(i=2;i<ido;i+=2){
		
			unsigned int ic=ido-i;
			unsigned int k;
			double *cc0k,*cc1k,*chk0,*chk1;
			
			for(k=0,cc0k=cc,cc1k=cc0k+ccz,chk0=ch,chk1=chk0+ido;k<l1;k++,cc0k+=ido,cc1k+=ido,chk0+=chz,chk1+=chz){
			
				double tr2=wa1[i-2]*cc1k[i-1]+wa1[i-1]*cc1k[i];
				double ti2=wa1[i-2]*cc1k[i]-wa1[i-1]*cc1k[i-1];
				
				chk0[i]=cc0k[i]+ti2;
				chk1[ic]=ti2-cc0k[i];
				chk0[i-1]=cc0k[i-1]+tr2;
				chk1[ic-1]=cc0k[i-1]-tr2;
			}
		}
	}
	if(ido-(ido/2)*2==1) return;
	{
		unsigned int k;
		double *cc0k,*cc1k,*chk0,*chk1;
		
		for(k=0,cc0k=cc,cc1k=cc0k+ccz,chk0=ch,chk1=chk0+ido;k<l1;k++,cc0k+=ido,cc1k+=ido,chk0+=chz,chk1+=chz){
			*chk1=-cc1k[ido-1];
			chk0[ido-1]=cc0k[ido-1];
		}
	}
	return;
}
