#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "sgilib.h"

void svimg_(char *filename,int *array,int xsize,int ysize,int zsize,int zoom,
		int llx,int lly,int dx,int dy,int *err);

void svimg_(char *filename,int *array,int xsize,int ysize,int zsize,int zoom,
		int llx,int lly,int dx,int dy,int *err) 
{
    int y,ptr,x;
    int oy,ox,z;
    sgi_t *image;

unsigned short rbuf[4096];
unsigned short gbuf[4096];
unsigned short bbuf[4096];
 
    *err = 0;
    image = sgiOpen(filename,SGI_WRITE,SGI_COMP_RLE,1,dx*zoom,dy*zoom,zsize);
    if (image == NULL) {
	*err = -1;
	return;
    }
    oy = 0;
    for(y=0; y<dy; y++) {
/* copy the line into the bufs  $aabbggrr */
	ptr = ((y + lly)*xsize) + llx;
	ox = 0;
	for(x=0; x<dx; x++) {
		for(z=0; z<zoom; z++) {
			rbuf[ox] = (array[ptr] & 0xff);
			gbuf[ox] = (array[ptr] >> 8) & 0xff;
			bbuf[ox] = (array[ptr] >> 16) & 0xff;
			ox = ox + 1;
		}
		ptr++;
	}
	/*
		fill rbuf, gbuf, and bbuf with pixel values 
	*/
	for(z=0; z<zoom; z++) {
		sgiPutRow(image,rbuf,oy,0);	/* red row */
		if (zsize != 1) {
			sgiPutRow(image,gbuf,oy,1);	/* green row */
			sgiPutRow(image,bbuf,oy,2);	/* blue row */
		}
		oy = oy + 1;
	}
    }
    sgiClose(image);
    return;
}
void SVIMG(char *filename,int *array,int xsize,int ysize,int zsize,int zoom,
		int llx,int lly,int dx,int dy,int *err) 
{
svimg_(filename,array,xsize,ysize,zsize,zoom,llx,lly,dx,dy,err);
}

#ifdef OLDER_API

#include "image.h"

void svimg_(char *filename,int *array,int xsize,int ysize,int zsize,int zoom,
		int llx,int lly,int dx,int dy,int *err) 
{
    int y,ptr,x;
    int oy,ox,z;
    IMAGE *image;

unsigned short rbuf[4096];
unsigned short gbuf[4096];
unsigned short bbuf[4096];
 
    *err = 0;
    image = iopen(filename,"w",RLE(1),3,dx*zoom,dy*zoom,zsize);
    if (image == NULL) {
	*err = -1;
	return;
    }
    oy = 0;
    for(y=0; y<dy; y++) {
/* copy the line into the bufs  $aabbggrr */
	ptr = ((y + lly)*xsize) + llx;
	ox = 0;
	for(x=0; x<dx; x++) {
		for(z=0; z<zoom; z++) {
			rbuf[ox] = (array[ptr] & 0xff);
			gbuf[ox] = (array[ptr] >> 8) & 0xff;
			bbuf[ox] = (array[ptr] >> 16) & 0xff;
			ox = ox + 1;
		}
		ptr++;
	}
	/*
		fill rbuf, gbuf, and bbuf with pixel values 
	*/
	for(z=0; z<zoom; z++) {
		putrow(image,rbuf,oy,0);	/* red row */
		if (zsize != 1) {
			putrow(image,gbuf,oy,1);	/* green row */
			putrow(image,bbuf,oy,2);	/* blue row */
		}
		oy = oy + 1;
	}
    }
    iclose(image);
    return;
}
#endif
