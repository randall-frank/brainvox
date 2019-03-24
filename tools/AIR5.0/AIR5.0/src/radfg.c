/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/5/01 */

#include "AIR.h"

void AIR_radfg(const unsigned int ido, const unsigned int ip, const unsigned int l1, const unsigned int idl1, double *cc, double *c1, double *c2, double *ch, double *ch2, const double *wa)

{
	const unsigned int ccz=ip*ido;
	const unsigned int c1z=l1*ido;
	const unsigned int c2z=idl1;
	const unsigned int chz=l1*ido;
	const unsigned int ch2z=idl1;

	const double tpi=8.0*atan(1.0);
	const double arg=tpi/ip;
	const double dcp=cos(arg);
	const double dsp=sin(arg);
	
	const unsigned int ipph=(ip+1)/2;
	const unsigned int nbd=(ido-1)/2;
	
	if(ido!=1){
		{
			unsigned int ik;
			double *ch20,*c20;
			
			for(ik=0,ch20=ch2,c20=c2;ik<idl1;ik++,ch20++,c20++){
				*ch20=*c20;
			}
		}
		{
			unsigned int j;
			double *chjk,*c1jk;

			for(j=1,chjk=ch+chz,c1jk=c1+c1z;j<ip;j++){
			
				unsigned int k;
				
				for(k=0;k<l1;k++,chjk+=ido,c1jk+=ido){
					*chjk=*c1jk;
				}
			}
		}
		if(nbd<=l1){
		
			unsigned int is=1;
			unsigned int j;
			double *chjk,*c1jk;
			
			for(j=1,chjk=ch+chz,c1jk=c1+c1z;j<ip;j++,chjk+=chz,c1jk+=c1z, is+=ido){
			
				unsigned int idij=is;
				unsigned int i;
				
				for(i=2;i<ido;i+=2,chjk-=chz,c1jk-=c1z, idij+=2){
				
					unsigned int k;
					
					for(k=0;k<l1;k++,chjk+=ido,c1jk+=ido){
						chjk[i-1]=wa[idij-1]*c1jk[i-1]+wa[idij]*c1jk[i];
						chjk[i]=wa[idij-1]*c1jk[i]-wa[idij]*c1jk[i-1];
					}
				}
			}
		}
		else{
		
			unsigned int is=1;
			unsigned int j;
			double *chjk,*c1jk;
			
			for(j=1,chjk=ch+chz,c1jk=c1+c1z;j<ip;j++, is+=ido){
			
				unsigned int k;
				
				for(k=0;k<l1;k++,chjk+=ido,c1jk+=ido){
				
					unsigned int idij=is;
					unsigned int i;
					
					for(i=2;i<ido;i+=2, idij+=2){
						chjk[i-1]=wa[idij-1]*c1jk[i-1]+wa[idij]*c1jk[i];
						chjk[i]=wa[idij-1]*c1jk[i]-wa[idij]*c1jk[i-1];
					}
				}
			}
		}
		if(nbd>=l1){
			
			unsigned int j;
			double *chjk,*c1jk;
			double *chjck,*c1jck;
	
			for(j=1,chjk=ch+chz,c1jk=c1+c1z,chjck=ch+chz*(ip-1),c1jck=c1+c1z*(ip-1);j<ipph;j++,chjck-=2*chz,c1jck-=2*c1z){
				
				unsigned int k;
				
				for(k=0;k<l1;k++,chjk+=ido,c1jk+=ido,chjck+=ido,c1jck+=ido){
				
					unsigned int i;
					
					for(i=2;i<ido;i+=2){
						c1jk[i-1]=chjk[i-1]+chjck[i-1];
						c1jck[i-1]=chjk[i]-chjck[i];
						c1jk[i]=chjk[i]+chjck[i];
						c1jck[i]=chjck[i-1]-chjk[i-1];
					}
				}
			}
		}
		else{
			
			unsigned int j;
			double *chjk, *c1jk, *chjck, *c1jck;
			
			for(j=1,chjk=ch+chz,c1jk=c1+c1z,chjck=ch+chz*(ip-1),c1jck=c1+c1z*(ip-1);j<ipph;j++,chjk+=chz,c1jk+=c1z,chjck-=chz,c1jck-=c1z){
				
				unsigned int i;
				
				for(i=2;i<ido;i+=2,chjk-=chz,c1jk-=c1z,chjck-=chz,c1jck-=chz){
				
					unsigned int k;
					
					for(k=0;k<l1;k++,chjk+=ido,c1jk+=ido,chjck+=ido,c1jck+=ido){
						c1jk[i-1]=chjk[i-1]+chjck[i-1];
						c1jck[i-1]=chjk[i]-chjck[i];
						c1jk[i]=chjk[i]+chjck[i];
						c1jck[i]=chjck[i-1]-chjk[i-1];
					}
				}
			}
		}
	}
	else{
		unsigned int ik;
		double *c20,*ch20;
		
		for(ik=0,c20=c2,ch20=ch2;ik<idl1;ik++,c20++,ch20++){
			*c20=*ch20;
		}
	}
	{
		unsigned int j;
		double *chjk,*c1jk,*chjck,*c1jck;
		
		for(j=1,chjk=ch+chz,c1jk=c1+c1z,chjck=ch+chz*(ip-1),c1jck=c1+c1z*(ip-1);j<ipph;j++,chjck-=2*chz,c1jck-=2*c1z){
			
			unsigned int k;
			
			for(k=0;k<l1;k++,chjk+=ido,c1jk+=ido,chjck+=ido,c1jck+=ido){
				*c1jk=*chjk+*chjck;
				*c1jck=*chjck-*chjk;
			}
		}
	}
	{
		double ar1=1.0;
		double ai1=0.0;
		unsigned int l;
		double *ch2l,*ch2lc,*c20,*c21,*c2ip;
		
		for(l=1,ch2l=ch2+ch2z,ch2lc=ch2+ch2z*(ip-1),c20=c2,c21=c2+c2z,c2ip=c2+c2z*(ip-1);l<ipph;l++,ch2l+=ch2z,ch2lc-=ch2z){
			double ar1h=dcp*ar1-dsp*ai1;
			ai1=dcp*ai1+dsp*ar1;
			ar1=ar1h;
			{
				unsigned int ik;
				
				for(ik=0;ik<idl1;ik++){
					ch2l[ik]=c20[ik]+ar1*c21[ik];
					ch2lc[ik]=ai1*c2ip[ik];
				}
			}
			{
				double dc2=ar1;
				double ds2=ai1;
				double ar2=ar1;
				double ai2=ai1;
				unsigned int j;
				double *c2j,*c2jc;
				
				for(j=2,c2j=c2+c2z*2,c2jc=c2+c2z*(ip-2);j<ipph;j++,c2j+=c2z,c2jc-=c2z){
				
					double ar2h=dc2*ar2-ds2*ai2;
					
					ai2=dc2*ai2+ds2*ar2;
					ar2=ar2h;
					{
						unsigned int ik;
						
						for(ik=0;ik<idl1;ik++){
							ch2l[ik]+=ar2*c2j[ik];
							ch2lc[ik]+=ai2*c2jc[ik];
						}
					}
				}
			}
		}
	}
	{
		unsigned int j;
		double *ch20,*c2j;
		
		for(j=1,ch20=ch2,c2j=c2+c2z;j<ipph;j++,ch20-=ch2z){
		
			unsigned int ik;
			
			for(ik=0;ik<idl1;ik++,c2j++,ch20++){
				*ch20+=*c2j;
			}
		}
	}
	if(ido>=l1){
	
		unsigned int k;
		double *cck0,*ch0k;
		
		for(k=0,cck0=cc,ch0k=ch;k<l1;k++,cck0+=ccz){
		
			unsigned int i;
			
			for(i=0;i<ido;i++,ch0k++){
				cck0[i]=*ch0k;
			}
		}
	}
	else{
	
		unsigned int i;
		
		for(i=0;i<ido;i++){
		
			unsigned int k;
			double *cck0,*ch0k;
			
			for(k=0,cck0=cc+i,ch0k=ch+i;k<l1;k++,cck0+=ccz,ch0k+=ido){
				*cck0=*ch0k;
			}
		}
	}
	{
		unsigned int j;
		double *chjk,*chjck;
		
		for(j=1,chjk=ch+chz,chjck=ch+(ip-1)*chz;j<ipph;j++,chjck-=chz*2){
		
			unsigned int j2=j+j+1;
			unsigned int k;
			double *cckj21,*cckj22;
			
			for(k=0,cckj21=cc+(j2-1)*ido,cckj22=cckj21-ido;k<l1;k++,cckj22+=ccz,cckj21+=ccz,chjk+=ido,chjck+=ido){
				cckj22[ido-1]=*chjk;
				*cckj21=*chjck;
			}
		}
	}
	if(ido==1) return;
	if(nbd>=l1){
	
		unsigned int j;
		double *chjk,*chjck;
		
		for(j=1,chjk=ch+chz,chjck=ch+(ip-1)*chz;j<ipph;j++,chjck-=chz*2){
		
			unsigned int j2=j+j+1;
			unsigned int k;
			double *cckj21,*cckj22;
			
			for(k=0,cckj21=cc+(j2-1)*ido,cckj22=cckj21-ido;k<l1;k++,cckj21+=chz,cckj22+=chz,chjk+=ido,chjck+=ido){
				
				unsigned int i;
				
				for(i=2;i<ido;i+=2){
				
					unsigned int ic=ido-i;
					
					cckj21[i-1]=chjk[i-1]+chjck[i-1];
					cckj22[ic-1]=chjk[i-1]-chjck[i-1];
					cckj21[i]=chjk[i]+chjck[i];
					cckj22[ic]=chjck[i]-chjk[i];
				}
			}
		}
		return;
	}
	{
		unsigned int j;
		double *chjk,*chjck;
		
		for(j=1,chjk=ch+chz,chjck=ch+(ip-1)*chz;j<ipph;j++,chjk+=chz,chjck-=chz){
		
			unsigned int j2=j+j+1;
			unsigned int i;
			
			for(i=2;i<ido;i+=2,chjk-=chz,chjck-=chz){
			
				unsigned int ic=ido-i;
				unsigned int k;
				double *cckj21,*cckj22;
				
				for(k=0,cckj21=cc+(j2-1)*ido,cckj22=cc+(j2-2)*ido;k<l1;k++,cckj21+=chz,cckj22+=chz,chjk+=ido,chjck+=ido){
					cckj21[i-1]=chjk[i-1]+chjck[i-1];
					cckj22[ic-1]=chjk[i-1]-chjck[i-1];
					cckj21[i]=chjk[i]+chjck[i];
					cckj22[ic]=chjck[i]-chjk[i];
				}
			}
		}
	}
	return;
}
