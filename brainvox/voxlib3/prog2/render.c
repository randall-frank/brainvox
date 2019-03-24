#include <stdio.h>
#include <malloc.h>
#include <gl.h>
#include <device.h>
#include <sys/types.h>
#include <sys/time.h>

#include "../voxel.h"

float p1[3] = {-180., 180., 0.00};
float p2[3] = { 180., 180., 0.00};
float p3[3] = { 180.,-180., 0.00};
float p4[3] = {-180.,-180., 0.00};


void render(int dx,int dy,int dz,char *lutf,char *opacityf,
		 char *lutf2, char *opacityf2,unsigned char *data,double squf) 
{
/* time stuff */
	struct timeval before,after;
	struct timezone zone;
	float time1,time2;

/* GL graphics stuff */
	int wid,cutwid;
	int dev;
	short val;

/* my variables */
	long a,b;
	int dxy;
	long rlut[256],llut[256];
	unsigned char lut[768],opacity[768],lut2[768],opacity2[768];
	long i,cont;
	float rx,ry,rz;
	long mx,my,wx,wy;
	int speed;
	int imagex,imagey;
	vset set;
	unsigned char *image;
	short *zimage;
	short *zbuff;
	int dirty,mode;
	int plane;
	float squeeze_factor;

	float rots[3];
	float dist;
	char *slice;
	int *slice2;

	double n[3],p[3];
/* plane parallel to the screen at depth 0 */
	n[0] = 0;
	n[1] = 0;
	n[2] = -1;
        p[0] = 0;
        p[1] = 0;
        p[2] = 1;
	plane = 0;

	squeeze_factor = squf;

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

	(void) prefsize(256,256);
	cutwid = winopen("Cut image");
	(void) singlebuffer();
	(void) RGBmode();
	(void) gconfig();

	(void) qdevice(WINQUIT);
	(void) qdevice(F12KEY); 
	(void) qdevice(F11KEY); 
	(void) qdevice(F10KEY); 
	(void) qdevice(F9KEY); 
	(void) qdevice(F8KEY); 
	(void) qdevice(F7KEY); 
	(void) qdevice(F6KEY); 
	(void) qdevice(F5KEY); 
	(void) qdevice(F1KEY); 
	(void) qdevice(F2KEY); 
	(void) qdevice(F3KEY); 
	(void) qdevice(F4KEY); 
	(void) qdevice(PADASTERKEY) ;
	(void) qdevice(PAD0) ;
	(void) qdevice(PAD1) ;
	(void) qdevice(PAD2) ;
	(void) qdevice(PAD3) ;
	(void) qdevice(LEFTMOUSE);
	(void) qdevice(MIDDLEMOUSE);
	(void) qdevice(RIGHTMOUSE);
	(void) qenter(REDRAW,wid);
	(void) qenter(REDRAW,cutwid);
	
	(void) vl_init_(&set);
	(void) vl_data_(dx,dy,dz,data,&set);

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

	zbuff = malloc(sizeof(short)*imagex*imagey);
	if (zbuff == 0) {
		printf("Unable to allocate memory for buffer saving\n");
		exit(1);
	};

	slice = malloc(256*256*sizeof(char));
	slice2 = (int *)malloc(256*256*sizeof(int));

	(void) vl_image_(imagex,imagey,image,zbuff,&set);
/* attempt to load any saved packing scheme 
	(void) vl_loadpack_(&set,"_packing",&i);
*/
	if (i != VL_NOERR) {
/* if unsuccessful in loading a scheme let's make our own 
		(void) vl_pack_on_(&set);
		(void) vl_set_pack_type_(&set,VL_CLOSEPACK);
		(void) vl_savepack_(&set,"_packing",&i);
 NOT ! */
	}

	
	mode = 0;
	rx = 0.0;
	ry = 0.0;
	rz = 0.0;
	speed = 1;
	dirty = TRUE;

	cont = 1;
	while (cont) {
		dev = qread(&val);
		switch(dev)
		{
		case INPUTCHANGE :
			if (val != 0) winset(val);
			break;

	        case REDRAW :
			if (val == wid ) {
			winset(wid);
			(void) reshapeviewport();
			(void) vl_setsqu_(&squeeze_factor,&set); 
			(void) vl_rotations_((double)rx,(double)ry,
				(double)rz,&set);
			(void) vl_translations_(0.0,0.0,0.0,&set);
			(void) vl_speed_(speed,&set);
			(void) vl_lookups_(rlut,llut,0x00000000,&set);
			(void) vl_plane_(plane,p,n,&set);
			if (dirty) {

				(void) gettimeofday(&before,&zone);
				(void) vl_render_(mode,MIDDLEMOUSE,&i,&set);
				(void) gettimeofday(&after,&zone);
				(void) zbuffer(FALSE);
/*				(void) zclear(); */
				if (mode == 0) {
/* grab the Zbuffer */
					(void) readsource(SRC_ZBUFFER);
					(void) lrectread(0,0,imagex-1,imagey-1,
						(unsigned long *)zimage);	
					(void) readsource(SRC_AUTO);
				}
				if (i) {
					printf("Render returned code:%d\n",i);
					qreset();
				};
				if (getbutton(LEFTALTKEY)) {
					savebuffers(set.image,zimage,imagex,imagey);
				}
		time1 = ((float)before.tv_usec/1000000.0);
		time2 = ((float)after.tv_usec/1000000.0);
		time2 = time2 + (float)(after.tv_sec - before.tv_sec);
		printf(" %d %d    %d %d \n", before.tv_sec, before.tv_usec,
				after.tv_sec , after.tv_usec);
		printf("%d voxels in %f seconds = %f voxels per sec\n",
		dx*dy*dz,time2-time1,
		((float)(dx*dy*dz))/(time2-time1));
				dirty = FALSE;
				} else {
				(void) zbuffer(FALSE);
				if (mode == 0 ) {
					(void) zdraw(TRUE);
					(void) lrectwrite(0,0,imagex-1,imagey-1,
						(unsigned long *)(zimage));
					(void) zdraw(FALSE);
				}
				(void) lrectwrite(0,0,imagex-1,imagey-1,
					(unsigned long *)(set.image));
				};
			};
			if (val == cutwid) {
				winset(cutwid);
				(void) reshapeviewport();
				(void) lrectwrite(0,0,255,255,(unsigned long *)slice2);
			}
			break;
		case WINQUIT :
			cont = 0;
			break;
		case MIDDLEMOUSE :
			if ((val == 1) && (winget() == wid)) {
				rots[0] = rx/10.0;
				rots[1] = ry/10.0;
				rots[2] = 0.0;
				dist = 0.0;
				vl_resample_(256,256,slice,rots,&dist,&set);
				for (i=0; i<256*256; i++) {
					slice2[i]=slice[i];
					slice2[i]=slice2[i] | (slice[i] << 8);
					slice2[i]=slice2[i] | (slice[i] << 16);
				}
				winset(cutwid);
				(void) lrectwrite(0,0,255,255,(unsigned long *)slice2);
				winset(wid);
			}
			break;
		case RIGHTMOUSE :
			if ((val == 1) && (winget() == wid)) {
				(void) getorigin(&wx,&wy);
				mx = getvaluator(MOUSEX) - wx;
				my = getvaluator(MOUSEY) - wy;
				rx = ((mx-256)/256.0)*1800.0;
				ry = ((my-256)/256.0)*1800.0;
				if (getbutton(RIGHTMOUSE)) {
        				(void) getsize(&a,&b);
        (void) ortho(-(a/2.0),(a/2.0),-(b/2.0),(b/2.0),-(a/2.0),(a/2.0));
				};
/* animate while button is down */
				a = mx;
				b = my;
				while (getbutton(RIGHTMOUSE)) {
					mx = getvaluator(MOUSEX) - wx;
					my = getvaluator(MOUSEY) - wy;
					rx = ((mx-256)/256.0)*1800.0;
					ry = ((my-256)/256.0)*1800.0;
/* redraw only if needed */
					if ((a != mx) && (b != my)) {
						a = mx;
						b = my;
						(void) zbuffer(FALSE);
						(void) zdraw(TRUE);
						(void) frontbuffer(FALSE);
				(void) lrectwrite(0,0,imagex-1,imagey-1,
					(unsigned long *)(zimage));
						(void) zdraw(FALSE);
						(void) frontbuffer(TRUE);
				(void) lrectwrite(0,0,imagex-1,imagey-1,
					(unsigned long *)(set.image));
						(void) zbuffer(TRUE);
						(void) blendfunction(BF_SA, BF_MSA);
						(void) cpack(0xa0303030);
						(void) pushmatrix();
						(void) rotate((float)rx,'x');
						(void) rotate((float)ry,'y');
						(void) bgnpolygon();
						(void) v3f(p1);
						(void) v3f(p2);
						(void) v3f(p3);
						(void) v3f(p4);
						(void) endpolygon();
						(void) popmatrix();
						(void) zbuffer(FALSE);
						(void) blendfunction(BF_ONE, BF_ZERO);
					}

				}
			};
			break;
		case LEFTMOUSE :
			if ((val == 1) && (winget() == wid)) {
				(void) getorigin(&wx,&wy);
				mx = getvaluator(MOUSEX) - wx;
				my = getvaluator(MOUSEY) - wy;
				rx = ((mx-256)/256.0)*180.0;
				ry = ((my-256)/256.0)*180.0;
/* if button is still down then we will be drawing */
				if (getbutton(LEFTMOUSE)) {
        				(void) getsize(&a,&b);
        (void) ortho(-(a/2.0),(a/2.0),-(b/2.0),(b/2.0),-(a/2.0),(a/2.0));
					(void) blendfunction(BF_ONE, BF_ZERO);
        				(void) subpixel(FALSE);
        				(void) pntsmooth(SMP_OFF);
        				(void) zbuffer(FALSE);
  				        (void) zclear();
				}
/* animate while button is down */
				a = mx;
				b = my;
				while (getbutton(LEFTMOUSE)) {
					mx = getvaluator(MOUSEX) - wx;
					my = getvaluator(MOUSEY) - wy;
					rx = ((mx-256)/256.0)*180.0;
					ry = ((my-256)/256.0)*180.0;
/* redraw only if needed */
					if ((a != mx) && (b != my)) {
						a = mx;
						b = my;
					(void) vl_rotations_((double)rx,
				(double)ry,
				(double)rz,&set);
					(void) cpack(set.backpack);	
        				(void) clear();
					(void) cpack(0x00ffffff);
					(void) vl_drawcube_(&set);
					}

				}
				printf("Rx = %f Ry = %f\n",rx,ry); 
				(void) qenter(REDRAW,wid);
				dirty = TRUE;
			};
			break;
		case PADASTERKEY :
			if (val == 1) {
				squeeze_factor = squeeze_factor - 0.1;
				if (squeeze_factor < 0.01) squeeze_factor = 1.0;
				printf("Squeeze = %f\n",squeeze_factor);
				(void) vl_setsqu_(&squeeze_factor,&set); 
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
				(void) qenter(REDRAW,wid);
				dirty = TRUE;
			};
			break;
		case F12KEY :
			if (val == 1) {
				 ry = 270;
				(void) qenter(REDRAW,wid);
				dirty = TRUE;
				};
			break;
		case F11KEY :
			if (val == 1) {
				 ry = 180;
				(void) qenter(REDRAW,wid);
				dirty = TRUE;
			};
			break;
		case F10KEY :
			if (val == 1) {
				 ry = 90;
				(void) qenter(REDRAW,wid);
				dirty = TRUE;
			};
			break;
		case F9KEY :
			if (val == 1) {
				 ry = 0;
				(void) qenter(REDRAW,wid);
				dirty = TRUE;
			};
			break;
		case F8KEY :
			if (val == 1) {
				 rx = 270;
				(void) qenter(REDRAW,wid);
				dirty = TRUE;
			};
			break;
		case F7KEY :
			if (val == 1) {
				 rx = 180;
				(void) qenter(REDRAW,wid);
				dirty = TRUE;
			};
			break;
		case F6KEY :
			if (val == 1) {
				 rx = 90;
				(void) qenter(REDRAW,wid);
				dirty = TRUE;
			};
			break;
		case F5KEY :
			if (val == 1) {
				 rx = 0;
				(void) qenter(REDRAW,wid);
				dirty = TRUE;
			};
			break;
		case F3KEY :
			if (val == 1) {
				 mode = mode ^ VL_BACKGROUND;
				printf("Mode = %d\n",mode);
				(void) qenter(REDRAW,wid);
				dirty = TRUE;
			};
			break;
		case F4KEY :
			if (val == 1) {
				 plane = 1-plane;     
				printf("Plane = %d\n",plane);
				(void) qenter(REDRAW,wid);
				dirty = TRUE;
			};
			break;
		case PAD0 :
			if (val == 1) {
				draw_rect(&set);
				(void) qenter(REDRAW,wid);
			}
			break;
		};
	};

	(void) free(image);
	(void) free(zimage);
	(void) free(zbuff);

	free((char *)slice2);
	free(slice);

}
