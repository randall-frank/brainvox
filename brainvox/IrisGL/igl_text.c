#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "igl_common.h"
#include "gl.h"

extern	BitmapFontRec	myfont;

/* Font interface */

long getdescender(void)
{
	long int	i,height=-1;
	for(i=0;i<myfont.num_chars;i++) {
		if (myfont.ch[i]) {
			if (height < myfont.ch[i]->yorig) {
				height = myfont.ch[i]->yorig;
			}
		}
	}
	return(height);
}

long strwidth(char *s)
{
	int c, length;
	const BitmapCharRec *ch;
	unsigned char	*string = (unsigned char *)s;

	length = 0;
  	for (; *string != '\0'; string++) {
    		c = *string;
    		if ((c >= myfont.first) && 
		    (c < myfont.first + myfont.num_chars)) {
      			ch = myfont.ch[c - myfont.first];
      			if (ch) length += ch->advance;
    		}
  	}
	return(length);
}

long getheight(void)
{
	long int	i,height=-1;
	for(i=0;i<myfont.num_chars;i++) {
		if (myfont.ch[i]) {
			if (height < myfont.ch[i]->height) {
				height = myfont.ch[i]->height;
			}
		}
	}
	return(height);
}

void charstr(char *s)
{
  	const BitmapCharRec *ch;
  	GLint swapbytes, lsbfirst, rowlength;
  	GLint skiprows, skippixels, alignment;
  	int	c;
	char	*string = s;

    	/* Save current modes. */
    	glGetIntegerv(GL_UNPACK_SWAP_BYTES, &swapbytes);
    	glGetIntegerv(GL_UNPACK_LSB_FIRST, &lsbfirst);
    	glGetIntegerv(GL_UNPACK_ROW_LENGTH, &rowlength);
    	glGetIntegerv(GL_UNPACK_SKIP_ROWS, &skiprows);
    	glGetIntegerv(GL_UNPACK_SKIP_PIXELS, &skippixels);
    	glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment);
    	glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);
    	glPixelStorei(GL_UNPACK_LSB_FIRST, GL_FALSE);
    	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    	glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
    	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  	for (; *string != '\0'; string++) {
		c = *string;
  		if (c < myfont.first ||
    			c >= myfont.first + myfont.num_chars) continue;
  		ch = myfont.ch[c - myfont.first];
  		if (ch) {
    			glBitmap(ch->width, ch->height, ch->xorig, ch->yorig,
      				ch->advance, 0, ch->bitmap);
		}
	}
    	/* Restore saved modes. */
    	glPixelStorei(GL_UNPACK_SWAP_BYTES, swapbytes);
    	glPixelStorei(GL_UNPACK_LSB_FIRST, lsbfirst);
    	glPixelStorei(GL_UNPACK_ROW_LENGTH, rowlength);
    	glPixelStorei(GL_UNPACK_SKIP_ROWS, skiprows);
    	glPixelStorei(GL_UNPACK_SKIP_PIXELS, skippixels);
    	glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
}

/* FORTRAN interfaces */
long getdes_(void)
{
	return(getdescender());
}

long strwid_(char *str,long *len)
{
	char tmp[2048];
	memcpy(tmp,str,*len);
	tmp[*len] = '\0';
	return(strwidth(tmp));
}

long gethei_(void)
{
	return(getheight());
}

void charst_(char *str, long *len)
{
	char tmp[2048];
	memcpy(tmp,str,*len);
	tmp[*len] = '\0';
	charstr(tmp);
}
