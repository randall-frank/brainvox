/*
** $RCSfile: open.c,v $
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
** $Id: open.c 1835 2006-07-23 22:48:13Z rjfrank $
**
*/
/*
**
**  Abstract:
**
**  Author:
**
*/

/*
 *	iopen -
 *
 *				Paul Haeberli - 1984
 *
 */
#include	<stdio.h>
#include	<stdlib.h>
#include	<sys/types.h>
#ifndef WIN32
#include 	<unistd.h>
#endif
#include	"image.h"

IMAGE *iopen(char *file,char *mode,unsigned long type,unsigned long dim,
	unsigned long xsize,unsigned long ysize,unsigned long zsize)
{
    return(imgopen(0, file, mode, type, dim, xsize, ysize, zsize));
}

IMAGE *fiopen(long int f,char *mode,unsigned long type,unsigned long dim,
	unsigned long xsize,unsigned long ysize,unsigned long zsize)
{
    return(imgopen(f, 0, mode, type, dim, xsize, ysize, zsize));
}

IMAGE *imgopen(long int f,char *file,char *mode,unsigned long type,unsigned long dim,
	unsigned long xsize,unsigned long ysize,unsigned long zsize)
{
	register IMAGE 	*image;
	register int rw;
	int tablesize;
	register int i, max;
	char	str[1024];
	
	image = (IMAGE*)calloc(1,sizeof(IMAGE));
	rw = mode[1] == '+';
	if(rw) {
	    i_errhdlr("iopen: read/write mode not supported");
		return NULL;
	}
	if (*mode=='w') {
		if (file) {
		    f = creat(file, 0666);
		    if (rw && f>=0) {
			    close(f);
			    f = open(file, 2);
		    }
		}
		if (f < 0) {
			sprintf(str,"iopen: can't open output file %s",file);
		    i_errhdlr(str);
		    return NULL;
		}
		image->imagic = IMAGIC;
		image->type = type;
		image->xsize = xsize;
		image->ysize = 1;
		image->zsize = 1;
		if (dim>1)
		    image->ysize = ysize;
		if (dim>2)
		    image->zsize = zsize;
		if(image->zsize == 1) {
		    image->dim = 2;
		    if(image->ysize == 1)
			image->dim = 1;
		} else {
		    image->dim = 3;
		}
		image->min = 10000000;
		image->max = 0;
		isetname(image,"no name"); 
		image->wastebytes = 0;
		image->dorev = 0;
		if (write(f,image,sizeof(IMAGE)) != sizeof(IMAGE)) {
		    i_errhdlr("iopen: error on write of image header");
		    return NULL;
		}
	} else {
		if (file)
		    f = open(file, rw? 2: 0);
		if (f < 0)
		    return(NULL);
		if (read(f,image,sizeof(IMAGE)) != sizeof(IMAGE)) {
		    i_errhdlr("iopen: error on read of image header");
		    return NULL;
		}
		if( ((image->imagic>>8) | ((image->imagic&0xff)<<8)) 
							     == IMAGIC ) {
		    image->dorev = 1;
		    cvtimage((long *)image);
		} else
		    image->dorev = 0;
		if (image->imagic != IMAGIC) {
			sprintf(str,"iopen: bad magic in image file %x",image->imagic);
			i_errhdlr(str);
		    return NULL;
		}
	}
	if (rw)
	    image->flags = _IORW;
	else if (*mode != 'r')
	    image->flags = _IOWRT;
	else
	    image->flags = _IOREAD;
	if(ISRLE(image->type)) {
	    tablesize = image->ysize*image->zsize*sizeof(long);
	    image->rowstart = (unsigned long *)malloc(tablesize);
	    image->rowsize = (long *)malloc(tablesize);
	    if( image->rowstart == 0 || image->rowsize == 0 ) {
		i_errhdlr("iopen: error on table alloc");
		return NULL;
	    }
	    image->rleend = 512L+2*tablesize;
	    if (*mode=='w') {
		max = image->ysize*image->zsize;
		for(i=0; i<max; i++) {
		    image->rowstart[i] = 0;
		    image->rowsize[i] = -1;
		}
	    } else {
		tablesize = image->ysize*image->zsize*sizeof(long);
		lseek(f, 512L, 0);
		if (read(f,image->rowstart,tablesize) != tablesize) {
		    i_errhdlr("iopen: error on read of rowstart");
		    return NULL;
		}
		if(image->dorev)
		    cvtlongs((long *)image->rowstart,tablesize);
		if (read(f,image->rowsize,tablesize) != tablesize) {
		    i_errhdlr("iopen: error on read of rowsize");
		    return NULL;
		}
		if(image->dorev)
		    cvtlongs(image->rowsize,tablesize);
	    }
	}
	image->cnt = 0;
	image->ptr = 0;
	image->base = 0;
	if( (image->tmpbuf = ibufalloc(image)) == 0 ) {	
		sprintf(str,"iopen: error on tmpbuf alloc %ld",image->xsize);
	    i_errhdlr(str);
	    return NULL;
	}
	image->x = image->y = image->z = 0;
	image->file = f;
	image->offset = 512L;			/* set up for img_optseek */
	lseek(image->file, 512L, 0);
	return(image);
}

unsigned short *ibufalloc(IMAGE *image)
{
    return (unsigned short *)malloc(IBUFSIZE(image->xsize));
}

unsigned long reverse(unsigned long lwrd) 
{
    return ((lwrd>>24) 		| 
	   (lwrd>>8 & 0xff00) 	| 
	   (lwrd<<8 & 0xff0000) | 
	   (lwrd<<24) 		);
}

void cvtshorts(unsigned short *buffer,long n)
{
    register short i;
    register long nshorts = n>>1;
    register unsigned short swrd;

    for(i=0; i<nshorts; i++) {
	swrd = *buffer;
	*buffer++ = (swrd>>8) | (swrd<<8);
    }
}

void cvtlongs(long *buffer,long n)
{
    register short i;
    register long nlongs = n>>2;
    register unsigned long lwrd;

    for(i=0; i<nlongs; i++) {
	lwrd = buffer[i];
	buffer[i] =     ((lwrd>>24) 		| 
	   		(lwrd>>8 & 0xff00) 	| 
	   		(lwrd<<8 & 0xff0000) 	| 
	   		(lwrd<<24) 		);
    }
}

void cvtimage(long *buffer)
{
    cvtshorts((unsigned short *)buffer,12);
    cvtlongs(buffer+3,12);
    cvtlongs(buffer+26,4);
}

static void (*i_errfunc)(char *);

/*	error handler for the image library.  If the iseterror() routine
	has been called, sprintf's the args into a string and calls the
	error function.  Otherwise calls fprintf with the args and then
	exit.  This allows 'old' programs to assume that no errors
	ever need be worried about, while programs that know how and
	want to can handle the errors themselves.  Olson, 11/88
*/
void i_errhdlr(char *str)
{
	if(i_errfunc) {
		(*i_errfunc)(str);
		return;
	}
	fprintf(stderr, "%s\n", str);
	return;
}

/* this function sets the error handler for i_errhdlr */
void i_seterror(void (*func)(char *))
{
	i_errfunc = func;
}
