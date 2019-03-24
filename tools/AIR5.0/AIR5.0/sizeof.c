/* Copyright 1995-2002 Roger P. Woods, M.D. */
/* Modified: 1/17/02 */

/*
 * This program prints out actual and expected sizes of
 *  variables and structures used in the AIR package.
 *
 * A routine for deriving the radix of the current machine is also
 * incorporated. The reference for the radix routine is:
 * Michael A. Malcolm, "Algorithms to reveal properties of floating-point
 * arithmetic",Communications of the ACM 1972;15:949-951.
 */


#include "AIRmain.h"
#include "HEADERmain.h"
#include "VERSION.h"
#include <float.h>

int main()

{
	printf("Sizes of variables on this machine (corresponding size in development environment):\n\n");
	printf("\tstruct AIR_Key_info %i (40)\n",(int)sizeof(struct AIR_Key_info));
	printf("\tstruct AIR_Air16 %i (720)\n",(int)sizeof(struct AIR_Air16));
	printf("\tstruct AIR_Oldair %i (688)\n",(int)sizeof(struct AIR_Oldair));
	printf("\tstruct AIR_Field %i (600)\n",(int)sizeof(struct AIR_Field));
	printf("\tstruct AIR_Warp %i (608)\n",(int)sizeof(struct AIR_Warp));
	printf("\tstruct AIR_Air168 %i (2384)\n",(int)sizeof(struct AIR_Air168));
	printf("\tstruct AIR_hdr %i (348)\n",(int)sizeof(struct AIR_hdr));
	printf("\tAIR_Pixels %i (1 or 2)\n",(int)sizeof(AIR_Pixels));
	printf("\n");
	printf("\tshort int %i (2)\n",(int)sizeof(short int));
	printf("\tint %i (4)\n",(int)sizeof(int));
	printf("\tlong int %i (4)\n",(int)sizeof(long int));
	printf("\tunsigned short int %i (2)\n",(int)sizeof(unsigned short int));
	printf("\tunsigned int %i (4)\n",(int)sizeof(unsigned int));
	printf("\tunsigned long int %i (4)\n",(int)sizeof(unsigned long int));

	printf("\tchar %i (1)\n",(int)sizeof(char));
	printf("\tunsigned char %i (1)\n",(int)sizeof(unsigned char));

	
	printf("\tfloat %i (4)\n",(int)sizeof(float));

	printf("\tdouble %i (8)\n",(int)sizeof(double));
	

	printf("\nThis is AIR version %s\n\n",VERSION);
	printf("Compile time options from AIR.h:\n");
	printf("AIR_CONFIG_AUTO_BYTESWAP %i\n",AIR_CONFIG_AUTO_BYTESWAP);
	printf("AIR_CONFIG_CONFIG %i\n",AIR_CONFIG_CONFIG);
#if(AIR_CONFIG_PIPES)
	printf("AIR_CONFIG_COMPRESSED_SUFFIX %s\n",AIR_CONFIG_COMPRESSED_SUFFIX);
	printf("AIR_CONFIG_DECOMPRESS %i\n",AIR_CONFIG_DECOMPRESS);
#if(AIR_CONFIG_DECOMPRESS)
	printf("AIR_CONFIG_DECOMPRESS_COMMAND %s\n",AIR_CONFIG_DECOMPRESS_COMMAND);
#endif
#endif
	printf("AIR_CONFIG_HDR_SUFF %s\n",AIR_CONFIG_HDR_SUFF);
	printf("AIR_CONFIG_IMG_SUFF %s\n",AIR_CONFIG_IMG_SUFF);
	printf("AIR_CONFIG_PIPES %i\n",AIR_CONFIG_PIPES);
	printf("AIR_CONFIG_MAX_POSS_VALUE %u\n",AIR_CONFIG_MAX_POSS_VALUE);
	printf("AIR_CONFIG_OUTBITS %i\n",AIR_CONFIG_OUTBITS);
	printf("AIR_CONFIG_PATHS %i\n",AIR_CONFIG_PATHS);
	printf("AIR_CONFIG_PIX_SIZE_ERR %e\n",AIR_CONFIG_PIX_SIZE_ERR);
	
#if(AIR_CONFIG_OUTBITS==16)
		printf("AIR_CONFIG_REP16 %i\n",AIR_CONFIG_REP16);
#endif
	printf("AIR_CONFIG_REQ_PERMS %i\n",AIR_CONFIG_REQ_PERMS);
	printf("AIR_CONFIG_THRESHOLD1 %i\n",AIR_CONFIG_THRESHOLD1);
	printf("AIR_CONFIG_THRESHOLD2 %i\n",AIR_CONFIG_THRESHOLD2);
	printf("AIR_CONFIG_VERBOSITY %i\n",AIR_CONFIG_VERBOSITY);

	printf("\n");
#if(AIR_CONFIG_OUTBITS==16)
	if(sizeof(AIR_Pixels)!=2){
		printf("COMPILE TIME CONFIGURATION ERROR RESULTING in %i BITS PER PIXEL INSTEAD OF 16\n",8*(int)sizeof(AIR_Pixels));
	}
#endif
#if(AIR_CONFIG_OUTBITS==8)
	if(sizeof(AIR_Pixels)!=1){
		printf("COMPILE TIME CONFIGURATION ERROR RESULTING IN %i BITS PER PIXEL INSTEAD OF 8\n",8*(int)sizeof(AIR_Pixels));
	}
#endif
#if AIR_CONFIG_DECOMPRESS
        printf("Decompression command: %sfilename%s%s > filename%s\n", AIR_CONFIG_DECOMPRESS_COMMAND, AIR_CONFIG_IMG_SUFF, AIR_CONFIG_COMPRESSED_SUFFIX, AIR_CONFIG_IMG_SUFF);
#endif
	return 0;
}
