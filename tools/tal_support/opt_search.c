/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski, M.D.
 *
 * $Id: opt_search.c 1250 2005-09-16 15:51:42Z dforeman $
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

#include "opt_search.h"

/*
        Routines for N-dimensional graph search

        Revision History:

                1 Oct 1995 Randy Frank (rjf)
                        Complete version checkin.
                5 Oct 1995 rjf
                        Support for additional return values (warnings)
		8 Oct 1995 rjf
			Add the Rotate function for wrap-around

        TTD:
                No bias for surface selection in "tie" cases

		Rotate formulation only allows (1) wrap-around dimension

        Tests:

*/

#define MIN(a,b) (((a)<(b)?(a):(b)))
#define MAX(a,b) (((a)>(b)?(a):(b)))

#define MALLOC(a) malloc(a)
#define FREE(a) free(a)

/* local prototypes */
static void	Init_Vect(long int *v,long int *lims,long int n,long int rev);
static int	Incr_Vect(long int *v,long int *lims,long int n);
static int  Decr_Vect(long int *v,long int *lims,long int n);
static long int Vect_to_Index(long int *v,long int *dd,long int n);
static int  Condition(float *arr,long int inc,long int n,long int opts);
static int Rotate(void *arr,long int *d,long int ndims,
	long int axis,long int dist,long int size);

/* search the 3D volume COST for minimum cost surface, returned as a
        "depth" map in D.  Connectivity is determined by range[] (2m+1) */

long int opt_search(long int n,long int *d,float *cost,long int *depth,
	long int *range,long int *wrap,long int opts)
{
        float           *acc;
        long int        i,j,k,l,r,ll;
        float           t,val;
        long int        top,bot;
        long int        dd[MAX_DIM+1];
	long int	v[MAX_DIM];
        signed char     *holes;
	long int	ret,con_err;
	long int	n_wrap,a_wrap,max_wrap,s_wrap;

	ret = OPT_NOERROR;

/* check the dimensionality */
	if (n > MAX_DIM) return(OPT_FATAL);
/* get the offsets */
	dd[0] = 1;
	for(i=1;i<=n;i++) dd[i] = d[i-1]*dd[i-1];
/* allocate accumulator memory */
        acc = (float *)MALLOC(dd[n]*sizeof(float));
        if (acc == 0L) return(OPT_FATAL);
/* and the holes array */
        holes = (signed char *)MALLOC(dd[n-1]);
        if (holes == 0L) {
                FREE(acc);
                return(OPT_FATAL);
        }
	for(i=0;i<dd[n-1];i++) holes[i] = depth[i];

if (opts & OPT_VVERB) {
	/* print input matrix */
        fprintf(stderr,"Input matrix:\n");
        l = 0;
	Init_Vect(v,d,n,0);
	do {
		if (v[0] == 0) fprintf(stderr,"\n");
		if ((v[0] == 0) && (v[1] == 0)) fprintf(stderr,"\n");
		fprintf(stderr,"%.3f ",cost[l++]);
	} while (Incr_Vect(v,d,n));
	fprintf(stderr,"\n");
}

/* wrapping stuff */
	n_wrap = -1; max_wrap = 0; a_wrap = 0;
	for(i=0;i<n;i++) {
		if (wrap[i]) {
			max_wrap = d[i]; /* how many */
			a_wrap = i;	 /* what axis */
		}
	}
	s_wrap = max_wrap/20;
	n_wrap = -s_wrap;

do {
	n_wrap += s_wrap;
	if (n_wrap > 0) {
		if (Rotate(cost,d,n,a_wrap,s_wrap,sizeof(float))) {
			n_wrap=max_wrap;
		}
		if (Rotate(holes,d,n-1,a_wrap,s_wrap,sizeof(signed char))) {
			n_wrap=max_wrap;
		}
	}

/* build accum matrix */
	Init_Vect(v,d,n-1,0);
	do {
/* over the Nth (n-1) dimension */
		l = Vect_to_Index(v,dd,n-1);
/* do the column */
		for(k=0;k<d[n-1];k++) {
			val = cost[l];
/* is it a hole? */
			if (holes[Vect_to_Index(v,dd,n-1)] == -1) {
				val = 0;
			} else {
/* look back in each dimension */	
				for(j=0;j<n-1;j++) {
/* over the value of 1 */
				if (v[j] > 0) {
					ll = l - dd[j];
/* search for min, starting at central */
					t = acc[ll];
/* up and down by range (if in legal bounds) */
					for(r=1;r<=range[j];r++) {
					    if (k-r >= 0) {
						t=MIN(t,acc[ll-(r*dd[n-1])]);
					    }
					    if (k+r <= d[n-1]-1) {
						t=MIN(t,acc[ll+(r*dd[n-1])]);
					    }
					}
					val += t;
				} /* if location[j] > 0 */
/* wrap around code (if enabled and on the edge) */
/* wrap=( +1(this dim) -1(next dim) )*/	
				if ((wrap[j]) && (v[j] == (d[j]-1))) {
					ll = l + dd[j] - dd[j+1];
/* search for min, starting at central */
					t = acc[ll];
/* up and down by range (if in legal bounds) */
					for(r=1;r<=range[j];r++) {
					    if (k-r >= 0) {
						t=MIN(t,acc[ll-(r*dd[n-1])]);
					    }
					    if (k+r <= d[n-1]-1) {
						t=MIN(t,acc[ll+(r*dd[n-1])]);
					    }
					}
					val += t;
				} /* if wrapped */			
				} /* next dimension */
			}
/* store the value */
			acc[l] = val;
/* next value */
			l += dd[n-1];
		}
/* condition the column */		
		l = Vect_to_Index(v,dd,n-1);
		ret |= Condition(&(acc[l]),dd[n-1],d[n-1],opts);
	} while (Incr_Vect(v,d,n-1));

if (opts & OPT_VVERB) {
/* print accum matrix */
	fprintf(stderr,"Accum matrix:\n");
        l = 0;
	Init_Vect(v,d,n,0);
	do {
		if (v[0] == 0) fprintf(stderr,"\n");
		if ((v[0] == 0) && (v[1] == 0)) fprintf(stderr,"\n");
		fprintf(stderr,"%.3f ",acc[l++]);
	} while (Incr_Vect(v,d,n));
	fprintf(stderr,"\n");
}

/* search the dx,dy matrix in reverse to find the path */
	con_err = 0;
	Init_Vect(v,d,n-1,1);
	do {
/* over the Nth (n-1) dimension */
		l = Vect_to_Index(v,dd,n-1);
/* find the search limits */
		top = 0;
		bot = d[n-1]-1;
/* restrict by all other dimensions */
		for(j=0;j<n-1;j++) {
/* if not outside */
			if (v[j] < d[j]-1) {
/* or into a hole */
				if (holes[l] == holes[l+dd[j]]) {
					top=MAX(top,depth[l+dd[j]]-range[j]);
					bot=MIN(bot,depth[l+dd[j]]+range[j]);
				}
			}
/* if wrap enabled and we are at the origin */			
			if ((wrap[j]) && (v[j] == 0)) {
/* restrict by the wrapped column, wrap=( -1(this dim) +1(next dim) ) */
				if (holes[l] == holes[l-dd[j]+dd[j+1]]) {
					top=MAX(top,depth[l-dd[j]+dd[j+1]]
						-range[j]);
					bot=MIN(bot,depth[l-dd[j]+dd[j+1]]
						+range[j]);
				}
			
			}
		}
/* search for min ACC in restricted rgn (assume center first) */
		k = (top + bot)/2;
		t = acc[l + k*(dd[n-1])];
		depth[l] = k;
		for(k=top;k<=bot;k++) {
			if (t > acc[l + k*(dd[n-1])]) {
				depth[l] = k;
				t = acc[l + k*(dd[n-1])];
			}
		}
		if (top > bot) {
			ret |= OPT_WARN_SHIFT;
			con_err = 1;
		}
/* one more gridpoint chosen */
	} while(Decr_Vect(v,d,n-1));

} while((con_err == 1) && (n_wrap < max_wrap));

/* unwrap things... */
	if (n_wrap > 0) {
		Rotate(cost,d,n,a_wrap,-n_wrap,sizeof(float));
		Rotate(depth,d,n-1,a_wrap,-n_wrap,sizeof(long int));
		Rotate(holes,d,n-1,a_wrap,-n_wrap,sizeof(signed char));
	}

if (opts & OPT_VERB) {
/* print depth matrix */
        fprintf(stderr,"Depth matrix:\n");
        l = 0;
	Init_Vect(v,d,n-1,0);
	do {
		if (v[0] == 0) fprintf(stderr,"\n");
		if ((v[0] == 0) && (v[1] == 0)) fprintf(stderr,"\n");
		fprintf(stderr,"%3ld ",depth[l++]);
	} while (Incr_Vect(v,d,n-1));
	fprintf(stderr,"\n");
}

/* reset holes */
        for(i=0;i<dd[n-1];i++) if (holes[i] == -1) depth[i] = -1;

/* done */
        FREE(acc);
        FREE(holes);

        return(ret);
}

/* utility functions */
static void Init_Vect(long int *v,long int *lims,long int n,long int rev)
{
	long int i;

/* zero out the incremental vector */
	if (rev) {
		for(i=0;i<n;i++) v[i] = lims[i]-1;
	} else {
		for(i=0;i<n;i++) v[i] = 0;
	}

	return;
}

static int  Incr_Vect(long int *v,long int *lims,long int n)
{
	long int i;

	i = 0;
	while (i < n) {
/* increment the current dimension */
		v[i] += 1;
/* if the value is valid, return with a new vector */
		if (v[i] < lims[i]) return(1);
/* otherwise, zero the current dimension */
		v[i] = 0;
/* and move on to the next */
		i++;
	}
/* if we get here, all dimensions have been passed through */
	return(0);  
}

static int  Decr_Vect(long int *v,long int *lims,long int n)
{
	long int i;

	i = 0;
	while (i < n) {
/* decrement the current dimension */
		v[i] -= 1;
/* if the value is valid, return with a new vector */
		if (v[i] >= 0) return(1);
/* otherwise, zero the current dimension */
		v[i] = lims[i]-1;
/* and move on to the next */
		i++;
	}
/* if we get here, all dimensions have been passed through */
	return(0);  
}

static long int Vect_to_Index(long int *v,long int *dd,long int n)
{
	long int i,s;

	s = v[0];
	for(i=1;i<n;i++) s += (v[i]*dd[i]);

	return(s);
}

#define MAX_ACCUM 16777000

/* on SGI adding 1.0 to a float of 16777216.0 or greater results in roundoff */
/* we assume here that the min cost function difference will be 1.0 */

static int  Condition(float *arr,long int inc,long int n,long int opts)
{
	long int	i = 0;
	long int	j;
	float		min = arr[0];
	float		max = arr[0];
	double		d;
	int		ret = OPT_NOERROR;
	
	if ((opts & OPT_NORM) == 0) return(ret);  /* nothing to do */
/* find the min and max in the column */
	j = 0;
	for(i=0;i<n;i++) {
		if (arr[j] < min) min = arr[j];
		if (arr[j] > max) max = arr[j];
		j += inc;
	}
	d = min;
/* subtract the mean */
	j = 0;
	for(i=0;i<n;i++) {
		arr[j] -= d;
/* trunc if desired */		
		if (opts & OPT_TRUNC) {
			if (arr[j] > MAX_ACCUM) {
				arr[j] = MAX_ACCUM;
				ret = OPT_WARN_TRUNC;
			}
		}
		j += inc;
	}
	return(ret);
}

/* code for rotating an n-dimensional  volume along one axis */
static int Rotate(void *in,long int *d,long int ndims,
	long int axis,long int dist,long int size)
{
	long int	v[MAX_DIM];
	long int	z_d[MAX_DIM],dd[MAX_DIM+1];
	long int	i,ptr,inc,s,t,sz;
	unsigned char	*temp;
	unsigned char	*arr;

	arr = (unsigned char *)in;
	if (axis >= ndims) return(1); /* check for stupid errors */
#ifdef DEBUG
	printf("WRAPPING!!!,%ld %ld %ld %ld\n",d[axis],ndims,size,dist);
#endif

/* extra memory */
	temp = (unsigned char *)MALLOC(d[axis]*size);
	if (temp == 0L) return(1);
/* fake volume step */
	for(i=0;i<ndims;i++) {
		if (i == axis) {
			z_d[i] = 1;
		} else {
			z_d[i] = d[i];
		}
	}
/* get the offsets */
	dd[0] = 1;
	for(i=1;i<=ndims;i++) dd[i] = d[i-1]*dd[i-1];
/* get increment along a scan line */
	inc = 1;
	for(i=1;i<axis;i++) inc *= d[i];
/* for all scan lines (of the AXIS axis) */
	Init_Vect(v,z_d,ndims,0);
	do {
/* get the start of the scanline (assume v[axis] = 0) */
		ptr = Vect_to_Index(v,dd,ndims); /* note use of d NOT z_d */
#ifdef DEBUG
		printf("ptr=%ld %ld %ld\n",ptr,v[0],v[1]);
#endif
/* copy the data to temp */	
		t = 0;
		for(i=0;i<d[axis];i++) {
			for(sz=0;sz<size;sz++) {
				temp[(i*size)+sz] = arr[(ptr+t)*size+sz];
			}
			t += inc;
		}
/* rotate the data and paint back into the volume */
		s = dist;
		while(s < 0) s += d[axis];
		while(s >= d[axis]) s -= d[axis];
		t = 0;
		for(i=0;i<d[axis];i++) {
			for(sz=0;sz<size;sz++) {
				arr[(ptr+t)*size+sz] = temp[(s*size)+sz];
			}
			s += 1;
			if (s >= d[axis]) s = 0;
			t += inc;
		}
	} while (Incr_Vect(v,z_d,ndims));

	FREE(temp);
	return(0);
}
