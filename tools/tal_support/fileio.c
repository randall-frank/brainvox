/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski
 *
 * $Id: fileio.c 1934 2006-09-05 00:50:29Z rjfrank $
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
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <sys/stat.h>

#ifndef WIN32
#include <unistd.h>
#else
FILE *popen(char *a, char *b) {
	return((FILE *)NULL);
}
int pclose(FILE *f) {
	return(-1);
}
#endif

#include "proto.h"
#include "nifti1.h"
#include "roi_utils.h"
#include "tal_shm_interface.h"

#ifdef USE_ZLIB
#include "zlib/zlib.h"
#endif

/*
typedef struct {
        double  points[3][3];
        double  CA_front,CP_back,CA_CP;
        double  CA_top,CA_bottom;
        double  CA_right,CA_left;
        double  xaxis[3];
        double  yaxis[3];
        double  zaxis[3];
} tal_conv;
*/

/* local prototypes */
FILE *OpenFileR(char *file,long int *pop,long int bin);
FILE *OpenFileW(char *file,long int *pop,long int bin);
int CloseFile(FILE *fp,long int pop);
long int ASCII_io(char *file,char type,void *data,long int dx,long int dy,
		long int dz,long int flip);

/* Check for single file I/O */
int tal_img_is_file(char *file,long int *slice,char *shortname,
	long int *volhdr,long int *slicehdr,long int *flip);

/* brainvox connectivity */
int tal_shm_is_file(char *file);
int tal_shm_IO(char *file,char type,long int dx,long int dy,long int dz,
        unsigned char *data);
void tal_shm_cleanup(void);

/* voxnet connectivity */
int tal_voxnet_is_file(char *file);
int tal_voxnet_IO(char *file,char type,long int dx,long int dy,long int dz,
        unsigned char *data);
void tal_voxnet_cleanup(void);

/* globals */
static	double roi_zoom = 0.5;
static	long int roi_flip = 1;

/* routines to read/write images and datasets */

void tal_read(char *file,double ip,tal_conv *tal)
{
	FILE 		*fp;
	long int 	i,j;

	fp = fopen(file,"r");
	if (fp == 0L) ex_err("Unable to read conversion file.");
/* read the points */
	fscanf(fp,"%lf %lf %lf",&(tal->points[0][PT_CA]),
		&(tal->points[1][PT_CA]),&(tal->points[2][PT_CA]));
	fscanf(fp,"%lf %lf %lf",&(tal->points[0][PT_CP]),
		&(tal->points[1][PT_CP]),&(tal->points[2][PT_CP]));
	fscanf(fp,"%lf %lf %lf",&(tal->points[0][PT_PLANE]),
		&(tal->points[1][PT_PLANE]),&(tal->points[2][PT_PLANE]));
/* convert to pixels */
	for(i=0;i<3;i++) {
		for(j=0;j<3;j++) {
			tal->points[i][j] = tal->points[i][j] / ip;
		}	
	}
/* read the values */
	fscanf(fp,"%lf %lf %lf",&(tal->CA_front),&(tal->CP_back),&(tal->CA_CP));
	tal->CA_front = tal->CA_front/ip;
	tal->CP_back = tal->CP_back/ip;
	tal->CA_CP = tal->CA_CP/ip;
	fscanf(fp,"%lf %lf",&(tal->CA_top),&(tal->CA_bottom));
	tal->CA_top = tal->CA_top/ip;
	tal->CA_bottom = tal->CA_bottom/ip;
	fscanf(fp,"%lf %lf",&(tal->CA_right),&(tal->CA_left));
	tal->CA_right = tal->CA_right/ip;
	tal->CA_left = tal->CA_left/ip;
/* xaxis */
	fscanf(fp,"%lf %lf %lf",&(tal->xaxis[0]),&(tal->xaxis[1]),
		&(tal->xaxis[2]));
	fscanf(fp,"%lf %lf %lf",&(tal->yaxis[0]),&(tal->yaxis[1]),
		&(tal->yaxis[2]));
	fscanf(fp,"%lf %lf %lf",&(tal->zaxis[0]),&(tal->zaxis[1]),
		&(tal->zaxis[2]));
	fclose(fp);
	return;
}

void	bswap(unsigned char *data,long int dx,long int dy,long int dz)
{
	long int 	i;
	unsigned char 	t;

/* don't do it if it does not make sense (ie 8 bit) */
	if (dz == 2) {
		for(i=0;i<(dx*dy*dz);i=i+2) {
			t = data[i];
			data[i] = data[i+1];
			data[i+1] = t;
		}
	} else if (dz == 4) {
		for(i=0;i<(dx*dy*dz);i=i+4) {
			t = data[i];
			data[i] = data[i+3];
			data[i+3] = t;
			t = data[i+1];
			data[i+1] = data[i+2];
			data[i+2] = t;
		}
	}
	return;
}

void	yflip(unsigned char *data,long int dx,long int dy,long int dz)
{
	long int x,y;
	unsigned char *p1,*p2,t;

	for(y=0;y<(dy/2);y++) {
		p1 = data + (y*dx*dz);
		p2 = data + ((dy-y-1)*dx*dz);
		for(x=0;x<(dx*dz);x++) {
			t = *p2;
			*p2 = *p1;
			*p1 = t;
			p1++; p2++;
		}
	}
	return;
}

void	xflip(unsigned char *data,long int dx,long int dy,long int dz)
{
	long int x,y,i;
	unsigned char *p1,*p2,t;

	for(y=0;y<dy;y++) {
		p1 = data + (y*dx*dz);
		p2 = p1 + ((dx-1)*dz);
		for(x=0;x<dx/2;x++) {
			for(i=0;i<dz;i++) {
				t = p2[i];
				p2[i] = p1[i];
				p1[i] = t;
			}
			p1+=dz; p2-=dz;
		}
	}
	return;
}

void bin_roi_flip(long int xflip) 
{
	roi_flip = xflip;
	return;
}

void bin_roi_scale(double scale) 
{
	roi_zoom = scale;
	return;
}

#define M_PTS 20000


long int bin_io(char *file,char type,void *data,long int dx,long int dy,
		long int dz,long int inflip,long int swab,long int header)
{
	FILE 		*fp;
	unsigned char 	*ptr,*tmp;
	long int 	i,j,y,pop;
	double		val;
	char		er_str[80];
	long int	st,en;
	long int	npts,strips[M_PTS];
	Trace		roi;

/* we allow these to be overridden by templates */
	long int	volhdr = header;
	long int	flip = inflip;

/* try Analyze I/O */
        i = NIFTI_io(file,type,data,dx,dy,dz,flip);
        if (i != -1) return(i);

/* try ASCII I/O */
	i = ASCII_io(file,type,data,dx,dy,dz,flip);
	if (i != -1) return(i);

/* binary I/O */
	if (type == 'r') {
/* NULL file */
		if (strcmp(file,"-") == 0) {
			ptr = (unsigned char *)data;
			for(i=0;i<(dx*dy*dz);i++) *ptr++ = 0;
			return(0);
/* fixed value file */
		} else if (file[0] == '=') {
			val = 0;
			val = atof(&(file[1]));
			switch(dz) {
				case 1:
					if (val < 0) val = 0;
					if (val > 255) val = 255;
					for(i=0;i<(dx*dy);i++) {
						((unsigned char *)data)[i]=val;
					}
					break;
				case 2:
					if (val < 0) val = 0;
					if (val > 65535) val = 65535;
					for(i=0;i<(dx*dy);i++) {
						((unsigned short *)data)[i]=val;
					}
					break;
				case 4:
					for(i=0;i<(dx*dy);i++) {
						((float *)data)[i]=val;
					}
					break;
			}
			return(0);
/* ROI file */
		} else if (strstr(file,".roi") != 0L) {
/* fill with 0 */
			switch(dz) {
				case 1:
					for(i=0;i<(dx*dy);i++) {
						((unsigned char *)data)[i]=0;
					}
					break;
				case 2:
					for(i=0;i<(dx*dy);i++) {
						((unsigned short *)data)[i]=0;
					}
					break;
				case 4:
					for(i=0;i<(dx*dy);i++) {
						((float *)data)[i]=0;
					}
					break;
			}
/* read the ROI */
			init_roi_utils();
			if (read_roi(file,&roi)) return(1); /* error */
/* scale */
			scale_roi(&roi,roi_zoom);
/* paint the roi */
			npts = M_PTS;
			roi_2_strips(&roi,strips,&npts);
			for(i=0;i<npts;i+=3) {
				st = strips[i];
				en = strips[i+1];
				if (st < 0) st = 0;
				if (en > dx-1) en = dx-1;

				y = strips[i+2];
				if (roi_flip) y = (dy-1) - y;
				if (y < 0) continue;
				if (y >= dy) continue;
/* fill the scan line with 1 */
				switch(dz) {
					case 1:
					for(j=(y*dx)+st;j<=(y*dx)+en;j++) {
						((unsigned char *)data)[j] = 1;
					}
					break;
					case 2:
					for(j=(y*dx)+st;j<=(y*dx)+en;j++) {
						((unsigned short *)data)[j] = 1;
					}
					break;
					case 4:
					for(j=(y*dx)+st;j<=(y*dx)+en;j++) {
						((float *)data)[j] = 1;
					}
					break;
				}
			}
			if (flip) yflip(data,dx,dy,dz);
			return(0);
		}

/* normal raw binary file */
		if (tal_shm_is_file(file)) {
			if (tal_shm_IO(file,'r',dx,dy,dz,data)) {
		ex_err("Unable to open Brainvox shared memory for reading.");
				return(1);
			}
		} else if (tal_voxnet_is_file(file)) {
			if (tal_voxnet_IO(file,'r',dx,dy,dz,data)) {
		ex_err("Unable to connect to voxnet server for reading.");
				return(1);
			}
		} else if (tal_shmblk_is_file(file)) {
			if (tal_shmblk_IO(file,'r',dx,dy,dz,data)) {
		ex_err("Unable to connect to shared memory block for reading.");
				return(1);
			}
		} else {
			long int	slice,slicehdr = 0;
			char		fileout[1024];

/* seek the slice start for a single file volume */
 			if (tal_img_is_file(file,&slice,fileout,
			    &volhdr,&slicehdr,&flip)) {
				fp = OpenFileR(fileout,&pop,2);
				if (fp == 0L) {
	ex_err("Unable to open binary file for reading.");
					return(1);
				}
				if (pop != 0) {
	ex_err("Compression not supported w/single file volumes.");
					return(1);
				}
				fseek(fp,(slice-1)*(dx*dy*dz+slicehdr)+volhdr,
					SEEK_SET);
			} else {
				fp = OpenFileR(file,&pop,1);
				if (fp == 0L) {
	ex_err("Unable to open binary file for reading.");
					return(1);
				}
/* read the header (if any) */
                		if (volhdr != 0L) {
                        		tmp = malloc(volhdr);
                        		if (tmp == 0L) {
                                		CloseFile(fp,pop);
        ex_err("Unable to allocate memory for image header I/O.");
						return(1);
                        		}
#ifdef USE_ZLIB
					if (pop == 2) {
						gzread((gzFile)fp,tmp,volhdr);
					} else 
#endif
					{
						fread(tmp,1,volhdr,fp);
					}
                        		free(tmp);
                		}
			}
/* read the image data */
#ifdef USE_ZLIB
			if (pop == 2) {
				gzread((gzFile)fp,data,(dx*dy*dz));
			} else 
#endif
			{
				fread(data,(dx*dy*dz),1,fp);
			}
			CloseFile(fp,pop);
		}

		if (flip) yflip(data,dx,dy,dz);
		if (swab) bswap(data,dx,dy,dz);

	} else if (type == 'w') {
/* we do not write NULL files */
		if (strcmp(file,"-") == 0) return(0);

/* check to see if the template specifies flipping */
		(void)tal_img_is_file(file,NULL,NULL,NULL,NULL,&flip);

/* cannot flip and swap in place as it damages the input, so we buffer */
		if ((flip) || (swab)) {
			tmp = malloc(dx*dy*dz);
			if (tmp == 0) {
			ex_err("Unable to allocate memory for image buffer.");
				return(1);
			}
			memcpy(tmp,data,dx*dy*dz);

			if (flip) yflip(tmp,dx,dy,dz);
			if (swab) bswap(tmp,dx,dy,dz);

		} else {
			tmp = data;
		}

		if (tal_shm_is_file(file)) {
			if (tal_shm_IO(file,'w',dx,dy,dz,tmp)) {
		ex_err("Unable to open Brainvox shared memory for writing.");
				return(1);
			}
		} else if (tal_voxnet_is_file(file)) {
			if (tal_voxnet_IO(file,'w',dx,dy,dz,tmp)) {
		ex_err("Unable to connect to voxnet server for writing.");
				return(1);
			}
		} else if (tal_shmblk_is_file(file)) {
			if (tal_shmblk_IO(file,'w',dx,dy,dz,tmp)) {
		ex_err("Unable to connect to shared memory block for writing.");
				return(1);
			}
		} else {
			long int	slice,slicehdr = 0;
			char		fileout[1024];

			if (tal_img_is_file(file,&slice,fileout,
			    &volhdr,&slicehdr,&flip)) {
				fp = OpenFileW(fileout,&pop,2);
				if (fp == 0L) {
					if ((flip) || (swab)) free(tmp);
	ex_err("Unable to open binary file for writing.");
					return(1);
				}
				if (pop != 0) {
					if ((flip) || (swab)) free(tmp);
	ex_err("Compression not supported w/single file volumes.");
					return(1);
				}
				fseek(fp,(slice-1)*(dx*dy*dz+slicehdr)+volhdr,
					SEEK_SET);
			} else {
				fp = OpenFileW(file,&pop,1);
				if (fp == 0L) {
					if ((flip) || (swab)) free(tmp);
	ex_err("Unable to open binary file for writing.");
					return(1);
				}
			}
#ifdef USE_ZLIB
			if (pop == 2) {
				gzwrite((gzFile)fp,tmp,(dx*dy*dz));
			} else 
#endif
			{
				fwrite(tmp,(dx*dy*dz),1,fp);
			}
			CloseFile(fp,pop);
		}
		if ((flip) || (swab)) free(tmp);

	} else {
		sprintf(er_str,"Invalid bin_io type:%c",type);
		ex_err(er_str);
		return(1);
	}
	return(0);
}

FILE *OpenFileR(char *file,long int *pop,long int bin)
{
	FILE	*fp;
	char	zcmd[1024];
	struct stat myfbuf;
	char	how[3];

	if (bin != 0) {
		strcpy(how,"rb");
	} else {
		strcpy(how,"r");
	}

	if ((file[strlen(file)-2] == '.') && (file[strlen(file)-1] == 'Z')) {
		*pop = 1;

		if (stat(file,&myfbuf) != 0) return(0);

		sprintf(zcmd,"zcat %s",file);
                fp = popen(zcmd,"r");
	} else if ((file[strlen(file)-3] == '.') && 
			(file[strlen(file)-2] == 'g') &&
			(file[strlen(file)-1] == 'z')) {

		if (stat(file,&myfbuf) != 0) return(0);
#ifdef USE_ZLIB
		if (bin) {
			*pop = 2;
			fp = (FILE *)gzopen(file,how);
		} else 
#endif
		{
			*pop = 1;
			sprintf(zcmd,"gzip -dc %s",file);
                	fp = popen(zcmd,"r");
		}
	} else {
		*pop = 0;
		fp = fopen(file,how);
	}
	return(fp);
}

FILE *OpenFileW(char *file,long int *pop,long int bin)
{
	FILE		*fp;
	char		zcmd[1024];
	char		how[3];
	struct stat	myfbuf;

	if (bin == 1) {
		strcpy(how,"wb");
	} else if (bin == 2) {
		if (stat(file,&myfbuf) != 0) {
			strcpy(how,"wb");  /* create a new file */
		} else {
			strcpy(how,"r+b"); /* modify an existing file */
		}
	} else {
		strcpy(how,"w");
	}

	if ((file[strlen(file)-2] == '.') && (file[strlen(file)-1] == 'Z')) {
		*pop = 1;
		sprintf(zcmd,"compress -c  - > %s",file);
                fp = popen(zcmd,"w");
	} else if ((file[strlen(file)-3] == '.') && 
			(file[strlen(file)-2] == 'g') &&
			(file[strlen(file)-1] == 'z')) {
#ifdef USE_ZLIB
		if (bin) {
			*pop = 2;
			fp = (FILE *)gzopen(file,how);
		} else
#endif
		{
			*pop = 1;
			sprintf(zcmd,"gzip > %s",file);
                	fp = popen(zcmd,"w");
		}
	} else {
		*pop = 0;
		fp = fopen(file,how);
	}
	return(fp);
}

int CloseFile(FILE *fp,long int pop)
{
	int	t;

	if (pop == 1) {
		t=pclose(fp);
#ifdef USE_ZLIB
	} else if (pop == 2) {
		t=gzclose((gzFile)fp);
#endif
	} else {
		t=fclose(fp);
	}
	return(t);
}

long int ASCII_io(char *file,char type,void *data,long int dx,long int dy,
		long int dz,long int flip)
{
	FILE		*fp;
	char		name[1024];
	long int	pop;
	long int	i,j,k;
	float		*f_d = (float *)data;
	short		*f_s = (short *)data;
	unsigned char	*f_c = (unsigned char *)data;

/* check filename */
	if (strstr(file,"=t") == 0) return(-1);
	strcpy(name,file);
/* chop it off */
	*(strstr(name,"=t")) = '\0';
	if (strlen(name) < 1) return(-1);

/* read or write */
	if (type == 'r') {
		fp = OpenFileR(name,&pop,0);
		if (fp == 0L) {
			ex_err("Unable to open text file for reading.");
			return(1);
		}
		switch(dz) {
			case 1:
				for(i=0;i<dx*dy;i++) {
					fscanf(fp,"%ld",&j);	
					f_c[i] = j;
				}
				break;
			case 2:
				for(i=0;i<dx*dy;i++) {
					fscanf(fp,"%ld",&j);	
					f_s[i] = j;
				}
				break;
			case 4:
				for(i=0;i<dx*dy;i++) fscanf(fp,"%f",&(f_d[i]));	
				break;
		}
		CloseFile(fp,pop);
		if (flip) yflip(data,dx,dy,dz);
		return(0);

	} else if (type == 'w') {
		fp = OpenFileW(name,&pop,0);
		if (fp == 0L) {
			ex_err("Unable to open text file for writing.");
			return(1);
		}
		if (flip) yflip(data,dx,dy,dz);
		switch(dz) {
			case 1:
				k = 0;
				for(i=0;i<dx*dy;i++) {
					j = f_c[i];
					fprintf(fp,"%ld ",j);	
					k++;
					if (k == dx) {
						k = 0;
						fprintf(fp,"\n");
					}
				}
				break;
			case 2:
				k = 0;
				for(i=0;i<dx*dy;i++) {
					j = f_s[i];
					fprintf(fp,"%ld ",j);	
					k++;
					if (k == dx) {
						k = 0;
						fprintf(fp,"\n");
					}
				}
				break;
			case 4:
				k = 0;
				for(i=0;i<dx*dy;i++) {
					fprintf(fp,"%f ",f_d[i]);	
					k++;
					if (k == dx) {
						k = 0;
						fprintf(fp,"\n");
					}
				}
				break;
		}
		if (flip) yflip(data,dx,dy,dz);
		CloseFile(fp,pop);
		return(0);
	} 

/* unknown error */
	return(-1);
}

void img_interpolate(void  *st,void *en,void *out,long int dx,long int dy,
	long int dz,long int n)
{
        long int i,dxy;
        long int j,ptr;
        double wstart,wend,wadd;
	unsigned char	*bstart,*bend,*bout;
	unsigned short	*sstart,*send,*sout;
	float 		*fstart,*fend,*fout;

/* weighting to add with each slice */
        wadd = 1.0/(n+1.0);
        wend = wadd;
        wstart = 1.0 - wend;
        dxy = dx*dy;
        ptr = 0;
/* setup the pointers (casting) */
	bout = (unsigned char *)out;
	bstart = (unsigned char *)st;
	bend = (unsigned char *)en;
	sout = (unsigned short *)out;
	sstart = (unsigned short *)st;
	send = (unsigned short *)en;
	fout = (float *)out;
	fstart = (float *)st;
	fend = (float *)en;

        for(j=0; j<n; j++) {
/* for each slice */
		if (dz == 1) {
/* bytes */
                	for(i=0;i<dxy;i++) {
                       		bout[ptr++] = bstart[i]*wstart + bend[i]*wend;
                	}
		} else if (dz == 2) {
/* shorts */
                	for(i=0;i<dxy;i++) {
                       		sout[ptr++] = sstart[i]*wstart + send[i]*wend;
                	}
		} else if (dz == 4) {
/* floats */
                	for(i=0;i<dxy;i++) {
                       		fout[ptr++] = fstart[i]*wstart + fend[i]*wend;
                	}
		}
                wend = wend + wadd;
                wstart = 1.0 - wend;
        };
	return;
}

void    size_bin_file_(char *file,long int *bytes,long int *err)
{
        FILE    *fp;
        int     comp;
        char    zcmd[1024];
        struct stat myfbuf;
        long int        j;

        *err = 1;
        if ((file[strlen(file)-2] == '.') && (file[strlen(file)-1] == 'Z')) {
                comp = 1;
        } else {
                comp = 0;
        }
        j = 0;
        if (comp) {
                if (stat(file,&myfbuf) != 0) return;

                sprintf(zcmd,"zcat %s",file);
                fp = popen(zcmd,"r");
                if (fp != 0) {
                        while (fgetc(fp) != EOF) j++;
                        pclose(fp);
                } else {
                        return;
                }
        } else {
/* get some stats */
                if (stat(file,&myfbuf) == 0) {
                        j = myfbuf.st_size;
                } else {
                        return;
                }
        }
        *bytes = j;
        *err = 0;
        return;
}

long int read_pset(char *file,Pset *set)
{
	FILE		*fp;
	char		tstr[256],*st;
	int	n,i;

	fp = fopen(file,"r");
	if (fp == 0) return(1);

	fgets(tstr,255,fp);
	set->n = 0;
	sscanf(tstr,"%d",&(n));
	strcpy(set->title,":Untitled");
	st=strchr(tstr,':');
	if (st != 0) {
		strcpy(set->title,st);
	}
	st=strchr(set->title,'\n');
	if (st != 0) *st = '\0';

	for(i=0;i<n;i++) {
		if (fgets(tstr,255,fp)) {
			sscanf(tstr,"%ld %ld %ld %ld",&(set->x[set->n]),
				&(set->y[set->n]),&(set->z[set->n]),
				&(set->c[set->n]));
			set->n += 1;
		}
	}

	fclose(fp);

	return(0);
}

long int write_pset(char *file,Pset *set)
{
	FILE		*fp;
	long int	i;

	fp = fopen(file,"w");
	if (fp == 0) return(1);

	fprintf(fp,"%ld%s\n",set->n,set->title);

	for(i=0;i<set->n;i++) {
		fprintf(fp,"%ld %ld %ld %ld\n",set->x[i],set->y[i],
			set->z[i],set->c[i]);
	}

	fclose(fp);

	return(0);
}

void get_xyz(long int *xyz,char *str)
{
        char    t[256];
        char    *p;

        strcpy(t,str);

        p = t;
        sscanf(p,"%ld",&(xyz[0]));

        p = strchr(p,':');
        if (p) {
                p++;
                if (*p == '\0') return;
	} else {
		return;
        }
        sscanf(p,"%ld",&(xyz[1]));

        p = strchr(p,':');
        if (p) {
                p++;
                if (*p == '\0') return;
	} else {
		return;
        }
        sscanf(p,"%ld",&(xyz[2]));

        return;
}

void get_xy(long int *xy,char *str)
{
        char    t[256];
        char    *p;

        strcpy(t,str);

        p = t;
        sscanf(p,"%ld",&(xy[0]));

        p = strchr(p,':');
        if (p) {
                p++;
                if (*p == '\0') return;
	} else {
		return;
        }
        sscanf(p,"%ld",&(xy[1]));

        return;
}

void get_d_xy(double *xy,char *str)
{
        char    t[256];
        char    *p;

        strcpy(t,str);

        p = t;
        sscanf(p,"%lf",&(xy[0]));
	xy[1] = xy[0];

        p = strchr(p,':');
        if (p) {
                p++;
                if (*p == '\0') return;
	} else {
		return;
        }
        sscanf(p,"%lf",&(xy[1]));

	return;
}

void get_d_xyz(double *xyz,char *str)
{
        char    t[256];
        char    *p;

        strcpy(t,str);

        p = t;
        sscanf(p,"%lf",&(xyz[0]));
	xyz[1] = xyz[0];
	xyz[2] = xyz[0];

        p = strchr(p,':');
        if (p) {
                p++;
                if (*p == '\0') return;
	} else {
		return;
        }
        sscanf(p,"%lf",&(xyz[1]));
	xyz[2] = xyz[1];

        p = strchr(p,':');
        if (p) {
                p++;
                if (*p == '\0') return;
	} else {
		return;
        }
        sscanf(p,"%lf",&(xyz[2]));

        return;
}


/* optional support for a direct connection to the brainvox shared memory 
	system */
#ifndef BV_SHMEM_SUPPORT
/* other functions linked in through the brainvox libs */
int tal_shm_is_file(char *file) {
	return(0);
}
int tal_shm_IO(char *file,char type,long int dx,long int dy,long int dz,
        unsigned char *data) {
	return(1);
}
#endif

/* optional support for a voxnet connection to a voxblast server system */
#ifndef VOXNET_SUPPORT
/* other functions linked in through the voxnet libs */
int tal_voxnet_is_file(char *file) {
	return(0);
}
int tal_voxnet_IO(char *file,char type,long int dx,long int dy,long int dz,
        unsigned char *data) {
	return(1);
}
#endif

/* the exit function with optional cleanup code */
void tal_exit(int i) 
{
#ifdef BV_SHMEM_SUPPORT
	tal_shm_cleanup();
#endif
#ifdef VOXNET_SUPPORT
	tal_voxnet_cleanup();
#endif
	tal_shmblk_cleanup();
	NIFTI_cleanup();

	exit(i);
}

/* 
 * Check to see if the file has the proper convention:
 * "xxxxxxx:0_%%%{_volhdr{_slicehdr}}"
 */
int tal_img_is_file(char *file,long int *slice,char *shortname,
	long int *volhdr,long int *slicehdr,long int *flip)
{
	char	*p;

	p = strstr(file,":0_");
	if (!p) return(0);
	if (p == file) return(0);
	if (strlen(p) < 4) return(0);

	if (slice) *slice = atoi(p+3);

	if (shortname) {
		long int	len = strlen(file) - strlen(p);
		memcpy(shortname,file,len);
		shortname[len] = '\0';
	}

/* bump past the ':0_' */
	p += 3;

/* check for '_f' flip flag */
	if (flip) {
		if (strstr(p,"_f")) *flip = 1;
	}

	p = strchr(p,'_');
	if (p) {
		p++;
		if (volhdr) {
			*volhdr = atoi(p);
		}
		p = strchr(p,'_');
		if (p) {
			p++;
			if (slicehdr) {
				*slicehdr = atoi(p);
			}
		}
	}

	return(1);
}
