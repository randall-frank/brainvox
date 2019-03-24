/*
** $RCSfile: pix.c,v $
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
** $Id: pix.c 770 2005-04-17 16:48:17Z rjfrank $
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
 *	getpix and putpix -
 *
 *				Paul Haeberli - 1984
 *
 */
#include	<stdio.h>
#include	"image.h"

#undef getpix
#undef putpix

long int getpix(IMAGE *image);
long int putpix(IMAGE *image,unsigned long pix);

long int getpix(IMAGE *image)
{
    if(--(image)->cnt>=0)
    	return *(image)->ptr++;
    else
	return ifilbuf(image);
}

long int putpix(IMAGE *image,unsigned long pix)
{
    if(--(image)->cnt>=0)
        return *(image)->ptr++ = pix;
    else
	return iflsbuf(image,pix);
}
