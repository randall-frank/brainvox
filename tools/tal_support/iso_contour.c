/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski
 *
 * $Id: iso_contour.c 1673 2006-03-07 02:27:43Z rjfrank $
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
#include "iso_tab.h"
#include "proto.h"

#ifndef MALLOC
#define MALLOC malloc
#endif
#ifndef REALLOC
#define REALLOC realloc
#endif
#ifndef FREE
#define FREE free
#endif

/* generally visible routines */
/* routine to compute a polygonal surface approximating an isocontour
	value in a voxel slice set */
long int slice_isocontour(unsigned char *data,unsigned char *mask,
	long int dx,long int dy,long int count,double *ipixel,double islice,
	double *sa,double *vol);

/* routine for full masked mcubes */
long int vol_isocontour(void *data,unsigned char *mask,long int dx,long int dy,
	long int dz,long int depth,double thres,double *ipixel,double islice,
	double *sa,long int *n_verts,float **verts,float **norms,
	long int *n_tris,long int **tris);
/* local data */

/*
	from iso_tab.h:
static short iso_tab[256][15];
*/

/* how to get from an edge number (iso_tab) to a vnum.  These are
	the deltas and edges(0-2) from a cube point p[3]. */
static short delta[12][4] = {
	{0,0,0, 0},
	{0,0,0, 1},
	{0,1,0, 0},
	{1,0,0, 1},
	{0,0,0, 2},
	{0,1,0, 2},
	{1,1,0, 2},
	{1,0,0, 2},
	{0,0,1, 0},
	{0,0,1, 1},
	{0,1,1, 0},
	{1,0,1, 1}
};

/* local structures */

/* local prototypes */
double  tri3d_area(double *i0,double *i1,double *i2,double *r);

void	get_vertex(long int x,long int y,long int z,long int ed,
		void *data,double thres,long int dx,long int dy,
		long int dz,double *v);
void build_gauss(float *gauss,long int *radius,long int cutoff);
float	get_d_value(void *data,long int dz,long int i);
long int hash_index(long int n,long int *table,long int val);

/* routines begin */
float	get_d_value(void *data,long int dz,long int i)
{
	float v = 0;

	switch(dz) {
		case 1:
			v = ((unsigned char *)data)[i];
			break;
		case 2:
			v = ((unsigned short *)data)[i];
			break;
		case 4:
			v = ((float *)data)[i];
			break;
	}
	return(v);
}


/* routine to get a vertex from an edge reference (x,y,z,edge) */
/* edge 0=xaxis 1=yaxis 2=zaxis */
void	get_vertex(long int x,long int y,long int z,long int ed,
		void *data,double thres,long int dx,long int dy,long int dz,
		double *v)
{
	double		v1,v2;
	long int	i;

/* get the index */
	i = x + (y*dx) + (z*dx*dy);

/* get base value */
	v1 = get_d_value(data,dz,i);

/* pointer to second value */
	switch(ed) {
		case 0:
			i += (1);
			break;
		case 1:
			i += (dx);
			break;
		case 2:
			i += (dx*dy);
			break;
	}

/* get second value */
	v2 = get_d_value(data,dz,i);

/* set the vertex base */
	v[0] = x;
	v[1] = y;
	v[2] = z;

/* add in the direction of the edge */
	if (v2 != v1) {
		v[ed] += (thres-v1)/(v2-v1);
	}

	return;
}

/* routine to compute a polygonal surface approximating an isocontour
	value in a voxel slice set */
/* returns:0=no error,1=user abort,other error */

/* data is a pointer to count dxXdy 8bit image planes with values 0 and 255 */
/* mask is a pointer to count dxXdy 8bit masks with 0 outside the area of 
	interest */
/* surface area is added to *sa */
long int slice_isocontour(unsigned char *data,unsigned char *mask,
	long int dx,long int dy,long int count,double *ipixel,double islice,
	double *sa,double *vol)
{
	long int	dxy,b;
	long int	radius;
	long int	x,y,z,i,j,k,l,ptr,ptr2;
	long int	rad[3],valid,rim,nvalid;
	unsigned char	*smooth,*smask;
	float		*kern;
	double		v0[3],v1[3],v2[3],norm[3],conv[3];
	double		sum;
	double		surf_area = 0.0;
	double		volume = 0.0;
	double		thres = 128.0 + 0.001;

/* valid data */
	if (data == 0L) return(2);
	if (mask == 0L) return(2);

	smooth = (unsigned char *)MALLOC(2*dy*dx);
	if (smooth == 0) return(2);
	smask = (unsigned char *)MALLOC(2*dy*dx);
	if (smask == 0) {
		FREE(smooth);
		return(2);
	}
	
/* various const vars */
	dxy = dx*dy;
	radius = (count - 2)/2;  /* (slices-2)/2 */
	conv[0] = ipixel[0];
	conv[1] = ipixel[1];
	conv[2] = islice;

/* get a gaussian kernal */
	i = (radius*2 + 1)*(radius*2 + 1)*(radius*2 + 1);
	kern = (float *)MALLOC(sizeof(float)*i);
	if (kern == 0) {
		FREE(smooth);
		FREE(smask);
		return(2);
	}
	nvalid = 0.30*i;
	if (radius == 0) {
		kern[0] = 1.0;
		kern[1] = 1.0;
	} else {
		rad[0] = radius; rad[1] = radius; rad[2] = radius;
		build_gauss(kern,rad,radius);
	}

#ifdef DEBUG
printf("icount=%ld radius=%ld ip=%lf is=%lf\n",count,radius,ipixel[0],islice);
printf("smoothing\n");
#endif

/* smooth the slices (if the mask is touched by the kernal) */
	for(z=radius;z<=(radius+1);z += 1) {
	for(y=radius;y<(dy-radius)-1;y += 1) {
	for(x=radius;x<(dx-radius)-1;x += 1) {
		ptr = (x) + (y*dx) + ((z-radius)*dxy);
		sum = 0.0;
		l = (x)+(y*dx)+(z*dxy);
		ptr2 = 1;  /* kern[0] is the sum */
		valid = 0;
		rim = 0;
/* smooth the data */
		for(k=-radius;k<=radius;k++) {
		for(j=-radius;j<=radius;j++) {
		for(i=-radius;i<=radius;i++) {
			if (data[l+i+(j*dx)+(k*dxy)] > thres) valid++;
			if (mask[l+i+(j*dx)+(k*dxy)] != 0) rim += 1;
			sum += data[l+i+(j*dx)+(k*dxy)]*kern[ptr2];
			ptr2++;
		}
		}
		}
/* we are outside */
		smask[ptr] = 0;
/* if in the mask, we are in */
		if (mask[l] != 0) {
			smask[ptr] = 1;
/* if some other part of the kernal hits the mask && the number of >thres 
	voxels is less than 25%, then we are in */
		} else if ((mask[l] == 0) && (rim) && (valid < nvalid)) {
			smask[ptr] = 1;
		}
/* normalize the smooth */
		smooth[ptr] = sum/kern[0];
	}
	}
	}

#ifdef DEBUG
printf("searching\n");
#endif

/* scan Z */
	z = 0;
/* scan Y */
	for(y=radius;y<(dy-radius)-2;y += 1) {
/* scan X */
	for(x=radius;x<(dx-radius)-2;x += 1) {
/* build the index byte */
		b = 0;
		j = (y*dx) + (x);
		if (smooth[j] > thres) b |= 1;
		if (smooth[j+1] > thres) b |= 2;
		if (smooth[j+dx] > thres) b |= 4;
		if (smooth[j+dx+1] > thres) b |= 8;
		if (smooth[j+dxy] > thres) b |= 16;
		if (smooth[j+dxy+1] > thres) b |= 32;
		if (smooth[j+dxy+dx] > thres) b |= 64;
		if (smooth[j+dxy+dx+1] > thres) b |= 128;
/* check validity. Is this point in the smoothed masks? */
		if (smask[j] == 0) continue; 
/* add volume */
		if (smooth[j] > thres) volume += 1;
#ifdef DEBUG
		if (iso_tab[b][0] != -1) {
		printf("Cube: %ld %ld %ld : %ld\n",x,y,z,b);
		}
#endif
/* scan the polys for this cube looking for the -1 terminator */
		j = 0;
		while (iso_tab[b][j] != -1) {
/* we have a triangle */
#ifdef DEBUG
			printf("%ld %ld %ld\n",iso_tab[b][j],iso_tab[b][j+1],
				iso_tab[b][j+2]);
#endif
/* get the verticies */
			get_vertex(x+delta[iso_tab[b][j+0]][0],
				   y+delta[iso_tab[b][j+0]][1],
				   z+delta[iso_tab[b][j+0]][2],
				   delta[iso_tab[b][j+0]][3],
				   smooth,thres,dx,dy,1,v0);
			get_vertex(x+delta[iso_tab[b][j+1]][0],
				   y+delta[iso_tab[b][j+1]][1],
				   z+delta[iso_tab[b][j+1]][2],
				   delta[iso_tab[b][j+1]][3],
				   smooth,thres,dx,dy,1,v1);
			get_vertex(x+delta[iso_tab[b][j+2]][0],
				   y+delta[iso_tab[b][j+2]][1],
				   z+delta[iso_tab[b][j+2]][2],
				   delta[iso_tab[b][j+2]][3],
				   smooth,thres,dx,dy,1,v2);
/* convert to real world values */
			for(k=0;k<3;k++) v0[k] = v0[k]*conv[k];
			for(k=0;k<3;k++) v1[k] = v1[k]*conv[k];
			for(k=0;k<3;k++) v2[k] = v2[k]*conv[k];

/* compute the polygon SA */
			surf_area += tri3d_area(v0,v1,v2,norm);

/* next triangle in the table */
			j += 3;

		} /* isotab values */
	} /* X loop */
	} /* Y loop */
	  /* Z loop */

	FREE(smooth);
	FREE(smask);
	FREE(kern);

	*sa += surf_area;
	*vol += (volume*ipixel[0]*ipixel[1]*islice);

        return(0);
}

double  tri3d_area(double *i0,double *i1,double *i2,double *r)
{
        double  area,v1[3],v2[3];
        int     j;
/*
C       assume bad
*/
        area = 0.0;
/*
C
C	convert to vector form
C
*/
	for(j=0;j<3;j++) v1[j] = i0[j] - i1[j];
	for(j=0;j<3;j++) v2[j] = i2[j] - i1[j];
/*
C       compute X prod          0   1   2
C                               v10 v11 v12 v10 v11
C                               v20 v21 v22 v20 v21
*/
        r[0] = ((v1[1]*v2[2]) - (v1[2]*v2[1]));
        r[1] = ((v1[2]*v2[0]) - (v1[0]*v2[2]));
        r[2] = ((v1[0]*v2[1]) - (v1[1]*v2[0]));
/*
C       1/2 length of resultant vector is the area
*/
        area = r[0]*r[0] + r[1]*r[1] + r[2]*r[2];
        if (area != 0.0) {
                area = sqrt(r[0]*r[0] + r[1]*r[1] + r[2]*r[2]);
/*     
C       Normalize the vector
*/
                for(j=0;j<3;j++) {
                        r[j] = r[j]/area;
                }
        }
        area = area*0.5;

        return(area);
}


void build_gauss(float *gauss,long int *radius,long int cutoff)
{
        long int        i,j,k,m;
        double          r,s;

/* compute sigma for FWHM (=0.5) at cutoff */
        s = -(double)(cutoff*cutoff)/(2*log(0.5));

        m = 1;
        gauss[0] = 0;
        for(k=-radius[2];k<=radius[2];k++) {
        for(j=-radius[1];j<=radius[1];j++) {
        for(i=-radius[0];i<=radius[0];i++) {
                r = (k*k) + (j*j) + (i*i);
                gauss[m] = 255.0*exp(-(r/(2*s))); /*scaled by 255 for int work*/
                gauss[0] += gauss[m];
                m++;
        }
        }
        }
        return;
}

long int hash_index(long int n,long int *table,long int val)
{
	long int	bot = 0;
	long int	top = n-1;
	long int	mid;

	if (table[bot] == val) return(bot);
	if (table[top] == val) return(top);
	while(1) {
		mid = (bot+top)/2;
		if (table[mid] == val) {
			return(mid);
		} else if (table[mid] > val) {
			top = mid;
		} else {
			bot = mid;
		}
		if (top - bot <= 1) return(-1);
	}
}

/* Full volume isocontour w/output mesh */
long int vol_isocontour(void *data,unsigned char *mask,long int dx,long int dy,
	long int dz,long int depth,double thres,double *ipixel,double islice,
	double *sa, long int *n_verts,float **i_verts,float **i_norms,
	long int *n_tris,long int **i_tris)
{
	long int	ntris,ntris_alloc;
	long int	nverts,nverts_alloc;
	long int	*tris=0L;
	long int	*hashv=0l;
	float		*verts=0L;
	float		*norms=0L;
	long int	x,y,z,i,j,b,k;
	long int	hash;
	float		f1,f2;
	double		v[3],v0[3],v1[3],v2[3];

/* pass 1: collect verticies */
	nverts=0;
	nverts_alloc=2000;
	verts =(float *)MALLOC(nverts_alloc*3*sizeof(float));
	if (verts == 0L) goto err_out;
	hashv = (long int *)MALLOC(nverts_alloc*sizeof(long int));
	if (hashv == 0L) goto err_out;

	for(z=0;z<dz;z++) {
	for(y=0;y<dy;y++) {
	for(x=0;x<dx;x++) {
	    j=x+(y*dx)+(z*dx*dy);
	    if (mask[j]) {
		f1 = get_d_value(data,depth,j);
		if ((x < (dx-1)) && mask[j+1]) {
			f2 = get_d_value(data,depth,j+1);
			if ((f2-thres)*(f1-thres) <= 0) {
				get_vertex(x,y,z,0,data,thres,dx,dy,depth,v);
				verts[(nverts*3)+0] = v[0]*ipixel[0];
				verts[(nverts*3)+1] = v[1]*ipixel[1];
				verts[(nverts*3)+2] = v[2]*islice;
				hashv[nverts] = (x+(y*dx)+(z*dx*dy))*3 + 0;
#ifdef DEBUG
			printf("Hash=%ld,%ld\n",nverts,hashv[nverts]);
#endif
				nverts += 1;
			}
		}
		if ((y < (dy-1)) && mask[j+dx]) {
			f2 = get_d_value(data,depth,j+dx);
			if ((f2-thres)*(f1-thres) <= 0) {
				get_vertex(x,y,z,1,data,thres,dx,dy,depth,v);
				verts[(nverts*3)+0] = v[0]*ipixel[0];
				verts[(nverts*3)+1] = v[1]*ipixel[1];
				verts[(nverts*3)+2] = v[2]*islice;
				hashv[nverts] = (x+(y*dx)+(z*dx*dy))*3 + 1;
#ifdef DEBUG
			printf("Hash=%ld,%ld\n",nverts,hashv[nverts]);
#endif
				nverts += 1;
			}
		}
		if ((z < (dz-1)) && mask[j+(dx*dy)]) {
			f2 = get_d_value(data,depth,j+(dx*dy));
			if ((f2-thres)*(f1-thres) <= 0) {
				get_vertex(x,y,z,2,data,thres,dx,dy,depth,v);
				verts[(nverts*3)+0] = v[0]*ipixel[0];
				verts[(nverts*3)+1] = v[1]*ipixel[1];
				verts[(nverts*3)+2] = v[2]*islice;
				hashv[nverts] = (x+(y*dx)+(z*dx*dy))*3 + 2;
#ifdef DEBUG
			printf("Hash=%ld,%ld\n",nverts,hashv[nverts]);
#endif
				nverts += 1;
			}
		}
	    }
/* increase allocations if needed */
	    if (nverts_alloc-nverts < 4) {
		nverts_alloc += 1000;
		verts = (float *)REALLOC(verts,nverts_alloc*3*sizeof(float));
		if (verts == 0L) goto err_out;
		hashv=(long int *)REALLOC(hashv,nverts_alloc*sizeof(long int));
		if (hashv == 0L) goto err_out;
	    }
	}
	}
	}

/* pass 2: collect triangles */
	norms =(float *)MALLOC(nverts*3*sizeof(float));
	if (norms == 0L) goto err_out;
	for(j=0;j<nverts*3;j++) norms[j] = 0;
	ntris=0;
	ntris_alloc=2000;
	tris = (long int *)MALLOC(ntris_alloc*3*sizeof(long int));
	if (tris == 0L) goto err_out;
/* scan Z */
	for(z=0;z<dz-1;z+=1) {
/* scan Y */
	for(y=0;y<dy-1;y+=1) {
/* scan X */
	for(x=0;x<dx-1;x+=1) {
/* build the index byte */
		b = 0;
		j = (z*dx*dy) + (y*dx) + (x);
		if (get_d_value(data,depth,j) > thres) b |= 1;
		if (get_d_value(data,depth,j+1) > thres) b |= 2;
		if (get_d_value(data,depth,j+dx) > thres) b |= 4;
		if (get_d_value(data,depth,j+dx+1) > thres) b |= 8;
		if (get_d_value(data,depth,j+dx*dy) > thres) b |= 16;
		if (get_d_value(data,depth,j+dx*dy+1) > thres) b |= 32;
		if (get_d_value(data,depth,j+dx*dy+dx) > thres) b |= 64;
		if (get_d_value(data,depth,j+dx*dy+dx+1) > thres) b |= 128;
/* check validity. Are all points in the mask? */
		if (mask[j] == 0) continue; 
		if (mask[j+1] == 0) continue; 
		if (mask[j+dx] == 0) continue; 
		if (mask[j+dx+1] == 0) continue; 
		if (mask[j+dx*dy] == 0) continue; 
		if (mask[j+dx*dy+1] == 0) continue; 
		if (mask[j+dx*dy+dx] == 0) continue; 
		if (mask[j+dx*dy+dx+1] == 0) continue; 
#ifdef DEBUG
		if (iso_tab[b][0] != -1) {
		printf("Cube: %ld %ld %ld : %ld\n",x,y,z,b);
		}
#endif
/* scan the polys for this cube looking for the -1 terminator */
		j = 0;
		while (iso_tab[b][j] != -1) {
/* we have a triangle */
#ifdef DEBUG
			printf("%ld %ld %ld\n",iso_tab[b][j],iso_tab[b][j+1],
				iso_tab[b][j+2]);
#endif
/* get the verticies */
			hash = (x+delta[iso_tab[b][j+0]][0])*3;
			hash += (y+delta[iso_tab[b][j+0]][1])*dx*3;
			hash += (z+delta[iso_tab[b][j+0]][2])*dx*dy*3;
			hash += (delta[iso_tab[b][j+0]][3]);
			k = hash_index(nverts,hashv,hash);
#ifdef DEBUG
			printf("Hash=%ld\n",hash);
#endif
			if (k == -1) ex_err("Invalid hash value detected");
			tris[(ntris*3)+0] = k;

			hash = (x+delta[iso_tab[b][j+1]][0])*3;
			hash += (y+delta[iso_tab[b][j+1]][1])*dx*3;
			hash += (z+delta[iso_tab[b][j+1]][2])*dx*dy*3;
			hash += (delta[iso_tab[b][j+1]][3]);
			k = hash_index(nverts,hashv,hash);
#ifdef DEBUG
			printf("Hash=%ld\n",hash);
#endif
			if (k == -1) ex_err("Invalid hash value detected");
			tris[(ntris*3)+1] = k;

			hash = (x+delta[iso_tab[b][j+2]][0])*3;
			hash += (y+delta[iso_tab[b][j+2]][1])*dx*3;
			hash += (z+delta[iso_tab[b][j+2]][2])*dx*dy*3;
			hash += (delta[iso_tab[b][j+2]][3]);
			k = hash_index(nverts,hashv,hash);
#ifdef DEBUG
			printf("Hash=%ld\n",hash);
#endif
			if (k == -1) ex_err("Invalid hash value detected");
			tris[(ntris*3)+2] = k;

			for(k=0;k<3;k++) {
				v0[k] = verts[(tris[(ntris*3)+0]*3)+k];
				v1[k] = verts[(tris[(ntris*3)+1]*3)+k];
				v2[k] = verts[(tris[(ntris*3)+2]*3)+k];
			}

/* compute the polygon SA */
			*sa += tri3d_area(v0,v1,v2,v);

/* add the normal to the vertex norm acc */
			for(k=0;k<3;k++) {
				norms[(tris[(ntris*3)+0]*3)+k] += v[k];
				norms[(tris[(ntris*3)+1]*3)+k] += v[k];
				norms[(tris[(ntris*3)+2]*3)+k] += v[k];
			}

/* done adding a triangle */
			ntris += 1;

/* increase allocations if needed */
	    		if (ntris_alloc-ntris < 4) {
				ntris_alloc += 1000;
				tris = (long int *)REALLOC(tris,
					ntris_alloc*3*sizeof(long int));
				if (tris == 0L) goto err_out;
			}

/* next triangle in the table */
			j += 3;

		} /* isotab values */
	} /* X loop */
	} /* Y loop */
	} /* Z loop */

/* renormalize the normals */
	for(j=0;j<nverts;j++) {	
		f1  = norms[(j*3)+0]*norms[(j*3)+0];
		f1 += norms[(j*3)+1]*norms[(j*3)+1];
		f1 += norms[(j*3)+2]*norms[(j*3)+2];
		if (f1 > 0) {
			f1 = sqrt(f1);
			norms[(j*3)+0] /= f1;
			norms[(j*3)+1] /= f1;
			norms[(j*3)+2] /= f1;
		} else {
			norms[(j*3)+0] = 0;
			norms[(j*3)+1] = 0;
			norms[(j*3)+2] = 1;
		}
	}

/* there can be unused verts, because of the handling of "=" cases,
   remove them here */
/* what verts are being used? */
	memset(hashv,0,sizeof(long int)*nverts);
	for(i=0;i<ntris*3;i++) if (!hashv[tris[i]]) hashv[tris[i]] = 1;
/* repack them, skipping unused and creating a remapping table */
	j = 0;
	for(i=0;i<nverts;i++) {
	    if (hashv[i] && (i != j)) {
		memcpy(verts+j*3,verts+i*3,3*sizeof(float));
		hashv[i] = j;
		j += 1;
            }
	}
	nverts = j;
/* remap the triangles */
	for(i=0;i<ntris*3;i++) tris[i] = hashv[tris[i]];

/* return the fruits of our labor */
	*n_verts = nverts;
	*n_tris = ntris;
	*i_verts = verts;
	*i_norms = norms;
	*i_tris = tris;
	if (hashv) FREE(hashv);

	return(0);

/* abnormal termination */
err_out:
	if (verts) FREE(verts);
	if (tris) FREE(tris);
	if (norms) FREE(norms);
	if (hashv) FREE(hashv);

	return(1);
}
