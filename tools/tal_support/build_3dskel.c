/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski
 *
 * $Id: build_3dskel.c 772 2005-04-17 17:18:04Z rjfrank $
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
#include <string.h>
#include <math.h>

#undef CONN6

void rot_x(short *in)
{
	short s;
	s = in[1-1];
	in[1-1] = in[5-1];
	in[5-1] = in[6-1];
	in[6-1] = in[2-1];
	in[2-1] = s;
	s = in[3-1];
	in[3-1] = in[7-1];
	in[7-1] = in[8-1];
	in[8-1] = in[4-1];
	in[4-1] = s;
	return;
}

void rot_y(short *in)
{
	short s;
	s = in[1-1];
	in[1-1] = in[3-1];
	in[3-1] = in[4-1];
	in[4-1] = in[2-1];
	in[2-1] = s;
	s = in[5-1];
	in[5-1] = in[7-1];
	in[7-1] = in[8-1];
	in[8-1] = in[6-1];
	in[6-1] = s;
	return;
}

void rot_z(short *in)
{
	short s;
	s = in[4-1];
	in[4-1] = in[8-1];
	in[8-1] = in[6-1];
	in[6-1] = in[2-1];
	in[2-1] = s;
	s = in[3-1];
	in[3-1] = in[7-1];
	in[7-1] = in[5-1];
	in[5-1] = in[1-1];
	in[1-1] = s;
	return;
}

void get_val(long in,short *out)
{
	long int	i,j;
	
	j = 1;
	for(i=0;i<8;i++) {
		if ((in & j) == j) {
			out[i] = 1;
		} else {
			out[i] = 0;
		}
		j = j * 2;
	}
	return;
}

void put_val(short *in,long *out)
{
	long int	i,j;
	
	j = 1;
	*out = 0;
	for(i=0;i<8;i++) {
		if (in[i] == 1) {
			*out |= j;
		}
		j = j * 2;
	}
	return;
}

int main(int argc,char **argv)
{
	long	known[23] = {0,1,3,9,129,7,67,22,15,39,23,135,195,
			105,233,188,248,126,246,252,254,255,27};
#ifdef CONN6
	short	kvals[23] = {0,1,0,2,2,-1,1,3,0,-2,-2,0,0,
				4,-1,-3,-1,-6,-2,0,1,0,-2};
#else
	short	kvals[23] = {0,1,0,-2,-6,-1,-3,-1,0,-2,-2,0,0,
				4,3,1,-1,2,2,0,1,0,-2};
#endif
	
	short	tbl[256],tblv[256]; 
	long	i;
	short	x,y,z,vec[8];
	long	j;
	FILE	*fp;
	
	for(i=0;i<256;i++) {
		tbl[i] = 0;
		tblv[i] = 0;
	}
	
	for(i=0;i<23;i++) {
		printf("Working on %ld of %ld...\n",i,23L);
		get_val(known[i],vec);
		for(x=0;x<4;x++) {
		for(y=0;y<4;y++) {
		for(z=0;z<4;z++) {
			put_val(vec,&j);
			if ((j < 0 ) || (j> 255)) {
				printf("Invalid index detected %ld\n",j);
				exit(0);
			}
			if (tbl[j] == 0) {
				tbl[j] = 1;
				tblv[j] = kvals[i];
			}
			rot_z(vec);
		}
			rot_y(vec);
		}
			rot_x(vec);
		}
	}

	for(i=0;i<256;i++) {
		if (tbl[i] == 0) {
			j = i;
			get_val(j,vec);
			printf("Unknown: %ld -> %d %d %d %d %d %d %d %d\n",j,vec[7],
				vec[6],vec[5],vec[4],vec[3],vec[2],vec[1],vec[0]);
		}
	}
	fp = fopen("skel3d_etable.h","w");
	if (fp == 0L) exit(0);
	fprintf(fp,"static short etable[256] = {\n");
	for(i=0;i<256;i+=16) {
		fprintf(fp,"\t");
		for(j=0;j<16;j++) {
			fprintf(fp,"% hd,",tblv[i+j]);
		}
		fprintf(fp,"\n");
	}
	fprintf(fp,"\t};\n");
	fclose(fp);
	
	exit(0);
}
