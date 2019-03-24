
/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski
 *
 * $Id: autotrace.h 1250 2005-09-16 15:51:42Z dforeman $
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

typedef struct {
        unsigned char   *data;
        long int        d[3];
        long int        start[3],end[3];
} Dvol;

typedef struct {
        short 	x;
        short 	y;
        short 	z;
        short 	col;
} Point3D;

/* routines for implementing the volumetric dataset segmenting scheme */
/* which reserves the high order bit for a connectivity tag boolean */

/* generally visible routines */
/* prepare the volume by dividing it by 2 with optional histogram computation *//* fills in hist[0-255] with a histogram of values if hist != 0L */
void volume_shift(Dvol *set,long int *hist);

/* clear all the high oder bits in the volume */
void volume_clear(Dvol *set,long int *hist);

/* recompute the volume histogram */
void volume_hist(Dvol *set,long int *hist);

/* perform 3D flood fill from a set of preselected seeds */
void volume_autotrace(Dvol *set,long int low,long int high,
        Point3D *seeds,long int nseeds,long int *count);

/* Set the high order bits in a cubic region around a list of points */
void volume_tag(Dvol *set,Point3D *seeds,long int npts,long int t_size,
        long int i);

/* perform 2D isovalue contour following */
void autotrace_2d(unsigned char *image,long int dx,long int dy,long int dz,
	long int val,long int sx,long int sy,long int *out,long int *num);

/* message printing function */
void    vl_puts(char *);

/* image grid access function */
long int grid(long int i,long int j,long int dx,long int dy,long int dz,
        unsigned char *image);
