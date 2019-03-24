/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/5/01 */

#include "AIR.h"

void AIR_radf5(const unsigned int ido, const unsigned int l1, double *cc, double *ch, const double *wa1, const double *wa2, const double *wa3, const double *wa4)

{
	const unsigned int ccz=l1*ido;
	const unsigned int chz=5*ido;

	const double pi=4.0*atan(1.0);
	const double tr11=sin(.1*pi);
	const double ti11=sin(.4*pi);
	const double tr12=-sin(.3*pi);
	const double ti12=sin(.2*pi);
	
	{
		unsigned int k;
		double *cc0k,*cc1k,*cc2k,*cc3k,*cc4k,*chk0,*chk1,*chk2,*chk3,*chk4;
		
		for(k=0,cc0k=cc,cc1k=cc0k+ccz,cc2k=cc1k+ccz,cc3k=cc2k+ccz,cc4k=cc3k+ccz,chk0=ch,chk1=chk0+ido,chk2=chk1+ido,chk3=chk2+ido,chk4=chk3+ido;k<l1;k++,cc0k+=ido,cc1k+=ido,cc2k+=ido,cc3k+=ido,cc4k+=ido,chk0+=chz,chk1+=chz,chk2+=chz,chk3+=chz,chk4+=chz){

			double cr2=*cc4k+*cc1k;
			double ci5=*cc4k-*cc1k;
			double cr3=*cc3k+*cc2k;
			double ci4=*cc3k-*cc2k;
			
			*chk0=*cc0k+cr2+cr3;
			chk1[ido-1]=*cc0k+tr11*cr2+tr12*cr3;
			*chk2=ti11*ci5+ti12*ci4;
			chk3[ido-1]=*cc0k+tr12*cr2+tr11*cr3;
			*chk4=ti12*ci5-ti11*ci4;
		}
	}
	if(ido==1) return;
	if((ido-1)/2>=l1){
	
		unsigned int k;
		double *cc0k,*cc1k,*cc2k,*cc3k,*cc4k,*chk0,*chk1,*chk2,*chk3,*chk4;

		
		for(k=0,cc0k=cc,cc1k=cc0k+ccz,cc2k=cc1k+ccz,cc3k=cc2k+ccz,cc4k=cc3k+ccz,chk0=ch,chk1=chk0+ido,chk2=chk1+ido,chk3=chk2+ido,chk4=chk3+ido;k<l1;k++,cc0k+=ido,cc1k+=ido,cc2k+=ido,cc3k+=ido,cc4k+=ido,chk0+=chz,chk1+=chz,chk2+=chz,chk3+=chz,chk4+=chz){

			unsigned int i;

			for(i=2;i<ido;i+=2){
			
				unsigned int ic=ido-i;
				double dr2=wa1[i-2]*cc1k[i-1]+wa1[i-1]*cc1k[i];
				double di2=wa1[i-2]*cc1k[i]-wa1[i-1]*cc1k[i-1];
				double dr3=wa2[i-2]*cc2k[i-1]+wa2[i-1]*cc2k[i];
				double di3=wa2[i-2]*cc2k[i]-wa2[i-1]*cc2k[i-1];
				double dr4=wa3[i-2]*cc3k[i-1]+wa3[i-1]*cc3k[i];
				double di4=wa3[i-2]*cc3k[i]-wa3[i-1]*cc3k[i-1];
				double dr5=wa4[i-2]*cc4k[i-1]+wa4[i-1]*cc4k[i];
				double di5=wa4[i-2]*cc4k[i]-wa4[i-1]*cc4k[i-1];
				double cr2=dr2+dr5;
				double ci5=dr5-dr2;
				double cr5=di2-di5;
				double ci2=di2+di5;
				double cr3=dr3+dr4;
				double ci4=dr4-dr3;
				double cr4=di3-di4;
				double ci3=di3+di4;
				
				chk0[i-1]=cc0k[i-1]+cr2+cr3;
				chk0[i]=cc0k[i]+ci2+ci3;
				{
					double tr2=cc0k[i-1]+tr11*cr2+tr12*cr3;
					double ti2=cc0k[i]+tr11*ci2+tr12*ci3;
					double tr3=cc0k[i-1]+tr12*cr2+tr11*cr3;
					double ti3=cc0k[i]+tr12*ci2+tr11*ci3;
					double tr5=ti11*cr5+ti12*cr4;
					double ti5=ti11*ci5+ti12*ci4;
					double tr4=ti12*cr5-ti11*cr4;
					double ti4=ti12*ci5-ti11*ci4;
					
					chk2[i-1]=tr2+tr5;
					chk1[ic-1]=tr2-tr5;
					chk2[i]=ti2+ti5;
					chk1[ic]=ti5-ti2;
					chk4[i-1]=tr3+tr4;
					chk3[ic-1]=tr3-tr4;
					chk4[i]=ti3+ti4;
					chk3[ic]=ti4-ti3;
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
			double *cc0k,*cc1k,*cc2k,*cc3k,*cc4k,*chk0,*chk1,*chk2,*chk3,*chk4;

			for(k=0,cc0k=cc,cc1k=cc0k+ccz,cc2k=cc1k+ccz,cc3k=cc2k+ccz,cc4k=cc3k+ccz,chk0=ch,chk1=chk0+ido,chk2=chk1+ido,chk3=chk2+ido,chk4=chk3+ido;k<l1;k++,cc0k+=ido,cc1k+=ido,cc2k+=ido,cc3k+=ido,cc4k+=ido,chk0+=chz,chk1+=chz,chk2+=chz,chk3+=chz,chk4+=chz){

				double dr2=wa1[i-2]*cc1k[i-1]+wa1[i-1]*cc1k[i];
				double di2=wa1[i-2]*cc1k[i]-wa1[i-1]*cc1k[i-1];
				double dr3=wa2[i-2]*cc2k[i-1]+wa2[i-1]*cc2k[i];
				double di3=wa2[i-2]*cc2k[i]-wa2[i-1]*cc2k[i-1];
				double dr4=wa3[i-2]*cc3k[i-1]+wa3[i-1]*cc3k[i];
				double di4=wa3[i-2]*cc3k[i]-wa3[i-1]*cc3k[i-1];
				double dr5=wa4[i-2]*cc4k[i-1]+wa4[i-1]*cc4k[i];
				double di5=wa4[i-2]*cc4k[i]-wa4[i-1]*cc4k[i-1];
				double cr2=dr2+dr5;
				double ci5=dr5-dr2;
				double cr5=di2-di5;
				double ci2=di2+di5;
				double cr3=dr3+dr4;
				double ci4=dr4-dr3;
				double cr4=di3-di4;
				double ci3=di3+di4;
				
				chk0[i-1]=cc0k[i-1]+cr2+cr3;
				chk0[i]=cc0k[i]+ci2+ci3;
				{
					double tr2=cc0k[i-1]+tr11*cr2+tr12*cr3;
					double ti2=cc0k[i]+tr11*ci2+tr12*ci3;
					double tr3=cc0k[i-1]+tr12*cr2+tr11*cr3;
					double ti3=cc0k[i]+tr12*ci2+tr11*ci3;
					double tr5=ti11*cr5+ti12*cr4;
					double ti5=ti11*ci5+ti12*ci4;
					double tr4=ti12*cr5-ti11*cr4;
					double ti4=ti12*ci5-ti11*ci4;
					
					chk2[i-1]=tr2+tr5;
					chk1[ic-1]=tr2-tr5;
					chk2[i]=ti2+ti5;
					chk1[ic]=ti5-ti2;
					chk4[i-1]=tr3+tr4;
					chk3[ic-1]=tr3-tr4;
					chk4[i]=ti3+ti4;
					chk3[ic]=ti4-ti3;
				}
			}
		}
	}
	return;
}
