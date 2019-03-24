/* Copyright 1998-2001 Roger P. Woods, M.D. */
/* Modified 5/22/01 */

/*
 * Tests whether save(...filename,permission,...)
 *  saveas1(...filename,permission...) or
 *  save1as1(...filename,permission...) will be successful
 *
 * returns an error number (see fprob_err() for decoding) if there will be a problem creating the specified output file
 *
 * returns 0 if no problems are anticipated
 *
 */

#include "AIR.h"

AIR_Error AIR_save_probw(const char *outfile, const AIR_Boolean permission)

{
	char *filename;

	/* Allocate memory and copy outfile */
	{
		unsigned int length=(unsigned int)strlen(AIR_CONFIG_HDR_SUFF);

		unsigned int test=(unsigned int)strlen(AIR_CONFIG_IMG_SUFF);
		if(test>length) length=test;

		filename=malloc((strlen(outfile)+length+1)*sizeof(char));
		if(!filename){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("memory allocation failure\n");
			return AIR_MEMORY_ALLOCATION_ERROR;
		}
		strcpy(filename,outfile);
	}

	/* Test the header, with automatic overwrite permission */
	{
		AIR_Error errcode;

		AIR_clip_suffix(filename);
		strcat(filename,AIR_CONFIG_HDR_SUFF);

		errcode=AIR_fprobw(filename,TRUE);
		if(errcode!=0){
			free(filename);
			return errcode;
		}
	}

	/* Test the image */
	{
		AIR_Error errcode;

		AIR_clip_suffix(filename);
		strcat(filename,AIR_CONFIG_IMG_SUFF);

		errcode=AIR_fprobw(filename,permission);
		if(errcode!=0){
			free(filename);
			return errcode;
		}
	}
	free(filename);
	return 0;
}
