/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/22/01 */

#include "AIR.h"

void AIR_rfftb1(const unsigned int n, double *c, double *ch, const double *wa, const unsigned int *ifac)

{	
	unsigned int na=0;
	{
		unsigned int nf=ifac[1];
		unsigned int l1=1;
		unsigned int iw=0;
		unsigned int k1;
		
		for(k1=1;k1<=nf;k1++){
		
			unsigned int ix2, ix3;
			unsigned int ip=ifac[k1+1];
			unsigned int l2=ip*l1;
			unsigned int ido=n/l2;
			unsigned int idl1=ido*l1;
			
			if(ip==4){
				ix2=iw+ido;
				ix3=ix2+ido;
				if(na==0){
					AIR_radb4(ido,l1,c,ch,&wa[iw],&wa[ix2],&wa[ix3]);
				}
				else{
					AIR_radb4(ido,l1,ch,c,&wa[iw],&wa[ix2],&wa[ix3]);
				}
				na=1-na;
			}
			else if(ip==2){
				if(na==0){
					AIR_radb2(ido,l1,c,ch,&wa[iw]);
				}
				else{
					AIR_radb2(ido,l1,ch,c,&wa[iw]);
				}
				na=1-na;
			}
			else if(ip==3){
				ix2=iw+ido;
				if(na==0){
					AIR_radb3(ido,l1,c,ch,&wa[iw],&wa[ix2]);
				}
				else{
					AIR_radb3(ido,l1,ch,c,&wa[iw],&wa[ix2]);
				}
				na=1-na;
			}
			else if (ip==5){
			
				unsigned int ix4;
				
				ix2=iw+ido;
				ix3=ix2+ido;
				ix4=ix3+ido;
				if(na==0){
					AIR_radb5(ido,l1,c,ch,&wa[iw],&wa[ix2],&wa[ix3],&wa[ix4]);
				}
				else{
					AIR_radb5(ido,l1,ch,c,&wa[iw],&wa[ix2],&wa[ix3],&wa[ix4]);
				}
				na=1-na;
			}
			else{
				if(na==0){
					AIR_radbg(ido,ip,l1,idl1,c,c,c,ch,ch,&wa[iw]);
				}
				else{
					AIR_radbg(ido,ip,l1,idl1,ch,ch,ch,c,c,&wa[iw]);
				}
				if(ido==1) na=1-na;
			}
			l1=l2;
			iw+=(ip-1)*ido;
		}
		if(na==0) return;
	}
	{
		unsigned int i;
		
		for(i=0;i<n;i++){
			c[i]=ch[i];
		}
	}
	return;
}
