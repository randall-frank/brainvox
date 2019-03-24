/* Copyright 1997-2001 Roger P. Woods, M.D. */
/* Modified: 5/22/01 */

/* This program will compute the hash value of a volume	*/

#include "AIR.h"

AIR_Error AIR_do_identify(const char *input)

{
	AIR_Pixels ***datain;
	struct AIR_Key_info dimensions;
	
	/*Load the file*/
	
	{
		AIR_Error errcode;

		datain=AIR_load(input, &dimensions, TRUE, &errcode);
		if(!datain){
			return(errcode);
		}
	}
	printf("%010lu\n",AIR_hash(datain,&dimensions));
	AIR_free_vol3(datain);

	return 0;
}

