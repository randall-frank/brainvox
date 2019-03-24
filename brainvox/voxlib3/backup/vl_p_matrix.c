#include "voxel.h"
#include "vl_util.h"
#include "vl_polygon.h"
#include "vl_putil.h"
#include <stdio.h>
#include <math.h>
/*
        Revisions:

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

void vl_matmult4x4(double *in,double *out,double mat[4][4])
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

void vl_vset_to_4x4(vset *set,double mat[4][4])
{
	int i,j;
	for(i=0;i<3;i++) {
		for(j=0;j<3;j++) {
			mat[i][j] = set->xform[i][j];
		}
		mat[i][3] = set->trans[i];
		mat[3][i] = 0.0;
	}
	mat[3][3] = 1.0;
}

void vl_vcopy_3to4(double *in,double *out)
{
	out[0] = in[0];
	out[1] = in[1];
	out[2] = in[2];
	out[3] = 1.0;
}

void vl_vcopy_4to4(double *in,double *out)
{
	out[0] = in[0];
	out[1] = in[1];
	out[2] = in[2];
	out[3] = in[3];
}

void vl_vcopy_3to3(double *in,double *out)
{
	out[0] = in[0];
	out[1] = in[1];
	out[2] = in[2];
}

void vl_vcopy_4to3(double *in,double *out)
{
	out[0] = in[0];
	out[1] = in[1];
	out[2] = in[2];
}

double vl_dprod(double *v1,double *v2)
{
	double temp;
	temp = v1[0]*v2[0];
	temp += (v1[1]*v2[1]);
	temp += (v1[2]*v2[2]);
	return(temp);
}

void vl_prep_poly(double mat[4][4],long int n,vl_object *object,
	vl_vertex *verts,vl_int_vertex *poly_p,vl_light *light,
	vl_material *material)
{
	long int j,i;
	double	temp[4],H[4],I,NH,LN;
	double 	nmat[4][4],d;

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
			H[0] = light->norm[0] / 2.0;
			H[1] = light->norm[1] / 2.0;
			H[2] = (light->norm[2] + 1.0)/ 2.0;
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
			LN = fabs(vl_dprod(light->norm,poly_p[j].norm));
/* compute the RGB colors */
			for(i=0;i<3;i++) {
/* lighting I=LaKa(rgb) + Ld(Kd(rgb)(LN) + Ks(NH)^Kn) */
				I = (light->ambient)*(material->ambient[i]);
				I += (light->diffuse)*((material->diffuse[i])*LN);
				if (material->specular[i] != 0.0) {
				I += (light->diffuse)*((material->specular[i])*
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
