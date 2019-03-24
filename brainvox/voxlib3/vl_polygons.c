#include "voxel.h"
#include "vl_util.h"
#include "vl_polygon.h"
#include "vl_putil.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


/* Revision history


	4 4 95:
	fixed to properly normalize the matrix
	rac: oct 17 95... added multiple lights;

*/
/* polygon globals */
vl_polygon_params vl_polygon_globals = {0, /* screen door */
			0, /* light mode */
			{0,0,0,0,0,0,0,0,0,0},
			{ {{0,0,1},0,0,{0,0,0}},
			{{0,0,1},0,0,{0,0,0}},
			{{0,0,1},0,0,{0,0,0}},
			{{0,0,1},0,0,{0,0,0}},
			{{0,0,1},0,0,{0,0,0}},
			{{0,0,1},0,0,{0,0,0}},
			{{0,0,1},0,0,{0,0,0}},
			{{0,0,1},0,0,{0,0,0}},
			{{0,0,1},0,0,{0,0,0}},
			{{0,0,1},0,0,{0,0,0}} },
			{NULL,{0,0,0},{
			0,1,2,3,4,5,6,7,8,9,
			10,1,2,3,4,5,6,7,8,9,
			20,1,2,3,4,5,6,7,8,9,
			30,1,2,3,4,5,6,7,8,9,
			40,1,2,3,4,5,6,7,8,9,
			50,1,2,3,4,5,6,7,8,9,
			60,1,2,3,4,5,6,7,8,9,
			70,1,2,3,4,5,6,7,8,9,
			80,1,2,3,4,5,6,7,8,9,
			90,1,2,3,4,5,6,7,8,9,
			100,1,2,3,4,5,6,7,8,9,
			110,1,2,3,4,5,6,7,8,9,
			120,1,2,3,4,5,6,7,8,9,
			130,1,2,3,4,5,6,7,8,9,
			140,1,2,3,4,5,6,7,8,9,
			150,1,2,3,4,5} },
			FALSE,0, 0,
			NULL};


typedef void ( *preppoly_proc )(VLFLOAT64 mat[4][4], 
							    VLFLOAT64 NormMat[3][3], 
							    VLINT32 n,
							    vl_object *object,
								vl_vertex *verts,
								vl_int_vertex *poly_p,
								vl_light *light,
								VLINT32 nlights,
								vl_material *material);

void vl_polygon_render_(vset *set,vl_object *objects,vl_vertex *verts,
	vl_light *lights,vl_material *materials,VLINT32 clear)
{
	VLFLOAT64			mat[4][4];
	VLFLOAT64 NormMat[3][3];
	VLFLOAT64 tempMat[3][3];
	VLINT32 		i,dxdy;
	register VLINT32	j;
	vl_int_vertex		poly_p[3];
	vl_text_attrib	texta;
	vl_light clights[VL_MAXLIGHTS];
	preppoly_proc  prep_poly = &vl_prep_poly;
	int nlights = 1;
	
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

	if ( vl_polygon_globals.enable_picking  &&
		 !set->pickbuffer ) {
	 	vl_puts("Error: Polygon picking requires offscreen picking and zbuffers");
		return;
	}


	if ( vl_polygon_globals.light_mode == VL_P_MULTILIGHTS ) {

    /* prepare the light array */
		memset( clights,0,sizeof(vl_light) * VL_MAXLIGHTS);
	 	clights[0] = vl_polygon_globals.lights[VL_P_AMBIENT];
		
		if ( !vl_polygon_globals.light_enabled[VL_P_AMBIENT]) {
	 		clights[0].color[0] = 0;
			clights[0].color[1] = 0;
			clights[0].color[2] = 0;
		}				 
	
		for(i=VL_P_DIFFUSE0;i < VL_MAXLIGHTS;i++){
	   		if ( vl_polygon_globals.light_enabled[i]) {
	 			clights[nlights] = vl_polygon_globals.lights[i];
				nlights++;
	   		}
		}
		lights = clights;
		prep_poly = &vl_prep_poly_ex;
	}

	/* get the tranformation matrix */
	vl_vset_to_4x4(set,mat);
/* Get the Normal Matrix */

	for(i=0;i<3;i++){
		for(j=0;j<3;j++){
			tempMat[i][j] = mat[i][j];
		}
	}

	/* Invert it */
	invert3x3(tempMat );
	/* Transpose it */
	for(i=0;i<3;i++){
		for(j=0;j<3;j++){
			 NormMat[i][j] = tempMat[j][i];
		}
	}

	
	dxdy = (set->imagex)*(set->imagey);
/* hard passed clear commands */
	switch(clear) {
		case VL_P_CLEAR :
/* clear zbuffer to infinity and the image buffer to the background color */
			for(j=0;j<dxdy;j++) {
				((VLUINT32 *)set->image)[j] = (set->backpack);
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
				((VLUINT32 *)set->image)[j] = (set->backpack);
			}
			break;
		case VL_P_PCLEAR :
			for(j=0;j<dxdy;j++) {
				set->pickbuffer[j] = vl_polygon_globals.pick_clear_value;
			}
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
					((VLUINT32 *)set->image)[j] = (set->backpack);
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
					((VLUINT32 *)set->image)[j] = (set->backpack);
				}
				break;
			case VL_P_POLY :
/* extract the vertex data and transform */
				prep_poly(mat, NormMat, 3L,&(objects[i]),verts,poly_p,
					lights,	nlights ,
					&(materials[objects[i].material]));
/* draw the polygon */
				vl_p_rendpoly((objects[i].type & VL_P_MODEMASK),
					(objects[i].type & VL_P_OPTSMASK),
					poly_p,set);
				if ( vl_polygon_globals.enable_picking ) {
					vl_p_rendpickbuffer((objects[i].type & VL_P_MODEMASK), 
									    (objects[i].type & VL_P_OPTSMASK),
										 poly_p,set);
				}
				break;
			case VL_P_LINE :
/* extract the vertex data and transform */
				prep_poly(mat, NormMat,2L,&(objects[i]),verts,poly_p,
						lights,	nlights ,
					&(materials[objects[i].material]));
/* draw the line */
				vl_p_rendline((objects[i].type & VL_P_MODEMASK),
					(objects[i].type & VL_P_OPTSMASK),
					&(poly_p[0]),&(poly_p[1]),set);
				if ( vl_polygon_globals.enable_picking ) {
					vl_p_rendpickline((objects[i].type & VL_P_MODEMASK),
									  (objects[i].type & VL_P_OPTSMASK),
									  &(poly_p[0]),&(poly_p[1]),set);
				}
				break;
			case VL_P_POINT :
/* extract the vertex data and transform */
				prep_poly(mat,NormMat,1L,&(objects[i]),verts,poly_p,
					lights,	nlights ,
					&(materials[objects[i].material]));
/* single point for now */
				poly_p[1] = poly_p[0];
				poly_p[1].screen[0] += 1;
/* draw the line */
				vl_p_rendline((objects[i].type & VL_P_MODEMASK),
					(objects[i].type & VL_P_OPTSMASK),
					&(poly_p[0]),&(poly_p[1]),set);
				if ( vl_polygon_globals.enable_picking ) {
					vl_p_rendpickline((objects[i].type & VL_P_MODEMASK),
									  (objects[i].type & VL_P_OPTSMASK),
									  &(poly_p[0]),&(poly_p[1]),set);
				}
				break;
			case VL_P_MESH :
/* extract the vertex data and transform */
				prep_poly(mat,NormMat,3L,&(objects[i]),verts,poly_p,
					lights,	nlights ,
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
				if ( vl_polygon_globals.enable_picking ) {
						vl_p_rendpickline((objects[i].type & VL_P_MODEMASK),
										  (objects[i].type & VL_P_OPTSMASK),
										  &(poly_p[0]),&(poly_p[1]),set);
						vl_p_rendpickline((objects[i].type & VL_P_MODEMASK),
										  (objects[i].type & VL_P_OPTSMASK),
									      &(poly_p[1]),&(poly_p[2]),set);
						vl_p_rendpickline((objects[i].type & VL_P_MODEMASK),
										  (objects[i].type & VL_P_OPTSMASK),
										  &(poly_p[2]),&(poly_p[0]),set);
				}
				break;
			case VL_P_TEXT :
/* extract the vertex data and transform */
				prep_poly(mat,NormMat,1L,&(objects[i]),verts,poly_p,
					lights,	nlights ,
					&(materials[objects[i].material]));
/* index 0 = x,y,z */
/* index 1 = ptr to string */
				vl_draw_string((VLCHAR *)(objects[i].vindex[1]),objects[i].type,
					&texta,&(poly_p[0]),set);
				if ( vl_polygon_globals.enable_picking ) {
						vl_pick_string((VLCHAR *)(objects[i].vindex[1]),objects[i].type,
										&texta,&(poly_p[0]),set);
				}
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
			if (vl_rcallback(VL_BACKGROUND,1,set)) return;
		}
		i = i + 1;  /* next object */
	}	
	return;
}

/* routine to set polygon globals */
void vl_set_polygon_param_(VLINT32 param, VLFLOAT64 *value)
{
	switch(param) {
		case VL_P_SCREENDOOR:
			if (*value == 1.0) {
				vl_polygon_globals.screendoor = 1;
			} else {
				vl_polygon_globals.screendoor = 0;
			}
			break;
		case VL_P_PICKING:
			vl_polygon_globals.enable_picking = *value;
		break;
		case VL_P_PICK_ID:
			vl_polygon_globals.object_id = *value;
		break;
		case VL_P_PICK_CLEAR_VALUE:
			vl_polygon_globals.pick_clear_value = *value;
		break;
	}

	return;
}
void vl_get_polygon_param_(VLINT32 param, VLFLOAT64 *value){
	switch(param) {
		case VL_P_SCREENDOOR:
			*value = vl_polygon_globals.screendoor;
			break;
		case VL_P_PICKING:
			*value = vl_polygon_globals.enable_picking;
		break;
		case VL_P_PICK_ID:
			*value = vl_polygon_globals.object_id;
		break;
		case VL_P_PICK_CLEAR_VALUE:
			*value = vl_polygon_globals.pick_clear_value;
		break;
	}
	return;
}
void vl_set_polygon_parami(VLINT32 param, VLINT32 value){
	switch(param) {
		case VL_P_SCREENDOOR:
			if (value == 1.0) {
				vl_polygon_globals.screendoor = 1;
			} else {
				vl_polygon_globals.screendoor = 0;
			}
			break;
		case VL_P_PICKING:
			vl_polygon_globals.enable_picking = value;
		break;
		case VL_P_PICK_ID:
			vl_polygon_globals.object_id = value;
		break;
		case VL_P_PICK_CLEAR_VALUE:
			vl_polygon_globals.pick_clear_value = value;
		break;
			
	}
	return;
}

void vl_get_polygon_parami(VLINT32 param, VLINT32 *value){

	switch(param) {
		case VL_P_SCREENDOOR:
			*value = vl_polygon_globals.screendoor;
			break;
		case VL_P_PICKING:
			*value = vl_polygon_globals.enable_picking;
		break;
		case VL_P_PICK_ID:
			*value = vl_polygon_globals.object_id;
		break;
		case VL_P_PICK_CLEAR_VALUE:
			*value = vl_polygon_globals.pick_clear_value;
		break;
			
			
	}
	return;
}

void vl_polygon_light_enable( VLINT32 light, VLINT32 OnOff ){

 	if (( light>=0 )  &&
		(light < VL_MAXLIGHTS)) {
		vl_polygon_globals.light_enabled[light] = OnOff;
	}
	else vl_puts("invalid light index");

}

VLINT32 vl_polygon_light_isenabled( VLINT32 light ){
	
	if (( light>=0 )  &&
		(light < VL_MAXLIGHTS)) {
		return vl_polygon_globals.light_enabled[light];
	}
	else vl_puts("invalid light index");
	return(0);
}

void vl_polygon_light_setmode( VLINT32 mode ) {
 	
	switch( mode & VL_P_LIGHTMODEMASK ) {
	case VL_P_SINGLELIGHT:
	case VL_P_MULTILIGHTS:
		 vl_polygon_globals.light_mode = mode;
	break;
	default:
	vl_puts("unknown lighting mode");
	break;
	}
}

VLINT32 vl_polygon_light_getmode(){
	return 	vl_polygon_globals.light_mode;
}

void vl_polygon_light_setparams(VLINT32 light, vl_light *params ){
	 if (( light>=0 )  &&
		(light < VL_MAXLIGHTS)) {
		memcpy(&vl_polygon_globals.lights[light] ,params, sizeof(vl_light));
		vl_normalize( vl_polygon_globals.lights[light].norm );
	}
	else vl_puts("invalid light index");
}

void vl_polygon_light_getparams(VLINT32 light, vl_light *params ){
	if (( light>=0 )  &&
		(light < VL_MAXLIGHTS)) {
		memcpy(params, &vl_polygon_globals.lights[light], sizeof(vl_light));
	}
	else vl_puts("invalid light index");
}



void vl_polygon_texture2d(VLUCHAR *image,VLINT32 dx,VLINT32 dy ){
	 vl_polygon_globals.texture.texture_ptr = image;
	 vl_polygon_globals.texture.d[0] = dx;
	 vl_polygon_globals.texture.d[1] = dy;
	 vl_polygon_globals.texture.d[2] = 0;

}
void vl_polygon_texture3d(VLUCHAR *image,VLINT32 dx,VLINT32 dy, VLINT32 dz){
	 vl_polygon_texture2d(image,dx,dy);
	 	 vl_polygon_globals.texture.d[2] = dz;

}
void vl_polygon_texture_lut(VLUINT32 lut[256]){
int i;

	for(i=0;i<256;i++) 	 
		vl_polygon_globals.texture.lut[i] = lut[i];

}
