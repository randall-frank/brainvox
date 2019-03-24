#include "voxel.h"
#include "vl_util.h"
#include <stdio.h>
#include <stdlib.h>

#include "colortbl.inc"

#ifndef min
#define min( a, b ) ( a < b ? a : b )
#endif

#ifndef max
#define max( a, b ) ( a > b ? a : b )
#endif


#ifndef FREE
#define FREE(A) free(A)
#endif

#ifndef MALLOC
#define MALLOC(A) malloc(A)
#endif

/* declare local functions */
void makestdcmap_(VLUCHAR *cmap);
VLINT32 colordist(VLINT32 i,VLINT32 r,VLINT32 g,VLINT32 b,VLUCHAR *cmap);
VLINT32 rgbffindindex(VLINT32 r,VLINT32 g,VLINT32 b);
VLINT32 rgbfindindex(VLINT32 r,VLINT32 g,VLINT32 b,VLUCHAR *cmap,
	VLUCHAR *itab);
void rgbdither(VLINT32 *image,VLUCHAR *o,VLINT32 dx,VLINT32 dy,
	VLUCHAR *cmap, VLUCHAR *itab,VLINT32 paltype);
void rgblookup(VLINT32 *image,VLUCHAR *o,VLINT32 dx,VLINT32 dy,
	 VLUCHAR *cmap, VLUCHAR *itab,VLINT32 paltype);
	 
	 
/* local defines */
#define VL_FIXED_TAB 1L
#define VL_INVERSE_TAB 0L

static VLINT32 vl_dither_clamp = 32;


void vl_dither_setclamp( VLINT32 value ){
	vl_dither_clamp = value;
	if ( vl_dither_clamp < 0 )  vl_dither_clamp = 0;
	if ( vl_dither_clamp > 255 ) vl_dither_clamp = 255;
		
}

void vl_dither_(VLINT32 *image,VLINT32 dx,VLINT32 dy,VLUCHAR *output,
	VLUCHAR *lut,VLUCHAR *itab, VLINT32 type)
{
	VLINT32 i,j;

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

VLINT32 colordist(VLINT32 i,VLINT32 r, VLINT32 g,VLINT32 b,VLUCHAR *cmap)
{
    register VLINT32 dist, dr, dg, db;

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

VLINT32 rgbffindindex(VLINT32 r,VLINT32 g,VLINT32 b)
{

    if(r>255) r = 255;
    if(g>255) g = 255;
    if(b>255) b = 255;
    if(r<0) r = 0;
    if(g<0) g = 0;
    if(b<0) b = 0;
    return red_inverse[r]+green_inverse[g]+blue_inverse[b];
} 

VLINT32 rgbfindindex(VLINT32 r,VLINT32 g,VLINT32 b,VLUCHAR *cmap,VLUCHAR *itab)
{
    VLINT32 i,rgbindex;
    VLINT32 dist, rgbdist;
    VLUINT16 *stab;
    VLUCHAR *vtab;
    VLINT32 color,low,high;

    stab = (VLUINT16 *)itab;
    vtab = itab + (4096*sizeof(VLINT16));

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

void vl_makeitable_(VLUCHAR *cmap, VLINT32 ncols, VLINT32 base,
	VLINT32 ntab, VLUCHAR **outtab,VLINT32 *tabsize)
{
	VLUINT16 *stab;
	VLUCHAR  *iout,*itab,*itab2;
	register VLINT32 r,g,b,i,j,ptr,k,ins;
	VLINT32 err[32],vals[32],dist;
	*tabsize = -1;

	itab = MALLOC((4096*sizeof(VLUINT16))+((ntab+1)*4096));
	if (itab == NULL) {
	    *outtab = NULL;
	    return;
	}
	stab = (VLUINT16 *)itab;

	iout = itab + (4096*sizeof(VLUINT16));
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
	j = (4096*sizeof(VLUINT16))+(ptr);
	itab2 = MALLOC(j);
	if (itab2 == NULL) {
	    FREE(itab);
	    *outtab = NULL;
	    return;
	}
	
	for(i=0;i<j;i++) itab2[i] = itab[i];
	*(outtab) = itab2;
	*tabsize = j;

/* free up the temp table */
	FREE(itab);

	return;
}


void vl_makeitable_ex_(VLUCHAR *cmap, 
  		   VLUCHAR *mask,
		   VLINT32 ncols,
		   VLINT32 base,
		   VLINT32 ntab, 
 		   VLUCHAR **outtab,
		   VLINT32 *tabsize)
{
	VLUINT16 *stab;
	VLUCHAR  *iout,*itab,*itab2;
	register VLINT32 r,g,b,i,j,ptr,k,ins;
	VLINT32 err[32],vals[32],dist,closest;
	*tabsize = -1;

	itab = MALLOC((4096*sizeof(VLUINT16))+((ntab+1)*4096));
	if (itab == NULL) {
	    *outtab = NULL;
	    return;
	}
	stab = (VLUINT16 *)itab;

	iout = itab + (4096*sizeof(VLUINT16));
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
		for(j=0;j<ncols;j++) {
			if ( mask[j] ) {
				dist = colordist(j,r,g,b,cmap);
/* is it in the ntab best? */
				k = 0;
				ins = -1;
				closest = 0x0FFFFFFL;
				for(k=0;k<ntab;k++) {
					if (err[k] == -1) { /* no entries at all */
						ins = k;
						break;
					}
					else
					if (err[k] > dist) {
						if (( err[k] - dist ) < closest ) {
							 ins = k;
							 closest = err[k] - dist;
						}  
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
	j = (4096*sizeof(VLUINT16))+(ptr);
	itab2 = MALLOC(j);
	if (itab2 == NULL) {
	    FREE(itab);
	    *outtab = NULL;
	    return;
	}
	
	for(i=0;i<j;i++) itab2[i] = itab[i];
	*(outtab) = itab2;
	*tabsize = j;

/* free up the temp table */
	FREE(itab);

	return;
}

void makestdcmap_(VLUCHAR *cmap)
{
	VLINT32 i;
/* fixed map (with 256 entries) */
	for(i=0;i<256;i++) {
		cmap[i] = red_map[i];
		cmap[i+256] = green_map[i];
		cmap[i+512] = blue_map[i];
	}
	return;
}

void rgbdither(VLINT32 *image,VLUCHAR *o,VLINT32 dx,VLINT32 dy,
	 VLUCHAR *cmap, VLUCHAR *itab,VLINT32 paltype)
{
    register VLINT16 i,j;
    register VLINT32 rwant, error;
    register VLINT32 gwant;
    register VLINT32 bwant;
	register VLINT32 clampvalue;
    VLINT32 rerror, rnext;
    VLINT32 gerror, gnext;
    VLINT32 berror, bnext;
    VLINT32 index;
    

    register VLINT32 *pixel;

    VLINT16 rerr[4096];
    VLINT16 gerr[4096];
    VLINT16 berr[4096];

	VLCHAR * clampval = NULL;
	

	
	

	clampvalue = vl_dither_clamp;

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
		error =  rerror + rnext;
		error = min(max(-clampvalue,error),clampvalue);
	    rwant = ((*pixel      ) & 0xff) + error;
		error = gerror + gnext;
		error = min(max(-clampvalue,error),clampvalue);
	    gwant = ((*pixel >>  8) & 0xff) + error;
		error =  berror + bnext;
		error = min(max(-clampvalue,error),clampvalue);
	    bwant = ((*pixel >> 16) & 0xff) + error;
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

void rgblookup(VLINT32 *image,VLUCHAR *o,VLINT32 dx,VLINT32 dy,
	 VLUCHAR *cmap, VLUCHAR *itab,VLINT32 paltype)
{
    register VLINT16 i,j;
    register VLINT32 rwant;
    register VLINT32 gwant;
    register VLINT32 bwant;
    register VLINT32 *pixel;



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

