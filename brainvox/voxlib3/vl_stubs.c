#include <voxel.h>

void VL_DIRTY(VLINT32 what, vset *set)
{
vl_dirty_(what, set);
}

void VL_INIT(vset *set)
{
vl_init_(set);
}

void VL_INIT_HW(void *datum, int flags, vset *set)
{
vl_init_hw_(datum, flags, set);
}

void VL_QUERY(int what, int *value, vset *set)
{
vl_query_(what,value,set);
}

void VL_FREE(int flags, vset *set)
{
vl_free_(flags, set);
}

void VL_DATA(VLINT32 dx,VLINT32 dy,VLINT32 dz,VLUCHAR *data,vset *set)
{
vl_data_(dx,dy,dz,data,set);
}

void VL_AUX_DATA(VLINT32 volnum,VLUCHAR *data,vset *set)
{
vl_aux_data_(volnum,data,set);
}

void VL_LIMITS(VLINT32 xs, VLINT32 xe, VLINT32 ys, VLINT32 ye, VLINT32 zs,VLINT32 ze, vset *set)
{
vl_limits_(xs, xe, ys, ye, zs, ze, set);
}

void VL_ROTATIONS(VLFLOAT64 xr, VLFLOAT64 yr, VLFLOAT64 zr, vset *set)
{
vl_rotations_(xr, yr, zr, set);
}

void VL_ROTS(VLINT32 xi, VLINT32 yi, VLINT32 zi, vset *set)
{
vl_rots_(xi, yi, zi, set);
}

void VL_ARB_ROTATE(VLFLOAT64 *angle, VLFLOAT64 *axis, vset *set)
{
vl_arb_rotate_(angle, axis, set);
}

void VL_TRANSLATIONS(VLFLOAT64 tx, VLFLOAT64 ty, VLFLOAT64 tz, vset *set)
{
vl_translations_(tx, ty, tz, set);
}

void VL_TRANS(VLINT32 ix, VLINT32 iy, VLINT32 iz, vset *set)
{
vl_trans_(ix, iy, iz, set);
}

void VL_SPEED(VLINT32 speed, vset *set)
{
vl_speed_(speed, set);
}

void VL_IMAGE(VLINT32 imagex,VLINT32 imagey,VLUCHAR *image,VLINT16 *zbuf,vset *set)
{
vl_image_(imagex,imagey,image,zbuf,set);
}

void VL_PICKBUFFER(VLUINT32 * pbuffer , vset * set)
{
vl_pickbuffer_(pbuffer, set);
}

void VL_PLANE(VLINT32 toggle, VLFLOAT64 *point, VLFLOAT64 *normal, vset *set)
{
vl_plane_(toggle, point, normal, set);
}

void VL_LOOKUPS(VLINT32 *rlut,VLINT32 *llut, VLINT32 background, vset *set)
{
vl_lookups_(rlut,llut, background, set);
}

void VL_RENDER(VLINT32 flags, VLINT32 interrpt, VLINT32 *ret, vset *set)
{
vl_render_(flags, interrpt, ret, set);
}

void VL_FILTER_OUTPUT(VLINT32 *image,VLINT32 dx,VLINT32 dy,VLINT32 filter)
{
vl_filter_output_(image,dx,dy,filter);
}

void VL_POSTLIGHT(VLINT32 *image,VLINT16 *zbuf,VLINT32 *out,VLINT32 imagex,VLINT32 imagey,VLFLOAT64 *norm)
{
vl_postlight_(image,zbuf,out,imagex,imagey,norm);
}

void VL_SET_LIGHT_PARAM(VLINT32 param, VLFLOAT64 *value)
{
vl_set_light_param_(param, value);
}

void VL_ADDHAZE(VLINT32 *image,VLINT16 *zbuf,VLINT32 *out,VLINT32 imagex,VLINT32 imagey,VLFLOAT64 *w) 
{
vl_addhaze_(image,zbuf,out,imagex,imagey,w);
}

void VL_ADDSHADOW(VLFLOAT64 di,VLFLOAT64 li,VLFLOAT64 *norm,VLINT32 filter, vset *set,VLINT32 backdrops)
{
vl_addshadow_(di,li,norm,filter,set,backdrops);
}

void VL_READ_IMAGES( VLCHAR *temp,VLINT32 x,VLINT32 y, VLINT32 st, VLINT32 ed,VLINT32 step,VLINT32 interp,VLUCHAR *data)
{
vl_read_images_(temp,x,y,st,ed,step,interp,data);
}

void VL_READ_IMAGES_H( VLCHAR *temp,VLINT32 x,VLINT32 y, VLINT32 st, VLINT32 ed,VLINT32 step,VLINT32 interp,VLUCHAR *data, VLINT32 header)
{
vl_read_images_h_(temp,x,y,st,ed,step,interp,data,header);
}

void VL_READ_PAL(VLCHAR *fname,VLUCHAR *data)
{
void vl_read_pal_(fname,data);
}

void VL_INTERP_IMAGES(VLUCHAR *start,VLUCHAR *end,VLINT32 dx,VLINT32 dy,VLINT32 n, VLUCHAR *out)
{
vl_interp_images_(start,end,dx,dy,n,out);
}

void VL_DRAWCUBE(vset *set)
{
vl_drawcube_(set);
}

void VL_GETMATRIX(VLFLOAT32 *mat, VLFLOAT32 *inv, vset *set)
{
vl_getmatrix_(mat,inv,set);
}

void VL_SETSQU(VLFLOAT32 *squ, vset *set)
{
vl_setsqu_(squ,set);
}

void VL_GETSQU(VLFLOAT32 *squ, vset *set)
{
vl_getsqu_(squ, set);
}

void VL_SETINTERP(VLINT32 interp, vset *set)
{
vl_setinterp_(interp,set);
}

VLINT32 VL_VOLUME2IMAGE(vset *set, VLINT32 vx, VLINT32 vy, VLINT32 vz, VLINT32 *ix, VLINT32 *iy, VLINT32 clip, VLINT32 *zvalue )
{
return(vl_volume2image_(set,vx,vy,vz,ix,iy,clip,zvalue));
}

VLINT32 VL_IMAGE2VOLUME( vset *set, VLINT32 ix, VLINT32 iy, VLINT32 *vx, VLINT32 *vy, VLINT32 *vz) {
return(vl_image2volume_(set,ix,iy,vx,vy,vz));
}

void VL_M_RENDER(VLINT32 nprocs,VLINT32 flags, VLINT32 interrpt, VLINT32 *ret, vset *set,VLINT32 *rect) {
vl_m_render_(nprocs,flags,interrpt,ret,set,rect);
}

void VL_M_NCPUS(VLINT32 *ncpus,VLINT32 *err) 
{
vl_m_ncpus_(ncpus,err);
}
