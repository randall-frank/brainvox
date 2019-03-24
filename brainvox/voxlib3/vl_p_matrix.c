#include "voxel.h"
#include "vl_util.h"
#include "vl_polygon.h"
#include "vl_putil.h"
#include <stdio.h>
#include <math.h>
/*
        Revisions:
		4 4 95 RAC,
		updated to account for normal matrix being passed in and
		new VL_P_VSHADED

        12 May 94 rjf
		Added support for material specularity as Ks(RGB)Kn.
		Fixed normal transforms so that they are not translated.
		Normals are renormalized as needed (Sx!=Sy!=Sz).

*/
/* mat[x][y]    x=0 1 2 3
             y=0: 1 0 0 0
               1: 0 1 0 0
               2: 0 0 1 0
               3: x y z 1
 vec[x y z 1]
*/


void vl_vcopy_f3tod4(VLFLOAT32 *in,VLFLOAT64  *out)
{
	out[0] = in[0];
	out[1] = in[1];
	out[2] = in[2];
	out[3] = 1.0;
}


void vl_matmult4x4(VLFLOAT64 *in,VLFLOAT64 *out,VLFLOAT64 mat[4][4])
{
	out[0] = (in[0]*mat[0][0] + in[1]*mat[0][1] +
		 in[2]*mat[0][2] + in[3]*mat[0][3]);
	out[1] = (in[0]*mat[1][0] + in[1]*mat[1][1] +
		 in[2]*mat[1][2] + in[3]*mat[1][3]);
	out[2] = (in[0]*mat[2][0] + in[1]*mat[2][1] +
		 in[2]*mat[2][2] + in[3]*mat[2][3]);
	out[3] = (in[0]*mat[3][0] + in[1]*mat[3][1] +
		 in[2]*mat[3][2] + in[3]*mat[3][3]);
}

void vl_vset_to_4x4(vset *set,VLFLOAT64 mat[4][4])
{
	VLINT32 i,j;
	for(i=0;i<3;i++) {
		for(j=0;j<3;j++) {
			mat[i][j] = set->xform[i][j];
		}
		mat[i][3] = set->trans[i];
		mat[3][i] = 0.0;
	}
	mat[3][3] = 1.0;
}

void vl_vcopy_3to4(VLFLOAT64 *in,VLFLOAT64 *out)
{
	out[0] = in[0];
	out[1] = in[1];
	out[2] = in[2];
	out[3] = 1.0;
}

void vl_vcopy_4to4(VLFLOAT64 *in,VLFLOAT64 *out)
{
	out[0] = in[0];
	out[1] = in[1];
	out[2] = in[2];
	out[3] = in[3];
}

void vl_vcopy_3to3(VLFLOAT64 *in,VLFLOAT64 *out)
{
	out[0] = in[0];
	out[1] = in[1];
	out[2] = in[2];
}
void vl_vcopy_f3tod3(VLFLOAT32 *in,VLFLOAT64 *out)
{
        out[0] = in[0];
        out[1] = in[1];
        out[2] = in[2];
}

void vl_vcopy_4to3(VLFLOAT64 *in,VLFLOAT64 *out)
{
	out[0] = in[0];
	out[1] = in[1];
	out[2] = in[2];
}

VLFLOAT64 vl_dprod(VLFLOAT64 *v1,VLFLOAT64 *v2)
{
	VLFLOAT64 temp;
	temp = v1[0]*v2[0];
	temp += (v1[1]*v2[1]);
	temp += (v1[2]*v2[2]);
	return(temp);
}

void vl_normalize( VLFLOAT64 V[3] ){
	VLFLOAT64   len = ( V[0] * V[0] ) + ( V[1] * V[1] ) + ( V[2] * V[2] );
	if ( len  >= 0.0 ) {
		len = sqrt( len );
		V[0] = V[0] / len;
		V[1] = V[1] / len;
		V[2] = V[2] / len;
	}
	else {
		V[0] = 0;
		V[1] = 0;
		V[2] = 1;
	}
}

#ifdef _USE_OLD_PREP_POLY
void vl_prep_poly(VLFLOAT64 mat[4][4],VLINT32 n,vl_object *object,
	vl_vertex *verts,vl_int_vertex *poly_p,vl_light *light,
	VLINT32 nlights,
	vl_material *material)
{
	VLINT32 j,i;
	VLFLOAT64	temp[4],H[4],I,NH,LN;
	VLFLOAT64 	nmat[4][4],d;

/* save the transform matrix */
        for(i=0;i<3;i++) {
                for(j=0;j<3;j++) {
                        nmat[i][j] = mat[i][j];
                }
/* preserving the rotations only! */
                nmat[i][3] = 0;
                nmat[3][i] = mat[3][i];
        }
        nmat[3][3] = 1.0;

/* work	 through the objects */
	for(j=0;j<n;j++) {
		vl_vcopy_3to4(verts[object->vindex[j]].loc,poly_p[j].loc);
		vl_vcopy_3to4(verts[object->vindex[j]].norm,temp);
		vl_matmult4x4(poly_p[j].loc,poly_p[j].screen,mat);
/* normals are not translated */
		vl_matmult4x4(temp,poly_p[j].norm,nmat);
		if ((object->type & VL_P_MODEMASK) == VL_P_VCOLORED) {
/* if the type is VL_P_VCOLORED then use vertex normal as the vertex color */
			vl_vcopy_3to3(verts[object->vindex[j]].norm,poly_p[j].color);
			poly_p[j].color[3] = material->alpha;
		} else {
/* H = (L + V) / 2 */
			H[0] = light[object->light].norm[0] / 2.0;
			H[1] = light[object->light].norm[1] / 2.0;
			H[2] = (light[object->light].norm[2] + 1.0)/ 2.0;
			H[3] = 1.0;
/* renormalize the normal in case Sx!=Sy!=Sz when needed */
			d = vl_dprod(poly_p[j].norm,poly_p[j].norm);
			if ((d > 0.0) && (d != 1.0)) {
				d = sqrt(d);
				poly_p[j].norm[0] /= d;
				poly_p[j].norm[1] /= d;
				poly_p[j].norm[2] /= d;
				poly_p[j].norm[3] = 1.0;
			}
			NH = fabs(vl_dprod(poly_p[j].norm,H));
			LN = fabs(vl_dprod(light[object->light].norm,poly_p[j].norm));
/* compute the RGB colors */
			for(i=0;i<3;i++) {
/* lighting I=LaKa(rgb) + Ld(Kd(rgb)(LN) + Ks(NH)^Kn) */
				I = (light[object->light].ambient)*(material->ambient[i]);
				I += (light[object->light].diffuse)*((material->diffuse[i])*LN);
				if (material->specular[i] != 0.0) {
				I += (light[object->light].diffuse)*((material->specular[i])*
					pow(NH,(material->specular[3])));
				}
				if (I < 0.0) I = 0.0;
				if (I > 1.0) I = 1.0;
				poly_p[j].color[i] = I;
			}
			poly_p[j].color[3] = material->alpha;
		}
	}
}

#else

void vl_prep_poly(VLFLOAT64 mat[4][4],
				  VLFLOAT64 NormMat[3][3],
				  VLINT32 n,
				  vl_object *object,
				  vl_vertex  *verts,
				  vl_int_vertex  *poly_p,
				  vl_light  *light,	 
				  VLINT32 nlights ,
				  vl_material  *material){
VLINT32 j,i;
VLFLOAT64	temp[4],H[4],I,NH,LN;
VLFLOAT64 d, ambient;



	if ((object->type & VL_P_MODEMASK) == VL_P_VSHADED ) {
/* if the type is VL_P_VCOLORED then use vertex color and shade */
			
		for(j=0;j<n;j++) {
			vl_vcopy_f3tod4(verts[object->vindex[j]].loc,poly_p[j].loc);
			vl_matmult4x4(poly_p[j].loc,poly_p[j].screen,mat);

			vl_vcopy_f3tod4(verts[object->vindex[j]].norm,temp);
			vl_matmult(temp,poly_p[j].norm,NormMat);
			vl_normalize( poly_p[j].norm );

			d = fabs( vl_dprod( light[object->light].norm, poly_p[j].norm ) );
			d *= light[object->light].diffuse/255.0;
			ambient = light[object->light].ambient/255.0; 
/* compute the RGB colors */
			for(i=0;i<3;i++) {
					I =  ((VLFLOAT64 ) verts[object->vindex[j]].color[i]) * d ;
					I += ((VLFLOAT64 ) verts[object->vindex[j]].color[i]) * ambient;
					if (I < 0.0) I = 0.0;
					if (I > 1.0) I = 1.0;
					poly_p[j].color[i]= I;
			}
			poly_p[j].color[3]= material->alpha;

	  }
  }
  else
  if ((object->type & VL_P_MODEMASK) == VL_P_VCOLORED ) {
/* if the type is VL_P_VCOLORED then use vertex normal as the vertex color */

			for(j=0;j<n;j++) {
				vl_vcopy_f3tod4(verts[object->vindex[j]].loc,poly_p[j].loc);
				vl_matmult4x4(poly_p[j].loc,poly_p[j].screen,mat);
			}
			for(j=0;j<n;j++) {
				poly_p[j].norm[0] = 0;
				poly_p[j].norm[1] = 0;
				poly_p[j].norm[2] = 1;
				poly_p[j].norm[3] = 1;
				poly_p[j].color[0]= verts[object->vindex[j]].norm[0];
				poly_p[j].color[1]= verts[object->vindex[j]].norm[1];
				poly_p[j].color[2]= verts[object->vindex[j]].norm[2];
				poly_p[j].color[3]= material->alpha;
			}
  }
  else
  if (((object->type & VL_P_MODEMASK) == VL_P_SMOOTH ) ||
      ((object->type & VL_P_MODEMASK) == VL_P_TEXTURE_2D ) ||
	  ((object->type & VL_P_MODEMASK) == VL_P_TEXTURE_3D )){

  /* H = (L + V) / 2 */
	H[0] = light[object->light].norm[0] / 2.0;
	H[1] = light[object->light].norm[1] / 2.0;
	H[2] = (light[object->light].norm[2] + 1.0)/ 2.0;
	H[3] = 1.0;
	for(j=0;j<n;j++) {
		vl_vcopy_f3tod4(verts[object->vindex[j]].loc,poly_p[j].loc);
		vl_matmult4x4(poly_p[j].loc,poly_p[j].screen,mat);

		vl_vcopy_f3tod4(verts[object->vindex[j]].norm,temp);
		vl_matmult(temp,poly_p[j].norm,NormMat);
		vl_normalize( poly_p[j].norm );

		NH = fabs(vl_dprod(poly_p[j].norm,H));
		LN = fabs(vl_dprod(light[object->light].norm,poly_p[j].norm));
/* compute the RGB colors */
		for(i=0;i<3;i++) {
/* lighting I=LaKa(rgb) + Ld(Kd(rgb)(LN) + Ks(NH)^Kn) */
				I = (light[object->light].ambient)*(material->ambient[i]);
				I += (light[object->light].diffuse)*((material->diffuse[i])*LN);
				if (material->specular[i] != 0.0) {
				I += (light[object->light].diffuse)*((material->specular[i])*
					pow(NH,(material->specular[3])));
				}
				if (I < 0.0) I = 0.0;
				if (I > 1.0) I = 1.0;
				poly_p[j].color[i] = I;
		}
		poly_p[j].color[3] = material->alpha;
	}
	}
	
    else  {
	/* if ((object->type & VL_P_MODEMASK) == VL_P_FLAT) {
		or any other style
	*/
			for(j=0;j<n;j++) {
				vl_vcopy_f3tod4(verts[object->vindex[j]].loc,poly_p[j].loc);
				vl_matmult4x4(poly_p[j].loc,poly_p[j].screen,mat);
				poly_p[j].norm[0] = 0;
				poly_p[j].norm[1] = 0;
				poly_p[j].norm[2] = 1;
				poly_p[j].norm[3] = 1;
			}
			for(j=0;j<n;j++) {
				poly_p[j].color[0]= material->ambient[0];
				poly_p[j].color[1]= material->ambient[1];
				poly_p[j].color[2]= material->ambient[2];
				poly_p[j].color[3]= material->alpha;
			}
   }


	/* if we are texture mapped the add the texture component */
   if (((object->type & VL_P_MODEMASK) == VL_P_TEXTURE_2D ) ||
		((object->type & VL_P_MODEMASK) == VL_P_TEXTURE_3D )){
/* if the type is VL_P_VCOLORED then use vertex normal as the vertex color */
			for(j=0;j<n;j++) {
				for( i= 0;i<3;i++ ) {
					poly_p[j].loc[i]= verts[object->vindex[j]].texture[i] * ((VLFLOAT64 ) vl_polygon_globals.texture.d[i] - 1.0);
					if (  poly_p[j].loc[i] > vl_polygon_globals.texture.d[i] - 1.0) {
						  poly_p[j].loc[i] = vl_polygon_globals.texture.d[i] - 1.0;
					}
					if (  poly_p[j].loc[i] < 0 )  {
						  poly_p[j].loc[i] = 0;
					}
					
				}
			}
  }
}

#endif

/* Prep Poly w/ mulitple lights
	Light 0 == diffuse
	Light 1->n are the rest  */

void vl_prep_poly_ex(VLFLOAT64 mat[4][4],	   
				  	VLFLOAT64 NormMat[3][3],
				  	VLINT32 n,
				  	vl_object *object,
				  	vl_vertex  *verts,
				  	vl_int_vertex  *poly_p,
				  	vl_light  *light,
				  	VLINT32 nlights,
				  	vl_material  *material){
VLINT32 j,i,k;
VLFLOAT64	temp[4],H[4],I,NH,LN;
VLFLOAT64 d,diffuse;
	
	for(j=0;j<n;j++) { 
			vl_vcopy_f3tod4(verts[object->vindex[j]].loc,poly_p[j].loc);
			vl_matmult4x4(poly_p[j].loc,poly_p[j].screen,mat);
	}

	switch(object->type & VL_P_MODEMASK) {
	case VL_P_VSHADED:
			
		for(j=0;j<n;j++) {
			vl_vcopy_f3tod4(verts[object->vindex[j]].norm,temp);
			vl_matmult(temp,poly_p[j].norm,NormMat);
			vl_normalize( poly_p[j].norm );
			for(i=0;i<3;i++) {
				poly_p[j].color[i]= light[0].color[i]/255.0;
				poly_p[j].color[i] *= ((VLFLOAT64 ) verts[object->vindex[j]].color[i]);
			}

			for( k=1;i<nlights;k++) {
				d = fabs( vl_dprod( light[k].norm, poly_p[j].norm ) );
				for(i=0;i<3;i++) { 
					diffuse = d * light[k].color[i]/255.0;
					I =  ((VLFLOAT64 ) verts[object->vindex[j]].color[i]) * diffuse ;
					
					poly_p[j].color[i] += I;
				}
			}	
			if (poly_p[j].color[i] < 0.0) poly_p[j].color[i] = 0.0;
			if (poly_p[j].color[i] > 1.0) poly_p[j].color[i] = 1.0;
			poly_p[j].color[3]= material->alpha;
	  	}
  break;
  case VL_P_SMOOTH :
  case VL_P_TEXTURE_2D:
  case VL_P_TEXTURE_3D:

	for(j=0;j<n;j++) {

		vl_vcopy_f3tod4(verts[object->vindex[j]].norm,temp);
		vl_matmult(temp,poly_p[j].norm,NormMat);
		vl_normalize( poly_p[j].norm );
		for(i=0;i<3;i++) {
			poly_p[j].color[i] = light[0].color[i] * material->ambient[i];
			poly_p[j].color[i] += material->emissive[i];
		}
	}

	for(k=1;k< nlights;k++) {  
			 /* H = (L + V) / 2 */
		H[0] = light[k].norm[0] / 2.0;
		H[1] = light[k].norm[1] / 2.0;
		H[2] = (light[k].norm[2] + 1.0)/ 2.0;
		H[3] = 1.0;

		for(j=0;j<n;j++){
			NH = fabs(vl_dprod(poly_p[j].norm,H));
			LN = fabs(vl_dprod(light[k].norm,poly_p[j].norm));
/* compute the RGB colors */
			for(i=0;i<3;i++) {
/* lighting I=LaKa(rgb) + Ld(Kd(rgb)(LN) + Ks(NH)^Kn) */
				I = (light[k].color[i])*((material->diffuse[i])*LN);
				if (material->specular[i] != 0.0) {
					I += (light[k].color[i])*((material->specular[i])*
						pow(NH,(material->specular[3])));
				}
				
				poly_p[j].color[i] += I;
			} 
		}
	}
	for( j=0;j<n;j++){
		for(i=0;i<3;i++) {
			if (poly_p[j].color[i] < 0.0) poly_p[j].color[i] = 0.0;
			if (poly_p[j].color[i] > 1.0) poly_p[j].color[i] = 1.0;
		}
		poly_p[j].color[3] = material->alpha;
	}
	break;
  	case VL_P_VCOLORED:
/* if the type is VL_P_VCOLORED then use vertex normal as the vertex color */

			for(j=0;j<n;j++) {
				poly_p[j].norm[0] = 0;
				poly_p[j].norm[1] = 0;
				poly_p[j].norm[2] = 1;
				poly_p[j].norm[3] = 1;
				poly_p[j].color[0]= verts[object->vindex[j]].norm[0];
				poly_p[j].color[1]= verts[object->vindex[j]].norm[1];
				poly_p[j].color[2]= verts[object->vindex[j]].norm[2];
				poly_p[j].color[3]= material->alpha;
			}
  	break;

	default : /*
// flat shading, volume textured, textured....
	 if ((object->type & VL_P_MODEMASK) == VL_P_FLAT) {
		or any other style
	*/
			for(j=0;j<n;j++) {
				poly_p[j].norm[0] = 0;
				poly_p[j].norm[1] = 0;
				poly_p[j].norm[2] = 1;
				poly_p[j].norm[3] = 1;
			}
			for(j=0;j<n;j++) {
				poly_p[j].color[0]= material->ambient[0];
				poly_p[j].color[1]= material->ambient[1];
				poly_p[j].color[2]= material->ambient[2];
				poly_p[j].color[3]= material->alpha;
			}
  	break;
  	}
	


	if (((object->type & VL_P_MODEMASK) == VL_P_TEXTURE_2D ) ||
		((object->type & VL_P_MODEMASK) == VL_P_TEXTURE_3D )){
			for(j=0;j<n;j++) {
				for( i= 0;i<3;i++ ) {
					poly_p[j].loc[i]= verts[object->vindex[j]].texture[i] * ((VLFLOAT64 ) vl_polygon_globals.texture.d[i] - 1.0);
					if (  poly_p[j].loc[i] > vl_polygon_globals.texture.d[i] - 1.0) {
						  poly_p[j].loc[i] = vl_polygon_globals.texture.d[i] - 1.0;
					}
					if (  poly_p[j].loc[i] < 0 )  {
						  poly_p[j].loc[i] = 0;
					}
					
				}
			}
  }
}
