#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "export.h"
#include "idl_tools.h"

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

/* function protos */
extern IDL_VPTR IDL_CDECL mesh_thin(int argc, IDL_VPTR argv[], char *argk);
extern void IDL_CDECL idl_slab(int argc, IDL_VPTR argv[], char *argk);
#ifdef USE_GTS
extern IDL_VPTR IDL_CDECL mesh_union(int argc, IDL_VPTR argv[], char *argk);
extern IDL_VPTR IDL_CDECL mesh_intersect(int argc, IDL_VPTR argv[], char *argk);
#endif

/* define the MESH functions */
static IDL_SYSFUN_DEF mesh_functions[] = {
	{ mesh_thin,	"MESH_THIN",	5, 5, IDL_SYSFUN_DEF_F_KEYWORDS},
#ifdef USE_GTS
	{ mesh_union,	"MESH_UNION",   6, 6, IDL_SYSFUN_DEF_F_KEYWORDS},
	{ mesh_intersect,	"MESH_INTERSECTION",   6, 6, 0},
#endif
};
static IDL_SYSFUN_DEF slab_procedure[] = {
	{ (IDL_FUN_RET)idl_slab,	"SLAB",	2, 2, IDL_SYSFUN_DEF_F_KEYWORDS},
};

/* startup call when DLM is loaded */
int mesh_startup(void)
{
        if (!IDL_AddSystemRoutine(mesh_functions, TRUE, 
                ARRLEN(mesh_functions))) {
                return IDL_FALSE;
        }
        if (!IDL_AddSystemRoutine(slab_procedure, FALSE, 
                ARRLEN(slab_procedure))) {
                return IDL_FALSE;
        }
        IDL_ExitRegister(mesh_exit_handler);

        return(IDL_TRUE);
}

/* called when IDL is shutdown */
void mesh_exit_handler(void)
{
}

#ifdef USE_GTS
/* local prototypes */
static void build_list(gpointer data, GList **list);
static void segment_cleanup(GtsSurface * surface);
static void prepend_triangle_bbox(GtsTriangle *t, GSList **bboxes);
static GtsSurface *create_gts_surface(IDL_VPTR *argv,int inv,GNode **tree);
static void tag_verts(GtsSegment *s, IDL_LONG *i);
static void collect_verts(GtsSegment *s, float *fp);
static void collect_edges(GtsSegment *s, void *d[2]);
static void vert_count(GtsPoint *p, IDL_LONG *i);
static void add_point(GtsPoint *p, void *d[2]);
static void add_triangle(GtsTriangle *t, void *d[2]);
static void reset_private(GtsTriangle *t, void *dummy);

static void build_list(gpointer data, GList **list)
{
  /* always use O(1) g_list_prepend instead of O(n) g_list_append */
  *list = g_list_prepend(*list, data);
}

static void segment_cleanup(GtsSurface * surface)
{
  GList * segments = NULL;
  GList * i;

  g_return_if_fail(surface != NULL);

  /* build list of segments */
  gts_surface_foreach_segment(surface, (GtsFunc)build_list, &segments);
  
  /* remove degenerate and duplicate segments.
   Note: we could use gts_segments_merge() to remove the duplicates and then
  remove the degenerate segments but it is more efficient to do everything 
  at once (and it's more pedagogical too ...) */
  i = segments;
  while (i) {
    GtsSegment *s = i->data;
    GtsSegment *duplicate;
    if (s->p1 == s->p2) /* segment is degenerate */
      /* destroy s and its endpoints (if they are not used 
	 by any other segment) */
      gts_segment_destroy (s, TRUE);
    else if ((duplicate = gts_segment_is_duplicate (s))) {
      /* replace s with its duplicate */
      gts_segment_replace (s, duplicate);
      /* destroy s (no need to try to destroy the endpoints as they are
	 used by duplicate) */
      gts_segment_destroy(s, FALSE);
    }
    i = i->next;
  }

  /* free list of segments */
  g_list_free (segments);
}

static void prepend_triangle_bbox(GtsTriangle *t, GSList **bboxes)
{
  *bboxes = g_slist_prepend(*bboxes, gts_bbox_triangle(t));
  return;
}

static GtsSurface *create_gts_surface(IDL_VPTR *argv,int inv,GNode **tree)
{
	IDL_LONG	*piConn;
	IDL_LONG	iConn;
	float		*pfVerts;
	IDL_LONG	iVerts;
	IDL_LONG	i,j;
	IDL_VPTR	vpTmp=NULL;
	IDL_VPTR	vpVerts=NULL,vpConn=NULL;
	GtsSurface	*surf;
	GtsPoint	**pPoints;

/* surface 1 */
	vpVerts = IDL_CvtFlt(1, &(argv[0]));
	pfVerts = (float *)vpVerts->value.arr->data;
	iVerts = vpVerts->value.arr->n_elts/3;

	vpConn = IDL_CvtLng(1, &(argv[1]));
	piConn = (IDL_LONG *)vpConn->value.arr->data;
	iConn = vpConn->value.arr->n_elts;

	pPoints = (GtsPoint **)IDL_GetScratch(&vpTmp, 1, iVerts*sizeof(GtsPoint *));
    for(i=0;i<iVerts;i++) {
		pPoints[i] = gts_point_new(pfVerts[0],pfVerts[1],pfVerts[2]);
		pfVerts += 3;
		if (!pPoints[i]) {
			for(j=0;j<i;j++) gts_point_destroy(pPoints[j]);
			return(NULL);
		}
	}

	/* ok, start the surface */
	surf = gts_surface_new();
	if (!surf) {
		for(i=0;i<iVerts;i++) gts_point_destroy(pPoints[i]);
		return(NULL);
	}

	/* convert the connectivity list into a triangle list and
	   add to the current surface */
	i = 0;
	while ((i < iConn) && (piConn[i] >= 0)) {
		if (piConn[i] >= 3) {
/* we only deal w/ triangles... (assume others convex) */
			for(j=0;j<(piConn[i]-2);j++) {
			    if ((piConn[i+j+1] >= 0) && 
                                (piConn[i+j+1] < iVerts) && 
                                (piConn[i+j+2] >= 0) && 
                                (piConn[i+j+2] < iVerts) &&
                                (piConn[i+piConn[i]] >= 0) && 
                                (piConn[i+piConn[i]] < iVerts)) {
					GtsSegment	*seg1,*seg2,*seg3;
					GtsTriangle	*tri;
					seg1 = gts_segment_new(pPoints[piConn[i+j+1]],pPoints[piConn[i+j+2]]);
					seg2 = gts_segment_new(pPoints[piConn[i+j+2]],pPoints[piConn[i+piConn[i]]]);
					seg3 = gts_segment_new(pPoints[piConn[i+piConn[i]]],pPoints[piConn[i+j+1]]);
					tri = gts_triangle_new(seg1,seg2,seg3);
					gts_surface_add_triangle(surf,tri);
				}
			}
		} 
		i += (piConn[i]+1);
	} 

/* merge the segments... */
	segment_cleanup(surf);

/* invert */
	if (inv) {
		gts_surface_foreach_triangle(surf, (GtsFunc)gts_triangle_revert, NULL);
	}

/* build the bbox tree */
	if (tree) {
		  GSList *bboxes = NULL;
		  gts_surface_foreach_triangle(surf,(GtsFunc)prepend_triangle_bbox, &bboxes);
		  /* build bounding box tree */
		  *tree = gts_bb_tree_new (bboxes);
          /* free list of bboxes */
          g_slist_free (bboxes);
	}

/* check validity */
	if ((!gts_surface_is_orientable(surf)) || 
		(!gts_surface_is_closed(surf))) {
		if (*tree) gts_bb_tree_destroy(*tree, TRUE);
		gts_surface_destroy(surf,TRUE);
		IDL_MessageFromBlock(msg_block, M_TOOLS_ERROR,
			IDL_MSG_LONGJMP,"Input surface is not closed or orientable.");
	}

/* get out */
	if (vpVerts != argv[0]) IDL_Deltmp(vpVerts);
	if (vpConn != argv[1]) IDL_Deltmp(vpConn);
    IDL_Deltmp(vpTmp);

	return(surf);
}

/*
 * IDL API:
 *	nedges = MESH_INTERSECT(verts1,conn1,verts2,conn2,vertsout,connout)
 *
 * TODO:
 *		???
 */
static void tag_verts(GtsSegment *s, IDL_LONG *i)
{
	if (i) {
		if (!s->p1->private) {
			s->p1->private = GINT_TO_POINTER(*i);
			*i += 1;
		}
		if (!s->p2->private) {
			s->p2->private = GINT_TO_POINTER(*i);
			*i += 1;
		}
	} else {
		s->p1->private = NULL;
		s->p2->private = NULL;
	}
}
static void collect_verts(GtsSegment *s, float *fp)
{
	IDL_LONG	i;
	i = (GPOINTER_TO_INT(s->p1->private)-1)*3;
	fp[i++] = s->p1->x;
	fp[i++] = s->p1->y;
	fp[i++] = s->p1->z;
	i = (GPOINTER_TO_INT(s->p2->private)-1)*3;
	fp[i++] = s->p2->x;
	fp[i++] = s->p2->y;
	fp[i++] = s->p2->z;
	return;
}
static void collect_edges(GtsSegment *s, void *d[2])
{
	IDL_LONG	*i = (IDL_LONG *)d[0];
	IDL_LONG	*conn = (IDL_LONG *)d[1];

	conn[(*i)+0] = 2;
	conn[(*i)+1] = GPOINTER_TO_INT(s->p1->private)-1;
	conn[(*i)+2] = GPOINTER_TO_INT(s->p2->private)-1;
	(*i) += 3;
}

IDL_VPTR IDL_CDECL mesh_intersect(int argc, IDL_VPTR argv[], char *argk)
{
	GtsSurface	*surf1,*surf2;
	GSList		*list;
	GNode		*tree1,*tree2;
	IDL_LONG	nVerts,nSegs;
	IDL_VPTR	vpTmp;

	IDL_ENSURE_ARRAY(argv[0]);
	IDL_ENSURE_ARRAY(argv[1]);
	IDL_ENSURE_ARRAY(argv[2]);
	IDL_ENSURE_ARRAY(argv[3]);
	IDL_EXCLUDE_EXPR(argv[4]);
	IDL_EXCLUDE_EXPR(argv[5]);

	surf1 = create_gts_surface(argv+0,0,&tree1);
	if (!surf1) {
		IDL_MessageFromBlock(msg_block, M_TOOLS_ERROR,
			IDL_MSG_LONGJMP,"Unable to create surface");
	}
	surf2 = create_gts_surface(argv+2,0,&tree2);
	if (!surf2) {
		gts_surface_destroy(surf1,TRUE);
		gts_bb_tree_destroy(tree1, TRUE);
		IDL_MessageFromBlock(msg_block, M_TOOLS_ERROR,
			IDL_MSG_LONGJMP,"Unable to create surface");
	}

	/* compute the edges */
	list = gts_surfaces_intersection(surf1,surf2,tree1,tree2);
    if (!list) {
		gts_surface_destroy(surf1, TRUE);
		gts_surface_destroy(surf2, TRUE);
		gts_bb_tree_destroy(tree1, TRUE);
		gts_bb_tree_destroy(tree2, TRUE);
		return(IDL_GettmpLong(-1));
	}

	/* count the verts and tag them */
	nSegs = g_slist_length(list);
	g_slist_foreach(list, (GFunc)tag_verts, NULL);
	nVerts = 1;
	g_slist_foreach(list, (GFunc)tag_verts, &nVerts);
	nVerts -= 1;

	/* allocate output space */
	if ((nSegs > 0) && (nVerts > 0)) {
		IDL_MEMINT	dim[IDL_MAX_ARRAY_DIM];
		float		*pfOut;
		IDL_LONG	*piConn,i;
		void		*pData[2];

		dim[0] = 3;
		dim[1] = nVerts;
		pfOut = (float *)IDL_MakeTempArray(IDL_TYP_FLOAT,2,dim, 
			IDL_BARR_INI_NOP,&vpTmp);
		IDL_VarCopy(vpTmp,argv[4]);

		piConn = (IDL_LONG *)IDL_MakeTempVector(IDL_TYP_LONG,
			nSegs*3,IDL_BARR_INI_NOP,&vpTmp);
		IDL_VarCopy(vpTmp,argv[5]);

		/* copy the data */
		g_slist_foreach(list, (GFunc)collect_verts, pfOut);
		i = 0;
		pData[0] = &i;
		pData[1] = piConn;
		g_slist_foreach(list, (GFunc)collect_edges, pData);
		g_slist_foreach(list, (GFunc)tag_verts, NULL);
	}

	/* cleanup */
	gts_surface_destroy(surf1, TRUE);
	gts_surface_destroy(surf2, TRUE);
	gts_bb_tree_destroy(tree1, TRUE);
	gts_bb_tree_destroy(tree2, TRUE);
	g_slist_free (list);

	if ((nSegs > 0) && (nVerts > 0)) {
		return(IDL_GettmpLong(nSegs));
	} else {
		return(IDL_GettmpLong(0));
	}
}

/*
 * IDL API:
 *	ntris = MESH_UNION(verts1,conn1,verts2,conn2,vertsout,connout
 *          [,/INVERT1][,/INVERT2][/INVERTOUTPUT])
 *
 * TODO:
 *
 * Issues:
 *	If two surfaces do not intersect, the call will not work (not crash)
 *    it works off of intersection segments
 *  This can be a problem if a surface has two chunks and either one
 *    or the other are not intersected
 * Bugs: 
 *	Cube - Cone in csg.pro
 *
 */

/* some callback functions */
static void vert_count(GtsPoint *p, IDL_LONG *i)
{
  IDL_LONG	j = (*i);
  p->private = GINT_TO_POINTER(j);
  (*i) = j + 1;
}
static void add_point(GtsPoint *p, void *d[2])
{
  IDL_LONG	*i = (IDL_LONG *)d[0];
  float		*fp = (float *)d[1];

  fp[(*i)+0] = p->x;
  fp[(*i)+1] = p->y;
  fp[(*i)+2] = p->z;
  (*i) += 3;
}
static void add_triangle(GtsTriangle *t, void *d[2])
{
  IDL_LONG	j,*i = (IDL_LONG *)d[0];
  IDL_LONG	*conn = (IDL_LONG *)d[1];
  GtsPoint	*p1,*p2,*p3;

  gts_triangle_points(t,p1,p2,p3);

  conn[(*i)+0] = 3;
  conn[(*i)+1] = GPOINTER_TO_INT(p1->private);
  conn[(*i)+2] = GPOINTER_TO_INT(p2->private);
  conn[(*i)+3] = GPOINTER_TO_INT(p3->private);

  (*i) += 4;
}
static void reset_private(GtsTriangle *t, void *dummy)
{
  GtsSegment * s1 = t->s1, * s2 = t->s2, * s3 = t->s3;

  s1->private = s2->private = s3->private = NULL;
  s1->p1->private = s1->p2->private = NULL;
  s2->p1->private = s2->p2->private = NULL;
  s3->p1->private = s3->p2->private = NULL;
}

IDL_VPTR IDL_CDECL mesh_union(int argc, IDL_VPTR inargv[], char *argk)
{
	IDL_VPTR	argv[6];
	IDL_VPTR	vpTmp=NULL;
	GtsSurface	*surf1,*surf2;
	GNode		*tree1,*tree2;
	GtsSurface	*surf3;
	IDL_LONG	nTris,nVerts;

static	IDL_LONG	iInv2;
static	IDL_LONG	iInv1;
static	IDL_LONG	iInv3;
static  IDL_KW_PAR      kw_pars[] = { IDL_KW_FAST_SCAN,
	{"INVERT1",IDL_TYP_LONG,1,IDL_KW_ZERO,0,IDL_CHARA(iInv1)},
	{"INVERT2",IDL_TYP_LONG,1,IDL_KW_ZERO,0,IDL_CHARA(iInv2)},
	{"INVERTOUTPUT",IDL_TYP_LONG,1,IDL_KW_ZERO,0,IDL_CHARA(iInv3)},
	{NULL}
	};

	IDL_KWCleanup(IDL_KW_MARK);
	IDL_KWGetParams(argc,inargv,argk,kw_pars,argv,1);

	IDL_ENSURE_ARRAY(argv[0]);
	IDL_ENSURE_ARRAY(argv[1]);
	IDL_ENSURE_ARRAY(argv[2]);
	IDL_ENSURE_ARRAY(argv[3]);
	IDL_EXCLUDE_EXPR(argv[4]);
	IDL_EXCLUDE_EXPR(argv[5]);

	surf1 = create_gts_surface(argv+0,iInv1,&tree1);
	if (!surf1) {
		IDL_MessageFromBlock(msg_block, M_TOOLS_ERROR,
			IDL_MSG_LONGJMP,"Unable to create surface");
	}
	surf2 = create_gts_surface(argv+2,iInv2,&tree2);
	if (!surf2) {
		gts_surface_destroy(surf1,TRUE);
		gts_bb_tree_destroy(tree1, TRUE);
		IDL_MessageFromBlock(msg_block, M_TOOLS_ERROR,
			IDL_MSG_LONGJMP,"Unable to create surface");
	}

/* here we go... */
	surf3 = gts_surfaces_union(surf1,surf2,tree1,tree2);
	if (!surf3) {
		gts_surface_destroy(surf1,TRUE);
		gts_surface_destroy(surf2,TRUE);
		gts_bb_tree_destroy(tree1, TRUE);
		gts_bb_tree_destroy(tree2, TRUE);
		IDL_KWCleanup(IDL_KW_CLEAN);
		return(IDL_GettmpLong(-1));
	}
/* quick test for validity */
	if ((!gts_surface_is_orientable(surf3)) || 
		(!gts_surface_is_closed(surf3))) {
		IDL_MessageFromBlock(msg_block, M_TOOLS_ERROR,
			IDL_MSG_INFO,"Output surface is not closed or orientable.");
	}
/* invert the output if requested*/
	if (iInv3) {
		gts_surface_foreach_triangle(surf3, 
			(GtsFunc)gts_triangle_revert, NULL);
	}
/* build the output */
/* count the tris */
	nTris = gts_surface_triangle_number(surf3);
/* tag the verts */
    nVerts = 0;
	gts_surface_foreach_point(surf3,(GtsFunc)vert_count, &nVerts);

	/* allocate output space */
	if ((nTris > 0) && (nVerts > 0)) {
		IDL_MEMINT	dim[IDL_MAX_ARRAY_DIM];
		float		*pfOut;
		IDL_LONG	*piConn,i;
		void		*pData[2];

		dim[0] = 3;
		dim[1] = nVerts;
		pfOut = (float *)IDL_MakeTempArray(IDL_TYP_FLOAT,2,dim, 
			IDL_BARR_INI_NOP,&vpTmp);
		IDL_VarCopy(vpTmp,argv[4]);

		piConn = (IDL_LONG *)IDL_MakeTempVector(IDL_TYP_LONG,
			nTris*4,IDL_BARR_INI_NOP,&vpTmp);
		IDL_VarCopy(vpTmp,argv[5]);

		/* copy the data */
		i = 0;
		pData[0] = &i;
		pData[1] = pfOut;
		gts_surface_foreach_point(surf3,(GtsFunc)add_point, pData);
		i = 0;
		pData[0] = &i;
		pData[1] = piConn;
		gts_surface_foreach_triangle(surf3,(GtsFunc)add_triangle, pData);
		gts_surface_foreach_triangle(surf3,(GtsFunc)reset_private, NULL);
	}

/* cleanup */
	gts_surface_destroy(surf1,TRUE);
	gts_surface_destroy(surf2,TRUE);
	gts_bb_tree_destroy(tree1, TRUE);
	gts_bb_tree_destroy(tree2, TRUE);
	gts_surface_destroy(surf3,TRUE);

	IDL_KWCleanup(IDL_KW_CLEAN);

	if ((nTris > 0) && (nVerts > 0)) {
		return(IDL_GettmpLong(nTris));
	} else {
		return(IDL_GettmpLong(0));
	}
}
#endif


/* local prototypes */
static long int list_conv(list *l,long int idx,long int vorig,long int vnew);
static long int list_del(list *l,long int idx,long int value);
static long int list_add(list *l,long int idx,long int value);
static list *list_alloc(long int num);
static void list_free(list *p);

static void tetra_check(long int v, long int *vert_status,list *v2vlist,
	list *v2tlist);
static long int remove_edge(long int rem_vert,long int new_vert,
	tlist *tlist,list *v2tlist,list *v2vlist,long int);
static long int calc_edge_cost(long int v,float *cost,
	tlist *tlist,list *v2tlist,list *v2vlist,long int flags,float );
static long int collapse_cost(long int v0,long int v1,tlist *tlist,
	list *v2tlist,float *cost,long int flags,float );
static long int next_v(triangle *t,long int base, long int chain);
static long int is_edge_v(long int v,tlist *tlist,list *v2tlist);
static void calc_normal(long int *idx,float *v,float *n);

static tlist *tlist_alloc(long int num,float *verts,
	IDL_LONG iAuxType,IDL_LONG iAuxSize,char *pAux,float *pfWeights);
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
	IDL_LONG iAuxType,IDL_LONG iAuxSize,char *pAux,float *pfWeights)
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

static void calc_normal(long int *idx,float *v,float *n)
{
	long int	j0,j1,j2;
	float		t[3],v0[3],v1[3];
	float		len;

	j0 = idx[0]*3;
	j1 = idx[1]*3;
	j2 = idx[2]*3;

	v0[0] = v[j1+0] - v[j0+0];
	v0[1] = v[j1+1] - v[j0+1];
	v0[2] = v[j1+2] - v[j0+2];

	v1[0] = v[j2+0] - v[j0+0];
	v1[1] = v[j2+1] - v[j0+1];
	v1[2] = v[j2+2] - v[j0+2];

	t[0] = v0[1]*v1[2] - v0[2]*v1[1];
	t[1] = v0[2]*v1[0] - v0[0]*v1[2];
	t[2] = v0[0]*v1[1] - v0[1]*v1[0];

	len = t[0]*t[0] + t[1]*t[1] + t[2]*t[2];
	if (len > 0) {
		len = (float)sqrt(len);
	} else {
		len = 1.0;
	}

	n[0] = t[0]/len;
	n[1] = t[1]/len;
	n[2] = t[2]/len;
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
	list *v2tlist, float *cost,long int flags,float scale)
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

	/* normal inversion test */
	for(i=0;i<v2tlist->list[v0].count;i++) {
		long int        t = v2tlist->list[v0].val[i];	
		float		n[3],dot;
		long int	idx[3];
			
		idx[0] = tlist->list[t].v[0];
		if (idx[0] == v1) continue;
		if (idx[0] == v0) idx[0] = v1;

		idx[1] = tlist->list[t].v[1];
		if (idx[1] == v1) continue;
		if (idx[1] == v0) idx[1] = v1;

		idx[2] = tlist->list[t].v[2];
		if (idx[2] == v1) continue;
		if (idx[2] == v0) idx[2] = v1;

		calc_normal(idx,tlist->verts,n);

		dot  = n[0]*tlist->list[t].n[0];
		dot += n[0]*tlist->list[t].n[0];
		dot += n[0]*tlist->list[t].n[0];

		if (dot < -0.6) return(0);
	}

	if (flags == 1) {
		float	var = 0;
		for(i=0;i<v2tlist->list[v0].count;i++) {
			long int        t = v2tlist->list[v0].val[i];
		
			/* compute curv */
			for(j=0;j<c;j++) {
				float	dot;

				dot  = tlist->list[t].n[0]*tlist->list[c_tris[j]].n[0];
				dot += tlist->list[t].n[1]*tlist->list[c_tris[j]].n[1];
				dot += tlist->list[t].n[2]*tlist->list[c_tris[j]].n[2];
				dot = (1.0f-dot)*0.5f;

				var += dot;
			}
		}
		var /= (float)(2.0*v2tlist->list[v0].count);

		/* compute edge length */
		len  = (tlist->verts[v0*3+0] - tlist->verts[v1*3+0]) *
			(tlist->verts[v0*3+0] - tlist->verts[v1*3+0]);
		len += (tlist->verts[v0*3+1] - tlist->verts[v1*3+1]) *
			(tlist->verts[v0*3+1] - tlist->verts[v1*3+1]);
		len += (tlist->verts[v0*3+2] - tlist->verts[v1*3+2]) *
			(tlist->verts[v0*3+2] - tlist->verts[v1*3+2]);

		if (len > 0.f) len = (float)sqrt(len)/scale;

		*cost = (len)*(var);
		
	} else {
		/* compute curv */
		for(i=0;i<v2tlist->list[v0].count;i++) {
			long int        t = v2tlist->list[v0].val[i];
			float		mcurv = 1.0f;
		
			/* compute curv */
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

		if (len > 0.f) len = (float)sqrt(len)/scale;
/*
	{
		char	tstr[256];
		sprintf(tstr,"%d %d - %f %f %f",v0,v1,len,curv,len*curv);
		IDL_MessageFromBlock(msg_block, M_TOOLS_ERROR, 
			IDL_MSG_RET,tstr);
	}
*/
		*cost = (len)*(curv);
	}
	
	if (tlist->pAux) {
		if (tlist->iAuxType == IDL_TYP_BYTE) {
			UCHAR	*p0 = ((UCHAR *)tlist->pAux) + (tlist->iAuxSize*v0);
			UCHAR	*p1 = ((UCHAR *)tlist->pAux) + (tlist->iAuxSize*v1);
			for(i=0;i<tlist->iAuxSize;i++) {
				wei = (float)((*p0++) - (*p1++));
				wei = IDL_ABS(wei);
				if (tlist->pfWeights) wei = tlist->pfWeights[i]*wei;
				*cost += wei;
			}
		} else {
			float	*p0 = ((float*)tlist->pAux) + (tlist->iAuxSize*v0);
			float	*p1 = ((float*)tlist->pAux) + (tlist->iAuxSize*v1);
			for(i=0;i<tlist->iAuxSize;i++) {
				wei = (*p0++) - (*p1++);
				wei = IDL_ABS(wei);
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
	tlist *tlist,list *v2tlist,list *v2vlist,long int flags,
	float scale)
{
	long int	min_dest = -1;
	float		d,dmin = 0;
	long int	i,status;

	/* compute for folding this vertex into each connected one */
	for(i=0;i<v2vlist->list[v].count;i++) {
		long int	vdest = v2vlist->list[v].val[i];

		status = collapse_cost(v,vdest,tlist,v2tlist,&d,flags,scale);
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
	tlist *tlist,list *v2tlist,list *v2vlist,long int newverts)
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

	if (newverts) {
		for(i=0;i<3;i++) {
			tlist->verts[new_vert*3+i] += tlist->verts[rem_vert*3+i];
			tlist->verts[new_vert*3+i] *= 0.5;
		}
	}

	return(1);
}

/*
 * IDL API:
 *	ntris = MESH_THIN(verts,conn,numtris,vout,cout[,BLOCKSIZE=b]
 *		[,/PRESERVE_EDGES][,AUXDATA_IN=ain][,AUXDATA_OUT=aout]
 *              [,AUXDATA_WEIGHTS=w][/NEW_VERTS])
 *
 *	TODO:
 *		Add surface "flatness" cost function
 *		Switch to incremental vertex evaluation (maybe remove blocking)
 *		Threaded implementation
 *		Performance tune (someday over the rainbow...)
 */
IDL_VPTR IDL_CDECL mesh_thin(int argc, IDL_VPTR inargv[], char *argk)
{
	IDL_VPTR	argv[5],vpVerts=NULL,vpConn=NULL,vpTmp=NULL;
	IDL_LONG	*piConn;
	IDL_LONG	iConn;
	float		*pfVerts;
	IDL_LONG	iVerts;
	IDL_LONG	iReqTris,iTris;
	IDL_LONG	i,j,k;
	long int	iFlags = 0;
	tlist		*tlist = NULL;
	list		*v2tlist = NULL;
	list		*v2vlist = NULL;
	long int	*vert_status = NULL;
	collapse	*pending = NULL;
	long int	big_block_size,count;
	float		*pfV,scale;

	IDL_LONG	iAuxSize = 0;
	IDL_LONG	iAuxType = 0;
	IDL_VPTR	vpAux = NULL;
	char		*pAux = NULL;
	float		*pfWeights = NULL;
	IDL_VPTR	vpWeights = NULL;

static	IDL_LONG	iBlocksize;
static	IDL_LONG	iNewVerts;
static	IDL_LONG	iPreserveEdges;
static	IDL_LONG	iSmooth;
static	IDL_VPTR	vpAuxIn;
static	IDL_VPTR	vpAuxOut;
static	IDL_VPTR	vpWeightsIn;
static  IDL_KW_PAR      kw_pars[] = { IDL_KW_FAST_SCAN,
	{"AUXDATA_IN",IDL_TYP_UNDEF,1,IDL_KW_VIN|IDL_KW_ZERO,0,IDL_CHARA(vpAuxIn)},
	{"AUXDATA_OUT",IDL_TYP_UNDEF,1,IDL_KW_OUT|IDL_KW_ZERO,0,IDL_CHARA(vpAuxOut)},
	{"AUXDATA_WEIGHTS",IDL_TYP_UNDEF,1,IDL_KW_VIN|IDL_KW_ZERO,0,IDL_CHARA(vpWeightsIn)},
	{"BLOCKSIZE",IDL_TYP_LONG,1,IDL_KW_ZERO,0,IDL_CHARA(iBlocksize)},
	{"NEW_VERTS",IDL_TYP_LONG,1,IDL_KW_ZERO,0,IDL_CHARA(iNewVerts)},
	{"PRESERVE_EDGES",IDL_TYP_LONG,1,IDL_KW_ZERO,0,IDL_CHARA(iPreserveEdges)},
	{"SMOOTH",IDL_TYP_LONG,1,IDL_KW_ZERO,0,IDL_CHARA(iSmooth)},
	{NULL}
	};

	IDL_KWCleanup(IDL_KW_MARK);
	IDL_KWGetParams(argc,inargv,argk,kw_pars,argv,1);

	IDL_EXCLUDE_EXPR(argv[3]);
	IDL_EXCLUDE_EXPR(argv[4]);
	IDL_ENSURE_ARRAY(argv[0]);
	IDL_ENSURE_ARRAY(argv[1]);

	if (iSmooth) iFlags = 1;

	vpVerts = IDL_CvtFlt(1, &(argv[0]));
	pfVerts = (float *)vpVerts->value.arr->data;
	iVerts = vpVerts->value.arr->n_elts/3;
	/* if the user will allow verts to change, we will clone
	 * the vert data here so we can modify it in place */
	if (iNewVerts && (vpVerts == argv[0])) {
		IDL_MEMINT	dim[IDL_MAX_ARRAY_DIM];
		IDL_VPTR	vpDummy;
		dim[0] = 3;
		dim[1] = iVerts;
		pfV = (float *)IDL_MakeTempArray(IDL_TYP_FLOAT,2,dim, 
			IDL_BARR_INI_NOP,&vpDummy);
		memcpy(pfV,pfVerts,iVerts*3*sizeof(float));
		pfVerts = pfV;
		vpVerts = vpDummy;
	}

/* get the data bounds */
	{ 
		float	mx[3],mn[3];

		pfV = pfVerts;
		mx[0] = mn[0] = *pfV++;
		mx[1] = mn[1] = *pfV++;
		mx[2] = mn[2] = *pfV++;
		for(i=1;i<iVerts;i++) {
			if (pfV[0] > mx[0]) mx[0] = pfV[0];
			if (pfV[1] > mx[0]) mx[1] = pfV[1];
			if (pfV[2] > mx[0]) mx[2] = pfV[2];
			if (pfV[0] < mn[0]) mn[0] = pfV[0];
			if (pfV[1] < mn[1]) mn[1] = pfV[1];
			if (pfV[2] < mn[2]) mn[2] = pfV[2];
			pfV += 3;
		}
		scale  = (mx[0] - mn[0])*(mx[0] - mn[0]);
		scale += (mx[1] - mn[1])*(mx[1] - mn[1]);
		scale += (mx[2] - mn[2])*(mx[2] - mn[2]);
		if (scale > 0) {
			scale = (float)sqrt(scale);
		} else {
			scale = 1.0f;
		}
	}

	vpConn = IDL_CvtLng(1, &(argv[1]));
	piConn = (IDL_LONG *)vpConn->value.arr->data;
	iConn = vpConn->value.arr->n_elts;

	iReqTris = IDL_LongScalar(argv[2]);

/* handle aux data */
	if (vpAuxIn) {
		IDL_ENSURE_ARRAY(vpAuxIn);
		if (vpAuxIn->type != IDL_TYP_BYTE) {
			vpAux = IDL_CvtFlt(1,&vpAuxIn);
		} else {
			vpAux = vpAuxIn;
		}
		pAux = (char *)vpAux->value.arr->data;
		iAuxSize = vpAux->value.arr->n_elts/iVerts;
		iAuxType = vpAux->type;
		if (vpAux->value.arr->dim[vpAux->value.arr->n_dim-1]
			!= iVerts) {
			IDL_MessageFromBlock(msg_block, M_TOOLS_BADAUX, 
				IDL_MSG_LONGJMP);
		}
	}
	if (vpWeightsIn) {
		IDL_ENSURE_ARRAY(vpWeightsIn);
		vpWeights = IDL_CvtFlt(1,&vpWeightsIn);
		pfWeights = (float *)vpWeights->value.arr->data;
		if (vpWeights->value.arr->n_elts < iAuxSize) {
			IDL_MessageFromBlock(msg_block, M_TOOLS_BADWEIGHTS, 
				IDL_MSG_LONGJMP);
		}
	}

/* find the number of triangles in the mesh */
	k = 0;
	i = 0;
	while ((i < iConn) && (piConn[i] >= 0)) {
		if (piConn[i] >= 3) k += (piConn[i] - 2);
		i += (piConn[i]+1);
	} 
	iTris = k;

/* get memory for the access tables */
	tlist = tlist_alloc(iTris,pfVerts,iAuxType,iAuxSize,pAux,pfWeights);
	if (!tlist) goto err_out;
	v2tlist = list_alloc(iVerts);
	if (!v2tlist) goto err_out;
	v2vlist = list_alloc(iVerts);
	if (!v2vlist) goto err_out;
	vert_status = (long int *)MALLOC((iVerts)*sizeof(long int));
	if (!vert_status) goto err_out;
	if (iBlocksize <= 0) iBlocksize = (IDL_LONG)(0.1*iTris)+1;
	big_block_size = (long int)(iBlocksize*2);
	pending = (collapse *)MALLOC(big_block_size*sizeof(collapse));
	if (!pending) goto err_out;

/* convert the connectivity list into a triangle list and */
	i = 0;
	while ((i < iConn) && (piConn[i] >= 0)) {
		if (piConn[i] >= 3) {
/* we only deal w/ triangles... (assume others convex) */
			for(j=0;j<(piConn[i]-2);j++) {
			    if ((piConn[i+j+1] >= 0) && 
                                (piConn[i+j+1] < iVerts) && 
                                (piConn[i+j+2] >= 0) && 
                                (piConn[i+j+2] < iVerts) &&
                                (piConn[i+piConn[i]] >= 0) && 
                                (piConn[i+piConn[i]] < iVerts)) {

				tlist_add(tlist,piConn[i+j+1],piConn[i+j+2],
					piConn[i+piConn[i]]);
			    }
			}
		} 
		i += (piConn[i]+1);
	} 
	if (vpConn && (vpConn != argv[1])) IDL_Deltmp(vpConn);
	vpConn = NULL;

/* create the vertex to triangle table */
	for(i=0;i<tlist->ntris;i++) {
		list_add(v2tlist,tlist->list[i].v[0],i);
		list_add(v2tlist,tlist->list[i].v[1],i);
		list_add(v2tlist,tlist->list[i].v[2],i);
	}

/* set the edge flags */
	if (iPreserveEdges) {
		for(i=0;i<iVerts;i++) {
			vert_status[i] = is_edge_v(i,tlist,v2tlist);
#ifdef DEBUG
printf("Edge: %d %d\n",i,vert_status[i]);
#endif
		}
	} else {
		memset(vert_status,0,(iVerts)*sizeof(long int));
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
	count = tlist->ntris - iReqTris;
	while(count > 0) {
		float		d;
		long int	n_rem;

		n_rem = 0;

		if (iBlocksize == 1) {
			pending[0].vold = -1;

			/* find the best edge */
			for(i=0;i<iVerts;i++) {
			    if (vert_status[i] == 0) {
				k=calc_edge_cost(i,&d,tlist,v2tlist,v2vlist,iFlags,scale);
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
			for(i=0;i<iVerts;i++) {
			    if (vert_status[i] == 0) {
				k=calc_edge_cost(i,&d,tlist,v2tlist,v2vlist,iFlags,scale);
				if (k >= 0) {
				    pending[n_rem].vold = i;
				    pending[n_rem].vnew = k;
				    pending[n_rem].cost = d;
				    n_rem += 1;

				    /* pack the array ? */
				    if (n_rem == big_block_size) {
					qsort(pending,n_rem,sizeof(collapse),
						comp_collapse);
					n_rem = iBlocksize;
				    }
				}
			    }
			}
			/* remove "blocksize" verts */
			if (n_rem > iBlocksize) n_rem = iBlocksize;
		}

/* if none could be found (or the user has hit break, stop */
		if (n_rem < 1) break;
		if (IDL_BailOut(IDL_FALSE)) break;

		for(i=0;i<n_rem;i++) {
			if ((pending[i].vold >= 0) && (count > 0)) {
#ifdef DEBUG
printf("%d - %d->%d  cost:%f\n",count,pending[i].vold,pending[i].vnew,
		pending[i].cost);
#endif
			    if (iPreserveEdges) {
				tetra_check(pending[i].vold,vert_status,
				    v2vlist,v2tlist);
			    }
			    if (vert_status[pending[i].vold] == 0) {
				/* remove the vertex */
				if (remove_edge(pending[i].vold,pending[i].vnew,
				      tlist, v2tlist,v2vlist,iNewVerts)) {
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
{
	IDL_MEMINT	dim[IDL_MAX_ARRAY_DIM],iSize;
	float		*pfOut;
	char		*pAuxOut;

	j = 0;
	for(i=0;i<iVerts;i++) if (vert_status[i] >= 0) j++;
	dim[0] = 3;
	dim[1] = j;
	pfOut = (float *)IDL_MakeTempArray(IDL_TYP_FLOAT,2,dim, 
		IDL_BARR_INI_NOP,&vpTmp);
	IDL_VarCopy(vpTmp,argv[3]);

	/* create AUX output */
	if (vpAuxOut) {
		/* differs only by trailing dim */
		memcpy(dim,vpAux->value.arr->dim,
			vpAux->value.arr->n_dim*sizeof(IDL_MEMINT));
		dim[vpAux->value.arr->n_dim-1] = j;
		pAuxOut = (char *)IDL_MakeTempArray(iAuxType,
			vpAux->value.arr->n_dim,dim, 
			IDL_BARR_INI_NOP,&vpTmp);
		IDL_VarCopy(vpTmp,vpAuxOut);
		iSize = iAuxSize*IDL_TypeSizeFunc(iAuxSize);
	}
	/* collapse verts */
	j = 0;
	for(i=0;i<iVerts;i++) {
		if (vert_status[i] >= 0) {
			/* setup the translate table */
			vert_status[i] = j;
			/* copy the vert */
			memcpy(pfOut+(3*j),pfVerts+(3*i),3*sizeof(float)); 
			/* handle aux data */
			if (vpAuxOut) {
				memcpy(pAuxOut+(j*iSize),pAux+(i*iSize),
					iSize);
			}
			j++;
		}
	}
	if (vpVerts && (vpVerts != argv[0])) IDL_Deltmp(vpVerts);
	vpVerts = NULL;
}


/* compress the triangle list (and renumber) */
	piConn = (IDL_LONG *)IDL_MakeTempVector(IDL_TYP_LONG,
		tlist->ntris*4,IDL_BARR_INI_NOP,&vpTmp);
	IDL_VarCopy(vpTmp,argv[4]);
	j = 0;
	for(i=0;i<tlist->alloc;i++) {
		if (tlist->list[i].v[0] >= 0) {
			piConn[j++] = 3;
			piConn[j++] = vert_status[tlist->list[i].v[0]];
			piConn[j++] = vert_status[tlist->list[i].v[1]];
			piConn[j++] = vert_status[tlist->list[i].v[2]];
#ifdef DEBUG
if (tris[j-1] < 0) printf("error: neg tri %d-%d\n",i,tlist->list[i].v[0]);
if (tris[j-2] < 0) printf("error: neg tri %d-%d\n",i,tlist->list[i].v[1]);
if (tris[j-3] < 0) printf("error: neg tri %d-%d\n",i,tlist->list[i].v[2]);
#endif
		}
	}

/* all done! */
	list_free(v2tlist);
	list_free(v2vlist);
	tlist_free(tlist);
	FREE(vert_status);
	FREE(pending);
	if (vpAux && (vpAux != vpAuxIn)) IDL_Deltmp(vpAux);
	if (vpWeights && (vpWeights != vpWeightsIn)) IDL_Deltmp(vpWeights);

	IDL_KWCleanup(IDL_KW_CLEAN);

	return(IDL_GettmpLong(argv[4]->value.arr->n_elts/4));

err_out:
/* clean up the arrays */
	if (pending)  FREE(pending);
	if (vert_status)  FREE(vert_status);
	if (tlist)  tlist_free(tlist);
	if (v2vlist) list_free(v2vlist);
	if (v2tlist) list_free(v2tlist);

	if (vpVerts && (vpVerts != argv[0])) IDL_Deltmp(vpVerts);
	if (vpConn && (vpConn != argv[1])) IDL_Deltmp(vpConn);
	if (vpAux && (vpAux != vpAuxIn)) IDL_Deltmp(vpAux);
	if (vpWeights && (vpWeights != vpWeightsIn)) IDL_Deltmp(vpWeights);

	IDL_KWCleanup(IDL_KW_CLEAN);

	return(IDL_GettmpLong(-1));
}

/*
 * IDL API:
 *	SLAB,invar,outvar[,POSITION=][,STRIDE=][,SIZE=][,/CLIP]
 *		[,/PASTE_ARRAY[,PASTE_POSITION=][,PASTE_STRIDE=]]
 *
 *		Copy the block of values from invar to outvar.  The block
 *		is of size SIZE (or defaults to INVAR size).  If /PASTE
 *		is specified, OUTVAR must be of the same type as INVAR
 *		and the block of values is "pasted" into the existing
 *		array using OUT_POS and OUT_STRIDE.  The input block is
 *		obtained using POSITION (defaults to zeros) and STRIDE
 *		(defaults to ones).  PASTE_POSITION defaults to zeros and
 *		PASTE_STRIDE defaults to ones.  By default, blocks which
 *		fall outside of the array bounds cause errors.  If /CLIP
 *		is specified, no error is thrown.
 *		
 *	TODO:
 *		Add STRING support
 *		Add STRUCT support
 */
void IDL_CDECL idl_slab(int argc, IDL_VPTR inargv[], char *argk)
{
	IDL_VPTR	argv[2];
	IDL_VPTR	vpTmp;
	IDL_LONG	iNumDims,i,j,iTrunc,dSize;
	char		*pOut,*pIn;
	IDL_LONG	src[IDL_MAX_ARRAY_DIM];
	IDL_LONG	dst[IDL_MAX_ARRAY_DIM];
	IDL_LONG	cnt[IDL_MAX_ARRAY_DIM];
	IDL_LONG	spos,dpos;
	IDL_LONG	sdim[IDL_MAX_ARRAY_DIM+1],ddim[IDL_MAX_ARRAY_DIM+1];

static	IDL_LONG	iClip;
static	IDL_LONG	iPaste;
static  IDL_LONG	piSrcPos[IDL_MAX_ARRAY_DIM];
static  IDL_LONG	piSize[IDL_MAX_ARRAY_DIM];
static  IDL_LONG	piSrcStr[IDL_MAX_ARRAY_DIM];
static  IDL_LONG	piDstStr[IDL_MAX_ARRAY_DIM];
static  IDL_LONG	piDstPos[IDL_MAX_ARRAY_DIM];
static  IDL_KW_ARR_DESC	piSrcPos_A = {(char*)piSrcPos,1,IDL_MAX_ARRAY_DIM,0};
static  IDL_KW_ARR_DESC	piSize_A = {(char*)piSize,1,IDL_MAX_ARRAY_DIM,0};
static  IDL_KW_ARR_DESC	piSrcStr_A = {(char*)piSrcStr,1,IDL_MAX_ARRAY_DIM,0};
static  IDL_KW_ARR_DESC	piDstStr_A = {(char*)piDstStr,1,IDL_MAX_ARRAY_DIM,0};
static  IDL_KW_ARR_DESC	piDstPos_A = {(char*)piDstPos,1,IDL_MAX_ARRAY_DIM,0};
static	int		iHStr,iHSize,iHPos,iHDstStr,iHDstPos;
static  IDL_KW_PAR      kw_pars[] = { IDL_KW_FAST_SCAN,
	{"CLIP",IDL_TYP_LONG,1,IDL_KW_ZERO,0,IDL_CHARA(iClip)},
	{"PASTE_ARRAY",IDL_TYP_LONG,1,IDL_KW_ZERO,0,IDL_CHARA(iPaste)},
	{"PASTE_POSITION",IDL_TYP_LONG,1,IDL_KW_ARRAY,&iHDstPos,
		IDL_CHARA(piDstPos_A)},
	{"PASTE_STRIDE",IDL_TYP_LONG,1,IDL_KW_ARRAY,&iHDstStr,
		IDL_CHARA(piDstStr_A)},
	{"POSITION",IDL_TYP_LONG,1,IDL_KW_ARRAY,&iHPos,IDL_CHARA(piSrcPos_A)},
	{"SIZE",IDL_TYP_LONG,1,IDL_KW_ARRAY,&iHSize,IDL_CHARA(piSize_A)},
	{"STRIDE",IDL_TYP_LONG,1,IDL_KW_ARRAY,&iHStr,IDL_CHARA(piSrcStr_A)},
	{NULL}
	};

	IDL_KWCleanup(IDL_KW_MARK);
	IDL_KWGetParams(argc,inargv,argk,kw_pars,argv,1);

	/* PASTE_XXX not allowed without PASTE */
	if ((!iPaste) && (iHDstPos || iHDstStr)) {
		IDL_MessageFromBlock(msg_block, M_TOOLS_BADKWDS, 
			IDL_MSG_LONGJMP);
	}

	/* arg sanity check */
	IDL_ENSURE_ARRAY(argv[0])
	IDL_EXCLUDE_STRING(argv[0]);
	IDL_EXCLUDE_STRUCT(argv[0]);
	IDL_EXCLUDE_FILE(argv[0]);
	if (iPaste) {
		IDL_ENSURE_ARRAY(argv[1]);
		IDL_EXCLUDE_EXPR(argv[1]);
		IDL_EXCLUDE_FILE(argv[1]);
		if ((argv[0]->type != argv[1]->type) ||
		    (argv[0]->value.arr->n_dim != 
			argv[1]->value.arr->n_dim)) {
			IDL_MessageFromBlock(msg_block, M_TOOLS_BADPASTE, 
				IDL_MSG_LONGJMP);
		}
		vpTmp = argv[1];
	} else {
		IDL_EXCLUDE_EXPR(argv[1]);
		vpTmp = NULL;
	}

	/* check keyword dimensions */
	iNumDims = argv[0]->value.arr->n_dim;
	if (iHDstPos && (piDstPos_A.n < iNumDims)) {
		IDL_MessageFromBlock(msg_block, M_TOOLS_BADDIMS, 
			IDL_MSG_LONGJMP,"PASTE_POSITION");
	}
	if (iHDstStr && (piDstStr_A.n < iNumDims)) {
		IDL_MessageFromBlock(msg_block, M_TOOLS_BADDIMS, 
			IDL_MSG_LONGJMP,"PASTE_STRIDE");
	}
	if (iHPos && (piSrcPos_A.n < iNumDims)) {
		IDL_MessageFromBlock(msg_block, M_TOOLS_BADDIMS, 
			IDL_MSG_LONGJMP,"POSITION");
	}
	if (iHStr && (piSrcStr_A.n < iNumDims)) {
		IDL_MessageFromBlock(msg_block, M_TOOLS_BADDIMS, 
			IDL_MSG_LONGJMP,"STRIDE");
	}
	if (iHSize && (piSize_A.n < iNumDims)) {
		IDL_MessageFromBlock(msg_block, M_TOOLS_BADDIMS, 
			IDL_MSG_LONGJMP,"SIZE");
	}
	/* fill in defaults */
	if (!iHDstPos) for(i=0;i<iNumDims;i++) piDstPos[i] = 0;
	if (!iHDstStr) for(i=0;i<iNumDims;i++) piDstStr[i] = 1;
	if (!iHPos) for(i=0;i<iNumDims;i++) piSrcPos[i] = 0;
	if (!iHStr) for(i=0;i<iNumDims;i++) piSrcStr[i] = 1;
	/* check for zero stride values */
	for(i=0;i<iNumDims;i++) {
		if ((piSrcStr[i] ==0) || (piDstStr[i] == 0)) {
			IDL_MessageFromBlock(msg_block, M_TOOLS_BADDIMS, 
				IDL_MSG_LONGJMP,"stride");
		}
	}
	/* default counts */
	if (!iHSize) for(i=0;i<iNumDims;i++) {
		piSize[i] = (argv[0]->value.arr->dim[i] - piSrcPos[i])/
			piSrcStr[i];
	}

	/* Hard bounds checks */
	iTrunc = 0;
	for(i=0;i<iNumDims;i++) {
		/* clamp the src start position */
		if (piSrcPos[i] < 0) {
			piSrcPos[i] = 0;
			iTrunc = 1;
		}
		if (piSrcPos[i] >= argv[0]->value.arr->dim[i]) {
			piSrcPos[i] = argv[0]->value.arr->dim[i]-1;
			iTrunc = 1;
		}
		/* clamp the src end position */
		j = piSrcPos[i] + (piSize[i]*piSrcStr[i]);
		if (j < 0) {
			if (piSrcStr[i] > 0) {
			    IDL_MessageFromBlock(msg_block, M_TOOLS_BADDIMS, 
				IDL_MSG_LONGJMP,"size");
			}
			while(j < 0) {
				j -= piSrcStr[i];
				piSize[i] -= 1;
			}
			iTrunc = 1;
		}
		if (j > argv[0]->value.arr->dim[i]) {
			if (piSrcStr[i] < 0) {
			    IDL_MessageFromBlock(msg_block, M_TOOLS_BADDIMS, 
				IDL_MSG_LONGJMP,"size");
			}
			while(j > argv[0]->value.arr->dim[i]) {
				j -= piSrcStr[i];
				piSize[i] -= 1;
			}
			iTrunc = 1;
		}
		if (iPaste) {
			/* clamp the dst start position */
			if (piDstPos[i] < 0) {
				piDstPos[i] = 0;
				iTrunc = 1;
			}
			if (piDstPos[i] >= argv[1]->value.arr->dim[i]) {
				piDstPos[i] = argv[1]->value.arr->dim[i]-1;
				iTrunc = 1;
			}
			/* clamp the dst end position */
			j = piDstPos[i] + (piSize[i]*piDstStr[i]);
			if (j < 0) {
			    if (piSrcStr[i] > 0) {
			        IDL_MessageFromBlock(msg_block, M_TOOLS_BADDIMS, 
				    IDL_MSG_LONGJMP,"size");
			    }
			    while(j < 0) {
				j -= piSrcStr[i];
				piSize[i] -= 1;
			    }
			    iTrunc = 1;
			}
			if (j > argv[1]->value.arr->dim[i]) {
			    if (piSrcStr[i] < 0) {
			        IDL_MessageFromBlock(msg_block, M_TOOLS_BADDIMS, 
				    IDL_MSG_LONGJMP,"size");
			    }
			    while(j > argv[1]->value.arr->dim[i]) {
				j -= piSrcStr[i];
				piSize[i] -= 1;
			    }
			    iTrunc = 1;
			}
		}
	}
	/* check for invalid counts */
	/* real or due to clipping */
	for(i=0;i<iNumDims;i++) {
		if (piSize[i] <= 0) {
			IDL_MessageFromBlock(msg_block, M_TOOLS_BADDIMS, 
				IDL_MSG_LONGJMP,"size");
		}
	}
	/* error if /CLIP is not set and truncation occurred */
	if ((!iClip) && (iTrunc)) {
		IDL_MessageFromBlock(msg_block, M_TOOLS_BADRANGE, 
			IDL_MSG_LONGJMP);
	}

	/* allocate a dst variable (if needed) */
	if (!vpTmp) {
		pOut = (char *)IDL_MakeTempArray(argv[0]->type,
				iNumDims,piSize,IDL_BARR_INI_NOP,&vpTmp);
	} else {
		pOut = (char *)argv[1]->value.arr->data;
	}
	pIn = (char *)argv[0]->value.arr->data;

	/* do the work */
	dSize = IDL_TypeSizeFunc(argv[0]->type);

	/* get the first pointer */
	spos = 0;
	dpos = 0;
	sdim[0] = 1;
	ddim[0] = 1;
	for(i=0;i<iNumDims;i++) {
		sdim[i+1] = sdim[i]*argv[0]->value.arr->dim[i];
		ddim[i+1] = ddim[i]*vpTmp->value.arr->dim[i];
		cnt[i] = piSize[i];
		src[i] = piSrcPos[i];
		spos += src[i]*sdim[i];
		dst[i] = piDstPos[i];
		dpos += dst[i]*ddim[i];
	}

	/* copy the values */
	while(1) {
		memcpy(pOut+dSize*dpos,pIn+dSize*spos,dSize);
		i = 0;
		while(i < iNumDims) {
			/* increment */
			spos += piSrcStr[i]*sdim[i];
			src[i] += piSrcStr[i];
			dpos += piDstStr[i]*ddim[i];
			dst[i] += piDstStr[i];
			cnt[i] -= 1;
			if (cnt[i] > 0) break;
			spos -= piSize[i]*sdim[i];
			dpos -= piSize[i]*ddim[i];
			src[i] = piSrcPos[i];
			dst[i] = piDstPos[i];
			cnt[i] = piSize[i];
			i++;
		}
		if (i >= iNumDims) break;
	}

	/* store the result (if needed) */
	if (vpTmp != argv[1]) IDL_VarCopy(vpTmp,argv[1]);
	
	/* cleanup and return */
	IDL_KWCleanup(IDL_KW_CLEAN);

	return;
}
