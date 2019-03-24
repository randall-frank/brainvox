/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 12/4/01 */

/*
 * double **read_airw()
 *
 * reads an AIR4 reslice parameter file
 *
 * failing that, it will try to read it as a 168 parameter nonlinear air struct
 *
 * failing that, it will try to read it as a 15 parameter linear air struct 
 * (returning unsuccessful if there are perspective transformations)
 *
 * failing that, it will try to read it as an old-style 12 parameter linear air struct
 *
 * returns:
 *	**e if successful
 *	NULL if unsuccessful
 * 	*error will be assigned 0 if successful, error code if unsuccessful			
 */


#include "AIR.h"

#define	MAX_ORDER 12		/* Code can support any appropriate value */
#define MAX_COEFFP 455		/* Code can support any appropriate value */
#define	MAX_COORD 3		/* Must modify code if this is changed */
#define MIN_COORD 2		/* Must modify code if this is changed */

double **AIR_read_airw(const char *filename, struct AIR_Warp *air1, AIR_Error *error)

{
	FILE *fp=fopen(filename,"rb");
	if(!fp){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("unable to open file %s for input\n",filename);
		*error=AIR_fprobr(filename);
		if(*error==0) *error=AIR_UNSPECIFIED_FILE_READ_ERROR;
		return NULL;
	}
	
	*error=0;

	/* Try to read it as a modern warp file */
	{
		AIR_Boolean success=TRUE;
		
#if(AIR_CONFIG_AUTO_BYTESWAP!=0)
		AIR_Boolean must_swap=FALSE;
#endif
	
		/* Read in warp file struct */
		if(fread(air1,sizeof(struct AIR_Warp),1,fp)!=1){
			success=FALSE;
		}
		if(success){
		
#if(AIR_CONFIG_AUTO_BYTESWAP!=0)

			if(air1->s.bits>sqrt(UINT_MAX)){
				must_swap=TRUE;
				AIR_swapbytes(&(air1->order),sizeof(air1->order)/2,1);
				AIR_swapbytes(&(air1->coeffp),sizeof(air1->coeffp)/2,1);
				AIR_swapbytes(&(air1->coord),sizeof(air1->coord)/2,1);
				AIR_swapbytes(&(air1->s_hash),sizeof(air1->s_hash)/2,1);
				AIR_swapbytes(&(air1->r_hash),sizeof(air1->r_hash)/2,1);
				AIR_swapbytes(&(air1->s_volume),sizeof(air1->s_volume)/2,1);
				AIR_swapbytes(&(air1->r_volume),sizeof(air1->r_volume)/2,1);
				
				AIR_swapbytes(&(air1->s.bits),sizeof(air1->s.bits)/2,1);
				AIR_swapbytes(&(air1->s.x_dim),sizeof(air1->s.x_dim)/2,1);
				AIR_swapbytes(&(air1->s.y_dim),sizeof(air1->s.y_dim)/2,1);
				AIR_swapbytes(&(air1->s.z_dim),sizeof(air1->s.z_dim)/2,1);
				AIR_swapbytes(&(air1->s.x_size),sizeof(air1->s.x_size)/2,1);
				AIR_swapbytes(&(air1->s.y_size),sizeof(air1->s.y_size)/2,1);
				AIR_swapbytes(&(air1->s.z_size),sizeof(air1->s.z_size)/2,1);
				
				AIR_swapbytes(&(air1->r.bits),sizeof(air1->r.bits)/2,1);
				AIR_swapbytes(&(air1->r.x_dim),sizeof(air1->r.x_dim)/2,1);
				AIR_swapbytes(&(air1->r.y_dim),sizeof(air1->r.y_dim)/2,1);
				AIR_swapbytes(&(air1->r.z_dim),sizeof(air1->r.z_dim)/2,1);
				AIR_swapbytes(&(air1->r.x_size),sizeof(air1->r.x_size)/2,1);
				AIR_swapbytes(&(air1->r.y_size),sizeof(air1->r.y_size)/2,1);
				AIR_swapbytes(&(air1->r.z_size),sizeof(air1->r.z_size)/2,1);
			}
			
#endif
			if(air1->order>MAX_ORDER){
				success=FALSE;
			}
			if(air1->coeffp==0 || air1->coeffp>MAX_COEFFP){
				success=FALSE;
			}
			if(air1->coord<MIN_COORD || air1->coord>MAX_COORD){
				success=FALSE;
			}
		}
		if(success){
		
			unsigned int coords=air1->coord;
			unsigned int coeffs=air1->coeffp;
			double **e=AIR_matrix2(coeffs,coords);
			
			if(!e){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("unable to allocate memory to load .warp file %s\n",filename);
				(void)fclose(fp);
				*error=AIR_MEMORY_ALLOCATION_ERROR;
				return NULL;
			}
			{
				unsigned int j;
				
				for(j=0;j<coords;j++){
				
					if(fread(e[j],sizeof(double),coeffs,fp)!=coeffs){
						success=FALSE;
						AIR_free_2(e);
					}
				}
			}
			if(success){
			
#if(AIR_CONFIG_AUTO_BYTESWAP!=0)
				if(must_swap){
					AIR_swapbytes(e[0],sizeof(e[0][0])/2,(size_t)coords*(size_t)coeffs);
				}
#endif
				if(fgetc(fp)!=EOF){
					success=FALSE;
					AIR_free_2(e);
					/* Leave fp open and try to read as a 168 parameter nonlinear air file */
				}
				else{
					(void)fclose(fp);
					return e;
				}
			}
		}
		/* If not success, then fp is left open and attempt made to read as 168 parameter  file */
	}


	/* Reading failed, try reading it as a 168 paramter nonlinear air file */
	{
		struct AIR_Air168	air4;

		rewind(fp);
		if(fread(&air4,sizeof(struct AIR_Air168),1,fp)==1){

			/* Verify success reading as 168 parameter nonlinear air file */
			if(fgetc(fp)!=EOF){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("file read error for .warp file %s--incorrect format and/or size\n",filename);
				(void)fclose(fp);
				*error=AIR_READ_WARP_FILE_ERROR;
				return NULL;
			}
			if(fclose(fp)!=0){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("error closing file %s\n",filename);
				*error=AIR_CANT_CLOSE_READ_ERROR;
				return NULL;
			}
			
#if(AIR_CONFIG_AUTO_BYTESWAP!=0)
			if(air4.s.bits>sqrt(UINT_MAX)){
				
				AIR_swapbytes(&(air4.e[0][0]),sizeof(air4.e[0][0])/2,56*4);
				AIR_swapbytes(&(air4.s_hash),sizeof(air4.s_hash)/2,1);
				AIR_swapbytes(&(air4.r_hash),sizeof(air4.r_hash)/2,1);
				AIR_swapbytes(&(air4.s_volume),sizeof(air4.s_volume)/2,1);
				AIR_swapbytes(&(air4.r_volume),sizeof(air4.r_volume)/2,1);
				
				AIR_swapbytes(&(air4.s.bits),sizeof(air4.s.bits)/2,1);
				AIR_swapbytes(&(air4.s.x_dim),sizeof(air4.s.x_dim)/2,1);
				AIR_swapbytes(&(air4.s.y_dim),sizeof(air4.s.y_dim)/2,1);
				AIR_swapbytes(&(air4.s.z_dim),sizeof(air4.s.z_dim)/2,1);
				AIR_swapbytes(&(air4.s.x_size),sizeof(air4.s.x_size)/2,1);
				AIR_swapbytes(&(air4.s.y_size),sizeof(air4.s.y_size)/2,1);
				AIR_swapbytes(&(air4.s.z_size),sizeof(air4.s.z_size)/2,1);
				
				AIR_swapbytes(&(air4.r.bits),sizeof(air4.r.bits)/2,1);
				AIR_swapbytes(&(air4.r.x_dim),sizeof(air4.r.x_dim)/2,1);
				AIR_swapbytes(&(air4.r.y_dim),sizeof(air4.r.y_dim)/2,1);
				AIR_swapbytes(&(air4.r.z_dim),sizeof(air4.r.z_dim)/2,1);
				AIR_swapbytes(&(air4.r.x_size),sizeof(air4.r.x_size)/2,1);
				AIR_swapbytes(&(air4.r.y_size),sizeof(air4.r.y_size)/2,1);
				AIR_swapbytes(&(air4.r.z_size),sizeof(air4.r.z_size)/2,1);
				
			}
#endif

			/* Figure out what order representation is actually needed */
			{
				unsigned int coeff_max=0;
				unsigned int coord_max=0;
				unsigned int j;

				for(j=0;j<56;j++){

					unsigned int i;

					for(i=0;i<3;i++){
						if(air4.e[j][i]!=0.0){
							if(j>coeff_max) coeff_max=j;
							if(i>coord_max) coord_max=i;
						}
					}
				}
				air1->coeffp=coeff_max+1;
				air1->coord=coord_max+1;
			}

			if(air1->coord==2){
				if(air1->coeffp>15){
					air1->order=5;
					air1->coeffp=21;
				}
				else if(air1->coeffp>10){
					air1->order=4;
					air1->coeffp=15;
				}
				else if(air1->coeffp>6){
					air1->order=3;
					air1->coeffp=10;
				}
				else if(air1->coeffp>3){
					air1->order=2;
					air1->coeffp=6;
				}
				else if(air1->coeffp>1){
					air1->order=1;
					air1->coeffp=3;
				}
				else{
					air1->order=0;
					air1->coeffp=1;
				}
			}
			else if(air1->coord==3){
				if(air1->coeffp>35){
					air1->order=5;
					air1->coeffp=56;
				}
				else if(air1->coeffp>20){
					air1->order=4;
					air1->coeffp=35;
				}
				else if(air1->coeffp>10){
					air1->order=3;
					air1->coeffp=20;
				}
				else if(air1->coeffp>4){
					air1->order=2;
					air1->coeffp=10;
				}
				else if(air1->coeffp>1){
					air1->order=1;
					air1->coeffp=4;
				}
				else{
					air1->order=0;
					air1->coeffp=1;
				}
			}
			else{
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("error reading old 168 parameter .warp file %s\n",filename);
				*error=AIR_READ_OLDWARP_FILE_ERROR;
				return NULL;
			}

			strncpy(air1->s_file,air4.s_file,127);
			strncpy(air1->r_file,air4.r_file,127);
			strncpy(air1->comment,air4.comment,127);
			strncpy(air1->reserved,air4.reserved,115);
			air1->s.bits=air4.s.bits;
			air1->s.x_dim=air4.s.x_dim;
			air1->s.y_dim=air4.s.y_dim;
			air1->s.z_dim=air4.s.z_dim;
			air1->s.x_size=air4.s.x_size;
			air1->s.y_size=air4.s.y_size;
			air1->s.z_size=air4.s.z_size;
			air1->r.bits=air4.r.bits;
			air1->r.x_dim=air4.r.x_dim;
			air1->r.y_dim=air4.r.y_dim;
			air1->r.z_dim=air4.r.z_dim;
			air1->r.x_size=air4.r.x_size;
			air1->r.y_size=air4.r.y_size;
			air1->r.z_size=air4.r.z_size;
			air1->s_hash=air4.s_hash;
			air1->r_hash=air4.r_hash;
			air1->s_volume=air4.s_volume;
			air1->s_volume=air4.s_volume;

			/* Create and complete matrix e */
			{
				double **e;

				e=AIR_matrix2(air1->coeffp,air1->coord);
				if(!e){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("unable to allocate memory to load old 168 parameter .warp file %s\n",filename);
					*error=AIR_MEMORY_ALLOCATION_ERROR;
					return NULL;
				}

				/* Insert values */
				{
					unsigned int j;

					for(j=0;j<air1->coeffp;j++){

						unsigned int i;

						for(i=0;i<air1->coord;i++){
							e[i][j]=air4.e[j][i];
						}
					}
				}
				return e;
			}
		}
	}

	/* Reading failed, try reading as a linear air file */
	{
		struct AIR_Air16	air3;

		rewind(fp);
		if(fread(&air3,sizeof(struct AIR_Air16),1,fp)==1){

			/* Verify success reading as a linear air file */
			if(fgetc(fp)!=EOF){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("file read error for .air file %s--EOF not at expected location\n",filename);
				(void)fclose(fp);
				*error=AIR_READ_WARP_FILE_ERROR;
				return NULL;
			}
			if(fclose(fp)!=0){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("error closing file %s\n",filename);
				*error=AIR_CANT_CLOSE_READ_ERROR;
				return NULL;
			}
			
#if(AIR_CONFIG_AUTO_BYTESWAP!=0)

			if(air3.s.bits>sqrt(UINT_MAX)){
				
				AIR_swapbytes(&(air3.e[0][0]),sizeof(air3.e[0][0])/2,4*4);
				AIR_swapbytes(&(air3.s_hash),sizeof(air3.s_hash)/2,1);
				AIR_swapbytes(&(air3.r_hash),sizeof(air3.r_hash)/2,1);
				AIR_swapbytes(&(air3.s_volume),sizeof(air3.s_volume)/2,1);
				AIR_swapbytes(&(air3.r_volume),sizeof(air3.r_volume)/2,1);
				
				AIR_swapbytes(&(air3.s.bits),sizeof(air3.s.bits)/2,1);
				AIR_swapbytes(&(air3.s.x_dim),sizeof(air3.s.x_dim)/2,1);
				AIR_swapbytes(&(air3.s.y_dim),sizeof(air3.s.y_dim)/2,1);
				AIR_swapbytes(&(air3.s.z_dim),sizeof(air3.s.z_dim)/2,1);
				AIR_swapbytes(&(air3.s.x_size),sizeof(air3.s.x_size)/2,1);
				AIR_swapbytes(&(air3.s.y_size),sizeof(air3.s.y_size)/2,1);
				AIR_swapbytes(&(air3.s.z_size),sizeof(air3.s.z_size)/2,1);
				
				AIR_swapbytes(&(air3.r.bits),sizeof(air3.r.bits)/2,1);
				AIR_swapbytes(&(air3.r.x_dim),sizeof(air3.r.x_dim)/2,1);
				AIR_swapbytes(&(air3.r.y_dim),sizeof(air3.r.y_dim)/2,1);
				AIR_swapbytes(&(air3.r.z_dim),sizeof(air3.r.z_dim)/2,1);
				AIR_swapbytes(&(air3.r.x_size),sizeof(air3.r.x_size)/2,1);
				AIR_swapbytes(&(air3.r.y_size),sizeof(air3.r.y_size)/2,1);
				AIR_swapbytes(&(air3.r.z_size),sizeof(air3.r.z_size)/2,1);
				
			}
#endif

			if(air3.e[0][3]!=0.0 || air3.e[1][3]!=0.0 || air3.e[2][3]!=0.0 || air3.e[3][3]!=1.0){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf(".air file %s contains perspective transformations which are not supported\n",filename);
				*error=AIR_READ_PERSPWARP_FILE_ERROR;
				return NULL;
			}

			air1->order=1;
			air1->coeffp=4;
			air1->coord=3;
			strcpy(air1->s_file,air3.s_file);
			strcpy(air1->r_file,air3.r_file);
			strcpy(air1->comment,air3.comment);
			strncpy(air1->reserved,air3.reserved,115);
			air1->s.bits=air3.s.bits;
			air1->s.x_dim=air3.s.x_dim;
			air1->s.y_dim=air3.s.y_dim;
			air1->s.z_dim=air3.s.z_dim;
			air1->s.x_size=air3.s.x_size;
			air1->s.y_size=air3.s.y_size;
			air1->s.z_size=air3.s.z_size;
			air1->r.bits=air3.r.bits;
			air1->r.x_dim=air3.r.x_dim;
			air1->r.y_dim=air3.r.y_dim;
			air1->r.z_dim=air3.r.z_dim;
			air1->r.x_size=air3.r.x_size;
			air1->r.y_size=air3.r.y_size;
			air1->r.z_size=air3.r.z_size;
			air1->s_hash=air3.s_hash;
			air1->r_hash=air3.r_hash;
			air1->s_volume=air3.s_volume;
			air1->r_volume=air3.r_volume;

			/* Create and complete matrix e */
			{
				double **e;

				e=AIR_matrix2(air1->coeffp,air1->coord);
				if(!e){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("unable to allocate memory to load .air file %s\n",filename);
					*error=AIR_MEMORY_ALLOCATION_ERROR;
					return NULL;
				}

				/* Assign properly adjusted elements */
				{
					double pixel_size_s;

					pixel_size_s=air1->s.x_size;
					if(air1->s.y_size<pixel_size_s) pixel_size_s=air1->s.y_size;
					if(air1->s.z_size<pixel_size_s) pixel_size_s=air1->s.z_size;

					e[0][0]=air3.e[3][0];
					e[1][0]=air3.e[3][1];
					e[2][0]=air3.e[3][2];
					e[0][1]=air3.e[0][0]*(air1->s.x_size/pixel_size_s);
					e[1][1]=air3.e[0][1]*(air1->s.x_size/pixel_size_s);
					e[2][1]=air3.e[0][2]*(air1->s.x_size/pixel_size_s);
					e[0][2]=air3.e[1][0]*(air1->s.y_size/pixel_size_s);
					e[1][2]=air3.e[1][1]*(air1->s.y_size/pixel_size_s);
					e[2][2]=air3.e[1][2]*(air1->s.y_size/pixel_size_s);
					e[0][3]=air3.e[2][0]*(air1->s.z_size/pixel_size_s);
					e[1][3]=air3.e[2][1]*(air1->s.z_size/pixel_size_s);
					e[2][3]=air3.e[2][2]*(air1->s.z_size/pixel_size_s);
				}
				return e;
			}
		}
	}

	/* Reading failed, try reading as an old linear air file */
	{

		struct AIR_Oldair	air2;

		rewind(fp);
		if(fread(&air2,sizeof(struct AIR_Oldair),1,fp)!=1){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf(".warp or .air file %s format not recognized\n",filename);
			(void)fclose(fp);
			*error=AIR_READ_WARP_FILE_ERROR;
			return NULL;
		}

		/* Verify success reading as an old 12 parameter air file */
		if(fgetc(fp)!=EOF){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("file read error for old 12 parameter .air file %s--EOF not at expected location\n",filename);
			(void)fclose(fp);
			*error=AIR_READ_WARP_FILE_ERROR;
			return NULL;
		}
		if(fclose(fp)!=0){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("failed to close file %s\n",filename);
			*error=AIR_CANT_CLOSE_READ_ERROR;
			return NULL;
		}
		
#if(AIR_CONFIG_AUTO_BYTESWAP!=0)
		if(air2.s.bits>sqrt(UINT_MAX)){
			
			AIR_swapbytes(&(air2.e[0][0]),sizeof(air2.e[0][0])/2,4*3);
			AIR_swapbytes(&(air2.s_hash),sizeof(air2.s_hash)/2,1);
			AIR_swapbytes(&(air2.r_hash),sizeof(air2.r_hash)/2,1);
			AIR_swapbytes(&(air2.s_volume),sizeof(air2.s_volume)/2,1);
			AIR_swapbytes(&(air2.r_volume),sizeof(air2.r_volume)/2,1);
			
			AIR_swapbytes(&(air2.s.bits),sizeof(air2.s.bits)/2,1);
			AIR_swapbytes(&(air2.s.x_dim),sizeof(air2.s.x_dim)/2,1);
			AIR_swapbytes(&(air2.s.y_dim),sizeof(air2.s.y_dim)/2,1);
			AIR_swapbytes(&(air2.s.z_dim),sizeof(air2.s.z_dim)/2,1);
			AIR_swapbytes(&(air2.s.x_size),sizeof(air2.s.x_size)/2,1);
			AIR_swapbytes(&(air2.s.y_size),sizeof(air2.s.y_size)/2,1);
			AIR_swapbytes(&(air2.s.z_size),sizeof(air2.s.z_size)/2,1);
			
			AIR_swapbytes(&(air2.r.bits),sizeof(air2.r.bits)/2,1);
			AIR_swapbytes(&(air2.r.x_dim),sizeof(air2.r.x_dim)/2,1);
			AIR_swapbytes(&(air2.r.y_dim),sizeof(air2.r.y_dim)/2,1);
			AIR_swapbytes(&(air2.r.z_dim),sizeof(air2.r.z_dim)/2,1);
			AIR_swapbytes(&(air2.r.x_size),sizeof(air2.r.x_size)/2,1);
			AIR_swapbytes(&(air2.r.y_size),sizeof(air2.r.y_size)/2,1);
			AIR_swapbytes(&(air2.r.z_size),sizeof(air2.r.z_size)/2,1);
			
		}
#endif

		air1->order=1;
		air1->coeffp=4;
		air1->coord=3;
		strncpy(air1->s_file,air2.s_file,127);
		strncpy(air1->r_file,air2.r_file,127);
		strncpy(air1->comment,air2.comment,127);
		strncpy(air1->reserved,air2.reserved,115);
		air1->s.bits=air2.s.bits;
		air1->s.x_dim=air2.s.x_dim;
		air1->s.y_dim=air2.s.y_dim;
		air1->s.z_dim=air2.s.z_dim;
		air1->s.x_size=air2.s.x_size;
		air1->s.y_size=air2.s.y_size;
		air1->s.z_size=air2.s.z_size;
		air1->r.bits=air2.r.bits;
		air1->r.x_dim=air2.r.x_dim;
		air1->r.y_dim=air2.r.y_dim;
		air1->r.z_dim=air2.r.z_dim;
		air1->r.x_size=air2.r.x_size;
		air1->r.y_size=air2.r.y_size;
		air1->r.z_size=air2.r.z_size;
		air1->s_hash=air2.s_hash;
		air1->r_hash=air2.r_hash;
		air1->s_volume=air2.s_volume;
		air1->r_volume=air2.r_volume;

		/* Create and complete matrix e */
		{
			double **e;

			e=AIR_matrix2(air1->coeffp,air1->coord);
			if(!e){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("unable to allocate memory to load old 12 parameter .air file %s\n",filename);
				*error=AIR_MEMORY_ALLOCATION_ERROR;
				return NULL;
			}

			/* Assign properly adjusted elements */
			{
				double pixel_size_s;

				pixel_size_s=air1->s.x_size;
				if(air1->s.y_size<pixel_size_s) pixel_size_s=air1->s.y_size;
				if(air1->s.z_size<pixel_size_s) pixel_size_s=air1->s.z_size;

				e[0][0]=air2.e[0][0];
				e[1][0]=air2.e[0][1];
				e[2][0]=air2.e[0][2];
				e[0][1]=air2.e[1][0]*(air1->s.x_size/pixel_size_s);
				e[1][1]=air2.e[1][1]*(air1->s.x_size/pixel_size_s);
				e[2][1]=air2.e[1][2]*(air1->s.x_size/pixel_size_s);
				e[0][2]=air2.e[2][0]*(air1->s.y_size/pixel_size_s);
				e[1][2]=air2.e[2][1]*(air1->s.y_size/pixel_size_s);
				e[2][2]=air2.e[2][2]*(air1->s.y_size/pixel_size_s);
				e[0][3]=air2.e[3][0]*(air1->s.z_size/pixel_size_s);
				e[1][3]=air2.e[3][1]*(air1->s.z_size/pixel_size_s);
				e[2][3]=air2.e[3][2]*(air1->s.z_size/pixel_size_s);
			}
			return e;
		}
	}
}
