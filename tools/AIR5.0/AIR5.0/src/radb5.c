/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/5/01 */

#include "AIR.h"

void AIR_radb5(const unsigned int ido, const unsigned int l1, double *cc, double *ch, const double *wa1, const double *wa2, const double *wa3, const double *wa4)

{	
	const unsigned int ccz=5*ido;
	const unsigned int chz=l1*ido;

	const double pi=4.0*atan(1.0);
	const double tr11=sin(0.1*pi);
	const double ti11=sin(0.4*pi);
	const double tr12=-sin(0.3*pi);
	const double ti12=sin(0.2*pi);

	{
		unsigned int k;
		double *ch0k,*ch1k,*ch2k,*ch3k,*ch4k,*cck0,*cck1,*cck2,*cck3,*cck4;

		for(k=0,ch0k=ch,ch1k=ch0k+chz,ch2k=ch1k+chz,ch3k=ch2k+chz,ch4k=ch3k+chz,cck0=cc,cck1=cck0+ido,cck2=cck1+ido,cck3=cck2+ido,cck4=cck3+ido;k<l1;k++,ch0k+=ido,ch1k+=ido,ch2k+=ido,ch3k+=ido,ch4k+=ido,cck0+=ccz,cck1+=ccz,cck2+=ccz,cck3+=ccz,cck4+=ccz){
			
			double ti5=*cck2+*cck2;
			double ti4=*cck4+*cck4;
			double tr2=cck1[ido-1]+cck1[ido-1];
			double tr3=cck3[ido-1]+cck3[ido-1];
			
			*ch0k=*cck0+tr2+tr3;
			{
				double cr2=*cck0+tr11*tr2+tr12*tr3;
				double cr3=*cck0+tr12*tr2+tr11*tr3;
				double ci5=ti11*ti5+ti12*ti4;
				double ci4=ti12*ti5-ti11*ti4;
				
				*ch1k=cr2-ci5;
				*ch2k=cr3-ci4;
				*ch3k=cr3+ci4;
				*ch4k=cr2+ci5;
			}
		}
	}
	if(ido==1) return;
	if((ido-1)/2>=l1){
	
		unsigned int k;
		double *ch0k,*ch1k,*ch2k,*ch3k,*ch4k,*cck0,*cck1,*cck2,*cck3,*cck4;

		for(k=0,ch0k=ch,ch1k=ch0k+chz,ch2k=ch1k+chz,ch3k=ch2k+chz,ch4k=ch3k+chz,cck0=cc,cck1=cck0+ido,cck2=cck1+ido,cck3=cck2+ido,cck4=cck3+ido;k<l1;k++,ch0k+=ido,ch1k+=ido,ch2k+=ido,ch3k+=ido,ch4k+=ido,cck0+=ccz,cck1+=ccz,cck2+=ccz,cck3+=ccz,cck4+=ccz){
			
			unsigned int i;
			
			for(i=2;i<ido;i+=2){
			
				unsigned int ic=ido-i;
				double ti5=cck2[i]+cck1[ic];
				double ti2=cck2[i]-cck1[ic];
				double ti4=cck4[i]+cck3[ic];
				double ti3=cck4[i]-cck3[ic];
				double tr5=cck2[i-1]-cck1[ic-1];
				double tr2=cck2[i-1]+cck1[ic-1];
				double tr4=cck4[i-1]-cck3[ic-1];
				double tr3=cck4[i-1]+cck3[ic-1];
				
				ch0k[i-1]=cck0[i-1]+tr2+tr3;
				ch0k[i]=cck0[i]+ti2+ti3;
				{
					double cr2=cck0[i-1]+tr11*tr2+tr12*tr3;
					double ci2=cck0[i]+tr11*ti2+tr12*ti3;
					double cr3=cck0[i-1]+tr12*tr2+tr11*tr3;
					double ci3=cck0[i]+tr12*ti2+tr11*ti3;
					double cr5=ti11*tr5+ti12*tr4;
					double ci5=ti11*ti5+ti12*ti4;
					double cr4=ti12*tr5-ti11*tr4;
					double ci4=ti12*ti5-ti11*ti4;
					double dr3=cr3-ci4;
					double dr4=cr3+ci4;
					double di3=ci3+cr4;
					double di4=ci3-cr4;
					double dr5=cr2+ci5;
					double dr2=cr2-ci5;
					double di5=ci2-cr5;
					double di2=ci2+cr5;
					
					ch1k[i-1]=wa1[i-2]*dr2-wa1[i-1]*di2;
					ch1k[i]=wa1[i-2]*di2+wa1[i-1]*dr2;
					ch2k[i-1]=wa2[i-2]*dr3-wa2[i-1]*di3;
					ch2k[i]=wa2[i-2]*di3+wa2[i-1]*dr3;
					ch3k[i-1]=wa3[i-2]*dr4-wa3[i-1]*di4;
					ch3k[i]=wa3[i-2]*di4+wa3[i-1]*dr4;
					ch4k[i-1]=wa4[i-2]*dr5-wa4[i-1]*di5;
					ch4k[i]=wa4[i-2]*di5+wa4[i-1]*dr5;
				}
			}
		}
		return;
	}
	{
		unsigned int i;
		
		for(i=2;i<ido;i+=2){
		
			unsigned int ic=ido-i;
			unsigned int k;
			double *ch0k,*ch1k,*ch2k,*ch3k,*ch4k,*cck0,*cck1,*cck2,*cck3,*cck4;

			for(k=0,ch0k=ch,ch1k=ch0k+chz,ch2k=ch1k+chz,ch3k=ch2k+chz,ch4k=ch3k+chz,cck0=cc,cck1=cck0+ido,cck2=cck1+ido,cck3=cck2+ido,cck4=cck3+ido;k<l1;k++,ch0k+=ido,ch1k+=ido,ch2k+=ido,ch3k+=ido,ch4k+=ido,cck0+=ccz,cck1+=ccz,cck2+=ccz,cck3+=ccz,cck4+=ccz){
				
				double ti5=cck2[i]+cck1[ic];
				double ti2=cck2[i]-cck1[ic];
				double ti4=cck4[i]+cck3[ic];
				double ti3=cck4[i]-cck3[ic];
				double tr5=cck2[i-1]-cck1[ic-1];
				double tr2=cck2[i-1]+cck1[ic-1];
				double tr4=cck4[i-1]-cck3[ic-1];
				double tr3=cck4[i-1]+cck3[ic-1];
				ch0k[i-1]=cck0[i-1]+tr2+tr3;
				ch0k[i]=cck0[i]+ti2+ti3;
				{
					double cr2=cck0[i-1]+tr11*tr2+tr12*tr3;
					double ci2=cck0[i]+tr11*ti2+tr12*ti3;
					double cr3=cck0[i-1]+tr12*tr2+tr11*tr3;
					double ci3=cck0[i]+tr12*ti2+tr11*ti3;
					double cr5=ti11*tr5+ti12*tr4;
					double ci5=ti11*ti5+ti12*ti4;
					double cr4=ti12*tr5-ti11*tr4;
					double ci4=ti12*ti5-ti11*ti4;
					double dr3=cr3-ci4;
					double dr4=cr3+ci4;
					double di3=ci3+cr4;
					double di4=ci3-cr4;
					double dr5=cr2+ci5;
					double dr2=cr2-ci5;
					double di5=ci2-cr5;
					double di2=ci2+cr5;
					
					ch1k[i-1]=wa1[i-2]*dr2-wa1[i-1]*di2;
					ch1k[i]=wa1[i-2]*di2+wa1[i-1]*dr2;
					ch2k[i-1]=wa2[i-2]*dr3-wa2[i-1]*di3;
					ch2k[i]=wa2[i-2]*di3+wa2[i-1]*dr3;
					ch3k[i-1]=wa3[i-2]*dr4-wa3[i-1]*di4;
					ch3k[i]=wa3[i-2]*di4+wa3[i-1]*dr4;
					ch4k[i-1]=wa4[i-2]*dr5-wa4[i-1]*di5;
					ch4k[i]=wa4[i-2]*di5+wa4[i-1]*dr5;
				}
			}
		}
	}
	return;
}
