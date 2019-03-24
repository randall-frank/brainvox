/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski, M.D.
 *
 * $Id: iso_trim.c 1250 2005-09-16 15:51:42Z dforeman $
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

#define MAX_KNIT 50
#ifndef MALLOC
#define MALLOC malloc
#endif
#ifndef FREE
#define FREE free
#endif
#ifndef REALLOC
#define REALLOC realloc
#endif

long int trim_polys(long int *n_tris,long int *tris,long int *n_verts,
	float *verts,long int n_left,long int verbose);

/* local structures */
typedef struct {
	long int vnum;
	double	value;
} Vval;

typedef struct {
	long int vnum;
	long int onum;
} VmapTri;

/* local prototypes */
int e_by_value(const void *y,const void *x);
int e_by_vnum(const void *x,const void *y);
int sort_longs(const void *xi,const void *yi);

int is_valid(long int n,long int *n_tris,long int *tris,long int *n_verts,
	float *verts,Vval *out);
int get_polys_from_vtex(long int vnum,long int *n_tris,long int *tris,
	long int *out,long int *n_out);
int knit_patch(long int n,long int *n_tris,long int *tris,long int *n_verts,
	float *verts,long int *deadtris);
void calc_normal(long int a,long int b,long int c,float *verts,double *r);
long int is_next(long int *tri,long int cur,long int n);
long int shorten_list(Vval *e_list,long int *lsize,long int *n_tris,
	long int *tris,long int *n_verts);

int sort_vmap(const void *xi,const void *yi);

/* local 'globals' */
static	long int 	verb;
static	VmapTri		*s1;
static	VmapTri		*s2;
static	VmapTri		*s3;


/* qsort routines */
int sort_vmap(const void *xi,const void *yi)
{
	VmapTri *x,*y;
	x = (VmapTri *)xi;
	y = (VmapTri *)yi;
/* return >0 if x>y and <0 if x<y */
	if (x->vnum > y->vnum) return(1);
	return(-1);
}

int sort_longs(const void *xi,const void *yi)
{
	long int *x,*y;
	x = (long int *)xi;
	y = (long int *)yi;
/* return >0 if x>y and <0 if x<y */
	if ((*x) > (*y)) return(1);
	return(-1);
}

int e_by_vnum(const void *xi,const void *yi)
{
	Vval *x,*y;
	x = (Vval *)xi;
	y = (Vval *)yi;
/* return >0 if x>y and <0 if x<y */
	if (x->vnum > y->vnum) return(1);
	return(-1);
}

int e_by_value(const void *xi,const void *yi)
{
	Vval *x,*y;
	x = (Vval *)xi;
	y = (Vval *)yi;
/* return >0 if x>y and <0 if x<y */
	if (x->value > y->value) return(1);
	return(-1);
}

/* find all the triangles containing a particular vertex */
/* uses binary search.  Assumes that the removal of a vertex does not
	affect any other vertex meshes still on the removal list */
int get_polys_from_vtex(long int vnum,long int *n_tris,long int *tris,
	long int *out,long int *n_out)
{
	long int n = 0;
	long int st,en,j,i;
	long int count;
	
/* binary search for any occurance of vnum */
/* Remember: actual list length is 0-(*n_tris)-1 */
	i = 0;
	count = 0;
	st = 0;
	en = (*n_tris)-1;
	j = (*n_tris)/2;
	while ((st != en) && (count < 50)) {
		i = st;
		if (s1[i].vnum == vnum) break;
		i = en;
		if (s1[i].vnum == vnum) break;
		i = st + j;
		if (vnum >= s1[i].vnum) {
			st = i;
		} else {
			en = i;
		}
		j = j/2;
		if (j == 0) {
			j = 1;
			count++;
		}
	}
	if (count < 45) {
/* backup to the first vnum */
		while ((i > 0) && (s1[i-1].vnum == vnum)) i--;
/* copy all vnums */
		while ((i < (*n_tris)) && (s1[i].vnum == vnum)) {
			out[n] = s1[i].onum;
			i++;
			n++;
			if (n >= (*n_out)) return(1);
		}
	}
/* binary search for any occurance of vnum */
	count = 0;
	st = 0;
	en = (*n_tris)-1;
	j = (*n_tris)/2;
	while ((st != en) && (count < 50)) {
		i = st;
		if (s2[i].vnum == vnum) break;
		i = en;
		if (s2[i].vnum == vnum) break;
		i = st + j;
		if (vnum >= s2[i].vnum) {
			st = i;
		} else {
			en = i;
		}
		j = j/2;
		if (j == 0) {
			j = 1;
			count++;
		}
	}
	if (count < 45) {
/* backup to the first vnum */
		while ((i > 0) && (s2[i-1].vnum == vnum)) i--;
/* copy all vnums */
		while ((i < (*n_tris)) && (s2[i].vnum == vnum)) {
			out[n] = s2[i].onum;
			i++;
			n++;
			if (n >= (*n_out)) return(1);
		}
	}
/* binary search for any occurance of vnum */
	count = 0;
	st = 0;
	en = (*n_tris) - 1;
	j = (*n_tris)/2;
	while ((st != en) && (count < 50)) {
		i = st;
		if (s3[i].vnum == vnum) break;
		i = en;
		if (s3[i].vnum == vnum) break;
		i = st + j;
		if (vnum >= s3[i].vnum) {
			st = i;
		} else {
			en = i;
		}
		j = j/2;
		if (j == 0) {
			j = 1;
			count++;
		}
	}
	if (count < 45) {
/* backup to the first vnum */
		while ((i > 0) && (s3[i-1].vnum == vnum)) i--;
/* copy all vnums */
		while ((i < (*n_tris)) && (s3[i].vnum == vnum)) {
			out[n] = s3[i].onum;
			i++;
			n++;
			if (n >= (*n_out)) return(1);
		}
	}
	*n_out = n;
	return(0);	
}

/* routine to compute the normal to a polygon */
void calc_normal(long int a,long int b,long int c,float *verts,double *r)
{
	double v1[3],v2[3],d;
	long int j,i;
	
/* get the normal (start with v1 (0-1) and v2 (2-1) */
	for(j=0;j<3;j++) v1[j] = verts[(a*3)+j] - verts[(b*3)+j];
	for(j=0;j<3;j++) v2[j] = verts[(c*3)+j] - verts[(b*3)+j];
	
	r[0] = ((v1[1]*v2[2]) - (v1[2]*v2[1]));
    	r[1] = ((v1[2]*v2[0]) - (v1[0]*v2[2]));
    	r[2] = ((v1[0]*v2[1]) - (v1[1]*v2[0]));
    
	d = (r[0]*r[0] + r[1]*r[1] + r[2]*r[2]);
	if (d > 0.0000001) {
    		d = sqrt(d);
    
    		for(i=0;i<3;i++) {
            		r[i] = r[i]/d;
    		}
	}
	return;
}
/* routine to get the next vnum which is not n and not current,
	it includes sanity checks for n and current */
long int is_next(long int *tri,long int cur,long int n)
{
	long int i,ok;
/* n must be there */
	ok = 0;
	for(i=0;i<3;i++) if (tri[i] == n) ok = 1;
	if (ok != 1) return(-1);
/* cur must be there */	
	ok = 0;
	for(i=0;i<3;i++) if (tri[i] == cur) ok = 1;
	if (ok != 1) return(-1);
/* get the other one */
	for(i=0;i<3;i++) {
		if ((tri[i] != cur) && (tri[i] != n)) {
			return(tri[i]);
		}
	}
	return(-1);	
}
/* check to see if a point is valid for removal, and evaluate its
	strength (return 0 if no go, 1 if it is valid) */
int is_valid(long int n,long int *n_tris,long int *tris,long int *n_verts,
	float *verts,Vval *out)
{
	long int 	trilist[MAX_KNIT];
	long int	nfound;
	long int	i,j,st,cur,k;
	double		norm[3],avg[3],d;
	
	out->vnum = n;
	out->value = 1000000.0;

/* return a list of triangle indicies which include the specified vertex */
	nfound = MAX_KNIT;
	if (get_polys_from_vtex(n,n_tris,tris,trilist,&nfound)) return(0);
#ifdef DEBUG
	printf("Is_valid %ld -> %ld tris\n",n,nfound);
#endif
	if (nfound < 3) return(0);

/* get the normals for those triangles, and average */
	for(j=0;j<3;j++) avg[j] = 0.0;
	for(i=0;i<nfound;i++) {
		calc_normal(tris[(trilist[i]*3)+0],tris[(trilist[i]*3)+1],
			tris[(trilist[i]*3)+2],verts,norm);
		for(j=0;j<3;j++) avg[j] += norm[j];
	}
/* compute d value, the closer d is to 0.0 the more likely vertex n can be
	removed */
	d = 0;
	for(i=0;i<nfound;i++) {
		calc_normal(tris[(trilist[i]*3)+0],tris[(trilist[i]*3)+1],
			tris[(trilist[i]*3)+2],verts,norm);
		for(j=0;j<3;j++) d += (fabs((avg[j]) - (norm[j])));
	}
	out->value = d;
/* is the polygon CLOSED?, that is the tris using it, ring it completely */
/* if so, we should be able to link our way around the polygons from one to
	the next and should use ALL the polygons */	
	st = tris[(trilist[0]*3)+0];
	if (st == n) st = tris[(trilist[0]*3)+1];
	cur = is_next(&(tris[(trilist[0]*3)]),st,n);
	trilist[0] = -1;
	i = nfound - 1;
	while ((cur != st) && (i > 0)) {
		j = 0;
		while (j < nfound)  {
			if (trilist[j] != -1) {
				k = is_next(&(tris[(trilist[j]*3)]),cur,n);
				if (k == -1) {
					j++;
				} else {
					cur = k;
					trilist[j] = -1;
					i--;
					j = 9999;
				}
			} else {
				j++;
			}
		}
/* bad loop */
		if (j != 9999) return(0);
	}
	if ((i != 0) || (st != cur)) return(0);
/* passes the CLOSED test and the d value is valid. */
	return(1);
}
/* routine to remove a vertex and reconstruct the missing patch */
int knit_patch(long int n,long int *n_tris,long int *tris,long int *n_verts,
	float *verts,long int *dead)
{
	long int 	trilist[MAX_KNIT],tmp[MAX_KNIT];
	long int	nfound;
	long int	i,j,st,cur,k;
	double		n1[3],n2[3];

/* return a list of triangle indicies which include the specified vertex */
	nfound = MAX_KNIT;
	if (get_polys_from_vtex(n,n_tris,tris,trilist,&nfound)) return(0);
/* retain the list for use after the closeure recheck */
	for(i=0;i<nfound;i++) tmp[i] = trilist[i];
/* recheck closure (in case a vertex was removed earlier) */
	st = tris[(trilist[0]*3)+0];
	if (st == n) st = tris[(trilist[0]*3)+1];
	cur = is_next(&(tris[(trilist[0]*3)]),st,n);
	trilist[0] = -1;
	i = nfound - 1;
	while ((cur != st) && (i > 0)) {
		j = 0;
		while (j < nfound)  {
			if (trilist[j] != -1) {
				k = is_next(&(tris[(trilist[j]*3)]),cur,n);
				if (k == -1) {
					j++;
				} else {
					cur = k;
					trilist[j] = -1;
					i--;
					j = 9999;
				}
			} else {
				j++;
			}
		}
/* bad loop */
		if (j != 9999) return(0);
	}
	if ((i != 0) || (st != cur)) return(0);

/* the polygon list around n is closed, thus we remove vertex N and
	two polygons which use N */
/* the first and last triangles (polygons) are not used in the repatching
	and are returned on the dead list for removal later */
	for(i=0;i<nfound;i++) trilist[i] = tmp[i];

/* use the direction of the first normal to guage the direction of
	the output normals */
	calc_normal(tris[(trilist[0]*3)+0],tris[(trilist[0]*3)+1],
		tris[(trilist[0]*3)+2],verts,n1);

/* 0 is a good place to start */
	st = tris[(trilist[0]*3)+0];
	if (st == n) st = tris[(trilist[0]*3)+1];
	cur = is_next(&(tris[(trilist[0]*3)]),st,n);
	if ((st == -1) || (cur == -1)) return(0);

/* kill the first pgon */
	dead[0] = trilist[0];
	trilist[0] = -1;
	i = nfound - 1;
	while (cur != st) {
		j = 0;
		while (j < nfound)  {
			if (trilist[j] != -1) {
				k = is_next(&(tris[(trilist[j]*3)]),cur,n);
				if (k == -1) {
					j++;
				} else {
/* and the last pgon */
/* pick direction based on dot product with original normal */
				calc_normal(st,cur,k,verts,n2);
				if (((n1[0]*n2[0]) + (n1[1]*n2[1]) +
						(n1[2]*n2[2])) < 0.0) {
					tris[(trilist[j]*3)+0] = k;
					tris[(trilist[j]*3)+1] = cur;
					tris[(trilist[j]*3)+2] = st;
				} else {
					tris[(trilist[j]*3)+0] = st;
					tris[(trilist[j]*3)+1] = cur;
					tris[(trilist[j]*3)+2] = k;
				}
					dead[1] = trilist[j];
					cur = k;
					trilist[j] = -1;
					j = 9999;
					i--;
				}
			} else {
				j++;
			}
		}
	}
	return(1);
}
/* this routine shortens an potential vertex list by removing
	any verticies which share an edge with another vertex on the
	list.  This prevents the patching routine from having to 
	patch a concave opening (in theory) */
long int shorten_list(Vval *e_list,long int *lsize,long int *n_tris,
	long int *tris,long int *n_verts)
{
	long int 	trilist[MAX_KNIT];
	long int	nfound;
	long int	i,j,k,n;
	long int	a,b,*vv;

	if (verb) fprintf(stderr,"Removing shared edges...\n");
/* remove the vertexes which share an edge with a lower ranking vertex */
/* first sort the list by increasing D value as we want to keep the
	lowest D numbers */
	qsort(e_list,(*lsize),sizeof(Vval),e_by_value);

/* ok the brute force mentality took over me here... */
/* this is a list which shows a vertex position (if any) in the
	e_list */
	vv = (long int *)MALLOC((*n_verts)*sizeof(long int));
	if (vv == 0L) return(0);
	for(i=0;i<(*n_verts);i++) vv[i] = -1;
	for(i=0;i<(*lsize);i++) vv[e_list[i].vnum] = i;

/* look for the connecting verticies */
	for(i=0;i<(*lsize);i++) {
		n = e_list[i].vnum;
/* skip removed ones */
		if (n == -1) continue;
/* return a list of triangle indicies which include the specified vertex */
		nfound = MAX_KNIT;
		if (get_polys_from_vtex(n,n_tris,tris,trilist,&nfound)) {
			nfound = 0;
		}
		for(j=0;j<nfound;j++) {
/* get the indicies (non-n) from each polygon */
			k = 0;
			a = tris[(trilist[j]*3)+k]; k += 1;
			if (a == n) {
				a = tris[(trilist[j]*3)+k];
				k += 1;
			}
			b = tris[(trilist[j]*3)+k]; k += 1;
			if (b == n) {
				b = tris[(trilist[j]*3)+k];
				k += 1;
			}
/* if a lower-valued vnum is in the list, remove it */
/* SLOW linear search way... 
			for(k=i+1;k<(*lsize);k++) {
				if (e_list[k].vnum == a) e_list[k].vnum = -1;
				if (e_list[k].vnum == b) e_list[k].vnum = -1;
			}
*/
/* faster, brute force method */
			if (vv[a] != -1) {
				e_list[vv[a]].vnum = -1;
				vv[a] = -1;
			}
			if (vv[b] != -1) {
				e_list[vv[b]].vnum = -1;
				vv[b] = -1;
			}
		}
	}
/* repack the array */
	j = 0;
	for(i=0;i<(*lsize);i++) {
		if (e_list[i].vnum != -1) {
			e_list[j].vnum = e_list[i].vnum;
			j++;
		}
	}
/* report new list size */
	(*lsize) = j;
	FREE(vv);
	return(0);
}

long int trim_polys(long int *n_tris,long int *tris,long int *n_verts,
	float *verts,long int n_left,long int verbose)
{
	long int	lmaxsize,lsize;
	Vval		*e_list = 0L;
	Vval		myval;
	long int 	i,n_wanted,j,k;
	long int	deadtris[2],dsize;
	long int	*dolist = 0L;
	long int	*vlist = 0L;
/* assume no errors */
	long int	err = 0L;

	verb = verbose;

while ((*n_verts) > (n_left+50)) {
/* init */	
	s1 = 0L;
	s2 = 0L;
	s3 = 0L;

/* build sorted tri/vtex tables */
/* ie s1[i].vnum is ever increasing */
/* ie s2[i].vnum is ever increasing */
/* ie s3[i].vnum is ever increasing */
	s1 = (VmapTri *)MALLOC(sizeof(VmapTri)*(*n_tris));
	s2 = (VmapTri *)MALLOC(sizeof(VmapTri)*(*n_tris));
	s3 = (VmapTri *)MALLOC(sizeof(VmapTri)*(*n_tris));
	if ((s1 == 0L) || (s2 == 0L) || (s3 == 0L)) {
		err = 1;
		goto errout;
	}
/* grab the vertex/tri mapping */
	for(i=0;i<(*n_tris);i++) {
		s1[i].vnum = tris[(i*3)+0];
		s1[i].onum = i;
		s2[i].vnum = tris[(i*3)+1];
		s2[i].onum = i;
		s3[i].vnum = tris[(i*3)+2];
		s3[i].onum = i;
	}
/* and sort for binary search access */
	qsort(s1,(*n_tris),sizeof(VmapTri),sort_vmap);
	qsort(s2,(*n_tris),sizeof(VmapTri),sort_vmap);
	qsort(s3,(*n_tris),sizeof(VmapTri),sort_vmap);
	
/* build the vertex removal list */	
	n_wanted = (*n_verts)-n_left;
	lmaxsize = n_wanted + (0.2 * (*n_verts));
	lsize = 0;

	e_list = (Vval *)MALLOC(sizeof(Vval)*lmaxsize);
	if (e_list == 0L) {
		err = 1;
		goto errout;
	}

	if (verb) fprintf(stderr,"Looking for canidate vertexes...\n");
/* look for and evaluate the vertexes for removal */
	for(i=0;i<(*n_verts);i++) {
		if ((i % 5000) == 0) if (verb) {
	fprintf(stderr,"Working on %ld of %ld list: %ld\n",i,(*n_verts),lsize);
		}
/* if it is valid, add it to the list */
		if (is_valid(i,n_tris,tris,n_verts,verts,&myval)) {
			e_list[lsize] = myval;
			lsize++;
/* if the list size exceeds the number wanted to be removed + 10%,
	sort the list and only keep the most likely candidates */
/* this reduces the number of sorts to 10 and still keeps the list
	fairly small */
			if (lsize >= lmaxsize) {
				shorten_list(e_list,&lsize,n_tris,tris,n_verts);
				if (lsize > n_wanted) lsize = n_wanted;
			}
		}
	}
/* keep the vertexes we will be removing */
	shorten_list(e_list,&lsize,n_tris,tris,n_verts);
	if (lsize == 0) {
		if (verb) fprintf(stderr,
		    "Unable to remove any more verticies. Process stopped.\n");
		err = 0;
		goto errout;
	}
/* limit to the number wanted and resort by the index value */
	if (lsize > n_wanted) lsize = n_wanted;
	qsort(e_list,lsize,sizeof(Vval),e_by_vnum);

	if (verb) fprintf(stderr,
		"Selected %ld vertexes. Removing and repatching...\n",lsize);
/* we will be freeing up 2 polys for each removed vertex */
	dolist = (long int *)MALLOC(sizeof(long int)*lsize*2);
	if (dolist == 0L) {
		err = 1;
		goto errout;
	}
	dsize = 0;
/* now we remove the vertexes from the list and reknit the network */
	for(i=0;i<lsize;i++) {
		if ((i % 5000) == 0) 
		if (verb) fprintf(stderr,"Working on %ld of %ld\n",i,lsize);
/* process the gap... */
		j = e_list[i].vnum;
		if (knit_patch(j,n_tris,tris,n_verts,verts,deadtris)) {
			dolist[dsize] = deadtris[0];
			dolist[dsize+1] = deadtris[1];
			dsize += 2;
/* destroy the dead polys */
			tris[(deadtris[0]*3)+0] = -1;
			tris[(deadtris[0]*3)+1] = -1;
			tris[(deadtris[0]*3)+2] = -1;
			tris[(deadtris[1]*3)+0] = -1;
			tris[(deadtris[1]*3)+1] = -1;
			tris[(deadtris[1]*3)+2] = -1;
		} else {
/* vertex could not be removed, do NOT allow it to be */
			e_list[i].vnum = -1;
		}
	}
/* sort the triangle removal list */
	qsort(dolist,dsize,sizeof(long int),sort_longs);
	if (verb)fprintf(stderr,"Cleaning up the vertex and triangle lists...\n");
/* purge the verticies and make up a new lookup table */
	vlist = (long int *)MALLOC(sizeof(long int)*(*n_verts));
	if (vlist == 0L) {
		err = 1;
		goto errout;
	}
	j = 0;
	while(e_list[j].vnum == -1) j++;
	k = 0;
	for(i=0;i<(*n_verts);i++) {
		if ((j < lsize) && (i == e_list[j].vnum)) {
			j++;
			while(e_list[j].vnum == -1) j++;
			vlist[i] = -1;
		} else {
			verts[(k*3)+0] = verts[(i*3)+0];
			verts[(k*3)+1] = verts[(i*3)+1];
			verts[(k*3)+2] = verts[(i*3)+2];
			vlist[i] = k;
			k++;
		}
	}
	*n_verts = k;
/* purge the unneeded polys */
	j = 0;
	k = 0;
	for(i=0;i<(*n_tris);i++) {
		if ((j < dsize) && (i == dolist[j])) {
			j++;
			while(dolist[j] == i) {
			if (verb) fprintf(stderr,
				"Warning:multiple %ld in deadlist.\n",i);
 				j++;
			}
		} else {
/* skrunch the triangle list and remap the vertex pointers */
			tris[(k*3)+0] = tris[(i*3)+0];
			tris[(k*3)+1] = tris[(i*3)+1];
			tris[(k*3)+2] = tris[(i*3)+2];
			tris[(k*3)+0] = vlist[tris[(k*3)+0]];
			tris[(k*3)+1] = vlist[tris[(k*3)+1]];
			tris[(k*3)+2] = vlist[tris[(k*3)+2]];
			k++;
		}
	}
	*n_tris = k;
/* free up RAM */
	if (e_list != 0L) FREE(e_list);
	if (dolist != 0L) FREE(dolist);
	if (vlist != 0L) FREE(vlist);
	e_list = 0L;
	dolist = 0L;
	vlist = 0L;
/* free up the sorting lists */
	if (s1 != 0L) FREE(s1);
	if (s2 != 0L) FREE(s2);
	if (s3 != 0L) FREE(s3);
	s1 = 0L;
	s2 = 0L;
	s3 = 0L;
/* next pass if not enough verts were removed */
}
/* thats all */
	err = 0;
	return(err);
/* error handling, free up memory and exit */
errout:
	if (e_list != 0L) FREE(e_list);
	if (dolist != 0L) FREE(dolist);
	if (vlist != 0L) FREE(vlist);
	e_list = 0L;
	dolist = 0L;
	vlist = 0L;
	if (s1 != 0L) FREE(s1);
	if (s2 != 0L) FREE(s2);
	if (s3 != 0L) FREE(s3);
	s1 = 0L;
	s2 = 0L;
	s3 = 0L;
	return(err);
}
