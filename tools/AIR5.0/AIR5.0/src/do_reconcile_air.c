/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 12/22/01 */

/*
 * This reconciles a net of .air files
 * It does error checking for compatibility
 */


#include "AIR.h"
#define TINY 1e-10
#define MAXITERS 30

static void free_function(/*@null@*/ /*@out@*/ /*@only@*/ struct AIR_Air16 **airn, /*@null@*/ /*@out@*/ /*@only@*/ struct AIR_Air16 *airnrows, /*@null@*/ /*@out@*/ /*@only@*/ double ****en, /*@null@*/ /*@out@*/ /*@only@*/ double ****em, /*@null@*/ /*@out@*/ /*@only@*/ double ***en2, /*@null@*/ /*@out@*/ /*@only@*/ double ***encols, /*@null@*/ /*@out@*/ /*@only@*/ double **enrows, /*@null@*/ /*@out@*/ /*@only@*/ double ***storage2, /*@null@*/ /*@out@*/ /*@only@*/ double ***storage4, /*@null@*/ /*@out@*/ /*@only@*/ char *filename)

{
	if(airn) free(airn);
	if(airnrows) free(airnrows);
	if(en) free(en);
	if(em) AIR_free_4(em);
	if(en2) AIR_free_3(en2);
	if(encols) free(encols);
	if(enrows) free(enrows);
	if(storage2) AIR_free_3(storage2);
	if(storage4) AIR_free_3(storage4);
	if(filename) free(filename);
}

AIR_Error AIR_do_reconcile_air(const char *program, const unsigned int iterations, const char *inprefix, const char *intron, const char *insuffix, const char *outprefix, const char *outsuffix, const unsigned int n, char **ids, const AIR_Boolean ow)

{
	char *filename;
	if(n<2) return AIR_USER_INTERFACE_ERROR;
	
	/* Figure out how many characters filename will need */
	{
		size_t count=0;
		{
			unsigned int k;
			char **a;
			
			for(k=0,a=ids;k<n;k++,a++){
			
				unsigned int m;
				char **b;
				
				for(m=0,b=ids;m<n;m++,b++){

					if(k!=m){
					
						size_t temp=strlen(*a)+strlen(*b);
						if(temp>count) count=temp;
					}
				}
			}
		}
		{
			size_t temp1=strlen(inprefix)+strlen(insuffix);
			size_t temp2=strlen(outprefix)+strlen(outsuffix);
			
			if(temp2>temp1) temp1=temp2;
			count+=temp1;
		}
		count+=strlen(intron);
		count++;	/* Space for null terminator */
		
		filename=malloc(count);
		if(!filename){
			return(AIR_MEMORY_ALLOCATION_ERROR);
		}
	}

	{
		/* Precheck of output permissions */
		
		unsigned int k;
		char **a;
		
		for(k=0,a=ids;k<n;k++,a++){
		
			unsigned int m;
			char **b;
			
			for(m=0,b=ids;m<n;m++,b++){
			
				
				/*Compose output file name*/
				sprintf(filename,"%s%s%s%s%s",outprefix,*a,intron,*b,outsuffix);
				
				{
					AIR_Error errcode=AIR_fprobw(filename,ow);
					if(errcode!=0){
						free(filename);
						return(errcode);
					}
				}	
			}
		}
	}
	{
		struct AIR_Air16 **airn=NULL;
		struct AIR_Air16 *airnrows=NULL;
		double ****en=NULL;
		double ****em=NULL;
		double ***en2=NULL;
		double ***encols=NULL;
		double **enrows=NULL;
		double ***storage2=NULL;
		double ***storage4=NULL;
		
		/*Allocate memory sufficient to handle all of the input files, etc*/
		airnrows=(struct AIR_Air16 *)malloc(n*n*sizeof(struct AIR_Air16));
		airn=(struct AIR_Air16 **)malloc(n*sizeof(struct AIR_Air16 *));
		enrows=(double **)malloc(4*n*n*sizeof(double *));
		encols=(double ***)malloc(n*n*sizeof(double **));
		en=(double ****)malloc(n*sizeof(double ***));
		em=AIR_matrix4(4,4,n,n);
		if(n>1) en2=AIR_matrix3(4,4,n);
		else en2=AIR_matrix3(4,4,2);
		storage2=AIR_matrix3(2,2,20);
		storage4=AIR_matrix3(4,4,13);

		if(!airnrows || !airn || !enrows || !encols || !en || !em || !en2 || !storage2 || !storage4){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("memory allocation failure in %s\n",program);
			free_function(airn,airnrows,en,em,en2,encols,enrows,storage2,storage4,filename);
			return(AIR_MEMORY_ALLOCATION_ERROR);
		}

		{
			unsigned int i;
			
			for(i=0;i<n;i++){
				airn[i]=&airnrows[n*i];
			}
		}
		{
			unsigned int i;
			
			for(i=0;i<n*n;i++){
				encols[i]=&enrows[4*i];
			}
		}
		{
			unsigned int i;
			
			for(i=0;i<n;i++){
				en[i]=&encols[n*i];
			}
		}
		/*Set pointers*/
		{
			unsigned int j;
			
			for(j=0;j<n;j++){
			
				unsigned int i;
				
				for(i=0;i<n;i++){
					en[j][i][0]=airn[j][i].e[0];
					en[j][i][1]=airn[j][i].e[1];
					en[j][i][2]=airn[j][i].e[2];
					en[j][i][3]=airn[j][i].e[3];
				}	
			}
		}

		/*Load all of the files*/
		{
			unsigned int k;
			char **a;
			
			for(k=0,a=ids;k<n;k++,a++){
			
				unsigned int m;
				char **b;
				
				for(m=0,b=ids;m<n;m++,b++){

					if(k!=m){
						
						sprintf(filename,"%s%s%s%s%s",inprefix,*a,intron,*b,insuffix);

						/* See if file exists and is readable */

						if(AIR_fprobr(filename)==0){

							{
								AIR_Error errcode=AIR_read_air16(filename,&airn[k][m]);
								
								if(errcode!=0){
									free_function(airn,airnrows,en,em,en2,encols,enrows,storage2,storage4,filename);
									return(errcode);
								}
							}
							
							/* Adjust both spaces to unitary voxel sizes */
							{
								double pixel_size_s=airn[k][m].s.x_size;
								if(airn[k][m].s.y_size<pixel_size_s) pixel_size_s=airn[k][m].s.y_size;
								if(airn[k][m].s.z_size<pixel_size_s) pixel_size_s=airn[k][m].s.z_size;

								{
									unsigned int j;
									
									for(j=0;j<3;j++){	/*Note: j<3 instead of j<4 is not an error*/
									
										unsigned int i;
										
										for(i=0;i<4;i++){
											en[k][m][j][i]/=pixel_size_s;
										}
									}
								}
							}
							{
								unsigned int j;
								
								for(j=0;j<4;j++){
										en[k][m][j][0]*=airn[k][m].r.x_size;
										en[k][m][j][1]*=airn[k][m].r.y_size;
										en[k][m][j][2]*=airn[k][m].r.z_size;
										/* Note: omission of en[k][m][j][3] is not an error*/
								}
							}
						}
						else{
							/*Try reading the inverse since forward file doesn't exist */
							sprintf(filename,"%s%s%s%s%s",inprefix,*b,intron,*a,insuffix);
							{
								AIR_Error errcode=AIR_read_air16(filename,&airn[k][m]);
								
								if(errcode!=0){
									printf("%s: %d: ",__FILE__,__LINE__);
									printf("Attempt to open file %s%s%s%s%s also failed\n",inprefix,*a,intron,*b,insuffix);
									free_function(airn,airnrows,en,em,en2,encols,enrows,storage2,storage4,filename);
									return(errcode);
								}
							}
							
							/* Adjust both spaces to unitary voxel sizes */
							{
								double pixel_size_s=airn[k][m].s.x_size;
								if(airn[k][m].s.y_size<pixel_size_s) pixel_size_s=airn[k][m].s.y_size;
								if(airn[k][m].s.z_size<pixel_size_s) pixel_size_s=airn[k][m].s.z_size;


								{
									unsigned int j;
									
									for(j=0;j<3;j++){	/*Note: j<3 instead of j<4 is not an error*/
									
										unsigned int i;
										
										for(i=0;i<4;i++){
											en[k][m][j][i]/=pixel_size_s;
										}
									}
								}
							}
							{
								unsigned int j;
								
								for(j=0;j<4;j++){
										en[k][m][j][0]*=airn[k][m].r.x_size;
										en[k][m][j][1]*=airn[k][m].r.y_size;
										en[k][m][j][2]*=airn[k][m].r.z_size;
										/* Note: omission of en[k][m][j][3] is not an error*/
								}
							}

							/* Invert the real world matrix */
							{
								AIR_Error errcode=AIR_gael(en[k][m]);
								
								if(errcode!=0){
									free_function(airn,airnrows,en,em,en2,encols,enrows,storage2,storage4,filename);
									return(errcode);
								}
							}
								/* Bookkeeping inversion */
							{
								char tempstr[AIR_CONFIG_MAX_PATH_LENGTH];

								strcpy(tempstr,airn[k][m].r_file);
								strcpy(airn[k][m].r_file,airn[k][m].s_file);
								strcpy(airn[k][m].s_file,tempstr);
							}
							{
								struct AIR_Key_info tempinfo=airn[k][m].r;
								airn[k][m].r=airn[k][m].s;
								airn[k][m].s=tempinfo;
							}
							{
								unsigned long int templong=airn[k][m].r_hash;
								airn[k][m].r_hash=airn[k][m].s_hash;
								airn[k][m].s_hash=templong;
							}
						}
						
						/* Fill in file names and attributes along the diagnonal except the last*/
						if(k==m+1){
							strcpy(airn[m][m].s_file, airn[k][m].s_file);
							airn[m][m].s=airn[k][m].s;
							airn[m][m].s_hash=airn[k][m].s_hash;
						}
						else if(m==k+1){
							strcpy(airn[k][k].r_file, airn[k][m].r_file);
							airn[k][k].r=airn[k][m].r;
							airn[k][k].r_hash=airn[k][m].r_hash;
						}

						/*Verify that standard file is compatible*/
						if(strcmp(airn[k][m].s_file,airn[0][m].s_file)!=0){
							printf("WARNING: potential standard file mismatch: %s %s\n",airn[k][m].s_file,airn[0][m].s_file);
							printf("detected in %s\n",filename);
						}
						{
							AIR_Error errcode=AIR_same_size(&airn[k][m].s,&airn[0][m].s);
							if(errcode!=0){
								printf("%s: %d: ",__FILE__,__LINE__);
								printf("ERROR: Standard file mismatch in size\n");
								printf("detected in %s\n",filename);
								free_function(airn,airnrows,en,em,en2,encols,enrows,storage2,storage4,filename);
								return(errcode);
							}
						}
						{
							AIR_Error errcode=AIR_same_dim(&airn[k][m].s,&airn[0][m].s);
							
							if(errcode!=0){
								printf("%s: %d: ",__FILE__,__LINE__);
								printf("ERROR: Standard file mismatch in dimension\n");
								printf("detected in %s\n",filename);
								free_function(airn,airnrows,en,em,en2,encols,enrows,storage2,storage4,filename);
								return(errcode);
							}
						}
						/*Verify that reslice file is compatible*/
						if(strcmp(airn[k][m].r_file,airn[k][0].r_file)!=0){
							printf("WARNING: potential reslice file mismatch: %s %s\n",airn[k][m].r_file,airn[k][0].r_file);
							printf("detected in %s\n",filename);
						}
						{
							AIR_Error errcode=AIR_same_size(&airn[k][m].r,&airn[k][0].r);
							
							if(errcode!=0){
								printf("%s: %d: ",__FILE__,__LINE__);
								printf("ERROR: Reslice file mismatch in size\n");
								printf("detected in %s\n",filename);
								free_function(airn,airnrows,en,em,en2,encols,enrows,storage2,storage4,filename);
								return(errcode);
							}
						}
						{
							AIR_Error errcode=AIR_same_dim(&airn[k][m].r,&airn[k][0].r);
							
							if(errcode!=0){
								printf("%s: %d: ",__FILE__,__LINE__);
								printf("ERROR: Reslice file mismatch in dimension\n");
								printf("detected in %s\n",filename);
								free_function(airn,airnrows,en,em,en2,encols,enrows,storage2,storage4,filename);
								return(errcode);
							}	
						}
					}
				}

				/* Identity matrix for registration to self for diagonal elements */
				en[k][k][0][1]=en[k][k][0][2]=en[k][k][0][3]=0;
				en[k][k][1][0]=en[k][k][1][2]=en[k][k][1][3]=0;
				en[k][k][2][0]=en[k][k][2][1]=en[k][k][2][3]=0;
				en[k][k][3][0]=en[k][k][3][1]=en[k][k][3][2]=0;
				en[k][k][0][0]=en[k][k][1][1]=en[k][k][2][2]=en[k][m][3][3]=1;
			}
			/* Fill in the last diagonal element */
			strcpy(airn[n-1][n-1].s_file, airn[n-2][n-1].s_file);
			airn[n-1][n-1].s=airn[n-2][n-1].s;
			airn[n-1][n-1].s_hash=airn[n-2][n-1].s_hash;
			
			strcpy(airn[n-1][n-1].r_file, airn[n-1][n-2].r_file);
			airn[n-1][n-1].r=airn[n-1][n-2].r;
			airn[n-1][n-1].r_hash=airn[n-1][n-2].r_hash;
		}

		{
			double toproot[8];
			double botroot[8];
			
			unsigned int iters=0;
			{
				AIR_Error errcode=AIR_padetanh(&toproot[0],&botroot[0]);
				if(errcode!=0){
					free_function(airn,airnrows,en,em,en2,encols,enrows,storage2,storage4,filename);
					return(errcode);
				}
			}

			while(iters<iterations){
			
				/*Do the averaging*/
				{
					unsigned int k;
					
					for(k=0;k<n;k++){
					
						unsigned int m;
						
						for(m=0;m<n;m++){
							if(k!=m){
							
								{
									unsigned int p;
									
									for(p=0;p<n;p++){
										AIR_dgemm('n','n',4,4,4,1.0,en[m][p],en[p][k],0.0,en2[p]);
									}
								}
								{
									double accuracy;
									AIR_Error errcode=AIR_comtloger(0,en2,n,em[m][k],TINY,MAXITERS,&accuracy,toproot,botroot,storage2,storage4);
									
									if(errcode!=0){
										printf("%s: %d: ",__FILE__,__LINE__);
										printf("Failure in comtloger()\n");
										free_function(airn,airnrows,en,em,en2,encols,enrows,storage2,storage4,filename);
										return(errcode);
									}
								}
							}
						}
					}
				}


				/*Reconcile inverses with each other*/
				{
					unsigned int k;
					
					for(k=0;k<n;k++){
					
						unsigned int m;
						
						for(m=0;m<n;m++){
							if(k!=m){
								{
									unsigned int j;
									
									for(j=0;j<4;j++){
									
										unsigned int i;
										
										for(i=0;i<4;i++){
											en2[0][j][i]=em[m][k][j][i];
											en2[1][j][i]=em[k][m][j][i];
										}
									}
								}

								/* Invert en2[1] */
								{
									AIR_Error errcode=AIR_gael(en2[1]);
									
									if(errcode!=0){
										free_function(airn,airnrows,en,em,en2,encols,enrows,storage2,storage4,filename);
										return(errcode);
									}
								}

								/* Make en2[1] exactly affine */
								en2[1][0][3]=en2[1][1][3]=en2[1][2][3]=0.0;
								en2[1][3][3]=1.0;

								{
									double accuracy;
									
									AIR_Error errcode=AIR_comtloger(0,en2,2,en[m][k],TINY,MAXITERS,&accuracy,toproot,botroot,storage2,storage4);
									if(errcode!=0){
										printf("%s: %d: ",__FILE__,__LINE__);
										printf("Failure in comtloger() during inversion reconciliation\n");
										free_function(airn,airnrows,en,em,en2,encols,enrows,storage2,storage4,filename);
										return(errcode);
									}
								}
							}
						}
					}
				}
				iters++;
			}
		}
		
		{
			const char *progname=strrchr(program,AIR_CONFIG_PATH_SEP);
			
			if(!progname) progname=program;
			else progname++;
			
			{
				/*Write out the results*/
				
				unsigned int k;
				char **a;
				
				for(k=0,a=ids;k<n;k++,a++){
				
					unsigned int m;
					char **b;
					
					for(m=0,b=ids;m<n;m++,b++){
					

						if(k!=m){

							/*Reverify existence of disk input file*/
							sprintf(filename,"%s%s%s%s%s",inprefix,*a,intron,*b,insuffix);
							{
								FILE *fp=fopen(filename,"rb");
								
								if(fp){
									if(fclose(fp)!=0){
										printf("%s: %d: ",__FILE__,__LINE__);
										printf("Failed to close input file %s\n",filename);
										free_function(airn,airnrows,en,em,en2,encols,enrows,storage2,storage4,filename);
										return(AIR_CANT_CLOSE_READ_ERROR);
									}
								}
								else continue;	/* Don't make output for name without input */
							}
						}

						/*Compose output file name*/
						sprintf(filename,"%s%s%s%s%s",outprefix,*a,intron,*b,outsuffix);

						/* Adjust both spaces from unitary voxel sizes */
						{
							unsigned int j;
							
							for(j=0;j<4;j++){
									en[k][m][j][0]/=airn[k][m].r.x_size;
									en[k][m][j][1]/=airn[k][m].r.y_size;
									en[k][m][j][2]/=airn[k][m].r.z_size;
									/* Note: omission of en[k][m][j][3] is not an error*/
							}
						}
						{
							double pixel_size_s=airn[k][m].s.x_size;
							if(airn[k][m].s.y_size<pixel_size_s) pixel_size_s=airn[k][m].s.y_size;
							if(airn[k][m].s.z_size<pixel_size_s) pixel_size_s=airn[k][m].s.z_size;

							{
								unsigned int j;
								
								for(j=0;j<3;j++){	/*Note: j<3 instead of j<4 is not an error*/
									
									unsigned int i;
									
									for(i=0;i<4;i++){
										en[k][m][j][i]*=pixel_size_s;
									}
								}
							}
						}
						if(strlen(progname)<(size_t)AIR_CONFIG_MAX_COMMENT_LENGTH){
							strcpy(airn[k][m].comment,program);
						}
						else{
							const char *shortname=AIR_file_from_path(progname);
							strncpy(airn[k][m].comment,shortname,(size_t)AIR_CONFIG_MAX_COMMENT_LENGTH-1);
							airn[k][m].comment[AIR_CONFIG_MAX_COMMENT_LENGTH-1]='\0';
						}
						{
							AIR_Error errcode=AIR_write_air16(filename,ow,en[k][m],TRUE,&airn[k][m]);
							
							if(errcode!=0){
								free_function(airn,airnrows,en,em,en2,encols,enrows,storage2,storage4,filename);
								return(errcode);
							}
						}
					}
				}
			}
		}
		free_function(airn,airnrows,en,em,en2,encols,enrows,storage2,storage4,filename);
	}
	return 0;
}
