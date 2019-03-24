#ifndef __VL_PUTIL_H__
#define __VL_PUTIL_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	double loc[4];
	double norm[4];
	double screen[4];
	double color[4];
} vl_int_vertex;

typedef struct {
	long int	screendoor;
} vl_polygon_params;

typedef struct {
        unsigned char   v[4][4];
} MaskRec;

extern vl_polygon_params vl_polygon_globals;
extern MaskRec vl_p_screendoor[11];

/* utilities */
void vl_matmult4x4(double *in,double *out,double mat[4][4]);
void vl_vset_to_4x4(vset *set,double mat[4][4]);
void vl_vcopy_3to4(double *in,double *out);
void vl_vcopy_4to4(double *in,double *out);
void vl_vcopy_3to3(double *in,double *out);
void vl_vcopy_4to3(double *in,double *out);
double vl_dprod(double *v1,double *v2);
void vl_prep_poly(double mat[4][4],long int n,vl_object *object,
vl_vertex *verts,vl_int_vertex *poly_p,vl_light *light,vl_material *material);

/* rendering */
void vl_p_rendpoly(long int style,long int opts,vl_int_vertex *v,vset *set);
void vl_p_rendline(long int style,long int opts,vl_int_vertex *v1,
	vl_int_vertex *v2,vset *set);
void vl_draw_string(char *s,long int type,vl_text_attrib *a,vl_int_vertex *pnt,vset *set);

void vl_incrementalize(long int *acc,long int *inc,long int n,double dist);
void vl_increment(long int *acc,long int *inc,long int n);


#ifdef __cplusplus
}
#endif

#endif
