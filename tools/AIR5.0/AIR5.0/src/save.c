/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 12/21/01 */

/*
 * This routine will write a volume onto disk with an appropriate
 *  header
 *
 * Returns:
 *	0 if save was successful
 *	error code if save was unsuccessful
 *
 */

#include "AIR.h"

AIR_Error AIR_save(AIR_Pixels ***pixels, const struct AIR_Key_info *stats, const char *outfile, const AIR_Boolean ow, const char *comment)

{
	const char *filename;
	char *tempfilename=NULL;

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
		/*Open image file if permitted to do so*/
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
		

#if(AIR_CONFIG_OUTBITS==8)
			/* Write 8 bit image data */
			{
				unsigned int k;
				for (k=0;k<stats->z_dim;k++){
					unsigned int j;
					for (j=0;j<stats->y_dim;j++){
						if(fwrite(pixels[k][j],sizeof(AIR_Pixels),stats->x_dim,fp)!=stats->x_dim){
							printf("%s: %d: ",__FILE__,__LINE__);
							printf("file write error writing %s\n",filename);
							if(fclose(fp)){
								printf("%s: %d: ",__FILE__,__LINE__);
								printf("failed to close file %s\n",filename);
								if(tempfilename) free(tempfilename);
								return AIR_CANT_CLOSE_WRITE_ERROR;
							}							
							if(tempfilename) free(tempfilename);
							return AIR_WRITE_IMAGE_FILE_ERROR;
						}
					}
				}
			}
#elif(AIR_CONFIG_OUTBITS==16)
			/* Write 16 bit image data */
			{
				unsigned int k;
				for (k=0;k<stats->z_dim;k++){
					unsigned int j;
					for(j=0;j<stats->y_dim;j++){
						unsigned int i;
						for(i=0;i<stats->x_dim;i++){

							unsigned short int temp16;

							temp16=pixels[k][j][i];
#if(AIR_CONFIG_REP16==2)
							temp16/=2;
#elif(AIR_CONFIG_REP16==3)
							if(temp16<32768) temp16+=32768;
							else temp16-=32768;
#endif
							if(fwrite(&temp16,sizeof(AIR_Pixels),1,fp)!=1){
								printf("%s: %d: ",__FILE__,__LINE__);
								printf("fill write error writing %s\n",filename);
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
					}
				}
			}
#endif						
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

#if(AIR_CONFIG_OUTBITS==8)
		flag[0]=0;
		flag[1]=AIR_CONFIG_MAX_POSS_VALUE;
#elif(AIR_CONFIG_OUTBITS==16)
#if(AIR_CONFIG_REP16==1)
		flag[0]=0;
		flag[1]=65535;
#elif(AIR_CONFIG_REP16==2)
		flag[0]=0;
		flag[1]=32767;
#elif(AIR_CONFIG_REP16==3)
		flag[0]=-32767;
		flag[1]=32767;
#endif
#endif
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


