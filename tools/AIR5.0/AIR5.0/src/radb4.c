/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/5/01 */

#include "AIR.h"

void AIR_radb4(const unsigned int ido, const unsigned int l1, double *cc, double *ch, const double *wa1, const double *wa2, const double *wa3)

{
	const unsigned int ccz=4*ido;
	const unsigned int chz=l1*ido;

	const double sqrt2=sqrt(2.0);

	{
		unsigned int k;
		double *ch0k,*ch1k,*ch2k,*ch3k,*cck0,*cck1,*cck2,*cck3;
		
		for(k=0,ch0k=ch,ch1k=ch0k+chz,ch2k=ch1k+chz,ch3k=ch2k+chz,cck0=cc,cck1=cck0+ido,cck2=cck1+ido,cck3=cck2+ido;k<l1;k++,ch0k+=ido,ch1k+=ido,ch2k+=ido,ch3k+=ido,cck0+=ccz,cck1+=ccz,cck2+=ccz,cck3+=ccz){
			
			double tr1=*cck0-cck3[ido-1];
			double tr2=*cck0+cck3[ido-1];
			double tr3=cck1[ido-1]+cck1[ido-1];
			double tr4=*cck2+*cck2;
			
			*ch0k=tr2+tr3;
			*ch1k=tr1-tr4;
			*ch2k=tr2-tr3;
			*ch3k=tr1+tr4;
		}
	}
	if(ido<2) return;
	if(ido==2){
	
		unsigned int k;
		double *ch0k,*ch1k,*ch2k,*ch3k,*cck0,*cck1,*cck2,*cck3;

		for(k=0,ch0k=ch,ch1k=ch0k+chz,ch2k=ch1k+chz,ch3k=ch2k+chz,cck0=cc,cck1=cck0+ido,cck2=cck1+ido,cck3=cck2+ido;k<l1;k++,ch0k+=ido,ch1k+=ido,ch2k+=ido,ch3k+=ido,cck0+=ccz,cck1+=ccz,cck2+=ccz,cck3+=ccz){
			
			double ti1=*cck1+*cck3;
			double ti2=*cck3-*cck1;
			double tr1=cck0[ido-1]-cck2[ido-1];
			double tr2=cck0[ido-1]+cck2[ido-1];
			
			ch0k[ido-1]=tr2+tr2;
			ch1k[ido-1]=sqrt2*(tr1-ti1);
			ch2k[ido-1]=ti2+ti2;
			ch3k[ido-1]=-sqrt2*(tr1+ti1);
		}
		return;
	}
	if((ido-1)/2>=l1){
	
		unsigned int k;
		double *ch0k,*ch1k,*ch2k,*ch3k,*cck0,*cck1,*cck2,*cck3;

		for(k=0,ch0k=ch,ch1k=ch0k+chz,ch2k=ch1k+chz,ch3k=ch2k+chz,cck0=cc,cck1=cck0+ido,cck2=cck1+ido,cck3=cck2+ido;k<l1;k++,ch0k+=ido,ch1k+=ido,ch2k+=ido,ch3k+=ido,cck0+=ccz,cck1+=ccz,cck2+=ccz,cck3+=ccz){
			
			unsigned int i;
			
			for(i=2;i<ido;i+=2){
			
				unsigned int ic=ido-i;
				double ti1=cck0[i]+cck3[ic];
				double ti2=cck0[i]-cck3[ic];
				double ti3=cck2[i]-cck1[ic];
				double tr4=cck2[i]+cck1[ic];
				double tr1=cck0[i-1]-cck3[ic-1];
				double tr2=cck0[i-1]+cck3[ic-1];
				double ti4=cck2[i-1]-cck1[ic-1];
				double tr3=cck2[i-1]+cck1[ic-1];
				
				ch0k[i-1]=tr2+tr3;
				{
					double cr3=tr2-tr3;
					
					ch0k[i]=ti2+ti3;
					{
						double ci3=ti2-ti3;
						double cr2=tr1-tr4;
						double cr4=tr1+tr4;
						double ci2=ti1+ti4;
						double ci4=ti1-ti4;
						
						ch1k[i-1]=wa1[i-2]*cr2-wa1[i-1]*ci2;
						ch1k[i]=wa1[i-2]*ci2+wa1[i-1]*cr2;
						ch2k[i-1]=wa2[i-2]*cr3-wa2[i-1]*ci3;
						ch2k[i]=wa2[i-2]*ci3+wa2[i-1]*cr3;
						ch3k[i-1]=wa3[i-2]*cr4-wa3[i-1]*ci4;
						ch3k[i]=wa3[i-2]*ci4+wa3[i-1]*cr4;
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
			double *ch0k,*ch1k,*ch2k,*ch3k,*cck0,*cck1,*cck2,*cck3;

			for(k=0,ch0k=ch,ch1k=ch0k+chz,ch2k=ch1k+chz,ch3k=ch2k+chz,cck0=cc,cck1=cck0+ido,cck2=cck1+ido,cck3=cck2+ido;k<l1;k++,ch0k+=ido,ch1k+=ido,ch2k+=ido,ch3k+=ido,cck0+=ccz,cck1+=ccz,cck2+=ccz,cck3+=ccz){
				
				double ti1=cck0[i]+cck3[ic];
				double ti2=cck0[i]-cck3[ic];
				double ti3=cck2[i]-cck1[ic];
				double tr4=cck2[i]+cck1[ic];
				double tr1=cck0[i-1]-cck3[ic-1];
				double tr2=cck0[i-1]+cck3[ic-1];
				double ti4=cck2[i-1]-cck1[ic-1];
				double tr3=cck2[i-1]+cck1[ic-1];
				ch0k[i-1]=tr2+tr3;
				{
					double cr3=tr2-tr3;
					ch0k[i]=ti2+ti3;
					{
						double ci3=ti2-ti3;
						double cr2=tr1-tr4;
						double cr4=tr1+tr4;
						double ci2=ti1+ti4;
						double ci4=ti1-ti4;
						
						ch1k[i-1]=wa1[i-2]*cr2-wa1[i-1]*ci2;
						ch1k[i]=wa1[i-2]*ci2+wa1[i-1]*cr2;
						ch2k[i-1]=wa2[i-2]*cr3-wa2[i-1]*ci3;
						ch2k[i]=wa2[i-2]*ci3+wa2[i-1]*cr3;
						ch3k[i-1]=wa3[i-2]*cr4-wa3[i-1]*ci4;
						ch3k[i]=wa3[i-2]*ci4+wa3[i-1]*cr4;
					}
				}
			}
		}
	}
	if(ido-(ido/2)*2==1) return;
	{
		unsigned int k;
		double *ch0k,*ch1k,*ch2k,*ch3k,*cck0,*cck1,*cck2,*cck3;

		for(k=0,ch0k=ch,ch1k=ch0k+chz,ch2k=ch1k+chz,ch3k=ch2k+chz,cck0=cc,cck1=cck0+ido,cck2=cck1+ido,cck3=cck2+ido;k<l1;k++,ch0k+=ido,ch1k+=ido,ch2k+=ido,ch3k+=ido,cck0+=ccz,cck1+=ccz,cck2+=ccz,cck3+=ccz){
			
			double ti1=*cck1+*cck3;
			double ti2=*cck3-*cck1;
			double tr1=cck0[ido-1]-cck2[ido-1];
			double tr2=cck0[ido-1]+cck2[ido-1];
			
			ch0k[ido-1]=tr2+tr2;
			ch1k[ido-1]=sqrt2*(tr1-ti1);
			ch2k[ido-1]=ti2+ti2;
			ch3k[ido-1]=-sqrt2*(tr1+ti1);
		}
	}
	return;
}
