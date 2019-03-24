

/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski
 *
 * $Id: roi_utils.h 1250 2005-09-16 15:51:42Z dforeman $
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

#define MAX_POINTS	10000

typedef struct {
        int x;
        int y;
} Point;

typedef struct {
        char filename[256];
        char name[80];
        int  num_points;
        Point thepnts[MAX_POINTS];
        int  visible;
        int  dirty;
        int  template;
} Trace;

void    init_roi_utils(void);
void    roi_set_image(long int dx,long int dy,unsigned char *in,
		unsigned char *in2);
void    roi_set_data_rep(double *rep);
long int read_roi(char *file,Trace *roi);
long int save_roi(char *file,Trace *roi);

long	clipped_point_img(long int x,long int y);
void	clip_point_img(long int *x,long int *y);
void 	init_roi(Trace *roi,char *name);
void   	roi_bounds(Trace *roi,Point *min,Point *max);
void   	roi_centroid(Trace *roi,Point *com);
double 	roi_area(Trace *roi);
double 	roi_perimeter(Trace *roi);
double 	roi_length(Trace *roi);
double 	roi_angle(Trace *roi);
void 	roi_axis(Trace *roi,double *maj,double *min);
void 	roi_axis_points(Trace *roi,double *maj,double *min,Point *ma,Point *mn);
void	fill_roi(Trace *roi,long int in,long int out);
void 	roi_data_rep(Trace *roi,double *min,double *max,double *mean,
	   double *stdev,double *pixels,double *sum,double *comx,double *comy);
void 	thin_roi(Trace *roi);
long int closed_roi(Trace *roi);
void 	edit_roi(Trace *roi,Point *edits);
void 	rotate_roi(Trace *roi,Point *edits);
void 	rotate_roi_ang(Trace *roi,double ang,long int x,long int y);
void 	move_roi(Trace *roi,long int dx, long int dy);
void    scale_roi(Trace *roi,double zoom);
void 	addpnt_roi(Trace *roi,long int x,long int y);
void    roi_2_strips(Trace *roi,long int *strips,long int *npts);
void    interp_roi(Trace *roi,long int n);
void    smooth_roi(Trace *roi,long int n);
void    sample_roi(Trace *roi,long int n);
void	roi_CCW(Trace *roi);
void 	reorder_roi_points(Trace *roi);
void 	triangulate_rois(Trace *roi[2],double dz,long int *ntris,
        	long int *vtex,long int *ctr);
