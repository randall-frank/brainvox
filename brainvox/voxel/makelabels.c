#include "gl.h"
#include <stdio.h>
#include "voxel.h"
#include "vl_util.h"
#include <math.h>

#ifndef M_PI_4
#define M_PI_4 0.78539816339744830962
#endif

typedef struct {
        double x[3],y[3],p[3],n[3];
        vset *theset;
        long int imdx,imdy;
} sample_common;

extern sample_common held_data;

void draw_labeled_plane(long int *points,char *thechar,long int *offset,
	long int zoom);

void vl_draw_labels_cutplane_(long int *offset,long int zoom,long int INS)
{
	long int	points[12];
	long int	i,k;

	for(i=0;i<6;i++) {
		k = 0;
		switch(i) {
			case 0:
				points[k++] = held_data.theset->d[0] - INS;
				points[k++] = held_data.theset->d[1];
				points[k++] = held_data.theset->d[2];

				points[k++] = held_data.theset->d[0] - INS;
				points[k++] = 0;
				points[k++] = held_data.theset->d[2];

				points[k++] = held_data.theset->d[0] - INS;
				points[k++] = 0;
				points[k++] = 0;

				points[k++] = held_data.theset->d[0] - INS;
				points[k++] = held_data.theset->d[1];
				points[k++] = 0;

				draw_labeled_plane(points,"L",offset,zoom);
				break;
			case 1:
				points[k++] = 0 + INS;
				points[k++] = held_data.theset->d[1];
				points[k++] = held_data.theset->d[2];

				points[k++] = 0 + INS;
				points[k++] = 0;
				points[k++] = held_data.theset->d[2];

				points[k++] = 0 + INS;
				points[k++] = 0;
				points[k++] = 0;

				points[k++] = 0 + INS;
				points[k++] = held_data.theset->d[1];
				points[k++] = 0;

				draw_labeled_plane(points,"R",offset,zoom);
				break;
			case 2:
				points[k++] = held_data.theset->d[0];
				points[k++] = held_data.theset->d[1] - INS;
				points[k++] = held_data.theset->d[2];

				points[k++] = 0;
				points[k++] = held_data.theset->d[1] - INS;
				points[k++] = held_data.theset->d[2];

				points[k++] = 0;
				points[k++] = held_data.theset->d[1] - INS;
				points[k++] = 0;

				points[k++] = held_data.theset->d[0];
				points[k++] = held_data.theset->d[1] - INS;
				points[k++] = 0;

				draw_labeled_plane(points,"T",offset,zoom);
				break;
			case 3:
				points[k++] = held_data.theset->d[0];
				points[k++] = 0 + INS;
				points[k++] = held_data.theset->d[2];

				points[k++] = 0;
				points[k++] = 0 + INS;
				points[k++] = held_data.theset->d[2];

				points[k++] = 0;
				points[k++] = 0 + INS;
				points[k++] = 0;

				points[k++] = held_data.theset->d[0];
				points[k++] = 0 + INS;
				points[k++] = 0;

				draw_labeled_plane(points,"B",offset,zoom);
				break;
			case 4:
				points[k++] = held_data.theset->d[0];
				points[k++] = held_data.theset->d[1];
				points[k++] = held_data.theset->d[2] - INS;

				points[k++] = 0;
				points[k++] = held_data.theset->d[1];
				points[k++] = held_data.theset->d[2] - INS;

				points[k++] = 0;
				points[k++] = 0;
				points[k++] = held_data.theset->d[2] - INS;

				points[k++] = held_data.theset->d[0];
				points[k++] = 0;
				points[k++] = held_data.theset->d[2] - INS;

				draw_labeled_plane(points,"A",offset,zoom);
				break;
			case 5:
				points[k++] = held_data.theset->d[0];
				points[k++] = held_data.theset->d[1];
				points[k++] = 0 + INS;

				points[k++] = 0;
				points[k++] = held_data.theset->d[1];
				points[k++] = 0 + INS;                      

				points[k++] = 0;
				points[k++] = 0;
				points[k++] = 0 + INS;                      

				points[k++] = held_data.theset->d[0];
				points[k++] = 0;
				points[k++] = 0 + INS;                      

				draw_labeled_plane(points,"P",offset,zoom);
				break;
		}
	}
	return;
}
void VL_DRAW_LABELS_CUTPLANE(long int *offset,long int zoom,long int INS)
{
vl_draw_labels_cutplane_(offset,zoom,INS);
}

void draw_labeled_plane(long int *points,char *thechar,long int *offset,
	long int zoom)
{
	double		vd1[3],vd2[3],vout[12],vv[3],q;
	long int	i,j,k,iz;
	double		hd[3],G,t,temp[3];
	double 		ang;


/* get Q for the plane equation */
	q = 0.0;
	for(i=0;i<3;i++) q += (held_data.p[i]*held_data.n[i]);
/* get dataset params */
	for(i=0;i<3;i++) hd[i] = (held_data.theset->d[i])/2.0;

/* get the first point (wraparound) */
	vd2[0] =(double)(points[9] - hd[0]);
	vd2[1] =(double)(points[10] - hd[1]);
	vd2[2] =((double)(points[11]-hd[2]))*(held_data.theset->squeeze_factor);

	k = 0; /* accomulates output */
	j = 0; /* points to input */
	for(i=0;i<4;i++) {

/* get one of the vectors */
		vd1[0] = vd2[0];  vd1[1] = vd2[1];  vd1[2] = vd2[2];
		vd2[0] =(double)(points[j+0] - hd[0]);
		vd2[1] =(double)(points[j+1] - hd[1]);
		vd2[2] =((double)(points[j+2] - hd[2]))*
				(held_data.theset->squeeze_factor);

/* intersect vd1-vd2 with cutting plane */
/* get the vector */
		vv[0] = (vd2[0] - vd1[0]);
		vv[1] = (vd2[1] - vd1[1]);
		vv[2] = (vd2[2] - vd1[2]);
/* compute demoninator of intersection equation */
		G = (held_data.n[0]*vv[0]);
		G += (held_data.n[1]*vv[1]);
		G += (held_data.n[2]*vv[2]);
/* if not coplanar */
		if (G != 0.0) {
			t = (q-((held_data.n[0]*vd1[0])+
			(held_data.n[1]*vd1[1])+(held_data.n[2]*vd1[2])))/G;
/* if valid T get the coordinates */
#ifdef DEBUG
			printf("T=%lf\n",t);
#endif
			if ((t >= 0.0) && (t <= 1.0)) {
/* get the 3D coords (pixel space) */
				vout[k+0] = vd1[0]+ (t*vv[0]);
				vout[k+1] = vd1[1]+ (t*vv[1]);
				vout[k+2] = vd1[2]+ (t*vv[2]);
/* vector from P to the point */
				temp[0] = vout[k+0] - held_data.p[0];
				temp[1] = vout[k+1] - held_data.p[1];
				temp[2] = vout[k+2] - held_data.p[2];
#ifdef DEBUG
				printf("Vout: %lf %lf %lf   temp: %lf %lf %lf\n",vout[k+0],vout[k+1],vout[k+2],temp[0],temp[1],temp[2]);
#endif
/* project onto cutting plane */
				vout[k+0] = (temp[0]*held_data.x[0]) +
						(temp[1]*held_data.x[1]) +
						(temp[2]*held_data.x[2]);

				vout[k+1] = (temp[0]*held_data.y[0]) +
						(temp[1]*held_data.y[1]) +
						(temp[2]*held_data.y[2]);

				vout[k+2] = (temp[0]*held_data.n[0]) +
						(temp[1]*held_data.n[1]) +
						(temp[2]*held_data.n[2]);
/* correct to 0-imagesize realm */
				vout[k+0] += (held_data.imdx/2.0);
				vout[k+1] += (held_data.imdy/2.0);
#ifdef DEBUG
				printf("Vout++: %lf %lf %lf\n",vout[k+0],vout[k+1],vout[k+2]);
#endif
				k = k + 3;
			}
		}
		j = j + 3;
	}
/* draw the letter */
	if (k > 3) {
/*
		for(i=0;i<6;i=i+3) {
		    vout[i]=(vout[i]-(double)(offset[0]))*(double)(zoom);
		    vout[i+1]=(vout[i+1]-(double)(offset[1]))*(double)(zoom);
#ifdef DEBUG
		    printf("Drawing: %lf %lf\n",vout[i],vout[i+1]);
#endif
		}
*/
		vout[6] = (vout[0] + vout[3])/2;
		vout[7] = (vout[1] + vout[4])/2;

/* push point out to edge of the image */
		vout[6] -= (held_data.imdx/2.0);
		vout[7] -= (held_data.imdy/2.0);
		ang = atan2(vout[7],vout[6]);

		if (fabs(ang) < M_PI_4) {
/* along X = (held_data.imdx/2.0) */
			vout[8] = (held_data.imdx/2.0) - 10;
			vout[9] = (vout[7]*vout[8])/vout[6];
		} else if ((ang > 0) && (fabs(ang) < 3*M_PI_4)) {
/* along Y = (held_data.imdy/2.0) */
			vout[9] = (held_data.imdy/2.0) - 12;
			vout[8] = (vout[9]*vout[6])/vout[7];
		} else if ((ang < 0) && (fabs(ang) < 3*M_PI_4)) {
/* along Y = -(held_data.imdy/2.0) */
			vout[9] = -(held_data.imdy/2.0) + 4;
			vout[8] = (vout[9]*vout[6])/vout[7];
		} else {
/* along X = -(held_data.imdx/2.0) */
			vout[8] = -(held_data.imdx/2.0) + 2;
			vout[9] = (vout[7]*vout[8])/vout[6];
		}
		vout[8] += (held_data.imdx/2.0);
		vout[9] += (held_data.imdy/2.0);
		vout[8] = (vout[8]-(double)(offset[0]))*(double)(zoom);
		vout[9] = (vout[9]-(double)(offset[1]))*(double)(zoom);

/* move the pen and draw */
		cmov2(vout[8],vout[9]);
		charstr(thechar);
	}

	return;
}
