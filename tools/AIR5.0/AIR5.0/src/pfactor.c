/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 5/22/01 */

/* Returns the largest prime factor of n */
/* May return 4 e.g, if there are no prime factors > 4 */

#include "AIR.h"

unsigned int AIR_pfactor(const unsigned int n)

{
	unsigned int nl=n;	/* portion of n yet to be factored */	
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
			else reentering=FALSE;
				
			{
				unsigned int nr=nl%ntry; /* remainder */
				if(nr==0){
					reentering=TRUE;
					nl/=ntry;
					break;	/* break out when nl is exact multiple of ntry */
				}
			}
		}
	}while(nl!=1);
		
	return ntry;
}
