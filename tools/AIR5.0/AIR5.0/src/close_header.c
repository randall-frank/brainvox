/* Copyright 1997-2001 Roger P. Woods, M.D. */
/* Modified 5/27/01 */

/*
 * void close_header()
 *
 * fp->errcode is set if file closure fails
 * fp->fp_hdr and fp->fp_img are always set to NULL
 */

#include "AIR.h"
#include "HEADER.h"

void AIR_close_header(struct AIR_Fptrs *fp)
{
	fp->errcode=0;
	
	/* Close the header */
	if(fp->fp_hdr){
		fp->errcode=AIR_fclose_decompress(fp->fp_hdr,fp->header_was_compressed);
		fp->fp_hdr=NULL;
	}
	
	/* Close the image */
	if(fp->fp_img){
		AIR_Error errcode=AIR_fclose_decompress(fp->fp_img,fp->image_was_compressed);
		fp->fp_img=NULL;
		if(errcode!=0) fp->errcode=errcode;
	}
}
