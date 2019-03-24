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
        long int dx,long int dy,long int count,double ipixel,double islice,
        double *sa,double *vol);
long int read_pset(char *file,Pset *set);
long int write_pset(char *file,Pset *set);
void get_xyz(long int *xyz,char *str);
void get_xy(long int *xy,char *str);
void bin_roi_flip(long int xflip);
void bin_roi_scale(double scale);

void    bswap(unsigned char *data,long int dx,long int dy,long int dz);
void    yflip(unsigned char *data,long int dx,long int dy,long int dz);
void    xflip(unsigned char *data,long int dx,long int dy,long int dz);

#ifdef __cplusplus
}
#endif

#endif
