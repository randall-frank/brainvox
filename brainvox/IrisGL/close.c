/*
** $RCSfile: close.c,v $
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
** $Id: close.c 770 2005-04-17 16:48:17Z rjfrank $
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
 *	iclose and iflush -
 *
 *				Paul Haeberli - 1984
 *
 */
#include	<stdio.h>
#include	<stdlib.h>
#include	"image.h"

long int iclose(IMAGE *image)
{
    long tablesize;
    int ret;

    iflush(image);
    img_optseek(image, 0);
    if (image->flags&_IOWRT) {
	if(image->dorev)
	    cvtimage((long *)image);
	if (img_write(image,(char *)image,sizeof(IMAGE)) != sizeof(IMAGE)) {
	    i_errhdlr("iclose: error on write of image header");
	    return EOF;
	}
	if(image->dorev)
	    cvtimage((long *)image);
	if(ISRLE(image->type)) {
	    img_optseek(image, 512L);
	    tablesize = image->ysize*image->zsize*sizeof(long);
	    if(image->dorev)
		cvtlongs((long *)image->rowstart,tablesize);
	    if (img_write(image,(char *)image->rowstart,tablesize)!=tablesize) {
		i_errhdlr("iclose: error on write of rowstart");
		return EOF;
	    }
	    if(image->dorev)
		cvtlongs(image->rowsize,tablesize);
	    if (img_write(image,(char *)image->rowsize,tablesize)!=tablesize) {
		i_errhdlr("iclose: error on write of rowsize");
		return EOF;
	    }
	}
    }
    if(image->base) {
	free(image->base);
	image->base = 0;
    }
    if(image->tmpbuf) {
	free(image->tmpbuf);
	image->tmpbuf = 0;
    }
    if(ISRLE(image->type)) {
	free(image->rowstart);
	image->rowstart = 0;
	free(image->rowsize);
	image->rowsize = 0;
    }
    ret = close(image->file);
    free(image);
    return ret;
}

long int iflush(IMAGE 	*image)
{
    unsigned short *base;

    if ( (image->flags&_IOWRT)
     && (base=image->base)!=NULL && (image->ptr-base)>0) {
	    if (putrow(image, base, image->y,image->z)!=image->xsize) {
		    image->flags |= _IOERR;
		    return(EOF);
	    }
    }
    return(0);
}
