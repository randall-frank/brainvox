#include <voxel.h>
/* This routine will interpolate N images between two
   other images.  The output image will be placed one
   after the next in the output buffer.  It is assumed that
   the buffer is of size dx*dy*N */

void vl_interp_images_(VLUCHAR *start,VLUCHAR *end,VLINT32 dx,
	VLINT32 dy, VLINT32 n, VLUCHAR *out)
{
	VLINT32 i,dxy;
	VLINT32 j;
	VLUCHAR *ptr;
	VLFLOAT64 wstart,wend,wadd;

/* weighting to add with each slice */
	wadd = 1.0/(n+1.0);
	wend = wadd;
	wstart = 1.0 - wend;
	dxy = dx*dy;
	ptr = out;
	
	for(j=0; j<n; j++) {
/* for each slice */
		for(i=0;i<dxy;i++) {
			*ptr++ = start[i]*wstart + end[i]*wend;
		};
		wend = wend + wadd;
		wstart = 1.0 - wend;
	};
}
