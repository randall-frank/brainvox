/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski
 *
 * $Id: tal_fft.c 1538 2005-12-27 21:10:45Z rjfrank $
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
#include "fftn.h"

int	verbose = 0;

void wrap_around3(float *data,int *d, int oneD);

void ex_err(char *s)
{
	fprintf(stderr,"Fatal error:%s\n",s);
	tal_exit(1);
}

void cmd_err(char *s)
{
	fprintf(stderr,"(%s:%s) Usage: %s [options] datatemp realtemp imagtemp\n",__DATE__,TAL_VERSION,s);
	fprintf(stderr,"Options: -x(dx) image dx default:256\n");
	fprintf(stderr,"         -y(dy) image dy default:256\n");
	fprintf(stderr,"         -z(dz) image depth in bytes default:1\n");
	fprintf(stderr,"         -f(start) first image number default:1\n");
	fprintf(stderr,"         -l(end) last image number default:119\n");
	fprintf(stderr,"         -i(step) image number step default:1\n");
	fprintf(stderr,"         -b swap bytes with 16bit images default:no\n");
	fprintf(stderr,"         -v verbose mode\n");
	fprintf(stderr,"         -r perform reverse transform\n");
	fprintf(stderr,"         -w perform wrap-around data reordering\n");
	fprintf(stderr,"         -1 perform dx*dy 1D FFTs in the Z dimension\n");
	fprintf(stderr,"         -o(offset) offset added to datatemp default:0\n");
	tal_exit(1);
}

int main(int argc,char **argv)
{
	int	dx = 256;
	int	dy = 256;
	int	dz = 1;
	int	istart = 1;
	int	iend = 119;
	int	istep = 1;
	int	swab = 0;
	int	reverse = 0;
	int	wrap = 0;
	int	oneD = 0;
	double		offset = 0;

	char		intemp[256],Retemp[256],Imtemp[256];
	char		tstr[256];
	long int	i,err,j,k;
	int	d[3];
	int		di[3];
	int	icount;
	float		v;

	unsigned char	*data;

	float		*Re;
	float		*Im;

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
			case 'o':
				offset = atof(&(argv[i][2]));
				break;
			case 'r':
				reverse = 1;
				break;
			case 'w':
				wrap = 1;
				break;
			case '1':
				oneD = 1;
				break;
			case 'b':
				swab = 1;
				break;
			case 'v':
				verbose = 1;
				break;
			default:	
				cmd_err(argv[0]);
				break;
		}
		i++;
	}
	if ((argc-i) != 3) cmd_err(argv[0]);
	strcpy(intemp,argv[i]);
	strcpy(Retemp,argv[i+1]);
	strcpy(Imtemp,argv[i+2]);

/* get the image memory */
	icount = 0;
	for(i=istart;i<=iend;i=i+istep) icount++;

	Re = (float *)malloc(dx*dy*icount*sizeof(float));
	if (Re == 0L) ex_err("Unable to allocate image memory.");

	Im = (float *)malloc(dx*dy*icount*sizeof(float));
	if (Im == 0L) ex_err("Unable to allocate image memory.");

/* for one buffereed slice */
	data = malloc(dz*dx*dy);
	if (data == 0L) ex_err("Unable to allocate image memory.");

/* read the images */
	if (reverse) {
		fprintf(stderr,"Reading datasets for reverse FFT\n");
	} else {
		fprintf(stderr,"Reading dataset for forward FFT\n");
	}

/* all slices */
	j = 0;
	for(i=istart;i<=iend;i=i+istep) {
		if (reverse) {
			name_changer(Retemp,i,&err,tstr);
			fprintf(stderr,"Reading the Real file:%s\n",tstr);
			bin_io(tstr,'r',&(Re[j]),dx,dy,sizeof(float),1,swab,0L);

			name_changer(Imtemp,i,&err,tstr);
			fprintf(stderr,"Reading the Imaginary file:%s\n",tstr);
			bin_io(tstr,'r',&(Im[j]),dx,dy,sizeof(float),1,swab,0L);
		} else {
			name_changer(intemp,i,&err,tstr);
			fprintf(stderr,"Reading the image file:%s\n",tstr);
			bin_io(tstr,'r',data,dx,dy,dz,1,swab,0L);
/* place into the Re array */
			for(k=0;k<(dx*dy);k++) {
				Im[j+k] = 0.0;
				switch (dz) {
					case 1:
					    Re[j+k]=((unsigned char *)data)[k];
					    break;
					case 2:
					    Re[j+k]=((unsigned short *)data)[k];
					    break;
					case 4:
					    Re[j+k]=((float *)data)[k];
					    break;
				}
				Re[j+k] += offset;
			}

		}
		j = j + (dx*dy);
	}


/* perform wrap-around if needed */
	if ((wrap) && (!reverse)) {
		fprintf(stderr,"Computing wrap-around\n");
		d[0] = dx; d[1] = dy; d[2] = icount;
		wrap_around3(Re,d,oneD);
	}

	if (oneD) {
		float	*fRe,*fIm;

/* build the axis vector */
		d[0] = icount;
		di[0] = icount;

		fRe = (float *)malloc(icount*sizeof(float));
		fIm = (float *)malloc(icount*sizeof(float));
		if (!fRe || !fIm) ex_err("Unable to allocate temp memory\n");

/* compute a series of 1D FFTs */
		for(i=0;i<dx*dy;i++) {

			for(j=0;j<icount;j++) {
				fRe[j] = Re[i + j*(dx*dy)];
				fIm[j] = Im[i + j*(dx*dy)];
			}

			if (reverse) {
				if (fftnf(1,di,fRe,fIm,-1,(double)(d[0]))) {
					ex_err("Error encountered during FFT");
				}
			} else {
				if (fftnf(1,di,fRe,fIm,+1,0.0)) {
					ex_err("Error encountered during FFT");
				}
			}

			for(j=0;j<icount;j++) {
				Re[i + j*(dx*dy)] = fRe[j];
				Im[i + j*(dx*dy)] = fIm[j];
			}
		}

		free(fRe);
		free(fIm);

	} else {
/* compute the 3D FFT */

/* build the axis vector */
		d[0] = dx; d[1] = dy; d[2] = icount;
		di[0] = dx; di[1] = dy; di[2] = icount;

		if (reverse) {
			fprintf(stderr,"Computing %dx%dx%d reverse FFT\n",
				dx,dy,icount);
			if (fftnf(3,di,Re,Im,-1,(double)(d[0]*d[1]*d[2]))) {
				ex_err("Error encountered during FFT");
			}
		} else {
			fprintf(stderr,"Computing %dx%dx%d forward FFT\n",
				dx,dy,icount);
			if (fftnf(3,di,Re,Im,+1,0.0)) {
				ex_err("Error encountered during FFT");
			}
		}
	}

/* perform wrap-around if needed */
	if ((wrap) && (reverse)) {
		fprintf(stderr,"Computing wrap-around\n");
		wrap_around3(Re,d,oneD);
	}

/* write the output */
	j = 0;
	for(i=istart;i<=iend;i=i+istep) {
		if (!reverse) {
			name_changer(Retemp,i,&err,tstr);
			fprintf(stderr,"Writing the Real file:%s\n",tstr);
			bin_io(tstr,'w',&(Re[j]),dx,dy,4,1,swab,0L);

			name_changer(Imtemp,i,&err,tstr);
			fprintf(stderr,"Writing the Imaginary file:%s\n",tstr);
			bin_io(tstr,'w',&(Im[j]),dx,dy,4,1,swab,0L);
		} else {
/* Extract from the Re array */
			for(k=0;k<(dx*dy);k++) {
				v = Re[j+k] + offset;
				switch (dz) {
					case 1:
					    if (v < 0) v = 0;
					    if (v > 255) v = 255;
					    ((unsigned char *)data)[k] = v;
					    break;
					case 2:
					    if (v < 0) v = 0;
					    if (v > 65535) v = 65535;
					    ((unsigned char *)data)[k] = v;
					    break;
					case 4:
					    ((float *)data)[k] = v;
					    break;
				}
			}
/* write slice to disk */
			name_changer(intemp,i,&err,tstr);
			fprintf(stderr,"Writing the image file:%s\n",tstr);
			bin_io(tstr,'w',data,dx,dy,dz,1,swab,0L);
		}
		j = j + (dx*dy);
	}

/* clean up the fft routine */
	fft_free();

/* return the memory */	
	free(data);
	free(Re);
	free(Im);

	tal_exit(0);

	exit(0);
}

/* local helpers */
static void linear_xform_even(float *data,int st,int n,int inc);
static void linear_xform_odd(float *data,int st,int n,int inc);

/* shift dimensions from 0-d to -d/2 to d/2 */
void wrap_around3(float *data, int *d, int oneD)
{
	long int x,y,z;
if (!oneD) {
/* along x scan lines */
	for(z=0;z<d[2];z++) {
	for(y=0;y<d[1];y++) {
		x = (y*d[0]) + (z*d[0]*d[1]);
		if (d[0] & 1) {
			linear_xform_odd(data,x,d[0],1);
		} else {
			linear_xform_even(data,x,d[0],1);
		}
	}
	}
/* along y scan lines */
	for(z=0;z<d[2];z++) {
	for(x=0;x<d[0];x++) {
		y = (x) + (z*d[0]*d[1]);
		if (d[1] & 1) {
			linear_xform_odd(data,y,d[1],d[0]);
		} else {
			linear_xform_even(data,y,d[1],d[0]);
		}
	}
	}
}
/* along z scan lines  (only case for 1D transform) */
	for(y=0;y<d[1];y++) {
	for(x=0;x<d[0];x++) {
		z = (x) + (y*d[0]);
		if (d[2] & 1) {
			linear_xform_odd(data,z,d[2],d[0]*d[1]);
		} else {
			linear_xform_even(data,z,d[2],d[0]*d[1]);
		}
	}
	}
/* done ! */
	return;
}

/*
even xform:
	0 1 2 3 -> 2 3 0 1
*/
static void linear_xform_even(float *data,int st,int n,int inc)
{
	long int 	i,j,x;
	float		tmp;

/* setup */
	i = st;
	j = (n/2);
	j = (j * inc) + st;

	for(x=0;x<(n/2);x++) {
/* swap (j) and (i) */
		tmp = data[i];
		data[i] = data[j];
		data[j] = tmp;
		i += inc;
		j += inc;
	}
	return;
}

/*
odd xform:
	0 1 2 3 4 => 2 3 4 0 1

	Current implementation sucks!
*/
static void linear_xform_odd(float *data,int st,int n,int inc)
{
	long int 	i,j,x;
	float		tmp[2049];

	if (n > 2049) ex_err("Unable to buffer odd tranforms > 2049");
/* setup */
	i = st;
/* copy */
	for(x=0;x<n;x++) {
		tmp[x] = data[i];
		i += inc;
	}
/* wrap */
	j = n/2;
	i = st;
	for(x=0;x<n;x++) {
		data[i] = tmp[j];
		i += inc;
		j += 1;
		if (j >= n) j -= n;
	}
	return;
}
