/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/5/01 */

#include "AIR.h"

void AIR_radb2(const unsigned int ido, const unsigned int l1, double *cc, double *ch, const double *wa1)

{
	const unsigned int ccz=2*ido;
	const unsigned int chz=l1*ido;

	{
		unsigned int k;
		double *ch0k,*ch1k,*cck0,*cck1;
		
		for(k=0,ch0k=ch,ch1k=ch0k+chz,cck0=cc,cck1=cck0+ido;k<l1;k++,ch0k+=ido,ch1k+=ido,cck0+=ccz,cck1+=ccz){
			*ch0k=*cck0+cck1[ido-1];
			*ch1k=*cck0-cck1[ido-1];
		}
	}
	if(ido<2) return;
	if(ido==2){
	
		unsigned int k;
		double *ch0k,*ch1k,*cck0,*cck1;
		
		for(k=0,ch0k=ch,ch1k=ch0k+chz,cck0=cc,cck1=cck0+ido;k<l1;k++,ch0k+=ido,ch1k+=ido,cck0+=ccz,cck1+=ccz){
			ch0k[ido-1]=cck0[ido-1]+cck0[ido-1];
			ch1k[ido-1]=-(*cck1+*cck1);
		}
		return;
	}
	if((ido-1)/2>=l1){
	
		unsigned int k;
		double *ch0k,*ch1k,*cck0,*cck1;

		for(k=0,ch0k=ch,ch1k=ch0k+chz,cck0=cc,cck1=cck0+ido;k<l1;k++,ch0k+=ido,ch1k+=ido,cck0+=ccz,cck1+=ccz){
			
			unsigned int i;
			
			for(i=2;i<ido;i+=2){
			
				unsigned int ic=ido-i;
				
				ch0k[i-1]=cck0[i-1]+cck1[ic-1];
				{
					double tr2=cck0[i-1]-cck1[ic-1];
					ch0k[i]=cck0[i]-cck1[ic];
					{
						double ti2=cck0[i]+cck1[ic];
						ch1k[i-1]=wa1[i-2]*tr2-wa1[i-1]*ti2;
						ch1k[i]=wa1[i-2]*ti2+wa1[i-1]*tr2;
					}
				}
			}
		}
	}
	else{
	
		unsigned int i;
		
		for(i=2;i<ido;i+=2){
		
			unsigned int ic=ido-i;
			unsigned int k;
			double *ch0k,*ch1k,*cck0,*cck1;

			for(k=0,ch0k=ch,ch1k=ch0k+chz,cck0=cc,cck1=cck0+ido;k<l1;k++,ch0k+=ido,ch1k+=ido,cck0+=ccz,cck1+=ccz){
				ch0k[i-1]=cck0[i-1]+cck1[ic-1];
				{
					double tr2=cck0[i-1]-cck1[ic-1];
					ch0k[i]=cck0[i]-cck1[ic];
					{
						double ti2=cck0[i]+cck1[ic];
						ch1k[i-1]=wa1[i-2]*tr2-wa1[i-1]*ti2;
						ch1k[i]=wa1[i-2]*ti2+wa1[i-1]*tr2;
					}
				}
			}
		}
	}
	if(ido-(ido/2)*2==1) return;
	{
		unsigned int k;
		double *ch0k,*ch1k,*cck0,*cck1;

		for(k=0,ch0k=ch,ch1k=ch0k+chz,cck0=cc,cck1=cck0+ido;k<l1;k++,ch0k+=ido,ch1k+=ido,cck0+=ccz,cck1+=ccz){
			ch0k[ido-1]=cck0[ido-1]+cck0[ido-1];
			ch1k[ido-1]=-(*cck1+*cck1);
		}
	}
	return;
}
