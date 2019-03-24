/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 5/22/01 */

/*
 * initialize the work arrays wa and ifac which are used in both
 * cfftf1 and cfftb1. The  prime factorization of N and a
 * tabulation of the trigonometric functions are computed and
 * stored in ifac and wa respectively
 */

#include "AIR.h"

void AIR_cffti1(const unsigned int n, double *wa, unsigned int *ifac)

{
	unsigned int nl=n;	/* portion of n yet to be factored */
	unsigned int nf=0;	/* number of factors extracted */
	unsigned int ntry=1;	/* number being tested as a factor {3,4,2,5,7,9,11,13,15...} */
	AIR_Boolean reentering=FALSE; /* Non-zero when a factor has just been extracted */

	do{				
		for(;;){
			if(!reentering){
				switch(ntry){
					case 1:
						ntry=3;
						break;
					case 3:
						ntry=4;
						break;
					case 4:
						ntry=2;
						break;
					case 2:
						ntry=5;
						break;
					default:
						ntry+=2;
						break;
				}
			}
			else reentering=0;
			{
				unsigned int nq=nl/ntry;		/*Effective reentry point after finding a factor*/
				{
					unsigned int nr=nl%ntry; /* remainder */
					if(nr==0){
						reentering=TRUE;
						nl=nq;	/* Set nl to new quotient */
						break;	/* break out when nl is exact multiple of ntry */
					}
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
			unsigned int i=1;
			unsigned int l1=1;
			{
				unsigned int k1;
				
				for(k1=1;k1<=nf;k1++){
					unsigned int ip=ifac[k1+1];
					unsigned int ld=0;
					unsigned int l2=l1*ip;
					unsigned int ido=n/l2;
					unsigned int idot=ido+ido+2;
					{
						unsigned int j;
						
						for(j=1;j<ip;j++){
						
							unsigned int i1=i;
							wa[i-1]=1.0;
							wa[i]=0.0;
							ld+=l1;
							{
								double fi=0.0;
								double argld=ld*argh;
								{
									unsigned int ii;
									
									for(ii=4;ii<=idot;ii+=2){
										i+=2;
										fi+=1.0;
										{
											double arg=fi*argld;
											wa[i-1]=cos(arg);
											wa[i]=sin(arg);
										}
									}
								}
							}
							if(ip>5){
								wa[i1-1]=wa[i-1];
								wa[i1]=wa[i];
							}
						}
					}
					l1=l2;
				}
			}
		}
	}
}



