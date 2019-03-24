/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/22/01 */

#include "AIR.h"

void AIR_cfftb1(const unsigned int n, double *c, double *ch, const double *wa, const unsigned int *ifac)

/* c is the array to transform */
/* ifac is a work array of length >=15 */

{
	unsigned int nf=ifac[1];
	unsigned int na=0;
	unsigned int l1=1;
	unsigned int iw=0;
	{
		unsigned int k1;
		
		for(k1=1;k1<=nf;k1++){
			unsigned int ip=ifac[k1+1];
			unsigned int l2=ip*l1;
			unsigned int ido=n/l2;
			unsigned int idot=ido+ido;
			unsigned int idl1=idot*l1;
			if(ip==4){
				unsigned int ix2=iw+idot;
				unsigned int ix3=ix2+idot;
				if(na==0){
					AIR_passb4(idot,l1,c,ch,&wa[iw],&wa[ix2],&wa[ix3]);
				}
				else{
					AIR_passb4(idot,l1,ch,c,&wa[iw],&wa[ix2],&wa[ix3]);
				}
				na=1-na;
			}
			else if(ip==2){
				if(na==0){
					AIR_passb2(idot,l1,c,ch,&wa[iw]);
				}
				else{
					AIR_passb2(idot,l1,ch,c,&wa[iw]);
				}
				na=1-na;
			}
			else if(ip==3){
				unsigned int ix2=iw+idot;
				if(na==0){
					AIR_passb3(idot,l1,c,ch,&wa[iw],&wa[ix2]);
				}
				else{
					AIR_passb3(idot,l1,ch,c,&wa[iw],&wa[ix2]);
				}
				na=1-na;
			}
			else if (ip==5){
				unsigned int ix2=iw+idot;
				unsigned int ix3=ix2+idot;
				unsigned int ix4=ix3+idot;
				if(na==0){
					AIR_passb5(idot,l1,c,ch,&wa[iw],&wa[ix2],&wa[ix3],&wa[ix4]);
				}
				else{
					AIR_passb5(idot,l1,ch,c,&wa[iw],&wa[ix2],&wa[ix3],&wa[ix4]);
				}
				na=1-na;
			}
			else{
				unsigned int nac;
				
				if(na==0){
					AIR_passb(&nac,idot,ip,l1,idl1,c,c,c,ch,ch,&wa[iw]);
				}
				else{
					AIR_passb(&nac,idot,ip,l1,idl1,ch,ch,ch,c,c,&wa[iw]);
				}
				if(nac!=0) na=1-na;
			}

			l1=l2;
			iw+=(ip-1)*idot;
		}
	}
	if(na==0) return;
	{
		unsigned int i;
		
		for(i=0;i<2*n;i++){
			c[i]=ch[i];
		}
	}
	return;
}
