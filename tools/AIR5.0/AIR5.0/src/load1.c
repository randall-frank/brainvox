/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 5/18/01 */

/*
 *  AIR_Pixels ***load1()
 *
 * This routine will read the header of a  binary
 *  file, read the file, and convert it into an 8 bit or 16 bit
 *  matrix (still with only 1's and 0's as values)
 *
 * Memory is deallocated and a NULL pointer returned
 *  in the event of any inconsistency
 *
 * Each new plane is assumed to start on a byte boundary.
 * Consequently, if x_dim*y_dim is not an exact multiple of 8,
 * the binary data is padded at the end of each plane of data	
 *
 * Returns:
 * 	pointer to the data if successfully read
 *	NULL pointer if not successfully read
 *	errcode will contain any error flags
 */

#include "AIR.h"

AIR_Pixels ***AIR_load1(const char *infile, struct AIR_Key_info *stats, AIR_Error *errcode)

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

		/*Verify bits/pixel*/
		if(stats->bits!=(unsigned int)sizeof(unsigned char)){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("header of %s indicates %u bits/pixel instead of expected %u\n",infile,stats->bits,(unsigned int)sizeof(unsigned char));
			*errcode=AIR_READ_NONASBINARY_FILE_ERROR;
			return NULL;
		}
	}

	/* Load the binary image and convert it to an 8 bit image */
	{
		size_t bytes_in_plane;
		unsigned char *bin_image;
		unsigned int x_dim=stats->x_dim;
		unsigned int y_dim=stats->y_dim;
		unsigned int z_dim=stats->z_dim;

		/* Load the binary image */
		{
			size_t bytes_in_file;

			/*Compute number of bytes in binary file plane*/
			bytes_in_plane=x_dim*y_dim/8;
			if (bytes_in_plane*8<x_dim*y_dim){
				bytes_in_plane++;
			}
			bytes_in_file=bytes_in_plane*z_dim;


			/*Allocate memory for the binary image*/
			bin_image=(unsigned char *)malloc(bytes_in_file);
			if(!bin_image){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("memory allocation failure\n");
				*errcode=AIR_MEMORY_ALLOCATION_ERROR;
				return NULL;
			}
			/* Allocate memory for the file name */
			{
				char *filename=malloc((strlen(infile)+strlen(AIR_CONFIG_IMG_SUFF)+1)*sizeof(char));
				if(!filename){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("memory allocation failure\n");
					free(bin_image);
					*errcode=AIR_MEMORY_ALLOCATION_ERROR;
					return NULL;
				}
				strcpy(filename, infile);
				AIR_clip_suffix(filename);
				strcat(filename, AIR_CONFIG_IMG_SUFF);

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
						printf("file read error for 1 bit per pixel file %s\n",filename);
						*errcode=AIR_READ_IMAGE_FILE_ERROR;
						(void)AIR_fclose_decompress(fp,compression_flag);
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
					*errcode=AIR_fclose_decompress(fp,compression_flag);
					if(*errcode!=0){
						printf("%s: %d: ",__FILE__,__LINE__);
						printf("failed to close file %s\n",filename);
						free(bin_image);
						free(filename);
						return NULL;
					}
				} /* End of file handling issues */

				free(filename);
			}
		} /* End of image loading */

		/* Convert the data to 8 or 16 bit for return */
		{
			/*Allocate memory for the 8 bit or 16 bit image*/

			AIR_Pixels ***image=AIR_create_vol3(x_dim, y_dim, z_dim);
			if(!image){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("memory allocation failure related to %s\n",infile);
				free(bin_image);
				*errcode=AIR_MEMORY_ALLOCATION_ERROR;
				return NULL;
			}

			/*Convert each plane of binary data into 8-bit data*/
			/*Take special precautions with final byte of binary*/
			/*data since it may contain excess pixels*/
			{
				unsigned int plane;

				for (plane=0;plane<z_dim;plane++){

					struct byte_bits {
						unsigned zeroth: 1;
						unsigned first: 1;
						unsigned second: 1;
						unsigned third: 1;
						unsigned fourth: 1;
						unsigned fifth: 1;
						unsigned sixth: 1;
						unsigned seventh: 1;
					};

					union u_type {
						unsigned char byte;
						struct byte_bits bit;
					} ones_and_zeros;
					{
						unsigned int t;
						
						for (t=0;t<bytes_in_plane-1;t++){
							ones_and_zeros.byte=bin_image[plane*bytes_in_plane+t];
							image[plane][0][t*8]=(AIR_Pixels)ones_and_zeros.bit.zeroth;
							image[plane][0][t*8+1]=(AIR_Pixels)ones_and_zeros.bit.first;
							image[plane][0][t*8+2]=(AIR_Pixels)ones_and_zeros.bit.second;
							image[plane][0][t*8+3]=(AIR_Pixels)ones_and_zeros.bit.third;
							image[plane][0][t*8+4]=(AIR_Pixels)ones_and_zeros.bit.fourth;
							image[plane][0][t*8+5]=(AIR_Pixels)ones_and_zeros.bit.fifth;
							image[plane][0][t*8+6]=(AIR_Pixels)ones_and_zeros.bit.sixth;
							image[plane][0][t*8+7]=(AIR_Pixels)ones_and_zeros.bit.seventh;
						}
					}
					ones_and_zeros.byte=bin_image[plane*bytes_in_plane+bytes_in_plane-1];
					image[plane][0][(bytes_in_plane-1)*8]=(AIR_Pixels)ones_and_zeros.bit.zeroth;
					if(bytes_in_plane*8-(x_dim*y_dim)<7){
						image[plane][0][(bytes_in_plane-1)*8+1]=(AIR_Pixels)ones_and_zeros.bit.first;
					}
					if(bytes_in_plane*8-(x_dim*y_dim)<6){
						image[plane][0][(bytes_in_plane-1)*8+2]=(AIR_Pixels)ones_and_zeros.bit.second;
					}
					if(bytes_in_plane*8-(x_dim*y_dim)<5){
						image[plane][0][(bytes_in_plane-1)*8+3]=(AIR_Pixels)ones_and_zeros.bit.third;
					}
					if(bytes_in_plane*8-(x_dim*y_dim)<4){
						image[plane][0][(bytes_in_plane-1)*8+4]=(AIR_Pixels)ones_and_zeros.bit.fourth;
					}
					if(bytes_in_plane*8-(x_dim*y_dim)<3){
						image[plane][0][(bytes_in_plane-1)*8+5]=(AIR_Pixels)ones_and_zeros.bit.fifth;
					}
					if(bytes_in_plane*8-(x_dim*y_dim)<2){
						image[plane][0][(bytes_in_plane-1)*8+6]=(AIR_Pixels)ones_and_zeros.bit.sixth;
					}
					if(bytes_in_plane*8-(x_dim*y_dim)<1){
						image[plane][0][(bytes_in_plane-1)*8+7]=(AIR_Pixels)ones_and_zeros.bit.seventh;
					}
				}
			} /* End of planewise conversion */

			free(bin_image);
			stats->bits=8*sizeof(AIR_Pixels);
			return image;

		} /* End of volume conversion */
	}
}
