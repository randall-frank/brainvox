#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <sys/stat.h>
#include "proto.h"
#include "roi_utils.h"

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
FILE *OpenFileR(char *file,long int *pop);
FILE *OpenFileW(char *file,long int *pop);
int CloseFile(FILE *fp,long int pop);
void	bswap(unsigned char *data,long int dx,long int dy,long int dz);
void	yflip(unsigned char *data,long int dx,long int dy,long int dz);
void	xflip(unsigned char *data,long int dx,long int dy,long int dz);

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
		long int dz,long int flip,long int swab,long int header)
{
	FILE 		*fp;
	unsigned char 	*ptr,*tmp;
	long int 	i,j,y,pop;
	double		val;
	char		er_str[80];
	long int	st,en;
	int		npts,strips[M_PTS];
	Trace		roi;

	if (type == 'r') {
		if (strcmp(file,"-") == 0) {
			ptr = (unsigned char *)data;
			for(i=0;i<(dx*dy*dz);i++) *ptr++ = 0;
			return(0);
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

		fp = OpenFileR(file,&pop);
		if (fp == 0L) {
			ex_err("Unable to open binary file for reading.");
			return(1);
		}
                if (header != 0L) {
                        tmp = malloc(header);
                        if (tmp == 0L) {
                                CloseFile(fp,pop);
                                ex_err("Unable to allocate memory for image header I/O.");
				return(1);
                        }
                        fread(tmp,1,header,fp);
                        free(tmp);
                }
		fread(data,(dx*dy*dz),1,fp);
		CloseFile(fp,pop);

		if (flip) yflip(data,dx,dy,dz);
		if (swab) bswap(data,dx,dy,dz);

	} else if (type == 'w') {
		if (strcmp(file,"-") == 0) return(0);

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


		fp = OpenFileW(file,&pop);
		if (fp == 0L) {
			if ((flip) || (swab)) free(tmp);
			ex_err("Unable to open binary file for writing.");
			return(1);
		}
		fwrite(tmp,(dx*dy*dz),1,fp);
		CloseFile(fp,pop);
		if ((flip) || (swab)) free(tmp);

	} else {
		sprintf(er_str,"Invalid bin_io type:%c",type);
		ex_err(er_str);
		return(1);
	}
	return(0);
}

FILE *OpenFileR(char *file,long int *pop)
{
	FILE	*fp;
	char	zcmd[1024];
	struct stat myfbuf;

	if ((file[strlen(file)-2] == '.') && (file[strlen(file)-1] == 'Z')) {
		*pop = 1;

		if (stat(file,&myfbuf) != 0) return(0);

		sprintf(zcmd,"zcat %s",file);
                fp = popen(zcmd,"rb");
	} else if ((file[strlen(file)-3] == '.') && 
			(file[strlen(file)-2] == 'g') &&
			(file[strlen(file)-1] == 'z')) {
		*pop = 1;

		if (stat(file,&myfbuf) != 0) return(0);

		sprintf(zcmd,"gzip -dc %s",file);
                fp = popen(zcmd,"rb");
	} else {
		*pop = 0;
		fp = fopen(file,"rb");
	}
	return(fp);
}

FILE *OpenFileW(char *file,long int *pop)
{
	FILE	*fp;
	char	zcmd[1024];

	if ((file[strlen(file)-2] == '.') && (file[strlen(file)-1] == 'Z')) {
		*pop = 1;
		sprintf(zcmd,"compress -c  - > %s",file);
                fp = popen(zcmd,"wb");
	} else if ((file[strlen(file)-3] == '.') && 
			(file[strlen(file)-2] == 'g') &&
			(file[strlen(file)-1] == 'z')) {
		*pop = 1;
		sprintf(zcmd,"gzip > %s",file);
                fp = popen(zcmd,"wb");
	} else {
		*pop = 0;
		fp = fopen(file,"wb");
	}
	return(fp);
}

int CloseFile(FILE *fp,long int pop)
{
	int	t;

	if (pop == 1) {
		t=pclose(fp);
	} else {
		t=fclose(fp);
	}
	return(t);
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
                fp = popen(zcmd,"rb");
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
	long int	n,i;

	fp = fopen(file,"r");
	if (fp == 0) return(1);

	fgets(tstr,255,fp);
	set->n = 0;
	sscanf(tstr,"%ld",&(n));
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
        if (p != 0) {
                p++;
                if (*p == '\0') return;
        }
        sscanf(p,"%ld",&(xyz[1]));

        p = strchr(p,':');
        if (p != 0) {
                p++;
                if (*p == '\0') return;
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
        if (p != 0) {
                p++;
                if (*p == '\0') return;
        }
        sscanf(p,"%ld",&(xy[1]));

        return;
}

