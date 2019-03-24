/*
** $RCSfile: image_protos.h,v $
** $Name:  $
**
** ASCI Visualization Project 
**
** Lawrence Livermore National Laboratory
** Information Management and Graphics Group
** P.O. Box 808, Mail Stop L-561
** Livermore, CA 94551-0808
**
** For information about this project see:
** 	http://www.llnl.gov/sccd/lc/img/
**
**      or contact: asciviz@llnl.gov
**
** For copyright and disclaimer information see:
**      $(ASCIVIS_ROOT)/copyright_notice_1.txt
**
** 	or man llnl_copyright
**
** $Id: image_protos.h 770 2005-04-17 16:48:17Z rjfrank $
**
*/
/*
**
**  Abstract:
**
**  Author:
**
*/

long int ifilbuf(IMAGE *image);
long int iflsbuf(IMAGE *image,unsigned long c);
long int iclose(IMAGE *image);
long int iflush(IMAGE *image);
void isetname(IMAGE *image,char *name);
void isetcolormap(IMAGE *image,long int colormap);

void cvtimage(long *buffer);
void cvtlongs(long *buffer,long n);
void cvtshorts(unsigned short *buffer,long n);
unsigned long reverse(unsigned long lwrd);
unsigned short *ibufalloc(IMAGE *image);

IMAGE *imgopen(long int f,char *file,char *mode,unsigned long type,unsigned long dim,
	unsigned long xsize,unsigned long ysize,unsigned long zsize);
IMAGE *fiopen(long int f,char *mode,unsigned long type,unsigned long dim,
	unsigned long xsize,unsigned long ysize,unsigned long zsize);
IMAGE *iopen(char *file,char *mode,unsigned long type,unsigned long dim,
	unsigned long xsize,unsigned long ysize,unsigned long zsize);

/*	
long int getpix(IMAGE *image);
long int putpix(IMAGE *image,unsigned long pix);
*/

unsigned long img_optseek(IMAGE *image,unsigned long offset);
long int img_read(IMAGE *image,char *buffer,long count);
long int img_write(IMAGE *image,char *buffer,long count);
long int img_badrow(IMAGE *image,long int y,long int z);
long int img_seek(IMAGE *image,unsigned long y,unsigned long z);

void img_rle_expand(unsigned short *rlebuf,long int ibpp,
	unsigned short *expbuf,long int obpp);
long int img_rle_compact(unsigned short *expbuf,long int ibpp,
	unsigned short *rlebuf,long int obpp,long int cnt);
void img_setrowsize(IMAGE *image,long cnt,long y,long z);
long int img_getrowsize(IMAGE *image);

long int putrow(IMAGE *image,unsigned short *buffer,unsigned long y,unsigned long z);
long int getrow(IMAGE *image,unsigned short *buffer,unsigned long y,unsigned long z);

void i_seterror(void (*func)(char *));
void i_errhdlr(char *err);
