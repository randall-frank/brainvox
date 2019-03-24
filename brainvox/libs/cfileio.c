#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/stat.h>

#include "z_compress.h"
#include "zlib.h"

#ifdef WIN32
#define stat _stat
#endif

#ifdef CAP_NOUS
#define write_bin_file_ WRITE_BIN_FILE
#define read_bin_file_ READ_BIN_FILE
#define size_bin_file_ SIZE_BIN_FILE
#define cnvt_16_to_8_ CNVT_16_TO_8
#define cnvt_f_to_8_ CNVT_F_TO_8
#define pack_palette_ PACK_PALETTE
#define unpack_palette_ UNPACK_PALETTE
#define cpack_palette_ CPACK_PALETTE
#endif

void	write_bin_file_(char *file,void *data,long int *bytes,long int *err);
void	read_bin_file_(char *file,void *data,long int *bytes,long int *err);
void	size_bin_file_(char *file,long int *bytes,long int *err);
void	cnvt_16_to_8_(unsigned short *in,unsigned char *out,long int *dx,
		long int *dy,float *lims,long int *bswap);
void	cnvt_f_to_8_(float *in,unsigned char *out,long int *dx,
		long int *dy,float *lims,long int *bswap);

void	size_bin_file_(char *file,long int *bytes,long int *err)
{
	FILE	*fp;
	int	comp;
	char	zcmd[1024];
	struct stat myfbuf;
	long int	j;

	*err = 1;
	if ((file[strlen(file)-2] == '.') && (file[strlen(file)-1] == 'Z')) {
		comp = 1;
        } else if ((file[strlen(file)-3] == '.') &&
                (file[strlen(file)-2] == 'g') &&
                (file[strlen(file)-1] == 'z')) {
		comp = 2;
	} else {
		comp = 0;
	}
	j = 0;
	if (comp != 0) {
                if (stat(file,&myfbuf) != 0) return;
#ifdef POPEN_COMPRESSED_IO
		if (comp == 1) {
			sprintf(zcmd,"zcat %s",file);
		} else {
			sprintf(zcmd,"gzip -dc %s",file);
		}
                fp = popen(zcmd,"r");
		if (fp != 0) {
                	while (fgetc(fp) != EOF) j++;
                	pclose(fp);
                } else {
			return;
		}
#else
		if (comp == 1) {
			j = size_compress_file(file);
                } else {
			gzFile zf;
			zf = gzopen(file,"rb");
			if (zf) {
				while(gzgetc(zf) != -1) j++;
				gzclose(zf);
                        }
		}
#endif
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

void	read_bin_file_(char *file,void *data,long int *bytes,long int *err)
{
	FILE	*fp;
	int	comp;
	long int j;
	char	zcmd[1024];
	struct stat myfbuf;

	*err = 1;
	if ((file[strlen(file)-2] == '.') && (file[strlen(file)-1] == 'Z')) {
		comp = 1;
        } else if ((file[strlen(file)-3] == '.') &&
                (file[strlen(file)-2] == 'g') &&
                (file[strlen(file)-1] == 'z')) {
		comp = 2;
	} else {
		comp = 0;
	}
	if (comp != 0) {
/* check file existance */
                if (stat(file,&myfbuf) != 0) return;
#ifdef POPEN_COMPRESSED_IO
		if (comp == 1) {
			sprintf(zcmd,"zcat %s",file);
		} else {
			sprintf(zcmd,"gzip -dc %s",file);
		}
                fp = popen(zcmd,"r");
		if (fp != 0) {
			j = fread(data,1,*bytes,fp);
			pclose(fp);
		} else {
			return;
		}
#else
		if (comp == 1) {
			j = read_compress_file(file,*bytes,data);
                } else {
			gzFile zf;
			zf = gzopen(file,"rb");
			if (zf) {
				j = gzread(zf,data,*bytes);
				gzclose(zf);
                        }
		}
#endif
	} else {
		fp = fopen(file,"rb");
		if (fp != 0) {
			j = fread(data,1,*bytes,fp);
			fclose(fp);
		} else {
			return;
		}
	}
	if (j == (*bytes)) *err = 0;
	return;
}

void	write_bin_file_(char *file,void *data,long int *bytes,long int *err)
{
	FILE	*fp;
	int	comp;
	long int	j;
	char	zcmd[1024];

	*err = 1;
	if ((file[strlen(file)-2] == '.') && (file[strlen(file)-1] == 'Z')) {
		comp = 1;
        } else if ((file[strlen(file)-3] == '.') &&
                (file[strlen(file)-2] == 'g') &&
                (file[strlen(file)-1] == 'z')) {
		comp = 2;
	} else {
		comp = 0;
	}
	if (comp != 0) {
#ifdef POPEN_COMPRESSED_IO
		if (comp == 1) {
			sprintf(zcmd,"compress -c  - > %s",file);
		} else {
			sprintf(zcmd,"gzip > %s",file);
		}
                fp = popen(zcmd,"w");
		if (fp != 0) {
			j = fwrite(data,1,*bytes,fp);
			pclose(fp);
		} else {
			return;
		}
#else
		if (comp == 1) {
			j = write_compress_file(file,*bytes,data);
                } else {
			gzFile zf;
			zf = gzopen(file,"wb");
			if (zf) {
				j = gzwrite(zf,data,*bytes);
				gzclose(zf);
                        }
                }
#endif
	} else {
		fp = fopen(file,"wb");
		if (fp != 0) {
			j = fwrite(data,1,*bytes,fp);
			fclose(fp);
		} else {
			return;
		}
	}
	if (j == (*bytes)) *err = 0;
	return;
}

void pack_palette_(long int *pal,unsigned char *pack)
{
	long int	i;

	for(i=0;i<256;i++) {
		pack[i] = pal[i] & 0xff;
		pack[i+256] = pal[i+256] & 0xff;
		pack[i+512] = pal[i+512] & 0xff;
		pack[i+768] = pal[i+768] & 0xff;
	}
	return;
}

void cpack_palette_(long int *pal,long int *opac,unsigned long *pack)
{
	long int	i;
	unsigned long int	j;

	for(i=0;i<256;i++) {
		j = pal[i] & 0xff;
		j |= (pal[i+256] & 0xff) << 8;
		j |= (pal[i+512] & 0xff) << 16;
		j |= (opac[i] & 0xff) << 24;
		pack[i] = j;  /* $aabbggrr */
	}
	return;
}

void unpack_palette_(unsigned char *pack,long int *pal) 
{
	long int	i;

	for(i=0;i<256;i++) {
		pal[i] = pack[i];
		pal[i+256] = pack[i+256];
		pal[i+512] = pack[i+512];
		pal[i+768] = pack[i+768];
	}
	return;
}

void	cnvt_16_to_8_(unsigned short *in,unsigned char *out,long int *dx,
		long int *dy,float *lims,long int *bswap)
{
	long int	i;
	double		d,mult;
	
	mult = 255.0/(double)(lims[1]-lims[0]);
	for(i=0;i<(*dx)*(*dy);i++) {
		d = in[i];
		if (*bswap) {
			unsigned short	tmp;
			tmp  = (in[i] & 0x00ff) << 8;
			tmp |= (in[i] & 0xff00) >> 8;
			d = tmp;
		}
		d = (d - lims[0])*mult;
		if (d < 0) d = 0;
		if (d > 255) d = 255;
		out[i] = d;
	}

	return;
}

void	cnvt_f_to_8_(float *in,unsigned char *out,long int *dx,
		long int *dy,float *lims,long int *bswap)
{
	long int	i;
	double		d,mult;

	mult = 255.0/(double)(lims[1]-lims[0]);
	for(i=0;i<(*dx)*(*dy);i++) {
		if (*bswap) {
			unsigned char 	tmp[4],t;
			float		ftmp;
			memcpy(tmp,in+i,sizeof(float));
			t = tmp[3]; tmp[3] = tmp[0]; tmp[0] = t;
			t = tmp[2]; tmp[2] = tmp[1]; tmp[1] = t;
			memcpy(&ftmp,tmp,sizeof(float));
			d = ftmp;
                } else {
			d = in[i];
		}
		d = (d - lims[0])*mult;
		if (d < 0) d = 0;
		if (d > 255) d = 255;
		out[i] = d;
	}

	return;
}
