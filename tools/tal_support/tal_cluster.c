/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski
 *
 * $Id: tal_cluster.c 1213 2005-08-27 20:51:21Z rjfrank $
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

#define MAX_C_SIZE 65535
#define POT_VOXEL 65535

typedef struct {
	double		thres;
	long int	dx;
	long int	dy;
	long int	dz;
	long int	icount;
	unsigned short	*mask;
	unsigned char	*data;
	unsigned short	*sdata;
	float 		*fdata;
	long int	neighbors;
	long int	csize;
	long int	lessthan;
} c_info;

typedef struct {
	double		loc[3];   	/* centroid */
	double		avg; 		/* average voxel value */
	long int	size;		/* # of voxels */
	long int	index;		/* serial # */
} cluster;

/* neighbor offsets */
/* 0 = the voxel  1-6 = 6neighbors 1-26 = 26neighbors */
long int	dxlist[27] = { 0, 1,-1, 0, 0, 0, 0,
			       0, 0, 0, 0,
			      -1, 1,-1, 1,
			       1, 1,-1,-1,
			      -1,-1,-1,-1, 1, 1, 1, 1
			     };
long int	dylist[27] = { 0, 0, 0, 1,-1, 0, 0,
			      -1, 1,-1, 1,
			       0, 0, 0, 0,
			      -1, 1,-1, 1,
			      -1,-1, 1, 1,-1,-1, 1, 1
			     };
long int	dzlist[27] = { 0, 0, 0, 0, 0, 1,-1,
			       1, 1,-1,-1,
			       1, 1,-1,-1,
			       0, 0, 0, 0,
			      -1, 1,-1, 1,-1, 1,-1, 1
			     };

static int cluster_check(const void *d1,const void *d2);
void indx2tal(double *indx,double *tal);
void tal2indx(double *tal,double *indx);
void ex_err(char *s);
void cmd_err(char *s);
long int track_cluster(c_info *info,long int x,long int y,long int z,
	double *cent,long int cnum);
long int check_location(c_info *info,long int x,long int y, long int z,
	long int d,float *val);
void plot_location(c_info *info,long int i,long int j, long int k,
	long int dat);

/* routine to set a voxel in the mask volume to a particular value */
void plot_location(c_info *info,long int i,long int j, long int k,
	long int dat)
{
	long int 	l;
	unsigned short	m;

	if ((i < 0) || (i >= info->dx)) return;
	if ((j < 0) || (j >= info->dy)) return;
	if ((k < 0) || (k >= info->icount)) return;
	
	l = i + (j*(info->dx)) + (k*(info->dx)*(info->dy));

	if (dat > 65534L) {
		m = 65534L;
	} else {
		m = dat;
	}
	info->mask[l] = m;

	return;
}

/* routine to see if a given neighbor is legal
(if so, tag the voxel in the mask volume and return its value) */
long int check_location(c_info *info,long int x,long int y, long int z,
	long int d,float *val)
{
	long int	i,j,k,l;
	double		v;

	i = x + dxlist[d];
	j = y + dylist[d];
	k = z + dzlist[d];

	if ((i < 0) || (i >= info->dx)) return(0);
	if ((j < 0) || (j >= info->dy)) return(0);
	if ((k < 0) || (k >= info->icount)) return(0);
	
	l = i + (j*(info->dx)) + (k*(info->dx)*(info->dy));
	if (info->mask[l] != POT_VOXEL) return(0);

	switch(info->dz) {
		case 1:
			v = info->data[l];
			break;
		case 2:
			v = info->sdata[l];
			break;
		case 4:
			v = info->fdata[l];
			break;
		default:
			v = 0;
			break;
	}

/* if lessthan is TRUE, reject v > thres as we are looking for low spots */
/* if lessthan is FALSE, reject v < thres as we are looking for high spots */
	if (info->lessthan) {
		if (v > info->thres) return(0);
	} else {
		if (v < info->thres) return(0);
	}

	*val = v;

	info->mask[l] = 0; /* tag the voxel so it cannot be found again */

	return(1);
}


/* this routine tracks a cluster starting at the given location and
	returns the size, centroid and average value of any detected
	cluster.  The cluster is also tagged in the mask volume */
long int track_cluster(c_info *info,long int x,long int y,long int z,
	double *cent,long int cnum)
{
/* stack based flood filling */
	long int	*xlist = 0;
	long int	*ylist = 0;
	long int	*zlist = 0;
	float		*vlist = 0;
	long int	onlist,done;
	long int	i,j;
	double		s;

	long int	lsize;

/* list starts */
	onlist = 0;
	done = 0;
	lsize = MAX_C_SIZE;

/* allocate memory for the cluster list */
	xlist = (long int *)malloc(lsize*sizeof(long int));
	ylist = (long int *)malloc(lsize*sizeof(long int));
	zlist = (long int *)malloc(lsize*sizeof(long int));
	vlist = (float *)malloc(lsize*sizeof(float));
	if ((xlist == 0) || (ylist == 0) || (zlist == 0) || (vlist == 0)) {
/* retval is 0 */
		goto errout;
	}


/* starting pointi(d=0) check and add to list */
	if (check_location(info,x,y,z,0,&(vlist[onlist])) == 0) goto errout;
	xlist[onlist] = x;
	ylist[onlist] = y;
	zlist[onlist] = z;
	onlist++;

/* look at neighbors of voxels on the list until they have all been checked */
	while (done < onlist) {
/* if neighbors are valid, add them */
		for(i=1;i<=(info->neighbors);i++) {
			if (check_location(info,xlist[done],ylist[done],
					zlist[done],i,&(vlist[onlist]))) {
				xlist[onlist] = xlist[done] + dxlist[i];
				ylist[onlist] = ylist[done] + dylist[i];
				zlist[onlist] = zlist[done] + dzlist[i];
/* if out of list space, add some more */
				if (onlist >= (lsize-3)) {

				lsize += 500;
				xlist = realloc(xlist,lsize*sizeof(long int));
				ylist = realloc(ylist,lsize*sizeof(long int));
				zlist = realloc(zlist,lsize*sizeof(long int));
				vlist = realloc(vlist,lsize*sizeof(float));

if ((xlist == 0) || (ylist == 0) || (zlist == 0) || (vlist == 0)) {
	ex_err("Unable to allocate list memory");
}
				}
				onlist++;
			}
		}
/* one more checked voxel */
		done++;
	}
/* tag the voxels on the list appropriately (by size or index) */
	if (info->csize) {
		j = onlist;
	} else {
		j = cnum;
	}

/* draw the cluster into the mask volume */
	for(i=0;i<onlist;i++) {
		plot_location(info,xlist[i],ylist[i],zlist[i],j);
	}

/* compute centroid and average values */
	s = 0;
	for(i=0;i<onlist;i++) s += xlist[i];
	cent[0] = s/((double)(onlist));
	s = 0;
	for(i=0;i<onlist;i++) s += ylist[i];
	cent[1] = s/((double)(onlist));
	s = 0;
	for(i=0;i<onlist;i++) s += zlist[i];
	cent[2] = s/((double)(onlist));
	s = 0;
	for(i=0;i<onlist;i++) s += vlist[i];
	cent[3] = s/((double)(onlist));

/* fre up any RAM */
errout:
	if (xlist != 0) free(xlist);
	if (ylist != 0) free(ylist);
	if (zlist != 0) free(zlist);
	if (vlist != 0) free(vlist);

/* return the size of the cluster */
	return(onlist);
}

void ex_err(char *s)
{
	fprintf(stderr,"Fatal error:%s\n",s);
	tal_exit(1);
}

void cmd_err(char *s)
{
	fprintf(stderr,"(%s:%s) Usage: %s [options] inputtemp\n",__DATE__,TAL_VERSION,s);
	fprintf(stderr,"Options: -x(dx) image dx default:256\n");
	fprintf(stderr,"         -y(dy) image dy default:256\n");
	fprintf(stderr,"         -z(dz) image depth in bytes default:2\n");
	fprintf(stderr,"         -o(offset) data offset (p+offset) default:0\n");
	fprintf(stderr,"         -f(start) first image number default:1\n");
	fprintf(stderr,"         -l(end) last image number default:119\n");
	fprintf(stderr,"         -i(step) image number step default:1\n");
	fprintf(stderr,"         -m(masktemp) template for a mask volume\n");
	fprintf(stderr,"         -t(thres) cluster threshold value default:100\n");
	fprintf(stderr,"         -csize output volume value is cluster size default:index number\n");
	fprintf(stderr,"         -clessthan search clusters less than thres default:greater than thres\n");
	fprintf(stderr,"         -6 use six neighbor rules for clustering default:26 neighbor rule\n");
	fprintf(stderr,"         -b swap bytes with 16bit images default:no\n");
	fprintf(stderr,"         -h(histofile) file to save cluster histogram in default:not saved\n");
	fprintf(stderr,"         -cvol(outtemp) template to save 16Bit cluster volume to disk\n");
	fprintf(stderr,"         -cmin(min) report only cluster sizes above min default:0\n");
	fprintf(stderr,"         -seeded use list of input seeds as cluster starting points\n");
	fprintf(stderr,"         -seed+thres like -seeded but a 4th input value is used as seed thres\n");
	fprintf(stderr,"         -notal seed points are in pixel coords default:Talairach coords\n");
	fprintf(stderr,"         -r(radius) radius to search for clusters around seeds default:3.0\n");
	fprintf(stderr,"The -r option is only used with the -seeded option.\n");
	fprintf(stderr,"When -seeded is specified, points are read from standard input in the form:\n");
	fprintf(stderr,"(XXX YYY ZZZ stat)\n");
	fprintf(stderr,"stat is used as the threshold value for the seed when -seed+thres is used.\nOtherwise, stat is ignored.\n");
	fprintf(stderr,"Note: Mask volumes must be 8Bit volumes.\n");
	tal_exit(1);
}

int main(int argc,char **argv)
{
	long int	dx = 256;
	long int	dy = 256;
	long int	dz = 2;
	long int	istart = 1;
	long int	iend = 119;
	long int	istep = 1;
	long int	swab = 0;
	double		offset = 0.0;
	double		thres = 100;
	double		cmin = 0;
	long int	csize = 0;
	long int	lessthan = 0;
	long int	neighbors = 26;
	double		radius = 3.0;
	long int	use_tal = 1;
	long int	seeded = 0;
	long int	indvthres = 0;

	double		nthres;
	long int	hist[65536];
	char		intemp[256],*mtemp,*histfile,*outtemp;
	char		tstr[256];
	long int	i,err,j,k;
	long int	icount;
	unsigned char	*data,*tp;
	unsigned short	*mask;

	double		tal[3],pnt[3],d;
	FILE		*ofp;
	long int 	x,y,z,r,cnum;
	c_info  	c_data;
	double		cent[4];  /* x,y,z,v */

	cluster		*found;
	long int	nfound,nalloc;

	mtemp = 0L;
	histfile = 0L;
	outtemp = 0L;

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
			case 't':
				thres = atof(&(argv[i][2]));
				break;
			case 'r':
				radius = atof(&(argv[i][2]));
				break;
			case 'b':
				swab = 1;
				break;
			case '6':
				neighbors = 6;
				break;
			case 'm':
				mtemp = &(argv[i][2]);
				break;
			case 'h':
				histfile = &(argv[i][2]);
				break;
			case 'n':
				if (strncmp("-notal",argv[i],6) == 0) {
					use_tal = 0;
				}
				break;
			case 's':
				if (strncmp("-seeded",argv[i],7) == 0) {
					seeded = 1;
				}
				if (strncmp("-seed+thres",argv[i],11) == 0) {
					seeded = 1;
					indvthres = 1;
				}
				break;
			case 'c':
				if (strncmp("-csize",argv[i],6) == 0) {
					csize = 1;
				} else if(strncmp("-clessthan",argv[i],10)==0) {
					lessthan = 1;
				} else if (strncmp("-cvol",argv[i],5) == 0) {
					outtemp = &(argv[i][5]);
				} else if (strncmp("-cmin",argv[i],5) == 0) {
					cmin = atof(&(argv[i][5]));
				}
				break;
			default:	
				cmd_err(argv[0]);
				break;
		}
		i++;
	}
	if ((argc-i) != 1) cmd_err(argv[0]);
	strcpy(intemp,argv[i]);

/* clear histogram */
	for(i=0;i<65536;i++) hist[i] = 0;

/* count the number of images */
	icount = 0;
	for(i=istart;i<=iend;i=i+istep) icount++;
/* get the image memory */
	data = malloc(dz*dx*dy*icount);
	if (data == 0L) ex_err("Unable to allocate image memory.");
/* mask volume */
	mask = (unsigned short *)malloc(dx*dy*icount*sizeof(unsigned short));
	if (mask == 0L) ex_err("Unable to allocate image memory.");
/* temp mask I/O image */
	tp = malloc(dx*dy);
	if (tp == 0L) ex_err("Unable to allocate image memory."); 

/* read the images */
	fprintf(stderr,"Reading %ld byte dataset.\n",
		(dz*dx*dy*icount));
	j = 0;
	k = 0;
	for(i=istart;i<=iend;i=i+istep) {
/* read image */
		name_changer(intemp,i,&err,tstr);
		fprintf(stderr,"Reading the file:%s\n",tstr);
		bin_io(tstr,'r',&(data[j]),dx,dy,dz,use_tal,swab,0L);
/* read mask (if any) */
		if (mtemp == 0) {
			for(err=0;err<dx*dy;err++) mask[k+err] = POT_VOXEL;
		} else {
/* read the slice and transform it into a short boolean */
			name_changer(mtemp,i,&err,tstr);
			fprintf(stderr,"Reading the mask file:%s\n",tstr);
			bin_io(tstr,'r',tp,dx,dy,1,use_tal,swab,0L);
			for(err=0;err<dx*dy;err++) {
				if (tp[err] == 0) {
					mask[k+err] = 0;
				} else {
					mask[k+err] = POT_VOXEL;
				}
			}
		}
		j = j + (dx*dy*dz);
		k = k + (dx*dy);
	}
/* no longer need temp mask holder */
	free(tp);

/* Mask volume definition: 0-outside the mask POT_VOXEL-potential voxel 
	else-clustered (size of cluster or cluster index) */

	if (lessthan) {
		fprintf(stderr,"Using data offset of %.3f and threshold of %.3f ((v+%.3f) < %.3f)\n",offset,thres,offset,thres);
	} else {
		fprintf(stderr,"Using data offset of %.3f and threshold of %.3f ((v+%.3f) > %.3f)\n",offset,thres,offset,thres);
	}
	if (use_tal) {
		fprintf(stderr,"Cluster locations in Talairach space coordinates\n");
	} else {
		fprintf(stderr,"Cluster locations in volume space coordinates\n");
	}
/* begin the task at hand... */
	c_data.thres = thres - offset;
	c_data.data = data;
	c_data.sdata = (unsigned short *)data;
	c_data.fdata = (float *)data;
	c_data.mask = mask;
	c_data.dx = dx;
	c_data.dy = dy;
	c_data.dz = dz;
	c_data.icount = icount;
	c_data.neighbors = neighbors;
	c_data.csize = csize;
	c_data.lessthan = lessthan;

/* structure to hold the results */
	nalloc = 500;
	nfound = 0;
	found = (cluster *)malloc(nalloc*sizeof(cluster));
	if (found == 0L) ex_err("Unable to allocate list memory.");

	cnum = 1; /* first cluster is #1 (because of mask volume def) */

/* use seeded or non-seeded mode */
if (seeded) {
/* read lines of xxx yyy zzz score */
	fprintf(stderr,"Seeded cluster search at a radius of %.3f voxels\n",
		radius);
	if (use_tal) {
		fprintf(stderr,"Enter Talairach space seed locations:\n");
	} else {
		fprintf(stderr,"Enter volume space seed locations:\n");
	}
	while(fgets(tstr,255,stdin) != 0L) {
		j = sscanf(tstr,"%lf %lf %lf %lf",
			&(tal[0]),&(tal[1]),&(tal[2]),&nthres);
		if ((j == 4) && (indvthres)) {
			c_data.thres = nthres - offset;
		} else {
			c_data.thres = thres - offset;
		}
		if (j >= 3) {
/* probe radius around tal[0]-tal[3] */
/* convert to volume indicies */
			if (use_tal) {
				tal2indx(tal,pnt);
			} else {
				pnt[0]=tal[0]; pnt[1]=tal[1]; pnt[2]=tal[2];
			}
/* compute values in a sphere around the point */
			r = radius + 1;
			for(z=pnt[2]-r;z<=pnt[2]+r;z++) {
			for(y=pnt[1]-r;y<=pnt[1]+r;y++) {
			for(x=pnt[0]-r;x<=pnt[0]+r;x++) {
/* check radius */
				d = (pnt[0] - x)*(pnt[0] - x);
				d += (pnt[1] - y)*(pnt[1] - y);
				d += (pnt[2] - z)*(pnt[2] - z);
				d = sqrt(d);
				if (d <= radius) {
/* does a cluster exist here ? */
		k = track_cluster(&c_data,x,y,z,cent,cnum);
		if (k > 0) {
/* add to histogram */
			if (k < 65536) {
				hist[k] +=1 ;
			} else {
				hist[65535] += 1;
			}
/* add to cluster list */
			found[nfound].loc[0] = cent[0];
			found[nfound].loc[1] = cent[1];
			found[nfound].loc[2] = cent[2];
			found[nfound].avg = cent[3] + offset;
			found[nfound].size = k;
			found[nfound].index = cnum;
			nfound += 1;

/* allocate further list memory if needed */
			if ((nalloc - nfound) < 10) {
				nalloc += 100;
				found = (cluster *)realloc(found,
					nalloc*sizeof(cluster));
				if (found == 0L) 
				   ex_err("Unable to realloc list memory.");
			}
/* next object index */
			cnum++;
		}
/* cluster handled and recorded */
				}

			} /* x */
			} /* y */
			} /* z */

		} /* legal point */
	} /* more points */
} else {
/* do the work */
	for(z=0;z<icount;z++) {
	for(y=0;y<dy;y++) {
	for(x=0;x<dx;x++) {
		k = track_cluster(&c_data,x,y,z,cent,cnum);
		if (k > 0) {
/* add to histogram */
			if (k < 65536) {
				hist[k] +=1 ;
			} else {
				hist[65535] += 1;
			}
/* add to cluster list */
			found[nfound].loc[0] = cent[0];
			found[nfound].loc[1] = cent[1];
			found[nfound].loc[2] = cent[2];
			found[nfound].avg = cent[3] + offset;
			found[nfound].size = k;
			found[nfound].index = cnum;
			nfound += 1;

/* allocate further list memory if needed */
			if ((nalloc - nfound) < 10) {
				nalloc += 100;
				found = (cluster *)realloc(found,
					nalloc*sizeof(cluster));
				if (found == 0L) 
				   ex_err("Unable to realloc list memory.");
			}
/* next object index */
			cnum++;
		}
	}
	}
	}
}

/* output histogram file */
if (histfile != 0) {
	k = 1;
	for(i=1;i<65535;i++) if (hist[i] > 0) k = i;
	if (strcmp(histfile,"-") == 0) {
		ofp = stdout;
	} else {
		ofp = fopen(histfile,"w");
		if (ofp == 0) fprintf(stderr,"Unable to open the file:%s\n",
			histfile);
	}
	if (ofp != 0L) {
		fprintf(ofp,"%ld\n",k);
		for(i=1;i<=k;i++) fprintf(ofp,"%ld\t%ld\n",i,hist[i]);
	}
	if ((ofp != 0L) && (ofp != stdout)) fclose(ofp);
}

/* write out the resultant mask volume if needed */
	if (outtemp != 0L) {
		j = 0;
		for(i=istart;i<=iend;i=i+istep) {
/* write image */
			name_changer(outtemp,i,&err,tstr);
			fprintf(stderr,"Writing the file:%s\n",tstr);
			for(k=0;k<(dx*dy);k++) {
				if (mask[j+k] == POT_VOXEL) mask[j+k] = 0;
			}
			bin_io(tstr,'w',&(mask[j]),dx,dy,
				sizeof(unsigned short),use_tal,swab,0L);
			j = j + (dx*dy);
		}
	}

/* output locations, sizes and values of the clusters */
/* sort clusters by size */	
	qsort(found,nfound,sizeof(cluster),cluster_check);
/* print sorted cluster list */
	for(i=(nfound-1);i>=0;i--) {
		if (found[i].size < cmin) continue;
		if (use_tal) indx2tal(found[i].loc,found[i].loc);
		fprintf(stdout,"%.3f\t%.3f\t%.3f\t%.4f\t%ld\t%ld\t%f\n",
			found[i].loc[0],found[i].loc[1],found[i].loc[2],
			found[i].avg,found[i].size,found[i].index,thres);
	}
	
/* done */	
	free(mask);
	free(data);
	free(found);
	tal_exit(0);
	exit(0);
}

static int cluster_check(const void *d1,const void *d2)
{
        cluster  *v1,*v2;
        v1 = (cluster *)d1;
        v2 = (cluster *)d2;

        if (v1->size > v2->size) return(1);
        if (v2->size > v1->size) return(-1);

        return(0);
}

void	tal2indx(double *tal,double *indx)
{
	indx[0] = 128.0 - tal[0];
	indx[1] = tal[1] + 128.0;
	indx[2] = tal[2] +  44.0;

	return;
}

void	indx2tal(double *indx,double *tal)
{
	tal[0] = 128.0 - indx[0];
	tal[1] = indx[1] - 128.0;
	tal[2] = indx[2] -  44.0;

	return;
}
