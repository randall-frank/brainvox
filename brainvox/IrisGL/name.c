/*
** $RCSfile: name.c,v $
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
** $Id: name.c 770 2005-04-17 16:48:17Z rjfrank $
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
 *	isetname and isetcolormap -
 *
 *				Paul Haeberli - 1984
 *
 */
#include	<stdio.h>
#include	<string.h>
#include	"image.h"

void isetname(IMAGE *image,char *name)
{
    strncpy(image->name,name,80);
}

void isetcolormap(IMAGE *image,long int colormap)
{
    image->colormap = colormap;
}
