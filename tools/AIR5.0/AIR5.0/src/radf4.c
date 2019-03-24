/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/5/01 */

#include "AIR.h"

void AIR_radf4(const unsigned int ido, const unsigned int l1, double *cc, double *ch, const double *wa1, const double *wa2, const double *wa3)

{
	const unsigned int ccz=l1*ido;
	const unsigned int chz=4*ido;

	const double hsqt2=.5*sqrt(2.0);

	{
		unsigned int k;
		double   *cc0k,*cc1k,*cc2k,*cc3k,*chk0,*chk1,*chk2,*chk3;
		
		for(k=0,cc0k=cc,cc1k=cc0k+ccz,cc2k=cc1k+ccz,cc3k=cc2k+ccz,chk0=ch,chk1=chk0+ido,chk2=chk1+ido,chk3=chk2+ido;k<l1;k++,cc0k+=ido,cc1k+=ido,cc2k+=ido,cc3k+=ido,chk0+=chz,chk1+=chz,chk2+=chz,chk3+=chz){

			double tr1=*cc1k+*cc3k;
			double tr2=*cc0k+*cc2k;
			
			*chk0=tr1+tr2;
			chk3[ido-1]=tr2-tr1;
			chk1[ido-1]=*cc0k-*cc2k;
			*chk2=*cc3k-*cc1k;
		}
	}
	if(ido<2) return;
	if(ido==2){

		unsigned int k;
		double   *cc0k,*cc1k,*cc2k,*cc3k,*chk0,*chk1,*chk2,*chk3;
		
		for(k=0,cc0k=cc,cc1k=cc0k+ccz,cc2k=cc1k+ccz,cc3k=cc2k+ccz,chk0=ch,chk1=chk0+ido,chk2=chk1+ido,chk3=chk2+ido;k<l1;k++,cc0k+=ido,cc1k+=ido,cc2k+=ido,cc3k+=ido,chk0+=chz,chk1+=chz,chk2+=chz,chk3+=chz){
			
			double ti1=-hsqt2*(cc1k[ido-1]+cc3k[ido-1]);
			double tr1=hsqt2*(cc1k[ido-1]-cc3k[ido-1]);
			
			chk0[ido-1]=tr1+cc0k[ido-1];
			chk3[ido-1]=cc0k[ido-1]-tr1;
			*chk1=ti1-cc2k[ido-1];
			*chk3=ti1+cc2k[ido-1];
		}
		return;
	}
	if((ido-1)/2>=l1){
	
		unsigned int k;
		double   *cc0k,*cc1k,*cc2k,*cc3k,*chk0,*chk1,*chk2,*chk3;
		
		for(k=0,cc0k=cc,cc1k=cc0k+ccz,cc2k=cc1k+ccz,cc3k=cc2k+ccz,chk0=ch,chk1=chk0+ido,chk2=chk1+ido,chk3=chk2+ido;k<l1;k++,cc0k+=ido,cc1k+=ido,cc2k+=ido,cc3k+=ido,chk0+=chz,chk1+=chz,chk2+=chz,chk3+=chz){
			
			unsigned int i;
			
			for(i=2;i<ido;i+=2){
			
				unsigned int ic=ido-i;
				double cr2=wa1[i-2]*cc1k[i-1]+wa1[i-1]*cc1k[i];
				double ci2=wa1[i-2]*cc1k[i]-wa1[i-1]*cc1k[i-1];
				double cr3=wa2[i-2]*cc2k[i-1]+wa2[i-1]*cc2k[i];
				double ci3=wa2[i-2]*cc2k[i]-wa2[i-1]*cc2k[i-1];
				double cr4=wa3[i-2]*cc3k[i-1]+wa3[i-1]*cc3k[i];
				double ci4=wa3[i-2]*cc3k[i]-wa3[i-1]*cc3k[i-1];
				double tr1=cr2+cr4;
				double tr4=cr4-cr2;
				double ti1=ci2+ci4;
				double ti4=ci2-ci4;
				double ti2=cc0k[i]+ci3;
				double ti3=cc0k[i]-ci3;
				double tr2=cc0k[i-1]+cr3;
				double tr3=cc0k[i-1]-cr3;
				
				chk0[i-1]=tr1+tr2;
				chk3[ic-1]=tr2-tr1;
				chk0[i]=ti1+ti2;
				chk3[ic]=ti1-ti2;
				chk2[i-1]=ti4+tr3;
				chk1[ic-1]=tr3-ti4;
				chk2[i]=tr4+ti3;
				chk1[ic]=tr4-ti3;
			}
		}
	}
	else{
	
		unsigned int i;
		
		for(i=2;i<ido;i+=2){
		
			unsigned int ic=ido-i;
			unsigned int k;
			double   *cc0k,*cc1k,*cc2k,*cc3k,*chk0,*chk1,*chk2,*chk3;
			
			for(k=0,cc0k=cc,cc1k=cc0k+ccz,cc2k=cc1k+ccz,cc3k=cc2k+ccz,chk0=ch,chk1=chk0+ido,chk2=chk1+ido,chk3=chk2+ido;k<l1;k++,cc0k+=ido,cc1k+=ido,cc2k+=ido,cc3k+=ido,chk0+=chz,chk1+=chz,chk2+=chz,chk3+=chz){

				double cr2=wa1[i-2]*cc1k[i-1]+wa1[i-1]*cc1k[i];
				double ci2=wa1[i-2]*cc1k[i]-wa1[i-1]*cc1k[i-1];
				double cr3=wa2[i-2]*cc2k[i-1]+wa2[i-1]*cc2k[i];
				double ci3=wa2[i-2]*cc2k[i]-wa2[i-1]*cc2k[i-1];
				double cr4=wa3[i-2]*cc3k[i-1]+wa3[i-1]*cc3k[i];
				double ci4=wa3[i-2]*cc3k[i]-wa3[i-1]*cc3k[i-1];
				double tr1=cr2+cr4;
				double tr4=cr4-cr2;
				double ti1=ci2+ci4;
				double ti4=ci2-ci4;
				double ti2=cc0k[i]+ci3;
				double ti3=cc0k[i]-ci3;
				double tr2=cc0k[i-1]+cr3;
				double tr3=cc0k[i-1]-cr3;
				
				chk0[i-1]=tr1+tr2;
				chk3[ic-1]=tr2-tr1;
				chk0[i]=ti1+ti2;
				chk3[ic]=ti1-ti2;
				chk2[i-1]=ti4+tr3;
				chk1[ic-1]=tr3-ti4;
				chk2[i]=tr4+ti3;
				chk1[ic]=tr4-ti3;
			}
		}
	}
	if(ido-(ido/2)*2==1) return;
	{
		unsigned int k;
		double *cc0k,*cc1k,*cc2k,*cc3k,*chk0,*chk1,*chk2,*chk3;
		
		for(k=0,cc0k=cc,cc1k=cc0k+ccz,cc2k=cc1k+ccz,cc3k=cc2k+ccz,chk0=ch,chk1=chk0+ido,chk2=chk1+ido,chk3=chk2+ido;k<l1;k++,cc0k+=ido,cc1k+=ido,cc2k+=ido,cc3k+=ido,chk0+=chz,chk1+=chz,chk2+=chz,chk3+=chz){

			double ti1=-hsqt2*(cc1k[ido-1]+cc3k[ido-1]);
			double tr1=hsqt2*(cc1k[ido-1]-cc3k[ido-1]);
			
			chk0[ido-1]=tr1+cc0k[ido-1];
			chk2[ido-1]=cc0k[ido-1]-tr1;
			*chk1=ti1-cc2k[ido-1];
			*chk3=ti1+cc2k[ido-1];
		}
	}
	return;
}
