/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/8/01 */

#include "AIR.h"

void AIR_passb(unsigned int *nac, const unsigned int ido, const unsigned int ip, const unsigned int l1, const unsigned int idl1, double *cc, double *c1, double *c2, double *ch, double *ch2, const double *wa)

{
	const unsigned int ccz=ip*ido;
	const unsigned int c1z=l1*ido;
	const unsigned int c2z=idl1;
	const unsigned int chz=l1*ido;
	const unsigned int ch2z=idl1;
	const unsigned int ccy=ido;
	const unsigned int c1y=ido;
	const unsigned int chy=ido;
 
	const unsigned int idot=ido/2;
	const unsigned int ipph=(ip+1)/2;
	const unsigned int idp=ip*ido;

	if(ido>=l1){
		{
			unsigned int j;
			
			for(j=1;j<ipph;j++){
			
				unsigned int jc=ip-j;
				unsigned int k;
				
				for(k=0;k<l1;k++){
				
					unsigned int i;
					
					for(i=0;i<ido;i++){

						/* ch[j][k][i]=cc[k][j][i]+cc[k][jc][i]; */
						*(ch+j*chz+k*chy+i)=*(cc+k*ccz+j*ccy+i)+*(cc+k*ccz+jc*ccy+i);

						/* ch[jc][k][i]=cc[k][j][i]-cc[k][jc][i]; */
						*(ch+jc*chz+k*chy+i)=*(cc+k*ccz+j*ccy+i)-*(cc+k*ccz+jc*ccy+i);
					}
				}
			}
		}
		{
			unsigned int k;
			
			for(k=0;k<l1;k++){
			
				unsigned int i;
				
				for(i=0;i<ido;i++){
					/* ch[0][k][i]=cc[k][0][i]; */
					*(ch+k*chy+i)=*(cc+k*ccz+i);
				}
			}
		}
	}
	else{
		{
			unsigned int j;
			
			for(j=1;j<ipph;j++){
			
				unsigned int jc=ip-j;
				unsigned int i;
				
				for(i=0;i<ido;i++){
				
					unsigned int k;
					
					for(k=0;k<l1;k++){
						/* ch[j][k][i]=cc[k][j][i]+cc[k][jc][i]; */
						*(ch+j*chz+k*chy+i)=*(cc+k*ccz+j*ccy+i)+*(cc+k*ccz+jc*ccy+i);
						/* ch[jc][k][i]=cc[k][j][i]-cc[k][jc][i]; */
						*(ch+jc*chz+k*chy+i)=*(cc+k*ccz+j*ccy+i)-*(cc+k*ccz+jc*ccy+i);
					}
				}
			}
		}
		{
			unsigned int i;
			
			for(i=0;i<ido;i++){
			
				unsigned int k;
				
				for(k=0;k<l1;k++){
					/* ch[0][k][i]=cc[k][0][i]; */
					*(ch+k*chy+i)=*(cc+k*ccz+i);
				}
			}
		}
	}
	{
		unsigned int idl=1;
		unsigned int inc=0;
		unsigned int l;
		
		for(l=1;l<ipph;l++, idl+=ido){
		
			unsigned int lc=ip-l;
			{
				unsigned int ik;
				
				for(ik=0;ik<idl1;ik++){

					/* c2[l][ik]=ch2[0][ik]+wa[idl-1]*ch2[1][ik]; */
					*(c2+l*c2z+ik)=*(ch2+ik)+wa[idl-1]**(ch2+ch2z+ik);

					/* c2[lc][ik]=wa[idl]*ch2[ip-1][ik]; */
					*(c2+lc*c2z+ik)=wa[idl]**(ch2+(ip-1)*ch2z+ik);

				}
			}
			{
				unsigned int idlj=idl;
				inc+=ido;
				{
					unsigned int j;
					
					for(j=2;j<ipph;j++){
					
						unsigned int jc=ip-j;
						idlj+=inc;
						/* note that idl%ido==1 so idlj!=idp */
						/* consequently next line could read idlj>=idp */
						/* but this would imply incorrectly that idlj could */
						/* become zero after subtraction */
						if(idlj>idp) idlj-=idp;
						{
							double war=wa[idlj-1];
							double wai=wa[idlj];
							unsigned int ik;
							
							for(ik=0;ik<idl1;ik++){
								/* c2[l][ik]+=war*ch2[j][ik]; */
								*(c2+l*c2z+ik)=*(c2+l*c2z+ik)+war**(ch2+j*ch2z+ik);
								/* c2[lc][ik]+=wai*ch2[jc][ik]; */
								*(c2+lc*c2z+ik)=*(c2+lc*c2z+ik)+wai**(ch2+jc*ch2z+ik);
							}
						}
					}
				}
			}
		}
	}
	{
		unsigned int j;
		
		for(j=1;j<ipph;j++){
		
			unsigned int ik;
			
			for(ik=0;ik<idl1;ik++){
				/* ch2[0][ik]+=ch2[j][ik]; */
				*(ch2+ik)=*(ch2+ik)+*(ch2+j*ch2z+ik);
			}
		}
	}
	{
		unsigned int j;
		
		for(j=1;j<ipph;j++){
		
			unsigned int jc=ip-j;
			unsigned int ik;
			
			for(ik=1;ik<idl1;ik+=2){


				/* ch2[j][ik-1]=c2[j][ik-1]-c2[jc][ik]; */
				*(ch2+j*ch2z+ik-1)=*(c2+j*c2z+ik-1)-*(c2+jc*c2z+ik);

				/* ch2[jc][ik-1]=c2[j][ik-1]+c2[jc][ik]; */
				*(ch2+jc*ch2z+ik-1)=*(c2+j*c2z+ik-1)+*(c2+jc*c2z+ik);

				/* ch2[j][ik]=c2[j][ik]+c2[jc][ik-1]; */
				*(ch2+j*ch2z+ik)=*(c2+j*c2z+ik)+*(c2+jc*c2z+ik-1);

				/* ch2[jc][ik]=c2[j][ik]-c2[jc][ik-1]; */
				*(ch2+jc*ch2z+ik)=*(c2+j*c2z+ik)-*(c2+jc*c2z+ik-1);
			}
		}
	}
	*nac=1;
	if(ido==2) return;
	*nac=0;
	{
		unsigned int ik;
		
		for(ik=0;ik<idl1;ik++){
			/* c2[0][ik]=ch2[0][ik]; */
			*(c2+ik)=*(ch2+ik);
		}
	}
	{
		unsigned int j;
		
		for(j=1;j<ip;j++){
		
			unsigned int k;
			
			for(k=0;k<l1;k++){
				/* c1[j][k][0]=ch[j][k][0]; */
				*(c1+j*c1z+k*c1y)=*(ch+j*chz+k*chy);
				/* c1[j][k][1]=ch[j][k][1]; */
				*(c1+j*c1z+k*c1y+1)=*(ch+j*chz+k*chy+1);
			}
		}
	}
	if(idot<=l1){
		unsigned int idij=1;
		unsigned int j;
		
		for(j=1;j<ip;j++, idij+=2){
		
			unsigned int i;
			
			for(i=3;i<ido;i+=2){
				idij+=2;
				{
					unsigned int k;
					
					for(k=0;k<l1;k++){
						/* c1[j][k][i-1]=wa[idij-1]*ch[j][k][i-1]-wa[idij]*ch[j][k][i]; */
						*(c1+j*c1z+k*c1y+i-1)=wa[idij-1]**(ch+j*chz+k*chy+i-1)-wa[idij]**(ch+j*chz+k*chy+i);
						/* c1[j][k][i]=wa[idij-1]*ch[j][k][i]+wa[idij]*ch[j][k][i-1]; */
						*(c1+j*c1z+k*c1y+i)=wa[idij-1]**(ch+j*chz+k*chy+i)+wa[idij]**(ch+j*chz+k*chy+i-1);
					}
				}
			}
		}
		return;
	}
	{
		unsigned int idj=1;
		unsigned int j;
		
		for(j=1;j<ip;j++, idj+=ido){

			unsigned int k;
			
			for(k=0;k<l1;k++){
				unsigned int idij=idj;
				unsigned int i;
				
				for(i=3;i<ido;i+=2){
					idij+=2;
					/* c1[j][k][i-1]=wa[idij-1]*ch[j][k][i-1]-wa[idij]*ch[j][k][i]; */
					*(c1+j*c1z+k*c1y+i-1)=wa[idij-1]**(ch+j*chz+k*chy+i-1)-wa[idij]**(ch+j*chz+k*chy+i);
					/* c1[j][k][i]=wa[idij-1]*ch[j][k][i]+wa[idij]*ch[j][k][i-1]; */
					*(c1+j*c1z+k*c1y+i)=wa[idij-1]**(ch+j*chz+k*chy+i)+wa[idij]**(ch+j*chz+k*chy+i-1);
				}
			}
		}
	}
}


