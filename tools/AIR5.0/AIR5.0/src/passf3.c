/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/4/01 */

#include "AIR.h"

void AIR_passf3(const unsigned int ido, const unsigned int l1, double *cc, double *ch, const double *wa1, const double *wa2)

{
	const unsigned int ccz=3*ido;
	const unsigned int chz=l1*ido;

	const double taur=-.5;
	const double taui=-.5*sqrt((double)3.0);

	if(ido==2){
	{
		unsigned int k;
		double *ch0k,*ch1k,*ch2k,*cck0,*cck1,*cck2;
		
		for(k=0,ch0k=ch,ch1k=ch0k+chz,ch2k=ch1k+chz,cck0=cc,cck1=cck0+ido,cck2=cck1+ido;k<l1;k++,ch0k+=ido,ch1k+=ido,ch2k+=ido,cck0+=ccz,cck1+=ccz,cck2+=ccz){

				double tr2=cck1[0]+cck2[0];
				double cr2=cck0[0]+taur*tr2;
				ch0k[0]=cck0[0]+tr2;
				{
					double ti2=cck1[1]+cck2[1];
					double ci2=cck0[1]+taur*ti2;
					ch0k[1]=cck0[1]+ti2;
					{
						double cr3=taui*(cck1[0]-cck2[0]);
						double ci3=taui*(cck1[1]-cck2[1]);
						
						ch1k[0]=cr2-ci3;
						ch2k[0]=cr2+ci3;
						ch1k[1]=ci2+cr3;
						ch2k[1]=ci2-cr3;
					}
				}
			}
			return;
		}
	}
	if(ido/2>=l1){
	{
		unsigned int k;
		double *ch0k,*ch1k,*ch2k,*cck0,*cck1,*cck2;
		
		for(k=0,ch0k=ch,ch1k=ch0k+chz,ch2k=ch1k+chz,cck0=cc,cck1=cck0+ido,cck2=cck1+ido;k<l1;k++,ch0k+=ido,ch1k+=ido,ch2k+=ido,cck0+=ccz,cck1+=ccz,cck2+=ccz){
				
				unsigned int i;
				
				for(i=1;i<ido;i+=2){
					double tr2=cck1[i-1]+cck2[i-1];
					double cr2=cck0[i-1]+taur*tr2;
					ch0k[i-1]=cck0[i-1]+tr2;
					{
						double ti2=cck1[i]+cck2[i];
						double ci2=cck0[i]+taur*ti2;
						ch0k[i]=cck0[i]+ti2;
						{
							double cr3=taui*(cck1[i-1]-cck2[i-1]);
							double ci3=taui*(cck1[i]-cck2[i]);
							double dr2=cr2-ci3;
							double dr3=cr2+ci3;
							double di2=ci2+cr3;
							double di3=ci2-cr3;
							
							ch1k[i]=wa1[i-1]*di2-wa1[i]*dr2;
							ch1k[i-1]=wa1[i-1]*dr2+wa1[i]*di2;
							ch2k[i]=wa2[i-1]*di3-wa2[i]*dr3;
							ch2k[i-1]=wa2[i-1]*dr3+wa2[i]*di3;
						}
					}
				}
			}
			return;
		}
	}
	{
		unsigned int i;
		
		for(i=1;i<ido;i+=2){
		
			unsigned int k;
			double *ch0k,*ch1k,*ch2k,*cck0,*cck1,*cck2;
			
			for(k=0,ch0k=ch,ch1k=ch0k+chz,ch2k=ch1k+chz,cck0=cc,cck1=cck0+ido,cck2=cck1+ido;k<l1;k++,ch0k+=ido,ch1k+=ido,ch2k+=ido,cck0+=ccz,cck1+=ccz,cck2+=ccz){
				
				double tr2=cck1[i-1]+cck2[i-1];
				double cr2=cck0[i-1]+taur*tr2;
				ch0k[i-1]=cck0[i-1]+tr2;
				{
					double ti2=cck1[i]+cck2[i];
					double ci2=cck0[i]+taur*ti2;
					ch0k[i]=cck0[i]+ti2;
					{
						double cr3=taui*(cck1[i-1]-cck2[i-1]);
						double ci3=taui*(cck1[i]-cck2[i]);
						double dr2=cr2-ci3;
						double dr3=cr2+ci3;
						double di2=ci2+cr3;
						double di3=ci2-cr3;
						
						ch1k[i]=wa1[i-1]*di2-wa1[i]*dr2;
						ch1k[i-1]=wa1[i-1]*dr2+wa1[i]*di2;
						ch2k[i]=wa2[i-1]*di3-wa2[i]*dr3;
						ch2k[i-1]=wa2[i-1]*dr3+wa2[i]*di3;
					}
				}
			}
		}
	}
}


