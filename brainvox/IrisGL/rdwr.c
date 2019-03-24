/*
** $RCSfile: rdwr.c,v $
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
** $Id: rdwr.c 1835 2006-07-23 22:48:13Z rjfrank $
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
 *	img_seek, img_write, img_read, img_optseek -
 *
 *				Paul Haeberli - 1984
 *
 */
#include	<stdio.h>
#include	<sys/types.h>
#ifndef WIN32
#include	<unistd.h>
#endif
#include	"image.h"

long int img_seek(IMAGE *image,unsigned long y,unsigned long z)
{
    if(img_badrow(image,y,z)) {
	i_errhdlr("img_seek: row number out of range");
	return EOF;
    }
    image->x = 0;
    image->y = y;
    image->z = z;
    if(ISVERBATIM(image->type)) {
	switch(image->dim) {
	    case 1:
		return img_optseek(image, 512L);
	    case 2: 
		return img_optseek(image,512L+(y*image->xsize)*BPP(image->type));
	    case 3: 
		return img_optseek(image,
		    512L+(y*image->xsize+z*image->xsize*image->ysize)*
							BPP(image->type));
	    default:
		i_errhdlr("img_seek: weird dim");
		break;
	}
    } else if(ISRLE(image->type)) {
	switch(image->dim) {
	    case 1:
		return img_optseek(image, image->rowstart[0]);
	    case 2: 
		return img_optseek(image, image->rowstart[y]);
	    case 3: 
		return img_optseek(image, image->rowstart[y+z*image->ysize]);
	    default:
		i_errhdlr("img_seek: weird dim");
		break;
	}
    } else 
	i_errhdlr("img_seek: weird image type");
	return(-1);
}

long int img_badrow(IMAGE *image,long int y,long int z)
{
    if(y>=image->ysize || z>=image->zsize)
	return 1;
    else
        return 0;
}

long int img_write(IMAGE *image,char *buffer,long count)
{
    long retval;

    retval =  write(image->file,buffer,count);
    if(retval == count) 
	image->offset += count;
    else
	image->offset = -1;
    return retval;
}

long int img_read(IMAGE *image,char *buffer,long count)
{
    long retval;

    retval =  read(image->file,buffer,count);
    if(retval == count) 
	image->offset += count;
    else
	image->offset = -1;
    return retval;
}

unsigned long img_optseek(IMAGE *image,unsigned long offset)
{
    if(image->offset != offset) {
       image->offset = offset;
       return lseek(image->file,offset,0);
   }
   return offset;
}

