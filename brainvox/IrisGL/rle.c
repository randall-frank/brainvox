/*
** $RCSfile: rle.c,v $
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
** $Id: rle.c 770 2005-04-17 16:48:17Z rjfrank $
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
 *	img_getrowsize, img_setrowsize, img_rle_compact, img_rle_expand -
 *
 *				Paul Haeberli - 1984
 *
 */
#include	<stdio.h>
#include	"image.h"

long int img_getrowsize(IMAGE *image)
{
    switch(image->dim) {
	case 1:
	    return image->rowsize[0];
	case 2:
	    return image->rowsize[image->y];
	case 3:
	    return image->rowsize[image->y+image->z*image->ysize];
    }
    return(0);
}

void img_setrowsize(IMAGE *image,long cnt,long y,long z)
{
    long *sizeptr;

    if(img_badrow(image,y,z)) 
	return;
    switch(image->dim) {
	case 1:
	    sizeptr = &image->rowsize[0];
	    image->rowstart[0] = image->rleend;
	    break;
	case 2:
	    sizeptr = &image->rowsize[y];
	    image->rowstart[y] = image->rleend;
	    break;
	case 3:
	    sizeptr = &image->rowsize[y+z*image->ysize];
	    image->rowstart[y+z*image->ysize] = image->rleend;
    }	
    if(*sizeptr != -1) 
	image->wastebytes += *sizeptr;
    *sizeptr = cnt;
    image->rleend += cnt;
}

#define docompact 							\
	while(iptr<ibufend) {						\
	    sptr = iptr;						\
	    iptr += 2;							\
	    while((iptr<ibufend)&&((iptr[-2]!=iptr[-1])||(iptr[-1]!=iptr[0])))\
		iptr++;							\
	    iptr -= 2;							\
	    count = iptr-sptr;						\
	    while(count) {						\
		todo = count>126 ? 126:count; 				\
		count -= todo;						\
		*optr++ = 0x80|todo;					\
		while(todo--)						\
		    *optr++ = *sptr++;					\
	    }								\
	    sptr = iptr;						\
	    cc = *iptr++;						\
	    while( (iptr<ibufend) && (*iptr == cc) )			\
		iptr++;							\
	    count = iptr-sptr;						\
	    while(count) {						\
		todo = count>126 ? 126:count; 				\
		count -= todo;						\
		*optr++ = todo;						\
		*optr++ = cc;						\
	    }								\
	}								\
	*optr++ = 0;

long int img_rle_compact(unsigned short *expbuf,long int ibpp,
	unsigned short *rlebuf,long int obpp,long int cnt)
{
	char	str[80];
	
    if(ibpp == 1 && obpp == 1) {
	register unsigned char *iptr = (unsigned char *)expbuf;
	register unsigned char *ibufend = iptr+cnt;
	register unsigned char *sptr;
	register unsigned char *optr = (unsigned char *)rlebuf;
	register short todo, cc;
	register long count;

	docompact;
	return optr - (unsigned char *)rlebuf;
    } else if(ibpp == 1 && obpp == 2) {
	register unsigned char *iptr = (unsigned char *)expbuf;
	register unsigned char *ibufend = iptr+cnt;
	register unsigned char *sptr;
	register unsigned short *optr = rlebuf;
	register short todo, cc;
	register long count;

	docompact;
	return optr - rlebuf;
    } else if(ibpp == 2 && obpp == 1) {
	register unsigned short *iptr = expbuf;
	register unsigned short *ibufend = iptr+cnt;
	register unsigned short *sptr;
	register unsigned char *optr = (unsigned char *)rlebuf;
	register short todo, cc;
	register long count;

	docompact;
	return optr - (unsigned char *)rlebuf;
    } else if(ibpp == 2 && obpp == 2) {
	register unsigned short *iptr = expbuf;
	register unsigned short *ibufend = iptr+cnt;
	register unsigned short *sptr;
	register unsigned short *optr = rlebuf;
	register short todo, cc;
	register long count;

	docompact;
	return optr - rlebuf;
    } else  {
    sprintf(str,"rle_compact: bad bpp: %ld %ld",ibpp,obpp);
	i_errhdlr(str);
	return 0;
    }
}

#define doexpand				\
	while(1) {				\
	    pixel = *iptr++;			\
	    if ( !(count = (pixel & 0x7f)) )	\
		return;				\
	    if(pixel & 0x80) {			\
	       while(count--)			\
		    *optr++ = *iptr++;		\
	    } else {				\
	       pixel = *iptr++;			\
	       while(count--)			\
		    *optr++ = pixel;		\
	    }					\
	}

void img_rle_expand(unsigned short *rlebuf,long int ibpp,
	unsigned short *expbuf,long int obpp)
{
	char	str[80];
	
    if(ibpp == 1 && obpp == 1) {
	register unsigned char *iptr = (unsigned char *)rlebuf;
	register unsigned char *optr = (unsigned char *)expbuf;
	register unsigned short pixel,count;

	doexpand;
    } else if(ibpp == 1 && obpp == 2) {
	register unsigned char *iptr = (unsigned char *)rlebuf;
	register unsigned short *optr = expbuf;
	register unsigned short pixel,count;

	doexpand;
    } else if(ibpp == 2 && obpp == 1) {
	register unsigned short *iptr = rlebuf;
	register unsigned char  *optr = (unsigned char *)expbuf;
	register unsigned short pixel,count;

	doexpand;
    } else if(ibpp == 2 && obpp == 2) {
	register unsigned short *iptr = rlebuf;
	register unsigned short *optr = expbuf;
	register unsigned short pixel,count;

	doexpand;
    } else 
    sprintf(str,"rle_expand: bad bpp: %ld %ld",ibpp,obpp);
	i_errhdlr(str);
}
