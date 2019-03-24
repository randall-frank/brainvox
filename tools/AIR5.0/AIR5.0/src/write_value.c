/* Copyright 1994-2001 Roger P. Woods, M.D. */
/* Modified: 5/11/01 */


/*
 * writes a double into an ASCII file
 *
 * Returns:
 *	0 if successful
 *	errcode if unsuccessful
 */


#include "AIR.h"

AIR_Error AIR_write_value(const double value, const char *outfile, const char *suffix, const AIR_Boolean ow)

{
	const char *filename;
	char *tempfilename=NULL;

	/* See if outfile already ends with the proper suffix */
	{
		const char *dot=strrchr(outfile,'.');
		if(!dot) dot=outfile;
		if(strcmp(outfile, suffix)!=0){
			/* Allocate memory and copy outfile */
			tempfilename=malloc((strlen(outfile)+strlen(suffix)+1)*sizeof(char));
			if(!tempfilename){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("memory allocation failure associated with %s\n",outfile);
				return AIR_MEMORY_ALLOCATION_ERROR;
			}
			strcpy(tempfilename,outfile);
			AIR_clip_suffix(tempfilename);
			strcat(tempfilename, suffix);
			filename=tempfilename;
		}
		else filename=outfile;
	}

	/* Write the file */
	{
		/* Open the file if permitted to do so */
		if(!ow){
			AIR_Error errcode=AIR_fprobw(filename,ow);
			if(errcode!=0){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("Failed to write output file %s\n",filename);
				if(tempfilename) free(tempfilename);
				return errcode;
			}
		}
		{
			FILE *fp=fopen(filename,"w");
			if (!fp){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("Failed to write output file %s\n",filename);
				{
					AIR_Error errcode=AIR_fprobw(filename,ow);
					if(tempfilename) free(tempfilename);
					if(errcode==0) return AIR_UNSPECIFIED_FILE_WRITE_ERROR;
					return errcode;
				}
	        }

			/*Write out the value*/
			if(fprintf(fp,"%e\n",value)<0){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("error writing %s\n",filename);
				if(fclose(fp)!=0){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("failed to close file %s\n",filename);
					if(tempfilename) free(tempfilename);
					return AIR_CANT_CLOSE_WRITE_ERROR;
				}				
				if(tempfilename) free(tempfilename);
				return AIR_WRITE_SUFFIX_FILE_ERROR;
			}
			if(fclose(fp)!=0){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("failed to close file %s\n",filename);
				if(tempfilename) free(tempfilename);
				return AIR_CANT_CLOSE_WRITE_ERROR;
			}
		}
	}

	if(tempfilename) free(tempfilename);
	return 0;
}
