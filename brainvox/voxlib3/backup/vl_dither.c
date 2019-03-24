#include "voxel.h"
#include "vl_util.h"
#include <stdio.h>
#include <stdlib.h>

#include "colortbl.inc"

/* declare local functions */
void makestdcmap_(unsigned char *cmap);
long int colordist(long i,long r,long g,long b,unsigned char *cmap);
long int rgbffindindex(long r,long g,long b);
long int rgbfindindex(long r,long g,long b,unsigned char *cmap,
	unsigned char *itab);
void rgbdither(long int *image,unsigned char *o,long int dx,long int dy,
	unsigned char *cmap, unsigned char *itab,long int paltype);
void rgblookup(long int *image,unsigned char *o,long int dx,long int dy,
	 unsigned char *cmap, unsigned char *itab,long int paltype);
	 
	 
/* local defines */
#define VL_FIXED_TAB 1L
#define VL_INVERSE_TAB 0L

void vl_dither_(long int *image,long int dx,long int dy,unsigned char *output,
	unsigned char *lut,unsigned char *itab, long int type)
{
	long int i,j;

	switch (type) {
		case VL_GRAY:
			for(i=0;i<256;i++) {
				lut[i] = i;
				lut[i+256] = i;
				lut[i+512] = i;
			}
/* weight 30%red (77) 59%green (151) 11%blue (28) */
			for(i=0;i<dx*dy;i++) {
				j = (image[i] & 0xff) * 77;
				j += ((image[i] >> 8) & 0xff) * 151;
				j += ((image[i] >> 16) & 0xff) * 28;
				output[i] = (j >> 8) & 0xff;
			}
			break;

		case VL_COLOR:  /* currently not truely implemented */
			if (itab != 0L) {
				rgbdither(image,output,dx,dy,lut,itab,
					VL_INVERSE_TAB);
			} else {
				vl_puts("Warning : no inverse table.\n");
				makestdcmap_(lut);
				rgbdither(image,output,dx,dy,lut,0L,
					VL_FIXED_TAB);
			}
			break;

		case VL_FIXEDCOLOR:
			makestdcmap_(lut);
			rgbdither(image,output,dx,dy,lut,0L,VL_FIXED_TAB);
			break;
			
		case VL_COLORLOOKUP:
			if (itab != 0L) {
				rgblookup(image,output,dx,dy,lut,itab,
					VL_INVERSE_TAB);
			} else {
				makestdcmap_(lut);
				rgblookup(image,output,dx,dy,lut,0L,
					VL_FIXED_TAB);
			}
			break;
		     
	}
	return;
}

/* maximum linear error beyond which the 4 bit quant does just as good a job */
#define RGB_MAX_ERR 48  /* ie 16 levels each for r,g,b */ 

/* 
 *      color dithering
 */

long int colordist(i,r,g,b,cmap)
long int i, r, g, b;
unsigned char *cmap;
{
    register long int dist, dr, dg, db;

    dr = r-cmap[i];
    dg = g-cmap[i+256];
    db = b-cmap[i+512];
    dist = 0;
    if(dr<0) 
	dist -= dr;
    else
	dist += dr;
    if(dg<0) 
	dist -= dg;
    else 
	dist += dg;
    if(db<0) 
	dist -= db;
    else 
	dist += db;
    return dist;
}

long int rgbffindindex(r,g,b)
long int r,g,b;
{
    long int bwindex, rgbindex;
    long int bwdist, rgbdist;

    if(r>255) r = 255;
    if(g>255) g = 255;
    if(b>255) b = 255;
    if(r<0) r = 0;
    if(g<0) g = 0;
    if(b<0) b = 0;
    return red_inverse[r]+green_inverse[g]+blue_inverse[b];
} 

long int rgbfindindex(r,g,b,cmap,itab)
long int r,g,b;
unsigned char *cmap;
unsigned char *itab;
{
    long int i,rgbindex;
    long int dist, rgbdist;
    unsigned short *stab;
    unsigned char *vtab;
    long int color,low,high;

    stab = (unsigned short *)itab;
    vtab = itab + (4096*sizeof(short));

    if(r>255) r = 255;
    if(g>255) g = 255;
    if(b>255) b = 255;
    if(r<0) r = 0;
    if(g<0) g = 0;
    if(b<0) b = 0;

/* build 0000bbbbggggrrrr */
    color  = ((r & 0xf0) >> 4); 
    color |= (g & 0xf0); 
    color |= ((b & 0xf0) << 4); 

/* get the possible index list top and bottom */
    low = stab[color]+1;
    high = low + vtab[stab[color]];

/* prime to first on list */
    rgbindex = vtab[low];
    rgbdist = colordist(rgbindex,r,g,b,cmap);

/* look through other indexes on the list looking for the closest */
    for(i=low+1;i<high;i++) {
	dist = colordist(vtab[i],r,g,b,cmap);
	if (dist < rgbdist) {
		rgbindex = vtab[i];
		rgbdist = dist;
	}
    }

    return (rgbindex);
}

void vl_makeitable_(unsigned char *cmap, long int ncols, long int base,
	long int ntab, unsigned char **outtab,long int *tabsize)
{
	unsigned short *stab;
	unsigned char  *iout,*itab,*itab2;
	register long r,g,b,i,j,ptr,k,ins;
	long err[32],vals[32],dist;
	*tabsize = -1;

	itab = malloc((4096*sizeof(unsigned short))+((ntab+1)*4096));
	if (itab == NULL) {
	    *outtab = NULL;
	    return;
	}
	stab = (unsigned short *)itab;

	iout = itab + (4096*sizeof(unsigned short));
	ptr = 0;
/* i = 0000bbbbggggrrrr */
/* for each 'quadrant' */
	for(i=0;i<4096;i++) {
/* point at the sub itab */
		stab[i] = ptr;

/* get the RGB value */
		r = ((i & 0xf  ) << 4);
		g = ((i & 0xf0 )     );
		b = ((i & 0xf00) >> 4);
/* at the center of the box */
		r |= 0x08;
		g |= 0x08;
		b |= 0x08;

/* zero out the search table */
		for(j=0;j<ntab;j++) {
			vals[j] = 0;
			err[j] = -1;
		}
/* search the colors for the ntab closest neighbors */
		for(j=base;j<base+ncols;j++) {
			dist = colordist(j,r,g,b,cmap);
/* is it in the ntab best? */
			k = 0;
			ins = -1;
			for(k=0;k<ntab;k++) {
				if ((err[k] == -1) || (err[k] > dist)) {
					ins = k;
					break;  
				}
			}
/* if so, insert the element */
			if (ins != -1) {
				for(k=ntab-1;k>ins;k--) {
					err[k] = err[k-1];
					vals[k] = vals[k-1];
				}
				err[ins] = dist;
				vals[ins] = j;
			}
		}
/* init the subitab count (always at least one entry) */
		k = ptr;
		iout[ptr++] = 1;
/* fill out the first subitab entry */
		iout[ptr++] = vals[0];
/* add others as tolerance allows */
		for(j=1;j<ntab;j++) {
			if (err[j] < RGB_MAX_ERR) {
				iout[ptr++] = vals[j];
				iout[k] += 1;
			}
		}
	}

/* copy the shortened table (index table + subitabs) */
/* since index table pointers are all relative we can just copy */
	j = (4096*sizeof(unsigned short))+(ptr);
	itab2 = malloc(j);
	if (itab2 == NULL) {
	    free(itab);
	    *outtab = NULL;
	    return;
	}
	
	for(i=0;i<j;i++) itab2[i] = itab[i];
	*(outtab) = itab2;
	*tabsize = j;

/* free up the temp table */
	free(itab);

	return;
}

void makestdcmap_(unsigned char *cmap)
{
	int i;
/* fixed map (with 256 entries) */
	for(i=0;i<256;i++) {
		cmap[i] = red_map[i];
		cmap[i+256] = green_map[i];
		cmap[i+512] = blue_map[i];
	}
	return;
}

void rgbdither(long int *image,unsigned char *o,long int dx,long int dy,
	 unsigned char *cmap, unsigned char *itab,long int paltype)
{
    register short i,j;
    register long int rwant, error;
    register long int gwant;
    register long int bwant;
    long int rerror, rnext;
    long int gerror, gnext;
    long int berror, bnext;
    long int index, temp;
    long int mylow, myhigh;

    register long *pixel;

    short rerr[4096];
    short gerr[4096];
    short berr[4096];

    for(i=0;i<4096;i++) rerr[i] = gerr[i] = berr[i] = 0;

	rerror = 0;
	rnext = rerr[0] = 0;
	gerror = 0;
	gnext = gerr[0] = 0;
	berror = 0;
	bnext = berr[0] = 0;

    for(j=0; j<dy; j++) {
#ifdef DEBUG
	if ((j & 0xf) == 0) printf("Working on %d of %d ...\n",j,dy); 
#endif
	pixel = &(image[j*dx]);
	for(i=0; i<dx; i++ ) {
	    rwant = ((*pixel      ) & 0xff) + rerror + rnext;
	    gwant = ((*pixel >>  8) & 0xff) + gerror + gnext;
	    bwant = ((*pixel >> 16) & 0xff) + berror + bnext;
	    pixel++;

	    if (paltype == VL_FIXED_TAB) {
/* high speed lookup */
		*o++ = index = rgbffindindex(rwant,gwant,bwant);
	    } else {
/* full featured lookup */
		*o++ = index =rgbfindindex(rwant,gwant,bwant,cmap,itab);
	    }

/* propagate error terms */
	    error =  rwant-cmap[index];
	    rerror = (3*error)/8;
	    rerr[i] += rerror;
	    rnext = rerr[i+1];
	    rerr[i+1] = error-2*rerror;

	    error =  gwant-cmap[index+256];
	    gerror = (3*error)/8;
	    gerr[i] += gerror;
	    gnext = gerr[i+1];
	    gerr[i+1] = error-2*gerror;

	    error =  bwant-cmap[index+512];
	    berror = (3*error)/8;
	    berr[i] += berror;
	    bnext = berr[i+1];
	    berr[i+1] = error-2*berror;
	}
    }
    return;
}

void rgblookup(long int *image,unsigned char *o,long int dx,long int dy,
	 unsigned char *cmap, unsigned char *itab,long int paltype)
{
    register short i,j;
    register long int rwant;
    register long int gwant;
    register long int bwant;
    register long *pixel;



    for(j=0; j<dy; j++) {
	pixel = &(image[j*dx]);
	for(i=0; i<dx; i++ ) {
	    rwant = ((*pixel      ) & 0xff);
	    gwant = ((*pixel >>  8) & 0xff);
	    bwant = ((*pixel >> 16) & 0xff);
	    pixel++;
	    if (paltype == VL_FIXED_TAB) {
/* high speed lookup */
		*o++  = rgbffindindex(rwant,gwant,bwant);
	    } else {
/* full featured lookup */
		*o++  = rgbfindindex(rwant,gwant,bwant,cmap,itab);
	    }

	}
    }
    return;
}

