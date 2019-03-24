#include "voxel.h"
#include <stdio.h>
#include <stdlib.h>

void vl_image_stats_(long int x,
		     long int y,
                     long int st,
                     long int ed,
                     long int step,
                     long int interp,
		     unsigned char *data,
		     long int *hist,
                     unsigned long int *chk)
{
	register long int i,j;
	long int	isize;
	register long int ptr;
	register unsigned char *lptr;
	register long int csum;

/* init vars */
	isize = x*y;
	ptr = 0;
	csum = 0;
	for(i=0;i<256; i++) hist[i] = 0;
/* for every REAL slice */
	for(i=st; i<=ed; i = i + step) {
		lptr = data + ptr;
/* over the current image sum the pixels and add to the histogram */
		for(j=0;j<x*y;j++) {
			hist[*lptr]++;
			csum += *lptr;
			lptr++;
		}
/* bump over interpolated slices */
		ptr = ptr + isize + (isize*interp);
	}
	*chk = csum;
        
	return;
}
