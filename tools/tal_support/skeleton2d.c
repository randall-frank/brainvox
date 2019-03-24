/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski
 *
 * $Id: skeleton2d.c 1250 2005-09-16 15:51:42Z dforeman $
 *                University of Iowa, Department of Neurology,
 *                Human Neuroanatomy and Neuroimaging Lab
 *                University of Iowa, Image Analysis Facility
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All modifications to the source code must be clearly marked as
 *    such.  Binary redistributions based on modified source code
 *    must be clearly marked as modified versions in the documentation
 *    and/or other materials provided with the distribution.
 * 4. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgment:
 *      This product includes software developed by Randall Frank, Thomas
 *      Grabowski and other unpaid contributors.
 * 5. The names of Randall Frank or Thomas Grabowski may not be used to
 *    endorse or promote products derived from this software without 
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY RANDALL FRANK AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL RANDALL FRANK OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* external prototypes */
#include "skeleton.h"

/* routine to perform the classical thinning routine from Pavlidis (p 200) */
/* pixel locations:
   3 2 1
   4 P 0
   5 6 7
*/

#define PNT(i,j)  ((i)+(dx*(j)))

long int	checkpatterns(unsigned char *in,long int dx,long int dy,
	long int i,long int j,long int *inc);

long int	calc_skel_(unsigned char *in,long int dx,long int dy,
	long int *hist)
{
	long int	i,j,d,inc[8],c;
	long int	remain,skel,off;
		
/* compute the thresholded image */
	for(i=0;i<(dx*dy);i++) if (in[i] > 0) in[i] = 1;
/* clear boundaries */
	for(i=0;i<dx;i++) {
		in[i] = 0;
		in[((dy-1)*dx)+i] = 0;
	}
	for(i=0;i<dy;i++) {
		in[(i*dx)] = 0;
		in[(i*dx)+dx-1] = 0;
	}
/* build increment table */
	inc[0] = 1;
	inc[2] = -dx;
	inc[4] = -1;
	inc[6] = dx;
/* angles */
	inc[1] = inc[0] + inc[2];
	inc[3] = inc[2] + inc[4];
	inc[5] = inc[4] + inc[6];
	inc[7] = inc[6] + inc[0];
/* start of the routine */
	remain = 1;
	while(remain) {
		remain = 0;
		for(d=0;d<7;d=d+2) {
/* for all pixels */
			for(j=1;j<dy-1;j++) {
			off = PNT(1,j);
			for(i=1;i<dx-1;i++) {
				if ((in[off] == 1) && (in[off+inc[d]] == 0)) {
					skel = 0;
					if (checkpatterns(in,dx,dy,i,j,inc) > 0) {
						skel = 1;
					}
					if (skel) {
						in[off] = 2;
					} else {
						in[off] = 3;
						remain = 1;
					}
				}
				off += 1;
			}
			}
/* for all pixels */
			c = 0;
			for(j=1;j<dy-1;j++) {
			off = PNT(1,j);
			for(i=1;i<dx-1;i++) {
				if (in[off] == 3) {
					in[off] = 0;
					c++;
				}
				off += 1;
			}
			}
			printf("%ld pixels removed...\n",c);
		}
	}
/* count the number of neighbors */
	for(j=1;j<dy-1;j++) {
	off = PNT(1,j);
	for(i=1;i<dx-1;i++) {
		c = 0;
		if (in[off] != 0) {
			c = 1;
			for(d=0;d<8;d++) {
				if (in[off+inc[d]] != 0) c = c + 1;
			}
			in[off] = c;
		}
		hist[c] += 1;
		off += 1;
	}
	}
	return(0);	
}
/* patterns for matching */
/* At least one of each A or B group must be set 
1:      2:           Bits
  A A A    A 0 B     3 2 1
  0 P 0    A P B     4 P 0
  B B B    A 0 B     5 6 7
3:      4:       5:       6:
  A A A    A A A    2 0 A    A 0 2
  A P 0    0 P A    0 P A    A P 0
  A 0 2    2 0 A    A A A    A A A
  
  mask = 0123 4567 
*/

long int	checkpatterns(unsigned char *in,long int dx,long int dy,
	long int i,long int j,long int *inc)
{
/* build a bit mask from the locales  0123 4567 and repeat it to allow for 
	bit shift rotations */
	long int mask,n;
	
	mask = 0;
	for(n=0;n<8;n++) {
		mask = mask << 1;
		if (in[PNT(i,j)+inc[n]] != 0) {
			mask = mask | 0x0001;
		} else {
			mask = mask & 0xfffe;
		}
	}
/* thus the mask = 0123 4567 */

/* check pattern 1 */
	if ( ((mask & 0x70) != 0) && ((mask & 0x07) != 0) && ((mask & 0x88) == 0) ) {
		return(1);
	}
/* check pattern 2 */
	if ( ((mask & 0x1C) != 0) && ((mask & 0xC1) != 0) && ((mask & 0x22) == 0) ) {
		return(2);
	}
/* check pattern 3 */
	if ( ((mask & 0x01) != 0) && ((mask & 0x7C) != 0) && ((mask & 0x82) == 0) ) {
		return(3);
	}
/* check pattern 4 */
	if ( ((mask & 0x04) != 0) && ((mask & 0xF1) != 0) && ((mask & 0x0A) == 0) ) {
		return(4);
	}
/* check pattern 5 */
	if ( ((mask & 0x10) != 0) && ((mask & 0xC7) != 0) && ((mask & 0x28) == 0) ) {
		return(5);
	}
/* check pattern 6 */
	if ( ((mask & 0x40) != 0) && ((mask & 0x1F) != 0) && ((mask & 0xA0) == 0) ) {
		return(6);
	}
	return(0);
}

/*
 *	00ZzYyXx = edge test bits (Max,min)
 */

/* routine to compute a EDM (euclidean distance map) */
long int calc_edm_(unsigned char *in,long int dx,long int dy,long int feature,
	long int *hist)
{
	long int i,j,off;
	long int c,d,inc[8];
	unsigned char edges[8] = { 0x02, 0x06, 0x04, 0x05, 
                                   0x01, 0x09, 0x08, 0x0a };
	unsigned char edgex,edgey;
	
/* build increment table */
	inc[0] = 1;
	inc[2] = -dx;
	inc[4] = -1;
	inc[6] = dx;
/* angles */
	inc[1] = inc[0] + inc[2];
	inc[3] = inc[2] + inc[4];
	inc[5] = inc[4] + inc[6];
	inc[7] = inc[6] + inc[0];	
/* fill all the feature values out */	
	if (feature == 0) {
		for(j=0;j<dy;j++) {
		off = PNT(0,j);
		for(i=0;i<dx;i++) {
			if (in[off] == 0) {
				in[off] = 255;
			} else {
				in[off] = 0;
			}
			off += 1;
		}
		}
	} else {
		for(j=0;j<dy;j++) {
		off = PNT(0,j);
		for(i=0;i<dx;i++) {
			if (in[off] != 0) {
				in[off] = 255;
			} else {
				in[off] = 0;
			}
			off += 1;
		}
		}
	}
#if 0
/* clear boundaries */
	for(i=0;i<dx;i++) {
		in[i] = 0;
		in[((dy-1)*dx)+i] = 0;
	}
	for(i=0;i<dy;i++) {
		in[(i*dx)] = 0;
		in[(i*dx)+dx-1] = 0;
	}	
#endif
/* left to right, top to bottom pass */
	for(j=0;j<dy;j++) {
	off = PNT(0,j);
	edgey = 0;
	if (j == 0) edgey |= 0x04;
	if (j == dy-1) edgey |= 0x08;
	for(i=0;i<dx;i++) {
		if (in[off] != 0) {
			edgex = 0;
			if (i == 0) edgex |= 0x01;
			if (i == dx-1) edgex |= 0x02;
			c = in[off];
			for(d=0;d<8;d++) {
				if ((edgex | edgey) & edges[d]) continue;
				if (in[off+inc[d]] < c) c = in[off+inc[d]];
			}
			if (c > 254) c = 254;
			in[off] = c + 1;
		}
		off += 1;
	}
	}
/* bottom to top, right to left pass */
	for(j=dy-1;j>=0;j--) {
	off = PNT(dx-1,j);
	edgey = 0;
	if (j == 0) edgey |= 0x04;
	if (j == dy-1) edgey |= 0x08;
	for(i=dx-1;i>=0;i--) {
		if (in[off] != 0) {
			edgex = 0;
			if (i == 0) edgex |= 0x01;
			if (i == dx-1) edgex |= 0x02;
			c = in[off];
			for(d=0;d<8;d++) {
				if ((edgex | edgey) & edges[d]) continue;
				if (in[off+inc[d]] < c) c = in[off+inc[d]];
			}
			if (c > 254) c = 254;
			in[off] = c + 1;
		}
		hist[in[off]] += 1;
		off -= 1;
	}
	}
	return(0);
}


/* routine to compute a scaled EDM (euclidean distance map) */
long int calc_edm_s_(unsigned short *in,long int dx,long int dy,
	long int feature,long int scale,long int *hist)
{
	long int i,j,off;
	long int c,d,inc[8];
	long int dist_tab[8];
	unsigned char edges[8] = { 0x02, 0x06, 0x04, 0x05, 
                                   0x01, 0x09, 0x08, 0x0a };
	unsigned char edgex,edgey;
	
/* build increment table */
	inc[0] = 1;
	inc[2] = -dx;
	inc[4] = -1;
	inc[6] = dx;
/* angles */
	inc[1] = inc[0] + inc[2];
	inc[3] = inc[2] + inc[4];
	inc[5] = inc[4] + inc[6];
	inc[7] = inc[6] + inc[0];	
/* distances */
	for(i=0;i<8;i++) {
		if ((i & 1) == 1) {
			dist_tab[i] = sqrt(2.0)*(double)(scale);
		} else {
			dist_tab[i] = 1.0*(double)(scale);
		}
	}
/* fill all the feature values out */	
	if (feature == 0) {
		for(j=0;j<dy;j++) {
		off = PNT(0,j);
		for(i=0;i<dx;i++) {
			if (in[off] == 0) {
				in[off] = 65535;
			} else {
				in[off] = 0;
			}
			off += 1;
		}
		}
	} else {
		for(j=0;j<dy;j++) {
		off = PNT(0,j);
		for(i=0;i<dx;i++) {
			if (in[off] != 0) {
				in[off] = 65535;
			} else {
				in[off] = 0;
			}
			off += 1;
		}
		}
	}
#if 0
/* clear boundaries */
	for(i=0;i<dx;i++) {
		in[i] = 0;
		in[((dy-1)*dx)+i] = 0;
	}
	for(i=0;i<dy;i++) {
		in[(i*dx)] = 0;
		in[(i*dx)+dx-1] = 0;
	}	
#endif
/* left to right, top to bottom pass */
	for(j=0;j<dy;j++) {
	off = PNT(0,j);
	edgey = 0;
	if (j == 0) edgey |= 0x04;
	if (j == dy-1) edgey |= 0x08;
	for(i=0;i<dx;i++) {
		if (in[off] != 0) {
			edgex = 0;
			if (i == 0) edgex |= 0x01;
			if (i == dx-1) edgex |= 0x02;
			c = in[off] + dist_tab[0];
			for(d=0;d<8;d++) {
				if ((edgex | edgey) & edges[d]) continue;
				if ((in[off+inc[d]]+dist_tab[d]) < c) {
					c = in[off+inc[d]] + dist_tab[d];
				}
			}
			in[off] = c;
		}
		off += 1;
	}
	}
/* bottom to top, right to left pass */
	for(j=dy-1;j>=0;j--) {
	off = PNT(dx-1,j);
	edgey = 0;
	if (j == 0) edgey |= 0x04;
	if (j == dy-1) edgey |= 0x08;
	for(i=dx-1;i>=0;i--) {
		if (in[off] != 0) {
			edgex = 0;
			if (i == 0) edgex |= 0x01;
			if (i == dx-1) edgex |= 0x02;
			c = in[off] + dist_tab[0];
			for(d=0;d<8;d++) {
				if ((edgex | edgey) & edges[d]) continue;
				if ((in[off+inc[d]]+dist_tab[d]) < c) {
					c = in[off+inc[d]] + dist_tab[d];
				}
			}
			in[off] = c;
		}
		hist[in[off]] += 1;
		off -= 1;
	}
	}
	return(0);
}
