/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski
 *
 * $Id: tal_resample.c 1213 2005-08-27 20:51:21Z rjfrank $
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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "proto.h"

void ex_err(char *s)
{
	fprintf(stderr,"Fatal error:%s\n",s);
	tal_exit(1);
}

void cmd_err(char *s)
{
	fprintf(stderr,"(%s:%s) Usage: %s [options] inputtemp outputtemp\n",__DATE__,TAL_VERSION,s);
	fprintf(stderr,"Options: -x(dx) image dx default:256\n");
	fprintf(stderr,"         -y(dy) image dy default:256\n");
	fprintf(stderr,"         -z(dz) image depth in bytes default:2\n");
	fprintf(stderr,"         -p(ipx[:ipy]) interpixel spacing default:1.0[:1.0]\n");
	fprintf(stderr,"         -s(interslice) interslice spacing default:6.64\n");
	fprintf(stderr,"         -f(start) first image number default:1\n");
	fprintf(stderr,"         -l(end) last image number default:124\n");
	fprintf(stderr,"         -i(step) image number step default:1\n");
	fprintf(stderr,"         -o(dz) output interslice spacing default:1.0\n");
	fprintf(stderr,"         -d(value) background value default:0\n");
	fprintf(stderr,"         -b swap bytes with 16bit images default:no\n");
	fprintf(stderr,"         -t(talfile) calib input file default:_talairach\n");
	fprintf(stderr,"         -u use unit scaling, _talairach file used for location and attitude\n");
	fprintf(stderr,"         -L(minz:maxz) tal space Z sampling limits defaults:%0.2f %0.2f\n",-TAL_BOTTOM,TAL_TOP);
	tal_exit(1);
}

int main(int argc,char **argv)
{
	long int	dx = 256;
	long int	dy = 256;
	long int	dz = 2;
	long int	istart = 1;
	long int	iend = 124;
	long int	istep = 1;
	double 		back = 0;
	double		ddz = 1.0;
	double		ipixel[2] = {1.0,1.0};
	double		islice = 6.64;
	long int	swab = 0;
	long int	unit_scale = 0;
	double		sqf;
	long int	interp,n_slices,z_images;
	char		intemp[256],outtemp[256];
	char		tal_file[256],tstr[256];
	long int	i,err,j;
	tal_conv	tal;
	unsigned char 	*data,*jp,*pj,*oimage;
	long int	p[3],k,ptr;
	double		x,y,z,i_islice;
	double		tin[3],pmm[3],ft[2];
        double		minx,miny,minz,maxx,maxy,maxz;

	strcpy(tal_file,"_talairach");

/* default ranges */
        miny = -(TAL_BACK+TAL_MIDDLE);
        maxy = TAL_FRONT;
        minx = -(TAL_LEFT);
        maxx = TAL_RIGHT;
        minz = -TAL_BOTTOM;
        maxz = TAL_TOP;

/* parse command line */
	i = 1;
	while ((i < argc) && (argv[i][0] == '-')) {
		switch (argv[i][1]) {
			case 'f':
				istart = atoi(&(argv[i][2]));
				break;
			case 'l':
				iend = atoi(&(argv[i][2]));
				break;
			case 'i':
				istep = atoi(&(argv[i][2]));
				break;
			case 'x':
				dx = atoi(&(argv[i][2]));
				break;
			case 'y':
				dy = atoi(&(argv[i][2]));
				break;
			case 'z':
				dz = atoi(&(argv[i][2]));
				break;
			case 'd':
				back = atof(&(argv[i][2]));
				break;
			case 'p':
				get_d_xy(ipixel,&(argv[i][2]));
				break;
			case 's':
				islice = atof(&(argv[i][2]));
				break;
			case 'o':
				ddz = atof(&(argv[i][2]));
				break;
			case 'L':
				get_d_xy(ft,&(argv[i][2]));
                                minz = ft[0];
                                maxz = ft[1];
                                if (ft[0] > ft[1]) {
                                   minz = ft[1];
                                   maxz = ft[0];
                                }
				break;
			case 't':
				strcpy(tal_file,&(argv[i][2]));
				break;
			case 'u':
				unit_scale = 1;
				break;
			case 'b':
				swab = 1;
				break;
			default:	
				cmd_err(argv[0]);
				break;
		}
		i++;
	}
	if ((argc-i) != 2) cmd_err(argv[0]);
	strcpy(intemp,argv[i]);
	strcpy(outtemp,argv[i+1]);
/* count the slices */
	n_slices = 0;
	i = istart;
	while (i <= iend) {
		i += istep;
		n_slices++;
	}
/* compute the squeeze */
        sqf = 10.0;
        interp = -1;
        while ((sqf > 1.0) && (interp < 20)) {
                interp = interp + 1;
                sqf = ((double)(n_slices)*islice)/
			((double)(n_slices+(interp*(n_slices-1)))*ipixel[0]);
	}
	printf("Reading %ld slices with interp = %ld and squeeze = %f\n",
		n_slices,interp,sqf);
	z_images = n_slices+(interp*(n_slices-1));
	i_islice = ((double)(n_slices)*islice)/(double)(z_images);
	printf("True interslice = %f\n",i_islice);

/* read the tal_space calibration */
/* I WANT it in MM NOT in pixels */
	tal_read(tal_file,1.0,&tal);

/* use the full bounds (inset by one pixel) */
        miny = -(dy/2)+1.0;
        maxy = (dy/2)-1.0;
        minx = -(dx/2)+1.0;
        maxx = (dx/2)-1.0;

/* remove the scaling */
	if (unit_scale) {
		tal.CA_right = TAL_RIGHT;
		tal.CA_left = TAL_LEFT;
		tal.CA_top = TAL_TOP;
		tal.CA_bottom = TAL_BOTTOM;
		tal.CA_front = TAL_FRONT;
		tal.CA_CP = TAL_MIDDLE;
		tal.CP_back = TAL_BACK;
	}

/* get the image memory */
	data = malloc(z_images*dz*dx*dy);
	if (data == 0L) ex_err("Unable to allocate volume memory.");
/* read the images */
	jp = data;
	pj = data;
	for(i=istart;i<=iend;i=i+istep) {
		name_changer(intemp,i,&err,tstr);
		printf("Reading the file:%s\n",tstr);
		bin_io(tstr,'r',jp,dx,dy,dz,1,swab,0L);
		if ((i != istart) && (interp != 0)) {
			img_interpolate(pj,jp,(pj+(dx*dy*dz)),dx,dy,dz,interp);
		}
		pj = jp;
		jp = jp + ((dx*dy*dz)*(interp+1));
	}
/* resample for output */
	j = 1;
	oimage = malloc(dx*dy*dz);
	if (oimage == 0L) ex_err("Unable to allocate image memory.");
	for(z=minz;z<=maxz;z=z+ddz) {
/* clear the image */
		for(ptr=0;ptr<(dx*dy);ptr++) {
			if (dz == 2) {
				((unsigned short *)oimage)[ptr] = back;
			} else if (dz == 1) {
				oimage[ptr] = back;
			} else if (dz == 4) {
				((float *)oimage)[ptr] = back;
			}
		}
		ptr = 0;
#ifdef NEVER
		for(y= -(TAL_BACK+TAL_MIDDLE);y<=(TAL_FRONT);y=y+1) {
			for(x= -(TAL_LEFT);x<=(TAL_RIGHT);x=x+1) {
#endif
		for(y=miny;y<=maxy;y=y+1) {
			for(x=minx;x<=maxx;x=x+1) {
				tin[0] = x;
				tin[1] = y;
				tin[2] = z;
				tal_2_mm(tin,pmm,&tal);
/* convert MM to voxel value */
				p[0] = (pmm[0]/ipixel[0])+((double)(dx)/2);
				p[1] = (pmm[1]/ipixel[1])+((double)(dy)/2);
				p[2] = (pmm[2]/i_islice)+((double)(z_images)/2);
/* if it is in the volume, resample else zero */
				if ((p[0] < 0) || (p[0] >= dx) ||
					(p[1] < 0) || (p[1] >= dy) ||
				(p[2] < 0) || (p[2] >= z_images)) {
/* zero */
				} else {
/* resample */
				i = (p[2]*(dx*dy*dz));
				i += (p[1]*(dx*dz));
				i += (p[0]*(dz));
				ptr = (((int)(y+(dy/2))*dx) + 
					(int)((dx/2)-x))*dz;
				for(k=0;k<dz;k++) oimage[ptr++] = data[i++];
				}
				
			}
		}
		name_changer(outtemp,j,&err,tstr);
		printf("Writing sampled file:%s\n",tstr);
		bin_io(tstr,'w',oimage,dx,dy,dz,1,swab,0L);
		j++;
	}

	free(oimage);
	free(data);
	tal_exit(0);

	exit(0);
}
