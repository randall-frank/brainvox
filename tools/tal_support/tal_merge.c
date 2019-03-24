/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski
 *
 * $Id: tal_merge.c 1213 2005-08-27 20:51:21Z rjfrank $
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

void ex_err(char *s)
{
	fprintf(stderr,"Fatal error:%s\n",s);
	tal_exit(1);
}

void cmd_err(char *s)
{
	fprintf(stderr,"(%s:%s) Usage: %s [options] inputtemp inputtemp2 outputtemp\n",__DATE__,TAL_VERSION,s);
	fprintf(stderr,"Options: -x(dx) image dx default:256\n");
	fprintf(stderr,"         -y(dy) image dy default:256\n");
	fprintf(stderr,"         -z(dz) image depth in bytes default:2\n");
	fprintf(stderr,"         -f(start) first image number default:1\n");
	fprintf(stderr,"         -l(end) last image number default:15\n");
	fprintf(stderr,"         -i(step) image number step default:1\n");
	fprintf(stderr,"         -b swap bytes with 16bit images default:no\n");
	fprintf(stderr,"         -m(mult1) default:0.5\n");
	fprintf(stderr,"         -n(mult2) default:0.5\n");
	fprintf(stderr,"         -o(offset1) default:-256.0\n");
	fprintf(stderr,"         -p(offset2) default:-256.0\n");
	fprintf(stderr,"         -c(const) default:256.0\n");
	fprintf(stderr," output = ((p1+offset1)*mult1) + ((p2+offset2)*mult2) + const \n");
	tal_exit(1);
}

int main(int argc,char **argv)
{
	long int	dx = 256;
	long int	dy = 256;
	long int	dz = 2;
	long int	istart = 1;
	long int	iend = 15;
	long int	istep = 1;
	long int	swab = 0;
	double		mult1 = 0.5;
	double		mult2 = 0.5;
	double		offset1 = -256.0;
	double		offset2 = -256.0;
	double		cnst = 256.0;
	char		intemp1[256],intemp2[256],outtemp[256];
	char		tstr[256];
	long int	i,err,j;
	unsigned char 	*data1,*data2,*oimage;
	unsigned char	*bd1,*bd2,*bd3;
	unsigned short	*sd1,*sd2,*sd3;
	float 		*fd1,*fd2,*fd3;
	double		ftmp;

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
				mult1 = atof(&(argv[i][2]));
				break;
			case 'n':
				mult2 = atof(&(argv[i][2]));
				break;
			case 'o':
				offset1 = atof(&(argv[i][2]));
				break;
			case 'p':
				offset2 = atof(&(argv[i][2]));
				break;
			case 'c':
				cnst = atof(&(argv[i][2]));
				break;
			case 'b':
				swab = 1;
				break;
			default:	
				cmd_err(argv[0]);
				break;
		}
		i++;
	}
	if ((argc-i) != 3) cmd_err(argv[0]);
	strcpy(intemp1,argv[i]);
	strcpy(intemp2,argv[i+1]);
	strcpy(outtemp,argv[i+2]);
	printf("Operation: %s = ((%s + %f) * %f) + ((%s + %f) * %f) + %f\n",
		outtemp,intemp1,offset1,mult1,intemp2,offset2,mult2,cnst);
/* get the image memory */
	data1 = malloc(dz*dx*dy);
	if (data1 == 0L) ex_err("Unable to allocate image memory.");
	data2 = malloc(dz*dx*dy);
	if (data2 == 0L) ex_err("Unable to allocate image memory.");
	oimage = malloc(dx*dy*dz);
	if (oimage == 0L) ex_err("Unable to allocate image memory.");
/* read the images */
	for(i=istart;i<=iend;i=i+istep) {
/* read one */
		name_changer(intemp1,i,&err,tstr);
		printf("Reading the file:%s\n",tstr);
		bin_io(tstr,'r',data1,dx,dy,dz,0,swab,0L);
/* read two */
		name_changer(intemp2,i,&err,tstr);
		printf("Reading the file:%s\n",tstr);
		bin_io(tstr,'r',data2,dx,dy,dz,0,swab,0L);
/* apply transformation */
		if (dz == 1) {
			bd1 = (unsigned char *)data1;
			bd2 = (unsigned char *)data2;
			bd3 = (unsigned char *)oimage;
			for(j=0;j<(dx*dy);j++) {
				ftmp = (((double)(*bd1)+offset1)*mult1) 
				    + (((double)(*bd2)+offset2)*mult2) + cnst;
				if (ftmp < 0.0) ftmp = 0.0;
				if (ftmp > 255.0) ftmp = 255.0;
				*bd3 = ftmp;
				bd3++;
				bd2++;
				bd1++;
			}
		} else if (dz == 2) {
			sd1 = (unsigned short *)data1;
			sd2 = (unsigned short *)data2;
			sd3 = (unsigned short *)oimage;
			for(j=0;j<(dx*dy);j++) {
				ftmp = (((double)(*sd1)+offset1)*mult1) 
				    + (((double)(*sd2)+offset2)*mult2) + cnst;
				if (ftmp < 0.0) ftmp = 0.0;
				if (ftmp > 65535.0) ftmp = 65535.0;
				*sd3 = ftmp;
				sd3++;
				sd2++;
				sd1++;
			}
		} else if (dz == 4) {
			fd1 = (float *)data1;
			fd2 = (float *)data2;
			fd3 = (float *)oimage;
			for(j=0;j<(dx*dy);j++) {
				ftmp = (((double)(*fd1)+offset1)*mult1) 
				    + (((double)(*fd2)+offset2)*mult2) + cnst;
				*fd3 = ftmp;
				fd3++;
				fd2++;
				fd1++;
			}
		}
/* write the output image */
		name_changer(outtemp,i,&err,tstr);
		printf("Writing sampled file:%s\n",tstr);
		bin_io(tstr,'w',oimage,dx,dy,dz,0,swab,0L);
	}
	free(oimage);
	free(data1);
	free(data2);
	tal_exit(0);
	exit(0);
}
