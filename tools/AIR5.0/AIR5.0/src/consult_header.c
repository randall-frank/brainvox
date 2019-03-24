/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 5/16/01 */

/*
 * float consult_header()
 *
 * This variant of consult_header() is designed to deal with the default AIR image format
 * in which data from each plane is represented sequentially. File formats that do not have
 * sequential data representation may require considerably more complex implementations.
 *
 * For non-sequential image formats, it may be necessary to turn off image decompression
 * 
 * Calls to this routine should always be preceeded by a call to open_header that returns a 
 * non-NULL value for fp->fp_hdr. Failure to do so will result in an error.
 *
 * This routine serves the following roles:
 *	1. If fp->fp_img is NULL, it will open INFILE as fp->fp_img
 *	2. If SKIP_PLANES is greater than zero, it will advance fp->fp_img by this number of planes
 *	3. It will return a scaling factor applicable to the plane now queued to be read from fp->fp_img
 *
 * Upon return:
 *	If successful: fp->errcode=0;
 *	If unsuccessful: fp->errcode will contain an appropriate error code
 *
 * The final call to this routine should be followed by a call to close_header
 *
 * The integer variable 'flag' is designed to carry additional information to the routine that 
 * actually reads the data. The first two values are reserved for global minimum and maximum information.
 * The remaining six entries are unused in the default implementation. Defining a new member of the AIR_Fptrs struct
 * in AIR.h is an alternate (and probably better) way to track any information that is needed for other image
 * formats (e.g., you might need to track an absolute plane number).
 *
 *  Guidelines for routines calling this function:
 *
 * 1. Call open_header() before calling this routine (this will open and read the header and set fp->fp_img to NULL).
 * 2. Check fp.errcode to make sure that open_header() succeeded.
 * 3. You can call this function repeatedly to access subsequent (but not previous) planes
 * 4. Check fp.errcode after each call to this function to identify errors
 * 5. Call close_header() after the last call to this function to assure that all files are closed
 * 6. If you are reading compressed images, and do not read all of the planes in the image,
 *	you will need to consume the remainder of the piped input stream
 *	using: "while(getc(fp->fp_img));" before you close the file, otherwise pclose() will generate an error.
 * 7. It is strongly recommended that programs that must close and reopen files for each input plane should not
 *	support reading compressed images (a 100 plane image will be uncompressed 100 times). The compression
 *	status of the image can be checked after the first call to this routine by checking fp->image_was_compressed
 *
 */

#include "AIR.h"
#include "HEADER.h"

float AIR_consult_header(const char *infile, struct AIR_Fptrs *fp, struct AIR_Key_info *stats, const unsigned int skip_planes, /*@unused@*/ int *flag)

{
    /* If initialization wasn't done--reject*/
    if(!fp->fp_hdr){
        printf("%s: %d: ",__FILE__,__LINE__);
        printf("Coding error\n");
        fp->errcode=AIR_HEADER_INIT_CODING_ERROR;
        return 0.0;
    }

    /* Position the pointer and compute the scale */
    {
        if(!fp->fp_img){

            char *filename=malloc((strlen(infile)+strlen(AIR_CONFIG_IMG_SUFF)+1)*sizeof(char));
            if(!filename){
                printf("%s: %d: ",__FILE__,__LINE__);
                printf("memory allocation failure\n");
                fp->errcode=AIR_MEMORY_ALLOCATION_ERROR;
                return 0.0;
            }
            strcpy(filename,infile);
            AIR_clip_suffix(filename);
            strcat(filename, AIR_CONFIG_IMG_SUFF);
            
            fp->fp_img=AIR_fopen_decompress(filename, &fp->image_was_compressed, &fp->errcode);
            if(!fp->fp_img){
                printf("%s: %d: ",__FILE__,__LINE__);
                printf("unable to open input file %s",filename);
                switch(fp->image_was_compressed){
#if(AIR_CONFIG_DECOMPRESS!=0)
                    case 1:
                        printf("%s\n", AIR_CONFIG_COMPRESSED_SUFFIX);
                        break;
#endif
                    default:
                        printf("\n");
                        break;
                }
                free(filename);
                return 0.0;
            }
            free(filename);
        }
        if(skip_planes!=0){
#if(AIR_CONFIG_DECOMPRESS!=0)
            switch(fp->image_was_compressed){
                case 1:
                    /* We cannot fseek from a pipe */
                    /* Skip all but the last character without checking return value */
                    {
                        long int i;
                        for(i=0;i<(unsigned long int)skip_planes*(stats->bits/8)*stats->x_dim*stats->y_dim-1;i++) (void)getc(fp->fp_img);
                    }
                    /* Check return for last skipped value */
                    if(getc(fp->fp_img)==EOF){
                        printf("%s: %d: ",__FILE__,__LINE__);
                        printf("unable to advance to desired image location in file %s\n",infile);
                        fp->errcode=AIR_READ_IMAGE_FILE_ERROR;
                        return 0.0;
                    }
                    break;
                default:
#endif
                    if(fseek(fp->fp_img,(long int)skip_planes*(stats->bits/8)*stats->x_dim*stats->y_dim,SEEK_CUR)!=0){
                        printf("%s: %d: ",__FILE__,__LINE__);
                        printf("unable to seek in image file %s\n",infile);
                        fp->errcode=AIR_FSEEK_READ_ERROR;
                        return 0.0;
                    }
#if(AIR_CONFIG_DECOMPRESS!=0)
            }
#endif
        }
        fp->errcode=0;
        return 1.0;
    }
}
