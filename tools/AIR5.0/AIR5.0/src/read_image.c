/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 12/8/01 */

/*
 *  This routine should be customized to the particular data
 *   type that you want to read.
 *
 *  This routine should work in concert with the routine
 *	consult_header which should have already positioned a
 *	pointer to the start of the data.
 *
 *  The underlying assumption here is that all of the data
 *	for a single plane will be located together at one
 *	location in the image file and that that the entire
 *	plane should be scaled (if at all)
 *	by the same amount.
 *
 *  If you need to know how many bits/pixel are wanted
 *	use 8*sizeof(AIR_Pixels) to find out.
 *
 *  If you need to know the maximum representable number, use
 * 	MAX_POSS_VALUE (defined in AIR.h).
 *
 *  The integer variable 'flag' is designed to carry additional
 *	coded information from the consult_header routine.
 *
 *  Generically, this routine should read one plane of data into
 *	the designated location.
 *
 *  If scaling!=1, data should be scaled by this factor
 *	multiplicatively
 *
 *  Return:
 *	0, if successful
 * 	error code, if not successful
 *
 *  Never close *fp_img, always test that it is not NULL
 *
 */

#include "AIR.h"

AIR_Error AIR_read_image(AIR_Pixels *data, FILE **fp_img, const struct AIR_Key_info *stats, const int *flag, const float scale)

/* Data is read from fp_img into data using dimensions from stats under the influence of flag and scale */

{
	/* Configured for AIR default format images*/

	if((!data || !*fp_img) || scale<=0.0){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("programming error\n");
		return AIR_READIMAGE_CODING_ERROR;
	}

	/* If the data is 8 bit (always presumed unsigned) then reading is easy */
	if(stats->bits==8){

		const unsigned int x_dim=stats->x_dim;
		const unsigned int y_dim=stats->y_dim;
#if(AIR_CONFIG_OUTBITS==8)
		if(fread(data,sizeof(AIR_Pixels),x_dim*y_dim,*fp_img)!=x_dim*y_dim){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("file read error\n");
			return AIR_READ_IMAGE_FILE_ERROR;
		}
		return 0;
#elif(AIR_CONFIG_OUTBITS==16)
		/*No need to rescale when converting 8 bit to 16 bit, just bit shift*/
		{
			unsigned long int i;

			for(i=0;i<x_dim*y_dim;i++){
				unsigned char temp8;
				unsigned short int temp16;

				if(fread(&temp8,sizeof(unsigned char),1,*fp_img)!=1){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("file read error\n");
					return AIR_READ_IMAGE_FILE_ERROR;
				}
				temp16=temp8;
				data[i]=temp16*256;
			}
			return 0;
		}
#endif
	}

	/* If the data is 16 bit, reading is more complicated
	 * 16 bit data can be of three types
	 * type 1: unsigned short int--can be read directly 
	 * type 2: signed short int with negative values undefined
	 * type 3: signed short int with maximally negative number actually representing zero
	 *
	 * Note that data from disk is being read into an unsigned short int
	 * All of the conversions assume that the machine uses the two's complement method
	 * to represent negative values (i.e., unsigned 32768-65535 corresponds to 
	 * signed -32768 to -1
	 */

	else if(stats->bits==16){

		unsigned long int i;
		const unsigned int x_dim=stats->x_dim;
		const unsigned int y_dim=stats->y_dim;

		for(i=0;i<x_dim*y_dim;i++){
			unsigned short int temp16;

			if(fread(&temp16,sizeof(short int),1,*fp_img)!=1){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("file read error\n");
				return AIR_READ_IMAGE_FILE_ERROR;
			}
			if (flag[2]) AIR_swapbytes(&temp16,sizeof(temp16)/2,1);

			if(flag[0]>=0){
				if(flag[1]<=32767){
					/*We are dealing with type 2 data--values >32767 undefined (see note above) */
					/*Values <32767 need to be bit shifted upwards*/ 
					if(temp16>32767) temp16=0;
					else temp16*=2;
				}
			}
			else{
				/*We are dealing with type 3 data--trade values <32768 with those >32767 (see note above)*/
				if(temp16>32767) temp16-=32768;
				else temp16+=32768;
			}
#if(AIR_CONFIG_OUTBITS==8)
			temp16*=scale;
			data[i]=temp16/256;
#elif(AIR_CONFIG_OUTBITS==16)
			data[i]=temp16;
#endif
		}
		return 0;
	}
	else{
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("cannot process %u bits/pixel in input image\n",stats->bits);
		return AIR_INFO_BITS_ERROR;
	}
}

