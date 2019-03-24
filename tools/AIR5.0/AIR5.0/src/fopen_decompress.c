/* Copyright 2001 Roger P. Woods, M.D. */
/* Modified 5/22/01 */

/*
 *	Any stream opened with this routine must be closed using fclose_decompress(). Furthermore,
 *	the value of compression_flag as set by this routine must be provided unchanged to
 *	fclose_decompress(). (A non-zero value indicates that pclose() must be used rather than fclose())
 *
 *	This routine will attempt to open a binary input stream from the file named
 *	INFILE, unless this file is not a regular file or symbolic link.
 *
 *	If INFILE exists (even if it is not a regular file), no attempt will
 *	be made to open a corresponding compressed image.
 * 
 *	If INFILE does not exist and the macro CONFIG_DECOMPRESS is non-zero (implying non-zero CONFIG_PIPE),
 * 	the routine will look for a corresponding compressed file. If a file
 * 	with the appropriate suffix exists and is a regular file or symbolic link,
 * 	an input stream will be attempted using a UNIX pipe from the UNIX decompression
 *	utility. The suffix indicating a compressed file (default is ".gz") and the command
 *	to decompress (default is "gzunzip -c ") are configured in AIR.h
 *
 *	The flag *compression_flag will be non-zero if and only if:
 *		1. the uncompressed file does not exist
 *		2. CONFIG_DECOMPRESS (and hence CONFIG_PIPE) is non-zero
 *		3. a file with the proper compression suffix appended to INFILE exists
 *
 *	The error message placed in *errcode will be applicable to INFILE, unless *compression_flag
 *	is non-zero, in which case it will be applicable to INFILE with the compression suffix appended.
 *
 *	This routine prints error messages only for memory allocation failures.
 */

#include "AIR.h"

FILE *AIR_fopen_decompress(const char *infile, AIR_Boolean *compression_flag, AIR_Error *errcode)
{
    {
        /* Is infile readable? */
        AIR_Error error=AIR_fprobr(infile);
        if(error==0){
            /* Open it for reading */
            FILE *fp=fopen(infile,"rb");
            if(fp){
                *compression_flag=FALSE;
                *errcode=0;
                return fp;
            }
            *compression_flag=FALSE;
            *errcode=AIR_UNSPECIFIED_FILE_READ_ERROR;
            return NULL;
        }
#if(AIR_CONFIG_DECOMPRESS==0)
        /* That's all we can do */
        *compression_flag=FALSE;
        *errcode=error;
        return NULL;
    }
#else
        /* We may be able to decompress, but should only try if INFILE didn't exist */
        if(error!=AIR_NO_FILE_READ_ERROR){
            *compression_flag=FALSE;
            *errcode=error;
            return NULL;
        }
    }
    /* INFILE does not exist */
    {
        /* Allocate enough memory to append the compression suffix to INFILE */
        char *filename=malloc((strlen(infile)+strlen(AIR_CONFIG_COMPRESSED_SUFFIX)+1)*sizeof(char));
        if(!filename){
            printf("%s: %d: ",__FILE__,__LINE__);
            printf("memory allocation failure related to %s\n",infile);
            *compression_flag=FALSE;
            *errcode=AIR_MEMORY_ALLOCATION_ERROR;
            return NULL;
        }
        strcpy(filename,infile);
        strcat(filename, AIR_CONFIG_COMPRESSED_SUFFIX); /* e.g., ".gz" */
        {
            /* Is the file with the compression suffix readable? */
            AIR_Error error=AIR_fprobr(filename);
            if(error!=0){

                free(filename);
                if(error==AIR_NO_FILE_READ_ERROR){
                    /* File doesn't even exist */
                    *compression_flag=FALSE;
                }
                else{
                    /* File exists, but isn't readable */
                    *compression_flag=TRUE;
                }
                /* error is appropriate in either case */
                *errcode=error;
                return NULL;
            }
            /* The file with the compression suffix is readable */
            {
                /* Allocate enough memory to generate the command for decompression to standard out */
                char zip[]=AIR_CONFIG_DECOMPRESS_COMMAND; /* e.g., "gunzip -c " */
                char *command=malloc((strlen(zip)+strlen(filename)+1)*sizeof(char *));
                if(!command){
                    printf("%s: %d: ",__FILE__,__LINE__);
                    printf("memory allocation failure related to %s\n",infile);
                    free(filename);
                    *compression_flag=TRUE;
                    *errcode=AIR_MEMORY_ALLOCATION_ERROR;
                    return NULL;
                }
                strcpy(command,zip);
                strcat(command,filename);
                {
                    /* Try to open a pipe from the output of the decompression command */
                    FILE *fp=popen(command,"r");
                    if(fp){
                        /* Apparent success (this doesn't mean that decompression is working, */
                        /* we won't know that for certain until pclose()) */
                        free(filename);
                        free(command);
                        *compression_flag=TRUE;
                        *errcode=0;
                        return fp;
                    }
                }
                /* Failure */
                free(filename);
                free(command);
                *compression_flag=TRUE;
                *errcode=AIR_UNSPECIFIED_FILE_READ_ERROR;
                return NULL;
            }
        }
    }
#endif
}
