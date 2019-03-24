/* Copyright 1998-2002 Roger P. Woods, M.D. */
/* Modified: 1/16/02 */

/* 
 */

#include "AIR.h"

static unsigned int parse_line(char *line, unsigned int expectedvalidvalues, double *values){

	unsigned int validvalues=0;
	char *lineptr=line;

	for(;;){
		/* Skip any white space except \n */
		while(isspace((int)*lineptr)){
			if(*lineptr=='\n') break;
			lineptr++;
		}
		if(*lineptr=='\n') break;
		{
			double value;
			/* Try to read another */
			if(validvalues>=expectedvalidvalues){
				/* We do not have storage for the result */
				if(sscanf(lineptr,"%le",&value)!=1) return 0;
			}
			else{
				/* Store the result */
				if(sscanf(lineptr,"%le",values)!=1) return 0;
				else values++;
			}
		}
		validvalues++;
			
		/* Advance to next white space */
		while(!isspace((int)*lineptr)) lineptr++;
	}
	return validvalues;
}

AIR_Error AIR_do_reslice_ucf(const char *airfile, const char *ucf_in, const char *ucf_out, AIR_Boolean ow)

{
	struct AIR_Air16 air1;
	double *e[4];
	FILE *fp_out;
	FILE *fp_in=fopen(ucf_in,"r");
	
	
	if(!fp_in){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("Failed to open file %s\n",ucf_in);
		{
			AIR_Error errcode=AIR_fprobr(ucf_in);
			if(errcode==0) errcode=AIR_UNSPECIFIED_FILE_READ_ERROR;
			return errcode;
		}
	}
	{
		AIR_Error errcode=AIR_fprobw(ucf_out,ow);
		if(errcode!=0){
			(void)fclose(fp_in);
			return errcode;
		}
	}
	fp_out=fopen(ucf_out,"w");
	if(!fp_out){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("Failed to open file %s for output\n",ucf_out);
		{
			AIR_Error errcode=AIR_fprobw(ucf_out,ow);
			if(errcode==0) errcode=AIR_UNSPECIFIED_FILE_WRITE_ERROR;
			(void)fclose(fp_in);
			return errcode;
		}
	}
	
	/*Get the reslice parameters*/
	
	e[0]=air1.e[0];
	e[1]=air1.e[1];
	e[2]=air1.e[2];
	e[3]=air1.e[3];
	{
		AIR_Error errcode=AIR_read_air16((airfile),&air1);
		
		if(errcode!=0){
			(void)fclose(fp_in);
			(void)fclose(fp_out);
			return(errcode);
		}
	}
	/* Invert the air file */
	{
		/* Mathematical inversion of e */
		{
			double f0[16];
			double *f[4];
			
			f[0]=f0;
			f[1]=f0+4;
			f[2]=f0+8;
			f[3]=f0+12;
			{
				AIR_Error errcode=AIR_inverter(e,f,TRUE,&air1.s,&air1.r);
				if(errcode!=0){
					printf("%s: %d: ",__FILE__,__LINE__);
					printf("Inversion failed\n");
					return(errcode);
				}
			}
			/* Copy back into e */
			{
				int j;
				for(j=0;j<4;j++){
					int i;
					for(i=0;i<4;i++){
						e[j][i]=f[j][i];
					}
				}
			}
		}
	}
	/* Adjust to millimeter coordinates */
	{
		double pixel_size_s=air1.s.x_size;
		if(air1.s.y_size<pixel_size_s) pixel_size_s=air1.s.y_size;
		if(air1.s.z_size<pixel_size_s) pixel_size_s=air1.s.z_size;

		{
			unsigned int j;
			
			for(j=0;j<3;j++){	/*Note: j<3 instead of j<4 is not an error*/
			
				unsigned int i;
				
				for(i=0;i<4;i++){
					e[j][i]/=pixel_size_s;
				}
			}
		}
		{
			unsigned int j;
			
			for(j=0;j<4;j++){
				e[j][0]*=air1.r.x_size;
				e[j][1]*=air1.r.y_size;
				e[j][2]*=air1.r.z_size;
			}
		}
	}
	{
		/* Determine how much memory needs allocated to hold the longest line in the file */
		unsigned int maxchars=0;
		{
			unsigned int chars=0;
			for(;;){
			
				int thischar=fgetc(fp_in);
				if(thischar==EOF){
					/* End of file (or error) */
					if(feof(fp_in)!=0) break;
					(void)fclose(fp_in);
					(void)fclose(fp_out);
					return AIR_UNSPECIFIED_FILE_READ_ERROR;
				}
				if((char)thischar=='\n' || (char)thischar=='\r'){
					/* End of line */
					if(chars>maxchars) maxchars=chars;
					chars=0;
				}
				else chars++;
			}
			maxchars+=2; /* Additional space for newline and null terminator */
		}
		rewind(fp_in);
		{
			char *line=malloc(maxchars*sizeof(char));
			if(!line){
				(void)fclose(fp_in);
				(void)fclose(fp_out);
				return AIR_MEMORY_ALLOCATION_ERROR;
			}
			{
				unsigned int expectedvalidvalues=0;
				double *values=NULL;
	
				for(;;){ /* Loop over lines */
					unsigned int validvalues=0;
					AIR_Boolean lastline=FALSE;
					char *lineptr=line;
					
					/* Read in a line */
					/* Can't use fgets() here because it won't recognize \r */
					for(;;){
						int thischar=fgetc(fp_in);
						
						if(thischar==EOF){
							if(feof(fp_in)!=0) lastline=TRUE;
							else{
								if(values) free(values);
								free(line);
								(void)fclose(fp_in);
								(void)fclose(fp_out);
								return AIR_UNSPECIFIED_FILE_READ_ERROR;
							}
						}
	
						/* Terminate if end of line or file with newline and terminator */
						if((char)thischar=='\n' || (char)thischar=='\r' || thischar==EOF){
							*lineptr++='\n';
							*lineptr='\0';
							break;
						}
						*lineptr++=(char)thischar;
					}

					/* Inspect the contents of the line */
					if(values==NULL){
						/* Test whether this is a string of three or more numbers */
						validvalues=parse_line(line,expectedvalidvalues,values);
						if(validvalues>=3){
							values=(double *)malloc(validvalues*sizeof(double));
							if(!values){
								free(line);
								(void)fclose(fp_in);
								(void)fclose(fp_out);
								return AIR_MEMORY_ALLOCATION_ERROR;
							}
							expectedvalidvalues=validvalues;
						}
						else{
							/* Deal with width line */
							if(strncmp(line,"<width=>",8)==0){
							
								lineptr=line;
								for(;;){
									int thischar=fgetc(fp_in);
									
									if(thischar==EOF){
										if(feof(fp_in)!=0) lastline=TRUE;
										else{
											if(values) free(values);
											free(line);
											(void)fclose(fp_in);
											(void)fclose(fp_out);
											return AIR_UNSPECIFIED_FILE_READ_ERROR;
										}
									}
				
									/* Terminate if end of line or file */
									if((char)thischar=='\n' || (char)thischar=='\r' || thischar==EOF){
										*lineptr='\0';
										break;
									}
									*lineptr++=(char)thischar;
								}
								
								{
									AIR_Error errcode;
									unsigned int width=AIR_parse_uint(line,&errcode);
									if(errcode!=0){
									
										if(values) free(values);
										free(line);
										(void)fclose(fp_in);
										(void)fclose(fp_out);
										return AIR_READ_UCF_FILE_ERROR;
									}
									if(width!=air1.r.x_dim){
										printf("WARNING: Based on .air file %s, expected input width of %u but found %u\n",airfile,air1.r.x_dim,width);
									}
									if(fprintf(fp_out,"%s\n%u\n","<width=>",air1.s.x_dim)==EOF){
										if(values) free(values);
										free(line);
										(void)fclose(fp_in);
										(void)fclose(fp_out);
										return AIR_WRITE_UCF_FILE_ERROR;
									}
								}					
							}
							/* Deal with height line */
							else if(strncmp(line,"<height=>",9)==0){
							
								lineptr=line;
								for(;;){
									int thischar=fgetc(fp_in);
									
									if(thischar==EOF){
										if(feof(fp_in)!=0) lastline=TRUE;
										else{
											if(values) free(values);
											free(line);
											(void)fclose(fp_in);
											(void)fclose(fp_out);
											return AIR_UNSPECIFIED_FILE_READ_ERROR;
										}
									}
				
									/* Terminate if end of line or file */
									if((char)thischar=='\n' || (char)thischar=='\r' || thischar==EOF){
										*lineptr='\0';
										break;
									}
									*lineptr++=(char)thischar;
								}
								
								{
									AIR_Error errcode;
									unsigned int height=AIR_parse_uint(line,&errcode);
									if(errcode!=0){
									
										if(values) free(values);
										free(line);
										(void)fclose(fp_in);
										(void)fclose(fp_out);
										return AIR_READ_UCF_FILE_ERROR;
									}
									if(height!=air1.r.y_dim){
										printf("WARNING: Based on .air file %s, expected input height of %u but found %u\n",airfile,air1.r.y_dim,height);
									}
									if(fprintf(fp_out,"%s\n%u\n","<height=>",air1.s.y_dim)==EOF){
										if(values) free(values);
										free(line);
										(void)fclose(fp_in);
										(void)fclose(fp_out);
										return AIR_WRITE_UCF_FILE_ERROR;
									}
								}					
							}
							/* Deal with xrange line */
							else if(strncmp(line,"<xrange=>",9)==0){
							
								double low_xrange, hi_xrange;
							
								lineptr=line;
								{
									int thischar;
									thischar=fgetc(fp_in);
									while(isspace(thischar)){
										thischar=fgetc(fp_in);
									}
									for(;;){
										
										if(thischar==EOF){
											if(feof(fp_in)!=0) lastline=TRUE;
											else{
												if(values) free(values);
												free(line);
												(void)fclose(fp_in);
												(void)fclose(fp_out);
												return AIR_UNSPECIFIED_FILE_READ_ERROR;
											}
										}
					
										/* Terminate if space or end of line or file */
										if(isspace(thischar) ||(char)thischar=='\n' || (char)thischar=='\r' || thischar==EOF){
											*lineptr='\0';
											break;
										}
										*lineptr++=(char)thischar;
										thischar=fgetc(fp_in);
									}
								}
								{
									AIR_Error errcode;
									low_xrange=AIR_parse_double(line,&errcode);
									if(errcode!=0){
									
										if(values) free(values);
										free(line);
										(void)fclose(fp_in);
										(void)fclose(fp_out);
										return AIR_READ_UCF_FILE_ERROR;
									}
								}
								lineptr=line;
								{
									int thischar;
									thischar=fgetc(fp_in);
									while(isspace(thischar)){
										thischar=fgetc(fp_in);
									}
									for(;;){
										
										if(thischar==EOF){
											if(feof(fp_in)!=0) lastline=TRUE;
											else{
												if(values) free(values);
												free(line);
												(void)fclose(fp_in);
												(void)fclose(fp_out);
												return AIR_UNSPECIFIED_FILE_READ_ERROR;
											}
										}
					
										/* Terminate if space or end of line or file */
										if(isspace(thischar) ||(char)thischar=='\n' || (char)thischar=='\r' || thischar==EOF){
											*lineptr='\0';
											break;
										}
										*lineptr++=(char)thischar;
										thischar=fgetc(fp_in);
									}
								}
								{
									AIR_Error errcode;
									hi_xrange=AIR_parse_double(line,&errcode);
									if(errcode!=0){
									
										if(values) free(values);
										free(line);
										(void)fclose(fp_in);
										(void)fclose(fp_out);
										return AIR_READ_UCF_FILE_ERROR;
									}
								}
								if(fabs(low_xrange+air1.r.x_size/2.0)>AIR_CONFIG_PIX_SIZE_ERR){
									printf("WARNING: Based on .air file %s, expected lower xrange of %f but found %f\n",airfile,-air1.r.x_size/2.0,low_xrange);
								}
								if(fabs(hi_xrange-(air1.r.x_size*air1.r.x_dim-air1.r.x_size/2.0))>AIR_CONFIG_PIX_SIZE_ERR){
									printf("WARNING: Based on .air file %s, expected upper xrange of %f but found %f\n",airfile,air1.r.x_size*air1.r.x_dim-air1.r.x_size/2.0,hi_xrange);
								}
								if(fprintf(fp_out,"%s\n%f %f\n","<xrange=>",-air1.s.x_size/2.0,air1.s.x_size*air1.s.x_dim-air1.s.x_size/2.0)==EOF){
									if(values) free(values);
									free(line);
									(void)fclose(fp_in);
									(void)fclose(fp_out);
									return AIR_WRITE_UCF_FILE_ERROR;
								}
							}
							/* Deal with yrange line */
							else if(strncmp(line,"<yrange=>",9)==0){
							
								double low_yrange, hi_yrange;
							
								lineptr=line;
								{
									int thischar;
									thischar=fgetc(fp_in);
									while(isspace(thischar)){
										thischar=fgetc(fp_in);
									}
									for(;;){
										
										if(thischar==EOF){
											if(feof(fp_in)!=0) lastline=TRUE;
											else{
												if(values) free(values);
												free(line);
												(void)fclose(fp_in);
												(void)fclose(fp_out);
												return AIR_UNSPECIFIED_FILE_READ_ERROR;
											}
										}
					
										/* Terminate if space or end of line or file */
										if(isspace(thischar) ||(char)thischar=='\n' || (char)thischar=='\r' || thischar==EOF){
											*lineptr='\0';
											break;
										}
										*lineptr++=(char)thischar;
										thischar=fgetc(fp_in);
									}
								}
								{
									AIR_Error errcode;
									low_yrange=AIR_parse_double(line,&errcode);
									if(errcode!=0){
									
										if(values) free(values);
										free(line);
										(void)fclose(fp_in);
										(void)fclose(fp_out);
										return AIR_READ_UCF_FILE_ERROR;
									}
								}
								lineptr=line;
								{
									int thischar;
									thischar=fgetc(fp_in);
									while(isspace(thischar)){
										thischar=fgetc(fp_in);
									}
									for(;;){
										
										if(thischar==EOF){
											if(feof(fp_in)!=0) lastline=TRUE;
											else{
												if(values) free(values);
												free(line);
												(void)fclose(fp_in);
												(void)fclose(fp_out);
												return AIR_UNSPECIFIED_FILE_READ_ERROR;
											}
										}
					
										/* Terminate if space or end of line or file */
										if(isspace(thischar) ||(char)thischar=='\n' || (char)thischar=='\r' || thischar==EOF){
											*lineptr='\0';
											break;
										}
										*lineptr++=(char)thischar;
										thischar=fgetc(fp_in);
									}
								}
								{
									AIR_Error errcode;
									hi_yrange=AIR_parse_double(line,&errcode);
									if(errcode!=0){
									
										if(values) free(values);
										free(line);
										(void)fclose(fp_in);
										(void)fclose(fp_out);
										return AIR_READ_UCF_FILE_ERROR;
									}
								}
								if(fabs(low_yrange+air1.r.y_size/2.0)>AIR_CONFIG_PIX_SIZE_ERR){
									printf("WARNING: Based on .air file %s, expected lower yrange of %f but found %f\n",airfile,-air1.r.y_size/2.0,low_yrange);
								}
								if(fabs(hi_yrange-(air1.r.y_size*air1.r.y_dim-air1.r.y_size/2.0))>AIR_CONFIG_PIX_SIZE_ERR){
									printf("WARNING: Based on .air file %s, expected upper yrange of %f but found %f\n",airfile,air1.r.y_size*air1.r.y_dim-air1.r.y_size/2.0,hi_yrange);
								}
								if(fprintf(fp_out,"%s\n%f %f\n","<yrange=>",-air1.s.y_size/2.0,air1.s.y_size*air1.s.y_dim-air1.s.y_size/2.0)==EOF){
									if(values) free(values);
									free(line);
									(void)fclose(fp_in);
									(void)fclose(fp_out);
									return AIR_WRITE_UCF_FILE_ERROR;
								}
							}
							/* Deal with zrange line */
							else if(strncmp(line,"<zrange=>",9)==0){
							
								double low_zrange, hi_zrange;
							
								lineptr=line;
								{
									int thischar;
									thischar=fgetc(fp_in);
									while(isspace(thischar)){
										thischar=fgetc(fp_in);
									}
									for(;;){
										
										if(thischar==EOF){
											if(feof(fp_in)!=0) lastline=TRUE;
											else{
												if(values) free(values);
												free(line);
												(void)fclose(fp_in);
												(void)fclose(fp_out);
												return AIR_UNSPECIFIED_FILE_READ_ERROR;
											}
										}
					
										/* Terminate if space or end of line or file */
										if(isspace(thischar) ||(char)thischar=='\n' || (char)thischar=='\r' || thischar==EOF){
											*lineptr='\0';
											break;
										}
										*lineptr++=(char)thischar;
										thischar=fgetc(fp_in);
									}
								}
								{
									AIR_Error errcode;
									low_zrange=AIR_parse_double(line,&errcode);
									if(errcode!=0){
									
										if(values) free(values);
										free(line);
										(void)fclose(fp_in);
										(void)fclose(fp_out);
										return AIR_READ_UCF_FILE_ERROR;
									}
								}
								lineptr=line;
								{
									int thischar;
									thischar=fgetc(fp_in);
									while(isspace(thischar)){
										thischar=fgetc(fp_in);
									}
									for(;;){
										
										if(thischar==EOF){
											if(feof(fp_in)!=0) lastline=TRUE;
											else{
												if(values) free(values);
												free(line);
												(void)fclose(fp_in);
												(void)fclose(fp_out);
												return AIR_UNSPECIFIED_FILE_READ_ERROR;
											}
										}
					
										/* Terminate if space or end of line or file */
										if(isspace(thischar) ||(char)thischar=='\n' || (char)thischar=='\r' || thischar==EOF){
											*lineptr='\0';
											break;
										}
										*lineptr++=(char)thischar;
										thischar=fgetc(fp_in);
									}
								}
								{
									AIR_Error errcode;
									hi_zrange=AIR_parse_double(line,&errcode);
									if(errcode!=0){
									
										if(values) free(values);
										free(line);
										(void)fclose(fp_in);
										(void)fclose(fp_out);
										return AIR_READ_UCF_FILE_ERROR;
									}
								}
								if(fabs(low_zrange+air1.r.z_size/2.0)>AIR_CONFIG_PIX_SIZE_ERR){
									printf("WARNING: Based on .air file %s, expected lower zrange of %f but found %f\n",airfile,-air1.r.z_size/2.0,low_zrange);
								}
								if(fabs(hi_zrange-(air1.r.z_size*air1.r.z_dim-air1.r.z_size/2.0))>AIR_CONFIG_PIX_SIZE_ERR){
									printf("WARNING: Based on .air file %s, expected upper zrange of %f but found %f\n",airfile,air1.r.z_size*air1.r.z_dim-air1.r.z_size/2.0,hi_zrange);
								}
								if(fprintf(fp_out,"%s\n%f %f\n","<zrange=>",-air1.s.z_size/2.0,air1.s.z_size*air1.s.z_dim-air1.s.z_size/2.0)==EOF){
									if(values) free(values);
									free(line);
									(void)fclose(fp_in);
									(void)fclose(fp_out);
									return AIR_WRITE_UCF_FILE_ERROR;
								}
							}
							/* Pass the entire line through unmodified */
							else if(fprintf(fp_out,"%s",line)==EOF){
								printf("%s: %d: ",__FILE__,__LINE__);
								printf("Failed to write to output file %s\n", ucf_out);
								if(values) free(values);
								free(line);
								(void)fclose(fp_in);
								(void)fclose(fp_out);
								return AIR_WRITE_UCF_FILE_ERROR;
							}
						}
					}
					/* Note that this should be evaluated even if values has just become non-NULL above */
					if(values!=NULL){
						/* Test whether this is a string of three or more numbers */
						validvalues=parse_line(line,expectedvalidvalues,values);
													
						if(validvalues>=3){
							if(validvalues!=expectedvalidvalues){
								printf("%s: %d: ",__FILE__,__LINE__);
								printf("Expected %u valid values, found %u\n",expectedvalidvalues,validvalues);
								if(values) free(values);
								free(line);
								(void)fclose(fp_in);
								(void)fclose(fp_out);
								return AIR_READ_UCF_FILE_ERROR;
							}
							else{
								/* write out modified version of the numbers */
								{
									double x, y, z, t;
									x=e[0][0]*values[0]+e[1][0]*values[1]+e[2][0]*values[2]+e[3][0];
									y=e[0][1]*values[0]+e[1][1]*values[1]+e[2][1]*values[2]+e[3][1];
									z=e[0][2]*values[0]+e[1][2]*values[1]+e[2][2]*values[2]+e[3][2];
									t=e[0][3]*values[0]+e[1][3]*values[1]+e[2][3]*values[2]+e[3][3];
									
									x/=t;
									y/=t;
									z/=t;
									
									if(expectedvalidvalues>3){
										if(fprintf(fp_out,"%f %f %f ",x,y,z)==EOF){
											printf("%s: %d: ",__FILE__,__LINE__);
											printf("Failed to write to output file %s\n",ucf_out);
											if(values) free(values);
											free(line);
											(void)fclose(fp_in);
											(void)fclose(fp_out);
											return AIR_WRITE_UCF_FILE_ERROR;
										}
									}
									else{
										if(fprintf(fp_out,"%f %f %f\n",x,y,z)==EOF){
											printf("%s: %d: ",__FILE__,__LINE__);
											printf("Failed to write to output file %s\n",ucf_out);
											if(values) free(values);
											free(line);
											(void)fclose(fp_in);
											(void)fclose(fp_out);
											return AIR_WRITE_UCF_FILE_ERROR;
										}
									}
								}
								/* Write any additonal numbers associated with the point*/
								{
									unsigned int i;
									
									for(i=3;i<expectedvalidvalues-1;i++){
										if(fprintf(fp_out,"%f ",values[i])==EOF){
											printf("%s: %d: ",__FILE__,__LINE__);
											printf("Failed to write to output file %s\n",ucf_out);
											if(values) free(values);
											free(line);
											(void)fclose(fp_in);
											(void)fclose(fp_out);
											return AIR_WRITE_UCF_FILE_ERROR;
										}
									}
									if(expectedvalidvalues>3){
										if(fprintf(fp_out,"%f\n",values[i])==EOF){
											printf("%s: %d: ",__FILE__,__LINE__);
											printf("Failed to write to output file %s\n",ucf_out);
											if(values) free(values);
											free(line);
											(void)fclose(fp_in);
											(void)fclose(fp_out);
											return AIR_WRITE_UCF_FILE_ERROR;
										}
									}
								}
							}
						}
						else{
							/* Pass the entire line through unmodified */
							if(fprintf(fp_out,"%s",line)==EOF){
								printf("%s: %d: ",__FILE__,__LINE__);
								printf("Failed to write to output file %s\n", ucf_out);
								if(values) free(values);
								free(line);
								(void)fclose(fp_in);
								(void)fclose(fp_out);
								return AIR_WRITE_UCF_FILE_ERROR;
							}
						}
					}
					if(lastline) break;
				}
				if(values) free(values);
			}
			free(line);
		}
	}
	if(fclose(fp_in)!=0) return AIR_CANT_CLOSE_READ_ERROR;
	if(fclose(fp_out)!=0) return AIR_CANT_CLOSE_WRITE_ERROR;

	return EXIT_SUCCESS;
}
