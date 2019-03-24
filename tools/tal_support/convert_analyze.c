#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "proto.h"
#include "analyze_db.h"

/*
 * $Id: convert_analyze.c 1956 2006-09-11 01:15:37Z rjfrank $
 */

#define MIN(a,b) (((a)<(b)?(a):(b)))
#define MAX(a,b) (((a)>(b)?(a):(b)))

int	raw2analyze = 0;

int SwapAnalyzeHdrCheck(struct analyze_db *hdr);
void SwapAnalyzeHdr(struct analyze_db *hdr);
void SwapBytes(void *pin,int size,int num);

void ex_err(char *s)
{
	fprintf(stderr,"Fatal error:%s\n",s);
	tal_exit(1);
}

void cmd_err(char *s)
{
	if (raw2analyze) {
	fprintf(stderr,"(%s) Usage: %s [options] inputtemp outputfile\n",__DATE__,s);
	fprintf(stderr,"Options: -x(dx) image dx default:256\n");
	fprintf(stderr,"         -y(dy) image dy default:256\n");
	fprintf(stderr,"         -z(dz) image depth in bytes default:2\n");
	fprintf(stderr,"         -f(start) first image number default:1\n");
	fprintf(stderr,"         -l(end) last image number default:15\n");
	fprintf(stderr,"         -i(step) image number step default:1\n");
	fprintf(stderr,"         -b swap bytes with 16bit images (slices only) default:no\n");
	fprintf(stderr,"         -B swap header bytes default:native order\n");
	fprintf(stderr,"         -p(ipx[:ipy]) interpixel spacing default:1.0[:1.0]\n");
	fprintf(stderr,"         -s(interslice) interslice spacing default:6.64\n");
	fprintf(stderr,"         -t(xystep) stepping factor in X and Y axis default:1\n");
	fprintf(stderr,"\n");
	fprintf(stderr,"Individual input files will be read and the files {outputfile}.img and\n");
	fprintf(stderr,"{outputfile}.hdr will be written to disk.\n");
	fprintf(stderr,"The images written into analyze format will be decimated in the\n");
	fprintf(stderr,"XY plane according to the (xystep) parameter.  By default the images\n");
	fprintf(stderr,"stored in the analyze file are half size (xystep=2)\n");
	} else {
	fprintf(stderr,"(%s) Usage: %s [options] inputfile outputtemp\n",__DATE__,s);
	fprintf(stderr,"Options: -h print out hdr contents only\n");
	fprintf(stderr,"\n");
	fprintf(stderr,"The files {inputfile}.img and {inputfile}.hdr will be read and individual\n");
	fprintf(stderr,"files will be written to disk\n");
	}
	tal_exit(1);
}

int main(int argc,char **argv)
{
	long int	dx = 256;
	long int	dy = 256;
	long int	dz = 2;
	long int	xystep = 1;
	long int	istart = 1;
	long int	iend = 15;
	long int	istep = 1;
	long int	swab = 0;
	long int	flip = 1;
	long int	dumphdr = 0;
	double		ipixel[2] = {1.0,1.0};
	double		islice = 6.64;
	char		intemp[256],outtemp[256];
	char		tstr[256],*tp;
	long int	i,err,j;
	long int	icount,z,y,x;
	unsigned char	*data;
	unsigned char	*bptr,*bdata;
	unsigned short	*sptr,*sdata;
	float		*fptr,*fdata;
	unsigned char	*oimage;
	FILE		*fp;
	struct analyze_db	hdr;
	float		d_min,d_max;
	int		swap = 0;
	int		hdrSwap = 0;

        memset(&hdr,0,sizeof(hdr));
/* which direction...(The same program serves two purposes) */
	tp = argv[0];
	if (strrchr(tp,'/')) tp = strrchr(tp,'/') + 1;
	if (strrchr(tp,'\\')) tp = strrchr(tp,'\\') + 1;
	if (strstr(tp,"raw2analyze")) raw2analyze = 1;
/* parse command line */
	i = 1;
	while ((i < argc) && (argv[i][0] == '-')) {
		if (argv[i][1] == '\0') break;
		switch (argv[i][1]) {
			case 'f':
				istart = atoi(&(argv[i][2]));
				break;
			case 'l':
				iend = atoi(&(argv[i][2]));
				break;
			case 'i':
				istep = atoi(&(argv[i][2]));
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
			case 't':
				xystep = atoi(&(argv[i][2]));
				break;
			case 'p':
				get_d_xy(ipixel,&(argv[i][2]));
				break;
			case 's':
				islice = atof(&(argv[i][2]));
				break;
			case 'h':
				dumphdr = 1;
				break;
			case 'b':
				swab = 1;
				break;
			case 'B':
				hdrSwap = 1;
				break;
			case 'F':
				flip = 0;
				break;
			default:	
				cmd_err(argv[0]);
				break;
		}
		i++;
	}
	if ((argc-i) != 2) cmd_err(argv[0]);
	strcpy(intemp,argv[i]);
	strcpy(outtemp,argv[i+1]);
/* if converting from analyze, get the image size from the header */
	if (!raw2analyze) {
		strcpy(tstr,intemp);
		strcat(tstr,".hdr");
		printf("Reading %s \n",tstr);
		fp = fopen(tstr,"rb");
		if (fp == 0L) {
			printf("Error in opening the file %s\n",tstr);
			tal_exit(1);
		}
		if (fread(&hdr,sizeof(struct analyze_db),1,fp) != 1) {
			printf("Error in reading the file %s\n",tstr);
			tal_exit(1);
		}
		fclose(fp);

		swap = SwapAnalyzeHdrCheck(&hdr);
		if (swap == -1) {
			printf("Error: %s is not an Analyze hdr\n",tstr);
			tal_exit(1);
		}

/* get volume parameters */
		dx = hdr.dime.dim[1];
		dy = hdr.dime.dim[2];
		icount = hdr.dime.dim[3];
		switch(hdr.dime.datatype) {
			case DT_UNSIGNED_CHAR:
				dz = 1;
				break;
			case DT_SIGNED_SHORT:
				dz = 2;
				break;
			case DT_FLOAT:
				dz = 4;
				break;
			default:
				printf("Unknown datatype: %d\n",
                                       hdr.dime.datatype);
				break;
		}
/*
		dz = hdr.dime.bitpix;
		if (dz == 8) dz = 1;
		if (dz == 16) dz = 2;
		if ((dz != 1) && (dz != 2)) {
			printf("Error, number of bits per pixel is %ld.\n",
				dz);
			printf("Only 8 & 16 bits per pixel are supported.\n");
			tal_exit(1);
		}
*/
		if (dumphdr) {
		printf("%ld image of size %ldx%ld with %ld bytes/pixel\n",
			icount,dx,dy,dz);
			printf("Pixel dims: %f %f %f %f\n",hdr.dime.pixdim[0],
				hdr.dime.pixdim[1],hdr.dime.pixdim[2],
				hdr.dime.pixdim[3]);
			printf("Cal min and max: %f %f\n",hdr.dime.cal_min,
				hdr.dime.cal_max);
			printf("GL min and max: %d %d\n",hdr.dime.glmin,
				hdr.dime.glmax);
			
			exit(0);
		} else {
	printf("Output %ld images will be: %ldx%ld with %ld bytes per pixel\n",
			icount,dx,dy,dz);
		}
	} else {
		icount = 0;
		for(i=istart;i<=iend;i=i+istep) icount++;
	}
/* get the image memory */
	oimage = malloc(dz*dx*dy);  /* one slice */
	if (oimage == 0L) ex_err("Unable to allocate image memory.");

	data = malloc(dz*dx*dy*icount); /* entire volume */
	if (data == 0L) ex_err("Unable to allocate image memory.");

	if (dz == 1) {
		bptr = (unsigned char *)oimage;
		bdata = (unsigned char *)data;
	} else if (dz == 2) {
		sptr = (unsigned short *)oimage;
		sdata = (unsigned short *)data;
	} else {
		fptr = (float *)oimage;
		fdata = (float *)data;
	}

	if (raw2analyze) {
/* read the images */
		j = 0;
		for(i=istart;i<=iend;i=i+istep) {
			name_changer(intemp,i,&err,tstr);
			printf("Reading the file:%s\n",tstr);
			bin_io(tstr,'r',&(data[j]),dx,dy,dz,flip,swab,0L);
			if (flip) xflip(&(data[j]),dx,dy,dz);
			j = j + (dx*dy*dz);
		}
		for(i=0;i<icount*dx*dy;i++) {
			float	val;
			switch(dz) {
				case 2:
					val = sdata[i];
					break;
				case 4:
					val = fdata[i];
					break;
				default:
					val = bdata[i];
					break;
			}
			if (i == 0) {
				d_min = val;
				d_max = val;
			} else {
				if (val < d_min) d_min = val;
				if (val > d_max) d_max = val;
			}
		}
	} else {
		strcpy(tstr,intemp);
		strcat(tstr,".img");
		printf("Reading data volume %s \n",tstr);
		fp = fopen(tstr,"rb");
		if (fp == 0L) {
			printf("Error in opening the file %s\n",tstr);
			tal_exit(1);
		}
		if (fread(data,dx*dy*dz*icount,1,fp) != 1) {
			printf("Error in reading the file %s\n",tstr);
			tal_exit(1);
		}
		fclose(fp);
		if (swap) SwapBytes(data,dz,dx*dy*icount);
	}
/* write it out */
	if (raw2analyze) {
/* build the header */
/* hk */
		hdr.hk.sizeof_hdr = sizeof(struct analyze_db);
		hdr.hk.regular = 'r';
		hdr.hk.hkey_uno = 0;
		hdr.hk.extents = 0x00004000; /* ?? */
		strcpy(hdr.hk.data_type,"volume");
		strcpy(tstr,intemp);
		tstr[17] = 0;
		strcpy(hdr.hk.db_name,tstr);
/* dime */
		hdr.dime.dim[0] = 4;
		hdr.dime.dim[1] = dx/xystep;
		hdr.dime.dim[2] = dy/xystep;
		hdr.dime.dim[3] = icount;
		hdr.dime.dim[4] = 1;
		switch(dz) {
			case 1:
				hdr.dime.datatype = DT_UNSIGNED_CHAR;
				break;
			case 2:
				hdr.dime.datatype = DT_SIGNED_SHORT;
				break;
			case 4:
				hdr.dime.datatype = DT_FLOAT;
				break;
		}
		hdr.dime.pixdim[0] = 0.0;
		hdr.dime.pixdim[1] = ipixel[0]*xystep;
		hdr.dime.pixdim[2] = ipixel[1]*xystep;
		hdr.dime.pixdim[3] = islice*istep;
		hdr.dime.pixdim[4] = 0.0;
		hdr.dime.pixdim[5] = 0.0;
		hdr.dime.bitpix = dz*8;
		hdr.dime.funused1 = 1.0;
		hdr.dime.vox_offset = 0.0;
		hdr.dime.glmax = d_max;
		hdr.dime.glmin = d_min;
		hdr.dime.cal_max = d_max;
		hdr.dime.cal_min = d_min;
		strcpy(hdr.dhist.descrip,"Created by raw2analyze");

/* I am (not) completely neglecting hdr.dhist */
		for(i=0;i<6;i++) {
			hdr.dhist.origin[i] = 0;
		}

/* write the header */
		strcpy(tstr,outtemp);
		strcat(tstr,".hdr");
		printf("Writing %s \n",tstr);
		if (hdrSwap) SwapAnalyzeHdr(&hdr);
		fp = fopen(tstr,"wb");
		if (fp == 0L) {
			printf("Error in opening the file %s\n",tstr);
			tal_exit(1);
		}
		if (fwrite(&hdr,sizeof(struct analyze_db),1,fp) != 1) {
			printf("Error in writing the file %s\n",tstr);
			tal_exit(1);
		}
		fclose(fp);
		if (hdrSwap) SwapAnalyzeHdr(&hdr);
/* write the volume */
		strcpy(tstr,outtemp);
		strcat(tstr,".img");
		printf("Writing data volume %s \n",tstr);
		fp = fopen(tstr,"wb");
		if (fp == 0L) {
			printf("Error in opening the file %s\n",tstr);
			tal_exit(1);
		}
/* slice by slice */
		for(z=0;z<icount;z++) {
			i = 0;
			for(y=0;y<dy;y+=xystep) {
				for(x=0;x<dx;x+=xystep) {
					j = (z*dx*dy) + (y*dx) + x;
					if (dz == 1) {
						bptr[i] = bdata[j];
					} else if (dz == 2) {
						sptr[i] = sdata[j];
					} else { 
						fptr[i] = fdata[j];
					}
					i++;
				}
			}
		if (fwrite(oimage,(dx*dy*dz)/(xystep*xystep),1,fp) != 1) {
			printf("Error in writing the file %s\n",tstr);
			tal_exit(1);
		}
		}
		fclose(fp);
	} else {
/* write out the individual images */
		fprintf(stderr,
	"Writing:%ld images of size %ldx%ld at %ld bytes/pixel.\n",
			icount,dx,dy,dz);
		i = 0;
		for(z=0;z<icount;z++) {
			for(y=0;y<(dy);y+=1) {
				for(x=0;x<(dx);x+=1) {
					if (dz == 1) {
						bptr[x+(dx*y)] = bdata[i];
					} else if (dz == 2) {
						sptr[x+(dx*y)] = sdata[i];
					} else {
						fptr[x+(dx*y)] = fdata[i];
					}
					i++;
				}
			}		
			name_changer(outtemp,z+1,&err,tstr);
			printf("Writing sampled file:%s\n",tstr);
			if (flip) xflip(oimage,dx,dy,dz);
			bin_io(tstr,'w',oimage,dx,dy,dz,flip,swab,0L);
		}
	}
	
	free(data);
	free(oimage);
	tal_exit(0);

	exit(0);
}


#define MAGIC_LEN 	0x0000015c
#define MAGIC_LEN_SWAP	0x5c010000

int SwapAnalyzeHdrCheck(struct analyze_db *hdr)
{
	if (hdr->hk.sizeof_hdr == MAGIC_LEN) return(0);
	if (hdr->hk.sizeof_hdr != MAGIC_LEN_SWAP) return(-1);
	SwapAnalyzeHdr(hdr);
	return(1);
}

void SwapAnalyzeHdr(struct analyze_db *hdr)
{

	SwapBytes(&(hdr->hk.sizeof_hdr),sizeof(int),1);
	SwapBytes(&(hdr->hk.extents),sizeof(int),1);
	SwapBytes(&(hdr->hk.session_error),sizeof(short int),1);

	SwapBytes(hdr->dime.dim,sizeof(short int),8);
	SwapBytes(&(hdr->dime.datatype),sizeof(short int),1);
	SwapBytes(&(hdr->dime.bitpix),sizeof(short int),1);
	SwapBytes(&(hdr->dime.dim_uno),sizeof(short int),1);
	SwapBytes(hdr->dime.pixdim,sizeof(float),8);
	SwapBytes(&(hdr->dime.vox_offset),sizeof(float),1);
	SwapBytes(&(hdr->dime.cal_max),sizeof(float),1);
	SwapBytes(&(hdr->dime.cal_min),sizeof(float),1);
	SwapBytes(&(hdr->dime.compressed),sizeof(int),1);
	SwapBytes(&(hdr->dime.verified),sizeof(int),1);
	SwapBytes(&(hdr->dime.glmin),sizeof(int),1);
	SwapBytes(&(hdr->dime.glmax),sizeof(int),1);

	SwapBytes(&(hdr->dhist.views),sizeof(int),1);
	SwapBytes(&(hdr->dhist.vols_added),sizeof(int),1);
	SwapBytes(&(hdr->dhist.start_field),sizeof(int),1);
	SwapBytes(&(hdr->dhist.field_skip),sizeof(int),1);
	SwapBytes(&(hdr->dhist.omin),sizeof(int),1);
	SwapBytes(&(hdr->dhist.omax),sizeof(int),1);
	SwapBytes(&(hdr->dhist.smin),sizeof(int),1);
	SwapBytes(&(hdr->dhist.smax),sizeof(int),1);
}

void SwapBytes(void *pin,int size,int num)
{
	int		i;
	unsigned char	*p = (unsigned char *)pin;
	unsigned char	c;

	switch(size) {
		case 2:
			for(i=0;i<num;i++) {
				c = p[1]; p[1] = p[0]; p[0] = c;
				p += size;
			}
			break;
		case 4:
			for(i=0;i<num;i++) {
				c = p[3]; p[3] = p[0]; p[0] = c;
				c = p[2]; p[2] = p[1]; p[1] = c;
				p += size;
			}
			break;
		case 8:
			for(i=0;i<num;i++) {
				c = p[7]; p[7] = p[0]; p[0] = c;
				c = p[6]; p[6] = p[1]; p[1] = c;
				c = p[5]; p[5] = p[2]; p[2] = c;
				c = p[4]; p[4] = p[3]; p[3] = c;
				p += size;
			}
			break;
	}
	return;
}
