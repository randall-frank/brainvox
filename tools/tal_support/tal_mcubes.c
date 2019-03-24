/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski
 *
 * $Id: tal_mcubes.c 1213 2005-08-27 20:51:21Z rjfrank $
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
#include "geom_view_io.h"

#define MIN(a,b) (((a)<(b)?(a):(b)))
#define MAX(a,b) (((a)>(b)?(a):(b)))

int	verbose = 0;

void ex_err(char *s)
{
	fprintf(stderr,"Fatal error:%s\n",s);
	tal_exit(1);
}

void cmd_err(char *s)
{
	fprintf(stderr,"(%s:%s) Usage: %s [options] inputtemp outputfile\n",__DATE__,TAL_VERSION,s);
	fprintf(stderr,"Options: -x(dx) image dx default:256\n");
	fprintf(stderr,"         -y(dy) image dy default:256\n");
	fprintf(stderr,"         -z(dz) image depth in bytes default:1\n");
	fprintf(stderr,"         -f(start) first image number default:1\n");
	fprintf(stderr,"         -l(end) last image number default:119\n");
	fprintf(stderr,"         -i(step) image number step default:1\n");
	fprintf(stderr,"         -b swap bytes with 16bit images default:no\n");
	fprintf(stderr,"         -v verbose mode\n");
	fprintf(stderr,"         -p(ipx:[ipy]) interpixel spacing default:1.0[:1.0]\n");
	fprintf(stderr,"         -s(interslice) interslice spacing default:1.0\n");
	fprintf(stderr,"         -o(x,y,z) origin coords to subtract default:0:0:0\n");
	fprintf(stderr,"         -m(masktemp) filename template for a mask volume\n");
	fprintf(stderr,"         -t(threshold) threshold to isosurface at default:128\n");
	fprintf(stderr,"         -g write in geomview format default:obj\n");
	fprintf(stderr," output = triangle mesh at threshold t\n");
	tal_exit(1);
}

int main(int argc,char **argv)
{
	long int	dx = 256;
	long int	dy = 256;
	long int	dz = 1;
	long int	istart = 1;
	long int	iend = 119;
	long int	istep = 1;
	long int	swab = 0;
	long int	noff = 0;
	char		*masktemp = 0L;
	unsigned char	*maskvol = 0L;
	double		thres = 128.0;
	double		ipixel[2] = {1.0,1.0};
	double		islice = 1.0;
	double		origin[3] = {0,0,0};

	float		*verts = 0L,*norms = 0L;
	long int	*tris = 0L,n_verts,n_tris;
	double		sarea = 0.0;

	char		intemp[256],outfile[256];
	char		tstr[256];
	long int	i,err,j,k;
	long int	icount;
	unsigned char	*data;

	i = 1;
	while ((i < argc) && (argv[i][0] == '-')) {
		if (argv[i][1] == '\0') break;
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
			case 't':
				thres = atof(&(argv[i][2]));
				break;
			case 'm':
				masktemp = &(argv[i][2]);
				break;
			case 'o':
				get_d_xyz(origin,&(argv[i][2]));
				break;
			case 'b':
				swab = 1;
				break;
			case 'v':
				verbose = 1;
				break;
			case 'g':
				noff = 1;
				break;
			case 'p':
				get_d_xy(ipixel,&(argv[i][2]));
				break;
			case 's':
				islice = atof(&(argv[i][2]));
				break;
			default:	
				cmd_err(argv[0]);
				break;
		}
		i++;
	}
	if ((argc-i) != 2) cmd_err(argv[0]);
	strcpy(intemp,argv[i]);
	strcpy(outfile,argv[i+1]);

/* get the image memory */
	icount = 0;
	for(i=istart;i<=iend;i=i+istep) icount++;

	data = (unsigned char *)malloc(dz*dx*dy*icount);
	if (data == 0L) ex_err("Unable to allocate image memory.");

	maskvol = (unsigned char *)malloc(dx*dy*icount);
	if (maskvol == 0L) ex_err("Unable to allocate image memory.");

/* read the images */
	j = 0;
	for(i=istart;i<=iend;i=i+istep) {
		name_changer(intemp,i,&err,tstr);
		fprintf(stderr,"Reading the file:%s\n",tstr);
		bin_io(tstr,'r',&(data[j*dz]),dx,dy,dz,0,swab,0L);
		if (masktemp) {
			name_changer(masktemp,i,&err,tstr);
			fprintf(stderr,"Reading the mask file:%s\n",tstr);
			bin_io(tstr,'r',&(maskvol[j]),dx,dy,1,0,swab,0L);
		} else {
			for(k=0;k<dx*dy;k++) maskvol[j+k] = 255;
		}
		j = j + (dx*dy);
	}

/* compute the surface */
	i = vol_isocontour(data,maskvol,dx,dy,icount,dz,thres,ipixel,islice,
		&sarea,&n_verts,&verts,&norms,&n_tris,&tris);
	if (i != 0) {
		ex_err("Unable to complete contour operation");
	}

/* subtract origin */
	j = 0;
	for(i=0;i<n_verts;i++) {
		verts[j+0] -= origin[0];
		verts[j+1] -= origin[1];
		verts[j+2] -= origin[2];
		j += 3;
	}
	
/* save the surface */
        if (noff) {
		i = write_geom_file(outfile,n_verts,n_tris,verts,norms,tris);
        } else {
		i = write_OBJ_file(outfile,n_verts,n_tris,verts,norms,tris);
	}

	fprintf(stderr,"Surface area=%f\n",sarea);

/* free up memory */
	if (tris) free(tris);
	if (norms) free(norms);
	if (verts) free(verts);
	free(data);
	free(maskvol);

	tal_exit(0);

	exit(0);
}
