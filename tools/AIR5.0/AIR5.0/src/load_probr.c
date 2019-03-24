/* Copyright 1998-2001 Roger P. Woods, M.D. */
/* Modified 5/22/01 */

/*
 * Tests whether an image file will be readable
 * When called with a non-zero value for decompressable_read, 
 * it will test for files with appended CONFIG_COMPRESSED_SUFFIX as well
 *
 * returns an error code if there will be a problem loading the specified input file
 * returns 0 if no problems are anticipated
 */

#include "AIR.h"

AIR_Error AIR_load_probr(const char *infile, const AIR_Boolean decompressable_read)

{
    char *filename;

    /* Allocate memory and copy infile */
    {
        unsigned int length=(unsigned int)strlen(AIR_CONFIG_HDR_SUFF);
        unsigned int test=(unsigned int)strlen(AIR_CONFIG_IMG_SUFF);
        
        if(test>length) length=test;
        
#if(AIR_CONFIG_DECOMPRESS!=0)
        length+=strlen(AIR_CONFIG_COMPRESSED_SUFFIX);
#endif
        filename=malloc((strlen(infile)+length+1)*sizeof(char));
        if(!filename){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("memory allocation failure\n");
			return AIR_MEMORY_ALLOCATION_ERROR;
        }
        strcpy(filename,infile);
    }

    /* Test the header */
    {
        AIR_clip_suffix(filename);
        strcat(filename,AIR_CONFIG_HDR_SUFF);
        {
            AIR_Error orig_errcode=AIR_fprobr(filename);
            if(orig_errcode!=0){
#if(AIR_CONFIG_DECOMPRESS!=0)
                /* Try the compressed version */
                strcat(filename, AIR_CONFIG_COMPRESSED_SUFFIX);
                {
                    AIR_Error errcode=AIR_fprobr(filename);
                    if(errcode!=0){
                        free(filename);
                        return(orig_errcode); /* want original code returned */
                    }
                }
                filename[strlen(filename)-strlen(AIR_CONFIG_COMPRESSED_SUFFIX)]='\0';
#else
                free(filename);
                return orig_errcode;
#endif
            }
        }
    }
    AIR_clip_suffix(filename);
    strcat(filename,AIR_CONFIG_IMG_SUFF);

    /* Test the image */
    {
        AIR_Error orig_errcode=AIR_fprobr(filename);
        if(orig_errcode!=0){
            if(decompressable_read){
#if(AIR_CONFIG_DECOMPRESS!=0)
                /* Try the compressed version */
                strcat(filename, AIR_CONFIG_COMPRESSED_SUFFIX);
                {
                    AIR_Error errcode=AIR_fprobr(filename);
                    if(errcode!=0){
                        free(filename);
                        return(orig_errcode);
                    }
                    else{
                        free(filename);
                        return 0;
                    }
                }
#else
                free(filename);
                return(orig_errcode);
#endif
            }
            else{
#if AIR_CONFIG_DECOMPRESS
                /* See if the compressed file exists */
                strcat(filename, AIR_CONFIG_COMPRESSED_SUFFIX);
                {
                    AIR_Error errcode=AIR_fprobr(filename);
                    if(errcode!=0){
                        free(filename);
                        return(orig_errcode);
                    }
                    else{
                        free(filename);
                        return AIR_PLANEWISE_DECOMPRESS_READ_ERROR;
                    }
                }
#else
                free(filename);
                return(orig_errcode);
#endif
            }
        }
        free(filename);
        return 0;
    }
}

