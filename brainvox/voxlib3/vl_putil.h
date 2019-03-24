#ifndef __VL_PUTIL_H__
#define __VL_PUTIL_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	VLFLOAT64 loc[4];
	VLFLOAT64 norm[4];
	VLFLOAT64 screen[4];
	VLFLOAT64 color[4];
} vl_int_vertex;

typedef struct {
	VLUCHAR *texture_ptr;
	VLINT32 d[3];
	VLUINT32 lut[256];
} vl_texture;

typedef struct {
	VLINT32	screendoor;
	VLINT32 light_mode;
	VLINT32 light_enabled[VL_MAXLIGHTS];
	vl_light lights[VL_MAXLIGHTS];
	vl_texture texture;
	VLINT32 enable_picking;
	VLINT32 object_id;
	VLINT32 pick_clear_value;
	user_text_proc *  text_proc;
} vl_polygon_params;

typedef struct {
        VLUCHAR   v[4][4];
} MaskRec;

extern vl_polygon_params vl_polygon_globals;
extern MaskRec vl_p_screendoor[11];

/* utilities */
void vl_matmult4x4(VLFLOAT64 *in,VLFLOAT64 *out,VLFLOAT64 mat[4][4]);
void vl_vset_to_4x4(vset *set,VLFLOAT64 mat[4][4]);
void vl_vcopy_3to4(VLFLOAT64 *in,VLFLOAT64 *out);
void vl_vcopy_4to4(VLFLOAT64 *in,VLFLOAT64 *out);
void vl_vcopy_3to3(VLFLOAT64 *in,VLFLOAT64 *out);
void vl_vcopy_4to3(VLFLOAT64 *in,VLFLOAT64 *out);

void vl_vcopy_f3tod4(VLFLOAT32 *in,VLFLOAT64  *out);
void vl_vcopy_f3tod4(VLFLOAT32 *in,VLFLOAT64  *out);

VLFLOAT64 vl_dprod(VLFLOAT64 *v1,VLFLOAT64 *v2);
void vl_normalize( VLFLOAT64 V[3] );

void vl_prep_poly(VLFLOAT64 mat[4][4], VLFLOAT64 NormMat[3][3], VLINT32 n,vl_object *object,
vl_vertex *verts,vl_int_vertex *poly_p,vl_light *light,VLINT32 nlights, vl_material *material);

void vl_prep_poly_ex(VLFLOAT64 mat[4][4], VLFLOAT64 NormMat[3][3], VLINT32 n,vl_object *object,
vl_vertex *verts,vl_int_vertex *poly_p,vl_light *light,VLINT32 nlights, vl_material *material);

/* rendering */
void vl_p_rendpoly(VLINT32 style,VLINT32 opts,vl_int_vertex *v,vset *set);
void vl_p_rendline(VLINT32 style,VLINT32 opts,vl_int_vertex *v1,
	vl_int_vertex *v2,vset *set);
void vl_draw_string(VLCHAR *s,VLINT32 type,vl_text_attrib *a,vl_int_vertex *pnt,vset *set);

void vl_incrementalize(VLINT32 *acc,VLINT32 *inc,VLINT32 n,VLFLOAT64 dist);
void vl_increment(VLINT32 *acc,VLINT32 *inc,VLINT32 n);
/* picking */
void vl_p_rendpickbuffer(VLINT32 style,VLINT32 opts,vl_int_vertex *vec,vset *set);
void vl_p_rendpickline(VLINT32 style,VLINT32 opts,vl_int_vertex *vi1, vl_int_vertex *vi2,vset *set);
void vl_pick_pixmap_(void *map,VLINT16 *val,VLINT32 dx,VLINT32 dy,
	VLINT32 ox,VLINT32 oy,VLINT32 flag,VLINT32 color,vset *set);
void vl_pick_string(VLCHAR *s,VLINT32 type,vl_text_attrib *a,vl_int_vertex *pnt,vset *set);

typedef void (* vl_linefunc )(VLINT32 style,
							VLINT32 opts,
							vl_int_vertex *vi1, 
							vl_int_vertex *vi2,
							vset *set);

void vl_pick_pixmap_(void *map,
					 VLINT16 *val,
					 VLINT32 dx,
					 VLINT32 dy,
					 VLINT32 ox,
					 VLINT32 oy,
					 VLINT32 flag,
					 VLINT32 color,
					 vset *set);

void draw_string_vector(VLCHAR *s,
						VLINT32 type,
						vl_text_attrib *a,
						vl_int_vertex *pnt,
						vset *set,
						vl_linefunc drawfunc);


void vl_set_screen_mask(VLUINT32 *A,MaskRec **mask);

#ifdef __cplusplus
}
#endif

#endif
