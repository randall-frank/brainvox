/* Copyright 1995-2002 Roger P. Woods, M.D. */
/* Modified: 1/16/02 */

/*
 * reads an AIR reslice parameter file into an air struct
 *
 * program will also attempt to convert old 12 parameter
 * AIR files into new 15 parameter files
 *
 * returns:
 *	0 if successful
 *	error code if unsuccessful
 */

#include "AIR.h"

AIR_Error AIR_read_air16(const char *filename, struct AIR_Air16 *air1)

{
	FILE *fp=fopen(filename,"rb");
	if(!fp){

		printf("%s: %d: ",__FILE__,__LINE__);
		printf("unable to open file %s for input\n",filename);
		{
			AIR_Error error=AIR_fprobr(filename);
			if(error==0) return AIR_UNSPECIFIED_FILE_READ_ERROR;
			return error;
		}
	}
	
	/*Read in AIR file*/

	if(fread(air1,1,sizeof(struct AIR_Air16),fp)!=sizeof(struct AIR_Air16)){

		struct AIR_Oldair	air2;

		/*Try reading it as an old 12 parameter air file*/
		rewind(fp);
		if(fread(&air2,1,sizeof(struct AIR_Oldair),fp)!=sizeof(struct AIR_Oldair)){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("file read error for file %s\n",filename);
			(void)fclose(fp);
			return AIR_READ_AIR_FILE_ERROR;
		}
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
		air1->e[0][0]=air2.e[1][0];
		air1->e[0][1]=air2.e[1][1];
		air1->e[0][2]=air2.e[1][2];
		air1->e[0][3]=0;
		air1->e[1][0]=air2.e[2][0];
		air1->e[1][1]=air2.e[2][1];
		air1->e[1][2]=air2.e[2][2];
		air1->e[1][3]=0;
		air1->e[2][0]=air2.e[3][0];
		air1->e[2][1]=air2.e[3][1];
		air1->e[2][2]=air2.e[3][2];
		air1->e[2][3]=0;
		air1->e[3][0]=air2.e[0][0];
		air1->e[3][1]=air2.e[0][1];
		air1->e[3][2]=air2.e[0][2];
		air1->e[3][3]=1;
		
#if(AIR_CONFIG_AUTO_BYTESWAP!=0)
		
		if(air1->s.bits>sqrt(UINT_MAX)){
			/* Pre-swap these values so that they'll be swapped back to their starting values */
			AIR_swapbytes(&(air1->e[0][3]),sizeof(air1->e[0][3])/2,1);
			AIR_swapbytes(&(air1->e[1][3]),sizeof(air1->e[1][3])/2,1);
			AIR_swapbytes(&(air1->e[2][3]),sizeof(air1->e[2][3])/2,1);
			AIR_swapbytes(&(air1->e[3][3]),sizeof(air1->e[3][3])/2,1);
		}
#endif
	}
	
#if(AIR_CONFIG_AUTO_BYTESWAP!=0)

	if(air1->s.bits>sqrt(UINT_MAX)){
	
		/* Byte swap */
		
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
		
		AIR_swapbytes(&(air1->s_hash),sizeof(air1->s_hash)/2,1);
		AIR_swapbytes(&(air1->r_hash),sizeof(air1->r_hash)/2,1);
		
		AIR_swapbytes(&(air1->s_volume),sizeof(air1->s_volume)/2,1);
		AIR_swapbytes(&(air1->r_volume),sizeof(air1->r_volume)/2,1);
				
		AIR_swapbytes(&(air1->e[0][0]),sizeof(air1->e[0][0])/2,1);
		AIR_swapbytes(&(air1->e[0][1]),sizeof(air1->e[0][1])/2,1);
		AIR_swapbytes(&(air1->e[0][2]),sizeof(air1->e[0][2])/2,1);
		AIR_swapbytes(&(air1->e[0][3]),sizeof(air1->e[0][3])/2,1);
		AIR_swapbytes(&(air1->e[1][0]),sizeof(air1->e[1][0])/2,1);
		AIR_swapbytes(&(air1->e[1][1]),sizeof(air1->e[1][1])/2,1);
		AIR_swapbytes(&(air1->e[1][2]),sizeof(air1->e[1][2])/2,1);
		AIR_swapbytes(&(air1->e[1][3]),sizeof(air1->e[1][3])/2,1);
		AIR_swapbytes(&(air1->e[2][0]),sizeof(air1->e[2][0])/2,1);
		AIR_swapbytes(&(air1->e[2][1]),sizeof(air1->e[2][1])/2,1);
		AIR_swapbytes(&(air1->e[2][2]),sizeof(air1->e[2][2])/2,1);
		AIR_swapbytes(&(air1->e[2][3]),sizeof(air1->e[2][3])/2,1);
		AIR_swapbytes(&(air1->e[3][0]),sizeof(air1->e[3][0])/2,1);
		AIR_swapbytes(&(air1->e[3][1]),sizeof(air1->e[3][1])/2,1);
		AIR_swapbytes(&(air1->e[3][2]),sizeof(air1->e[3][2])/2,1);
		AIR_swapbytes(&(air1->e[3][3]),sizeof(air1->e[3][3])/2,1);
	}
#endif

	if(fgetc(fp)!=EOF){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("file read error for file %s--EOF not at expected location\n",filename);
		(void)fclose(fp);
		return AIR_READ_AIR_FILE_ERROR;
	}
	if(fclose(fp)!=0){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("failed to close file %s\n",filename);
		return AIR_CANT_CLOSE_READ_ERROR;
	}

	return 0;
}
