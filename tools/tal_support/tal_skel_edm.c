/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski
 *
 * $Id: tal_skel_edm.c 1213 2005-08-27 20:51:21Z rjfrank $
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
#include "label_comp.h"
#include "skeleton.h"

#define	M_SKEL3D	1
#define	M_SKEL2D	2
#define	M_EDM3D		3
#define	M_EDM2D		4
#define M_LBL3D		5
#define M_LBL2D		6

long int	mode;

void ex_err(char *s);
void cmd_err(char *s);

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
	fprintf(stderr,"         -z(dz) image depth in bytes default:1\n");
	fprintf(stderr,"         -o(offset) data offset (p+offset) default:0\n");
	fprintf(stderr,"         -f(start) first image number default:1\n");
	fprintf(stderr,"         -l(end) last image number default:119\n");
	fprintf(stderr,"         -i(step) image number step default:1\n");
	fprintf(stderr,"         -t(thres) threshold value default:100\n");
	fprintf(stderr,"         -b swap bytes with 16bit images default:no\n");
	fprintf(stderr,"         -2d perform plane by plane 2d operation default:3d operation\n");
	fprintf(stderr,"         -h(histofile) file to save histogram in default:not saved\n");
	if ((mode == M_LBL3D) || (mode == M_LBL2D)) {
	fprintf(stderr,"         -c(conn) specify connectivity default:4 or 6\n");
	fprintf(stderr,"	 Valid connectivity values: 2D:4,8  3D:6,18,26\n");
	}
	if ((mode == M_EDM3D) || (mode == M_EDM2D)) {
	fprintf(stderr,"         -s(scale) scale EDM output volumes by integer scale default:no\n");
	fprintf(stderr,"         -sip(ipx[:ipy]) specify interpixel spacing default:1.0[:1.0]\n");
	fprintf(stderr,"         -sis(islice) specify interslice spacing default:1.0\n");
	fprintf(stderr,"Note: Scaled EDM output volumes are 16bit instead of 8bit.\n");
	fprintf(stderr,"      -sip and -sis are only supported when -s is used\n");
	fprintf(stderr,"      and output volumes are always reported in pixels.\n");
	}
	fprintf(stderr,"Output volumes are 8bit\n");
	tal_exit(1);
}

/* connectivity tables */

long int	conn_tab[][24] = { 
    {4,M_LBL2D,2, -1,0, 0,-1}, 
    {8,M_LBL2D,4, -1,0, -1,-1, 0,-1, 1,-1},
    {6,M_LBL3D,3, -1,0,0, 0,-1,0, 0,0,-1},
    {18,M_LBL3D,6, -1,0,0, 0,-1,0, 0,0,-1, -1,-1,0, -1,0,-1, 0,-1,-1},
    {26,M_LBL3D,7, -1,0,0, 0,-1,0, 0,0,-1, -1,-1,0, -1,0,-1, 0,-1,-1, -1,-1,-1},
    {-1}
	};

int main(int argc,char **argv)
{
	long int	dx = 256;
	long int	dy = 256;
	long int	dz = 1;
	long int	istart = 1;
	long int	iend = 119;
	long int	istep = 1;
	long int	swab = 0;
	long int	scale = 0;
	long int	conn = 4;
	double		offset = 0.0;
	double		thres = 100;
	double		is = 1.0;
	double		ip[2] = {1.0,1.0};

	double		val;
	long int	hist[65536];
	char		intemp[256],*histfile,outtemp[256];
	char		tstr[256];
	long int	i,err,j,k;
	long int	icount;
	unsigned char	*data,*tp;
	unsigned short	*sdata;
	char		*tp2;
	long int	conn_idx,d[3];

	FILE		*ofp;

	histfile = 0L;
	mode = M_SKEL3D;

/* which program... */
        tp2 = argv[0];
        if (strrchr(tp2,'/') != 0L) tp2 = strrchr(tp2,'/') + 1;
        if (strcmp(tp2,"tal_edm") == 0L) mode = M_EDM3D;
        if (strcmp(tp2,"tal_label") == 0L) mode = M_LBL3D;

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
			case 'b':
				swab = 1;
				break;
			case '2':
				if (mode == M_SKEL3D) mode = M_SKEL2D;
				if (mode == M_EDM3D) mode = M_EDM2D;
				if (mode == M_LBL3D) mode = M_LBL2D;
				break;
			case 'c':
				if ((mode != M_LBL2D) && (mode != M_LBL3D)) {
					 cmd_err(argv[0]);
				}
				conn = atoi(&(argv[i][2]));
				break;
			case 'h':
				histfile = &(argv[i][2]);
				break;
			case 's':
				if ((mode != M_EDM2D) && (mode != M_EDM3D)) {
					 cmd_err(argv[0]);
				}
				if (strncmp(argv[i],"-sip",4) == 0) {
					get_d_xy(ip,&(argv[i][4]));
				} else if (strncmp(argv[i],"-sis",4) == 0) {
					is = atof(&(argv[i][4]));
				} else {
					scale = atoi(&(argv[i][2]));
				}
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

/* special case for defaults */ 
	if ((mode == M_LBL3D) && (conn == 4)) conn = 6;
/* find the right connectivity table */
	if ((mode == M_LBL3D) || (mode == M_LBL2D)) {
		conn_idx = 0;
		while(conn_tab[conn_idx][0] != -1) {
			if ((conn_tab[conn_idx][1] == mode) && 
				(conn_tab[conn_idx][0] == conn)) break;
		
			conn_idx++;
		}
		if (conn_tab[conn_idx][0] == -1) {
			fprintf(stderr,
			   "Valid connectivity values: 2D:4,8  3D:6,18,26\n");
			ex_err("Unknown connectivity value");
		}
	}


/* clear histogram */
	for(i=0;i<65536;i++) hist[i] = 0;

/* count the number of images */
	icount = 0;
	for(i=istart;i<=iend;i=i+istep) icount++;
/* get the image memory (bytes) */
	if (scale == 0) {
		data = malloc(dx*dy*icount);
	} else {
		data = malloc(dx*dy*icount*sizeof(short));
		sdata = (unsigned short *)data;
	}
	if (data == 0L) ex_err("Unable to allocate image memory.");
/* temp I/O image */
	tp = malloc(dx*dy*dz);
	if (tp == 0L) ex_err("Unable to allocate image memory."); 

/* read the images */
	fprintf(stderr,"Reading %ld byte dataset. Threshold=%f\n",
		(dx*dy*icount),thres);
	k = 0;
	for(i=istart;i<=iend;i=i+istep) {
/* read image */
		name_changer(intemp,i,&err,tstr);
		fprintf(stderr,"Reading the file:%s\n",tstr);
		bin_io(tstr,'r',tp,dx,dy,dz,0,swab,0L);
		for(err=0;err<dx*dy;err++) {
			switch(dz) {
				case 1:
			 		val=((unsigned char *)tp)[err];
					break;
				case 2:
			 		val=((unsigned short *)tp)[err];
					val += offset;
					break;
				case 4:
			 		val=((float *)tp)[err];
					break;
			}
/* internal 8 or 16 bit representation */
			if (scale == 0) {
				data[k+err] = 0;
				if (thres > 0) {
					if (val > thres) data[k+err] = 1;
				} else {
/*
					if (val < fabs(thres)) data[k+err] = 1;
*/
					if (val < (-thres)) data[k+err] = 1;
				}
			} else {
				sdata[k+err] = 0;
				if (thres > 0) {
					if (val > thres) sdata[k+err] = 1;
				} else {
/*
					if (val < fabs(thres)) sdata[k+err] = 1;
*/
					if (val < (-thres)) sdata[k+err] = 1;
				}
			}
		}
		k = k + (dx*dy);
	}
/* no longer need temp mask holder */
	free(tp);

/* do the work */
	for(i=0;i<65536;i++) hist[i] = 0;
	if (mode == M_EDM3D) {
		if (scale == 0) {
			calc_edm_3d_(data,dx,dy,icount,1,hist);
		} else {
			calc_edm_3d_s_(sdata,dx,dy,icount,1,scale,hist,is,ip);
		}
	} else if (mode == M_EDM2D) {
		j = 0;
		for(i=0;i<icount;i++) {
			if (scale == 0) {
				calc_edm_(&(data[j]),dx,dy,1,hist);
			} else {
				calc_edm_s_(&(sdata[j]),dx,dy,1,scale,hist);
			}
			j = j + (dx*dy);
		}
	} else if (mode == M_SKEL2D) {
		j = 0;
		for(i=0;i<icount;i++) {
			calc_skel_(&(data[j]),dx,dy,hist);
			j = j + (dx*dy);
		}
	} else if (mode == M_SKEL3D) {
		skeleton_3d_(data,0L,dx,dy,icount,hist);
	} else if (mode == M_LBL2D) {
		d[0] = dx; d[1] = dy;
		j = 0;
		for(i=0;i<icount;i++) {
			label_components_(&(data[j]),2,d,conn_tab[conn_idx][2],
				&(conn_tab[conn_idx][3]),&k);
			for(k=0;k<dx*dy;k++) hist[data[j+k]] += 1;
			j = j + (dx*dy);
		}
	} else if (mode == M_LBL3D) {
		d[0] = dx; d[1] = dy; d[2] = icount;
		label_components_(data,3,d,conn_tab[conn_idx][2],
				&(conn_tab[conn_idx][3]),&k);
		for(k=0;k<dx*dy*icount;k++) hist[data[k]] += 1;
	}

/* output histogram file */
if (histfile != 0) {
	k = 1;
	for(i=0;i<65535;i++) if (hist[i] > 0) k = i;
	if (strcmp(histfile,"-") == 0) {
		ofp = stdout;
	} else {
		ofp = fopen(histfile,"w");
		if (ofp == 0) fprintf(stderr,"Unable to open the file:%s\n",
			histfile);
	}
	if (ofp != 0L) {
		fprintf(ofp,"%ld\n",k);
		for(i=0;i<=k;i++) fprintf(ofp,"%ld\t%ld\n",i,hist[i]);
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
			if (scale == 0) {
				bin_io(tstr,'w',&(data[j]),dx,dy,1,0,0,0L);
			} else {
				bin_io(tstr,'w',&(sdata[j]),dx,dy,2,0,0,0L);
			}
			j = j + (dx*dy);
		}
	}
/* done */	
	free(data);
	tal_exit(0);

	exit(0);
}
