/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 12/9/01 */

/*
 * Maps an external file pixel value onto the appropriate 
 * AIR internal representation of that pixel value
 *
 * Returns the AIR internal representation of value if successful
 *  errcode if not successful
 *
 */

#include "AIR.h"

AIR_Pixels AIR_map_value(const char *filename, long int value, AIR_Error *errcode)

{
	struct AIR_Key_info	dimensions;
	int flag[8];

	/* Read the header */
	{
		struct AIR_Fptrs fps;
		
		(void)AIR_open_header(filename,&fps,&dimensions,flag);
		if(fps.errcode!=0){
			*errcode=fps.errcode;
			AIR_close_header(&fps);
			return 0;
		}
		AIR_close_header(&fps);
		if(fps.errcode!=0){
			*errcode=fps.errcode;
			return 0;
		}
	}

	/* Adjust the value as needed */
#if(AIR_CONFIG_OUTBITS==16)
	if(dimensions.bits==8) value*=256;		/* Type 0 data */
	else if(dimensions.bits==16){
		if(flag[0]>=0){				/* Type 1 or 2 data */
			if(flag[1]<=32767) value*=2; 	/*Type 2 data */
		}
		else value+=32768;			/* Type 3 data */
	}
#elif(AIR_CONFIG_OUTBITS==8)
	if(dimensions.bits==16){
		if(flag[0]>=0){
			if(flag[1]>32767){
				/* Type 1 data */
				value*=(65535.0/flag[1]);
				value/=256;
			}
			else{
				/* Type 2 data */
				value*=2;
				value*=(32767.0/flag[1]);
				value/=256;
			}
		}
		else{
			/* Type 3 data */
			value+=32768;
			value*=(65535.0/(flag[1]+32768));
			value/=256;
		}
	}
#endif
	if(value<0){
		*errcode=AIR_THRESHOLD_TOO_SMALL_ERROR;
		return 0;
	}
	if(value>(long int)AIR_CONFIG_MAX_POSS_VALUE){
		*errcode=AIR_THRESHOLD_TOO_LARGE_ERROR;
		return 0;
	}
	*errcode=0;
	return (AIR_Pixels)value;
}
