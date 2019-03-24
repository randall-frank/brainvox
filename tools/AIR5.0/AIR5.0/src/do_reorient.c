/* Copyright 1995-2001 Roger P. Woods, M.D. */
/* Modified: 5/22/01 */

/*
 * This program is designed to allow arbitrary reorientation
 *  of the axes of a file.
 */


#include "AIR.h"

static void adjust(double **f, double **e)

{
	AIR_matmul(e,f,e);
	{
		unsigned int j;
		
		for(j=0;j<4;j++){
		
			unsigned int i;
			
			for(i=0;i<4;i++) f[j][i]=0.0;
			f[j][j]=1.0;
		}
	}
}
	
AIR_Error AIR_do_reorient(const char *program, const char *input, const char *output, const unsigned int n, const unsigned int *transforms, const char *airfile, const AIR_Boolean ow)

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
	if(strlen(input)>=(size_t)AIR_CONFIG_MAX_PATH_LENGTH){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("File name %s is too long to incorporate into a .air file\n",input);
		return(AIR_PATH_TOO_LONG_ERROR);
	}
	if(strlen(output)>=(size_t)AIR_CONFIG_MAX_PATH_LENGTH){
		printf("%s: %d: ",__FILE__,__LINE__);
		printf("File name %s is too long to incorporate into a .air file\n",output);
		return(AIR_PATH_TOO_LONG_ERROR);
	}
	{
		AIR_Pixels 		***datain;
		struct AIR_Air16		air1;
		
		double			*e[4];
		double			*f[4];
		double			f0[16];


		{
			signed int mirror=1;

			{
				unsigned int j;
				
				for(j=0;j<4;j++){
					e[j]=air1.e[j];
					f[j]=f0+4*j;
					{
						unsigned int i;
						
						for(i=0;i<4;i++){
							e[j][i]=0.0;
							f[j][i]=0.0;
						}
					}
					e[j][j]=1.0;
					f[j][j]=1.0;
				}
			}

			strcpy(air1.r_file,input);
			strcpy(air1.s_file,output);

			
			{
				AIR_Error errcode;

				datain=AIR_load(air1.r_file, &air1.r, TRUE, &errcode);
				if(!datain){
					return(errcode);
				}
			}
			air1.s=air1.r;
			
			{
				unsigned int k;
				
				for(k=0;k<n;k++){
			
					switch(transforms[k]){
					
						case 1:	/* 'x' */
						
							f[0][0]=-1.0;
							f[3][0]=air1.s.x_dim-1;
							adjust(f,e);
							{
								AIR_Error errcode;
								AIR_Pixels ***temp=AIR_flip_x(datain,&air1.s,&errcode);
								if(!temp){
									AIR_free_vol3(datain);
									return(errcode);
								}
								AIR_free_vol3(datain);
								datain=temp;
							}
							mirror*=-1;
							break;
							
						case 2: /* 'y' */
						
							f[1][1]=-1.0;
							f[3][1]=air1.s.y_dim-1;
							adjust(f,e);
							{
								AIR_Error errcode;
								AIR_Pixels ***temp=AIR_flip_y(datain,&air1.s,&errcode);
								if(!temp){
									AIR_free_vol3(datain);
									return(errcode);
								}
								AIR_free_vol3(datain);
								datain=temp;
							}
							mirror*=-1;
							break;
							
						case 3:	/* 'z' */
						
							f[2][2]=-1.0;
							f[3][2]=air1.s.z_dim-1;
							adjust(f,e);
							{
								AIR_Error errcode;
								AIR_Pixels ***temp=AIR_flip_z(datain,&air1.s,&errcode);
								if(!temp){
									AIR_free_vol3(datain);
									return(errcode);
								}
								AIR_free_vol3(datain);
								datain=temp;
							}
							mirror*=-1;
							break;
							
						case 4:	/* 'xx' */
						
							f[1][1]=-1.0;
							f[3][1]=air1.s.y_dim-1;
							adjust(f,e);
							{
								AIR_Error errcode;
								AIR_Pixels ***temp=AIR_flip_y(datain,&air1.s,&errcode);
								if(!temp){
									AIR_free_vol3(datain);
									return(errcode);
								}
								AIR_free_vol3(datain);
								datain=temp;
							}
							f[2][2]=-1.0;
							f[3][2]=air1.s.z_dim-1;
							adjust(f,e);
							{
								AIR_Error errcode;
								AIR_Pixels ***temp=AIR_flip_z(datain,&air1.s,&errcode);
								if(!temp){
									AIR_free_vol3(datain);
									return(errcode);
								}
								AIR_free_vol3(datain);
								datain=temp;
							}
							break;
							
						case 5:	/* 'xy' */
						
							f[0][0]=-1.0;
							f[3][0]=air1.s.x_dim-1;
							adjust(f,e);
							{
								AIR_Error errcode;
								AIR_Pixels ***temp=AIR_flip_x(datain,&air1.s,&errcode);
								if(!temp){
									AIR_free_vol3(datain);
									return(errcode);
								}
								AIR_free_vol3(datain);
								datain=temp;
							}
							f[0][0]=0.0;
							f[1][0]=1.0;
							f[0][1]=1.0;
							f[1][1]=0.0;
							adjust(f,e);
							{
								AIR_Error errcode;
								AIR_Pixels ***temp=AIR_swap_x_and_y(datain,&air1.s,&air1.s,&errcode);
								if(!temp){
									AIR_free_vol3(datain);
									return(errcode);
								}
								AIR_free_vol3(datain);
								datain=temp;
							}
							break;
							
						case 6:	/* 'xz' */
						
							f[0][0]=-1.0;
							f[3][0]=air1.s.x_dim-1;
							adjust(f,e);
							{
								AIR_Error errcode;
								AIR_Pixels ***temp=AIR_flip_x(datain,&air1.s,&errcode);
								if(!temp){
									AIR_free_vol3(datain);
									return(errcode);
								}
								AIR_free_vol3(datain);
								datain=temp;
							}
							f[0][0]=0.0;
							f[2][0]=1.0;
							f[0][2]=1.0;
							f[2][2]=0.0;
							adjust(f,e);
							{
								AIR_Error errcode;
								AIR_Pixels ***temp=AIR_swap_x_and_z(datain,&air1.s,&air1.s,&errcode);
								if(!temp){
									AIR_free_vol3(datain);
									return(errcode);
								}
								AIR_free_vol3(datain);
								datain=temp;
							}
							break;
							
						case 7:	/* 'yx' */
						
							f[1][1]=-1.0;
							f[3][1]=air1.s.y_dim-1;
							adjust(f,e);
							{
								AIR_Error errcode;
								AIR_Pixels ***temp=AIR_flip_y(datain,&air1.s,&errcode);
								if(!temp){
									AIR_free_vol3(datain);
									return(errcode);
								}
								AIR_free_vol3(datain);
								datain=temp;
							}
							f[0][0]=0.0;
							f[1][0]=1.0;
							f[0][1]=1.0;
							f[1][1]=0.0;
							adjust(f,e);
							{
								AIR_Error errcode;
								AIR_Pixels ***temp=AIR_swap_x_and_y(datain,&air1.s,&air1.s,&errcode);
								if(!temp){
									AIR_free_vol3(datain);
									return(errcode);
								}
								AIR_free_vol3(datain);
								datain=temp;
							}
							break;
							
						case 8:	/* 'yy' */
						
							f[0][0]=-1.0;
							f[3][0]=air1.s.x_dim-1;
							adjust(f,e);
							{
								AIR_Error errcode;
								AIR_Pixels ***temp=AIR_flip_x(datain,&air1.s,&errcode);
								if(!temp){
									AIR_free_vol3(datain);
									return(errcode);
								}
								AIR_free_vol3(datain);
								datain=temp;
							}
							f[2][2]=-1.0;
							f[3][2]=air1.s.z_dim-1;
							adjust(f,e);
							{
								AIR_Error errcode;
								AIR_Pixels ***temp=AIR_flip_z(datain,&air1.s,&errcode);
								if(!temp){
									AIR_free_vol3(datain);
									return(errcode);
								}
								AIR_free_vol3(datain);
								datain=temp;
							}
							break;
							
						case 9:	/* 'yz' */
						
							f[1][1]=-1.0;
							f[3][1]=air1.s.y_dim-1;
							adjust(f,e);
							{
								AIR_Error errcode;
								AIR_Pixels ***temp=AIR_flip_y(datain,&air1.s,&errcode);
								if(!temp){
									AIR_free_vol3(datain);
									return(errcode);
								}
								AIR_free_vol3(datain);
								datain=temp;
							}
							f[1][1]=0.0;
							f[2][1]=1.0;
							f[1][2]=1.0;
							f[2][2]=0.0;
							adjust(f,e);
							{
								AIR_Error errcode;
								AIR_Pixels ***temp=AIR_swap_y_and_z(datain,&air1.s,&air1.s,&errcode);
								if(!temp){
									AIR_free_vol3(datain);
									return(errcode);
								}
								AIR_free_vol3(datain);
								datain=temp;
							}
							break;
							
						case 10:	/* 'zx' */
						
							f[2][2]=-1.0;
							f[3][2]=air1.s.z_dim-1;
							adjust(f,e);
							{
								AIR_Error errcode;
								AIR_Pixels ***temp=AIR_flip_z(datain,&air1.s,&errcode);
								if(!temp){
									AIR_free_vol3(datain);
									return(errcode);
								}
								AIR_free_vol3(datain);
								datain=temp;
							}
							f[0][0]=0.0;
							f[2][0]=1.0;
							f[0][2]=1.0;
							f[2][2]=0.0;
							adjust(f,e);
							{
								AIR_Error errcode;
								AIR_Pixels ***temp=AIR_swap_x_and_z(datain,&air1.s,&air1.s,&errcode);
								if(!temp){
									AIR_free_vol3(datain);
									return(errcode);
								}
								AIR_free_vol3(datain);
								datain=temp;
							}
							break;
							
						case 11:	/* 'zy' */
						
							f[2][2]=-1.0;
							f[3][2]=air1.s.z_dim-1;
							adjust(f,e);
							{
								AIR_Error errcode;
								AIR_Pixels ***temp=AIR_flip_z(datain,&air1.s,&errcode);
								if(!temp){
									AIR_free_vol3(datain);
									return(errcode);
								}
								AIR_free_vol3(datain);
								datain=temp;
							}
							f[1][1]=0.0;
							f[2][1]=1.0;
							f[1][2]=1.0;
							f[2][2]=0.0;
							adjust(f,e);
							{
								AIR_Error errcode;
								AIR_Pixels ***temp=AIR_swap_y_and_z(datain,&air1.s,&air1.s,&errcode);
								if(!temp){
									AIR_free_vol3(datain);
									return(errcode);
								}
								AIR_free_vol3(datain);
								datain=temp;
							}
							break;
				
						case 12:	/* 'zz' */
						
							f[0][0]=-1.0;
							f[3][0]=air1.s.x_dim-1;
							adjust(f,e);
							{
								AIR_Error errcode;
								AIR_Pixels ***temp=AIR_flip_x(datain,&air1.s,&errcode);
								if(!temp){
									AIR_free_vol3(datain);
									return(errcode);
								}
								AIR_free_vol3(datain);
								datain=temp;
							}
							f[1][1]=-1.0;
							f[3][1]=air1.s.y_dim-1;
							adjust(f,e);
							{
								AIR_Error errcode;
								AIR_Pixels ***temp=AIR_flip_y(datain,&air1.s,&errcode);
								if(!temp){
									AIR_free_vol3(datain);
									return(errcode);
								}
								AIR_free_vol3(datain);
								datain=temp;
							}
							break;
							
						default:
						
							AIR_free_vol3(datain);
							return(AIR_USER_INTERFACE_ERROR);						
					}
				}
			}

			{
				AIR_Error errcode=AIR_save(datain,&air1.s,air1.s_file,ow,program);
				
				if(errcode!=0){
					AIR_free_vol3(datain);
					return(errcode);
				}
			}
			AIR_free_vol3(datain);

			if(airfile){
				double pixel_size_s=air1.s.x_size;
				if(air1.s.y_size<pixel_size_s) pixel_size_s=air1.s.y_size;
				if(air1.s.z_size<pixel_size_s) pixel_size_s=air1.s.z_size;

				{
					unsigned int i;
					
					for(i=0;i<4;i++){
						e[0][i]/=(air1.s.x_size/pixel_size_s);
						e[1][i]/=(air1.s.y_size/pixel_size_s);
						e[2][i]/=(air1.s.z_size/pixel_size_s);
					}
				}
				if(strlen(program)<(size_t)AIR_CONFIG_MAX_COMMENT_LENGTH){
					strcpy(air1.comment,program);
				}
				else{
					const char *shortname=AIR_file_from_path(program);
					strncpy(air1.comment,shortname,(size_t)AIR_CONFIG_MAX_COMMENT_LENGTH-1);
					air1.comment[AIR_CONFIG_MAX_COMMENT_LENGTH-1]='\0';
				}
				{
					AIR_Error errcode=AIR_write_air16(airfile,ow,e,TRUE,&air1);
					
					if(errcode!=0){
						return(errcode);
					}
				}
			}

			if(mirror<0){
				printf("%s is the 3D mirror image of %s\n",air1.s_file,air1.r_file);
			}
		}
	}
	return 0;
}
