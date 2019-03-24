/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified 12/1/01 */

/*
 * float open_header()
 *
 *  This routine should be customized to the particular data
 *   type that you want to read.
 *
 *  There should be sufficient flexibility here to allow you
 *    to deal with headers and images in the same file,
 *    headers and images in two separate files. You can even
 *    deal with images all in separate files provided that
 *    you create a global header that will contain the total
 *    file dimensions in this last case.
 *
 *  Note that the plane numbering starts at zero, not at one
 *   consequently, the planes will end at one less than the
 *   z_dim that this routine loads into struct AIR_Key_info *stats
 *
 *  If you need to know how many bits/pixel are wanted (e.g
 *	for rescaling), use sizeof(AIR_Pixels) to find out.
 *
 *  If you need to know the maximum representable number, use
 * 	MAX_POSS_VALUE (defined in AIR.h).
 *
 *  The integer variable 'flag' is designed to carry additional
 *	coded information to the routine that actually reads
 *	the data.
 *
 *
 *  Generically,
 *    this routine should:
 *	1. open the header file	using fp->fp_hdr
 *	2. read data into struct AIR_Key_info *stats
 *		use the # bits/pixel for actual data on disk,
 *		not what you expect bits/pixel to become
 *		once the data is loaded.
 *	3. set *flag as required by read_image unless you
 *		plan to do this plane by plane
 *	4. if successful:
 *		leave fp->fp_hdr open
 *		return global scaling factor
 *	5. if a fatal error occurs:
 *		close fp->fp_hdr and make it NULL
 *		(don't close if you couldn't open)
 *		return 0 (zero)
 *	6. if a non-fatal error occurs:
 *		keep fp->fp_hdr open
 *		return the appropriate error code
 *
 *    	NOTE: close_header should always be called after this routine
 *
 *  Returns with fp->errcode
 *	0 when successful
 *	nonzero when errors occur--see fprob_errs() for codes
 *
 *
 *  Guidelines for routines calling this function:
 *
 * 1. This function should be called before consult_header
 * 2. Check for errors by testing whether fp.errcode!=0
 * 3. You can allocate memory for your image after calling this routine
 * 4. If you are loading multiple planes from the same data
 *	set, don't invoke close_header between planes.
 * 5. Every call to this routine should have a corresponding call to close_header
 * 6. Don't forget to verify and/or correct bits/pixel once
 *	loading of data is completed so that it will reflect
 *	the data in RAM, not the data on the disk
 */

#include "AIR.h"
#include "HEADER.h"

int swapanalyzehdr(struct AIR_hdr *hdr)
{
	if (hdr->sizeof_hdr == (int)sizeof(struct AIR_hdr)) return 0 ;

#if(AIR_CONFIG_AUTO_BYTESWAP!=0)

	/* See if swapping the header will help */
	AIR_swapbytes(&(hdr->sizeof_hdr),sizeof(hdr->sizeof_hdr)/2,1);

	if (hdr->sizeof_hdr != (int)sizeof(struct AIR_hdr)) return -1;
	
	AIR_swapbytes(&(hdr->extents),sizeof(hdr->extents)/2,1);
	AIR_swapbytes(&(hdr->dims),sizeof(hdr->dims)/2,1);
	AIR_swapbytes(&(hdr->x_dim),sizeof(hdr->x_dim)/2,1);
	AIR_swapbytes(&(hdr->y_dim),sizeof(hdr->y_dim)/2,1);
	AIR_swapbytes(&(hdr->z_dim),sizeof(hdr->z_dim)/2,1);
	AIR_swapbytes(&(hdr->t_dim),sizeof(hdr->t_dim)/2,1);
	AIR_swapbytes(&(hdr->datatype),sizeof(hdr->datatype)/2,1);
	AIR_swapbytes(&(hdr->bits),sizeof(hdr->bits)/2,1);
	AIR_swapbytes(&(hdr->x_size),sizeof(hdr->x_size)/2,1);
	AIR_swapbytes(&(hdr->y_size),sizeof(hdr->y_size)/2,1);
	AIR_swapbytes(&(hdr->z_size),sizeof(hdr->z_size)/2,1);
	AIR_swapbytes(&(hdr->glmax),sizeof(hdr->glmax)/2,1);
	AIR_swapbytes(&(hdr->glmin),sizeof(hdr->glmin)/2,1);

	return 1 ;
#else
	return -1;
#endif
}


float AIR_open_header(const char *infile, struct AIR_Fptrs *fp, struct AIR_Key_info *stats, int *flag)

{
    fp->fp_hdr=NULL;
    fp->fp_img=NULL;
    fp->errcode=0;

    {
	struct AIR_hdr hdr;
        char *filename=malloc((strlen(infile)+strlen(AIR_CONFIG_HDR_SUFF)+1)*sizeof(char));
        if(!filename){
        
            printf("%s: %d: ",__FILE__,__LINE__);
            printf("memory allocation failure related to %s\n",infile);

            fp->errcode=AIR_MEMORY_ALLOCATION_ERROR;
            return 0.0;
        }
        strcpy(filename,infile);
        AIR_clip_suffix(filename);
        strcat(filename,AIR_CONFIG_HDR_SUFF);
                
        /* Open the header */
        {
            fp->fp_hdr=AIR_fopen_decompress(filename, &fp->header_was_compressed, &fp->errcode);
            if(!fp->fp_hdr){
                printf("%s: %d: ",__FILE__,__LINE__);
                printf("unable to open input file %s",filename);
                
                switch(fp->header_was_compressed){
#if(AIR_CONFIG_DECOMPRESS!=0)
                    case 1:
                        printf("%s\n",AIR_CONFIG_COMPRESSED_SUFFIX);
                        break;
#endif
                    default:
                        printf("\n");
                        break;                        
                }
                free(filename);
                return 0.0;
            }
        }
        if(fread(&hdr,1,sizeof(struct AIR_hdr),fp->fp_hdr)!=sizeof(struct AIR_hdr)){
        
            printf("%s: %d: ",__FILE__,__LINE__);
            printf("failed to read data from file %s",filename);

            switch(fp->header_was_compressed){
#if(AIR_CONFIG_DECOMPRESS!=0)
                case 1:
                    printf("%s\n",AIR_CONFIG_COMPRESSED_SUFFIX);
                    break;
#endif
                default:
                    printf("\n");
                    break;                        
            }
            free(filename);
            (void)AIR_fclose_decompress(fp->fp_hdr,fp->header_was_compressed);
            fp->fp_hdr=NULL;
            fp->errcode=AIR_READ_HEADER_FILE_ERROR;
            return 0.0;
        }
        /* Since header is standalone file, verify that it is correct length */
        if(fgetc(fp->fp_hdr)!=EOF){
        
            printf("%s: %d: ",__FILE__,__LINE__);
            printf("more data than expected was found in file %s",filename);

            switch(fp->header_was_compressed){
#if(AIR_CONFIG_DECOMPRESS!=0)
                case 1:
                    printf("%s\n",AIR_CONFIG_COMPRESSED_SUFFIX);
                    break;
#endif
                default:
                    printf("\n");
                    break;                        
            }
            free(filename);
            (void)AIR_fclose_decompress(fp->fp_hdr,fp->header_was_compressed);
            fp->fp_hdr=NULL;
            fp->errcode=AIR_READ_HEADER_FILE_ERROR;
            return 0.0;
        }
	/*Copy required data from header struct and check data integrity */
	{
			{
				int swapheader=swapanalyzehdr(&hdr);
				if(swapheader<0){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("invalid header for file %s", filename);
					
					switch(fp->header_was_compressed){
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
					(void)AIR_fclose_decompress(fp->fp_hdr,fp->header_was_compressed);
					fp->fp_hdr=NULL;
					fp->errcode=AIR_INFO_HLENGTHFIELD_ERROR;
					return 0.0;
				}
				flag[2]=swapheader;
			}

			/* Convert negative header values to positives without comment, errors arise below */
            stats->bits=(unsigned int)hdr.bits;
            stats->x_dim=(unsigned int)abs(hdr.x_dim);
            stats->y_dim=(unsigned int)abs(hdr.y_dim);
            stats->z_dim=(unsigned int)abs(hdr.z_dim);
            stats->x_size=fabs(hdr.x_size);
            stats->y_size=fabs(hdr.y_size);
            stats->z_size=fabs(hdr.z_size);

            /* Check for locally fatal unsupported bits */
            if(hdr.bits!=8 && hdr.bits!=16 && hdr.bits!=1){
            
                printf("%s: %d: ",__FILE__,__LINE__);
                printf("cannot read %i bit/pixel data from file %s", hdr.bits, filename);

                switch(fp->header_was_compressed){
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
                (void)AIR_fclose_decompress(fp->fp_hdr,fp->header_was_compressed);
                fp->fp_hdr=NULL;
                fp->errcode=AIR_INFO_BITS_ERROR;
                return 0.0;
            }

            /* Negative dimensions return an error, but are not fatal to completion of subroutine */
            if(hdr.x_dim<=0||hdr.y_dim<=0||hdr.z_dim<=0){
            
                printf("%s: %d: ",__FILE__,__LINE__);
                printf("invalid dimensions less than or equal to zero in header file %s", filename);

                switch(fp->header_was_compressed){
#if(AIR_CONFIG_DECOMPRESS!=0)
                    case 1:
                        printf("%s\n",AIR_CONFIG_COMPRESSED_SUFFIX);
                        break;
#endif
                    default:
                        printf("\n");
                        break;                        
                }
                fp->errcode=AIR_INFO_DIM_ERROR;
            }

            /* Negative sizes return an error, but are not fatal to completion of subroutine */
            if(hdr.x_size<=0.0||hdr.y_size<=0.0||hdr.z_size<=0.0){
            
                printf("%s: %d: ",__FILE__,__LINE__);
                printf("invalid voxel sizes less than or equal to zero in header file %s",filename);
                
                switch(fp->header_was_compressed){
#if(AIR_CONFIG_DECOMPRESS!=0)
                    case 1:
                        printf("%s\n",AIR_CONFIG_COMPRESSED_SUFFIX);
                        break;
#endif
                    default:
                        printf("\n");
                        break;                        
                }
                fp->errcode=AIR_INFO_SIZE_ERROR;
            }
	}

	/* Deal with issues of image intensity and return value */
	{
            double value=1.0;

            flag[0]=hdr.glmin;
            flag[1]=hdr.glmax;

            /*Calculate global rescaling factor if image is 16 bit and compilation is 8 bit*/
#if(AIR_CONFIG_OUTBITS==8)
            if(stats->bits==16){
                if(hdr.glmin>=0){
                    if(hdr.glmax<=0){
                        printf("%s: %d: ",__FILE__,__LINE__);
                        printf("global maximum is <= global minimum in header file %s",filename);
                        switch(fp->header_was_compressed){
#if(AIR_CONFIG_DECOMPRESS!=0)
                            case 1:
                                printf("%s\n",AIR_CONFIG_COMPRESSED_SUFFIX);
                                break;
#endif
                            default:
                                printf("\n");
                                break;                        
                        }
                        value=0.0;
                        fp->errcode=AIR_INFO_LIMIT_ERROR;
                    }
                    else if(hdr.glmax>32767) value=65535.0/hdr.glmax;
                    else value=32767.0/hdr.glmax;
                }
                else{
                    if(hdr.glmax<=-32768){
                        printf("%s: %d: ",__FILE__,__LINE__);
                        printf("global maximum is out of possible range in header file %s",filename);
                        
                        switch(fp->header_was_compressed){
#if(AIR_CONFIG_DECOMPRESS!=0)
                            case 1:
                                printf("%s\n",AIR_CONFIG_COMPRESSED_SUFFIX);
                                break;
#endif
                            default:
                                printf("\n");
                                break;                        
                        }                        
                        
                        value=0.0;
                        fp->errcode=AIR_INFO_LIMIT_ERROR;
                    }
                    value=65535.0/(hdr.glmax+32768);
                }
            }
#endif
        
            free(filename);

            if(value==0.0){
                (void)AIR_fclose_decompress(fp->fp_hdr,fp->header_was_compressed);
                fp->fp_hdr=NULL;
            }
            return value;
        }
    }
}
