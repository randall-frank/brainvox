/* prototypes for the C entry points into the Voxel lib */

#ifndef __VOXEL_H__
#define __VOXEL_H__

#ifdef __cplusplus
extern "C" {
#endif

#define VL_MAX_VOLS	4   /* maximum number of volumes in a vset */

typedef struct voxstruct
	{
		/* first the voxel data set */
		long int d[3];  /* sizes of the major axis (dx,dy,dz) */

		/* pointer to the actual volume data this value is the same
			 as aux_data[0] */
		unsigned char *data;  

		/* pointers to volume datasets */
		unsigned char *aux_data[VL_MAX_VOLS];  

		/* next the portion of the set to use */
		long int start[3],end[3];

		/* now the position of the cube in space */
		double rots[3],trans[3];

		/* the resolution of the rendering */
		long int speed;

		/* the output frame buffer */
		long int imagex,imagey;
		unsigned char *image;  /* image is ix*iy*4bytes */
		short *zbuffer;  /* zbuffer is ix*iy*2bytes */
		/* note: the frame buffer is assumed to exist from
		-imagex/2 to (+imagex/2)-1 and likewise in the y */
		
		/* A possible cutting plane in space */
		long int plane;  /* 0=off !0 = on */
		double planept[3],planevec[3];

		/* the lookup tables:  aabbggrr[0-255] */
		/* rlut of one side of the plane llut on the other */
		long int rlut[256],llut[256];
		
		/* the projection matrix */
		double xform[3][3];

		/* background color $00bbggrr */
		long int	backpack;

		float	squeeze_factor;
} vset;

void vl_init_(vset *set);
void vl_data_(long int dx,long int dy,long int dz,unsigned char *data,vset *set);
void vl_aux_data_(long int volnum,unsigned char *data,vset *set);
void vl_limits_(long int xs, long int xe, long int ys, long int ye, long int zs,
	 long int ze, vset *set);
void vl_rotations_(double xr, double yr, double zr, vset *set);
void vl_rots_(long int xi, long int yi, long int zi, vset *set);
void vl_arb_rotate_(double *angle, double *axis, vset *set);
void vl_translations_(double tx, double ty, double tz, vset *set);
void vl_trans_(long int ix, long int iy, long int iz, vset *set);
void vl_speed_(long int speed, vset *set);
void vl_image_(long int imagex,long int imagey,unsigned char *image,
	 short *zbuf,vset *set);
void vl_plane_(long int toggle, double *point, double *normal, vset *set);
void vl_lookups_(long int *rlut,long int *llut, long int background, vset *set);
void vl_render_(long int flags, long int interrpt, long int *ret, vset *set);
/* dithering routines */
void vl_makeitable_(unsigned char *cmap, long int ncols, long int base,
        long int ntab, unsigned char **outtab,long int *tabsize);
void vl_dither_(long int *image,long int dx,long int dy,unsigned char *output,
        unsigned char *lut,unsigned char *itab,long int type);
/* filtering routines */
void vl_filter_output_(long int *image,long int dx,long int dy,long int filter);
/* lighting routines */
void vl_postlight_(long int *image,short *zbuf,long int *out,
                        long int imagex,long int imagey,double *norm);
void vl_set_light_param_(long int param, double *value);
void vl_addhaze_(long int *image,short *zbuf,long int *out,
                        long int imagex,long int imagey,double *w);
void vl_addshadow_(double di,double li,double *norm,long int filter,vset *set,
        long int backdrops);

/* rendering modes */
#define VL_FOREGROUND 0
#define VL_BACKGROUND 1
#define VL_RENDERPLANE 1   /* USED INTERNALLY ONLY */
#define VL_RENDEROVER 2
#define VL_RENDERLIGHT 4
#define VL_RENDERZCLIP 8
#define VL_RENDER16BIT 16  /* supported by vl_m_render_ only */
#define VL_RENDER24BIT 32  /* supported by vl_m_render_ only */

#define VL_INFINITY -32700  /* value to prime a Zbuffer to inf with */
#define VL_OUTTHERE -3600    /* value beyond which infinity can be assumed */

#define VL_NOERR 0          /* normal exit, no errors */
#define VL_MEMERR 1         /* procedure could not allocate memory */
#define VL_IMGERR 2         /* image buffer did not match window size */
#define VL_ABORTED 4        /* rendering aborted by user */


/* filter types */
#define VL_MEDIAN 10
#define VL_AVERAGE 11
#define VL_ZAVERAGE 15
#define VL_ZMEDIAN 16

/* dithering types */
#define VL_GRAY 12
#define VL_COLOR 13
#define VL_FIXEDCOLOR 14
#define VL_COLORLOOKUP 15

/* lighting parameters */
#define VL_JITTER 17
#define VL_MIN_INTEN 18
#define VL_MAX_INTEN 19
#define VL_ZSTRETCH 20
#define VL_RESET_LIGHTS 21
#define VL_ILIGHT_DX 22
#define VL_ILIGHT_DY 23
#define VL_ILIGHT_DZ 24
#define VL_ILIGHT_INTEN 25
#define VL_ILIGHT_SRCINTEN 26
#define VL_ILIGHT_RESET 27
#define VL_HAZE_RESET 28
#define VL_HAZE_NEAR 29
#define VL_HAZE_FAR 30

/* prototype for user utility routines */
void vl_name_changer(char *in, long int num, long int *error, char *out);
void vl_read_images_( char *temp,long int x,long int y, long int st,long int ed,
	long int step,long int interp,unsigned char *data);
void vl_read_images_h_( char *temp,long int x,long int y, long int st,
	long int ed,long int step,long int interp,unsigned char *data,
	long int header);
void vl_read_pal_(char *fname,unsigned char *data);
void vl_interp_images_(unsigned char *start,unsigned char *end,long int dx,
	 long int dy,long int n, unsigned char *out);
void vl_drawcube_(vset *set);
void vl_getmatrix_(float *mat, float *inv, vset *set);
void vl_setsqu_(float *squ, vset *set);
void vl_getsqu_(float *squ, vset *set);

/* prototypes for user supplied callback routines */
long int vl_rcallback(long type,vset *set);
void vl_puts(char *text);

/* multi-cpu routines */
void vl_m_render_(long int nprocs,long int flags, long int interrpt,
	long int *ret, vset *set,long int *rect);
void vl_m_ncpus_(long int *ncpus,long int *err);

#ifdef __cplusplus
}
#endif

#endif
