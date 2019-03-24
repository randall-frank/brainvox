/* prototypes for the C entry points into the Voxel lib */
/* Windows NT version -- uses IAFdefs.h  */
#ifndef __VOXEL_H__
#define __VOXEL_H__

#include <iafdefs.h>

#ifdef __cplusplus
extern "C" {
#endif

#define VL_MAX_VOLS	4   /* maximum number of volumes in a vset */

typedef struct voxstruct
	{
		/* first the voxel data set */
		VLINT32 d[3];  /* sizes of the major axis (dx,dy,dz) */

		/* pointer to the actual volume data this value is the same
			 as aux_data[0] */
		VLUCHAR *data;  

		/* pointers to volume datasets */
		VLUCHAR *aux_data[VL_MAX_VOLS];  

		/* next the portion of the set to use */
		VLINT32 start[3],end[3];

		/* now the position of the cube in space */
		VLFLOAT64 rots[3],trans[3];

		/* the resolution of the rendering */
		VLINT32 speed;

		/* the output frame buffer */
		VLINT32 imagex,imagey;
		VLUCHAR *image;  /* image is ix*iy*4bytes */
		VLINT16 *zbuffer;  /* zbuffer is ix*iy*2bytes */
		/* note: the frame buffer is assumed to exist from
		-imagex/2 to (+imagex/2)-1 and likewise in the y */
		VLUINT32 *pickbuffer;

		/* A possible cutting plane in space */
		VLINT32 plane;  /* 0=off !0 = on */
		VLFLOAT64 planept[3],planevec[3];

		/* the lookup tables:  aabbggrr[0-255] */
		/* rlut of one side of the plane llut on the other */
		VLINT32 rlut[256],llut[256];
		
		/* the projection matrix */
		VLFLOAT64 xform[3][3];

		/* background color $00bbggrr */
		VLINT32		backpack;

		/* squeeze factor */
		VLFLOAT32	squeeze_factor;
                VLINT32		num_interp;

		/* volume dirty bits */
		VLINT32		dirty_flags;

                /* HW volume rendering bits */
                VLPVOID         hw;   /* actually a *hw_vset */

} vset;

void vl_dirty_(VLINT32 what, vset *set);
void vl_init_(vset *set);
void vl_init_hw_(void *datum, int flags, vset *set);
void vl_query_(int what, int *value, vset *set);
void vl_free_(int flags, vset *set);
void vl_data_(VLINT32 dx,VLINT32 dy,VLINT32 dz,VLUCHAR *data,vset *set);
void vl_aux_data_(VLINT32 volnum,VLUCHAR *data,vset *set);
void vl_limits_(VLINT32 xs, VLINT32 xe, VLINT32 ys, VLINT32 ye, VLINT32 zs,
	 VLINT32 ze, vset *set);
void vl_rotations_(VLFLOAT64 xr, VLFLOAT64 yr, VLFLOAT64 zr, vset *set);
void vl_rots_(VLINT32 xi, VLINT32 yi, VLINT32 zi, vset *set);
void vl_arb_rotate_(VLFLOAT64 *angle, VLFLOAT64 *axis, vset *set);
void vl_translations_(VLFLOAT64 tx, VLFLOAT64 ty, VLFLOAT64 tz, vset *set);
void vl_trans_(VLINT32 ix, VLINT32 iy, VLINT32 iz, vset *set);
void vl_speed_(VLINT32 speed, vset *set);
void vl_image_(VLINT32 imagex,VLINT32 imagey,VLUCHAR *image,
	 VLINT16 *zbuf,vset *set);
void vl_pickbuffer_(VLUINT32 * pbuffer , vset * set);
void vl_plane_(VLINT32 toggle, VLFLOAT64 *point, VLFLOAT64 *normal, vset *set);
void vl_lookups_(VLINT32 *rlut,VLINT32 *llut, VLINT32 background, vset *set);
void vl_render_(VLINT32 flags, VLINT32 interrpt, VLINT32 *ret, vset *set);

/* dithering routines */
void vl_makeitable_(VLUCHAR *cmap, VLINT32 ncols, VLINT32 base,
        VLINT32 ntab, VLUCHAR **outtab,VLINT32 *tabsize);
void vl_makeitable_ex_(VLUCHAR *cmap, 
		   VLUCHAR *mask,
		   VLINT32 ncols,
		   VLINT32 base,
       		   VLINT32 ntab, 
       		   VLUCHAR **outtab,
       		   VLINT32 *tabsize);

void vl_dither_(VLINT32 *image,VLINT32 dx,VLINT32 dy,VLUCHAR *output,
        VLUCHAR *lut,VLUCHAR *itab,VLINT32 type);
void vl_dither_setclamp( VLINT32 value );

/* filtering routines */
void vl_filter_output_(VLINT32 *image,VLINT32 dx,VLINT32 dy,VLINT32 filter);
/* lighting routines */
void vl_postlight_(VLINT32 *image,VLINT16 *zbuf,VLINT32 *out,
                        VLINT32 imagex,VLINT32 imagey,VLFLOAT64 *norm);
void vl_set_light_param_(VLINT32 param, VLFLOAT64 *value);
void vl_addhaze_(VLINT32 *image,VLINT16 *zbuf,VLINT32 *out,
                        VLINT32 imagex,VLINT32 imagey,VLFLOAT64 *w);
void vl_addshadow_(VLFLOAT64 di,VLFLOAT64 li,VLFLOAT64 *norm,VLINT32 filter,
                   vset *set,VLINT32 backdrops);

/* rendering modes */
#define VL_FOREGROUND 0
#define VL_BACKGROUND 1
#define VL_RENDERPLANE 1   /* USED INTERNALLY ONLY */
#define VL_RENDEROVER 2
#define VL_RENDERLIGHT 4
#define VL_RENDERZCLIP 8
#define VL_RENDER16BIT 16  /* supported by vl_m_render_ only */
#define VL_RENDER24BIT 32  /* supported by vl_m_render_ only */
#define VL_RENDERMIP   64  
#define VL_RENDERMNIP  128  /* basically obsolete */
#define VL_RENDERMIP24  (VL_RENDER24BIT | VL_RENDERMIP)  
#define VL_RENDERYIQ  	256 
#define VL_RENDERMIPYIQ	(VL_RENDERYIQ | VL_RENDERMIP)
#define VL_RENDER3DTEX	512

/* flags to vl_init_hw_  & vl_free_ */
#define VL_PBUFFER 1

/* query values */
#define VL_QUERY_HW      0
#define VL_QUERY_HW_TYPE 1

/* vl_dirty flags */
#define VL_DIRTY_NONE    0
#define VL_DIRTY_PALETTE 1
#define VL_DIRTY_VOLUME  2
#define VL_DIRTY_IMAGE   4
#define VL_DIRTY_ALL     7

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
#define VL_ZSLICE_MASK 31

/* prototype for user utility routines */
void vl_name_changer(VLCHAR *in, VLINT32 num, VLINT32 *error, VLCHAR *out);
void vl_read_images_( VLCHAR *temp,VLINT32 x,VLINT32 y, VLINT32 st,VLINT32 ed,
	VLINT32 step,VLINT32 interp,VLUCHAR *data);
void vl_read_images_h_( VLCHAR *temp,VLINT32 x,VLINT32 y, VLINT32 st,
	VLINT32 ed,VLINT32 step,VLINT32 interp,VLUCHAR *data,
	VLINT32 header);
void vl_read_pal_(VLCHAR *fname,VLUCHAR *data);
void vl_interp_images_(VLUCHAR *start,VLUCHAR *end,VLINT32 dx,
	 VLINT32 dy,VLINT32 n, VLUCHAR *out);
void vl_drawcube_(vset *set);
void vl_getmatrix_(VLFLOAT32 *mat, VLFLOAT32 *inv, vset *set);
void vl_setsqu_(VLFLOAT32 *squ, vset *set);
void vl_getsqu_(VLFLOAT32 *squ, vset *set);
void vl_setinterp_(VLINT32 interp, vset *set);

/* image to volume and revers */
VLINT32 vl_volume2image_(vset *set,
			VLINT32 vx, VLINT32 vy, VLINT32 vz,
			VLINT32 *ix, VLINT32 *iy, 
			VLINT32 clip, VLINT32 *zvalue );
VLINT32 vl_image2volume_( vset *set,
			  VLINT32 ix, VLINT32 iy, 
			  VLINT32 *vx, VLINT32 *vy, VLINT32 *vz);

/* prototypes for user supplied callback routines */
VLINT32 vl_rcallback(VLINT32 type,VLINT32 inter,vset *set);
void vl_puts(VLCHAR *text);

/* multi-cpu routines */
void vl_m_render_(VLINT32 nprocs,VLINT32 flags, VLINT32 interrpt,
	VLINT32 *ret, vset *set,VLINT32 *rect);
void vl_m_ncpus_(VLINT32 *ncpus,VLINT32 *err);

#ifdef __cplusplus
}
#endif

#endif
