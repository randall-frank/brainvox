/*Copyright 1995-2001 Roger P.Woods, M.D.*/
/* Modified 5/5/01 */

#include "AIR.h"

void AIR_radbg(const unsigned int ido, const unsigned int ip, const unsigned int l1, const unsigned int idl1, double *cc, double *c1, double *c2, double *ch, double *ch2, const double *wa)	

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
	
	const unsigned int nbd=(ido-1)/2;
	const unsigned int ipph=(ip+1)/2;
	
	if(ido>=l1){
	
		unsigned int k;
		double *ch0k,*cck0;
		
		for(k=0,ch0k=ch,cck0=cc;k<l1;k++,cck0+=ccz){
		
			unsigned int i;
			
			for(i=0;i<ido;i++,ch0k++){
				*ch0k=cck0[i];
			}
		}
	}
	else{
	
		unsigned int i;
		
		for(i=0;i<ido;i++){
		
			unsigned int k;
			double *ch0k,*cck0;
			
			for(k=0,ch0k=ch+i,cck0=cc+i;k<l1;k++,ch0k+=ido,cck0+=ccz){
				*ch0k=*cck0;
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
			
			for(k=0,cckj21=cc+(j2-1)*ido,cckj22=cckj21-ido;k<l1;k++,chjk+=ido,chjck+=ido,cckj21+=ccz,cckj22+=ccz){
				*chjk=cckj22[ido-1]+cckj22[ido-1];
				*chjck=*cckj21+*cckj21;
			}
		}
	}
	if(ido!=1){
		if(nbd>=l1){
		
			unsigned int j;
			double *chjk,*chjck;
			
			for(j=1,chjk=ch+chz,chjck=ch+(ip-1)*chz;j<ipph;j++,chjck-=chz*2){
			
				unsigned int k;
				double *cck2j,*cck2j1;
				
				for(k=0,cck2j=cc+2*j*ido,cck2j1=cck2j-ido;k<l1;k++,chjk+=ido,chjck+=ido,cck2j+=ccz,cck2j1+=ccz){
					
					unsigned int i;
					
					for(i=2;i<ido;i+=2){
					
						unsigned int ic=ido-i;
						
						chjk[i-1]=cck2j[i-1]+cck2j1[ic-1];
						chjck[i-1]=cck2j[i-1]-cck2j1[ic-1];
						chjk[i]=cck2j[i]-cck2j1[ic];
						chjck[i]=cck2j[i]+cck2j1[ic];
					}
				}
			}
		}
		else{
		
			unsigned int j;
			double *chjk,*chjck;
			
			for(j=1,chjk=ch+chz,chjck=ch+(ip-1)*chz;j<ipph;j++,chjk+=chz,chjck-=chz){
			
				unsigned int i;
				
				for(i=2;i<ido;i+=2,chjk-=chz,chjck-=chz){
				
					unsigned int ic=ido-i;
					unsigned int k;
					double *cck2j,*cck2j1;
					
					for(k=0,cck2j=cc+2*j*ido,cck2j1=cck2j-ido;k<l1;k++,chjk+=ido,chjck+=ido,cck2j+=ccz,cck2j1+=ccz){
						chjk[i-1]=cck2j[i-1]+cck2j1[ic-1];
						chjck[i-1]=cck2j[i-1]-cck2j1[ic-1];
						chjk[i]=cck2j[i]-cck2j1[ic];
						chjck[i]=cck2j[i]+cck2j1[ic];
					}
				}
			}
		}
	}
	{
		double ar1=1.0;
		double ai1=0.0;
		{
			unsigned int l;
			double *c2l,*c2lc,*ch20,*ch21,*ch2ip;
			
			for(l=1,c2l=c2+c2z,c2lc=c2+(ip-1)*c2z,ch20=ch2,ch21=ch2+ch2z,ch2ip=ch2+(ip-1)*ch2z;l<ipph;l++,c2l+=c2z,c2lc-=c2z){
		
				double ar1h=dcp*ar1-dsp*ai1;
				ai1=dcp*ai1+dsp*ar1;
				ar1=ar1h;
				
				{
					unsigned int ik;
					
					for(ik=0;ik<idl1;ik++){
						c2l[ik]=ch20[ik]+ar1*ch21[ik];
						c2lc[ik]=ai1*ch2ip[ik];
					}
				}
				{
					double dc2=ar1;
					double ds2=ai1;
					double ar2=ar1;
					double ai2=ai1;
					{
					
						unsigned int j;
						double *ch2j,*ch2jc;
						
						for(j=2,ch2j=ch2+2*ch2z,ch2jc=ch2+(ip-2)*ch2z;j<ipph;j++,ch2jc-=2*ch2z){
							double ar2h=dc2*ar2-ds2*ai2;
							ai2=dc2*ai2+ds2*ar2;
							ar2=ar2h;
							
							{
								unsigned int ik;
								
								for(ik=0;ik<idl1;ik++,ch2j++,ch2jc++){
									c2l[ik]+=ar2**ch2j;
									c2lc[ik]+=ai2**ch2jc;
								}
							}
						}
					}
				}
			}
		}
	}
	{
		unsigned int j;
		double *ch20,*ch2j;
		
		for(j=1,ch20=ch2,ch2j=ch2+chz;j<ipph;j++){
		
			unsigned int ik;
			
			for(ik=0;ik<idl1;ik++,ch2j++){
				ch20[ik]+=*ch2j;
			}
		}
	}
	{
		unsigned int j;
		double *chjk,*chjck,*c1jk,*c1jck;
		
		for(j=1,chjk=ch+chz,chjck=ch+(ip-1)*chz,c1jk=c1+c1z,c1jck=c1+(ip-1)*c1z;j<ipph;j++,chjck-=2*chz,c1jck-=2*c1z){
			
			unsigned int k;
			
			for(k=0;k<l1;k++,chjk+=ido,chjck+=ido,c1jk+=ido,c1jck+=ido){
				*chjk=*c1jk-*c1jck;
				*chjck=*c1jk+*c1jck;
			}
		}
	}
	if(ido!=1){
		if(nbd>=l1){
			{
				unsigned int j;
				double *chjk,*chjck,*c1jk,*c1jck;
				
				for(j=1,chjk=ch+chz,chjck=ch+(ip-1)*chz,c1jk=c1+c1z,c1jck=c1+(ip-1)*c1z;j<ipph;j++,chjck-=2*chz,c1jck-=2*c1z){
					
					unsigned int k;
					
					for(k=0;k<l1;k++,chjk+=ido,chjck+=ido,c1jk+=ido,c1jck+=ido){
					
						unsigned int i;
						
						for(i=2;i<ido;i+=2){
							chjk[i-1]=c1jk[i-1]-c1jck[i];
							chjck[i-1]=c1jk[i-1]+c1jck[i];
							chjk[i]=c1jk[i]+c1jck[i-1];
							chjck[i]=c1jk[i]-c1jck[i-1];
						}
					}
				}
			}
		}
		else{
		
			unsigned int j;
			double *chjk,*chjck,*c1jk,*c1jck;
			
			for(j=1,chjk=ch+chz,chjck=ch+(ip-1)*chz,c1jk=c1+c1z,c1jck=c1+(ip-1)*c1z;j<ipph;j++,chjk+=chz,chjck-=chz,c1jk+=c1z,c1jck-=c1z){
				
				unsigned int i;
				
				for(i=2;i<ido;i+=2,chjk-=chz,chjck-=chz,c1jk-=c1z,c1jck-=c1z){
				
					unsigned int k;
					
					for(k=0;k<l1;k++,chjk+=ido,chjck+=ido,c1jk+=ido,c1jck+=ido){
						chjk[i-1]=c1jk[i-1]-c1jck[i];
						chjck[i-1]=c1jk[i-1]+c1jck[i];
						chjk[i]=c1jk[i]+c1jck[i-1];
						chjck[i]=c1jk[i]-c1jck[i-1];
					}
				}
			}
		}
	}
	if(ido==1) return;
	{
		unsigned int ik;
		double *c20,*ch20;
		
		for(ik=0,c20=c2,ch20=ch2;ik<idl1;ik++,c20++,ch20++){
			*c20=*ch20;
		}
	}
	{
		unsigned int j;
		double *c1jk,*chjk;
		
		for(j=1,c1jk=c1+c1z,chjk=ch+chz;j<ip;j++){
		
			unsigned int k;
			
			for(k=0;k<l1;k++,c1jk+=ido,chjk+=ido){
				*c1jk=*chjk;
			}
		}
	}
	if(nbd<=l1){
		unsigned int is=1;
		unsigned int j;
		double *c1jk,*chjk;
		
		for(j=1,c1jk=c1+c1z,chjk=ch+chz;j<ip;j++,c1jk+=c1z,chjk+=chz, is+=ido){
		
			unsigned int idij=is;
			unsigned int i;
			
			for(i=2;i<ido;i+=2,c1jk-=c1z,chjk-=chz, idij+=2){
			
				unsigned int k;
				
				for(k=0;k<l1;k++,c1jk+=ido,chjk+=ido){
					c1jk[i-1]=wa[idij-1]*chjk[i-1]-wa[idij]*chjk[i];
					c1jk[i]=wa[idij-1]*chjk[i]+wa[idij]*chjk[i-1];
				}
			}
		}
	}
	else{
		unsigned int is=1;
		unsigned int j;
		double *c1jk,*chjk;
		
		for(j=1,c1jk=c1+c1z,chjk=ch+chz;j<ip;j++, is+=ido){
			{
				unsigned int k;
				
				for(k=0;k<l1;k++,c1jk+=ido,chjk+=ido){
				
					unsigned int idij=is;
					unsigned int i;
					
					for(i=2;i<ido;i+=2, idij+=2){
						idij+=2;
						c1jk[i-1]=wa[idij-1]*chjk[i-1]-wa[idij]*chjk[i];
						c1jk[i]=wa[idij-1]*chjk[i]+wa[idij]*chjk[i-1];
					}
				}
			}
		}
	}
	return;
}
