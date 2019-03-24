
/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski
 *
 * $Id: proto.h 1889 2006-08-20 16:10:17Z rjfrank $
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

#define TAL_VERSION "1.53"

/* proto.h */

typedef struct {
	double	points[3][3];
	double	CA_front,CP_back,CA_CP;
	double	CA_top,CA_bottom;
	double	CA_right,CA_left;
	double	xaxis[3];
	double	yaxis[3];
	double	zaxis[3];
} tal_conv;

/*
C       Defines for the size of the atlas brain (ISBN 3-13-711701-1)
C       Talairach/Tournoux: Co-Planar Stereotaxic Atlas of the Human Brain
C       Georg Thieme Verlag Stuttgart, New York 1988
*/

#define TAL_FRONT  70.0
#define TAL_MIDDLE  23.5
#define TAL_BACK  80.0
/*
       0 to Back distance is really 103.5  (80+23.5=103.5)
*/
#define TAL_RIGHT  68.5
#define TAL_LEFT  68.5
#define TAL_TOP  74.5
#define TAL_BOTTOM  44.0

#define TOLERANCE  5.0
#define PT_CA  1
#define PT_CP  2
#define PT_PLANE  3


#define _MAX_POINTS      8192

typedef struct {
        long int        x[_MAX_POINTS];
        long int        y[_MAX_POINTS];
        long int        z[_MAX_POINTS];
        long int        c[_MAX_POINTS];
        long int        n;
        char            title[256];
} Pset;

#ifndef __BIN_IO_H__
#define __BIN_IO_H__

#ifdef __cplusplus
extern "C" {
#endif

void ex_err(char *s);
void cmd_err(char *s);
void tal_read(char *file,double ip,tal_conv *tal);
long int bin_io(char *file,char type,void *data,long int dx,long int dy,
	long int dz,long int flip,long int swab,long int header);
void name_changer(char *in, long int num, long int *error, char *out);
void tal_2_mm(double *tin,double *pmm,tal_conv *tal);
void img_interpolate(void  *st,void *en,void *out,long int dx,long int dy,
        long int dz,long int n);
void size_bin_file_(char *file,long int *bytes,long int *err);
long int slice_isocontour(unsigned char *data,unsigned char *mask,
        long int dx,long int dy,long int count,double *ipixel,double islice,
        double *sa,double *vol);
long int vol_isocontour(void *data,unsigned char *mask,long int dx,long int dy,
        long int dz,long int depth,double thres,double *ipixel,double islice,
	double *sa,long int *n_verts,float **verts,float **norms,
        long int *n_tris,long int **tris);
long int read_pset(char *file,Pset *set);
long int write_pset(char *file,Pset *set);
void get_xyz(long int *xyz,char *str);
void get_xy(long int *xy,char *str);
void get_d_xy(double *xy,char *str);
void get_d_xyz(double *xyz,char *str);
void bin_roi_flip(long int xflip);
void bin_roi_scale(double scale);

void    bswap(unsigned char *data,long int dx,long int dy,long int dz);
void    yflip(unsigned char *data,long int dx,long int dy,long int dz);
void    xflip(unsigned char *data,long int dx,long int dy,long int dz);

void 	tal_exit(int i);

/* macros....c */
long int C_parse_math(char *out);

#ifdef __cplusplus
}
#endif

#endif
