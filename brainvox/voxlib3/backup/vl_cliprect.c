#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "voxel.h"

/* local prototypes */
long int vl_CLIPp(long int x,long int y,
        long int Xmin,long int Ymin,long int Xmax,long int Ymax);
long int vl_CLIPt(double d,double n,double *tE,double *tL);

/* taken liberally from Foley and VanDam (2) pg 122 */
long int vl_CLIPt(double d,double n,double *tE,double *tL)
{
        double          t;
        long int        accept;

        accept = 1;
        if (d > 0.0) {
                t = n/d;
                if (t > (*tL)) {
                        accept = 0;
                } else if (t > (*tE)) {
                        *tE = t;
                }
        } else if (d < 0.0) {
                t = n/d;
                if (t < (*tE)) {
                        accept = 0;
                } else if (t < (*tL)) {
                        *tL = t;
                }
        } else {
                if (n > 0.0) accept = 0;
        }
        return(accept);
}

/* return 1 if inside the rect */
long int vl_CLIPp(long int x,long int y,
        long int Xmin,long int Ymin,long int Xmax,long int Ymax)
{

#ifdef DEBUG
        printf("vl_CLIPp %lf,%lf : %lf,%lf %lf,%lf\n",x/FF,y/FF,Xmin/FF,Ymin/FF,Xmax/FF,Ymax/FF);
#endif

        if (x < Xmin) return(0);
        if (x >= Xmax) return(0);
        if (y < Ymin) return(0);
        if (y >= Ymax) return(0);

        return(1);
}

/* all input values are 16.16 fixed point values */
long int vl_clipline(long int *vid,long int *vidd,long int nloops,
        long int Xmin, long int Ymin, long int Xmax, long int Ymax,
        long int *out)
{
/* vid = start point, vidd = deltas, nloops = steps */
/* v1=vid; v2=vid+(nloops*vidd); */
/* clips to Xmin,Ymin,Xmax,Ymax */
/* out[0] = nloops start, out[1] = nloops end */
/* returns 0 if clipped out */

        long int        visible,j;
        long int        xp,yp;
        double          tE,tL;
        double          dx,dy;

        dx = vidd[0]*(nloops);
        dy = vidd[1]*(nloops);
        visible = 0;
        out[0] = 0;
        out[1] = (nloops);
#ifdef DEBUG
        printf("vl_clipline x,y:%lf,%lf dx,dy:%lf,%lf: %lf,%lf %lf,%lf\n",
                vid[0]/FF,vid[1]/FF,dx/FF,dy/FF,Xmin/FF,Ymin/FF,Xmax/FF,Ymax/FF);
        printf("nloops=%ld\n",nloops);
#endif
       
        if ((dx == 0) && (dy == 0) && vl_CLIPp(vid[0],vid[1],Xmin,Ymin,Xmax,Ymax)) {
                visible = 1;
        } else {
                tE = 0.0;
                tL = 1.0;
                if (vl_CLIPt(dx,(double)(Xmin-vid[0]),&tE,&tL)) {
                        if (vl_CLIPt(-dx,(double)(vid[0]-Xmax),&tE,&tL)) {
                                if (vl_CLIPt(dy,(double)(Ymin-vid[1]),&tE,&tL))
{
                                        if (vl_CLIPt(-dy,(double)(vid[1]-Ymax),&tE,&tL)) {
                                                        visible = 1;
                                                        if (tE > 0.0) out[0] = tE*(double)(nloops);
                                                        if (tL < 1.0) out[1] = tL*(double)(nloops);
                                        }
                                }
                        }
                }
        }
        if (!visible) return(visible);

#ifdef DEBUG
        printf("out[0] = %ld, out[1] = %ld\n",out[0],out[1]);
#endif
/* now it is touchup time */
/* first for tE */
/*      if (out[0] != 0) { */
        if (1) {
                j = out[0];
                xp = vid[0]+vidd[0]*j;
                yp = vid[1]+vidd[1]*j;
                if (vl_CLIPp(xp,yp,Xmin,Ymin,Xmax,Ymax)) {
                        xp -= vidd[0];
                        yp -= vidd[1];
                        while ((j > 0) &&
                                (vl_CLIPp(xp,yp,Xmin,Ymin,Xmax,Ymax))) {
                                j -= 1;
                                xp -= vidd[0];
                                yp -= vidd[1];
                        }
                } else {
                        xp += vidd[0];
                        yp += vidd[1];
                        j += 1;
                        while ((j < nloops) &&
                                (!vl_CLIPp(xp,yp,Xmin,Ymin,Xmax,Ymax))) {
                                j += 1;
                                xp += vidd[0];
                                yp += vidd[1];
                        }
                }
                out[0] = j;
        }
/* last for tL */
/*      if (out[1] != (nloops)) { */
        if (1) {
                j = out[1];
                xp = vid[0]+vidd[0]*j;
                yp = vid[1]+vidd[1]*j;
                if (!vl_CLIPp(xp,yp,Xmin,Ymin,Xmax,Ymax)) {
                        j -= 1;
                        xp -= vidd[0];
                        yp -= vidd[1];
                        while ((j > 0) &&
                                (!vl_CLIPp(xp,yp,Xmin,Ymin,Xmax,Ymax))) {
                                j -= 1;
                                xp -= vidd[0];
                                yp -= vidd[1];
                        }
                } else {
                        xp += vidd[0];
                        yp += vidd[1];
                        while ((j < nloops) &&
                                (vl_CLIPp(xp,yp,Xmin,Ymin,Xmax,Ymax))) {
                                j += 1;
                                xp += vidd[0];
                                yp += vidd[1];
                        }
                }
                out[1] = j;
        }
#ifdef DEBUG
        printf("out[0] = %ld, out[1] = %ld\n",out[0],out[1]);
#endif
        return(visible);
}

static void get_vert(long int face,long int n,vset *set,double *v);
static void scan_tri(double *v1,double *v2,double *v3,long int *lne,vset *set);
static void cmp_swap(double *v1,double *v2);

static void  cmp_swap(double *v1,double *v2)
{
        double   temp;
	long int i;

/* if Y1 > Y2 or (Y1=Y2 and X1>X2) then swap */
        if ((v1[1] > v2[1]) ||
                ((v1[1] == v2[1])
                && (v1[0] > v2[0]))) {
		
		for(i=0;i<2;i++) {
                	temp = v1[i];
                	v1[i] = v2[i];
                	v2[i] = temp;
		}
        }
}

#define INTERP_V(a,b,per)  ((a)+(((b)-(a))*(per)))

static void scan_tri(double *v1,double *v2,double *v3,long int *lne,vset *set)
{
	double		p1[2],p2[2],p3[2];
	long int	i,dx1,dx2;
	long int	y,yt,yb,ym;
	double		d1,d2,inc1,inc2;

/* buffer and order the points */	
	for(i=0;i<2;i++) {
		p1[i] = v1[i];
		p2[i] = v2[i];
		p3[i] = v3[i];
	}
	cmp_swap(p1,p2);
	cmp_swap(p2,p3);
	cmp_swap(p1,p2);

#ifdef DEBUG
	printf("%lf %lf\n",p1[0],p1[1]);
	printf("%lf %lf\n",p2[0],p2[1]);
	printf("%lf %lf\n",p3[0],p3[1]);
#endif

/* get scan values */
	yt = p1[1];
	ym = p2[1];
	yb = p3[1];
	d1 = fabs(p1[1] - p2[1]);
	d2 = fabs(p1[1] - p3[1]);
	inc1 = 0;
	inc2 = 0;
/* scan from p1[1] to p2[1] */
	for(y=yt;y<ym;y++) {
/* interpolate along the lines */
		dx1 = INTERP_V(p1[0],p2[0],(inc1/d1));
		dx2 = INTERP_V(p1[0],p3[0],(inc2/d2));
		if ((y >= 0) && (y < set->imagey)) lne[y] += abs(dx1-dx2);
/* move along */
		inc1 += 1.0;
		inc2 += 1.0;
	}

/* scan from p2[1] to p3[1] */
	d1 = fabs(p2[1] - p3[1]);
	inc1 = 0;
	for(y=ym;y<yb;y++) {
/* interpolate along the lines */
		dx1 = INTERP_V(p2[0],p3[0],(inc1/d1));
		dx2 = INTERP_V(p1[0],p3[0],(inc2/d2));
		if ((y >= 0) && (y < set->imagey)) lne[y] += abs(dx1-dx2);
/* move along */
		inc1 += 1.0;
		inc2 += 1.0;
	}

	return;
}

/*

verts:          ^Z+
	      6-|----7
	  Y+\ |\|    |\
             \| 4------5
              2-|----3 |
               \|     \|
                0------1 --->X+

faces:0=x+ 1=x- 2=y+ 3=y- 4=z+ 5=z-

*/
static void get_vert(long int face,long int n,vset *set,double *v)
{
	long int	vtab[6][4] = {
				{1,3,7,5},{4,6,2,0},
				{6,7,3,2},{0,1,5,4},
				{4,5,7,6},{2,3,1,0} };

	v[3] = 1;

	switch (vtab[face][n]) {
		case 0:
			v[0] = set->start[0];
			v[1] = set->start[1];
			v[2] = set->start[2];
			break;
		case 1:
			v[0] = set->end[0];
			v[1] = set->start[1];
			v[2] = set->start[2];
			break;
		case 2:
			v[0] = set->start[0];
			v[1] = set->end[1];
			v[2] = set->start[2];
			break;
		case 3:
			v[0] = set->end[0];
			v[1] = set->end[1];
			v[2] = set->start[2];
			break;
		case 4:
			v[0] = set->start[0];
			v[1] = set->start[1];
			v[2] = set->end[2];
			break;
		case 5:
			v[0] = set->end[0];
			v[1] = set->start[1];
			v[2] = set->end[2];
			break;
		case 6:
			v[0] = set->start[0];
			v[1] = set->end[1];
			v[2] = set->end[2];
			break;
		case 7:
			v[0] = set->end[0];
			v[1] = set->end[1];
			v[2] = set->end[2];
			break;
	}

/* subtract 1/2 */
	v[0] = v[0] - (set->d[0]/2);
	v[1] = v[1] - (set->d[1]/2);
	v[2] = v[2] - (set->d[2]/2);

/* done */
	return;
}

void vl_calc_even_rects(vset *set,long int n,long int *lines)
{
	double		mat[4][4],nmat[4][4];
	double		acc,per;
	double		norm[4],normin[4];
	double		vin[4][4],v[4][4];
	long int	i,j,k;
	long int	*sline;
	long int	facenorm[6][3] = { 	{1,0,0},{-1,0,0},
						{0,1,0},{0,-1,0},
						{0,0,1},{0,0,-1} };

/* setup defaults in case this does not work */
	j = set->imagey/n;
	lines[0] = j;
	for(i=1;i<n;i++) {
		lines[i] = lines[i-1] + j;
	}
	lines[n-1] = set->imagey;
	if (n == 1) return;  /* trival case */

/* allocate the accumunlators */
	sline = malloc(set->imagey*sizeof(long int));
	if (sline == 0) return;
	for(i=0;i<set->imagey;i++) sline[i] = 0;

/* get the tranformation matrix */
        vl_vset_to_4x4(set,mat);

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

/* for each face */
	for(i=0;i<6;i++) {

/* get the verts:vin */
		for(j=0;j<4;j++) {
			get_vert(i,j,set,vin[j]);
		}
/* and normals */
		for(j=0;j<3;j++) normin[j] = facenorm[i][j];
		normin[3] = 1;

/* transform the normals */
                vl_matmult4x4(normin,norm,nmat);

/* backface cull: Norm dot Z > 0.0 then visible */
		if (norm[2] > 0) {
#ifdef DEBUG
			printf("Rendering plane %ld\n",i);
			printf("Norm:%lf %lf %lf\n",norm[0],norm[1],norm[2]);
#endif
/* transform verts */
                	vl_matmult4x4(vin[0],v[0],mat);
                	vl_matmult4x4(vin[1],v[1],mat);
                	vl_matmult4x4(vin[2],v[2],mat);
                	vl_matmult4x4(vin[3],v[3],mat);
/* shift for screen display */
			for(j=0;j<4;j++) {
				v[j][0] = v[j][0] + (set->imagex/2);
				v[j][1] = v[j][1] + (set->imagey/2);
			}
/* scan them into the line accumulators */
			scan_tri(v[0],v[1],v[2],sline,set);
			scan_tri(v[0],v[2],v[3],sline,set);
		}
	} /* next face */

/* create the lines array */
/* sum the number of pixels */
	j = 0;
	for(i=0;i<set->imagey;i++) {
		j += sline[i];
#ifdef DEBUG
		printf("%ld : %ld %ld\n",i,sline[i],j);
#endif
	}

/* add lines until the percentage exceeds the proper percent */
	k = 0;
	acc = 0;
/* note that lines[n-1] = set->imagey always */
	for(i=0;i<(n-1);i++) {
		per = (double)(i+1)/(double)(n);
		while((acc/(double)(j)) < per) acc += sline[k++];
		lines[i] = k;
	}
/* cleanup */
	free(sline);
/* done */
	return;
}
