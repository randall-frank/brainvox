/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski
 *
 * $Id: tal_roi_sample.c 1213 2005-08-27 20:51:21Z rjfrank $
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

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef struct {
	double x1;
	double y1;
	double z1;
	double x2;
	double y2;
	double z2;
	double c;
	double v[3];
	double d;
}	pointvec;

/* global data for the "pie"+cylinder form */
#define MAX_ANG	6
#define MAX_DISKS 200

typedef struct {
	long int	num_traces;
	long int 	num_disks;
	pointvec	cent_axis;
	double		disk_pnt[MAX_DISKS][MAX_ANG][3];
	double		disk_pnt_ang[MAX_DISKS][MAX_ANG];
	double		disk_cent[MAX_DISKS][3];
} Pie_Globals;

	Pie_Globals	gp;

        double          ipixel[2] = {1.0,1.0};
        double          islice = 6.64;
	double		squf = 1.0;
	long int	xystep = 2;
	long int	lefthand = 0;
	long int	verb = 0;

void resample_w_roi(unsigned char *data,long int dx,long int dy,long int dz,
	double cnst,unsigned char *mask,long int nsets,
	pointvec *pnts,FILE *ofp,double z,long int rad);

void data_output(double pdata,long int nsets,pointvec *pnts,double x,
	double y,double z,FILE *ofp);

void rad_data_output(double pdata,double x,double y,double z,FILE *ofp);

void read_pointset(char *file,long int *npts,long int *v);
void smooth_pointset(long int num,long int *v,long int n);

void	calc_angle_tables(pointvec P,long int nangles,
	float *v[MAX_ANG],long int n[MAX_ANG],long int avg_cent);

double	vec_angle(double *v1,double *v2,double *z);
void cross_prod(double *v1,double *v2,double *x);
double dot_prod(double *v1,double *v2);
void normalize(double *v);

void ex_err(char *s)
{
	fprintf(stderr,"Fatal error:%s\n",s);
	tal_exit(1);
}

void cmd_err(char *s)
{
	fprintf(stderr,"(%s:%s) Usage: %s [options] inputtemp roitemp outfile [pointset, pointset ...]\n",__DATE__,TAL_VERSION,s);
	fprintf(stderr,"Options: -x(dx) image dx default:256\n");
	fprintf(stderr,"         -y(dy) image dy default:256\n");
	fprintf(stderr,"         -z(dz) image depth in bytes default:2\n");
	fprintf(stderr,"         -k(xystep) pixel step in xy plane default:2\n");
	fprintf(stderr,"         -f(start) first image number default:1\n");
	fprintf(stderr,"         -l(end) last image number default:15\n");
	fprintf(stderr,"         -i(step) image number step default:1\n");
	fprintf(stderr,"         -p(ipx[:ipy]) interpixel spacing default:1.0[:1.0]\n");
	fprintf(stderr,"         -s(interslice) interslice spacing default:6.64\n");
	fprintf(stderr,"         -b swap bytes with 16bit images default:no\n");
	fprintf(stderr,"         -c(const) additive constant default:-256\n");
	fprintf(stderr,"         -r(scale) ROI scaling factor default:0.5\n");
	fprintf(stderr,"         -noflip do not flip ROIs over the X axis default:flip them\n");
	fprintf(stderr,"         -Radial use radial axis system (requires at least 3 pointsets)\n");
	fprintf(stderr,"         -Left use lefthand rule for radial axis default:righthand rule\n");
	fprintf(stderr,"         -S smooth the poinset before use default:no\n");
	fprintf(stderr,"         -v enable verbose mode default:quiet\n");
	fprintf(stderr,"         Output = (Pixel + const) inside ROI\n");
	tal_exit(1);
}

#define MAX_VECS 20
#define M_PTS 10000

int main(int argc,char **argv)
{
	long int	dx = 256;
	long int	dy = 256;
	long int	dz = 2;
	long int	istart = 1;
	long int	iend = 15;
	long int	istep = 1;
	long int	swab = 0;
	long int	flip = 1;
	long int	radial = 0;
	double 		cnst = -256;
	double		roi_scale = 0.5;
	long int	avg_cent = 0;
	long int	smooth = 0;
	char		intemp[256],roitemp[256],outfile[256],tstr[256];
	long int	i,err,j;
	unsigned char 	*data,*mask;
	FILE		*ofp;
	pointvec	pnts[MAX_VECS];
	long int	num,list[M_PTS];
	long int	nsets,k;
	double		z,d;
	long int	nanglepts[MAX_ANG];
	float		*anglepts[MAX_ANG];

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
			case 'c':
				cnst = atof(&(argv[i][2]));
				break;
			case 'k':
				xystep = atoi(&(argv[i][2]));
				break;
                        case 'p':
                                get_d_xy(ipixel,&(argv[i][2]));
                                break;
                        case 's':
                                islice = atof(&(argv[i][2]));
                                break;
                        case 'r':
                                roi_scale = atof(&(argv[i][2]));
                                break;
			case 'n':
				flip = 0;
				break;
			case 'R':
				radial = 1;
				break;
			case 'L':
				lefthand = 1;
				break;
			case 'S':
				smooth = strlen(argv[i])-1;
				break;
			case 'b':
				swab = 1;
				break;
			case 'v':
				verb = 1;
				break;
			case 'a':
				avg_cent = strlen(argv[i])-1;
				if (avg_cent > 2) avg_cent = 2;
				break;
			default:	
				cmd_err(argv[0]);
				break;
		}
		i++;
	}
	if ((argc-i) < 3) cmd_err(argv[0]);
	strcpy(intemp,argv[i]);
	strcpy(roitemp,argv[i+1]);
	strcpy(outfile,argv[i+2]);
	i += 3;
        if (strcmp(outfile,"-") == 0L) {
                ofp = stdout;
        } else {
                ofp = fopen(outfile,"w");
                if (ofp == 0L) {
                        printf("Unable to open output file:%s\n",outfile);
                        tal_exit(1);
                }
        }
/* setup the bin_io parameters for ROIs */
	bin_roi_flip(flip);  
	bin_roi_scale(roi_scale);
	
/* compute squeeze factor 
 Compute the number of interpolated slices in the resampled dataset
*/
        squf = 10.0;
	k = iend-istart+1;
        j = -1;
        while ((squf > 1.0) && (j < 20)) {
		j = j + 1;
                squf = ((double)(k)*islice)/((double)(k + (j*(k-1)))*ipixel[0]);
	}
/* initial number of pointsets */
/* check valid number of pointsets */
	if (radial) {
		if ((argc-i) < 3) {
	ex_err("At least three pointsets are needed for radial sampling.");
		}
		nsets = argc-i-1;
		if (nsets > MAX_ANG) {
			ex_err("Maximum number of traces exceeded.");
		}
	}
/* read the pointsets (if any) */
	nsets = 0;
	while ((i < argc) && (nsets < (MAX_VECS-1))) {
		strcpy(tstr,argv[i]);
		i++;
		err = 0;
		read_pointset(tstr,&num,list);
		if (smooth > 0) smooth_pointset(num,list,smooth);
		if (num != 0) {
			j = 0;
/* base point */
			pnts[nsets].x1 = list[j++];
			pnts[nsets].y1 = list[j++];
			pnts[nsets].z1 = list[j++];
			pnts[nsets].c = list[j++];

			d = 0;
/* while distance is short and more points are waiting */
			while ((j<num*4) && (d < (3.5*3.5))) {
/* next point */
				pnts[nsets].x2 = list[j++];
				pnts[nsets].y2 = list[j++];
				pnts[nsets].z2 = list[j++];
				pnts[nsets].c = list[j++];
/* compute the vector */
				pnts[nsets].v[0] = pnts[nsets].x2 -
					pnts[nsets].x1;
				pnts[nsets].v[1] = pnts[nsets].y2 -
					pnts[nsets].y1;
				pnts[nsets].v[2] = pnts[nsets].z2 -
					pnts[nsets].z1;
/* get distance */
				d = dot_prod(pnts[nsets].v,pnts[nsets].v);
			}
			err = 0;
		}
		if (err) {
			fprintf(stderr,"Bad format in pointset file %s\n",tstr);
			fprintf(stderr,"Unable to read pointset file %s\n",tstr);
		} else {
/* convert to MM */
			pnts[nsets].x1 *= ipixel[0];
			pnts[nsets].y1 *= ipixel[1];
			pnts[nsets].x2 *= ipixel[0];
			pnts[nsets].y2 *= ipixel[1];
/* and the Z values... */
			pnts[nsets].z1 *= squf;
			pnts[nsets].z2 *= squf;
			pnts[nsets].z1 *= ipixel[0];
			pnts[nsets].z2 *= ipixel[0];
/* compute length and normalize the vector */
			pnts[nsets].v[0] = pnts[nsets].x2 - pnts[nsets].x1;
			pnts[nsets].v[1] = pnts[nsets].y2 - pnts[nsets].y1;
			pnts[nsets].v[2] = pnts[nsets].z2 - pnts[nsets].z1;
/* get distance */
			pnts[nsets].d = sqrt(dot_prod(pnts[nsets].v,
				pnts[nsets].v));
/* normalize */
			normalize(pnts[nsets].v);
if (verb) {
	fprintf(stderr,"Data vector %f %f %f - %f %f %f\n",pnts[nsets].x1,
		pnts[nsets].y1,pnts[nsets].z1,pnts[nsets].x2,pnts[nsets].y2,
		pnts[nsets].z2);
	fprintf(stderr,"%f %f %f %f\n",pnts[nsets].v[0],pnts[nsets].v[1],
		pnts[nsets].v[2],pnts[nsets].d);
}

			nsets++;
			fprintf(stderr,"Read vector %ld from the file %s\n",nsets,tstr);

/* if in radial mode, store all but the first pointset for later use */
			if ((radial) && (nsets > 1)) {
/* copy points from list (num*4) of them */
				nanglepts[nsets-2] = num;
				anglepts[nsets-2] = (float *)malloc(sizeof(float)*num*4);
				if (anglepts[nsets-2] == 0l) {
		ex_err("Unable to allocate memory for trace storage.\n");
				}
				for(j=0;j<num;j++) {
				    anglepts[nsets-2][(j*4)+0] = list[(j*4)+0];
				    anglepts[nsets-2][(j*4)+1] = list[(j*4)+1];
				    anglepts[nsets-2][(j*4)+2] = list[(j*4)+2];
				    anglepts[nsets-2][(j*4)+3] = list[(j*4)+3];
/* convert to MM */
				    anglepts[nsets-2][(j*4)+0] *= ipixel[0];
				    anglepts[nsets-2][(j*4)+1] *= ipixel[1];
				    anglepts[nsets-2][(j*4)+2] *= squf;
				    anglepts[nsets-2][(j*4)+2] *= ipixel[0];
				}
			}

		}
	}
/* if in radial mode, build the tables */
	if (radial) {
		calc_angle_tables(pnts[0],nsets-1,anglepts,nanglepts,avg_cent);
	}
	
/* get the image memory */
	data = malloc(dz*dx*dy);
	if (data == 0L) ex_err("Unable to allocate image memory.");
	mask = malloc(dx*dy);
	if (mask == 0L) ex_err("Unable to allocate image memory.");
/* read the images */
	for(i=istart;i<=iend;i=i+istep) {
		name_changer(intemp,i,&err,tstr);
		fprintf(stderr,"Reading the image file:%s\n",tstr);
		bin_io(tstr,'r',data,dx,dy,dz,1,swab,0L);

/* ROImask image */
		for(j=0;j<(dx*dy);j++) mask[j] = 0;
		if (strcmp(roitemp,"-") == 0L) {
			fprintf(stderr,"Using whole image\n");
			for(j=0;j<(dx*dy);j++) mask[j] = 1;
			num = 1;
		} else {
			name_changer(roitemp,i,&err,tstr);
			bin_io(tstr,'r',mask,dx,dy,1,1,0,0L);
			num = 1;
		}
		z = (i-1)*islice;
		if (num != 0) {
			resample_w_roi((unsigned char *)data,dx,dy,dz,cnst,
				mask,nsets,pnts,ofp,z,radial);
		}
	}
/* free up the memory */
	free(data);
	free(mask);
	if (radial) {
		for(i=1;i<nsets;i++) {
			free(anglepts[i-1]);
		}
	}
/* and exit */
	tal_exit(0);
	exit(0);
}

void resample_w_roi(unsigned char *data,long int dx,long int dy,long int dz,
	double cnst,unsigned char *mask,long int nsets,
	pointvec *pnts,FILE *ofp,double z,long int radial)
{
        long int        x,y,j;
	unsigned short  *sdata;
	float 		*fdata;
	double		pdata,fx,fy;

	sdata = (unsigned short *)data;
	fdata = (float *)data;

/* scan the image (masked) */
	for(y=0;y<dy;y+=xystep) {
	j = y*dx;
	for(x=0;x<dx;x+=xystep) {
		if (mask[j] > 0) {
/* get the output data for a masked voxel */
			fx = ((double)x)*ipixel[0];
			fy = ((double)y)*ipixel[1];
			switch(dz) {
				case 1:
					pdata = data[j]+cnst;
					break;
				case 2:		
					pdata = sdata[j]+cnst;
					break;
				case 4:
					pdata = fdata[j]+cnst;
					break;
				default:
					pdata = 0.0;
					break;
			}
			if (radial) {
				rad_data_output(pdata,fx,fy,z,ofp);
			} else {
				data_output(pdata,nsets,pnts,fx,fy,z,ofp);
			}
		}
		j += xystep;
	}
	}
	return;
}

void data_output(double pdata,long int nsets,pointvec *pnts,double x,
	double y,double z,FILE *ofp)
{
	long int i;
	double	v[3],d;

	fprintf(ofp,"%.1f",pdata);
	for(i=0;i<nsets;i++) {
/* get the vector */
		v[0] = x - pnts[i].x1;
		v[1] = y - pnts[i].y1;
		v[2] = z - pnts[i].z1;
/* DOT product with the point vector */
		d = dot_prod(v,pnts[i].v);
/* divide by length of the vector (d is [0,1] over len of vector) */
		d = d / pnts[i].d;
/* over length of the vector, d is [-1,1] */
		d = (d * 2.0) - 1.0;
		fprintf(ofp,"\t%f",d);
	}
if (verb) {
	fprintf(ofp,"\t%.0f\t%.0f\t%.0f",x,y,z);
}
	fprintf(ofp,"\n");
}

void read_pointset(char *file,long int *npts,long int *v)
{
	FILE		*fp;
	char		tstr[256];
	long int	i,j,k;

	fp = fopen(file,"r");
	if (fp != 0L) {
		fgets(tstr,255,fp);
		sscanf(tstr,"%ld",&k);
		if (k > 4000) k = 4000;
		j = 0;
		for(i=0;i<k;i++) {
			fgets(tstr,255,fp);
			sscanf(tstr,"%ld %ld %ld %ld",&(v[j]),&(v[j+1]),
				&(v[j+2]),&(v[j+3]));
			j += 4;
		}
		*npts = k;
		fclose(fp);
	} else {
		*npts = 0;
	}
	return;
}


void	calc_angle_tables(pointvec P,long int nangles,
	float *v[MAX_ANG],long int n[MAX_ANG],long int avg_cent)
{
	double		t[3],d,tv[3],td,p[3];
	double		sum[3],sumn;
	long int	i,j,k;

	gp.num_traces = nangles;
	gp.num_disks = P.d;
	gp.cent_axis = P;
	if (gp.num_traces > MAX_ANG)ex_err("Maximum number of traces exceeded");
	if (gp.num_disks > MAX_DISKS)ex_err("Maximum axis length exceeded");

/* find the center of the disk (from p1 to p2) */
	for(i=0;i<gp.num_disks;i++) {
if (verb) {
	fprintf(stderr,"%ld\t",i);
}
		t[0] = P.x1 + (i*P.v[0]);
		t[1] = P.y1 + (i*P.v[1]);
		t[2] = P.z1 + (i*P.v[2]);
/* record for later */
		gp.disk_cent[i][0] = t[0];
		gp.disk_cent[i][1] = t[1];
		gp.disk_cent[i][2] = t[2];
/* prepare to calc average centroid */
		sum[0] = 0; sum[1] = 0; sum[2] = 0; sumn = 0;
/* consider all traces */
		for(j=0;j<gp.num_traces;j++) {
/* find the closest point to the plane in each of the trace lists */
			d = 1e+10;
			for(k=0;k<n[j];k++) {
/* vector from center of disk to point */
				p[0] = v[j][(k*4)+0];
				p[1] = v[j][(k*4)+1];
				p[2] = v[j][(k*4)+2];
				tv[0] = p[0] - t[0];
				tv[1] = p[1] - t[1];
				tv[2] = p[2] - t[2];
/* project onto central axis normal (dot product) */
				td = dot_prod(tv,gp.cent_axis.v);
/* save the shortest point (on the disk!) */
				if (fabs(td) < fabs(d)) {
					d = td;
					gp.disk_pnt[i][j][0] = p[0]-td*P.v[0];
					gp.disk_pnt[i][j][1] = p[1]-td*P.v[1];
					gp.disk_pnt[i][j][2] = p[2]-td*P.v[2];
				}
			}
/* average centroid */
			if ((avg_cent == 1) || ((avg_cent == 2) && 
				((j == 0) || (j == gp.num_traces-1)))) {
/* add if avg_cent is 1 or if avg_cent is 2 an you are an edge point */
				sum[0] += gp.disk_pnt[i][j][0];
				sum[1] += gp.disk_pnt[i][j][1];
				sum[2] += gp.disk_pnt[i][j][2];
				sumn += 1.0;
			}
		}
/* store off average centroid */
		if (avg_cent != 0) {
			gp.disk_cent[i][0] = sum[0]/sumn;
			gp.disk_cent[i][1] = sum[1]/sumn;
			gp.disk_cent[i][2] = sum[2]/sumn;
		}
/* for each trace, compute the vector (centroid to point) normalized and ang */
		for(j=0;j<gp.num_traces;j++) {
			gp.disk_pnt[i][j][0] = gp.disk_pnt[i][j][0] - 
				gp.disk_cent[i][0];
			gp.disk_pnt[i][j][1] = gp.disk_pnt[i][j][1] - 
				gp.disk_cent[i][1];
			gp.disk_pnt[i][j][2] = gp.disk_pnt[i][j][2] - 
				gp.disk_cent[i][2];
/* normalize the vector */
			normalize(gp.disk_pnt[i][j]);
/* compute the angle */
			if (j == 0) {
				gp.disk_pnt_ang[i][j]=0.0; /* by definition */
			} else {
				gp.disk_pnt_ang[i][j] = vec_angle(
					gp.disk_pnt[i][0],gp.disk_pnt[i][j],
					gp.cent_axis.v);
				if (lefthand) {
					gp.disk_pnt_ang[i][j] = 
						360.0 - gp.disk_pnt_ang[i][j];
				}
			}
if (verb) {
	fprintf(stderr,"\t%f",gp.disk_pnt_ang[i][j]);
}
		}
if (verb) {
	fprintf(stderr,"\n");
}
	}
/* ready to rock-n-roll! */
	return;
}

void rad_data_output(double pdata,double x,double y,double z,FILE *ofp)
{
	long int i,j;
	double	p[3],v[3],d,out[3];  /* t,theta,rad */


/* get the vector (axis end point to point) */
	v[0] = x - gp.cent_axis.x1;
	v[1] = y - gp.cent_axis.y1;
	v[2] = z - gp.cent_axis.z1;
/* DOT product with the axis vector */
	d = dot_prod(v,gp.cent_axis.v);
/* check to see if disk i is legal */
	i = d;
	if ((i < 0) || (i >= gp.num_disks)) return;

/* get the t axis info */
/* divide by length of the vector (t is [0,1] over len of vector) */
	out[0] = d / gp.cent_axis.d;
/* over length of the vector, t is [-1,1] */
	out[0] = (out[0] * 2.0) - 1.0;

/* get point on axis */
	p[0] = gp.disk_cent[i][0];
	p[1] = gp.disk_cent[i][1];
	p[2] = gp.disk_cent[i][2];
/* get vector (nearest point on the axis to point) */
	v[0] = x - p[0];
	v[1] = y - p[1];
	v[2] = z - p[2];
/* get the radius axis info */
	out[1] = dot_prod(v,v);
	if (out[1] > 0) out[1] = sqrt(out[1]);
	if (out[1] < 0.01) return;  /* too close to center */
/* normalize the vector from p to the point */
	normalize(v);

/* find the angle the point falls into */
	d = vec_angle(gp.disk_pnt[i][0],v,gp.cent_axis.v);
	if (lefthand) d = 360.0-d;
	j = 0;
	while ((d > gp.disk_pnt_ang[i][j]) && (j < gp.num_traces)) j++;

/* outside the arc */
	if (j == gp.num_traces) {
/* interpolate between ends... (j=>360.0) */
		out[2] = (double)(j) - ((360.0 - d)/
			(360.0 - gp.disk_pnt_ang[i][j-1]));
/* and correct (if > j+0.5 then it is negative) */
		if (out[2] > (double)(j)-0.5) {
			out[2] = out[2] - (double)(j);
		}

/* somewhere within the arc */
	} else {
/* interpolate between (j) and (j-1) */
/* (j-out/(j-(j-1)) = (h-d)/(h-l) */
/*  out = j - ((h-d)/(h-l)) */
		out[2] = (double)(j) - ((gp.disk_pnt_ang[i][j] - d)/
			(gp.disk_pnt_ang[i][j] - gp.disk_pnt_ang[i][j-1]));
	}

if (verb) {
	fprintf(ofp,"\t%.0f\t%.0f\t%.0f",x,y,z);
}
	fprintf(ofp,"%.1f",pdata);
	fprintf(ofp,"\t%f",out[0]);
	fprintf(ofp,"\t%f",out[1]);
	fprintf(ofp,"\t%f",out[2]);
	fprintf(ofp,"\n");

	return;
}

double dot_prod(double *v1,double *v2)
{
	double	d;

/* dot product */
	d = (v1[0]*v2[0]);
	d += (v1[1]*v2[1]);
	d += (v1[2]*v2[2]);
	return(d);
}

void normalize(double *v)
{
	double d;

	d = dot_prod(v,v);

	if (d > 0) {
		d = sqrt(d);
		v[0] /= d;
		v[1] /= d;
		v[2] /= d;
	}

	return;
}

void cross_prod(double *v1,double *v2,double *x)
{
/* cross product */
	x[0] = (v1[1]*v2[2]) - (v1[2]*v2[1]);
	x[1] = (v1[2]*v2[0]) - (v1[0]*v2[2]);
	x[2] = (v1[0]*v2[1]) - (v1[1]*v2[0]);

	return;
}

double	vec_angle(double *v1,double *v2,double *z)
{
	double	*x,y[3],d;
	double	xl,yl;

/* find x,y,z */
	x = v1;
	cross_prod(z,x,y);

/* get xl,yl */
	xl = dot_prod(v2,x);
	yl = dot_prod(v2,y);

	d = atan2(yl,xl);

	d = d*(180/M_PI);

	while(d < 0) d += 360.0;

	return(d);
}

void smooth_pointset(long int num,long int *v,long int n)
{
	long int 	i,j,k;
	double		s[3],cnt;
	long int	list[M_PTS];

	for(i=0;i<num;i++) {
		cnt = 0;
		for(j=0;j<3;j++) s[j] = 0;
		for(k=-n;k<=n;k++) {
			if (((k+i) >= 0) && ((k+i) < num)) {
				for(j=0;j<3;j++) s[j] += v[(i*4)+j+(k*4)];
				cnt += 1.0;
			}
		}
		for(j=0;j<3;j++) list[(i*4)+j] = s[j]/cnt;
	}
	for(i=0;i<num;i++) {
		v[i] = list[i];
	}
	return;
}

