/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 5/8/01 */

/* AIR_Pixels ***load()
 *
 * This routine reads the header of a file, allocates sufficient
 *  memory, and reads the file
 *
 * Returns:
 *	pointer to the data if successfully read
 *	NULL pointer if not successfully read
 *	errcode will contain any error flags
 *
 * Unsuccessful reading can be due to inability to read header,
 *  inconsistency between bit/pixel in header and bits/pixel expected,
 *  or inability to read the image file
 * Memory is deallocated before returning in the event of any inconsistency
 *
 * if stats->bits!=0, binary files will be converted and loaded
 */

#include "AIR.h"

AIR_Pixels ***AIR_load(const char *infile, struct AIR_Key_info *stats, const AIR_Boolean binaryok, AIR_Error *errcode)

{
    struct AIR_Fptrs fps;
    int flags[8];
    float globalscale;

    *errcode=0;

    /* Read the header */
    {    
        globalscale=AIR_open_header(infile,&fps,stats,flags);
        if(fps.errcode!=0){
            *errcode=fps.errcode;
            AIR_close_header(&fps);
            return NULL;
        }

        /*Deal with binary images by converting if allowed and refusing if not allowed*/
        if(stats->bits==1){
            AIR_close_header(&fps);
            if(fps.errcode!=0){
                printf("%s: %d: ",__FILE__,__LINE__);
                printf("Failed to close a header associated with %s\n",infile);
                *errcode=fps.errcode;
                return NULL;
            }
            if(binaryok) return AIR_load1(infile,stats,errcode);
            else{
                printf("%s: %d: ",__FILE__,__LINE__);
                printf("%s is a 1-bit per pixel file and was therefore not loaded\n",infile);
                *errcode=AIR_READ_BINARYASNON_FILE_ERROR;
                return NULL;
            }
        }
    }

    /* Allocate, name and load the image */
    {
        /*Allocate memory for the image*/
        unsigned int x_dim=stats->x_dim;
        unsigned int y_dim=stats->y_dim;
        unsigned int z_dim=stats->z_dim;
    
        AIR_Pixels ***image=AIR_create_vol3(x_dim, y_dim, z_dim);
        if(!image){
                printf("%s: %d: ",__FILE__,__LINE__);
                printf("memory allocation failure associated with %s\n",infile);
                AIR_close_header(&fps);
                *errcode=AIR_MEMORY_ALLOCATION_ERROR;
                return NULL;
        }

        /* Construct name for image and load it */
        {
            char *filename=malloc((strlen(infile)+strlen(AIR_CONFIG_IMG_SUFF)+1)*sizeof(char));
            if(!filename){
                printf("%s: %d: ",__FILE__,__LINE__);
                printf("memory allocation failure associated with %s\n",infile);
                *errcode=AIR_MEMORY_ALLOCATION_ERROR;
                AIR_close_header(&fps);
                AIR_free_vol3(image);
                return NULL;
            }
            strcpy(filename, infile);
            AIR_clip_suffix(filename);
            strcat(filename, AIR_CONFIG_IMG_SUFF);

            /*Read the image*/
            {
                unsigned int i;

                for (i=0;i<stats->z_dim;i++){

                    float scale=AIR_consult_header(filename,&fps,stats,0,flags);
                    if(fps.errcode!=0){
                        printf("%s: %d: ",__FILE__,__LINE__);
                        printf("failed to get needed information from header regarding plane %i in image file %s\n",i+1,filename);
                        *errcode=fps.errcode;
                        AIR_close_header(&fps);
                        AIR_free_vol3(image);
                        free(filename);
                        return NULL;
                    }
                    *errcode=AIR_read_image(*image[i],&(fps.fp_img),stats,flags,scale*globalscale);
                    if(*errcode!=0){
                        printf("%s: %d: ",__FILE__,__LINE__);
                        printf("failed to read plane %i in image file %s\n",i+1,filename);
                        AIR_close_header(&fps);
                        AIR_free_vol3(image);
                        free(filename);
                        return NULL;
                    }
                }
            }
            
            /* Exhaust input stream */
            /* This foregoes opportunity to check that all input data has been read in favor */
            /* of being able to handle data where the last plane doesn't end at the end of the file */
            
            while(fgetc(fps.fp_img)!=EOF);
            
            /* Close FILE pointer's */
            AIR_close_header(&fps);
            if(fps.errcode!=0){
                printf("%s: %d: ",__FILE__,__LINE__);
                printf("failed to close the image or header file %s\n",filename);
                *errcode=fps.errcode;
                AIR_free_vol3(image);
                free(filename);
                return NULL;
            }
            stats->bits=8*sizeof(AIR_Pixels);
            free(filename);
        }
        return image;	
    }
}

