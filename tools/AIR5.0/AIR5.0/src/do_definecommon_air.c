/* Copyright 1997-2001 Roger P. Woods, M.D. */
/* Modified: 5/23/01 */

/*
 * This defines a new average standard space that is the
 *  average of the reslice spaces of a set of .air files that
 *  share a common standard space.
 *
 * It does error checking for compatibility			
 */

#include "AIR.h"

static void free_function(/*@null@*/ /*@out@*/ /*@only@*/ struct AIR_Air16 *airn, /*@null@*/ /*@out@*/ /*@only@*/ double ***en, /*@null@*/ /*@out@*/ /*@only@*/ double **enrows, /*@null@*/ /*@out@*/ /*@only@*/ char *filename, /*@null@*/ /*@out@*/ /*@only@*/ char *filename2)

{
	free(airn);
	free(enrows);
	free(en);
	free(filename);
	free(filename2);
}

AIR_Error AIR_do_definecommon_air(const char *program, const char *inprefix, const char *insuffix, const char *outprefix, const char *outsuffix, const unsigned int n, char **ids, const AIR_Boolean ow)

{
	char *filename=NULL;
	char *filename2=NULL;
	
	/* Figure out how many characters filename will need*/
	{
		size_t count=0;
		{
			char **a;
			unsigned int k;
			
			for(a=ids,k=0;k<n;a++,k++){
			
				size_t temp=strlen(*a);
				if(temp>count) count=temp;
			}
		}
		{
			size_t temp1=strlen(inprefix)+strlen(insuffix);
			size_t temp2=strlen(outprefix)+strlen(outsuffix);
			
			if(temp2>temp1) temp1=temp2;
			count+=temp1;
		}
		count++;	/* Space for null terminator */
		
		filename=malloc(count);
		if(!filename){
			return(AIR_MEMORY_ALLOCATION_ERROR);
		}
		filename2=malloc(count);
		if(!filename2){
			free(filename);
			return(AIR_MEMORY_ALLOCATION_ERROR);
		}
	}
	

		
	/* Precheck of output permissions */
	{
		char **a;
		unsigned int k;
		
		for(a=ids,k=0;k<n;a++,k++){

			/*Compose output file name*/					
			sprintf(filename,"%s%s%s",outprefix,*a,outsuffix);
			
			{
				AIR_Error errcode=AIR_fprobw(filename,ow);
				if(errcode!=0){
					free(filename2);
					free(filename);
					return(errcode);
				}
			}		
		}
	}
	
	{
		struct AIR_Air16 *airn=NULL;
		double ***en=NULL;
		double **enrows=NULL;
		
		/*Allocate memory sufficient to handle all the input files*/
		airn=(struct AIR_Air16 *)malloc(n*sizeof(struct AIR_Air16));
		enrows=(double **)malloc(4*n*sizeof(double *));
		en=(double ***)malloc(n*sizeof(double **));

		if(!airn || !enrows || !en){
			free_function(airn,en,enrows,filename,filename2);
			return(AIR_MEMORY_ALLOCATION_ERROR);
		}
		{
			unsigned int i;
			
			for(i=0;i<n;i++){
				en[i]=&enrows[4*i];
			}
		}
		{
			unsigned int i;
			
			for(i=0;i<n;i++){
				en[i][0]=airn[i].e[0];
				en[i][1]=airn[i].e[1];
				en[i][2]=airn[i].e[2];
				en[i][3]=airn[i].e[3];
			}	
		}

		/*Load all of the files*/
		{
			unsigned int k;
			char **a;
			
			for(a=ids,k=0;k<n;a++,k++){

				sprintf(filename,"%s%s%s",inprefix,*a,insuffix);
				{
					AIR_Error errcode=AIR_read_air16(filename,&airn[k]);
					if(errcode!=0){
						free_function(airn,en,enrows,filename,filename2);
						return(errcode);
					}
				}
				if(k==0) strcpy(filename2,filename);
				else{
					/*Test compatibility of standard spaces*/
					{
						AIR_Error errcode=AIR_same_dim(&airn[k].s,&airn[0].s);
						if(errcode!=0){
							printf("%s: %d: ",__FILE__,__LINE__);
							printf("files %s and %s have incompatible standard file matrix dimensions\n",filename2,filename);
							free_function(airn,en,enrows,filename,filename2);
							return(errcode);
						}
					}
					{
						AIR_Error errcode=AIR_same_size(&airn[k].s,&airn[0].s);
						if(errcode!=0){
							printf("%s: %d: ",__FILE__,__LINE__);
							printf("files %s and %s have incompatible standard file voxel sizes\n",filename2,filename);
							free_function(airn,en,enrows,filename,filename2);
							return(errcode);
						}
					}
					if(strcmp(airn[k].s_file,airn[0].s_file)!=0){
						printf("WARNING: Potential standard file discrepancy\n\t.air file %s: %s\n\t.air file %s: %s\n",filename2,airn[0].s_file,filename,airn[k].s_file);
					}
				}
				{
					double pixel_size_s=airn[k].s.x_size;
					if(airn[k].s.y_size<pixel_size_s) pixel_size_s=airn[k].s.y_size;
					if(airn[k].s.z_size<pixel_size_s) pixel_size_s=airn[k].s.z_size;

					/* Adjust both spaces to unitary voxel sizes */
					{
						unsigned int j;
						
						for(j=0;j<3;j++){	/*Note: j<3 instead of j<4 is not an error*/
							
							unsigned int i;
							
							for(i=0;i<4;i++){
								en[k][j][i]/=pixel_size_s;
							}

						}
					}
				}
				{
					unsigned int j;
					
					for(j=0;j<4;j++){
							en[k][j][0]*=airn[k].r.x_size;
							en[k][j][1]*=airn[k].r.y_size;
							en[k][j][2]*=airn[k].r.z_size;
							/* Note: omission of en[k][j][3] is not an error*/
					}
				}
			}
		}

		/*Do the averaging*/
		{
			double toproot[8];
			double botroot[8];
			
			{
				AIR_Error errcode=AIR_padetanh(&toproot[0],&botroot[0]);
				if(errcode!=0){
					free_function(airn,en,enrows,filename,filename2);
					return(errcode);
				}
			}
			{
				double ***storage2, ***storage4;
				storage2=AIR_matrix3(2,2,20);
				if(!storage2){
					free_function(airn,en,enrows,filename,filename2);
					return(AIR_MEMORY_ALLOCATION_ERROR);
				}
				storage4=AIR_matrix3(4,4,14);
				if(!storage4){
					free_function(airn,en,enrows,filename,filename2);
					AIR_free_3(storage2);
					return(AIR_MEMORY_ALLOCATION_ERROR);
				}
				{
					AIR_Error errcode=AIR_comloger(0,en,n,toproot,botroot,storage2,storage4);
					if(errcode!=0){
						free_function(airn,en,enrows,filename,filename2);
						AIR_free_3(storage2);
						AIR_free_3(storage4);
						return(errcode);
					}
				}
				AIR_free_3(storage2);
				AIR_free_3(storage4);
			}
		}


		/* Now do the bookkeeping to write out the correct answers*/
		{
			unsigned int k;
			char **a;
			
			for(a=ids,k=0;k<n;a++,k++){
			
				/*Compose output file name*/
				sprintf(filename,"%s%s%s",outprefix,*a,outsuffix);
				{
					double pixel_size_s=airn[k].s.x_size;
					if(airn[k].s.y_size<pixel_size_s) pixel_size_s=airn[k].s.y_size;
					if(airn[k].s.z_size<pixel_size_s) pixel_size_s=airn[k].s.z_size;

					/* Adjust both spaces from unitary voxel sizes */
					{
						unsigned int j;
						
						for(j=0;j<4;j++){
								en[k][j][0]/=airn[k].r.x_size;
								en[k][j][1]/=airn[k].r.y_size;
								en[k][j][2]/=airn[k].r.z_size;
								/* Note: omission of en[k][j][3] is not an error*/
						}
					}
					{
						unsigned int j;
						
						for(j=0;j<3;j++){	/*Note: j<3 instead of j<4 is not an error*/
						
							unsigned int i;
							
							for(i=0;i<4;i++){
								en[k][j][i]*=pixel_size_s;
							}
						}
					}
				}
				/*Set standard file to null*/
				strcpy(airn[k].s_file,"null");
				airn[k].s_hash=0;

				/*Write name of this program into comment*/
				if(strlen(program)<(size_t)AIR_CONFIG_MAX_COMMENT_LENGTH){
					strcpy(airn[k].comment,program);
				}
				else{
					const char *shortname=AIR_file_from_path(program);
					strncpy(airn[k].comment,shortname,(size_t)AIR_CONFIG_MAX_COMMENT_LENGTH-1);
					airn[k].comment[AIR_CONFIG_MAX_COMMENT_LENGTH-1]='\0';
				}

				/*Write out the answer*/
				{
					AIR_Error errcode=AIR_write_air16(filename,ow,en[k],TRUE,&airn[k]);
					if(errcode!=0){
						free_function(airn,en,enrows,filename,filename2);
						return(errcode);
					}
				}
			}
		}

		/*Clean up*/
		free_function(airn,en,enrows,filename,filename2);
	}

	return 0;
}
