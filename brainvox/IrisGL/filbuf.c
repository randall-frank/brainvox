/*
** $RCSfile: filbuf.c,v $
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
** $Id: filbuf.c 770 2005-04-17 16:48:17Z rjfrank $
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
 *	ifilbuf -
 *
 *				Paul Haeberli - 1984
 *
 */
#include	"image.h"

/* TODO:  This fails under linux as _IOREAD is equal to 0 (O_RDONLY == 0) */
long int ifilbuf(IMAGE *image)
{
	int size;

	if ((image->flags&_IOREAD) == 0)
		return(EOF);
	if (image->base==NULL) {
		size = IBUFSIZE(image->xsize);
		if ((image->base = ibufalloc(image)) == NULL) {
			i_errhdlr("can't alloc image buffer");
			return EOF;
		}
	}
	image->cnt = getrow(image,image->base,image->y,image->z);
	image->ptr = image->base;
	if (--image->cnt < 0) {
		if (image->cnt == -1) {
			image->flags |= _IOEOF;
			if (image->flags & _IORW)
				image->flags &= ~_IOREAD;
		} else
			image->flags |= _IOERR;
		image->cnt = 0;
		return -1;
	}
	if(++image->y >= image->ysize) {
	    image->y = 0;
	    if(++image->z >= image->zsize) {
		image->z = image->zsize-1;
		image->flags |= _IOEOF;
		return -1;
	    }
	}
	return *image->ptr++ & 0xffff;
}
