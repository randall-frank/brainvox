/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski
 *
 * $Id: crop_volume.h 1250 2005-09-16 15:51:42Z dforeman $
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


static void BASE(smoothx_)(DTYPE *image, long d[3]);
static void BASE(smoothy_)(DTYPE *image, long d[3]);
static void BASE(smoothz_)(DTYPE *image, long d[3]);

#define DX d[0]
#define DY d[1]
#define DZ d[2]

static void BASE(smoothx_)(DTYPE *image, long d[3])
{
        register long x,y,z;
	float	      p1,p2,p3;

/* smooth along X scanlines using 242 kernel */
	for(z=0;z<DZ;z++) {
        for(y=0;y<DY;y++) {
                p1 = image[(z*DX*DY)+(y*DX)];
                p2 = image[(z*DX*DY)+(y*DX)+1];
                for(x=1;x<DX-1;x++) {
                        p3 = image[(z*DX*DY)+(y*DX)+x+1];
                        image[(z*DX*DY)+(y*DX)+x] = ((p1*2)+(p2*4)+(p3*2))/8;
                        p1 = p2;
                        p2 = p3;
                }
        }
	}
        return;
}

static void BASE(smoothy_)(DTYPE *image, long d[3])
{
        register long x,y,z;
	float	      p1,p2,p3;

/* smooth along Y scanlines using 242 kernel */
        for(z=0;z<DZ;z++) {
        for(x=0;x<DX;x++) {
                p1 = image[(z*DX*DY)+x];
                p2 = image[(z*DX*DY)+x+DX];
                for(y=1;y<DY-1;y++) {
                        p3 = image[(z*DX*DY)+(y*DX)+x+DX];
                        image[(z*DX*DY)+(y*DX)+x] = ((p1*2)+(p2*4)+(p3*2))/8;
                        p1 = p2;
                        p2 = p3;
                }
        }
	}
        return;
}

static void BASE(smoothz_)(DTYPE *image, long d[3])
{
        register long x,y,z;
	float	      p1,p2,p3;

/* smooth along Y scanlines using 242 kernel */
        for(y=0;y<DY;y++) {
        for(x=0;x<DX;x++) {
                p1 = image[(y*DX)+x];
                p2 = image[(y*DX)+x+(DX*DY)];
                for(z=1;z<DZ-1;z++) {
                        p3 = image[(z*DX*DY)+(y*DX)+x+(DX*DY)];
                        image[(z*DX*DY)+(y*DX)+x] = ((p1*2)+(p2*4)+(p3*2))/8;
                        p1 = p2;
                        p2 = p3;
                }
        }
	}
        return;
}

void BASE(Sample3D_)(DTYPE *in,long int id[3],
		DTYPE *out,long int od[3],
		long int origin[3],long int size[3],long int filter)
{
        register double xinc,yinc,zinc,xp,yp,zp;
        register long x,y,z;
        register long i,j,k,l;
        register long ptr;

        xinc = (double)size[0] / (double)od[0];
        yinc = (double)size[1] / (double)od[1];
        zinc = (double)size[2] / (double)od[2];

/* prefilter if decimating */
        if (filter) {
                if (xinc > 1.0) BASE(smoothx_)(in,id);
                if (yinc > 1.0) BASE(smoothy_)(in,id);
                if (zinc > 1.0) BASE(smoothz_)(in,id);
        }
/* resample */
        ptr = 0;
	zp = origin[2];
	for(z=0;z<od[2];z++) { /* over all planes in output volume */
		k = zp;
        	yp = origin[1];
        	for(y=0;y<od[1];y++) { /* over all scan lines in output plane */
                	j = yp;
                	xp = origin[0];
					/* over all pixels in each scanline */
			l = (j*id[0]) + (k*id[0]*id[1]);
                	for(x=0;x<od[0];x++) { 
                        	i = xp;
                        	out[ptr++] = in[i+l];
                        	xp += xinc;
                	}
                	yp += yinc;
		}
		zp += zinc;
        }
/* postfilter if magnifing */
        if (filter) {
                if (xinc < 1.0) BASE(smoothx_)(out,od);
                if (yinc < 1.0) BASE(smoothy_)(out,od);
                if (zinc < 1.0) BASE(smoothy_)(out,od);
        }
        return;
}  
