#include "voxel.h"
#include "vl_util.h"
#include "vl_polygon.h"
#include "vl_putil.h"
#include <stdio.h>
#include <math.h>

/* polygon globals */
vl_polygon_params vl_polygon_globals = {0};

void vl_polygon_render_(vset *set,vl_object *objects,vl_vertex *verts,
	vl_light *lights,vl_material *materials,long int clear)
{
	double			mat[4][4];
	long int 		i,dxdy;
	register long int	j;
	vl_int_vertex		poly_p[3];
	vl_text_attrib	texta;
	
	texta.size = 9.0;
	texta.rot = 0.0;
	texta.flip = -1.0;
	texta.center = 0;
	texta.font = 0;
	
/* check the buffers */	
	if ((set->image == 0L) || (set->zbuffer == 0L)) {
		vl_puts("Error: Polygon rendering requires offscreen image and zbuffers");
		return;
	}

/* get the tranformation matrix */	
	vl_vset_to_4x4(set,mat);
	
	dxdy = (set->imagex)*(set->imagey);
/* hard passed clear commands */
	switch(clear) {
		case VL_P_CLEAR :
/* clear zbuffer to infinity and the image buffer to the background color */
			for(j=0;j<dxdy;j++) {
				((unsigned long *)set->image)[j] = (set->backpack);
				set->zbuffer[j] = VL_INFINITY;
			}
			break;
		case VL_P_ZCLEAR :
/* clear zbuffer to infinity */
			for(j=0;j<dxdy;j++) {
				set->zbuffer[j] = VL_INFINITY;
			}
			break;
		case VL_P_ICLEAR :
/* clear the image buffer to the background color */
			for(j=0;j<dxdy;j++) {
				((unsigned long *)set->image)[j] = (set->backpack);
			}
			break;
	}
/* catch a couple of errors (or useful tools */
	if ((objects == 0L) || (verts == 0L)) return;
	i = 0;
/* TOP of the rendering loop (For all objects in the array */
	while ((objects[i].type & VL_P_TYPEMASK) != VL_P_END) {
		switch (objects[i].type & VL_P_TYPEMASK) {
			case VL_P_CLEAR :
/* clear zbuffer to infinity and the image buffer to the background color */
				for(j=0;j<dxdy;j++) {
					((unsigned long *)set->image)[j] = (set->backpack);
					set->zbuffer[j] = VL_INFINITY;
				}
				break;
			case VL_P_ZCLEAR :
/* clear zbuffer to infinity */
				for(j=0;j<dxdy;j++) {
					set->zbuffer[j] = VL_INFINITY;
				}
				break;
			case VL_P_ICLEAR :
/* clear the image buffer to the background color */
				for(j=0;j<dxdy;j++) {
					((unsigned long *)set->image)[j] = (set->backpack);
				}
				break;
			case VL_P_POLY :
/* extract the vertex data and transform */
				vl_prep_poly(mat,3L,&(objects[i]),verts,poly_p,
					&(lights[objects[i].light]),
					&(materials[objects[i].material]));
/* draw the polygon */
				vl_p_rendpoly((objects[i].type & VL_P_MODEMASK),
					(objects[i].type & VL_P_OPTSMASK),
					poly_p,set);
				break;
			case VL_P_LINE :
/* extract the vertex data and transform */
				vl_prep_poly(mat,2L,&(objects[i]),verts,poly_p,
					&(lights[objects[i].light]),
					&(materials[objects[i].material]));
/* draw the line */
				vl_p_rendline((objects[i].type & VL_P_MODEMASK),
					(objects[i].type & VL_P_OPTSMASK),
					&(poly_p[0]),&(poly_p[1]),set);
				break;
			case VL_P_POINT :
/* extract the vertex data and transform */
				vl_prep_poly(mat,1L,&(objects[i]),verts,poly_p,
					&(lights[objects[i].light]),
					&(materials[objects[i].material]));
/* single point for now */
				poly_p[1] = poly_p[0];
				poly_p[1].screen[0] += 1;
/* draw the line */
				vl_p_rendline((objects[i].type & VL_P_MODEMASK),
					(objects[i].type & VL_P_OPTSMASK),
					&(poly_p[0]),&(poly_p[1]),set);
				break;
			case VL_P_MESH :
/* extract the vertex data and transform */
				vl_prep_poly(mat,3L,&(objects[i]),verts,poly_p,
					&(lights[objects[i].light]),
					&(materials[objects[i].material]));
/* draw the lines */
				vl_p_rendline((objects[i].type & VL_P_MODEMASK),
					(objects[i].type & VL_P_OPTSMASK),
					&(poly_p[0]),&(poly_p[1]),set);
				vl_p_rendline((objects[i].type & VL_P_MODEMASK),
					(objects[i].type & VL_P_OPTSMASK),
					&(poly_p[1]),&(poly_p[2]),set);
				vl_p_rendline((objects[i].type & VL_P_MODEMASK),
					(objects[i].type & VL_P_OPTSMASK),
					&(poly_p[2]),&(poly_p[0]),set);
				break;
			case VL_P_TEXT :
/* extract the vertex data and transform */
				vl_prep_poly(mat,1L,&(objects[i]),verts,poly_p,
					&(lights[objects[i].light]),
					&(materials[objects[i].material]));
/* index 0 = x,y,z */
/* index 1 = ptr to string */
				vl_draw_string((char *)(objects[i].vindex[1]),objects[i].type,
					&texta,&(poly_p[0]),set);
				break;
			case VL_P_TATTRIB :
					texta.font = objects[i].material;
					texta.size = objects[i].light;
					if (texta.size < 6) texta.size = 6;
					texta.rot = objects[i].vindex[0];
					texta.center = objects[i].vindex[1];
					if (texta.center > 8) texta.center = 0;
					texta.flip = objects[i].vindex[2];
					if (texta.flip < 0) {
						texta.flip = 1.0;
					} else {
						texta.flip = -1.0;
					}
				break;
			default :
				vl_puts("Error: Invalid object type detected");
				return;
				break;
		}
		if ((i & 0x3f) == 0L) {
			if (vl_rcallback(VL_BACKGROUND,set)) return;
		}
		i = i + 1;  /* next object */
	}	
	return;
}

/* routine to set polygon globals */
void vl_set_polygon_param_(long int param, double *value)
{
	switch(param) {
		case VL_P_SCREENDOOR:
			if (*value == 1.0) {
				vl_polygon_globals.screendoor = 1;
			} else {
				vl_polygon_globals.screendoor = 0;
			}
			break;
	}

	return;
}
