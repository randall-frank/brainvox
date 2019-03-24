/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 12/21/01 */

/*
 * This routine will write a volume to disk as a
 * binary volume based on the thresholds provided
 *
 * Note that by convention, each new plane starts on a byte boundary
 * Consequently, if x_dim*y_dim is not an exact multiple of 8,
 * the binary data is padded at the end of each plane of data
 *
 * Values greater than or equal to min and less than or equal
 *  to max are converted to 1's, all other values are converted
 *  to 0's.
 *
 * Returns:
 *	0 if save was successful
 * 	error code if save was unsuccessful
 */

#include "AIR.h"

AIR_Error AIR_saveas1(AIR_Pixels ***pixels, const struct AIR_Key_info *stats, const char *outfile, const AIR_Boolean ow, const AIR_Pixels min, const AIR_Pixels max, const char *comment)

{
	unsigned char 	*bin_image;
	size_t bytes_in_file;

	/*Verify bits/pixel*/
	if(stats->bits!=8*sizeof(AIR_Pixels)){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("%s not saved because struct indicates %u bits/pixel instead of expected %u\n",outfile,stats->bits,8*(unsigned int)sizeof(AIR_Pixels));
		return AIR_WRITE_BADBITS_CODING_ERROR;
	}

	/*Verify acceptable pixel dimensions*/
	if(stats->x_dim<1 || stats->y_dim<1 || stats->z_dim<1){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("%s not saved because struct indicates impossible file dimension %u x %u x %u\n",outfile,stats->x_dim,stats->y_dim,stats->z_dim);
		return AIR_WRITE_BAD_DIM_ERROR;
	}

	/* Prepare the image data */
	{
		size_t bytes_in_plane;

		/*Compute number of bytes in binary file plane*/
		bytes_in_plane=stats->x_dim*stats->y_dim/8;
		if (bytes_in_plane*8<stats->x_dim*stats->y_dim){
			bytes_in_plane++;
		}
		bytes_in_file=bytes_in_plane*stats->z_dim;

		/*Allocate memory for the binary image*/
		bin_image=(unsigned char *)malloc(bytes_in_file);
		if(!bin_image){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("memory inadequate to create binary version of file %s\n",outfile);
			return AIR_MEMORY_ALLOCATION_ERROR;
		}

		/* Binarize the data */
		{
			AIR_Pixels ***image;

			/*Allocate memory to convert to 1's and 0's*/
			image=AIR_create_vol3(stats->x_dim,stats->y_dim,stats->z_dim);
			if(!image){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("memory inadequate to convert file %s to 1's and 0's\n",outfile);
				free(bin_image);
				return AIR_MEMORY_ALLOCATION_ERROR;
			}

			/*Convert to 1's and 0's*/
			{
				unsigned int k;
				for (k=0;k<stats->z_dim;k++){
					unsigned int j;
					for (j=0;j<stats->y_dim;j++){
						unsigned int i;
						for (i=0;i<stats->x_dim;i++){
							if(pixels[k][j][i]>=min && pixels[k][j][i]<=max){
								image[k][j][i]=1;
							}
							else image[k][j][i]=0;
						}
					}
				}
			}
	

			/*Assign values to binary matrix*/
			/*Pad at the end of each plane so that all planes start*/
			/*on a byte-boundary*/
			{
				unsigned int plane;

				for(plane=0;plane<stats->z_dim;plane++){

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

					/* Assign values to the struct byte_bits */
					{
						unsigned int t;

						for (t=0;t<bytes_in_plane-1;t++){
							ones_and_zeros.bit.zeroth=image[plane][0][t*8];
							ones_and_zeros.bit.first=image[plane][0][t*8+1];
							ones_and_zeros.bit.second=image[plane][0][t*8+2];
							ones_and_zeros.bit.third=image[plane][0][t*8+3];
							ones_and_zeros.bit.fourth=image[plane][0][t*8+4];
							ones_and_zeros.bit.fifth=image[plane][0][t*8+5];
							ones_and_zeros.bit.sixth=image[plane][0][t*8+6];
							ones_and_zeros.bit.seventh=image[plane][0][t*8+7];
							bin_image[plane*bytes_in_plane+t]=ones_and_zeros.byte;
						}
					}
					ones_and_zeros.bit.zeroth=image[plane][0][(bytes_in_plane-1)*8];
					if(bytes_in_plane*8-(stats->x_dim*stats->y_dim)<7){
						ones_and_zeros.bit.first=image[plane][0][(bytes_in_plane-1)*8+1];
					}
					if(bytes_in_plane*8-(stats->x_dim*stats->y_dim)<6){
						ones_and_zeros.bit.second=image[plane][0][(bytes_in_plane-1)*8+2];
					}
					if(bytes_in_plane*8-(stats->x_dim*stats->y_dim)<5){
						ones_and_zeros.bit.third=image[plane][0][(bytes_in_plane-1)*8+3];
					}
					if(bytes_in_plane*8-(stats->x_dim*stats->y_dim)<4){
						ones_and_zeros.bit.fourth=image[plane][0][(bytes_in_plane-1)*8+4];
					}
					if(bytes_in_plane*8-(stats->x_dim*stats->y_dim)<3){
						ones_and_zeros.bit.fifth=image[plane][0][(bytes_in_plane-1)*8+5];
					}
					if(bytes_in_plane*8-(stats->x_dim*stats->y_dim)<2){
						ones_and_zeros.bit.sixth=image[plane][0][(bytes_in_plane-1)*8+6];
					}
					if(bytes_in_plane*8-(stats->x_dim*stats->y_dim)<1){
						ones_and_zeros.bit.seventh=image[plane][0][(bytes_in_plane-1)*8+7];
					}
					bin_image[plane*bytes_in_plane+bytes_in_plane-1]=ones_and_zeros.byte;
				}
			}
			AIR_free_vol3(image);
		} /* Binarization complete */
	} /* Image data preparation complete */

	/* Save the data */
	{
		const char *filename;
		char *tempfilename=NULL;

		/* See if outfile already ends with the proper suffix */
		{
			const char *dot=strrchr(outfile,'.');
			if(!dot) dot=outfile;
			if(strcmp(dot,AIR_CONFIG_IMG_SUFF)!=0){
				/* Allocate memory and copy outfile */
				tempfilename=malloc((strlen(outfile)+strlen(AIR_CONFIG_IMG_SUFF)+1)*sizeof(char));
				if(!tempfilename){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("memory allocation failure associatedwith %s\n",outfile);
					free(bin_image);
					return AIR_MEMORY_ALLOCATION_ERROR;
				}
				strcpy(tempfilename,outfile);
				AIR_clip_suffix(tempfilename);
				strcat(tempfilename,AIR_CONFIG_IMG_SUFF);
				filename=tempfilename;
			}
			else filename=outfile;
		}

		/* Write the image file */
		{
			/*Open .img file if permitted to do so*/
			if(!ow){
				AIR_Error errcode=AIR_fprobw(filename,ow);
				if(errcode!=0){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("Failed to write output file %s\n",filename);
					free(bin_image);
					if(tempfilename) free(tempfilename);
					return errcode;
				}
			}
			{
				FILE *fp=fopen(filename,"wb");
				if(!fp){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("Failed to write output file %s\n",filename);
					free(bin_image);
					{
						AIR_Error errcode=AIR_fprobw(filename,ow);
						if(tempfilename) free(tempfilename);
						if(errcode==0) return AIR_UNSPECIFIED_FILE_WRITE_ERROR;
						return errcode;
					}
				}

				/*Write out the data*/
				if(fwrite(bin_image,sizeof(unsigned char),bytes_in_file,fp)!=bytes_in_file){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("error writing %s\n",filename);
					free(bin_image);
					if(fclose(fp)!=0){
						printf("%s: %d: ",__FILE__,__LINE__);
						printf("failed to close file %s\n",filename);
						if(tempfilename) free(tempfilename);
						return AIR_CANT_CLOSE_WRITE_ERROR;
					}					
					if(tempfilename) free(tempfilename);
					return AIR_WRITE_IMAGE_FILE_ERROR;
				}
				if(fclose(fp)!=0){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("failed to close file %s\n",filename);
					free(bin_image);
					if(tempfilename) free(tempfilename);
					return AIR_CANT_CLOSE_WRITE_ERROR;
				}
			}
		}
		free(bin_image);

		/* Write the header */
		{
			struct AIR_Key_info info;
			
			info.bits=1;
			info.x_dim=stats->x_dim;
			info.y_dim=stats->y_dim;
			info.z_dim=stats->z_dim;
			info.x_size=stats->x_size;
			info.y_size=stats->y_size;
			info.z_size=stats->z_size;

			/* Avoid unneccesary memory allocation in write_header */
			if(tempfilename){
				AIR_clip_suffix(tempfilename);
				strcat(tempfilename,AIR_CONFIG_HDR_SUFF);
			}
			{
				int flag[8]={0.0,1.0};
				AIR_Error errcode=AIR_write_header(filename,&info,comment,flag);
				if(errcode!=0){
					if(tempfilename) free(tempfilename);
					return errcode;
				}
			}
		}
		if(tempfilename) free(tempfilename);
	}
	return 0;
}
