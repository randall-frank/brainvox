/* Copyright 1993-2001 Roger P. Woods, M.D. */
/* Modified: 5/22/01 */

/* void clip_suffix()
 *
 * This routine will clip the .x (e.g. .img) suffix from a file name	
 */

#include "AIR.h"

void AIR_clip_suffix(char *filename)

{
    /* Find the file name (i.e., non-path part) */
    char *file=strrchr(filename, AIR_CONFIG_PATH_SEP);
    if(!file) file=filename;
    else file++;
    
    /* Find the last '.'  the non-path part */
    {
        char *orig_suffix=strrchr(file,'.');
        if(!orig_suffix) return;
       
        {
#if AIR_CONFIG_DECOMPRESS
            /* Check for compression suffix */;
            if(strcmp(orig_suffix, AIR_CONFIG_COMPRESSED_SUFFIX)==0){

                if(strcmp(orig_suffix, file)!=0) *orig_suffix='\0'; /* Hide compression suffix from strcmp() */
                else return;	/* There was no file name, e.g., path/.gz */
            }
#endif
            /* Clip supported suffixes */
            {
                char *suffix=strrchr(file,'.');
                if(!suffix){
                    *orig_suffix='.';
                    return;	/* There was no clippable suffix, e.g., file.gz */
                }
                if(strcmp(suffix, file)==0){
                    *orig_suffix='.';
                    return;	/* Clipping doesn't leave a legitimate file name, e.g., path/.hdr.gz */
                }
                if(strcmp(suffix, AIR_CONFIG_IMG_SUFF)==0){
                    *suffix='\0';
                    return;
                }
                if(strcmp(suffix, AIR_CONFIG_HDR_SUFF)==0){
                    *suffix='\0';
                    return;
                }
            }
            
            /* Restore compression suffix since no clippable suffix was found */
            *orig_suffix='.';
            return;
        }
    }
}




