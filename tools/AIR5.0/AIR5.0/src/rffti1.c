/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 11/11/01 */

/*
 * initialize the work arrays wa and ifac which are used in both
 * rfftf1 and rfftb1. The  prime factorization of N and a
 * tabulation of the trigonometric functions are computed and
 * stored in ifac and wa respectively
 */

#include "AIR.h"

void AIR_rffti1(const unsigned int n, double *wa, unsigned int *ifac)

{
	unsigned int nl=n; /* portion of n yet to be factored */
	unsigned int nf=0; /* number of factors extracted */
	unsigned int ntry=1; /* number being tested as a factor {4,2,3,5,7,9,11,13,15...} */
	AIR_Boolean reentering=FALSE; /* Non-zero when a factor has just been extracted */
		
		
	do{
		for(;;){
			if(!reentering){
				switch(ntry){
				
					case 1:
						ntry=4;
						break;
					case 4:
						ntry=2;
						break;
					case 2:
						ntry=3;
						break;
					case 3:
						ntry=5;
						break;
					default:
						ntry+=2;
						break;
				}
			}
			else reentering=FALSE;
			{
				unsigned int nr=nl%ntry; /* remainder */
				if(nr==0){
					reentering=TRUE;
					nl/=ntry; /*Reset nl to new quotient*/
					break;	/* break out when nl is exact multiple of ntry */
				}
			}
		}
		nf++;				/*The next factor has been found*/
		ifac[nf+1]=ntry;	/*The first two elements of ifac are reserved, append ntry*/
		if(ntry==2){
			if(nf!=1){
				{
					unsigned int i;
					
					for(i=nf;i>=2;i--){
						ifac[i+1]=ifac[i];	/*Shift isolated factor of 2 to the front of the list*/
					}
				}
				ifac[2]=2;
			}
		}
	}while(nl!=1);
	
	/*Complete reserved values in ifac */
	ifac[0]=n;			
	ifac[1]=nf;
	{
		double argh;
		{
			double tpi=8.0*atan(1.0);
			argh=tpi/n;
		}
		{
			unsigned int is=1;
			unsigned int l1=1;
			if(nf==1) return;
			{
				unsigned int k1;
				
				for(k1=1;k1<nf;k1++){
					unsigned int ip=ifac[k1+1];
					unsigned int ld=0;
					unsigned int l2=l1*ip;
					unsigned int ido=n/l2;
					{
						unsigned int j;
						
						for(j=1;j<ip;j++, is+=ido){
						
							unsigned int i=is;
							ld+=l1;
							{
								double argld=ld*argh;
								double fi=0.0;
								unsigned int ii;
								
								for(ii=3;ii<=ido;ii+=2, i+=2){
									fi+=1.0;
									{
										double arg=fi*argld;
										
										wa[i-1]=cos(arg);
										wa[i]=sin(arg);
									}
								}

							}
						}
					}
					l1=l2;
				}
			}
		}
	}
	return;
}
