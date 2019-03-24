/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 5/23/01 */

/* 
 * This program will perform binary operations on two binary files 	
 */

#include "AIR.h"

static void free_function(/*@null@*/ /*@out@*/ /*@only@*/ unsigned char *datain1, /*@null@*/ /*@out@*/ /*@only@*/ unsigned char *datain2)

{
	if(datain1) free(datain1);
	if(datain2) free(datain2);
}

AIR_Error AIR_do_binarymath(const char *program, const char *binfile1, const char *operation, const char *binfile2, const char *output, const AIR_Boolean ow)

{
	/* Precheck of output permissions */
	{
		AIR_Error errcode=AIR_save_probw(output,ow);
		if(errcode!=0){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("Computations not performed due to anticipated output problems with %s\n",output);
			return(errcode);
		}
	}
	{
		unsigned char
			*datain1=NULL,
			*datain2=NULL;
		
		struct AIR_Key_info
			dimensions1,
			dimensions2;
		
		{
			AIR_Error errcode;
			
			datain1=AIR_load1as1(binfile1,&dimensions1,&errcode);
			if (!datain1){
				free_function(datain1,datain2);
				return(errcode);
			}
		}
		if(strcmp(operation,"invert")!=0){
			{
				AIR_Error errcode;
				
				datain2=AIR_load1as1(binfile2,&dimensions2,&errcode);
				if(!datain2){
					free_function(datain1,datain2);
					return(errcode);
				}
			}
			{
				AIR_Error errcode=AIR_same_dim(&dimensions1,&dimensions2);
				
				if(errcode!=0){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("Incompatible file dimensions: %s=%u by %u by %u %s=%u by %u by %u\n",binfile1,dimensions1.x_dim,dimensions1.y_dim,dimensions1.z_dim,binfile2,dimensions2.x_dim,dimensions2.y_dim,dimensions2.z_dim);
					free_function(datain1,datain2);
					return(errcode);
				}
			}
		}
		{
			unsigned long int bytes_in_file;
			{
				unsigned int bytes_in_plane=dimensions1.x_dim*dimensions1.y_dim/8;
				if(bytes_in_plane*8<dimensions1.x_dim*dimensions1.y_dim){
					bytes_in_plane++;
				}
				bytes_in_file=bytes_in_plane*dimensions1.z_dim;
			}

			if(strcmp(operation,"intersect")==0){
				
				unsigned long int i;
				
				for(i=0;i<bytes_in_file;i++){
					datain1[i]=datain1[i] & datain2[i];
				}
			}
			else if(strcmp(operation,"add")==0){
			
				unsigned long int i;
				
				for(i=0;i<bytes_in_file;i++){
					datain1[i]=datain1[i] | datain2[i];
				}
			}
			else if(strcmp(operation,"contrast")==0){
			
				unsigned long int i;
				
				for(i=0;i<bytes_in_file;i++){
					datain1[i]=datain1[i] ^ datain2[i];
				}
			}
			else if(strcmp(operation,"subtract")==0){
			
				unsigned long int i;
				
				for(i=0;i<bytes_in_file;i++){
					datain1[i]=datain1[i] & (~datain2[i]);
				}
			}
			else if(strcmp(operation,"invert")==0){
			
				unsigned long int i;
				
				for(i=0;i<bytes_in_file;i++){
					datain1[i]=~datain1[i];
				}
			}
			else{
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("Cannot understand command %s\n",operation);
				free_function(datain1,datain2);
				return(AIR_USER_INTERFACE_ERROR);
			}
		}
		{
			AIR_Error errcode=AIR_save1as1(datain1,&dimensions1,output,ow,program);
			
			if(errcode!=0){
				free_function(datain1,datain2);
				return(errcode);
			}
		}
		free_function(datain1,datain2);
	}
	return 0;
}
