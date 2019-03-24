#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../voxel.h"
#include "../vl_polygon.h"

#define DX 128
#define DY 32

void draw_rect(vset *set)
{
	unsigned char	data[DX*DY];
	short		zdepth[DX*DY];

	long int	x,y,flag,i;

	i = 0;
	for(y=0;y<DY;y++) {
	for(x=0;x<DX;x++) {
		data[i] = 255-(x*2);
		if (y < (DY/2)) {
			zdepth[i] = -500;
		} else {
			zdepth[i] = 0;
		}
		i++;
	}
	}

	x = 0;
	y = -25;
	flag = 0;
	vl_render_pixmap_(data,zdepth,DX,DY,x,y,flag,0x00ff00ff,set);

	y = 25;
	flag = VL_P_PMAP_ZB;
	vl_render_pixmap_(data,zdepth,DX,DY,x,y,flag,0x00ff0000,set);

	return;
}
