/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski
 *
 * $Id: roi_utils.c 1250 2005-09-16 15:51:42Z dforeman $
 *                University of Iowa, Department of Neurology,
 *                Human Neuroanatomy and Neuroimaging Lab
 *                University of Iowa, Image Analysis Facility
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All modifications to the source code must be clearly marked as
 *    such.  Binary redistributions based on modified source code
 *    must be clearly marked as modified versions in the documentation
 *    and/or other materials provided with the distribution.
 * 4. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgment:
 *      This product includes software developed by Randall Frank, Thomas
 *      Grabowski and other unpaid contributors.
 * 5. The names of Randall Frank or Thomas Grabowski may not be used to
 *    endorse or promote products derived from this software without 
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY RANDALL FRANK AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL RANDALL FRANK OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "roi_utils.h"
#include "polyfill.h"

#ifndef M_PI
#define M_PI 3.141592653589793
#endif

/* take binary sign of a, either -1, or 1 if >= 0 */
#define SGN(a)	(((a)<0) ? -1 : 0)

/* take sign of a, either -1, 0, or 1 */
#define ZSGN(a)	(((a)<0) ? -1 : (a)>0 ? 1 : 0)  

typedef struct {
	long int 	imagex,imagey;
	unsigned char	*image_data;
	unsigned char	*image_buffer;
	double		data_rep[256];
	long int	histo[256];
} ROI_GLOBALS;

ROI_GLOBALS ROIglobal;

static long int Inited_package = 0;


void	init_roi_utils()
{
	long int	i;

	if (Inited_package) return;

	Inited_package = 1;
	ROIglobal.imagex = 256;
	ROIglobal.imagey = 256;
	ROIglobal.image_data = 0L;
	for(i=0;i<256;i++) {
		ROIglobal.data_rep[i] = i;
		ROIglobal.histo[i] = 0;
	}

	return;
}

void	roi_set_image(long int dx,long int dy,unsigned char *in,
		unsigned char *in2)
{
	ROIglobal.imagex = dx;
	ROIglobal.imagey = dy;
	ROIglobal.image_data = in;
	ROIglobal.image_buffer = in2;

	return;
}

void	roi_set_data_rep(double *rep)
{
	long int	i;
	
	for(i=0;i<256;i++) {
		ROIglobal.data_rep[i] = rep[i];
	}

	return;
}

long    clipped_point_img(long int x,long int y)
{
        if (x < 0) return(1);
        if (y < 0) return(1);
        if (x >= ROIglobal.imagex) return(1);
        if (y >= ROIglobal.imagey) return(1);

        return(0);
}

void    clip_point_img(long int *x,long int *y)
{
        if ((*x) < 0) *x=0;
        if ((*y) < 0) *y=0;
        if ((*x) >= ROIglobal.imagex) *x=ROIglobal.imagex-1;
        if ((*y) >= ROIglobal.imagey) *y=ROIglobal.imagey-1;

        return;
}

void init_roi(Trace *roi,char *name)
{
	char	*tname;

	roi->num_points = 0;
	roi->visible = 1;
	roi->dirty = 0;
	roi->template = 0;
	strcpy(roi->filename,name);
	tname = strrchr(name,'/')+1;
	if ((long int)tname == 1L) tname = name;
	strcpy(roi->name,tname);

	return;
}

void   roi_bounds(Trace *roi,Point *min,Point *max)
{
	long int	i;

	min->x = 100000;
	min->y = 100000;

	max->x = -100000;
	max->y = -100000;

	for(i=0;i<roi->num_points;i++) {
		if (roi->thepnts[i].x > max->x) max->x = roi->thepnts[i].x;
		if (roi->thepnts[i].x < min->x) min->x = roi->thepnts[i].x;

		if (roi->thepnts[i].y > max->y) max->y = roi->thepnts[i].y;
		if (roi->thepnts[i].y < min->y) min->y = roi->thepnts[i].y;
	}

	return;
}

void   roi_centroid(Trace *roi,Point *com)
{
	double	value = 0.0;
	double	tmp[4],det;
	long int	i;

	com->x = 0;
	com->y = 0;

	tmp[0] = roi->thepnts[roi->num_points-1].x;
	tmp[1] = roi->thepnts[roi->num_points-1].y;
	for(i=0;i<roi->num_points;i++) {
		tmp[2] = roi->thepnts[i].x;
		tmp[3] = roi->thepnts[i].y;
		det = ((tmp[0]*tmp[3])-(tmp[1]*tmp[2]));
		value += det;
/* com = com + area*avgpt;  avg pt = (p0+p1+0)/3. */
		com->x += (det*(tmp[0]+tmp[2]));
		com->y += (det*(tmp[1]+tmp[3]));
		tmp[0] = tmp[2];
		tmp[1] = tmp[3];
	}
	com->x = com->x/(3.*value);
	com->y = com->y/(3.*value);

	return;
}

void roi_CCW(Trace *roi)
{
	Trace	rtmp;
	double	value = 0.0;
	double	tmp[4];
	long int i,j;

	if (roi->num_points < 3) return;

	rtmp = (*roi);

/* assumes the ROI is closed!!! */
	tmp[0] = roi->thepnts[roi->num_points-1].x;
	tmp[1] = roi->thepnts[roi->num_points-1].y;
	for(i=0;i<roi->num_points;i++) {
		tmp[2] = roi->thepnts[i].x;
		tmp[3] = roi->thepnts[i].y;
		value = value + ((tmp[0]*tmp[3])-(tmp[1]*tmp[2]));
		tmp[0] = tmp[2];
		tmp[1] = tmp[3];
	}
/* if negative, swap directions */
	if (value < 0) {
		j = roi->num_points-1;
		for(i=0;i<roi->num_points;i++) {
			roi->thepnts[i] = rtmp.thepnts[j];
			j--;
		}
	}
	return;
}

double roi_area(Trace *roi)
{
	double	value = 0.0;
	double	tmp[4];
	long int i;

	if (roi->num_points < 3) return(value);
/* assumes the ROI is closed!!! */
	tmp[0] = roi->thepnts[roi->num_points-1].x;
	tmp[1] = roi->thepnts[roi->num_points-1].y;
	for(i=0;i<roi->num_points;i++) {
		tmp[2] = roi->thepnts[i].x;
		tmp[3] = roi->thepnts[i].y;
		value = value + ((tmp[0]*tmp[3])-(tmp[1]*tmp[2]));
		tmp[0] = tmp[2];
		tmp[1] = tmp[3];
	}
	value = fabs(value/2.0);
	return(value);
}

double roi_perimeter(Trace *roi)
{
	double	value = 0.0;
	double	tmp[4];
	long int i;

	if (roi->num_points < 3) return(value);
/* assumes the ROI is closed!!! */
	tmp[0] = roi->thepnts[roi->num_points-1].x;
	tmp[1] = roi->thepnts[roi->num_points-1].y;
	for(i=0;i<roi->num_points;i++) {
		tmp[2] = roi->thepnts[i].x;
		tmp[3] = roi->thepnts[i].y;
		value = value + sqrt(((tmp[0]-tmp[2])*(tmp[0]-tmp[2])) +
			((tmp[1]-tmp[3])*(tmp[1]-tmp[3])));
		tmp[0] = tmp[2];
		tmp[1] = tmp[3];
	}
	return(value);
}

double roi_length(Trace *roi)
{
	double	value = 0.0;
	double	tmp[4];
	long int i;

	if (roi->num_points < 2) return(value);

	for(i=1;i<roi->num_points;i++) {
		tmp[0] = roi->thepnts[i-1].x;
		tmp[1] = roi->thepnts[i-1].y;
		tmp[2] = roi->thepnts[i].x;
		tmp[3] = roi->thepnts[i].y;
	
		value += sqrt(((tmp[0]-tmp[2])*(tmp[0]-tmp[2])) +
			((tmp[1]-tmp[3])*(tmp[1]-tmp[3])));
	}

	return(value);
}

double roi_angle(Trace *roi)
{
	double	value = 0.0;
	double	v1[2],v2[2],d1,d2;
	long int i;

	if (roi->num_points < 3) return(value);
	i = roi->num_points -1;

	v1[0] = roi->thepnts[i].x - roi->thepnts[i-1].x;
	v1[1] = roi->thepnts[i].y - roi->thepnts[i-1].y;
	d1 = sqrt((v1[0]*v1[0]) + (v1[1]*v1[1]));

	v2[0] = roi->thepnts[i-2].x - roi->thepnts[i-1].x;
	v2[1] = roi->thepnts[i-2].y - roi->thepnts[i-1].y;
	d2 = sqrt((v2[0]*v2[0]) + (v2[1]*v2[1]));

	value = v1[0]*v2[0] + v1[1]*v2[1];

	if (d1 == 0.0) d1 = 0.001;
	if (d2 == 0.0) d2 = 0.001;

	value = value/(d1*d2);
	if (fabs(value - (-1.0)) < 0.001) {
		value = 3.14159;
	} else {
		value = acos(value);
	}
	value = (value/3.14159)*180.0;

	return(value);
}

void roi_axis(Trace *roi,double *maj,double *min)
{
	Point	ma[2],mn[2];

	roi_axis_points(roi,maj,min,ma,mn);

	return;
}

void roi_axis_points(Trace *roi,double *maj,double *min,Point *ma,Point *mn)
{
	long int i,j,x1,x2,x3,x4,y1,y2,y3,y4,s1,s2,s3,s4;
	double 	a1,a2,b1,b2,c1,c2;
	double	tx,ty,d;
	Point	mavg;

	*maj = 0.0;
	*min = 0.0;
	if (roi->num_points < 10) return;
/* find major axis (2 most distant points) */
	for(i=0;i<roi->num_points-1;i++) {
		for(j=i+1;j<roi->num_points;j++) {
			tx = (roi->thepnts[i].x - roi->thepnts[j].x);
			ty = (roi->thepnts[i].y - roi->thepnts[j].y);
			d = (tx*tx)+(ty*ty);
			if (d > (*maj)) {
				*maj = d;
				ma[0] = roi->thepnts[i];
				ma[1] = roi->thepnts[j];
			}
		}
	}
	*maj = sqrt((*maj));

/* now the minor axis */
/* midpoint of major axis */
	mavg.x = (ma[0].x + ma[1].x)/2;
	mavg.y = (ma[0].y + ma[1].y)/2;
/* slope of minor axis (negative reciprical) */
	ty = -(ma[0].x - ma[1].x);
	tx = ma[0].y - ma[1].y;
/* find the endpoints of the minor axis line segment */
#define BIG_NUM 10000
	x1 = mavg.x + (BIG_NUM*tx);
	y1 = mavg.y + (BIG_NUM*ty);
	x2 = mavg.x - (BIG_NUM*tx);
	y2 = mavg.y - (BIG_NUM*ty);
/* now find the intersections of the contour with this line segemnt */
	x3 = roi->thepnts[roi->num_points-1].x;
	y3 = roi->thepnts[roi->num_points-1].y;
	j = 0;
	for(i=0;i<roi->num_points;i++) {
		x4 = roi->thepnts[i].x;
		y4 = roi->thepnts[i].y;
/* from Pavlidis, Algorithms for Graphic and Image Processing, pg 329 */
/* check for intersections between segments 1,2 and 3,4 */
		s1 = x1*(y3-y4)+y1*(x4-x3)+(x3*y4 - y3*x4);
		s2 = x2*(y3-y4)+y2*(x4-x3)+(x3*y4 - y3*x4);
		s3 = x3*(y1-y2)+y3*(x2-x1)+(x1*y2 - y1*x2);
		s4 = x4*(y1-y2)+y4*(x2-x1)+(x1*y2 - y1*x2);
/* here we go (if s1 and s2 differ in signs and s3 and s4 differ in signs) */
		if ((SGN(s1) != SGN(s2)) && (SGN(s3) != SGN(s4))) {
/* find the point */
			a1 = (y1-y2);
			b1 = (x2-x1);
			c1 = ((x1*y2)-(y1*x2));

			a2 = (y3-y4);
			b2 = (x4-x3);
			c2 = ((x3*y4)-(y3*x4));

			mn[j].x = (c2*b1 - c1*b2)/(a1*b2 - a2*b1)+0.5;
			mn[j].y = (a2*c1 - a1*c2)/(a1*b2 - a2*b1)+0.5;
/* are we done? */
			if (j == 1) break;
			j = 1;
		}
/* next segment */
		x3 = x4;
		y3 = y4;
	}
/* compute the distance */	
	tx = mn[0].x - mn[1].x;
	ty = mn[0].y - mn[1].y;
	*min = sqrt(tx*tx + ty*ty);

	return;
}

#define M_PTS 20000

void	fill_roi(Trace *roi,long int in,long int out)
{
	long int     	list[M_PTS],strips[M_PTS];
	long int	ptr,num,i,x,y,en,st,j,kk;
	unsigned char	*img;

	if (roi->num_points < 3) return;
/* prep for the polyfill */
	num = 0;
	for(i=0;i<roi->num_points;i++) {
		list[num] = roi->thepnts[i].x;
		list[num+1] = roi->thepnts[i].y;
		num = num + 2;
	}
	if (num == 0) return;

/* perform the polygon fill on the list */
        ptr = M_PTS;
        polyfill_c_(list,num,strips,&ptr);
        if (ptr == -1) {
                fprintf(stderr,
			"Warning:overflow encountered in polygon filling.\n");
                return;
        }
        if (ptr == 0) return;

/* get the temporary buffer */
	img = malloc(ROIglobal.imagex*ROIglobal.imagey);
	if (img == 0L) return;
/* get a copy of the original image */
	for(i=0;i<(ROIglobal.imagex*ROIglobal.imagey);i++) {
		img[i] = ROIglobal.image_data[i];
	}
/* flood the image with its background if needed */
	if (out >= 0) {
		for(i=0;i<(ROIglobal.imagex*ROIglobal.imagey);i++) {
			ROIglobal.image_data[i] = out;
		}
	} else if (out == -2) {
		y = 0;
		for(j=0;j<(ROIglobal.imagey);j++) {
			for(i=0;i<(ROIglobal.imagex);i++) {
				if (ROIglobal.image_buffer) {
					kk = ROIglobal.image_buffer[y];
				} else {
					kk = 0;
				}
				ROIglobal.image_data[y] = kk;
				y++;
			}
		}
	} 
/* fill the interior of the ROI */
	for(i=0;i<ptr;i=i+3) {
		y = strips[i+2];
/* clip the strip to the current image */
		st = strips[i];
		en = strips[i+1];
		if (st < 0) st = 0;
		if (en >= ROIglobal.imagex) en = ROIglobal.imagex -1;
		if ((st <= en) && (y >= 0) && (y < ROIglobal.imagey)) {
/* fill the current strip */
			j = st+(y*ROIglobal.imagex);  /* start of line */
			if (in == -1) {
				for(x=st;x<=en;x++) {
					ROIglobal.image_data[j] = img[j];
					j++;
				}
			} else if (in == -2) {
				for(x=st;x<=en;x++) {
					if (ROIglobal.image_buffer != 0) {
						kk = ROIglobal.image_buffer[j];
					} else {
						kk = 0;
					}
/*
					kk = img_buffer_get_pixel(x,y);
*/
					if (kk < 0) kk = 0;
					ROIglobal.image_data[j] = kk;
					j++;
				}
			} else {
				for(x=st;x<=en;x++) {
					ROIglobal.image_data[j] = in;
					j++;
				}
			}
		}
	}
/* free up the temporary buffer */
	free(img);
	return;
}

void roi_data_rep(Trace *roi,double *min,double *max,double *mean,
	double *stdev,double *pixels,double *sum,double *comx,double *comy)
{
	long int     	list[M_PTS],strips[M_PTS];
	long int	ptr,num,i,x,y,en,st,j;
	double		ss,v;
	long int	*histo,dummy[256];

	*min = 0.0;
	*max = 0.0;
	*mean = 0.0;
	*stdev = 0.0;
	*pixels = 0.0;
	*sum = 0.0;
	*comx = 0.0;
	*comy = 0.0;

	if (roi->num_points < 3) return;

/* user may optionally compute a palette histogram for viewing (d_histo) */
	histo = dummy;  /* or histo = d_histo */
	if (ROIglobal.histo) histo = ROIglobal.histo;

	num = 0;
	for(i=0;i<roi->num_points;i++) {
		list[num] = roi->thepnts[i].x;
		list[num+1] = roi->thepnts[i].y;
		num = num + 2;
	}
	if (num == 0) return;

	/* perform the polygon fill on the list */
        ptr = M_PTS;
        polyfill_c_(list,num,strips,&ptr);
        if (ptr == -1) {
                fprintf(stderr,
			"Warning:overflow encountered in polygon filling.\n");
                return;
        }
        if (ptr == 0) return;

/* clear out histogram */
	for(i=0;i<256;i++) histo[i] = 0;

/* run through all the pixels */
	*min = 10000000.0;
	*max = -10000000.0;
	ss = 0;
	for(i=0;i<ptr;i=i+3) {
		y = strips[i+2];
/* clip the strip to the current image */
		st = strips[i];
		en = strips[i+1];
		if (st < 0) st = 0;
		if (en >= ROIglobal.imagex) en = ROIglobal.imagex -1;
		if ((st <= en) && (y >= 0) && (y < ROIglobal.imagey)) {
/* calc stats for the current strip */
			for(x=st;x<=en;x++) {
				j=ROIglobal.image_data[x+(y*ROIglobal.imagex)];
				histo[j] += 1;
				v = (ROIglobal.data_rep[j]);
				ss += (v*v);
				*sum += v;
				if (v < (*min)) *min = v;
				if (v > (*max)) *max = v;
				*comx += x;
				*comy += y;
			}
			*pixels += (en - st + 1);
		}
	}
	*comx = (*comx)/(*pixels);
	*comy = (*comy)/(*pixels);
	*mean = (*sum)/(*pixels);
	*stdev = sqrt(fabs((ss/(*pixels)) - ((*mean)*(*mean))));
	return;
}

void thin_roi(Trace *roi)
{
	long int i,j;
	
	j = 0;
	if (roi->num_points < 8) return;
	for(i=0;i<(roi->num_points-1);i=i+2) {
		roi->thepnts[j].x=(roi->thepnts[i].x + roi->thepnts[i+1].x)/2;
		roi->thepnts[j].y=(roi->thepnts[i].y + roi->thepnts[i+1].y)/2;
		j++;
	}
	roi->num_points = j;
	roi->dirty = 1;
	return;
}

long int closed_roi(Trace *roi)
{
	long int i;

	if (roi->num_points < 8) return(0);
	i = roi->num_points - 1;

	if ((abs(roi->thepnts[i].x - roi->thepnts[0].x) < 2) &&
		(abs(roi->thepnts[i].y - roi->thepnts[0].y) < 2)) {
		return(1);
	}
	return(0);
}

void edit_roi(Trace *roi,Point *edits)
{
	long int i,j,k,first;
	long int pt[3];
	double dist[3],d,dx,dy;
	Point	tmp[MAX_POINTS];

/* assume no points even close */
	dist[0] = 100000;
	dist[1] = 100000;
	dist[2] = 100000;
/* get end of loop */	
	i=roi->num_points;
/* find the closest points */
	for(j=0;j<i;j++) {
		for(k=0;k<3;k++) {
			dx = (double)(roi->thepnts[j].x-edits[k].x);
			dy = (double)(roi->thepnts[j].y-edits[k].y);
			d = sqrt((dx*dx)+(dy*dy));
			if (d < dist[k]) {
				pt[k] = j;
				dist[k] = d;
			}
		}
	}
/* starting at point 1, walk until you reach 0 or 2 */
	j = pt[1];
loop:
	j++;
/* wrap */
	if (j >= i) j = 0;
/* found 0 or 2 ? */
	if ((j != pt[0]) && (j != pt[2])) goto loop;
/* found a starting point */
	first = 0;
	if (j == pt[2]) first = 2;
/* start the recording... */
	k = 0;
	tmp[k] = roi->thepnts[j];
loop2:
	j++;
	if (j >= i) j = 0;
	if ((j == pt[0]) || (j == pt[2])) goto out;
/* copy the point */
	k++;
	tmp[k] = roi->thepnts[j];
	goto loop2;
out:
	k++;
	tmp[k] = roi->thepnts[j];
/*
C       now string out the tr array into the TRACE array
C       if the first point was 1 then do it forward
C       else do it in reverse order this leaves the last point
C       clicked on as the end of the trace.  The user need only continue
C       tracing instead of moving to the first point and trace...
*/
	if (first == 0) {
		for(j=0;j<=k;j++) {
			roi->thepnts[j] = tmp[j];
		}
		roi->num_points = k + 1;
	} else {
		i = 0;
		for(j=k;j>=0;j--) {
			roi->thepnts[i] = tmp[j];
			i++;
		}
		roi->num_points = i;
	}
	roi->dirty = 1;
/* done */
	return;
}

void rotate_roi(Trace *roi,Point *edits)
{
	long int i;
	double ang,v1[3],v2[3],v3[3],l;

/* get the vectors */	
	v1[0] = edits[0].x - edits[1].x;
	v1[1] = edits[0].y - edits[1].y;
	v1[2] = 0;
	l = sqrt((v1[0]*v1[0])+(v1[1]*v1[1])+(v1[2]*v1[2]));
	for (i=0;i<3;i++) v1[i] = v1[i] / l;
	v2[0] = edits[2].x - edits[1].x;
	v2[1] = edits[2].y - edits[1].y;
	v2[2] = 0;
	l = sqrt((v2[0]*v2[0])+(v2[1]*v2[1])+(v2[2]*v2[2]));
	for (i=0;i<3;i++) v2[i] = v2[i] / l;
/* X product */
	v3[0] = (v1[1]*v2[2]) - (v1[2]*v2[1]);
	v3[1] = (v1[2]*v2[0]) - (v1[0]*v2[2]);
	v3[2] = (v1[0]*v2[1]) - (v1[1]*v2[0]);
	l = sqrt((v3[0]*v3[0])+(v3[1]*v3[1])+(v3[2]*v3[2]));
/* since they are normal and in the x,y plane */
	ang = asin(v3[2]);

	ang = (ang/M_PI)*180.0;

	rotate_roi_ang(roi,ang,edits[1].x,edits[1].y);

	return;
}

void rotate_roi_ang(Trace *roi,double ang,long int x,long int y)
{
	long int i;
	double v1[3],v2[3],ca,sa;

	ca = cos((ang/180.0)*M_PI);
	sa = sin((ang/180.0)*M_PI);

	for(i=0;i<roi->num_points;i++) {
/* create rotation vector */
		v1[0] = roi->thepnts[i].x - x;
		v1[1] = roi->thepnts[i].y - y;
/* rotate */
		v2[0] = (v1[0]*ca) - (v1[1]*sa);
		v2[1] = (v1[0]*sa) + (v1[1]*ca);
/* translate back */
		roi->thepnts[i].x = x + v2[0];	
		roi->thepnts[i].y = y + v2[1];	
	}
	roi->dirty = 1;
	return;	
}

void	scale_roi(Trace *roi,double zoom)
{
        long int i;

        for(i=0;i<(roi->num_points);i++) {
                roi->thepnts[i].x *= zoom;
                roi->thepnts[i].y *= zoom;
        }
	
	roi->dirty = 1;
	return;
}

void move_roi(Trace *roi,long int dx, long int dy)
{
	long int i;

	for(i=0;i<(roi->num_points);i++) {
		roi->thepnts[i].x +=dx;
		roi->thepnts[i].y +=dy;
	}
	roi->dirty = 1;
	return;
}

void addpnt_roi(Trace *roi,long int x,long int y)
{
	long int i,tag;
	long int ox,oy;
	double	dx,dy,len,j;

	roi->dirty = 1;
	tag = 0;
	if (roi->num_points >= MAX_POINTS) {
    		fprintf(stderr,
			"Maximum size of ROI reached. Thinning the ROI.\n");
		thin_roi(roi);
		tag = 1;
	}
	i = roi->num_points-1;
/* special case of first point */
	if (roi->num_points == 0) {
		roi->thepnts[0].x = x;
		roi->thepnts[0].y = y;
		roi->num_points = 1;
		return;
	}

/* duplicate ? */
	if ((roi->thepnts[i].x == x) && (roi->thepnts[i].y == y)) return;
/* add the line */
	ox = roi->thepnts[i].x;
	oy = roi->thepnts[i].y;
	dx = (double)(x-ox);
	dy = (double)(y-oy);
	len = (dx*dx)+(dy*dy);
	if (len > 1.0) {
		len = sqrt((dx*dx)+(dy*dy));
	} else {
		len = 1.0;
	}
	dx = dx/len;
	dy = dy/len;
	j = 1.0;
/* along the line */
	while ((abs(x-(roi->thepnts[i].x)) > 1) ||
		(abs(y-(roi->thepnts[i].y)) > 1)) {
		if ((i+2) >= MAX_POINTS) {
			roi->num_points = i+1;
    			fprintf(stderr,
			    "Maximum size of ROI reached. Thinning the ROI.\n");
			thin_roi(roi);
			tag = 1;
			i = roi->num_points - 1;
		}
		roi->thepnts[i+1].x = ox + (int)(j*dx);
		roi->thepnts[i+1].y = oy + (int)(j*dy);
/* add unique points */
		if ((roi->thepnts[i+1].x != roi->thepnts[i].x) ||
			(roi->thepnts[i+1].y != roi->thepnts[i].y)) {
			i++;
		}
		j = j + 1.0;
	}
/* add the stopping point */
	i++;
	roi->thepnts[i].x = x;
	roi->thepnts[i].y = y;
	roi->num_points = i + 1;

	return;
}

long int read_roi(char *file,Trace *roi)
{
        FILE    *fp;
        int     i,j,x,y;

        fp = fopen(file,"r");
        if (fp == 0L) {
                fprintf(stderr,"Unable to open %s for reading.\n",file);
                return(1);
        }
	if (strstr(file,".ctr")) {
        	fscanf(fp,"%d",&i);
        	if (i >= MAX_POINTS) i = MAX_POINTS-1;
        	for(j=0;j<i;j++) {
                	fscanf(fp,"%d %d",&x,&y);
                	roi->thepnts[j].x = x;
                	roi->thepnts[j].y = y;
        	}
	} else {
        	fscanf(fp,"%d",&i);
        	i = i/2;
        	if (i >= MAX_POINTS) i = MAX_POINTS-1;
        	for(j=0;j<i;j++) {
                	fscanf(fp,"%d",&x);
                	fscanf(fp,"%d",&y);
                	roi->thepnts[j].x = x;
                	roi->thepnts[j].y = y;
        	}
	}
        fclose(fp);
        roi->num_points = i;
        roi->dirty = 0;
        fprintf(stderr,"Read %d points from: %s\n",roi->num_points,file);

        return(0);
}

long int save_roi(char *file,Trace *roi)
{
        FILE    *fp;
        int     i,x,y;

        fprintf(stderr,"Saving %d points as: %s\n",roi->num_points,file);

        fp = fopen(file,"w");
        if (fp == 0L) {
                fprintf(stderr,"Unable to open %s for writing.\n",file);
                return(1);
        }
	if (strstr(file,".ctr")) {
        	fprintf(fp,"%d\n",roi->num_points);
        	for(i=0;i<roi->num_points;i++) {
                	x = roi->thepnts[i].x;
                	y = roi->thepnts[i].y;
                	fprintf(fp,"%d %d\n",x,y);
		}
		fprintf(fp,"# Confidence levels\n");
		for(i=0;i<roi->num_points;i++) fprintf(fp,"%ld\n",255L);
		fprintf(fp,"# Misc info\n");
		fprintf(fp," 0.000000 z-coordinate\n");
		fprintf(fp," 1.000000 scale\n");
	} else {
        	fprintf(fp,"%d\n",roi->num_points*2);
        	for(i=0;i<roi->num_points;i++) {
                	x = roi->thepnts[i].x;
                	y = roi->thepnts[i].y;
                	fprintf(fp,"%d\n",x);
                	fprintf(fp,"%d\n",y);
        	}
	}
        fclose(fp);

        roi->dirty = 0;

        return(0);
}

void	roi_2_strips(Trace *roi,long int *strips,long int *npts)
{
	long int     	list[M_PTS];
	long int	num,i;

	if (roi->num_points < 3) return;
/* prep for the polyfill */
	num = 0;
	for(i=0;i<roi->num_points;i++) {
		list[num] = roi->thepnts[i].x;
		list[num+1] = roi->thepnts[i].y;
		num = num + 2;
	}
	if (num == 0) {
		*npts = 0;
		return;
	}

/* perform the polygon fill on the list */
        polyfill_c_(list,num,strips,npts);
	return;
}

void	smooth_roi(Trace *roi,long int n)
{
	Trace		tmp;
	long int	i,j,k;
	float		tx,ty;

/* clone the incoming */
	tmp = (*roi);

	for(i=0;i<tmp.num_points;i++) {
		tx = 0.0; ty = 0.0;
		for(j=0;j<n;j++) {
			k = i + j;
			if (k >= tmp.num_points) k -= tmp.num_points;
			tx += tmp.thepnts[k].x;
			ty += tmp.thepnts[k].y;
		}
		tx /= (float)(n); ty /= (float)(n);
		roi->thepnts[i].x = tx;
		roi->thepnts[i].y = ty;
	}
}

void	sample_roi(Trace *roi,long int n)
{
	float		per[MAX_POINTS],d,e;
	long int	i,j;

	while(roi->num_points < 2*n) interp_roi(roi,1);

/* compute incremental perimeter */
	per[0] = 0;
	for(i=0;i<roi->num_points;i++) {
		j = i + 1;
		if (j == roi->num_points) j = 0;
		d = (roi->thepnts[j].x - roi->thepnts[i].x);
		e = (roi->thepnts[j].y - roi->thepnts[i].y);
		d = sqrt(d*d + e*e);
		per[i+1] = per[i] + d;
	}
/* we need points every (perim/n) along this perimeter */
	d = 0;
	e = per[roi->num_points]/(float)(n);
	i = 0;
	for(j=0;j<n;j++) {
		roi->thepnts[j].x = roi->thepnts[i].x;
		roi->thepnts[j].y = roi->thepnts[i].y;
/* next point */
		d = d + e;
		while(per[i+1] < d) i++;
	}
	roi->num_points = n;
}

void	interp_roi(Trace *roi,long int n)
{
	Trace		tmp;
	long int	i,j,k,d;
	float		dx,dy;

	if ((n+1)*(roi->num_points) > MAX_POINTS) return;

/* clone the incoming */
	tmp = (*roi);
	
/* interpolate points */
	k = 0;
	for(i=0;i<tmp.num_points;i++) {
		j = i + 1;
		if (j == tmp.num_points) j = 0;
		dx = tmp.thepnts[j].x - tmp.thepnts[i].x;
		dy = tmp.thepnts[j].y - tmp.thepnts[i].y;
		dx = dx / (float)(n+1);
		dy = dy / (float)(n+1);
		for(d=0;d<(n+1);d++) {
			roi->thepnts[k].x = tmp.thepnts[i].x + ((float)(d)*dx);
			roi->thepnts[k].y = tmp.thepnts[i].y + ((float)(d)*dy);
			k++;
		}
	}
	roi->num_points = k;
	return;
}

void reorder_roi_angles(Trace *roi)
{
	double		ang[MAX_POINTS];
	Point		com;
	double		dx,dy;
	long int	i,j;

/* get COM */
	roi_centroid(roi,&com);
/* compute angles */
	for(i=0;i<roi->num_points;i++) {
		dx = roi->thepnts[i].x - com.x;
		dy = roi->thepnts[i].y - com.y;
		ang[i] = atan2(dy,dx);
		while(ang[i] < 0.0) ang[i] += (2*M_PI);
	}
/* sort the angles 
   (bubble... N should be pretty small and they should be close to in order) */
	for(i=0;i<(roi->num_points-1);i++) {
		for(j=i+1;j<roi->num_points;j++) {
			if (ang[j] < ang[i]) {
				dx = ang[i];
				ang[i] = ang[j];
				ang[j] = dx;
				com = roi->thepnts[i];
				roi->thepnts[i] = roi->thepnts[j];
				roi->thepnts[j] = com;
			}
		}
	}
/* done */
	return;
}

#define TAG 99999

void reorder_roi_points(Trace *roi)
{
	Trace		tmp;
	long int	i,k,j;
	double		d,dd;
	
	tmp = *roi;
	tmp.thepnts[0].x = TAG;
	
	for(j=1;j<tmp.num_points;j++) {
		d = 1e+20;
		k = -1;
		for(i=0;i<tmp.num_points;i++) {
			if (tmp.thepnts[i].x != TAG) {
				dd = ((tmp.thepnts[i].x - roi->thepnts[j-1].x)*
						(tmp.thepnts[i].x - roi->thepnts[j-1].x));
				dd += ((tmp.thepnts[i].y - roi->thepnts[j-1].y)*
						(tmp.thepnts[i].y - roi->thepnts[j-1].y));
				if (dd < d) {
					d = dd;
					k = i;
				}
			}
		}
		roi->thepnts[j] = tmp.thepnts[k];
		tmp.thepnts[k].x = TAG;
	}
	
	return;
}

/* compute the minimum surface are triangulation between two contours
	using brute force methods */
void triangulate_rois(Trace *roi[2],double dz,long int *ntris,
	long int *vtex,long int *ctr)
{
	long int	i,j,k,jj;
	float		*dist[2];
	float		sum[2],a[3],b[3];
	double		MinSA,acc_top,acc_bot,D0,D1,SA;
	long int	top,top1,bot,bot1,count,scount;
	long int	v[MAX_POINTS*2*3],c[MAX_POINTS*2*3];

/* none at this time */
	*ntris = 0;

/* perimeter arrays */
	dist[0] = (float *)malloc(roi[0]->num_points*sizeof(float));
	if (dist[0] == 0L) return;
	dist[1] = (float *)malloc(roi[1]->num_points*sizeof(float));
	if (dist[1] == 0L) {
		free(dist[0]);
		return;
	}

/* compute perimeter distances */
	for(k=0;k<2;k++) {
	    sum[k] = 0;
	    for(i=0;i<roi[k]->num_points;i++) {
		j = i + 1;
		if (j == roi[k]->num_points) j = 0;
		dist[k][i] = (roi[k]->thepnts[i].x-roi[k]->thepnts[j].x)*
			(roi[k]->thepnts[i].x-roi[k]->thepnts[j].x);
		dist[k][i] += (roi[k]->thepnts[i].y-roi[k]->thepnts[j].y)*
			(roi[k]->thepnts[i].y-roi[k]->thepnts[j].y);
		dist[k][i] = sqrt(dist[k][i]);
		sum[k] += dist[k][i];
	    }
	}

/* find the best fit (min surface area) */
	MinSA = 1.0E+30;
/* try all initial tie points */
	for(jj=0;jj<roi[1]->num_points;jj++) {
		acc_top = 0;
		acc_bot = 0;
		top = 0;
		bot = jj;
		count = 0;
		scount = 0;
		while (scount < (roi[0]->num_points+roi[1]->num_points)) {
/* initial step */
			v[count] = top;
			c[count] = 0;
			count += 1;
			v[count] = bot;
			c[count] = 1;
			count += 1;

/* compute the next points */
			bot1 = bot + 1;
			if (bot1 >= roi[1]->num_points) bot1 = 0;
			top1 = top + 1;
			if (top1 >= roi[0]->num_points) top1 = 0;

/* compute distances */
			D0 = dist[0][top];
			D1 = dist[1][bot];

/* choose */
			if ((acc_bot+(D1/sum[1])) > (acc_top+(D0/sum[0]))) {
/* move top */
				v[count] = top1;
				c[count] = 0;
				count += 1;
				top = top1;
				acc_top += (D0/sum[0]);
			} else {
/* move bottom */
				v[count] = bot1;
				c[count] = 1;
				count += 1;
				bot = bot1;
				acc_bot += (D1/sum[1]);
			}
			scount += 1;
		}
/* compute the surface area */
		SA = 0.0;
		for(j=0;j<scount*3;j+=3) {
			a[0] = roi[c[j]]->thepnts[v[j]].x;
			a[0] -= roi[c[j+1]]->thepnts[v[j+1]].x;
			a[1] = roi[c[j]]->thepnts[v[j]].y;
			a[1] -= roi[c[j+1]]->thepnts[v[j+1]].y;
			a[2] = dz*(float)(c[j]-c[j+1]);

			b[0] = roi[c[j+2]]->thepnts[v[j+2]].x;
			b[0] -= roi[c[j+1]]->thepnts[v[j+1]].x;
			b[1] = roi[c[j+2]]->thepnts[v[j+2]].y;
			b[1] -= roi[c[j+1]]->thepnts[v[j+1]].y;
			b[2] = dz*(float)(c[j+2]-c[j+1]);

/* 1/2 the magnitude of the cross product */
			D0  = (a[1]*b[2]-a[2]*b[1])*(a[1]*b[2]-a[2]*b[1]);
			D0 += (a[0]*b[1]-a[1]*b[0])*(a[0]*b[1]-a[1]*b[0]);
			D0 += (a[2]*b[0]-a[0]*b[2])*(a[2]*b[0]-a[0]*b[2]);
			SA += 0.5*sqrt(D0);
		}

/* save the best fit (so far) */
		if (SA < MinSA) {
			MinSA = SA;
			for(j=0;j<scount*3;j++) {
				vtex[j] = v[j];
				ctr[j] = c[j];
			}
			*ntris = scount;
		}

	}

/* all done */
	free(dist[0]);
	free(dist[1]);

	return;
}
