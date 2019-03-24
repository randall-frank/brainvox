/* Copyright 2001 Roger P. Woods, M.D. */
/* Modified: 5/22/01 */


/*
 * Gets	file names from a file, mimicing reading a list from the command line
 * This is primarily implemented to circumvent command line length limits on UNIX
 * 	systems.
 *
 * Tabs, spaces, line feeds and carriage returns are treated as white space
 */


#include "AIR.h"

char **AIR_parse_file_list(const char *filename, unsigned int *args, AIR_Error *errcode)

{
	int count=0;	/* fread return an integer */
	char *contents;
	*errcode=0;
	*args=0;
	
	/* Open the file and read the data */
	{
		FILE *fp=fopen(filename,"r");
		if (!fp){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("unable to open file %s\n",filename);
			*errcode=AIR_fprobr(filename);
			if(*errcode==0) *errcode=AIR_UNSPECIFIED_FILE_READ_ERROR;
			return NULL;
		}
	   
		/* Count the number of bytes in the file */
		{
			int initialwhitespace=0;
			
			for(;;){
				char test=fgetc(fp);
				if(feof(fp)!=0) break;
				if(count==0){
					if(isspace(test)) initialwhitespace++;
					else count++;
				}
				else count++;
			}
			
			/* Return error if file contained only white space */
			if(count==0){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("File %s did not contain any file names\n",filename);
				(void)fclose(fp);
				*errcode=AIR_READ_ASCII_FILE_ERROR;
				return NULL;
			}
		
			/* Return to start of file, skipping any initial white space */				
			
			if(fseek(fp,(long)initialwhitespace,SEEK_SET)!=0){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("failed to read ASCII data from file %s\n",filename);
				(void)fclose(fp);
				*errcode=AIR_FSEEK_READ_ERROR;
				return NULL;
			}

			/* Allocate memory */
			contents=malloc(count*sizeof(char));
			if(!contents){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("memory allocation error\n");
				(void)fclose(fp);
				*errcode=AIR_MEMORY_ALLOCATION_ERROR;
				return NULL;
			}
			
			/* Read the data from the file */
			if(fread(contents,sizeof(char),(size_t)count,fp)!=(size_t)count){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("failed to read ASCII data from file %s\n",filename);
				(void)fclose(fp);
				free(contents);
				*errcode=AIR_READ_ASCII_FILE_ERROR;
				return NULL;
			}
			
			/* Close the file */
			if(fclose(fp)!=0){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("failed to close file %s\n",filename);
				free(contents);
				*errcode=AIR_CANT_CLOSE_READ_ERROR;
				return NULL;
			}
		}
	}
	/* Set whitespace characters to nulls */
	{
		char *current=contents;
		for(;current<contents+count;current++){
			if(isspace(*current)){
				*current='\0';
			}
		}
	}

	{
		char **list;
		*args=1;		/* We have an argument in the zero position already */

		
		/* Count the number of arguments in contents */
		{
			char *prior=contents;
			char *current=contents+1;

			for(;current<contents+count;prior++,current++){
				if(*prior=='\0'){
					if(*current!='\0'){
						(*args)++;
					}
				}
			}
		}
	
		/* Allocate memory */
		list=(char **)malloc(*args*sizeof(char *));
		if(!list){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("Memory allocation error\n");
			free(contents);
			*errcode=AIR_MEMORY_ALLOCATION_ERROR;
			*args=0;
			return NULL;
		}

		/* Make list point to the args */
		{
			char **currentlist=list;
			char *prior=contents;
			char *current=contents;
			
			*currentlist++=current++;	/* The zero position in current must be an argument */
			
			for(;current<contents+count;prior++,current++){
				if(*prior=='\0'){
					if(*current!='\0'){
						*currentlist++=current;
					}
				}
			}
		}
		return list;
	}
}
		
