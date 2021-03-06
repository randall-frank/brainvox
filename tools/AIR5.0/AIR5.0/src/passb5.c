/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/2/01 */

#include "AIR.h"

void AIR_passb5(const unsigned int ido, const unsigned int l1, double *cc, double *ch, const double *wa1, const double *wa2, const double *wa3, const double *wa4)

{	
	unsigned int ccz=5*ido;
	unsigned int chz=l1*ido;

	unsigned int ccy=ido;
	unsigned int chy=ido;

	double pi=4.0*atan(1.0);
	double tr11=sin(0.1*pi);
	double ti11=sin(0.4*pi);
	double tr12=-sin(0.3*pi);
	double ti12=sin(0.2*pi);

	if(ido==2){
	
		unsigned int k;
		double *ch0k,*ch1k,*ch2k,*ch3k,*ch4k,*cck0,*cck1,*cck2,*cck3,*cck4;
		
		for(k=0,cck0=cc,cck1=cck0+ccy,cck2=cck1+ccy,cck3=cck2+ccy,cck4=cck3+ccy,ch0k=ch,ch1k=ch0k+chz,ch2k=ch1k+chz,ch3k=ch2k+chz,ch4k=ch3k+chz;k<l1;k++,cck0+=ccz,cck1+=ccz,cck2+=ccz,cck3+=ccz,cck4+=ccz,ch0k+=chy,ch1k+=chy,ch2k+=chy,ch3k+=chy,ch4k+=chy){
			
			double ti5=cck1[1]-cck4[1];
			double ti2=cck1[1]+cck4[1];
			double ti4=cck2[1]-cck3[1];
			double ti3=cck2[1]+cck3[1];
			double tr5=cck1[0]-cck4[0];
			double tr2=cck1[0]+cck4[0];
			double tr4=cck2[0]-cck3[0];
			double tr3=cck2[0]+cck3[0];
			ch0k[0]=cck0[0]+tr2+tr3;
			ch0k[1]=cck0[1]+ti2+ti3;
			{
				double cr2=cck0[0]+tr11*tr2+tr12*tr3;
				double ci2=cck0[1]+tr11*ti2+tr12*ti3;
				double cr3=cck0[0]+tr12*tr2+tr11*tr3;
				double ci3=cck0[1]+tr12*ti2+tr11*ti3;
				double cr5=ti11*tr5+ti12*tr4;
				double ci5=ti11*ti5+ti12*ti4;
				double cr4=ti12*tr5-ti11*tr4;
				double ci4=ti12*ti5-ti11*ti4;
				
				ch1k[0]=cr2-ci5;
				ch4k[0]=cr2+ci5;
				ch1k[1]=ci2+cr5;
				ch2k[1]=ci3+cr4;
				ch2k[0]=cr3-ci4;
				ch3k[0]=cr3+ci4;
				ch3k[1]=ci3-cr4;
				ch4k[1]=ci2-cr5;
			}
		}
		return;
	}
	if(ido/2>=l1){
	
		unsigned int k;
		double *ch0k,*ch1k,*ch2k,*ch3k,*ch4k,*cck0,*cck1,*cck2,*cck3,*cck4;
		
		for(k=0,cck0=cc,cck1=cck0+ccy,cck2=cck1+ccy,cck3=cck2+ccy,cck4=cck3+ccy,ch0k=ch,ch1k=ch0k+chz,ch2k=ch1k+chz,ch3k=ch2k+chz,ch4k=ch3k+chz;k<l1;k++,cck0+=ccz,cck1+=ccz,cck2+=ccz,cck3+=ccz,cck4+=ccz,ch0k+=chy,ch1k+=chy,ch2k+=chy,ch3k+=chy,ch4k+=chy){
			
			unsigned int i;
			
			for(i=1;i<ido;i+=2){
			
				double ti5=cck1[i]-cck4[i];
				double ti2=cck1[i]+cck4[i];
				double ti4=cck2[i]-cck3[i];
				double ti3=cck2[i]+cck3[i];
				double tr5=cck1[i-1]-cck4[i-1];
				double tr2=cck1[i-1]+cck4[i-1];
				double tr4=cck2[i-1]-cck3[i-1];
				double tr3=cck2[i-1]+cck3[i-1];
				
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
					
					ch1k[i-1]=wa1[i-1]*dr2-wa1[i]*di2;
					ch1k[i]=wa1[i-1]*di2+wa1[i]*dr2;
					ch2k[i-1]=wa2[i-1]*dr3-wa2[i]*di3;
					ch2k[i]=wa2[i-1]*di3+wa2[i]*dr3;
					ch3k[i-1]=wa3[i-1]*dr4-wa3[i]*di4;
					ch3k[i]=wa3[i-1]*di4+wa3[i]*dr4;
					ch4k[i-1]=wa4[i-1]*dr5-wa4[i]*di5;
					ch4k[i]=wa4[i-1]*di5+wa4[i]*dr5;
				}
			}
		}
		return;
	}
	{
		unsigned int i;
		
		for(i=1;i<ido;i+=2){
		
			unsigned int k;
			double *ch0k,*ch1k,*ch2k,*ch3k,*ch4k,*cck0,*cck1,*cck2,*cck3,*cck4;
			
			for(k=0,cck0=cc,cck1=cck0+ccy,cck2=cck1+ccy,cck3=cck2+ccy,cck4=cck3+ccy,ch0k=ch,ch1k=ch0k+chz,ch2k=ch1k+chz,ch3k=ch2k+chz,ch4k=ch3k+chz;k<l1;k++,cck0+=ccz,cck1+=ccz,cck2+=ccz,cck3+=ccz,cck4+=ccz,ch0k+=chy,ch1k+=chy,ch2k+=chy,ch3k+=chy,ch4k+=chy){
				
				double ti5=cck1[i]-cck4[i];
				double ti2=cck1[i]+cck4[i];
				double ti4=cck2[i]-cck3[i];
				double ti3=cck2[i]+cck3[i];
				double tr5=cck1[i-1]-cck4[i-1];
				double tr2=cck1[i-1]+cck4[i-1];
				double tr4=cck2[i-1]-cck3[i-1];
				double tr3=cck2[i-1]+cck3[i-1];
				
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
					
					ch1k[i-1]=wa1[i-1]*dr2-wa1[i]*di2;
					ch1k[i]=wa1[i-1]*di2+wa1[i]*dr2;
					ch2k[i-1]=wa2[i-1]*dr3-wa2[i]*di3;
					ch2k[i]=wa2[i-1]*di3+wa2[i]*dr3;
					ch3k[i-1]=wa3[i-1]*dr4-wa3[i]*di4;
					ch3k[i]=wa3[i-1]*di4+wa3[i]*dr4;
					ch4k[i-1]=wa4[i-1]*dr5-wa4[i]*di5;
					ch4k[i]=wa4[i-1]*di5+wa4[i]*dr5;
				}
			}
		}
	}
}

