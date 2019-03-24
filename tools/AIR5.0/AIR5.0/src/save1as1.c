/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 12/21/01 */

/*
 * This routine will write a binary volume to disk as a
 * binary volume.
 *
 * Note that by convention, each new plane starts on a byte boundary
 * Consequently, if x_dim*y_dim is not an exact multiple of 8,
 * the binary data is padded at the end of each plane of data
 *
 * Values greater than or equal to min and less than or equal
 *  to max are converted to 1's, all other values are converted
 *  to 0's.
 *
 * Note deliberate use of unsigned char
 *
 * Returns:
 *	0 if save was successful
 * 	error code if save was unsuccessful
 */

#include "AIR.h"

AIR_Error AIR_save1as1(const unsigned char *bin_image, const struct AIR_Key_info *stats, const char *outfile, const AIR_Boolean ow, const char *comment)

{
	const char *filename;
	char *tempfilename=NULL;

	/*Verify bits/pixel*/
	if(stats->bits!=(unsigned int)sizeof(unsigned char)){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("%s not saved because struct indicates %u bits/pixel instead of expected %u\n",outfile,stats->bits,(unsigned int)sizeof(unsigned char));
		return AIR_WRITE_NONASBINARY_CODING_ERROR;
	}

	/*Verify acceptable pixel dimensions*/
	if(stats->x_dim<1 || stats->y_dim<1 || stats->z_dim<1){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("%s not saved because struct indicates impossible file dimension %u x %u x %u\n",outfile,stats->x_dim,stats->y_dim,stats->z_dim);
		return AIR_WRITE_BAD_DIM_ERROR;
	}

	/* See if outfile already ends with the proper suffix */
	{
		const char *dot=strrchr(outfile,'.');
		if(!dot) dot=outfile;
		if(strcmp(dot,AIR_CONFIG_IMG_SUFF)!=0){
			/* Allocate memory and copy outfile */
			tempfilename=malloc((strlen(outfile)+strlen(AIR_CONFIG_IMG_SUFF)+1)*sizeof(char));
			if(!tempfilename){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("memory allocation failure associated with %s\n",outfile);
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
				if(tempfilename) free(tempfilename);
				return errcode;
			}
		}
		{
			FILE *fp=fopen(filename,"wb");
			if(!fp){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("Failed to write output file %s\n",filename);
				{
					AIR_Error errcode=AIR_fprobw(filename,ow);
					if(tempfilename) free(tempfilename);
					if(errcode==0) return AIR_UNSPECIFIED_FILE_WRITE_ERROR;
					return errcode;
				}
			}

			/*Write out the data*/
			{
				size_t bytes_in_file;

				/*Compute number of bytes in binary file plane*/
				{
					unsigned int bytes_in_plane;

					bytes_in_plane=stats->x_dim*stats->y_dim/8;
					if(bytes_in_plane*8<stats->x_dim*stats->y_dim){
						bytes_in_plane++;
					}
					bytes_in_file=(size_t)bytes_in_plane*stats->z_dim;
				}

				if(fwrite(bin_image,sizeof(unsigned char),bytes_in_file,fp)!=bytes_in_file){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("error writing %s\n",filename);
					if(fclose(fp)!=0){
						printf("%s: %d: ",__FILE__,__LINE__);
						printf("failed to close file %s\n",filename);
						if(tempfilename) free(tempfilename);
						return AIR_CANT_CLOSE_WRITE_ERROR;
					}					
					if(tempfilename) free(tempfilename);
					return AIR_WRITE_IMAGE_FILE_ERROR;
				}
			}

			if(fclose(fp)!=0){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("failed to close file %s\n",filename);
				if(tempfilename) free(tempfilename);
				return AIR_CANT_CLOSE_WRITE_ERROR;
			}
		}
	}

        /*Write header*/
	{
		int flag[8];

		flag[0]=0;
		flag[1]=1;

		/* Avoid unnecessary memory allocation in write_header */
		if(tempfilename){
			AIR_clip_suffix(tempfilename);
			strcat(tempfilename,AIR_CONFIG_HDR_SUFF);
		}
		{
			AIR_Error errcode=AIR_write_header(filename,stats,comment,flag);
			if(errcode!=0){
				if(tempfilename) free(tempfilename);
				return errcode;
			}
		}
	}

	if(tempfilename) free(tempfilename);
	return 0;
}
