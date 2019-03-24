/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 5/18/01 */

/*				
 * Routine will write out ANALYZE compatible header based on the struct
 * defined in HEADER.h
 *							
 * In addition, a comment that will be displayed at load time
 *  can be specified (only the first 79 characters of the comment are stored)
 *
 * Note that overwrite permission is assumed
 *							
 * Returns:						
 *	0 if successful					
 *	errorcode if unsuccessful
 *
*/

#include "AIR.h"
#include "HEADER.h"

AIR_Error AIR_write_header(const char *outfile, const struct AIR_Key_info *stats, const char *comment, const int *flag)

{
	const char *filename;
	char *tempfilename=NULL;

	/* See if outfile already ends with the proper suffix */
	{
		const char *dot=strrchr(outfile,'.');
		if(!dot) dot=outfile;
		if(strcmp(dot,AIR_CONFIG_HDR_SUFF)!=0){		
			/* Allocate memory and copy outfile */
			tempfilename=malloc((strlen(outfile)+strlen(AIR_CONFIG_HDR_SUFF)+1)*sizeof(char));
			if(!tempfilename){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("memory allocation error associated with %s\n",outfile);
				return AIR_MEMORY_ALLOCATION_ERROR;
			}
			strcpy(tempfilename,outfile);
			AIR_clip_suffix(tempfilename);
			strcat(tempfilename,AIR_CONFIG_HDR_SUFF);
			filename=tempfilename;
		}
		else filename=outfile;
	}

	/* Prepare the hdr struct */
	{
		struct AIR_hdr hdr;

		/*Initialize hdr to zeros*/
		memset(&hdr,0,sizeof(hdr));

		/*Copy required data into header struct*/
		hdr.bits=(short int)stats->bits;
		hdr.dims=4;
		hdr.x_dim=stats->x_dim;
		hdr.y_dim=stats->y_dim;
		hdr.z_dim=stats->z_dim;
		hdr.t_dim=1;

		if (stats->bits==8) hdr.datatype=2;		/*unsigned characters*/
		else if (stats->bits==1) hdr.datatype=1;	/*binary*/
		else if (stats->bits==16) hdr.datatype=4;	/*short int*/
		else if (stats->bits==32) hdr.datatype=8;	/*int (ignoring other option of floating point)*/
		else hdr.datatype=0;				/*unknown data type*/

		hdr.x_size=stats->x_size;
		hdr.y_size=stats->y_size;
		hdr.z_size=stats->z_size;
		hdr.glmax=flag[1];			/*global max*/
		hdr.glmin=flag[0];			/*global min*/
		hdr.sizeof_hdr=(int)sizeof(struct AIR_hdr);	/*standard=348*/
		hdr.extents=16384;
		hdr.regular='r';

		strncpy(hdr.descrip,comment,79);

		/* Save the struct data to the file */
		{
			FILE *fp=fopen(filename,"wb");
			if(!fp){

				printf("%s: %d: ",__FILE__,__LINE__);
				printf("cannot open file %s for output\n",filename);
				{
					AIR_Error errcode=AIR_fprobw(filename,TRUE);
					if(tempfilename) free(tempfilename);
					if(errcode==0) return AIR_UNSPECIFIED_FILE_WRITE_ERROR;
					return errcode;
				}
			}

			/*Write out header*/
			if(fwrite(&hdr,sizeof(struct AIR_hdr),1,fp)!=1){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("file write error writing header %s\n",filename);
				if(fclose(fp)!=0){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("failed to close header file %s\n",filename);
					if(tempfilename) free(tempfilename);
					return AIR_CANT_CLOSE_WRITE_ERROR;
				}				
				if(tempfilename) free(tempfilename);
				return AIR_WRITE_HEADER_FILE_ERROR;
			}
			if(fclose(fp)!=0){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("failed to close header file %s\n",filename);
				if(tempfilename) free(tempfilename);
				return AIR_CANT_CLOSE_WRITE_ERROR;
			}
		}
	}

	if(tempfilename) free(tempfilename);
	return 0;
}
