/*
======================================================================
Copyright (C) 1994. All Rights Reserved. 
Image Analysis Facility, University of Iowa

THIS SOFTWARE IS PROPERTY OF:
Image Analysis Facility
The University of Iowa, Iowa City, Iowa

-- OVERVIEW --------------

MODULE NAME: voxlib
FILE NAME: render.c
DATE:  
AUTHOR(S): 

========================================================================
PROJECT: vblast
STATUS: Development
PLATFORM: UNIX/IrisGL 
LANGUAGE: C
DESCRIPTION: Simple IrisGL viewer attached to voxlib
DEPENDENCIES: 
LIMITATIONS:
NOTES:
	Example of integrated GL display...


-- REVISION HISTORY --------------
DATE:
REASON:
REQUESTED BY:
AUTHOR:

-- GENERAL --------------



 ======================================================================
*/


#include <stdio.h>
#include <malloc.h>
#include <gl.h>
#include <device.h>
#include <sys/types.h>
#include <sys/time.h>
#include <math.h>

#include "voxel.h"

float p1[3] = {-180., 180., 0.00};
float p2[3] = { 180., 180., 0.00};
float p3[3] = { 180.,-180., 0.00};
float p4[3] = {-180.,-180., 0.00};

double	xaxis[3] = {1.0,0.0,0.0};
double	yaxis[3] = {0.0,1.0,0.0};

vset 	set;

void    loadset(char *fname,long int *lims);

void render(char *filename,char *lutf,char *opacityf,char *lutf2, 
	char *opacityf2) 
{
/* timing stuff */
	struct timeval before,after;
	struct timezone zone;
	float time1,time2;

/* GL graphics stuff */
	int wid;
	int dev;
	short val;

/* simple variables */
	long int	lims[6];
	int	dx,dy,dz;
	long 	a,b;
	int 	dxy;
	long 	rlut[256],llut[256];
	unsigned char lut[768],opacity[768],lut2[768],opacity2[768];
	long 	i,cont;
	double 	rx,ry,rz;
	long 	mx,my,wx,wy;
	int 	speed;
	int 	imagex,imagey;
	unsigned char *image;
	long 	*zimage;
	short 	*zbuff;
	int 	dirty,mode;
	int 	plane;

	float 	rots[3];
	float 	dist;
	double	norm[3];

	double n[3],p[3];

/* plane parallel to the screen at depth 0 */
	n[0] = 1;
	n[1] = 1;
	n[2] = -1;
        p[0] = 0;
        p[1] = 0;
        p[2] = 0;
	plane = 0;

/* window size */
	imagex = 512;
	imagey = 512;

/* read lookup tables from disk and */
/* build rlut and llut */
	(void) vl_read_pal_(opacityf,opacity);
	(void) vl_read_pal_(lutf,lut);
	(void) vl_read_pal_(opacityf2,opacity2);
	(void) vl_read_pal_(lutf2,lut2);
	for (i=0; i<256; i++ ) {
/* now the rlut */
		rlut[i] = 1L*lut[i] + 256L*lut[i+256] + 65536L*lut[i+512];
		rlut[i] = rlut[i] | (opacity[i] << 24);
/* and the llut */
		llut[i] = 1L*lut2[i] + 256L*lut2[i+256] + 65536L*lut2[i+512];
		llut[i] = llut[i] | (opacity2[i] << 24);
	};

/* dxy is a slice size IE there are dz planes of data or size dxy */

	(void) prefsize(imagex,imagey);
	wid = winopen("Voxel image");
	(void) singlebuffer();
	(void) RGBmode();
	(void) gconfig();
	(void) lsetdepth(0L,512L);

	(void) qdevice(WINQUIT);
	(void) qdevice(F1KEY); 
	(void) qdevice(F2KEY); 
	(void) qdevice(F3KEY); 
	(void) qdevice(F4KEY); 
	(void) qdevice(LEFTMOUSE);
	(void) qdevice(MIDDLEMOUSE);
	(void) qdevice(RIGHTMOUSE);
	(void) qenter(REDRAW,wid);
	
	(void) vl_init_(&set);
	loadset(filename,lims);
	dx = set.d[0];
	dy = set.d[1];
	dz = set.d[2];

	image = malloc(sizeof(int)*imagex*imagey);
	if (image == 0) {
		printf("Unable to allocate memory for buffer saving\n");
		exit(1);
	};

	zimage = malloc(sizeof(int)*imagex*imagey);
	if (zimage == 0) {
		printf("Unable to allocate memory for buffer saving\n");
		exit(1);
	};
	for(i=0;i<imagex*imagey;i++) zimage[i] = 10000;

	zbuff = malloc(sizeof(short)*imagex*imagey);
	if (zbuff == 0) {
		printf("Unable to allocate memory for buffer saving\n");
		exit(1);
	};

	(void) vl_image_(imagex,imagey,image,zbuff,&set);
	
	mode = 0;
	rx = 0.0;
	ry = 0.0;
	rz = 0.0;
	speed = 1;
	dirty = TRUE;

/* start an IrisGL event loop */
	cont = 1;
	while (cont) {
		dev = qread(&val);
		switch(dev)
		{
		case INPUTCHANGE:
			if (val != 0) winset(val);
			break;

	        case REDRAW:
			if (val == wid ) {
			winset(wid);
			(void) reshapeviewport();
			(void) vl_translations_(0.0,0.0,0.0,&set);
			(void) vl_speed_(speed,&set);
			(void) vl_lookups_(rlut,llut,0x00000000,&set);
			(void) vl_plane_(plane,p,n,&set);
			if (dirty) {
/* new rendering */
				(void) gettimeofday(&before,&zone);
				(void) vl_render_(mode,MIDDLEMOUSE,&i,&set);
				(void) gettimeofday(&after,&zone);
				(void) zbuffer(FALSE);
				if (i) {
					printf("Render returned code:%d\n",i);
					qreset();
				};
		time1 = ((float)before.tv_usec/1000000.0);
		time2 = ((float)after.tv_usec/1000000.0);
		time2 = time2 + (float)(after.tv_sec - before.tv_sec);
		printf("%d voxels in %f seconds = %f voxels per sec\n",
			dx*dy*dz,time2-time1,
			((float)(dx*dy*dz))/(time2-time1));
				dirty = FALSE;
				}
/* plain redraw */
				(void) zbuffer(FALSE);
				(void) lrectwrite(0,0,imagex-1,imagey-1,
					(unsigned long *)(set.image));
			};
			break;
		case WINQUIT :
			cont = 0;
			break;
		case RIGHTMOUSE :
			if (val == 1) {
				(void) getorigin(&wx,&wy);
				mx = getvaluator(MOUSEX) - wx;
				my = getvaluator(MOUSEY) - wy;
				ry = ((mx-256)/256.0)*1800.0;
				rx = ((my-256)/256.0)*1800.0;
				if (getbutton(RIGHTMOUSE)) {
        				(void) getsize(&a,&b);
        (void) ortho(-(a/2.0),(a/2.0),-(b/2.0),(b/2.0),-(a/2.0),(a/2.0));
					lsetdepth(0,a*8);
				};
				for(i=0;i<imagex*imagey;i++) {
					if (zbuff[i] <= VL_INFINITY) {
						zimage[i] = 0x007fffff;
					} else {
						zimage[i] = ((a/2)*8)-zbuff[i];
					}
				}
/* animate while button is down */
				a = mx;
				b = my;
				while (getbutton(RIGHTMOUSE)) {
					mx = getvaluator(MOUSEX) - wx;
					my = getvaluator(MOUSEY) - wy;
					ry = ((mx-256)/256.0)*1800.0;
					rx = ((my-256)/256.0)*1800.0;
/* redraw only if needed */
					if ((a != mx) || (b != my)) {
						a = mx;
						b = my;
						zbuffer(FALSE);
						zdraw(TRUE);
						frontbuffer(FALSE);
				(void) lrectwrite(0,0,imagex-1,imagey-1,
					(unsigned long *)(zimage));
						zdraw(FALSE);
						frontbuffer(TRUE);
				(void) lrectwrite(0,0,imagex-1,imagey-1,
					(unsigned long *)(set.image));
						zbuffer(TRUE);
						cpack(0xa0303030);
						pushmatrix();
						rotate((float)rx,'x');
						rotate((float)ry,'y');
						bgnpolygon();
						v3f(p1);
						v3f(p2);
						v3f(p3);
						v3f(p4);
						endpolygon();
						popmatrix();
						zbuffer(FALSE);
					}

				}
			};
			break;
		case LEFTMOUSE :
			if (val == 1) {
				(void) getorigin(&wx,&wy);
				mx = getvaluator(MOUSEX) - wx;
				my = getvaluator(MOUSEY) - wy;
/* if button is still down then we will be drawing */
				if (getbutton(LEFTMOUSE)) {
        				getsize(&a,&b);
        				ortho(-(a/2.0),(a/2.0),-(b/2.0),
						(b/2.0),-(a/2.0),(a/2.0));
  				        zclear();
        				zbuffer(FALSE);
				}
/* animate while button is down */
				a = mx;
				b = my;
				while (getbutton(LEFTMOUSE)) {
					mx = getvaluator(MOUSEX) - wx;
					my = getvaluator(MOUSEY) - wy;
/* redraw only if needed */
					if ((a != mx) || (b != my)) {
						rx = (b - my);
						ry = -(a - mx);
						vl_arb_rotate_(&rx,xaxis,&set);
						vl_arb_rotate_(&ry,yaxis,&set);
						a = mx;
						b = my;
						cpack(set.backpack);	
        					clear();
						cpack(0x00ffffff);
						vl_drawcube_(&set);
					}

				}
				(void) qenter(REDRAW,wid);
				dirty = TRUE;
			};
			break;
		case F1KEY :
			if (val == 1) {
				speed = speed + 1;
				if (speed >= 5) speed = 1;
				printf("Speed = %d\n",speed);
				(void) qenter(REDRAW,wid);
				dirty = TRUE;
			};
			break;
		case F2KEY :
			if (val == 1) {
/* build rlut  and llut */
	vl_read_pal_(opacityf,opacity);
	vl_read_pal_(lutf,lut);
	vl_read_pal_(opacityf2,opacity2);
	vl_read_pal_(lutf2,lut2);
	for (i=0; i<256; i++ ) {
/* now the rlut */
		rlut[i] = 1L*lut[i] + 256L*lut[i+256] + 65536L*lut[i+512];
		rlut[i] = rlut[i] | (opacity[i] << 24);
/* and the llut */
		llut[i] = 1L*lut2[i] + 256L*lut2[i+256] + 65536L*lut2[i+512];
		llut[i] = llut[i] | (opacity2[i] << 24);
	};
				qenter(REDRAW,wid);
				dirty = TRUE;
			};
			break;
		case F3KEY :
			if (val == 1) {
/* simple zbuffer filtering */
				vl_filter_output_((long *)zbuff,imagex,imagey,
                                	VL_ZMEDIAN);
/* apply a light source */
				norm[0] = 0; /* dx */
				norm[1] = 0; /* dy */
				norm[2] = 1; /* dz */
				norm[3] = 1; /* intensity */
				vl_postlight_((long *)image,zbuff,
					(long *)image,imagex,imagey,norm);
				qenter(REDRAW,wid);
			}
			break;
		case F4KEY :
			if (val == 1) {
				plane = 1-plane;     
				printf("Plane = %d\n",plane);
				qenter(REDRAW,wid);
				dirty = TRUE;
			};
			break;
		};
	};

	free(image);
	free(zimage);
	free(zbuff);
}
