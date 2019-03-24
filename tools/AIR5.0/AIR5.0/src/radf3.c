/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/5/01 */

#include "AIR.h"

void AIR_radf3(const unsigned int ido, const unsigned int l1, double *cc, double *ch, const double *wa1, const double *wa2)

{	
	const unsigned int ccz=l1*ido;
	const unsigned int chz=3*ido;

	const double taur=-.5;
	const double taui=0.5*sqrt((double)3.0);
	
	{
		unsigned int k;
		double 	*cc0k,*cc1k,*cc2k,*chk0,*chk1,*chk2;
		
		for(k=0,cc0k=cc,cc1k=cc0k+ccz,cc2k=cc1k+ccz,chk0=ch,chk1=chk0+ido,chk2=chk1+ido;k<l1;k++,cc0k+=ido,cc1k+=ido,cc2k+=ido,chk0+=chz,chk1+=chz,chk2+=chz){
			
			double cr2=*cc1k+*cc2k;

			*chk0=*cc0k+cr2;

			*chk2=taui*(*cc2k-*cc1k);
			chk1[ido-1]=*cc0k+taur*cr2;
		}
	}
	if(ido==1) return;
	if((ido-1)/2>=l1){
	
		unsigned int k;
		double 	*cc0k,*cc1k,*cc2k,*chk0,*chk1,*chk2;
		
		for(k=0,cc0k=cc,cc1k=cc0k+ccz,cc2k=cc1k+ccz,chk0=ch,chk1=chk0+ido,chk2=chk1+ido;k<l1;k++,cc0k+=ido,cc1k+=ido,cc2k+=ido,chk0+=chz,chk1+=chz,chk2+=chz){
			
			unsigned int i;
			
			for(i=2;i<ido;i+=2){
				unsigned int ic=ido-i;
				double dr2=wa1[i-2]*cc1k[i-1]+wa1[i-1]*cc1k[i];
				double di2=wa1[i-2]*cc1k[i]-wa1[i-1]*cc1k[i-1];
				double dr3=wa2[i-2]*cc2k[i-1]+wa2[i-1]*cc2k[i];
				double di3=wa2[i-2]*cc2k[i]-wa2[i-1]*cc2k[i-1];
				double cr2=dr2+dr3;
				double ci2=di2+di3;
				
				chk0[i-1]=cc0k[i-1]+cr2;
				chk0[i]=cc0k[i]+ci2;
				{
					double tr2=cc0k[i-1]+taur*cr2;
					double ti2=cc0k[i]+taur*ci2;
					double tr3=taui*(di2-di3);
					double ti3=taui*(dr3-dr2);
					
					chk2[i-1]=tr2+tr3;
					chk1[ic-1]=tr2-tr3;
					chk2[i]=ti2+ti3;
					chk1[ic]=ti3-ti2;
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
			double 	*cc0k,*cc1k,*cc2k,*chk0,*chk1,*chk2;
	
			for(k=0,cc0k=cc,cc1k=cc0k+ccz,cc2k=cc1k+ccz,chk0=ch,chk1=chk0+ido,chk2=chk1+ido;k<l1;k++,cc0k+=ido,cc1k+=ido,cc2k+=ido,chk0+=chz,chk1+=chz,chk2+=chz){
				double dr2=wa1[i-2]*cc1k[i-1]+wa1[i-1]*cc1k[i];
				double di2=wa1[i-2]*cc1k[i]-wa1[i-1]*cc1k[i-1];
				double dr3=wa2[i-2]*cc2k[i-1]+wa2[i-1]*cc2k[i];
				double di3=wa2[i-2]*cc2k[i]-wa2[i-1]*cc2k[i-1];
				double cr2=dr2+dr3;
				double ci2=di2+di3;
				
				chk0[i-1]=cc0k[i-1]+cr2;
				chk0[i]=cc0k[i]+ci2;
				{
					double tr2=cc0k[i-1]+taur*cr2;
					double ti2=cc0k[i]+taur*ci2;
					double tr3=taui*(di2-di3);
					double ti3=taui*(dr3-dr2);
					
					chk2[i-1]=tr2+tr3;
					chk1[ic-1]=tr2-tr3;
					chk2[i]=ti2+ti3;
					chk1[ic]=ti3-ti2;
				}
			}
		}
	}
	return;
}
