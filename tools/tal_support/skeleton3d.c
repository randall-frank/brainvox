/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski
 *
 * $Id: skeleton3d.c 1250 2005-09-16 15:51:42Z dforeman $
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

/* local tables */
#include "skel3d_etable.h"

/* external prototypes */
#include "skeleton.h"

/* local prototypes */
static void calc_one(unsigned char *data,long int dx,long int dy,long int dz,
     long int x,long int y,long int z,long int *n0,long int *n1,long int *nei);

/* local macros */
#define G_DATA(X,Y,Z) data[((X))+(((Y))*dx)+(((Z))*dx*dy)]

/* Routine to take the 3d volume in DATA, threshold it to the level THRESH,
 and return the 3d skeleton in DATA.  Return values are the number of neighbors
 a skeleton voxel has.  The volume size is given by DX,DY,DZ.
*/
long int skeleton_3d_(unsigned char *data,long int thresh,
	long int dx,long int dy,long int dz,long int *hist) /* hist[28] */
{
	long int	i,x,y,z;
	long int	count,n0,n1,nei;

/* data definition: 0=not selected 1=sched for removal 3=permanent 2=possiblility */
/* threshold the volume */
	for(z=0;z<(dz);z++) {
	for(y=0;y<(dy);y++) {
	for(x=0;x<(dx);x++) {
/* 'paint' borders, threshold others */
		if ((x == 0) || (y == 0) || (z == 0) || 
			(x == (dx-1)) || (y == (dy-1)) || (z == (dz-1))) {
			G_DATA(x,y,z) = 0;
		} else if (G_DATA(x,y,z) > thresh) {
			G_DATA(x,y,z) = 2;
		} else {
			G_DATA(x,y,z) = 0;
		}
	}
	}
	}
/* repeated passes of the thinning routine until nothing is removed */
	count = 1;
	while (count > 0) {
		for(z=1;z<(dz-1);z++) {
		for(y=1;y<(dy-1);y++) {
		for(x=1;x<(dx-1);x++) {
/* only consider where the voxel can be removed */
		if (G_DATA(x,y,z) == 2) {
			calc_one(data,dx,dy,dz,x,y,z,&n0,&n1,&nei);
/* if one neighbor, set to perm */
			if (nei == 2) {
				G_DATA(x,y,z) = 3;
/* otherwise, consider contributions for euler characteristic with and without
	center voxel.  if the euler characteristic is the same, remove the voxel */
			} else if (n0 == n1) {
				G_DATA(x,y,z) = 1;
			}
		}
		}
		}
		}
/* remove all tagged voxels */
		count = 0;
		for(i=0;i<dx*dy*dz;i++) {
			if (data[i] == 1) {
				data[i] = 0;
				count++;
			}
		}
		fprintf(stderr,"Removed %ld voxels...\n",count);
	}  /* until count == 0 */
/* clear the histogram */
	for(i=0;i<256;i++) hist[i] = 0;
/* repaint the volume, to the number of neighbors */
	for(z=1;z<(dz-1);z++) {
	for(y=1;y<(dy-1);y++) {
	for(x=1;x<(dx-1);x++) {
/* only consider where the voxel exists */
		nei = 0;
		if (G_DATA(x,y,z) > 0) {
			calc_one(data,dx,dy,dz,x,y,z,&n0,&n1,&nei);
			G_DATA(x,y,z) = nei;
		}
		hist[nei] += 1;
	}
	}
	}
/* that's it */
	fprintf(stderr,"Done.\n");
	return(0);
}

static void calc_one(unsigned char *data,long int dx,long int dy,long int dz,
long int x,long int y,long int z,long int *n0,long int *n1,long int *nei)
{
	long int	xp,yp,zp;
	long int	save,val,i;
	
	*nei = 0;
	*n0 = 0;
	*n1 = 0;
	
/* count neighbors */
	for(zp=-1;zp<2;zp++) {
	for(yp=-1;yp<2;yp++) {
	for(xp=-1;xp<2;xp++) {
		if (G_DATA(x+xp,y+yp,z+zp) > 0) *nei += 1;
	}
	}
	}
	save = G_DATA(x,y,z);
/* compute n0,n1 */
	for(zp=0;zp<2;zp++) {
	for(yp=0;yp<2;yp++) {
	for(xp=0;xp<2;xp++) {
/* calc for center of 0 or 1 */
		for(i=0;i<3;i+=2) {
			G_DATA(x,y,z) = i;
			val = 0;
			if (G_DATA(x-1+xp+0,y-1+yp+0,z-1+zp+1)>1) val |= 0x0001;
			if (G_DATA(x-1+xp+1,y-1+yp+0,z-1+zp+1)>1) val |= 0x0002;
			if (G_DATA(x-1+xp+0,y-1+yp+0,z-1+zp+0)>1) val |= 0x0004;
			if (G_DATA(x-1+xp+1,y-1+yp+0,z-1+zp+0)>1) val |= 0x0008;
			if (G_DATA(x-1+xp+0,y-1+yp+1,z-1+zp+1)>1) val |= 0x0010;
			if (G_DATA(x-1+xp+1,y-1+yp+1,z-1+zp+1)>1) val |= 0x0020;
			if (G_DATA(x-1+xp+0,y-1+yp+1,z-1+zp+0)>1) val |= 0x0040;
			if (G_DATA(x-1+xp+1,y-1+yp+1,z-1+zp+0)>1) val |= 0x0080;
			if (i == 0) {
				*n0 += etable[val];
			} else {
				*n1 += etable[val];
			}
		}
/* next euler square */
	}
	}
	}
/* restore the saved location */
	G_DATA(x,y,z) = save;
	
	return;
}

#define PNT(i,j,k)  ((i)+(dx*(j))+(dx*dy*(k)))
/*
 *      00ZzYyXx = edge test bits (Max,min)
 */

/* routine to compute a 3D EDM (euclidean distance map) */
long int calc_edm_3d_(unsigned char *in,long int dx,long int dy,long int dz,
	long int feature,long int *hist)
{
        long int i,j,k,off;
        long int c,d,inc[26];
        unsigned char edges[26] = { 0x02, 0x06, 0x04, 0x05,
                                    0x01, 0x09, 0x08, 0x0a,
                                    0x20, 0x10,
                                    0x22, 0x26, 0x24, 0x25,
                                    0x21, 0x29, 0x28, 0x2a,
                                    0x12, 0x16, 0x14, 0x15,
                                    0x11, 0x19, 0x18, 0x1a };
        unsigned char edgex,edgey,edgez;


/* build increment table */
        inc[0] = 1;
        inc[2] = -dx;
        inc[4] = -1;
        inc[6] = dx;
/* z+,z- */
	inc[8] = dx*dy;
	inc[9] = -dx*dy;
/* angles */
        inc[1] = inc[0] + inc[2];
        inc[3] = inc[2] + inc[4];
        inc[5] = inc[4] + inc[6];
        inc[7] = inc[6] + inc[0];
/* plane above */
	for(i=0;i<8;i++) inc[10+i] = inc[i] + inc[8];
/* plane below */
	for(i=0;i<8;i++) inc[18+i] = inc[i] + inc[9];

/* fill all the feature values out */
        if (feature == 0) {
                for(k=0;k<dz;k++) {
                for(j=0;j<dy;j++) {
		off = PNT(0,j,k);
                for(i=0;i<dx;i++) {
                        if (in[off] == 0) {
                                in[off] = 255;
                        } else {
                                in[off] = 0;
                        }
			off += 1;
                }
                }
		}
        } else {
                for(k=0;k<dz;k++) {
                for(j=0;j<dy;j++) {
		off = PNT(0,j,k);
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
        }
/* left to right, top to bottom pass */
        for(k=0;k<dz;k++) {
	edgez = 0;
	if (k == 0) edgez |= 0x10;
	if (k == dz-1) edgez |= 0x20;
        for(j=0;j<dy;j++) {
	off = PNT(0,j,k);
	edgey = 0;
	if (j == 0) edgey |= 0x04;
	if (j == dy-1) edgey |= 0x08;
        for(i=0;i<dx;i++) {
                if (in[off] != 0) {
			edgex = 0;
			if (i == 0) edgex |= 0x01;
			if (i == dx-1) edgex |= 0x02;
                        c = in[off];
                        for(d=0;d<26;d++) {
				if ((edgex|edgey|edgez) & edges[d]) continue;
                                if (in[off+inc[d]] < c) c = in[off+inc[d]];
                        }
                        if (c > 254) c = 254;
                        in[off] = c + 1;
                }
		off += 1;
        }
        }
	}
/* bottom to top, right to left pass */
        for(k=dz-1;k>=0;k--) {
	edgez = 0;
	if (k == 0) edgez |= 0x10;
	if (k == dz-1) edgez |= 0x20;
        for(j=dy-1;j>=0;j--) {
	off = PNT(dx-1,j,k);
	edgey = 0;
	if (j == 0) edgey |= 0x04;
	if (j == dy-1) edgey |= 0x08;
        for(i=dx-1;i>=0;i--) {
                if (in[off] != 0) {
			edgex = 0;
			if (i == 0) edgex |= 0x01;
			if (i == dx-1) edgex |= 0x02;
                        c = in[off];
                        for(d=0;d<26;d++) {
				if ((edgex|edgey|edgez) & edges[d]) continue;
                                if (in[off+inc[d]] < c) c = in[off+inc[d]];
                        }
                        if (c > 254) c = 254;
                        in[off] = c + 1;
                }
		hist[in[off]] += 1;
		off -= 1;
        }
        }
	}
        return(0);
}

/* routine to compute a scaled 3D EDM (euclidean distance map) */
long int calc_edm_3d_s_(unsigned short *in,long int dx,long int dy,long int dz,
	long int feature,long int scale,long int *hist,double is,double *ip)
{
        long int 	i,j,k,off;
        long int 	c,d,inc[26];
	long int	dist_tab[26];
	double		btab[26] = {1,2,1,2,1,2,1,2,1,1,
				    2,3,2,3,2,3,2,3,
				    2,3,2,3,2,3,2,3};
	double		r;

        unsigned char edges[26] = { 0x02, 0x06, 0x04, 0x05,
                                    0x01, 0x09, 0x08, 0x0a,
                                    0x20, 0x10,
                                    0x22, 0x26, 0x24, 0x25,
                                    0x21, 0x29, 0x28, 0x2a,
                                    0x12, 0x16, 0x14, 0x15,
                                    0x11, 0x19, 0x18, 0x1a };
        unsigned char edgex,edgey,edgez;
	
/* TODO: fix for ip[] */
/* correct for the is != ip */
	r = is/ip[0];

/* build increment table */
        inc[0] = 1;
        inc[2] = -dx;
        inc[4] = -1;
        inc[6] = dx;
/* z+,z- */
	inc[8] = dx*dy;
	inc[9] = -dx*dy;
/* angles */
        inc[1] = inc[0] + inc[2];
        inc[3] = inc[2] + inc[4];
        inc[5] = inc[4] + inc[6];
        inc[7] = inc[6] + inc[0];
/* plane above */
	for(i=0;i<8;i++) inc[10+i] = inc[i] + inc[8];
/* plane below */
	for(i=0;i<8;i++) inc[18+i] = inc[i] + inc[9];
/* build distance table */
	for(i=0;i<26;i++) dist_tab[i] = sqrt(btab[i])*(double)(scale);
/* adjust z+ and z- */
	for(i=8;i<26;i++) dist_tab[i] *= r;

/* fill all the feature values out */
        if (feature == 0) {
                for(k=0;k<dz;k++) {
                for(j=0;j<dy;j++) {
		off = PNT(0,j,k);
                for(i=0;i<dx;i++) {
                        if (in[off] == 0) {
                                in[off] = 65535;
                        } else {
                                in[off] = 0;
                        }
			off += 1;
                }
                }
		}
        } else {
                for(k=0;k<dz;k++) {
                for(j=0;j<dy;j++) {
		off = PNT(0,j,k);
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
        }
/* left to right, top to bottom pass */
        for(k=0;k<dz;k++) {
	edgez = 0;
	if (k == 0) edgez |= 0x10;
	if (k == dz-1) edgez |= 0x20;
        for(j=0;j<dy;j++) {
	off = PNT(0,j,k);
	edgey = 0;
	if (j == 0) edgey |= 0x04;
	if (j == dy-1) edgey |= 0x08;
        for(i=0;i<dx;i++) {
                if (in[off] != 0) {
			edgex = 0;
			if (i == 0) edgex |= 0x01;
			if (i == dx-1) edgex |= 0x02;
                        c = in[off] + dist_tab[0];
                        for(d=0;d<26;d++) {
				if ((edgex|edgey|edgez) & edges[d]) continue;
                                if ((in[off+inc[d]]+dist_tab[d]) < c) {
					c = in[off+inc[d]]+dist_tab[d];
				}
                        }
                        in[off] = c;
                }
		off += 1;
        }
        }
	}
/* bottom to top, right to left pass */
        for(k=dz-1;k>=0;k--) {
	edgez = 0;
	if (k == 0) edgez |= 0x10;
	if (k == dz-1) edgez |= 0x20;
        for(j=dy-1;j>=0;j--) {
	off = PNT(dx-1,j,k);
	edgey = 0;
	if (j == 0) edgey |= 0x04;
	if (j == dy-1) edgey |= 0x08;
        for(i=dx-1;i>=0;i--) {
                if (in[off] != 0) {
			edgex = 0;
			if (i == 0) edgex |= 0x01;
			if (i == dx-1) edgex |= 0x02;
                        c = in[off] + dist_tab[0];
                        for(d=0;d<26;d++) {
				if ((edgex|edgey|edgez) & edges[d]) continue;
                                if ((in[off+inc[d]]+dist_tab[d]) < c) {
					c = in[off+inc[d]]+dist_tab[d];
				}
                        }
                        in[off] = c;
                }
		hist[in[off]] += 1;
		off -= 1;
        }
        }
	}
        return(0);
}
