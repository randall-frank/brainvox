/*
 * Copyright (c), 1990-1999 Randall Frank and Thomas Grabowski
 *
 * $Id: tal_warp.c 1213 2005-08-27 20:51:21Z rjfrank $
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

#define MIN(a,b) (((a)<(b)?(a):(b)))
#define MAX(a,b) (((a)>(b)?(a):(b)))
#define LERP(l,h,a) ((l)+(((h)-(l))*(a)))

void ex_err(char *s)
{
	fprintf(stderr,"Fatal error:%s\n",s);
	exit(1);
}

void cmd_err(char *s)
{
	fprintf(stderr,"(%s:%s) Usage: %s [options] intemp outtemp dxtemp dytemp dytemp\n",__DATE__,TAL_VERSION,s);
	fprintf(stderr,"Options: -x(dx) image dx default:256\n");
	fprintf(stderr,"         -y(dy) image dy default:256\n");
	fprintf(stderr,"         -z(dz) image depth in bytes default:1\n");
	fprintf(stderr,"         -f(start) first image number default:1\n");
	fprintf(stderr,"         -l(end) last image number default:119\n");
	fprintf(stderr,"         -i(step) image number step default:1\n");
	fprintf(stderr,"         -b swap bytes default:no\n");
	fprintf(stderr,"         -n normalize the dx,dy,dz vectors default:no\n");
	fprintf(stderr,"         -I trilinear interpolate values default:nearest\n");
	fprintf(stderr,"         -o(back) value for src or dst outside of mask default:0.0\n");
	fprintf(stderr,"         -s(x:y:z) vector to multiply the vectors by default:1.0:1.0:1.0\n");
	fprintf(stderr,"         -d(x:y:z) dims of output, vector and mask vols default:input vol dims\n");
	fprintf(stderr,"         -m(masktemp) filename template for a mask volume default:none\n");
	fprintf(stderr,"         -R(srctemp) template for source mask volume output default:none\n");
	fprintf(stderr,"intemp=z,outtemp=z,dxtemp=dytemp=dztemp=z4\n");
	fprintf(stderr,"Notes:Normalization occurs before multiplication.\n");
	fprintf(stderr,"      Mask, output and vector volumes are the same\n");
	fprintf(stderr,"      size and are number starting at 1 and stepping by 1.\n");
	exit(1);
}

static float interpolate(float *vv,long int *pi,
	long int dx,long int dy,long int dz,long int size,void *data);

int main(int argc,char **argv)
{
	long int	dx = 256;
	long int	dy = 256;
	long int	dz = 1;
	long int	istart = 1;
	long int	iend = 119;
	long int	istep = 1;
	long int	swab = 0;
	double		mult[3] = {1.0,1.0,1.0};
	long int	normalize = 0;
	long int	interp = 0;
	float		background = 0.0;
	long int	dims[3] = {256,256,119};
	long int	bHaveDims = 0;

	char		*masktemp = NULL;
	char		*srcmasktemp = NULL;

	char		*intemp,*outtemp;
	char		*X_temp,*Y_temp,*Z_temp;

	char		tstr[256];
	long int	i,err,j,k,pi[3],x,y;
	long int	icount;
	unsigned char	*mask,*srcmask=NULL;
	unsigned char	*data;
	unsigned char	*slice;
	float		*Xw,*Yw,*Zw,v;

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
			case 'm':
				masktemp = &(argv[i][2]);
				break;
			case 'R':
				srcmasktemp = &(argv[i][2]);
				break;
			case 'o':
				background = atof(&(argv[i][2]));
				break;
			case 's':
				get_d_xyz(mult,&(argv[i][2]));
				break;
			case 'd':
				bHaveDims = 1;
				get_xyz(dims,&(argv[i][2]));
				break;
			case 'b':
				swab = 1;
				break;
			case 'n':
				normalize = 1;
				break;
			case 'I':
				interp = 1;
				break;
			default:	
				cmd_err(argv[0]);
				break;
		}
		i++;
	}
	if ((argc-i) != 5) cmd_err(argv[0]);
	intemp=argv[i];
	outtemp=argv[i+1];
	X_temp=argv[i+2];
	Y_temp=argv[i+3];
	Z_temp=argv[i+4];

/* get the image memory */
	icount = 0;
	for(i=istart;i<=iend;i=i+istep) icount++;

/* get output and mask dimensions */
	if (!bHaveDims) {
		dims[0] = dx;
		dims[1] = dy;
		dims[2] = icount;
	}

/* store the volume to be interpolated */
	data = (unsigned char *)malloc(dx*dy*icount*dz);  /* volume to be resampled */
	if (data == 0L) ex_err("Unable to allocate image memory.");
	mask = malloc(dims[0]*dims[1]); /* mask slice */
	if (mask == 0L) ex_err("Unable to allocate image memory.");
	memset(mask,0xff,dims[0]*dims[1]);
	slice = (void *)malloc(dims[0]*dims[1]*dz); /* single slice */
	if (slice == 0L) ex_err("Unable to allocate image memory.");
	Xw = (float *)malloc(dims[0]*dims[1]*sizeof(float)); /* single slice */
	if (Xw == 0L) ex_err("Unable to allocate image memory.");
	Yw = (float *)malloc(dims[0]*dims[1]*sizeof(float)); /* single slice */
	if (Yw == 0L) ex_err("Unable to allocate image memory.");
	Zw = (float *)malloc(dims[0]*dims[1]*sizeof(float)); /* single slice */
	if (Zw == 0L) ex_err("Unable to allocate image memory.");
	if (srcmasktemp) {
		srcmask = malloc(dims[0]*dims[1]*dims[2]);
		if (srcmask == 0L) ex_err("Unable to allocate image memory.");
		memset(srcmask,0,dims[0]*dims[1]*dims[2]);
	}

/* read the images */
	fprintf(stderr,"Reading %ld byte dataset for warp computation\n",
		(dx*dy*icount*dz));

	j = 0;
	for(i=istart;i<=iend;i=i+istep) {
		name_changer(intemp,i,&err,tstr);
		fprintf(stderr,"Reading the file:%s\n",tstr);
		bin_io(tstr,'r',&(data[j]),dx,dy,dz,0,swab,0L);
		j = j + (dx*dy*dz);
	}


/* do the work */
	for(i=1;i<=dims[2];i++) {
/* read a mask slice */
		if (masktemp) {
			name_changer(masktemp,i,&err,tstr);
			fprintf(stderr,"Reading the mask file:%s\n",tstr);
			bin_io(tstr,'r',mask,dims[0],dims[1],1,0,swab,0L);
		}

/* read vector slices */
		name_changer(X_temp,i,&err,tstr);
		fprintf(stderr,"Reading the X_warp file:%s\n",tstr);
		bin_io(tstr,'r',Xw,dims[0],dims[1],sizeof(float),0,swab,0L);
		name_changer(Y_temp,i,&err,tstr);
		fprintf(stderr,"Reading the Y_warp file:%s\n",tstr);
		bin_io(tstr,'r',Yw,dims[0],dims[1],sizeof(float),0,swab,0L);
		name_changer(Z_temp,i,&err,tstr);
		fprintf(stderr,"Reading the Z_warp file:%s\n",tstr);
		bin_io(tstr,'r',Zw,dims[0],dims[1],sizeof(float),0,swab,0L);

/* warp the slice */
		j = 0;
		for(y=0;y<dims[0];y++) {
		for(x=0;x<dims[1];x++) {

/* the the value is not currently valid and is on the inside... */
			v = background;

			if (mask[j] > 0) {
				float	vv[3];
		
/* grab the vector */
				vv[0] = Xw[j];
				vv[1] = Yw[j];
				vv[2] = Zw[j];

/* normalize */
				if (normalize) {
					float	d;
					d = vv[0]*vv[0] + vv[1]*vv[1] + 
						vv[2]*vv[2];
					if (d > 0) {
						d = sqrt(d);
						vv[0] /= d;
						vv[1] /= d;
						vv[2] /= d;
					}
				}

/* get the voxel location (after mult) */
				vv[0] = x + vv[0]*mult[0];
				vv[1] = y + vv[1]*mult[1];
				vv[2] = (i-1) + vv[2]*mult[2];

/* floor... */
				pi[0] = vv[0];
				pi[1] = vv[1];
				pi[2] = vv[2];

/* paint back into "output mask volume" */				
				if (srcmasktemp) {
				    if ((pi[0] >= 0) && (pi[0] < dims[0]-1) && 
                                        (pi[1] >= 0) && (pi[1] < dims[1]-1) && 
                                        (pi[2] >= 0) && (pi[2] < dims[2]-1)) {
/* this volume has the value of the destination mask stored at the
 * source location, provided that location exists in the destination volume */
				        k = (pi[2]*dims[0]*dims[1]) + 
					    (pi[1]*dims[0]) + pi[0];
				        srcmask[k] = mask[j];
				    }
				}

/* pick the interpolation method */
				if (interp) {
				    if ((pi[0] >= 0) && (pi[0] < dx-1) && 
                                        (pi[1] >= 0) && (pi[1] < dy-1) && 
                                        (pi[2] >= 0) && (pi[2] < icount-1)) {
					v = interpolate(vv,pi,dx,dy,icount,dz,data);
				    }
				} else {
/* is it in bounds */
				    if ((pi[0] >= 0) && (pi[0] < dx) && 
                                        (pi[1] >= 0) && (pi[1] < dy) && 
                                        (pi[2] >= 0) && (pi[2] < icount)) {
/* sample the point */
					k = (pi[2]*dx*dy) + (pi[1]*dx) + pi[0];
					switch(dz) {
						case 1:
						v = ((unsigned char *)data)[k];
							break;
						case 2:
						v = ((unsigned short *)data)[k];
							break;
						case 4:
						v = ((float *)data)[k];
							break;
					}
				    }
				}
			}
/* store the value */
			switch(dz) {
				case 1:
					((unsigned char *)slice)[j]=v;
					break;
				case 2:
					((unsigned short *)slice)[j]=v;
					break;
				case 4:
					((float *)slice)[j]=v;
					break;
			}
			j++;
		}
		}
/* write out the result */
		name_changer(outtemp,i,&err,tstr);
		fprintf(stderr,"Writing the file:%s\n",tstr);
		bin_io(tstr,'w',slice,dims[0],dims[1],dz,0,swab,0L);
	}  /* over the z planes */

/* output the resampled mask volume if required */
	if (srcmasktemp) {
		for(i=1;i<=dims[2];i++) {
			name_changer(srcmasktemp,i,&err,tstr);
			fprintf(stderr,"Writing the file:%s\n",tstr);
			bin_io(tstr,'w',srcmask+((i-1)*dims[0]*dims[1]),
					dims[0],dims[1],1,0,swab,0L);
		}
		free(srcmask);
	}

/* done */	
	free(data);
	free(slice);
	free(mask);
	free(Xw);
	free(Yw);
	free(Zw);

	exit(0);
}

static float interpolate(float *vv,long int *pi,
	long int dx,long int dy,long int dz,long int size,void *data)
{
	float		tmp[6],vals[8];
	float		w0;
	long int	k,i;

	for(i=0;i<8;i++) {
		k = pi[0] + (dx*pi[1]) + (dx*dy*pi[2]);
		if (i & 1) k += 1;
		if (i & 2) k += dx;
		if (i & 4) k += (dx*dy);
		switch(size) {
			case 1:
				vals[i] = ((unsigned char *)data)[k];
				break;
			case 2:
				vals[i] = ((unsigned short *)data)[k];
				break;
			case 4:
				vals[i] = ((float *)data)[k];
				break;
		}
	}

	/* interpolate in X 8->4 */
	w0 = vv[0] - pi[0];
	tmp[0]=LERP(vals[0],vals[1],w0);
	tmp[1]=LERP(vals[2],vals[3],w0);
	tmp[2]=LERP(vals[4],vals[5],w0);
	tmp[3]=LERP(vals[6],vals[7],w0);
	/* interpolate in Y 4->2 */
	w0 = vv[1] - pi[1];
	tmp[4]=LERP(tmp[0],tmp[1],w0);
	tmp[5]=LERP(tmp[2],tmp[3],w0);
	/* interpolate in Z 2->1 */
	w0 = vv[2] - pi[2];
	return(LERP(tmp[4],tmp[5],w0));
}

