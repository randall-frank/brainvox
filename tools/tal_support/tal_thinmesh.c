/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski
 *
 * $Id: tal_thinmesh.c 1213 2005-08-27 20:51:21Z rjfrank $
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

#define NEW_DECIMATE_CODE

int	verbose = 0;

long int trim_polys(long int *n_tris,long int *objs,long int *n_verts,
        float *verts,long int n_wanted,long int verb);
long int decimate_mesh(long int *n_tris,long int *objs,long int *n_verts,
        float *verts,long int n_wanted,long int block_size,long int flags,
        long int n_vaux, float *aux,float *auxweights);
void calc_normal(long int a,long int b,long int c,float *verts,double *r);

void ex_err(char *s)
{
	fprintf(stderr,"Fatal error:%s\n",s);
	tal_exit(1);
}

void cmd_err(char *s)
{
	fprintf(stderr,"(%s:%s) Usage: %s [options] inputfile outputfile\n",__DATE__,TAL_VERSION,s);
#ifdef NEW_DECIMATE_CODE
	fprintf(stderr,"Options: -t(thin) fraction of triangles to remove default:0.0\n");
	fprintf(stderr,"         -b(size) block triangle count fraction default:0.1\n");
#else
	fprintf(stderr,"Options: -t(thin) fraction of verticies to remove default:0.0\n");
#endif
	fprintf(stderr,"         -v enable verbose mode\n");
	fprintf(stderr,"         -g read/write in geomview format default:OBJ\n");
	tal_exit(1);
}

int main(int argc,char **argv)
{
	double		percent = 0.0;
	double		blocksize = 0.1;
	int		noff = 0;

	float		*verts = 0L,*norms = 0L;
	long int	*tris = 0L,n_verts,n_tris;

	char		infile[256],outfile[256];
	long int	i,k;
	float		d;
	double		v[3];

	i = 1;
	while ((i < argc) && (argv[i][0] == '-')) {
		if (argv[i][1] == '\0') break;
		switch (argv[i][1]) {
			case 'b':
				blocksize = atof(&(argv[i][2]));
				break;
			case 't':
				percent = atof(&(argv[i][2]));
				break;
			case 'v':
				verbose = 1;
				break;
			case 'g':
				noff = 1;
				break;
			default:	
				cmd_err(argv[0]);
				break;
		}
		i++;
	}
	if ((argc-i) != 2) cmd_err(argv[0]);
	strcpy(infile,argv[i]);
	strcpy(outfile,argv[i+1]);

/* read the surface */
	if (noff) {
		i = read_geom_file(infile,&n_verts,&n_tris,&verts,&norms,&tris);
	} else {
		i = read_OBJ_file(infile,&n_verts,&n_tris,&verts,&norms,&tris);
	}
	if (i != 0) ex_err("Unable to read input file");

/* thin to win */
	if (percent > 1.0) percent = 0.99;
	if (percent < 0.0) percent = 0.0;

#ifdef NEW_DECIMATE_CODE
	percent = (1.0-percent)*(double)n_tris;
	blocksize = percent*0.5*blocksize;
	if (verbose) fprintf(stderr,"Reducing from %ld to %ld tris\n",
		n_tris,(long int)percent);
	i = decimate_mesh(&n_tris,tris,&n_verts,verts,(long int)percent,
		(long int)blocksize,1,0,NULL,NULL);
	if (i == 0) ex_err("Unable to complete operation");
#else
	percent = (1.0-percent)*(double)n_verts;
	if (verbose) fprintf(stderr,"Reducing from %ld to %ld verts\n",
		n_verts,(long int)percent);
	i = trim_polys(&n_tris,tris,&n_verts,verts,(long int)percent,verbose);
	if (i != 0) ex_err("Unable to complete operation");
#endif

/* recompute surface normals */
	for(i=0;i<n_verts*3;i++) norms[i] = 0.0;
/* sum them */
	for(i=0;i<n_tris*3;i+=3) {
		calc_normal(tris[i],tris[i+1],tris[i+2],verts,v);
		for(k=0;k<3;k++) {
			norms[(tris[i+0]*3)+k] += v[k];
			norms[(tris[i+1]*3)+k] += v[k];
			norms[(tris[i+2]*3)+k] += v[k];
		}
	}
/* renormalize */
	for(i=0;i<n_verts*3;i+=3) {
		d  = (norms[i+0]*norms[i+0]);
		d += (norms[i+1]*norms[i+1]);
		d += (norms[i+2]*norms[i+2]);
		if (d > 0.0) {
			d = sqrt(d);
			norms[i+0] /= d;
			norms[i+1] /= d;
			norms[i+2] /= d;
		} else {
			norms[i+0] = 0.0;
			norms[i+1] = 0.0;
			norms[i+2] = 1.0;
		}
	}

/* save the surface */
	if (noff) {
		i = write_geom_file(outfile,n_verts,n_tris,verts,norms,tris);
	} else {
		i = write_OBJ_file(outfile,n_verts,n_tris,verts,norms,tris);
	}

/* free up memory */
	if (tris) free(tris);
	if (norms) free(norms);
	if (verts) free(verts);

	tal_exit(0);

	exit(0);
}
