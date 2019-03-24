#include "voxel.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "vl_util.h"
#include "vl_postlight.h"

#ifndef M_PI
#define M_PI 3.14159
#endif

#undef DEBUG

#ifdef DEBUG
void  dumpfile(VLCHAR *file,void *data,VLINT32 num);
#endif
/*
typedef struct {
	VLFLOAT64 	zstretch;
	VLFLOAT64 	jitter_frac;
	VLINT32	mult_min;
	VLINT32	mult_max; 
	VLFLOAT64  rl_x;
	VLFLOAT64  rl_y;
	VLFLOAT64  rl_z;
	VLFLOAT64  rl_inten;
	VLFLOAT64  rl_srcinten;
	VLINT32 *l_table;
	VLINT32 hz_near;
	VLINT32 hz_far;
} vl_light_params;
*/

void vl_arbrot_vector(VLFLOAT64 *in,VLFLOAT64 *axis,VLFLOAT64 angle,VLFLOAT64 *out);
VLINT32 vl_shadowb_(VLUCHAR *smask,vset *set,VLFLOAT64 *norm,VLINT32 filter);

void vl_addshadow_(VLFLOAT64 di,VLFLOAT64 li,VLFLOAT64 *norm,VLINT32 filter,vset *set,
	VLINT32 backdrops)
{
	VLINT32		i;
	VLFLOAT64			d;
	VLUCHAR 		*mask;
	VLUINT32	*m,*img;
	VLUINT32	rl,gl,bl,rd,gd,bd;
	VLUINT32	lm[256],dm[256];

	for(i=0;i<256;i++) {
		d = li*i;
		if (d < 0.0) d = 0.0;
		if (d > 255.0) d = 255.0;
		lm[i] = d;
		d = di*i;
		if (d < 0.0) d = 0.0;
		if (d > 255.0) d = 255.0;
		dm[i] = d;
	}

	if ((set->zbuffer == 0L)||(set->image == 0L)) {
		vl_puts("Error: No vset buffers specified");
		return;
	}

	mask = MALLOC(set->imagex*set->imagey);
	if (mask == 0L) return;

	if (backdrops < 0) {
		for(i=0;i<(set->imagex*set->imagey);i++) {
			if (set->zbuffer[i] < VL_OUTTHERE) {
				set->zbuffer[i] = backdrops;
			}
		}
	}
	
#ifdef DEBUG
	dumpfile("Zin.16",set->zbuffer,(set->imagex*set->imagey)*sizeof(VLINT16));
	printf("Computing the shadow buffer\n");
#endif
/* compute shadow mask buffer */
	if(vl_shadowb_(mask,set,norm,filter) == 0) {
/* apply the mask  0=shadow 255=lighted */
#ifdef DEBUG
	printf("Applying the shadow buffer\n");
	dumpfile("shadow.8",mask,(set->imagex*set->imagey));
#endif
		img = (VLUINT32 *)set->image;
		for(i=0;i<(set->imagex*set->imagey);i++) {
/* mask  0=shadow   255=non-shadow */
			rd = dm[(img[i] & 0x000000ff)];
			gd = dm[(img[i] & 0x0000ff00) >>  8];
			bd = dm[(img[i] & 0x00ff0000) >> 16];
			rl = lm[(img[i] & 0x000000ff)];
			gl = lm[(img[i] & 0x0000ff00) >>  8];
			bl = lm[(img[i] & 0x00ff0000) >> 16];
			rd = ((mask[i]*rl) + ((255-mask[i])*rd)) >> 8;
			gd = ((mask[i]*gl) + ((255-mask[i])*gd)) >> 8;
			bd = ((mask[i]*bl) + ((255-mask[i])*bd)) >> 8;
			img[i] = rd | (gd << 8) | (bd << 16);
		}
	}

	FREE(mask);
	return;
}

VLINT32 vl_shadowb_(VLUCHAR *smask,vset *set,VLFLOAT64 *norm,VLINT32 filter)
{
	VLFLOAT64 		l,angle;
	VLFLOAT64 		lv[3],ev[3],eXl[3];
	VLFLOAT64		ex[3],ey[3],lx[3],ly[3],lz[3];
	VLFLOAT64		oldrots[3];
	VLUCHAR	*timage,*oimage;
	VLINT16		*tzbuf,*ozbuf;
	VLINT32	i,j,k,count,x,y,z,firsts;
	VLFLOAT64		p[3],p2[3];

	if (smask == 0L) {
		vl_puts("Error: No shadow buffer allocated");
		return(1);
	}
	if (set == 0L) {
		vl_puts("Error: No vset specified");
		return(1);
	}
	if ((set->zbuffer == 0L)||(set->image == 0L)) {
		vl_puts("Error: No vset buffers specified");
		return(1);
	}


	l = norm[0]*norm[0] + norm[1]*norm[1] + norm[2]*norm[2];
	if (l == 0.0) {
		vl_puts("Error: Zero length lighting normal");
		return(1);
	}
	l = sqrt(l);

	lv[0] = norm[0] / l;
	lv[1] = norm[1] / l;
	lv[2] = norm[2] / l;
	
	if (lv[2] < 0) {
		lv[0] = -lv[0];
		lv[1] = -lv[1];
		lv[2] = -lv[2];
	}

	ev[0] = 0; ex[0] = 1; ey[0] = 0;
	ev[1] = 0; ex[1] = 0; ey[1] = 1;
	ev[2] = 1; ex[2] = 0; ey[2] = 0;

/* cross product to get rotational axis */
	eXl[0] = (ev[1]*lv[2]) - (ev[2]*lv[1]);
	eXl[1] = (ev[2]*lv[0]) - (ev[0]*lv[2]);
	eXl[2] = (ev[0]*lv[1]) - (ev[1]*lv[0]);
	angle = asin(sqrt((eXl[0]*eXl[0])+(eXl[1]*eXl[1])+(eXl[2]*eXl[2])));
	angle = -(angle*180)/M_PI;  /* negative?  or positive */
#ifdef DEBUG
	printf("ev %lf %lf %lf\n",ev[0],ev[1],ev[2]);
	printf("lv %lf %lf %lf\n",lv[0],lv[1],lv[2]);
	printf("Rotating %lf deg over %lf %lf %lf\n",angle,
		eXl[0],eXl[1],eXl[2]);
#endif
	vl_arbrot_vector(ev,eXl,-angle,lz);
	vl_arbrot_vector(ex,eXl,-angle,lx);
	vl_arbrot_vector(ey,eXl,-angle,ly);
#ifdef DEBUG
	printf("lx %lf %lf %lf\n",lx[0],lx[1],lx[2]);
	printf("ly %lf %lf %lf\n",ly[0],ly[1],ly[2]);
	printf("lz %lf %lf %lf\n",lz[0],lz[1],lz[2]);
#endif

/* save off old rotations */
	oldrots[0] = set->rots[0];
	oldrots[1] = set->rots[1];
	oldrots[2] = set->rots[2];

/* allocate the temporary images  and swap current ones out */
	timage = MALLOC(set->imagex*set->imagey*
		(sizeof(VLINT32)+sizeof(VLINT16)));
	if (timage == 0L) return(1);
	tzbuf=(VLINT16 *)((VLINT32)timage+
		(set->imagex*set->imagey*sizeof(VLINT32)));
	oimage = set->image;
	ozbuf = set->zbuffer;
	set->image = timage;
	set->zbuffer = tzbuf;

/* arbrot over axis */
	vl_arb_rotate_(&angle,eXl,set);

#ifdef DEBUG
	printf("Rendering the volume %lf %lf %lf from %lf %lf %lf\n",
		set->rots[0],set->rots[1],set->rots[2],
		oldrots[0],oldrots[1],oldrots[2]);
#endif
/* render the volume */
	 vl_render_(VL_BACKGROUND,0,&i,set);

/* replace original images */
	set->image = oimage;
	set->zbuffer = ozbuf;

/* return old rotations */
	vl_rotations_(oldrots[0],oldrots[1],oldrots[2],set);

/* filter the volume */
#ifdef DEBUG
	printf("filtering the output zbuffer\n");
	dumpfile("Zlight.16",tzbuf,(set->imagex*set->imagey)*sizeof(VLINT16));
#endif
	switch (filter) {
		case VL_ZMEDIAN:
		case VL_ZAVERAGE:
			vl_filter_output_((VLINT32 *)tzbuf,set->imagex,
				set->imagey,filter);
			break;
	}

/* compute shadow mask by comparing ez to lz along lv' */

/* assume all pixels are visible from both... */
	for(i=0;i<(set->imagex*set->imagey);i++) smask[i] = 255;

/* loop through all pixels in the lz image */
	for(y=0;y<set->imagey;y++) {
#ifdef DEBUG
		if ((y % 50) == 0) {
			printf("calculate the mask %ld of %ld\n",y,set->imagey);
		}
#endif
		for(x=0;x<set->imagex;x++) {
			z = tzbuf[x+(y*set->imagex)];
/* only work with those points in lz which are in space */
			if (z > VL_OUTTHERE) {
/* get the starting point by projecting into ez space */
				p[0] = x - (set->imagex/2);
				p[1] = y - (set->imagey/2);
				p[2] = (z/8);
				p2[0] = lx[0]*p[0] + ly[0]*p[1] + lz[0]*p[2];
				p2[1] = lx[1]*p[0] + ly[1]*p[1] + lz[1]*p[2];
				p2[2] = lx[2]*p[0] + ly[2]*p[1] + lz[2]*p[2];
/* walk this point along lv until it reaches infinity */
				count = 0;
				firsts = 1;
				while (1) {
/* if point is on the screen */
					j = p2[0]+(set->imagex/2);
					k = p2[1]+(set->imagey/2);
					if ((j < set->imagex) && (j>=0) &&
						(k<set->imagey) && (k>=0)) {
						i = (k*set->imagex) + j;	
/* if the point is close to ez, then there is a shadow projected here */
/* with the exception of the first surface hit */
		if (fabs((p2[2]*8) - set->zbuffer[i]) < (1.5*8)) {
			if (!firsts) smask[i] = 0;
		} 
					}
		if (count > 3) firsts = 0;
/* increment the point */
					p2[0] -= lv[0];
					p2[1] -= lv[1];
					p2[2] -= lv[2];
					count++;
					if ((count > 8000) || ((p2[2]*8) < 
						VL_OUTTHERE)) break;
				}
			}
		}
	}
	FREE(timage);

	return(0);
}

void vl_arbrot_vector(VLFLOAT64 *in,VLFLOAT64 *axis,VLFLOAT64 angle,VLFLOAT64 *out)
{
        VLFLOAT64 temp2[3][3],dang,u[3],l;

/* convert angle to radians */
        dang = -((angle)/180.0)*M_PI;

/* make sure it is a UNIT vector */
        l = (axis[0]*axis[0]) + (axis[1]*axis[1]) + (axis[2]*axis[2]);
        if (l == 0.0) {
                vl_puts("Error: Zero length unit axis specified.");
                return;
        }
        l = sqrt(l);
        u[0] = axis[0] / l;
        u[1] = axis[1] / l;
        u[2] = axis[2] / l;

/* build the matrix for rotation about an arbitrary axis (u) */
        temp2[0][0] = u[0]*u[0] + cos(dang)*(1.0 - (u[0]*u[0]));
        temp2[1][0] = u[0]*u[1]*(1.0 - cos(dang)) - u[2]*sin(dang);
        temp2[2][0] = u[2]*u[0]*(1.0 - cos(dang)) + u[1]*sin(dang);

        temp2[0][1] = u[0]*u[1]*(1.0 - cos(dang)) + u[2]*sin(dang);
        temp2[1][1] = u[1]*u[1] + cos(dang)*(1.0 - (u[1]*u[1]));
        temp2[2][1] = u[1]*u[2]*(1.0 - cos(dang)) - u[0]*sin(dang);

        temp2[0][2] = u[2]*u[0]*(1.0 - cos(dang)) - u[1]*sin(dang);
        temp2[1][2] = u[1]*u[2]*(1.0 - cos(dang)) + u[0]*sin(dang);
        temp2[2][2] = u[2]*u[2] + cos(dang)*(1.0 - (u[2]*u[2]));

/* do the multiply */
	vl_matmult(in,out,temp2);

	return;
}

#ifdef DEBUG
void  dumpfile(VLCHAR *file,void *data,VLINT32 num)
{
	FILE *fp;	
	fp = fopen(file,"w");
	if (fp != 0L) {
		fwrite (data,1L,num,fp); 
		fclose(fp);
	}
	return;
}
#endif
