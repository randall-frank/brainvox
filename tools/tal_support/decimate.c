/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski, M.D.
 *
 * $Id: decimate.c 1250 2005-09-16 15:51:42Z dforeman $
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
#include <math.h>
#include <string.h>

#define OPTS_PRESERVE_EDGES 	1

#define ALLOC_SIZE	10
#define MAX_AUX_VECTOR 100

#define MALLOC malloc
#define REALLOC realloc
#define FREE free

typedef struct {
	long int	vold;
	long int	vnew;
	float		cost;
} collapse;

typedef struct {
	long int	v[3];
	float		n[3];
} triangle;

typedef struct {
	long int	alloc;
	long int	ntris;
	float 		*verts;
	long int	iAuxSize;
	long int	iAuxType;
	char		*pAux;
	float		*pfWeights;
	triangle 	*list;
} tlist;

typedef struct {
	long int	count;
	long int	alloc;
	long int	*val;
} element;

typedef struct {
	long int	num;
	element 	*list;
} list;

/* exported functions */
long int decimate_mesh(long int *n_tris,long int *tris,long int *n_verts,
	float *verts,long int ask_tris,long int block_size,long int flags,
	long int n_vaux, float *aux,float *auxweights);

/* local prototypes */
static long int list_conv(list *l,long int idx,long int vorig,long int vnew);
static long int list_del(list *l,long int idx,long int value);
static long int list_add(list *l,long int idx,long int value);
static list *list_alloc(long int num);
static void list_free(list *p);

static void tetra_check(long int v, long int *vert_status,list *v2vlist,
	list *v2tlist);
static long int remove_edge(long int rem_vert,long int new_vert,
	tlist *tlist,list *v2tlist,list *v2vlist);
static long int calc_edge_cost(long int v,float *cost,
	tlist *tlist,list *v2tlist,list *v2vlist,long int flags);
static long int collapse_cost(long int v0,long int v1,tlist *tlist,
	list *v2tlist,float *cost,long int flags);
static long int next_v(triangle *t,long int base, long int chain);
static long int is_edge_v(long int v,tlist *tlist,list *v2tlist);

static tlist *tlist_alloc(long int num,float *verts,
	long int iAuxType,long int iAuxSize,char *pAux,float *pfWeights);
static void tlist_free(tlist *p);
static void tlist_recalc(tlist *l,long int n);
static long int tlist_add(tlist *l,long int v0,long int v1,long int v2);
static long int tlist_conv(tlist *l,long int idx,long int vorig,long int vnew);
static long int tlist_del(tlist *l,long int n);

static int comp_collapse(const void *p0,const void *p1);

/* potential collapse sorting routine */
static int comp_collapse(const void *p0,const void *p1)
{
	collapse	*c0 = (collapse *)p0;
	collapse	*c1 = (collapse *)p1;

	if (c0->cost < c1->cost) return(-1);
	if (c0->cost > c1->cost) return( 1);
	return(0);
}

/* triangle list manipulation functions */
static tlist *tlist_alloc(long int num, float *verts,
	long int iAuxType,long int iAuxSize,char *pAux,float *pfWeights)
{
	tlist		*l;
	triangle	*p;

	p = (triangle *)MALLOC(num*sizeof(triangle));
	if (p) {
		memset(p,0,num*sizeof(triangle));
	} else {
		return(NULL);
	}
	l = (tlist *)MALLOC(sizeof(tlist));
	if (!l) {
		FREE(p);
	} else {
		l->list = p;
		l->alloc = num;
		l->ntris = 0;
		l->verts = verts;
		l->pAux = pAux;
		l->iAuxType = iAuxType;
		l->iAuxSize = iAuxSize;
		l->pfWeights = pfWeights;
	}

	return(l);
}
static void tlist_free(tlist *p)
{
	FREE(p->list);
	FREE(p);
	return;
}

static void tlist_recalc(tlist *l,long int n)
{
	float		t[3],v0[3],v1[3];
	float		len;
	long int	j0,j1,j2;

	if (l->list[n].v[0] < 0) return;

	j0 = l->list[n].v[0]*3;
	j1 = l->list[n].v[1]*3;
	j2 = l->list[n].v[2]*3;

	v0[0] = l->verts[j1+0] - l->verts[j0+0];
	v0[1] = l->verts[j1+1] - l->verts[j0+1];
	v0[2] = l->verts[j1+2] - l->verts[j0+2];

	v1[0] = l->verts[j2+0] - l->verts[j0+0];
	v1[1] = l->verts[j2+1] - l->verts[j0+1];
	v1[2] = l->verts[j2+2] - l->verts[j0+2];

	t[0] = v0[1]*v1[2] - v0[2]*v1[1];
	t[1] = v0[2]*v1[0] - v0[0]*v1[2];
	t[2] = v0[0]*v1[1] - v0[1]*v1[0];

	len = t[0]*t[0] + t[1]*t[1] + t[2]*t[2];
	if (len > 0) {
		len = (float)sqrt(len);
	} else {
		len = 1.0;
	}

	l->list[n].n[0] = t[0]/len;
	l->list[n].n[1] = t[1]/len;
	l->list[n].n[2] = t[2]/len;

	return;
}

static long int tlist_add(tlist *l,long int v0,long int v1,long int v2)
{
	long int	i;

	if (l->ntris >= l->alloc) return(0);
	
	i = l->ntris;

	l->list[i].v[0] = v0;
	l->list[i].v[1] = v1;
	l->list[i].v[2] = v2;

	tlist_recalc(l,i);

	l->ntris += 1;

	return(1);
}

static long int tlist_del(tlist *l,long int n)
{
	if (l->list[n].v[0] < 0) return(0);

	l->list[n].v[0] = -1;

	l->ntris -= 1;

	return(1);
}

static long int tlist_conv(tlist *l,long int idx,long int vorig,long int vnew)
{
	long int	c = 0;

	if (l->list[idx].v[0] < 0) return(0);

	if (l->list[idx].v[0] == vorig) l->list[idx].v[0] = vnew;
	if (l->list[idx].v[1] == vorig) l->list[idx].v[1] = vnew;
	if (l->list[idx].v[2] == vorig) l->list[idx].v[2] = vnew;

	if (l->list[idx].v[0] == vnew) c++;
	if (l->list[idx].v[1] == vnew) c++;
	if (l->list[idx].v[2] == vnew) c++;

	if (c > 1) {
		tlist_del(l,idx);
		return(2);
	}

	if (c > 0) tlist_recalc(l,idx);

	return(1);
}

/* integer list manipulation functions */
static long int list_conv(list *l,long int idx,long int vorig,long int vnew)
{
	long int        i;
	long int	porig = -1;
	long int	pnew = -1;

	for(i=0;i<l->list[idx].count;i++) {
		if (l->list[idx].val[i] == vorig) porig = i;
		if (l->list[idx].val[i] == vnew) pnew = i;
	}

	/* not there at all? */
	if (porig == -1) return(0);

	/* does the new value already exist? */
	if (pnew == -1) {
		/* no? just convert the value */
		l->list[idx].val[porig] = vnew;
	} else {
		/* yes? just remove the orig value */
		for(i=porig+1;i<l->list[idx].count;i++) {
			l->list[idx].val[i-1] = l->list[idx].val[i];
		}
		l->list[idx].count -= 1;
	}

	return(1);
}

static long int list_del(list *l,long int idx,long int value)
{
	long int	i,j;

	/* already exist? */
	for(i=0;i<l->list[idx].count;i++) {
		if (l->list[idx].val[i] == value) {
			for(j=i+1;j<l->list[idx].count;j++) {
				l->list[idx].val[j-1] = l->list[idx].val[j];
			}
			l->list[idx].count -= 1;
			return(1);
		}
	}

	return(0);
}

static long int list_add(list *l,long int idx,long int value)
{
	long int	i;

	/* already exist? */
	for(i=0;i<l->list[idx].count;i++) {
		if (l->list[idx].val[i] == value) return(1);
	}

	if (l->list[idx].count == l->list[idx].alloc) {
		l->list[idx].alloc += ALLOC_SIZE;
		l->list[idx].val = REALLOC(l->list[idx].val,
			l->list[idx].alloc*sizeof(long int));
		if (!l->list[idx].val) {
		        l->list[idx].alloc = 0;
			l->list[idx].count = 0;
			return(0);
		}
	}

	l->list[idx].val[l->list[idx].count] = value;
	l->list[idx].count += 1;

	return(1);
}

static list *list_alloc(long int num)
{
	list	*l;
	element	*p;

	p = (element *)MALLOC(num*sizeof(element));
	if (p) {
		memset(p,0,num*sizeof(element));
	} else {
		return(NULL);
	}
	l = (list *)MALLOC(sizeof(list));
	if (!l) {
		FREE(p);
	} else {
		l->list = p;
		l->num = num;
	}

	return(l);
}

static void list_free(list *p)
{
	long int	i;

	for(i=0;i<p->num;i++) {
		if (p->list[i].val) FREE(p->list[i].val);
	}
	FREE(p->list);
	FREE(p);

	return;
}


/* "walk" a triangle: find next vert which is not the base or the chain */
static long int next_v(triangle *t,long int base, long int chain)
{
	long int	v[3] = {0,0,0};
	long int	i,c = 0;

	for(i=0;i<3;i++) {
		if (t->v[i] == base) { v[i] = 1; c++; };
		if (t->v[i] == chain) { v[i] = 1; c++; };
	}
	if (c != 2) return(-1);
	if (v[0] == 0) return(t->v[0]);
	if (v[1] == 0) return(t->v[1]);
	if (v[2] == 0) return(t->v[2]);
	return(-1);
}

/* return true if the triangles do not encircle the vert in question */
static long int is_edge_v(long int v,tlist *tlist,list *v2tlist)
{
	long int	firstv = -1;
	long int	*tris = v2tlist->list[v].val;
	long int	ntris = v2tlist->list[v].count;
	triangle	*t;
	long int	next,i,cur_v,prev;
	
	if (ntris == 0) return(1);

	t = tlist->list+tris[0];
	if (t->v[0] != v) firstv = t->v[0];
	if (t->v[1] != v) firstv = t->v[1];

	if (firstv == -1) return(1);

	/* we are done when walking from curv we get to first_v */
	cur_v = next_v(t,v,firstv);
	if (cur_v < 0) return(1);

	/* walk the triangles */
	prev = firstv;
	while(cur_v != firstv) {
		next = -1;
		for(i=0;i<ntris;i++) {
			t = tlist->list+tris[i];
			next = next_v(t,v,cur_v);
			if ((next > 0) && (next != prev)) {
				prev = cur_v;
				cur_v = next;
				break;
			} else {
				next = -1;
			}
		}
		if (next < 0) return(1);
	}

	return(0);
}


static long int collapse_cost(long int v0,long int v1,tlist *tlist,
	list *v2tlist, float *cost,long int flags)
{
	long int	c=0;
	long int	c_tris[2];
	long int	i,j;
	float		curv = 0.0;
	float		len = 0.0;
	float		wei;

	/* find the collapsing tris */
	for(i=0;i<v2tlist->list[v0].count;i++) {
		long int	t = v2tlist->list[v0].val[i];

		if (tlist->list[t].v[0] == v1) c_tris[c++] = t;
		if (c == 2) break;
		if (tlist->list[t].v[1] == v1) c_tris[c++] = t;
		if (c == 2) break;
		if (tlist->list[t].v[2] == v1) c_tris[c++] = t;
		if (c == 2) break;
	}

	/* compute curv */
	for(i=0;i<v2tlist->list[v0].count;i++) {
		long int        t = v2tlist->list[v0].val[i];
		float		mcurv = 1.0f;
		
		for(j=0;j<c;j++) {
			float	dot;

			dot  = tlist->list[t].n[0]*tlist->list[c_tris[j]].n[0];
			dot += tlist->list[t].n[1]*tlist->list[c_tris[j]].n[1];
			dot += tlist->list[t].n[2]*tlist->list[c_tris[j]].n[2];
			dot = (1.0f-dot)*0.5f;

			if (dot < mcurv) mcurv = dot;
		}
		if (mcurv > curv) curv = mcurv;
	}

	/* compute edge length */
	len  = (tlist->verts[v0*3+0] - tlist->verts[v1*3+0]) *
		(tlist->verts[v0*3+0] - tlist->verts[v1*3+0]);
	len += (tlist->verts[v0*3+1] - tlist->verts[v1*3+1]) *
		(tlist->verts[v0*3+1] - tlist->verts[v1*3+1]);
	len += (tlist->verts[v0*3+2] - tlist->verts[v1*3+2]) *
		(tlist->verts[v0*3+2] - tlist->verts[v1*3+2]);

	if (len > 0.f) len = (float)sqrt(len);

	*cost = len*curv;
	
	if (tlist->pAux) {
		if (tlist->iAuxType == 1) {
			char	*p0 = ((char *)tlist->pAux) + (tlist->iAuxSize*v0);
			char	*p1 = ((char *)tlist->pAux) + (tlist->iAuxSize*v1);
			for(i=0;i<tlist->iAuxSize;i++) {
				wei = (float)((*p0++) - (*p1++));
				wei = fabs(wei);
				if (tlist->pfWeights) wei = tlist->pfWeights[i]*wei;
				*cost += wei;
			}
		} else {
			float	*p0 = ((float*)tlist->pAux) + (tlist->iAuxSize*v0);
			float	*p1 = ((float*)tlist->pAux) + (tlist->iAuxSize*v1);
			for(i=0;i<tlist->iAuxSize;i++) {
				wei = (*p0++) - (*p1++);
				wei = fabs(wei);
				if (tlist->pfWeights) wei = tlist->pfWeights[i]*wei;
				*cost += wei;
			}
		}
	}

	return(1);
}

static void tetra_check(long int v, long int *vert_status,list *v2vlist,
	list *v2tlist)
{
	long int	i,j,count;

	if (vert_status[v] != 0) return;

	/* vertex and triangle counts for v must be 3 */
	if (v2vlist->list[v].count != 3) return;
	if (v2tlist->list[v].count != 3) return;
	
	/* and connected verts */
	for(i=0;i<3;i++) {
		j = v2vlist->list[v].val[i];
		if (v2vlist->list[j].count != 3) return;
		if (v2tlist->list[j].count != 3) return;
	}

	/* from any one of the connected verts, the verts it is
	   connected to must be the other two in the original vert */
	count = 0;
	j = v2vlist->list[v].val[0];
	for(i=0;i<3;i++) {
		if (v2vlist->list[j].val[i] == v) count++;
		if (v2vlist->list[j].val[i] == 
			v2vlist->list[v].val[1]) count++;
		if (v2vlist->list[j].val[i] == 
			v2vlist->list[v].val[2]) count++;
	}
	if (count != 3) return;

	/* tag these verts as fixed */
	vert_status[v] = 1;
	for(i=0;i<3;i++) {
		j = v2vlist->list[v].val[i];
		if (vert_status[j] == 0) vert_status[j] = 1;
	}

	return;
}

static long int calc_edge_cost(long int v,float *cost,
	tlist *tlist,list *v2tlist,list *v2vlist,long int flags)
{
	long int	min_dest = -1;
	float		d,dmin = 0;
	long int	i,status;

	/* compute for folding this vertex into each connected one */
	for(i=0;i<v2vlist->list[v].count;i++) {
		long int	vdest = v2vlist->list[v].val[i];

		status = collapse_cost(v,vdest,tlist,v2tlist,&d,flags);
		if (status) {
			if ((min_dest == -1) || (d < dmin)) {
				dmin = d;
				min_dest = vdest;
			}
		}
	}
	
	/* return the minimum cost */
	if (min_dest != -1) {
		*cost = dmin;
		return(min_dest);
	}

	return(-1);
}

static long int remove_edge(long int rem_vert,long int new_vert,
	tlist *tlist,list *v2tlist,list *v2vlist)
{
	long int	bad_tri[2];
	long int	c = 0;
	long int	i,j;

/* get the two tris to remove and update tlist (rem_vert->new_vert) */
/* only need to check tris from rem_vert */
	for(i=0;i<v2tlist->list[rem_vert].count;i++) {
		long int	t = v2tlist->list[rem_vert].val[i];
		
		/* convert verts */
		j = tlist_conv(tlist,t,rem_vert,new_vert);

		/* is this a collapsed tri? */
		if (j == 2) {
			if (c == 2) return(0);
			bad_tri[c++] = t;
		}
	}

/* new_vert: add the tris and verts from rem_vert */
/* new_vert: remove rem_vert references from v2v, remove tris from v2t */
	for(i=0;i<v2tlist->list[rem_vert].count;i++) {
		list_add(v2tlist,new_vert,v2tlist->list[rem_vert].val[i]);
	}
	for(i=0;i<v2vlist->list[rem_vert].count;i++) {
		list_add(v2vlist,new_vert,v2vlist->list[rem_vert].val[i]);
	}
	for(j=0;j<c;j++) list_del(v2tlist,new_vert,bad_tri[j]);
	list_del(v2vlist,new_vert,rem_vert);
	list_del(v2vlist,new_vert,new_vert);

/* for each vert referenced by new_vert: */
/* 	remove tris from v2t and convert rem_vert to new_vert in v2v */
	for(i=0;i<v2vlist->list[new_vert].count;i++) {
		long int        v = v2vlist->list[new_vert].val[i];

		for(j=0;j<c;j++) list_del(v2tlist,v,bad_tri[j]);
		list_conv(v2vlist,v,rem_vert,new_vert);
	}

	return(1);
}

long int decimate_mesh(long int *n_tris,long int *tris,long int *n_verts,
        float *verts,long int ask_tris,long int block_size,long int flags,
        long int n_vaux, float *aux,float *auxweights)
{
	long int	i,k,j;
	long int	count;
	tlist		*tlist = NULL;
	list		*v2tlist = NULL;
	list		*v2vlist = NULL;
	long int	*vert_status = NULL;
	collapse	*pending = NULL;
	long int	big_block_size;

/* get memory for the access tables */
	tlist = tlist_alloc(*n_tris,verts,sizeof(float),
		n_vaux,(void *)aux,auxweights);
	if (!tlist) goto err_out;
	v2tlist = list_alloc(*n_verts);
	if (!v2tlist) goto err_out;
	v2vlist = list_alloc(*n_verts);
	if (!v2vlist) goto err_out;
	vert_status = (long int *)MALLOC((*n_verts)*sizeof(long int));
	if (!vert_status) goto err_out;
	if (block_size < 1) block_size = 1;
	big_block_size = block_size*1.2;
	pending = (collapse *)MALLOC(big_block_size*sizeof(collapse));
	if (!pending) goto err_out;

/* create the triangle and v2v lists */
	j = 0;
	for(i=0;i<*n_tris;i++) {
		tlist_add(tlist,tris[j+0],tris[j+1],tris[j+2]);
		j += 3;
	}

/* create the vertex to triangle table */
	for(i=0;i<tlist->ntris;i++) {
		list_add(v2tlist,tlist->list[i].v[0],i);
		list_add(v2tlist,tlist->list[i].v[1],i);
		list_add(v2tlist,tlist->list[i].v[2],i);
	}

/* set the edge flags */
	if (flags & OPTS_PRESERVE_EDGES) {
		for(i=0;i<(*n_verts);i++) {
			vert_status[i] = is_edge_v(i,tlist,v2tlist);
#ifdef DEBUG
printf("Edge: %d %d\n",i,vert_status[i]);
#endif
		}
	} else {
		memset(vert_status,0,(*n_verts)*sizeof(long int));
	}

/* create the vertex to vertex (edges) list */
	for(i=0;i<tlist->ntris;i++) {
		long int	*v = tlist->list[i].v;

		list_add(v2vlist,v[0],v[1]);
		list_add(v2vlist,v[1],v[0]);

		list_add(v2vlist,v[0],v[2]);
		list_add(v2vlist,v[2],v[0]);

		list_add(v2vlist,v[2],v[1]);
		list_add(v2vlist,v[1],v[2]);
	}

/* here we go collapsing one edge and removing one vert with each pass */
	count = tlist->ntris - ask_tris;
	while(count > 0) {
		float		d;
		long int	n_rem;

		n_rem = 0;

		if (block_size == 1) {
			pending[0].vold = -1;

			/* find the best edge */
			for(i=0;i<*n_verts;i++) {
			    if (vert_status[i] == 0) {
				k=calc_edge_cost(i,&d,tlist,v2tlist,v2vlist,0);
				if (k >= 0) {
				    if ((pending[0].vold == -1) || 
				        (d < pending[0].cost)) {
					pending[0].vold = i;
					pending[0].vnew = k;
					pending[0].cost = d;
					n_rem = 1;
				    }
				}
			    }
			}
		} else {
			for(i=0;i<*n_verts;i++) {
			    if (vert_status[i] == 0) {
				k=calc_edge_cost(i,&d,tlist,v2tlist,v2vlist,0);
				if (k >= 0) {
				    pending[n_rem].vold = i;
				    pending[n_rem].vnew = k;
				    pending[n_rem].cost = d;
				    n_rem += 1;

				    /* pack the array ? */
				    if (n_rem == big_block_size) {
					qsort(pending,n_rem,sizeof(collapse),
						comp_collapse);
					n_rem = block_size;
				    }
				}
			    }
			}
			/* remove "blocksize" verts */
			if (n_rem > block_size) n_rem = block_size;
		}

/* if none could be found, stop */
		if (n_rem < 1) break;

		for(i=0;i<n_rem;i++) {
			if ((pending[i].vold >= 0) && (count > 0)) {
#ifdef DEBUG
printf("%d - %d->%d  cost:%f\n",count,pending[i].vold,pending[i].vnew,
		pending[i].cost);
#endif
			    if (flags & OPTS_PRESERVE_EDGES) {
				tetra_check(pending[i].vold,vert_status,
				    v2vlist,v2tlist);
			    }
			    if (vert_status[pending[i].vold] == 0) {
				/* remove the vertex */
				if (remove_edge(pending[i].vold,pending[i].vnew,
				      tlist, v2tlist,v2vlist)) {
					vert_status[pending[i].vold] = -1;
					count -= 2;
				}
			        /* purge any linked verts in pending list */
				for(j=i+1;j<n_rem;j++) {
				    if (pending[j].vold >= 0) {
					element		*elem =
					    &(v2vlist->list[pending[i].vnew]);
					if ((pending[j].vold ==
							pending[i].vold) ||
						(pending[j].vnew ==
							pending[i].vold)) {
						pending[j].vold = -1;
					} else {
					    for(k=0;k<elem->count;k++) {
					        if ((pending[j].vold ==
							elem->val[k]) ||
						    (pending[j].vnew ==
							elem->val[k])) {
						    pending[j].vold = -1;
						    break;
						}
					    }
					}
				    }
				}
			    }
			}
		}
	}

/* compress vertex array (and build renumbering table) */
	j = 0;
	for(i=0;i<*n_verts;i++) {
		if (vert_status[i] >= 0) {
			vert_status[i] = j;
			memcpy(verts+(3*j),verts+(3*i),3*sizeof(float)); 
			if (aux) {
				memcpy(aux+(j*n_vaux),aux+(i*n_vaux),
					n_vaux*sizeof(float));
			}
			j++;
		}
	}
	*n_verts = j;

/* compress the triangle list (and renumber) */
	j = 0;
	for(i=0;i<*n_tris;i++) {
		if (tlist->list[i].v[0] >= 0) {
			tris[j++] = vert_status[tlist->list[i].v[0]];
			tris[j++] = vert_status[tlist->list[i].v[1]];
			tris[j++] = vert_status[tlist->list[i].v[2]];
#ifdef DEBUG
if (tris[j-1] < 0) printf("error: neg tri %d-%d\n",i,tlist->list[i].v[0]);
if (tris[j-2] < 0) printf("error: neg tri %d-%d\n",i,tlist->list[i].v[1]);
if (tris[j-3] < 0) printf("error: neg tri %d-%d\n",i,tlist->list[i].v[2]);
#endif
		}
	}
	*n_tris = j/3;

/* all done! */
	list_free(v2tlist);
	list_free(v2vlist);
	tlist_free(tlist);
	FREE(vert_status);
	FREE(pending);

	return(1);

err_out:
/* clean up the arrays */
	if (pending)  FREE(pending);
	if (vert_status)  FREE(vert_status);
	if (tlist)  tlist_free(tlist);
	if (v2vlist) list_free(v2vlist);
	if (v2tlist) list_free(v2tlist);

	return(0);
}
