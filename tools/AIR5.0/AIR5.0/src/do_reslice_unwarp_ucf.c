/* Copyright 1998-2002 Roger P. Woods, M.D. */
/* Modified: 1/16/02 */

/* 
 */

#include "AIR.h"

static void free_function(double **e, char *line, double *values, double *dx, double *dx_pi, double *dx_pj, double *dx_pk)
{
	if(e) AIR_free_2(e);
	if(line) free(line);
	if(values) free(values);
	if(dx) free(dx);
	if(dx_pi) free(dx_pi);
	if(dx_pj) free(dx_pj);
	if(dx_pk) free(dx_pk);
}

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

AIR_Error AIR_do_reslice_unwarp_ucf(const char *warpfile, const char *ucf_in, const char *ucf_out, const signed int parity, AIR_Boolean strict, AIR_Boolean ow)

{	
	/* Verify that warpfile can be read*/
	if(warpfile){
		AIR_Error errcode=AIR_fprobr(warpfile);
		
		if(errcode!=0){
			printf("%s: %d: ",__FILE__,__LINE__);
			printf("Problem with file %s\n",warpfile);
			return(errcode);
		}
	}
	/* Verify that output can be saved */
	{
		AIR_Error errcode=AIR_save_probw(ucf_out,ow);
		
		if(errcode!=0) return errcode;
	}
	{
		struct AIR_Warp airwarp1;
		double **e=NULL;
		
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
		{
			AIR_Error errcode;
			e=AIR_read_airw(warpfile,&airwarp1,&errcode);
			
			if(!e){
				(void)fclose(fp_in);
				(void)fclose(fp_out);
				return(errcode);
			}
			if(airwarp1.coord!=3){
				printf("%s: %d: ",__FILE__,__LINE__);
				printf("3D warp files are required\n");
				AIR_free_2(e);
				(void)fclose(fp_in);
				(void)fclose(fp_out);
				return AIR_POLYNOMIAL_DIMENSIONS_ERROR;
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
						AIR_free_2(e);
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
					AIR_free_2(e);
					(void)fclose(fp_in);
					(void)fclose(fp_out);
					return AIR_MEMORY_ALLOCATION_ERROR;
				}
				{
					unsigned int expectedvalidvalues=0;
					double *values=NULL;
					
					double *dx=NULL;
					double *dx_pi=NULL;
					double *dx_pj=NULL;
					double *dx_pk=NULL;
					
					double *ed[3];
					double ed0[9];
	
					ed[0]=ed0;
					ed[1]=ed0+3;
					ed[2]=ed0+6;
					
					{
						unsigned int coeffp=(airwarp1.order+1)*(airwarp1.order+2)*(airwarp1.order+3)/6;
						dx=AIR_matrix1(coeffp);
						dx_pi=AIR_matrix1(coeffp);
						dx_pj=AIR_matrix1(coeffp);
						dx_pk=AIR_matrix1(coeffp);
						
						if(!dx || !dx_pi || !dx_pj || !dx_pk){
							free_function(e, line, values, dx, dx_pi, dx_pj, dx_pk);
							(void)fclose(fp_in);
							(void)fclose(fp_out);
							printf("%s: %d: ",__FILE__,__LINE__);
							printf("unable to allocate memory to reslice file\n");
							return AIR_MEMORY_ALLOCATION_ERROR;
						}
					}
		
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
									free_function(e, line, values, dx, dx_pi, dx_pj, dx_pk);
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
									free_function(e, line, values, dx, dx_pi, dx_pj, dx_pk);
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
												free_function(e, line, values, dx, dx_pi, dx_pj, dx_pk);
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
										
											free_function(e, line, values, dx, dx_pi, dx_pj, dx_pk);
											(void)fclose(fp_in);
											(void)fclose(fp_out);
											return AIR_READ_UCF_FILE_ERROR;
										}
										if(width!=airwarp1.s.x_dim){
											printf("WARNING: Based on .warp file %s, expected input width of %u but found %u\n",warpfile,airwarp1.s.x_dim,width);
										}
										if(fprintf(fp_out,"%s\n%u\n","<width=>",airwarp1.r.x_dim)==EOF){
											free_function(e, line, values, dx, dx_pi, dx_pj, dx_pk);
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
												free_function(e, line, values, dx, dx_pi, dx_pj, dx_pk);
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
										
											free_function(e, line, values, dx, dx_pi, dx_pj, dx_pk);
											(void)fclose(fp_in);
											(void)fclose(fp_out);
											return AIR_READ_UCF_FILE_ERROR;
										}
										if(height!=airwarp1.s.y_dim){
											printf("WARNING: Based on .warp file %s, expected input height of %u but found %u\n",warpfile,airwarp1.s.y_dim,height);
										}
										if(fprintf(fp_out,"%s\n%u\n","<height=>",airwarp1.r.y_dim)==EOF){
											free_function(e, line, values, dx, dx_pi, dx_pj, dx_pk);
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
													free_function(e, line, values, dx, dx_pi, dx_pj, dx_pk);
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
										
											free_function(e, line, values, dx, dx_pi, dx_pj, dx_pk);
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
													free_function(e, line, values, dx, dx_pi, dx_pj, dx_pk);
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
										
											free_function(e, line, values, dx, dx_pi, dx_pj, dx_pk);
											(void)fclose(fp_in);
											(void)fclose(fp_out);
											return AIR_READ_UCF_FILE_ERROR;
										}
									}
									if(fabs(low_xrange+airwarp1.s.x_size/2.0)>AIR_CONFIG_PIX_SIZE_ERR){
										printf("WARNING: Based on .warp file %s, expected lower xrange of %f but found %f\n",warpfile,-airwarp1.s.x_size/2.0,low_xrange);
									}
									if(fabs(hi_xrange-(airwarp1.s.x_size*airwarp1.s.x_dim-airwarp1.s.x_size/2.0))>AIR_CONFIG_PIX_SIZE_ERR){
										printf("WARNING: Based on .warp file %s, expected upper xrange of %f but found %f\n",warpfile,airwarp1.s.x_size*airwarp1.s.x_dim-airwarp1.s.x_size/2.0,hi_xrange);
									}
									if(fprintf(fp_out,"%s\n%f %f\n","<xrange=>",-airwarp1.r.x_size/2.0,airwarp1.r.x_size*airwarp1.r.x_dim-airwarp1.r.x_size/2.0)==EOF){
										free_function(e, line, values, dx, dx_pi, dx_pj, dx_pk);
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
													free_function(e, line, values, dx, dx_pi, dx_pj, dx_pk);
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
										
											free_function(e, line, values, dx, dx_pi, dx_pj, dx_pk);
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
													free_function(e, line, values, dx, dx_pi, dx_pj, dx_pk);
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
										
											free_function(e, line, values, dx, dx_pi, dx_pj, dx_pk);
											(void)fclose(fp_in);
											(void)fclose(fp_out);
											return AIR_READ_UCF_FILE_ERROR;
										}
									}
									if(fabs(low_yrange+airwarp1.s.y_size/2.0)>AIR_CONFIG_PIX_SIZE_ERR){
										printf("WARNING: Based on .warp file %s, expected lower yrange of %f but found %f\n",warpfile,-airwarp1.s.y_size/2.0,low_yrange);
									}
									if(fabs(hi_yrange-(airwarp1.s.y_size*airwarp1.s.y_dim-airwarp1.s.y_size/2.0))>AIR_CONFIG_PIX_SIZE_ERR){
										printf("WARNING: Based on .warp file %s, expected upper yrange of %f but found %f\n",warpfile,airwarp1.s.y_size*airwarp1.s.y_dim-airwarp1.s.y_size/2.0,hi_yrange);
									}
									if(fprintf(fp_out,"%s\n%f %f\n","<yrange=>",-airwarp1.r.y_size/2.0,airwarp1.r.y_size*airwarp1.r.y_dim-airwarp1.r.y_size/2.0)==EOF){
										free_function(e, line, values, dx, dx_pi, dx_pj, dx_pk);
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
													free_function(e, line, values, dx, dx_pi, dx_pj, dx_pk);
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
										
											free_function(e, line, values, dx, dx_pi, dx_pj, dx_pk);
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
													free_function(e, line, values, dx, dx_pi, dx_pj, dx_pk);
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
										
											free_function(e, line, values, dx, dx_pi, dx_pj, dx_pk);
											(void)fclose(fp_in);
											(void)fclose(fp_out);
											return AIR_READ_UCF_FILE_ERROR;
										}
									}
									if(fabs(low_zrange+airwarp1.s.z_size/2.0)>AIR_CONFIG_PIX_SIZE_ERR){
										printf("WARNING: Based on .warp file %s, expected lower zrange of %f but found %f\n",warpfile,-airwarp1.s.z_size/2.0,low_zrange);
									}
									if(fabs(hi_zrange-(airwarp1.s.z_size*airwarp1.s.z_dim-airwarp1.s.z_size/2.0))>AIR_CONFIG_PIX_SIZE_ERR){
										printf("WARNING: Based on .warp file %s, expected upper zrange of %f but found %f\n",warpfile,airwarp1.s.z_size*airwarp1.s.z_dim-airwarp1.s.z_size/2.0,hi_zrange);
									}
									if(fprintf(fp_out,"%s\n%f %f\n","<zrange=>",-airwarp1.r.z_size/2.0,airwarp1.r.z_size*airwarp1.r.z_dim-airwarp1.r.z_size/2.0)==EOF){
										free_function(e, line, values, dx, dx_pi, dx_pj, dx_pk);
										(void)fclose(fp_in);
										(void)fclose(fp_out);
										return AIR_WRITE_UCF_FILE_ERROR;
									}
								}
								/* Pass the entire line through unmodified */
								else if(fprintf(fp_out,"%s",line)==EOF){
									printf("%s: %d: ",__FILE__,__LINE__);
									printf("Failed to write to output file %s\n", ucf_out);
									free_function(e, line, values, dx, dx_pi, dx_pj, dx_pk);
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
									free_function(e, line, values, dx, dx_pi, dx_pj, dx_pk);
									(void)fclose(fp_in);
									(void)fclose(fp_out);
									return AIR_READ_UCF_FILE_ERROR;
								}
								else{
								
									double x_p, y_p, z_p;
									
									/* Convert to voxel based coordinates */
									values[0]/=airwarp1.s.x_size;
									values[1]/=airwarp1.s.y_size;
									values[2]/=airwarp1.s.z_size;
									
									if(AIR_warp3D(e,values[0],values[1],values[2],&x_p,&y_p,&z_p,airwarp1.order,dx)!=0){
										if(!strict) continue;
										else{
											free_function(e, line, values, dx, dx_pi, dx_pj, dx_pk);
											(void)fclose(fp_in);
											(void)fclose(fp_out);
											return AIR_ANTIWARP_ERROR;
										}
									}
				
									/* Make sure that determinant is positive here */
									{
										if(AIR_diffg3D(e,values[0],values[1],values[2],ed,airwarp1.order,dx_pi,dx_pj,dx_pk)!=0){
											if(!strict) continue;
											else{
												free_function(e, line, values, dx, dx_pi, dx_pj, dx_pk);
												(void)fclose(fp_in);
												(void)fclose(fp_out);
												return AIR_ANTIWARP_ERROR;
											}
										}
										{
											unsigned int ipvt[3];
											if(AIR_dgefa(ed,3,ipvt)!=3){
												if(!strict) continue;
												else{
													free_function(e, line, values, dx, dx_pi, dx_pj, dx_pk);
													(void)fclose(fp_in);
													(void)fclose(fp_out);
													return AIR_ANTIWARP_ERROR;
												}
											}
											if(parity*AIR_ddet(ed,3,ipvt)<=0){
												if(!strict) continue;
												else{
													free_function(e, line, values, dx, dx_pi, dx_pj, dx_pk);
													(void)fclose(fp_in);
													(void)fclose(fp_out);
													return AIR_NON_POSITIVE_DETERMINANT_ERROR;
												}
											}
										}
									}
									/* Convert to absolute distance units */
									x_p*=airwarp1.r.x_size;
									y_p*=airwarp1.r.y_size;
									z_p*=airwarp1.r.z_size;
									
									if(expectedvalidvalues>3){
										if(fprintf(fp_out,"%f %f %f ",x_p,y_p,z_p)==EOF){
											printf("%s: %d: ",__FILE__,__LINE__);
											printf("Failed to write to output file %s\n",ucf_out);
											free_function(e, line, values, dx, dx_pi, dx_pj, dx_pk);
											(void)fclose(fp_in);
											(void)fclose(fp_out);
											return AIR_WRITE_UCF_FILE_ERROR;
										}
									}
									else{
										if(fprintf(fp_out,"%f %f %f\n",x_p,y_p,z_p)==EOF){
											printf("%s: %d: ",__FILE__,__LINE__);
											printf("Failed to write to output file %s\n",ucf_out);
											free_function(e, line, values, dx, dx_pi, dx_pj, dx_pk);
											(void)fclose(fp_in);
											(void)fclose(fp_out);
											return AIR_WRITE_UCF_FILE_ERROR;
										}
									}
									/* Write any additonal numbers associated with the point*/
									{
										unsigned int i;
										
										for(i=3;i<expectedvalidvalues-1;i++){
											if(fprintf(fp_out,"%f ",values[i])==EOF){
												printf("%s: %d: ",__FILE__,__LINE__);
												printf("Failed to write to output file %s\n",ucf_out);
												free_function(e, line, values, dx, dx_pi, dx_pj, dx_pk);
												(void)fclose(fp_in);
												(void)fclose(fp_out);
												return AIR_WRITE_UCF_FILE_ERROR;
											}
										}
										if(expectedvalidvalues>3){
											if(fprintf(fp_out,"%f\n",values[i])==EOF){
												printf("%s: %d: ",__FILE__,__LINE__);
												printf("Failed to write to output file %s\n",ucf_out);
												free_function(e, line, values, dx, dx_pi, dx_pj, dx_pk);
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
									free_function(e, line, values, dx, dx_pi, dx_pj, dx_pk);
									(void)fclose(fp_in);
									(void)fclose(fp_out);
									return AIR_WRITE_UCF_FILE_ERROR;
								}
							}
						}
						if(lastline) break;
					}
					free_function(e, line, values, dx, dx_pi, dx_pj, dx_pk);
				}
			}
		}
		if(fclose(fp_in)!=0) return AIR_CANT_CLOSE_READ_ERROR;
		if(fclose(fp_out)!=0) return AIR_CANT_CLOSE_WRITE_ERROR;
	}

	return EXIT_SUCCESS;
}
