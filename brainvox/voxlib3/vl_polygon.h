#ifndef __VL_POLYGON_H__
#define __VL_POLYGON_H__

/*
	Revisions:

	12 May 94	RJF
		Material Specularity is now RGB.
	21 July 94	RJF
		Added VL_P_POINT primitive. 
		
	4  APR 95  RAC
		Updated the vl_vertex structure for RGB info
		changed storage to floats.
		and new primitives
		 
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	VLFLOAT32 loc[3];
	VLFLOAT32 norm[3];
	VLFLOAT32 texture[3];
	VLUCHAR  color[4];  /* R,G,B,A not used */
} vl_vertex;

typedef struct {
	VLFLOAT64	diffuse[3]; /* RGB */
	VLFLOAT64	ambient[3]; /* RGB */
	VLFLOAT64	specular[4]; /* ks(RGB)kn*/
	VLFLOAT64	emissive[3];
	VLFLOAT64	alpha;
} vl_material;

typedef struct {
	VLFLOAT64	norm[3];
	VLFLOAT64	ambient;
	VLFLOAT64	diffuse;
	VLFLOAT64	color[3];
} vl_light;

typedef struct {
	VLINT32	type;
	VLINT32	material;
	VLINT32	light;
	VLINT32	vindex[3];
} vl_object;

typedef struct {
        VLFLOAT64  size;
        VLFLOAT64  rot;
        VLFLOAT64  flip;
        VLINT32        font;
        VLINT32        center;
} vl_text_attrib;

typedef void (user_text_proc)(VLCHAR *,vl_text_attrib *,VLINT32 *,VLINT32 *,VLVOID **);

/* tags:end of list,clear buffer,filled poly,two point line,framed poly */
#define VL_P_END 0x00
#define VL_P_CLEAR 0x01
#define VL_P_ZCLEAR 0x06
#define VL_P_ICLEAR 0x07
#define VL_P_PCLEAR 0x08
#define VL_P_POLY 0x02
#define VL_P_LINE 0x03
#define VL_P_MESH 0x04
#define VL_P_TEXT 0x05
#define VL_P_TATTRIB 0x08
#define VL_P_POINT 0x09
#define VL_P_POLYSTRIP 0x10
#define VL_P_MESHSTRIP 0x11

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
#define VL_P_VSHADED  (0x40L)
#define VL_P_TEXTURE_2D  (0x50L)
#define VL_P_TEXTURE_3D  (0x60L)

#define VL_P_MODEMASK 0x000000f0

/* options */
#define VL_P_NOZWRITE (0x0100)
#define VL_P_NOZCOMP (0x0200)
#define VL_P_PMAP_32 (0x0400)
#define VL_P_PMAP_ZB (0x0800)
#define VL_P_OPTSMASK 0x0000ff00

/* Global polygon rendering parameters */
#define VL_P_SCREENDOOR	1
#define VL_P_PICKING	2
#define VL_P_PICK_ID	3
#define VL_P_PICK_CLEAR_VALUE 4



/* Lighting defines */
#define VL_MAXLIGHTS 10

#define VL_P_AMBIENT  0x0
#define VL_P_DIFFUSE0 0x1
#define VL_P_DIFFUSE1 0x2
#define VL_P_DIFFUSE2 0x3
#define VL_P_DIFFUSE3 0x4
#define VL_P_DIFFUSE4 0x5
#define VL_P_DIFFUSE5 0x6
#define VL_P_DIFFUSE6 0x7
#define VL_P_DIFFUSE7 0x8
#define VL_P_DIFFUSE8 0x9

#define VL_P_LIGHTIDMASK 0x0F

#define VL_P_SINGLELIGHT 0x00		/* use the one defined by the object */
#define VL_P_MULTILIGHTS 0x10		/* light w/ internal lights */
#define VL_P_LIGHTMODEMASK 0xF0



/* Lighting API */
void vl_polygon_light_enable( VLINT32 light,VLINT32 OnOff );
VLINT32 vl_polygon_light_isenabled( VLINT32 light );

void vl_polygon_light_setmode( VLINT32 mode );
VLINT32 vl_polygon_light_getmode();

void vl_polygon_light_setparams(VLINT32 light, vl_light *params );
void vl_polygon_light_getparams(VLINT32 light, vl_light *params );

void vl_polygon_render_(vset *set,vl_object *objects,vl_vertex *verts,
	vl_light *lights,vl_material *materials,VLINT32 clear);

/* texture mapping API */

void vl_polygon_texture2d(VLUCHAR *image,VLINT32 dx,VLINT32 dy );
void vl_polygon_texture3d(VLUCHAR *image,VLINT32 dx,VLINT32 dy, VLINT32 dz);
void vl_polygon_texture_lut(VLUINT32 lut[256]);



/* polygon I/O routines */
void vl_polygon_read_(VLCHAR *filename,vl_object **objects,vl_vertex **verts,
        vl_light **lights,vl_material **materials,VLINT32 *n_objects,
	VLINT32 *n_verts,VLINT32 *n_lights,VLINT32 *n_materials,
	VLINT32 *err);
void vl_polygon_write_(VLCHAR *filename,vl_object *objects,vl_vertex *verts,
        vl_light *lights,vl_material *materials,VLINT32 n_objects,
        VLINT32 n_verts,VLINT32 n_lights,VLINT32 n_materials,
	VLINT32 *err);
void vl_polygon_write_bin_(VLCHAR *filename,vl_object *objects,vl_vertex *verts,
        vl_light *lights,vl_material *materials,VLINT32 n_objects,
        VLINT32 n_verts,VLINT32 n_lights,VLINT32 n_materials,
	VLINT32 *err);
void vl_polygon_free_(vl_object **objects,vl_vertex **verts,vl_light **lights,
        vl_material **materials);

/* Pixmap rendering util */
void vl_render_pixmap_(void *map,VLINT16 *val,VLINT32 dx,VLINT32 dy,
        VLINT32 ox,VLINT32 oy,VLINT32 flag,VLINT32 color,vset *set);

/* user text function */
void vl_set_user_text_func_(user_text_proc *func);

/* renderer parameters */
void vl_set_polygon_param_(VLINT32 param, VLFLOAT64 *value);
void vl_get_polygon_param_(VLINT32 param, VLFLOAT64 *value);
void vl_set_polygon_parami(VLINT32 param, VLINT32 value);
void vl_get_polygon_parami(VLINT32 param, VLINT32 *value);


#ifdef __cplusplus
}
#endif

#endif

