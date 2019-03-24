#ifndef _SCRIPT_H_
#define _SCRIPT_H_

void execute_command_(char *inbuff, int len);
void continue_command_(vset *set, int *more, 
                       int *cmd, float *arr, char *name, int len);

/* nothing to do */
#define SCR_CMD_NONE 0
/* rots */
#define SCR_CMD_ROT 1

/* color in dset */
#define SCR_CMD_COLOR 2
/* palettes in dset */
#define SCR_CMD_PAL 3

/* filtered image in dset */
#define SCR_CMD_FILTER 4
/* render */
#define SCR_CMD_RENDER 5
/* hwrender on/off */
#define SCR_CMD_HWRENDER 9
/* autorender on/off */
#define SCR_CMD_AUTORENDER 10
/* texrender on/off */
#define SCR_CMD_TEXRENDER 11

/* filename */
#define SCR_CMD_SAVE 6
/* filename, which,interp,flags,min,max,min,max */
#define SCR_CMD_DATA 7

/* 6 floats (on),dx,dy,dz,src,light */
#define SCR_CMD_LIGHTING 8

/* recompute volume bounds */
#define SCR_CMD_REBOUND 12

#endif
