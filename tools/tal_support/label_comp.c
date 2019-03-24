/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski
 *
 * $Id: label_comp.c 1250 2005-09-16 15:51:42Z dforeman $
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
#include "label_comp.h"

/* local prototypes */
static void relabel(unsigned char *data,long int last,long int *remap,
	long int *comp);

/* Routine to compute a labeled connectivity map from a binary map 
	data[] - pointer to the map (packed form) with values of 0 and non-0
	dim - dimensionality of the data
	d[] - the length of each dimension
	nconn - number of offset vector entries in conn
	conn - the offset vector list (nconn*dim ints)  each vector specifies
		the vector offset to a neighbor which must be tested
	ncomp - returns the # of components (is negative if an error occurred)
	
	Note: the routine contains dynamic remapping code to allow for more
		than 256 uncollapsed components, but the maximum number of
		allowed components is 256-2.
*/

#define MAX_DIM 10

void label_components_(unsigned char *data,long int dim,long int *d,
	 long int nconn,long int *conn,long int *ncomp)
{
	long int	i,j,k,ptr;
	long int	r[MAX_DIM];  /* vector of location in the volume */
	long int	inc[MAX_DIM];/* vector of axis increments */
	long int	vsize;	     /* size of the volume */
	long int	remap[256];  /* component remapping table */
	long int	comp;	     /* current number of components */
	long int	nei,labels[256];  /* # of labeled neighbors & lbls */

/* get the dataset size and dimensional increments */
	*ncomp = -1;  /* assume error */
	if (dim > MAX_DIM) return;
	vsize = 1;
	inc[0] = vsize;
	for(i=0;i<dim;i++) {
		vsize *= d[i];
		inc[i+1] = vsize;
		r[i] = 0;
	}

/* make sure the data is binary */
	for(i=0;i<vsize;i++) if (data[i] > 0) data[i] = 1;

/* init the label remapping list */
	remap[0] = 0;  remap[1] = 1;
	comp = 2;

/* start the labeling process */
	for(i=0;i<vsize;i++) {
/* is an object voxel? */
		if (data[i] == 1) {
/* potential new label */
			nei = 0;
			labels[255] = 0;
/* count labeled neighbors (and save the min label as labels[255]) */
			for(j=0;j<(nconn*dim);j+=dim) {
				ptr = i;
/* build up the location of the neighbor point */
				for(k=0;k<dim;k++) {
/* out of bounds? */
					if (((conn[j+k] + r[k]) < 0)  ||
					   ((conn[j+k] + r[k]) >= d[k])) {
						ptr = -1;
						break;
					} else {
/* add the offset vector to the current location */
						 ptr += (conn[j+k]*inc[k]);
					}
				}
/* was the location legal (in bounds) */
				if (ptr != -1) {
/* has the new location previously been labeled */
					if (data[ptr] > 1) {
/* record the list of matching (remapped) labels and the minimum label */
						k = remap[data[ptr]];
						labels[nei] = k;
						if (labels[255] < k) {
							labels[255] = k;
						}
						nei += 1;
					}
				}
			}

/* add the new label if nei == 0 */
			if (nei == 0) {
/* is there room? if not, try to make some... */
				if (comp == 256) {
					relabel(data,vsize,remap,&comp);
					if (comp == 256) {
/* error: too many components!! */
						*ncomp = -(comp-2);
						return;
					}
				}
/* tag this pixel with new comp tag */
				data[i] = comp;
				remap[comp] = comp;
				comp += 1;
			} else {
/* tag the pixel with the smallest neighbor tag */
				data[i] = labels[255];
/* map all other tags to the smallest neighbor tag */
				for(j=0;j<nei;j++) {
					if (labels[j] != labels[255]) {
						remap[labels[j]] = labels[255];
					}
				}
			}
		}
/* next voxel (in a variable number of dimensions) */
		j = 0;
		while (j < dim) {
			r[j] += 1;
			if (r[j] < d[j]) break;
			r[j] = 0;
			j++;
		}
	}

/* remap the matching labels */
	relabel(data,vsize,remap,&comp);

/* done */
	*ncomp = comp - 2;  /* don't count 0 or 1 */

	return;
}

/* routine to map all previously labeled voxels to their final component
	labels.  It also cleans the mapping vector, removing any redundant
	labels, making room for more.  This routine can be called after any
	voxel has been processed to free up space in the mapping vector and
	must be called at the end of the last voxel to set the final voxel
	labels */

static void relabel(unsigned char *data,long int last,long int *remap,
	long int *comp)
{
	long int 	i,j,k;

/* adjust the remappings until a base remapping can be found */
	for(j=2;j<(*comp);j++) {
		k = j;
		while (remap[k] != k) {
			k = remap[k];
		}
		remap[j] = k;
	}

/* pack the remap array towards 2 so labels are contiguous */
	i = 2;
	for(j=2;j<(*comp);j++) {
		for(k=2;k<(*comp);k++) {
/* any (j)s ? */
			if (remap[k] == j) break;
		}
/* if so, pack them down to i */
		if (k != (*comp)) {
			for(k=2;k<(*comp);k++) if (remap[k] == j) remap[k] = i;
			i += 1;
		}
	}

/* apply the remappings to the image */
	for(j=0;j<last;j++) data[j] = remap[data[j]];

/* clean up the remap array (we will have a contiguous mapping) */
	for(j=2;j<i;j++) remap[j] = j;

/* remaining number of components */
	*comp = i;

/* done */
	return;
}
