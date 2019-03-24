#include "voxel.h"
#include "vl_util.h"
#include "autotrace.h"
#include <math.h>
#include <stdio.h>

#ifndef CLAMP
#define CLAMP(v,l,h) ((v)<(l) ? (l) : (v) > (h) ? (h) : (v))
#endif

void do_flood_fill_(vset *set,long int *verts,long int nseeds,
	long int low,long int high,long int chan);

void    polyfill_c_(int *trace,int count,int *list,int *ptr);

static void do_oper(long int x,long int y,long int z,unsigned char *data,
	long int oper,long int *d);

/*
   this routine takes a list of points in space and treats them as
   a polygon.  The polygon is projected through the voxel volume and
   an operation is performed on the voxels within the polygon
*/

/* verts are in "voxel" space (ie squeezed and origin at 0,0,0) */
/* chan = channel to operate on */
/* oper = 0-255 = fill clip with 0-255 */
/* oper = -1 = clear clip bit 7 */
/* oper = -2 = set clip bit 7 */
/* oper = -3 = fill volume (to nverts) */
/* oper = -4 = clear volume bit 7 */
/* oper = -5 = set volume bit 7 */
/* oper = -6 = mult vol(chan) by (nverts/256) */ 
/* oper = -7 = mult vol0 by vol(chan) */
/* oper = -8 = swap vol0 and vol(chan)  */
/* oper = -9 = add vol(chan) and (nverts/256)  */
/* oper = -10 = add vol0 and vol(chan)  */
/* oper = -11 = copy vol(chan) to vol0 */
/* oper = -12 = copy vol0 to vol(chan)  */
/* verts[0-2] = point1-x,y,z (see measurements.f) */

#define MAX_PNTS 8192
#define STRIP_SIZE 10000

void fill_volume_(vset *set,long int chan,long int oper,
	double	*verts,long int nverts)
{
	double 		temp[3],xform[3][3];
	double		lv[3],n[3],B,t,x,y,z;
	long int	i,j,plane,pl,st,en,li;
	int		trace[MAX_PNTS],list[STRIP_SIZE],ptr,count;
	unsigned char	tmp;
	long int	v;

	if ((chan > VL_MAX_VOLS) || (chan < 0)) return;
	if (set->aux_data[chan] == 0) return;

/* fill volume to value */
	if (oper == -3) {
		for(i=0;i<(set->d[0]*set->d[1]*set->d[2]);i++) 
			set->aux_data[chan][i] = nverts;
		return;
	} 
/* clear volume bit 7 */
	else if (oper == -4) {
		for(i=0;i<(set->d[0]*set->d[1]*set->d[2]);i++) 
			set->aux_data[chan][i] &= 0x7f;
		return;
	}
/* set volume bit 7 */
	else if (oper == -5) {
		for(i=0;i<(set->d[0]*set->d[1]*set->d[2]);i++) 
			set->aux_data[chan][i] |= 0x80;
		return;
	}
/* mult volume (chan) by nverts/256 */
	else if (oper == -6) {
		t = (double)(nverts)/256.0;
		for(i=0;i<(set->d[0]*set->d[1]*set->d[2]);i++)  {
			v = set->aux_data[chan][i] * t;
			set->aux_data[chan][i] = CLAMP(v,0,255);
		}
		return;
	}
/* mult volume 0 by chan (store in chan) */
	else if (oper == -7) {
		for(i=0;i<(set->d[0]*set->d[1]*set->d[2]);i++) {
			v=(set->aux_data[chan][i] * set->aux_data[0][i])/256;
			set->aux_data[chan][i] = CLAMP(v,0,255);
		}
		return;
/* swap volume data */
	}
	else if (oper == -8) {
		for(i=0;i<(set->d[0]*set->d[1]*set->d[2]);i++) {
			tmp = set->aux_data[chan][i];
			set->aux_data[chan][i] = set->aux_data[0][i];
			set->aux_data[0][i] = tmp;
		}
		return;
	}
/* oper = -9 = add vol(chan) and (nverts/256)  */
	else if (oper == -9) {
		t = (double)(nverts)/256.0;
		for(i=0;i<(set->d[0]*set->d[1]*set->d[2]);i++) {
			v = set->aux_data[chan][i] + t;
			set->aux_data[chan][i] = CLAMP(v,0,255);
		}
		return;
	}
/* oper = -10 = add vol0 and vol(chan)  */
	else if (oper == -10) {
		for(i=0;i<(set->d[0]*set->d[1]*set->d[2]);i++) {
			v = set->aux_data[chan][i] + set->aux_data[0][i];
			set->aux_data[chan][i] = CLAMP(v,0,255);
		}
		return;
	}
/* oper = -11 = copy vol(chan) to vol0 */
	else if (oper == -11) {
		for(i=0;i<(set->d[0]*set->d[1]*set->d[2]);i++) 
			set->aux_data[0][i] = set->aux_data[chan][i];
		return;
	}
/* oper = -12 = copy vol0 to vol(chan)  */
	else if (oper == -12) {
		for(i=0;i<(set->d[0]*set->d[1]*set->d[2]);i++) 
			set->aux_data[chan][i] = set->aux_data[0][i];
		return;
	}

/* get the inverse matrix */
	for(i=0;i<3;i++) {
		for(j=0;j<3;j++) {
			xform[i][j] = set->xform[i][j];
		}
	}
	invert3x3(xform);

/* compute the projection vector (lv) */
	temp[0] = 0; temp[1] = 0; temp[2] = 0;
	vl_matmult(temp,n,xform);
	temp[0] = 0; temp[1] = 0; temp[2] = 1;
	vl_matmult(temp,lv,xform);
	lv[0] = lv[0] - n[0];
	lv[1] = lv[1] - n[1];
	lv[2] = lv[2] - n[2];

/* line formula p' = P + t*lv */

/* compute the best plane to work on (most normal to projection) */
	if ((fabs(lv[0])>= fabs(lv[1]))&& (fabs(lv[0])>= fabs(lv[2]))){
		plane = 0; /* X */
		n[0] = 1; n[1] = 0; n[2] = 0;
	} else if ((fabs(lv[1])>= fabs(lv[0]))&& (fabs(lv[1])>= fabs(lv[2]))){
		plane = 1; /* Y */
		n[0] = 0; n[1] = 1; n[2] = 0;
	} else {
		plane = 2; /* Z */
		n[0] = 0; n[1] = 0; n[2] = 1;
	}

#ifdef DEBUG
	printf("plane=%ld N=%lf,%lf,%lf\n",plane,n[0],n[1],n[2]);
	printf("lv=%lf,%lf,%lf\n",lv[0],lv[1],lv[2]);
#endif

/* over all slice planes */
	for(pl=0;pl<set->d[plane];pl++) {

/* compute the plane equation Nx+Ny+Nz = B */
		temp[0] = 0; temp[1] = 0; temp[2] = 0;
		switch(plane) {
			case 0:
				temp[0] = pl - (0.5*set->d[0]);
				break;
			case 1:
				temp[1] = pl - (0.5*set->d[1]);
				break;
			case 2:
				temp[2] = (pl - (0.5*set->d[2]))*
					(set->squeeze_factor);
				break;
		}
		B = n[0]*temp[0] + n[1]*temp[1] + n[2]*temp[2];
#ifdef DEBUG
	printf("B=%lf N=%lf,%lf,%lf temp=%lf,%lf,%lf\n",B,n[0],n[1],n[2],
		temp[0],temp[1],temp[2]);
#endif

/* intersect the lines with the plane */
		count = 0;
		for(i=0;i<nverts;i++) {
			t = n[0]*lv[0] + n[1]*lv[1] + n[2]*lv[2];
			if (t == 0) return;
			t = (B - (n[0]*verts[i*3] + n[1]*verts[(i*3)+1] + 
				n[2]*verts[(i*3)+2]))/t;

/* get the point location in volume centered cubic space */
			x = verts[(i*3)+0] + t*lv[0];
			y = verts[(i*3)+1] + t*lv[1];
			z = verts[(i*3)+2] + t*lv[2];

/* convert to voxel index space */
			x = x + (0.5*set->d[0]);
			y = y + (0.5*set->d[1]);
			z = z / (set->squeeze_factor);  /* adjust for Z */
			z = z + (0.5*set->d[2]);

/* store off the planar form */
			switch(plane) {
				case 0:
					list[count++] = y;
					list[count++] = z;
					break;
				case 1:
					list[count++] = x;
					list[count++] = z;
					break;
				case 2:
					list[count++] = x;
					list[count++] = y;
					break;
			}
		}
/* do the operation ... */
		ptr = STRIP_SIZE;
		polyfill_c_(list,count,list,&ptr);
		if (ptr > 0) {
/* for each strip */
        	for(i=0;i<ptr;i=i+3) {
			st = list[i]; en = list[i+1];
			li = list[i+2];
			switch(plane) {
				case 0: /*y,z*/
					if (li < 0) break;
					if (li >= set->d[2]) break;	
					if (st < 0) st = 0;
					if (en >= set->d[1]) en = set->d[1]-1;
					for(j=st;j<=en;j++) do_oper(pl,j,li,
					       set->aux_data[chan],oper,set->d);
					break;
				case 1: /*x,z*/
					if (li < 0) break;
					if (li >= set->d[2]) break;
					if (st < 0) st = 0;
					if (en >= set->d[0]) en = set->d[0]-1;
					for(j=st;j<=en;j++) do_oper(j,pl,li,
					       set->aux_data[chan],oper,set->d);
					break;
				case 2: /*x,y*/
					if (li < 0) break;
					if (li >= set->d[1]) break;
					if (st < 0) st = 0;
					if (en >= set->d[0]) en = set->d[0]-1;
					for(j=st;j<=en;j++) do_oper(j,li,pl,
					       set->aux_data[chan],oper,set->d);
					break;
			}
		}
		}

	}

	return;
}
void FILL_VOLUME(vset *set,long int chan,long int oper,
	double	*verts,long int nverts)
{
fill_volume_(set,chan,oper,verts,nverts);
}

/* oper = 0-255 = fill with 0-255 */
/* oper = -1 = clear bit 7 */
/* oper = -2 = set bit 7 */
static void do_oper(long int x,long int y,long int z,unsigned char *data,
	long int oper,long int *d)
{
	long int	i;

	i = x + (y*d[0]) + (z*d[0]*d[1]);
	
	if (oper >= 0) {
		data[i] = oper;
	} else if (oper == -1) {
		data[i] &= 0x7f;
	} else if (oper == -2) {
		data[i] |= 0x80;
	}
	
	return;
}

/* routine to "glue" to the flood fill algo */

void do_flood_fill_(vset *set,long int *verts,long int nseeds,
	long int low,long int high,long int chan)
{
	Dvol		vol;
	long int	i,count;
	Point3D		seeds[MAX_PNTS];

/* get the volume */
	vol.data = set->aux_data[0];
	vol.data2 = set->aux_data[chan];
	for(i=0;i<3;i++) {
		vol.d[i] = set->d[i];
		vol.start[i] = set->start[i];
		vol.end[i] = set->end[i];
	}
/* get the seeds */
	for(i=0;i<nseeds;i++) {
		seeds[i].x = verts[(i*3)+0];
		seeds[i].y = verts[(i*3)+1];
		seeds[i].z = verts[(i*3)+2];
		seeds[i].col = 0;
	}

	volume_autotrace(&vol,low,high,seeds,nseeds,&count);

	return;
}
void DO_FLOOD_FILL(vset *set,long int *verts,long int nseeds,
	long int low,long int high,long int chan)
{
do_flood_fill_(set,verts,nseeds,low,high,chan);
}
