/* Copyright 2001 Roger P. Woods, M.D. */
/* Modified 12/1/01 */

/* 
 * Converts endianness of an array
 */
 
#include "AIR.h"

void AIR_swapbytes(void *array, const size_t byte_pairs, const size_t elements)
{
	unsigned char *p=(unsigned char *)array;
	size_t i;
	
	for(i=0;i<elements;i++){
	
		unsigned char *lo=p;
		p+=byte_pairs*2;
		{
			unsigned char *hi=p-1;
			size_t j;
			
			for(j=0;j<byte_pairs;j++,lo++,hi--){
			
				unsigned char c=*lo;
				*lo=*hi;
				*hi=c;
			}
		}
	}
}

