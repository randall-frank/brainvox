/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 5/18/01 */

/*
 *  unsigned char ***load1as1()
 *
 * This routine will read the header of an ANALYZE binary
 *  file, and read the file as a binary file
 *
 * Memory is deallocated and a NULL pointer returned
 *  in the event of any inconsistency
 *
 * Compressed data will be decompressed, read, and recompressed
 *
 * Note that by convention, each new plane starts on a byte boundary
 * Consequently, if x_dim*y_dim is not an exact multiple of 8,
 * the binary data is padded at the end of each plane of data
 *
 * Note deliberate use of unsigned char
 *
 * Returns:
 * 	pointer to the data if successfully read
 *	NULL pointer if not successfully read
 *	*errcode will contain any error flags
 */

#include "AIR.h"

unsigned char *AIR_load1as1(const char *infile, struct AIR_Key_info *stats, AIR_Error *errcode)

{
	*errcode=0;

	/*Read and validate the header*/
	{
		struct AIR_Fptrs fps;
		{
			int flags[8];
			(void)AIR_open_header(infile,&fps,stats,flags);
		}
		if(fps.errcode!=0){
			*errcode=fps.errcode;
			AIR_close_header(&fps);
			return NULL;
		}
		AIR_close_header(&fps);
		if(fps.errcode!=0){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("failed to close a header associated with %s\n",infile);
			*errcode=fps.errcode;
			return NULL;
		}
        }

        /*Verify bits/pixel*/
        if(stats->bits!=(unsigned int)sizeof(unsigned char)){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("header of %s indicates %u bits/pixel instead of expected %u\n", infile, stats->bits, (unsigned int)sizeof(unsigned char));
			*errcode=AIR_READ_NONASBINARY_FILE_ERROR;
			return NULL;
        }

	/* Load the binary image */
	{
		unsigned char *bin_image;
		size_t bytes_in_file;

		/*Compute number of bytes in binary file plane*/
		{
			unsigned int x_dim=stats->x_dim;
			unsigned int y_dim=stats->y_dim;
			unsigned int z_dim=stats->z_dim;
			size_t bytes_in_plane=x_dim*y_dim/8;
			if (bytes_in_plane*8<x_dim*y_dim){
				bytes_in_plane++;
			}
			bytes_in_file=bytes_in_plane*z_dim;
		}

		/*Allocate memory for the binary image*/
		bin_image=(unsigned char *)malloc(bytes_in_file);
		if(!bin_image){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("memory allocation failure associated with %s\n",infile);
			*errcode=AIR_MEMORY_ALLOCATION_ERROR;
			return NULL;
		}
		{
			char *filename=malloc((strlen(infile)+strlen(AIR_CONFIG_IMG_SUFF)+1)*sizeof(char));
			if(!filename){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("memory allocation failure associated with %s\n",infile);
				*errcode=AIR_MEMORY_ALLOCATION_ERROR;
				free(bin_image);
				return NULL;
			}
			strcpy(filename,infile);
			AIR_clip_suffix(filename);
			strcat(filename,AIR_CONFIG_IMG_SUFF);

			/*Read the binary image*/
			{
				AIR_Boolean compression_flag;
				FILE *fp=AIR_fopen_decompress(filename, &compression_flag, errcode);

				if(!fp){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("unable to open 1 bit per pixel file %s\n",filename);
					free(bin_image);
					free(filename);
					return NULL;
				}

				if(fread(bin_image,1,bytes_in_file,fp)!=bytes_in_file){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("file read error for 1 bit per pixel file %s--check header\n",filename);
					*errcode=AIR_READ_IMAGE_FILE_ERROR;
					(void)AIR_fclose_decompress(fp, compression_flag);
					free(bin_image);
					free(filename);
					return NULL;
				}

				if(fgetc(fp)!=EOF){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("more data than header predicted in image file %s\n",filename);
					*errcode=AIR_READ_IMAGE_FILE_ERROR;
					(void)AIR_fclose_decompress(fp, compression_flag);
					free(bin_image);
					free(filename);
					return NULL;
				}
				*errcode=AIR_fclose_decompress(fp, compression_flag);
				if(*errcode!=0){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("failed to close file %s\n",filename);
					free(bin_image);
					free(filename);
					return NULL;
				}
			}
			free(filename);
		}
		return bin_image;
	}
}
