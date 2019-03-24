/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/22/01 */

/* Compute the forward fourier transform of a real sequence */

#include "AIR.h"

void AIR_rfftf1(const unsigned int n, double *c, double *ch, const double *wa, const unsigned int *ifac)

{
	unsigned int na=1;
	if(n<2) return;
	{		
		unsigned int nf=ifac[1];	/* Number of factors */
		unsigned int l2=n;
		unsigned int iw=n-1;	/* Decremented from Fortran value */
		unsigned int kh;
		
		for(kh=nf-1;;kh--){
		
			unsigned int ip=ifac[kh+2];
			unsigned int l1=l2/ip;
			unsigned int ido=n/l2;
			unsigned int idl1=ido*l1;
			
			iw-=(ip-1)*ido;
			na=1-na;
			if(ip==4){
				unsigned int ix2=iw+ido;
				unsigned int ix3=ix2+ido;
				if(na==0){
					AIR_radf4(ido,l1,c,ch,&wa[iw],&wa[ix2],&wa[ix3]);
				}
				else{
					AIR_radf4(ido,l1,ch,c,&wa[iw],&wa[ix2],&wa[ix3]);
				}
			}
			else if(ip==2){
				if(na==0){
					AIR_radf2(ido,l1,c,ch,&wa[iw]);
				}
				else{
					AIR_radf2(ido,l1,ch,c,&wa[iw]);
				}
			}
			else if(ip==3){
				unsigned int ix2=iw+ido;
				if(na==0){
					AIR_radf3(ido,l1,c,ch,&wa[iw],&wa[ix2]);
				}
				else{
					AIR_radf3(ido,l1,ch,c,&wa[iw],&wa[ix2]);
				}
			}
			else if(ip==5){
							
				unsigned int ix2=iw+ido;
				unsigned int ix3=ix2+ido;
				unsigned ix4=ix3+ido;
				if(na==0){
					AIR_radf5(ido,l1,c,ch,&wa[iw],&wa[ix2],&wa[ix3],&wa[ix4]);
				}
				else{
					AIR_radf5(ido,l1,ch,c,&wa[iw],&wa[ix2],&wa[ix3],&wa[ix4]);
				}
			}
			else{
				if(ido==1) na=1-na;
				if(na==0){
					AIR_radfg(ido,ip,l1,idl1,c,c,c,ch,ch,&wa[iw]);
					na=1;
				}
				else{
					AIR_radfg(ido,ip,l1,idl1,ch,ch,ch,c,c,&wa[iw]);
					na=0;
				}
			}
			l2=l1;
			if(kh==0) break;
		}
	}
	if(na==1) return;
	{
		unsigned int i;	/* Decremented from Fortran value */
		
		for(i=0;i<n;i++){
			c[i]=ch[i];
		}
	}
	return;
}
