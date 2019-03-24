/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski
 *
 * $Id: tal_pts2vol.c 1213 2005-08-27 20:51:21Z rjfrank $
 *                University of Iowa, Department of Neurology,
 *                Human Neuroanatomy and Neuroimaging Lab
 *                University of Iowa, Image Analysis Facility
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All modifications to the source code must be clearly marked as
 *    such.  Binary redistributions based on modified source code
 *    must be clearly marked as modified versions in the documentation
 *    and/or other materials provided with the distribution.
 * 4. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgment:
 *      This product includes software developed by Randall Frank, Thomas
 *      Grabowski and other unpaid contributors.
 * 5. The names of Randall Frank or Thomas Grabowski may not be used to
 *    endorse or promote products derived from this software without 
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY RANDALL FRANK AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL RANDALL FRANK OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

/* Tal_pts2vol converts a (brainvox) pointset to a volume and vice versa (using the -r option).
* The  intensity value for the value is identical to the value specified in the 4th column of the
* pointset except when the value is 0; then the intensity value is set to 14.  All coordinate
* points not specified by the pointset are set to 0.  Note that zscale = ceil(islice/ipixel).
* The conversion of the z column pointset to the z-slice value is zslice = 
* (z-pointset value/zscale) + 1.  When converting a volume to a points, the pointset can
* be limitted to a user-specified range of intensity values. 
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "proto.h"
#include "roi_utils.h"

#define MAX_PTS 5000
#define FALSE 0
#define TRUE 1


typedef struct {
long int	x;
long int	y;
long int	z;
long int	v;
long int	j;
} pointset;


void ex_err(char *s)
{
	fprintf(stderr,"Fatal error:%s\n",s);
	tal_exit(1);
}

void cmd_err(char *s)
{
	fprintf(stderr,"(%s:%s) Usage: %s [options] poinset volumetemp\n",__DATE__,TAL_VERSION,s);
	fprintf(stderr,"Options: -x(dx) image dx default:256\n");
	fprintf(stderr,"         -y(dy) image dy default:256\n");
	fprintf(stderr,"	 -z(dz) image dz default:1\n");
	fprintf(stderr,"         -f(start) first image number default:1\n");
	fprintf(stderr,"         -l(end) last image number default:124\n");
	fprintf(stderr,"         -i(step) image number step default:1\n");
	fprintf(stderr,"         -d(value) background value default:0\n");
	fprintf(stderr,"         -p(interpixel) interpixel spacing default:1.0\n");
	fprintf(stderr,"         -s(interslice) interslice spacing default:1.0\n");
	fprintf(stderr,"	 -r perform tranform from volume to pointset\n");
	fprintf(stderr,"	 -t lower threshold default:1.0\n");
	fprintf(stderr,"	 -u upper threshold default:255\n");
	fprintf(stderr,"	 -v(value) (0-13) default:2\n");
	fprintf(stderr,"	 -n use intensity value from the volume to be the pointset\n");
	fprintf(stderr,"Note that if the -n option is used, the volume should be 8bit.\n");
	fprintf(stderr,"The intensity range should be 1-13 for intensities to be converted to points.\n");
	tal_exit(1);
}
int read_pts(char *file, int zscale, int dx, pointset point[])
{
	FILE 		*fp;
	long int	i;
	long int	l;
	char		line[255];
	pointset 	temp;
	int		sorted = FALSE;
	long int	num;
	
	fp = fopen(file,"r");
	if (fp == 0L) ex_err("Unable to read point set file.");
	
	/* read the points */ 
	i = 0;
	while(fgets(line,255,fp) != 0L) {
		l = fscanf(fp,"%ld %ld %ld %ld",&point[i].x,&point[i].y,&point[i].z,&point[i].v);
		i++;
		}
	fclose(fp);
	num = i - 1;
	
	
	for (i=0;i<num;i++)
		{
		point[i].z=(point[i].z/zscale)+1;
		point[i].j=(point[i].y*dx) + point[i].x;
		if (point[i].v == 0) point[i].v = 14;
		}
	
	/* sort points */

	while (!sorted)
	{
		sorted=TRUE;
		for (i=0;i<(num-1);i++)
		{
		if (point[i].z > point[i+1].z)
		{
		sorted=FALSE;
		temp = point[i];
		point[i] = point[i+1];
		point[i+1] = temp;
		}
		}
	}
	
	return(num);
}

void write_pts(char *file, int num_pts, pointset point[])
{
	FILE 		*fp;
	int 		i;

	printf("Writing point set:%s\n",file);
	fp = fopen(file,"w");
	fprintf(fp,"	%3d :%s\n",num_pts,file);
	for (i=0;i<num_pts;i++)
	  {
	  fprintf(fp,"	%3ld 	%3ld 	%3ld 	%3ld\n",point[i].x,point[i].y,point[i].z,point[i].v);
	  }
	 fclose(fp);
}


	
int main(int argc,char **argv)
{
	long int	dx = 256;
	long int	dy = 256;
	long int	dz = 1;
	long int	kstart = 1;
	long int	kend = 124;
	long int	kstep = 1;
	double 		back = 0;	
	long int	swab = 0;
	long int	n_slices;
	char		outtemp[256];
	char		tstr[256],point_file[256];
	long int	i=0;
	long int	err,n,j;
	long int	k=0;
        unsigned char 	*data,*outputimage;
	double 		volume;
	long int	zscale,num_pts;
	double		islice = 1;
	double		ipixel = 1;
	int		first_pass = FALSE;
	long int	reverse = 0;
	double		lower=0;
	double		upper=255;
	long int	value=2;
	pointset	point[MAX_PTS];
	long int 	input = 0;	

	i = 1;
	while ((i < argc) && (argv[i][0] == '-')) { 	
		if (argv[i][1]=='\0') break;
      
		switch (argv[i][1]) 
		{
			case 'f':
				kstart = atoi(&(argv[i][2]));
				break;
			case 'l':
				kend = atoi(&(argv[i][2]));
				break;
			case 'i':
				kstep = atoi(&(argv[i][2]));
				break;
			case 'x':
				dx = atoi(&(argv[i][2]));
				break;
			case 'y':
				dy = atoi(&(argv[i][2]));
				break;
			case 'z':
				dz = atoi(&(argv[i][2]));
				break;	
			case 'p':
				ipixel = atof(&(argv[i][2]));
				break;
			case 'r':
				reverse = 1;
				break;
			case 's':
				islice = atof(&(argv[i][2]));
				break;
			case 't':
				lower = atof(&(argv[i][2]));
				break;
			case 'u':
				upper = atof(&(argv[i][2]));
				break;
			case 'v':
				value = atoi(&(argv[i][2]));
				break;	
			case 'n':
				input = 1;
				break;
			default:	
				cmd_err(argv[0]);
				break;
		}
		i++;
	}
	if ((argc-i) != 2) cmd_err(argv[0]);
	strcpy(point_file,argv[i]);
	strcpy(outtemp,argv[i+1]);
	
/* compute z scale factor */

	zscale = ceil(islice/ipixel);
	
if (reverse)	
{
/* read the images */

 /*get the image memory*/
   	data = malloc(dx*dy*dz);
   	if (data == 0L) ex_err("Unable to allocate volume memory.");
   	
   	i=0;
   
 for (k=kstart;k<=kend;k=k+kstep)
  {
  	
	name_changer(outtemp,k,&err,tstr);
	printf("Reading the file:%s\n",tstr);
	bin_io(tstr,'r',data,dx,dy,dz,1,swab,0L);   
	
	
	outputimage = malloc(dx*dy*dz);
	if (outputimage == 0L) ex_err("Unable to allocate image memory.");
	
	
	for (j=0;j<dx*dy;j++)
	{
	
	
   		if (dz == 2)
   			volume = ((unsigned short *)data)[j];
   		else if (dz == 1)
   			volume = ((unsigned char *)data)[j];
   		else if (dz == 4)
   			volume = ((float *)data)[j];
   		outputimage[j] = volume; 
		
		if ((outputimage[j] > lower)&&(outputimage[j] <= upper)&&(input==0))
		{
			point[i].y = (j/dx);
	 		point[i].x = j%dx;
			point[i].z = (k-1)*zscale;
			point[i].v = value;
			i++;  
		}
		else if ((outputimage[j] > lower)&&(outputimage[j] <= upper)&&(input==1))
		{
			point[i].y = (j/dx);
	 		point[i].x = j%dx;
			point[i].z = (k-1)*zscale;
			if (point[i].v < 13) {
			point[i].v = ((unsigned char *)outputimage)[j];
			}
			else {
			point[i].v = 13;
			}
			i++;  
		}
		


	}
   }  
   free(outputimage);
   free(data);
   num_pts=i;
  	
 write_pts(point_file,num_pts,point); 
 
 for(i=0;i<num_pts;i++)
 {
 }
}


if (!reverse)
{

 num_pts = read_pts(point_file, zscale, dx, point);
 
	for (k=kstart;k<=kend;k=k+kstep)
	{
	
	if (dz == 1)
		outputimage = calloc(dx*dy*1, sizeof(unsigned char));
	else if (dz == 2)
		outputimage = calloc(dx*dy*1, sizeof(unsigned short));
	else if (dz == 4)
		outputimage = calloc(dx*dy*1, sizeof(float));

	if (outputimage == 0L) ex_err("Unable to allocate image memory.");
	
	  for(i=0;i<num_pts;i++) 
     	  {
		if (point[i].z == k)
		{
		 for(j=0;j<(dx*dy);j++) 
                 {
	         if (point[i].j == j)
	       		{

   			
   			if (dz == 2)
   				((unsigned short *)outputimage)[j] = (unsigned short)point[i].v;
   			else if (dz == 1)
   				((unsigned char *)outputimage)[j] = (unsigned char)point[i].v;
   			else if (dz == 4)
   				((float *)outputimage)[j] = (float)point[i].v;
   			




			}
		 }	
		}
	   }
	   name_changer(outtemp,k,&err,tstr);
	   printf("Writing sampled file:%s\n",tstr);		
	   bin_io(tstr,'w',outputimage,dx,dy,dz,1,swab,0L);
	   free(outputimage);
	}
	}
	tal_exit(0);

	return(0);
}
