/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski, M.D.
 *
 * $Id: tal_roi2mesh.c 1213 2005-08-27 20:51:21Z rjfrank $
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
#include "roi_utils.h"
#include "proto.h"
#include "geom_view_io.h"

#define MAX_ROIS 300

void ex_err(char *s)
{
        fprintf(stderr,"Fatal error:%s\n",s);
        tal_exit(1);
}

void cmd_err(char *s)
{
	fprintf(stderr,"(%s:%s) Usage: %s [options] outputfile\n",__DATE__,TAL_VERSION,s);
	fprintf(stderr,"Options: -t(thin) fraction to thin contours by (default:1.0)\n");
	fprintf(stderr,"         -o(x:y:z) origin coords to subtract (default:0:0:0)\n");
	fprintf(stderr,"         -m(x:y:z) scale to multiply by (default:1:1:1)\n");
	fprintf(stderr,"         -g write in geomview format (default:OBJ)\n");
	fprintf(stderr,"Stdin consists of lines of:\nROINAME ZDEPTH\n");
	fprintf(stderr,"One for each contour in the object.  The ROIs can be presented in any order.\n");

	tal_exit(1);
}

int main(int argc,char **argv)
{
/* params */
	double		thin = 1.0;
	double		origin[3] = {0,0,0};
	double		scale[3] = {1.0,1.0,1.0};
	int		noff = 0;

/* vars */
	char		tstr[256];
	long int	i,j,k,cnt;
	char		t2[256];
	char		*ofile;
	long int	n_rois,n_verts;

/* roi vars and work verts */	
	float		depth[MAX_ROIS],d,p[3];
	float		p0[3],p1[3],p2[3],v1[3],v2[3];
	float		norm[3],*norms,*verts;
	Trace		*roi[MAX_ROIS],*troi;
	Trace		*pair[2];

/* triangle vars */
	long int	*tris,num,n_tris,alloc_tris;

	long int	vsum[MAX_ROIS];
	long int	*v,*c;

/* command line */
	if (argc < 2) cmd_err(argv[0]);
/* get options */
	i = 1;
	while ((argv[i][0] == '-') && (argv[i][1] != '\0')) {
		switch(argv[i][1]) {
			case 'o':
				get_d_xyz(origin,&(argv[i][2]));
				break;
			case 'm':
				get_d_xyz(scale,&(argv[i][2]));
				break;
			case 't':
				thin = atof(&(argv[i][2]));
				break;
			case 'g':
				noff = 1;
				break;
			default:
				fprintf(stderr,"Unknown option: %s\n",argv[i]);
				tal_exit(1);
			break;
		}
		i += 1;
		if (argv[i] == 0L) ex_err("No output filename specified");
	}

/* get filename */
	if (i >= argc) ex_err("No output filename specified");
	ofile = argv[i];

/* setup the roi routines */
        init_roi_utils();

/* read the rois */
	n_rois = 0;
	while(fgets(tstr,256,stdin) != 0l) {
		sscanf(tstr,"%s %f\n",t2,&(depth[n_rois]));
		roi[n_rois] = (Trace *)malloc(sizeof(Trace));
		if (roi[n_rois] == 0L) ex_err("Unable to allocate ROI memory");
		if (read_roi(t2,roi[n_rois])) {
			fprintf(stderr,"Unable to read %s, file skipped.\n",t2);
			continue;
		} else {
			roi_CCW(roi[n_rois]);
			n_rois += 1;
		}
		if (n_rois >= MAX_ROIS) {
			ex_err("Maximum number of ROIs exceeded");
		}
	}
	if (n_rois < 2) {
		ex_err("At least two contours required for triangulation.");
	}

/* sort by depth (brutish) */
	for(i=0;i<n_rois-1;i++) {
	for(j=i+1;j<n_rois;j++) {
		if (depth[i] > depth[j]) {
			troi = roi[j];
			roi[j] = roi[i];
			roi[i] = troi;
			d = depth[j];
			depth[j] = depth[i];
			depth[i] = d;
		}
	}
	}

/* smooth the roi coordinates */
	n_verts = 0;
	vsum[0] = 0;
	if (thin < 0.001) thin = 0.001;
	if (thin > 1.0) thin = 1.0;
	for(i=0;i<n_rois;i++) {
		j = thin*roi[i]->num_points;
		sample_roi(roi[i],j);
		n_verts += roi[i]->num_points;
		vsum[i+1] = vsum[i] + roi[i]->num_points;
	}

/* allocate space for normals */
	norms = (float *)malloc(n_verts*sizeof(float)*3);
	if (norms == 0l) ex_err("Unable to allocate normal memory");
	for(i=0;i<n_verts*3;i++) norms[i] = 0.0;

/* compute the triangulations (vvv,vvv,...) */
	n_tris = 0;
	alloc_tris = 5000;
	tris = (long int *)malloc(3*alloc_tris*sizeof(long int));
	if (tris == 0L) ex_err("Unable to allocate triangle memory");

/* extra memory to hold slice pair triangulations */
	v = (long int *)malloc(MAX_POINTS*2*3*sizeof(long int));
	if (v == 0L) ex_err("Unable to allocate vertex memory");
	c = (long int *)malloc(MAX_POINTS*2*3*sizeof(long int));
	if (c == 0L) ex_err("Unable to allocate contour level memory");

/* for each ROI pair */
	for(i=0;i<n_rois-1;i++) {

/* compute the triangulation */
		num = MAX_POINTS*2;
		pair[0] = roi[i];
		pair[1] = roi[i+1];
		triangulate_rois(pair,depth[i+1]-depth[i],&num,v,c);

/* store the triangles */
/* more space? */
		if (n_tris+num > alloc_tris-2) {
			alloc_tris += 500;
			tris = (long int *)realloc(tris,
				3*alloc_tris*sizeof(long int));
		}
		for(j=0;j<num*3;j+=3) {

/* find the proper vertex indicies */
			tris[n_tris*3+0] = v[j]+vsum[i+c[j]];
			tris[n_tris*3+1] = v[j+1]+vsum[i+c[j+1]];
			tris[n_tris*3+2] = v[j+2]+vsum[i+c[j+2]];

/* compute the normal for the triangle */
			p0[0] = roi[i+c[j]]->thepnts[v[j]].x;
			p0[1] = roi[i+c[j]]->thepnts[v[j]].y;
			p0[2] = depth[i+c[j]];

			p1[0] = roi[i+c[j+1]]->thepnts[v[j+1]].x;
			p1[1] = roi[i+c[j+1]]->thepnts[v[j+1]].y;
			p1[2] = depth[i+c[j+1]];

			p2[0] = roi[i+c[j+2]]->thepnts[v[j+2]].x;
			p2[1] = roi[i+c[j+2]]->thepnts[v[j+2]].y;
			p2[2] = depth[i+c[j+2]];

			for(k=0;k<3;k++) p0[k] = (p0[k] - origin[k])*scale[k];
			for(k=0;k<3;k++) p1[k] = (p1[k] - origin[k])*scale[k];
			for(k=0;k<3;k++) p2[k] = (p2[k] - origin[k])*scale[k];

			for(k=0;k<3;k++) {
				v1[k] = p0[k] - p1[k];
				v2[k] = p2[k] - p1[k];
			}

			norm[0] = v1[1]*v2[2]-v2[1]*v1[2];
			norm[1] = v1[2]*v2[0]-v2[2]*v1[0];
			norm[2] = v1[0]*v2[1]-v2[0]*v1[1];

			d = norm[0]*norm[0];
			d += norm[1]*norm[1];
			d += norm[2]*norm[2];
			if (d <= 0) d = 1.0;
			d = sqrt(d);

			norm[0] /= d;
			norm[1] /= d;
			norm[2] /= d;

/* sum the normals into each vertex */
			k = tris[n_tris*3+0];
			norms[k*3] += norm[0];
			norms[k*3+1] += norm[1];
			norms[k*3+2] += norm[2];
			k = tris[n_tris*3+1];
			norms[k*3] += norm[0];
			norms[k*3+1] += norm[1];
			norms[k*3+2] += norm[2];
			k = tris[n_tris*3+2];
			norms[k*3] += norm[0];
			norms[k*3+1] += norm[1];
			norms[k*3+2] += norm[2];

/* add the triangle */
			n_tris += 1;
		}
	}

/* renormalize the normal sums */
	for(i=0;i<n_verts*3;i+=3) {
		d = (norms[i]*norms[i]);
		d += (norms[i+1]*norms[i+1]);
		d += (norms[i+2]*norms[i+2]);
		if (d <= 0.0) d = 1.0;
		d = sqrt(d);
		norms[i] /= d;
		norms[i+1] /= d;
		norms[i+2] /= d;
	}

	verts=(float *)malloc(n_verts*3*sizeof(float));
	if (verts == 0L) ex_err("Unable to allocate vertex memory.");

/* verts */
	cnt = 0;
	for(i=0;i<n_rois;i++) {
		for(j=0;j<roi[i]->num_points;j++) {
			p[0] = roi[i]->thepnts[j].x;
			p[1] = roi[i]->thepnts[j].y;
			p[2] = depth[i];

/* transform (p[]-origin[])*scale[] */
			for(k=0;k<3;k++) p[k] = (p[k] - origin[k])*scale[k];
			verts[cnt] = p[0];
			verts[cnt+1] = p[1];
			verts[cnt+2] = p[2];

			cnt += 3;
		}
	}

/* write the file (for geomview) */
	if (noff) {
		i = write_geom_file(ofile,n_verts,n_tris,verts,norms,tris);
	} else {
		i = write_OBJ_file(ofile,n_verts,n_tris,verts,norms,tris);
	}
	if (i) {
		fprintf(stderr,"Unable to write the file %s\n",ofile);
	}

	
/* done */
	free(norms);
	free(tris);
	free(v);
	free(c);
	free(verts);

	for(i=0;i<n_rois;i++) {
		free(roi[i]);
	}

	tal_exit(0);
	exit(0);
}
