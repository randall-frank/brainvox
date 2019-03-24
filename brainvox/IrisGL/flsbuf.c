/*
** $RCSfile: flsbuf.c,v $
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
** $Id: flsbuf.c 770 2005-04-17 16:48:17Z rjfrank $
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
 *	iflsbuf -
 *
 *				Paul Haeberli - 1984
 *
 */
#include	"image.h"

long int iflsbuf(IMAGE *image,unsigned long c)
{
	register unsigned short *base;
	register int n, rn;
	char c1;
	int size;

	if ((image->flags&_IOWRT)==0)
		return(EOF);
	if ((base=image->base)==NULL) {
		size = IBUFSIZE(image->xsize);
		if ((image->base=base=ibufalloc(image)) == NULL) {
			i_errhdlr("flsbuf: error on buf alloc");
			return EOF;
		}
		rn = n = 0;
	} else if ((rn = n = image->ptr - base) > 0)  {
			n = putrow(image,base,image->y,image->z);
			if(++image->y >= image->ysize) {
			    image->y = 0;
			    if(++image->z >= image->zsize) {
				image->z = image->zsize-1;
				image->flags |= _IOEOF;
				return -1;
			    }
			}
 	}
	image->cnt = image->xsize-1;
	*base++ = c;
	image->ptr = base;
	if (rn != n) {
		image->flags |= _IOERR;
		return(EOF);
	}
	return(c);
}
