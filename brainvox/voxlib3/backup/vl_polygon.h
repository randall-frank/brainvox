#ifndef __VL_POLYGON_H__
#define __VL_POLYGON_H__

/*
	Revisions:

	12 May 94	RJF
		Material Specularity is now RGB.
	21 July 94	RJF
		Added VL_P_POINT primitive.  
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	double loc[3];
	double norm[3];
} vl_vertex;

typedef struct {
	double	diffuse[3]; /* RGB */
	double	ambient[3]; /* RGB */
	double	specular[4]; /* ks(RGB)kn*/
	double	alpha;
} vl_material;

typedef struct {
	double	norm[3];
	double	ambient;
	double	diffuse;
} vl_light;

typedef struct {
	long int	type;
	long int	material;
	long int	light;
	long int	vindex[3];
} vl_object;

typedef struct {
        double  size;
        double  rot;
        double  flip;
        long int        font;
        long int        center;
} vl_text_attrib;

typedef void (user_text_proc)(char *,vl_text_attrib *,long *,long *,void **);

/* tags:end of list,clear buffer,filled poly,two point line,framed poly */
#define VL_P_END 0x00
#define VL_P_CLEAR 0x01
#define VL_P_ZCLEAR 0x06
#define VL_P_ICLEAR 0x07
#define VL_P_POLY 0x02
#define VL_P_LINE 0x03
#define VL_P_MESH 0x04
#define VL_P_TEXT 0x05
#define VL_P_TATTRIB 0x08
#define VL_P_POINT 0x09
#define VL_P_TYPEMASK 0x0000000f

/* ASCII type encoding XX.YY.oo */
/* two chars:EN,CL,ZC,IC,PO,LI,ME,TE,TA */
/* one char:FL,SM,TE,VC */
/* optional chars:w,z */

/* shading types:flat,gourand,colored by voxel,specific vertex color(A&D) */
#define VL_P_FLAT (0x00)
#define VL_P_SMOOTH (0x10)
#define VL_P_TEXTURED (0x20)
#define VL_P_VCOLORED (0x30)
#define VL_P_MODEMASK 0x000000f0

/* options */
#define VL_P_NOZWRITE (0x0100)
#define VL_P_NOZCOMP (0x0200)
#define VL_P_PMAP_32 (0x0400)
#define VL_P_PMAP_ZB (0x0800)
#define VL_P_OPTSMASK 0x0000ff00

/* Global polygon rendering parameters */
#define VL_P_SCREENDOOR	1

void vl_polygon_render_(vset *set,vl_object *objects,vl_vertex *verts,
	vl_light *lights,vl_material *materials,long int clear);

/* polygon I/O routines */
void vl_polygon_read_(char *filename,vl_object **objects,vl_vertex **verts,
        vl_light **lights,vl_material **materials,long int *n_objects,
	long int *n_verts,long int *n_lights,long int *n_materials,
	long int *err);
void vl_polygon_write_(char *filename,vl_object *objects,vl_vertex *verts,
        vl_light *lights,vl_material *materials,long int n_objects,
        long int n_verts,long int n_lights,long int n_materials,
	long int *err);
void vl_polygon_write_bin_(char *filename,vl_object *objects,vl_vertex *verts,
        vl_light *lights,vl_material *materials,long int n_objects,
        long int n_verts,long int n_lights,long int n_materials,
	long int *err);
void vl_polygon_free_(vl_object **objects,vl_vertex **verts,vl_light **lights,
        vl_material **materials);

/* Pixmap rendering util */
void vl_render_pixmap_(void *map,short *val,long int dx,long int dy,
        long int ox,long int oy,long int flag,long int color,vset *set);

/* user text function */
void vl_set_user_text_func_(user_text_proc *func);

/* renderer parameters */
void vl_set_polygon_param_(long int param, double *value);

#ifdef __cplusplus
}
#endif

#endif

