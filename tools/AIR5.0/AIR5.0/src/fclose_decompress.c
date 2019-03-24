/* Copyright 2001 Roger P. Woods, M.D. */
/* Modified 5/22/01 */

/*
 *	This routine should be used to close streams opened with fopen_decompress(). The value of
 *	compression_flag should be the same value returned from fopen_decompress().
 */
 

#include "AIR.h"

AIR_Error AIR_fclose_decompress(FILE *fp, const AIR_Boolean compression_flag)
{
#if(AIR_CONFIG_DECOMPRESS!=0)
    if(compression_flag){
        if(pclose(fp)!=0) return AIR_DECOMPRESS_READ_ERROR;
        return 0;
    }
    if(fclose(fp)!=0) return AIR_CANT_CLOSE_READ_ERROR;
    return 0;
#else
    if(compression_flag){
        printf("%s: %d: ",__FILE__,__LINE__);
        printf("Coding error\n");
        return AIR_BAD_CODE_READ_ERROR;
    }
    if(fclose(fp)) return AIR_CANT_CLOSE_READ_ERROR;
    return 0;
#endif
}
