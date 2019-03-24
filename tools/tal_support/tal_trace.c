/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski
 *
 * $Id: tal_trace.c 1213 2005-08-27 20:51:21Z rjfrank $
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
#include "autotrace.h"
#include "roi_utils.h"
#include "polyfill.h"

#define MIN(a,b) (((a)<(b)?(a):(b)))
#define MAX(a,b) (((a)>(b)?(a):(b)))

/* callback functions */
void    vl_puts(char *in)
{
        fprintf(stderr,"%s\n",in);
}

void ex_err(char *s)
{
	fprintf(stderr,"Fatal error:%s\n",s);
	tal_exit(1);
}

void cmd_err(char *s)
{
	fprintf(stderr,"(%s:%s) Usage: %s [options] inputtemp roibase\n",__DATE__,TAL_VERSION,s);
	fprintf(stderr,"Options: -x(dx) image dx default:256\n");
	fprintf(stderr,"         -y(dy) image dy default:256\n");
	fprintf(stderr,"         -z(dz) image depth in bytes default:1\n");
	fprintf(stderr,"         -f(start) first image number default:1\n");
	fprintf(stderr,"         -l(end) last image number default:124\n");
	fprintf(stderr,"         -i(step) image number step default:1\n");
	fprintf(stderr,"         -b swap bytes with 16bit images default:no\n");
	fprintf(stderr,"         -t(threshold) value to autotrace at default:1\n");
	fprintf(stderr,"         -s(scale) ROI scaling factor default:2.0\n");
        fprintf(stderr,"         -noflip do not flip ROIs over the X axis default:flip them\n");
        fprintf(stderr,"         -omax pixels outside the image are maximum valued default:minumum\n");
	fprintf(stderr,"Note: float input volumes not currently supported.\n");
	tal_exit(1);
}

#define M_PTS 20000

int main(int argc,char **argv)
{
	long int	dx = 256;
	long int	dy = 256;
	long int	dz = 1;
	long int	iOutMax = -1;
	long int	istart = 1;
	long int	iend = 124;
	long int	istep = 1;
	long int	swab = 0;
	long int	thres = 1;
        long int        flipit = 1;
        double          scale = 2.0;

	char		intemp[256],outtemp[256];
	char		tstr[256],tstr2[256];
	long int	i,err,j,i1,i2;
	unsigned char	*mask;
	void 		*timg;
	long int	npts;
	long int	pnts[M_PTS],strips[M_PTS],ptr;
	long int	x,y,roi_count;
	Trace		roi;

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
				if ((dz != 1) && (dz != 2)) {
				  ex_err("Only 8 and 16 bit volume supported.");
				}
				break;
			case 't':
				thres = atoi(&(argv[i][2]));
				break;
			case 'b':
				swab = 1;
				break;
                        case 'n':    /* -noflip */
                                flipit = 0;
                                break;
                        case 'o':    /* -omax */
                                iOutMax = 1;
                                break;
                        case 's':
                                scale = atof(&(argv[i][2]));
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

/* setup the roi routines */
	init_roi_utils();
        bin_roi_flip(flipit);
        bin_roi_scale(scale);

/* get the image memory */
	timg = (void *)malloc(dz*dx*dy);
	if (timg == 0L) ex_err("Unable to allocate image memory.");

	mask = malloc(dx*dy);
	if (mask == 0L) ex_err("Unable to allocate image memory.");

/* process the images */
	for(i=istart;i<=iend;i=i+istep) {
		name_changer(intemp,i,&err,tstr);
		fprintf(stderr,"Reading the file:%s\n",tstr);
		bin_io(tstr,'r',timg,dx,dy,dz,flipit,swab,0L);
		for(j=0;j<dx*dy;j++) mask[j] = 0;
		roi_count = 0;
		for(x=0;x<dx;x++) {
		for(y=0;y<dy;y++) {
			i1 = grid(x,y,dx,dy,dz*iOutMax,timg);
			i2 = grid(x+1,y,dx,dy,dz*iOutMax,timg);

/* if masks are clear and we straddle a contour */
			if ( (mask[(y*dx)+x] == 0) &&
/*			     (mask[(y*dx)+x+1] == 0) && */
			     (((i1 > thres) && (i2 <= thres)) ||
			     ((i1 <= thres) && (i2 > thres)))) {
				npts = 10000;
				autotrace_2d(timg,dx,dy,dz*iOutMax,thres,x,y,
					pnts,&npts);
				if (npts > 6) {
/* create the ROI */
					init_roi(&roi,"junk");
					for(j=0;j<npts/2;j++) {
					    roi.thepnts[roi.num_points].x = 
						pnts[j*2];
					    roi.thepnts[roi.num_points].y = 
						pnts[j*2+1];
					    roi.num_points += 1;
					}
/* fill the ROI in the mask */
					ptr = M_PTS;
					polyfill_c_(pnts,npts,strips,&ptr);
					if (ptr > 0) {
					    long int	x2,y2,st,en;
					    for(j=0;j<ptr;j=j+3) {
                                        	y2 = strips[j+2];
                                        	if (y2 <= 0) y2 = 1;
                                        	if (y2 >= dy-1) y2 = dy-2;
                                        	st = strips[j]-1;
                                        	if (st < 0) st = 0;
                                        	en = strips[j+1]+1;
                                        	if (en >= dx) en = dx-1;
                                        	for(x2=st;x2<=en;x2++) {
                                                    mask[(y2*dx)+x2] = 1;
                                                    mask[((y2-1)*dx)+x2] = 1;
                                                    mask[((y2+1)*dx)+x2] = 1;
						}
                                            }
					}
/* build an ROI name and save */
					name_changer(outtemp,i,&err,tstr);
					strcat(tstr,"_%%%.roi");
					name_changer(tstr,roi_count,&err,tstr2);
					if (roi_count > 999) {
			ex_err("Too many ROIs encoutered on one slice.");
					}
/* flipit handled by reading the image... */
					if (scale != 1.0) {
						for(j=0;j<roi.num_points;j++) {
    						    roi.thepnts[j].y *= scale;
    						    roi.thepnts[j].x *= scale;
						}
					}
					save_roi(tstr2,&roi);
					roi_count += 1;
				}
			}
			mask[(y*dx)+x] = 1;
		}
		}
	}



/* done */	
	free(mask);
	free(timg);

	tal_exit(0);

	exit(0);
}
