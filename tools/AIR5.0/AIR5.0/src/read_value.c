/* Copyright 1993-2001 Roger P. Woods, M.D. */
/* Modified: 5/22/01 */


/* double read_value()
 *
 * Reads a double from the ASCII normalization file and returns
 *  it.
 *
 * Returns:
 *	normalization factor if successfully read
 *	0 if unsuccessfully read or <0	
 *	errcode will contain any errors			
 */


#include "AIR.h"

double AIR_read_value(const char *infile, const char *suffix, AIR_Error *errcode)

{
	char *filename=malloc((strlen(infile)+strlen(suffix)+1)*sizeof(char));
        if(!filename){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("memory allocation failure associated with %s\n",infile);
			*errcode=AIR_MEMORY_ALLOCATION_ERROR;
			return 0.0;
        }
        strcpy(filename,infile);
        AIR_clip_suffix(filename);
        strcat(filename, suffix);
	*errcode=0;

	/* Read the file */
	{
		double 	value;
		{
			FILE *fp=fopen(filename,"rb");

			if (!fp){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("unable to open file %s\n",filename);
				*errcode=AIR_fprobr(filename);
				if(*errcode==0) *errcode=AIR_UNSPECIFIED_FILE_READ_ERROR;
				free(filename);
				return 0.0;
                        }
			if(fscanf(fp,"%le",&value)!=1){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("error reading file %s\n",filename);
				free(filename);
				(void)fclose(fp);
				*errcode=AIR_READ_SUFFIX_FILE_ERROR;
				return 0.0;
			}
			if(fclose(fp)!=0){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("failed to close file %s\n",filename);
				free(filename);
				*errcode=AIR_CANT_CLOSE_READ_ERROR;
				return 0.0;
			}
		}
		if(value<=0){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("file %s generates impossible normalization value of %e\n",filename,value);
			free(filename);
			*errcode=AIR_READ_SUFFIX_FILE_ERROR;
			return 0.0;
		}
		free(filename);
		return value;
	}
}
