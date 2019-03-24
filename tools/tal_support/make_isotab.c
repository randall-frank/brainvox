/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski
 *
 * $Id: make_isotab.c 1250 2005-09-16 15:51:42Z dforeman $
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


/*
Vertex numbering:
            6------------7
           /|           /|
          2------------3 |
          | |          | |
          | 4----------|-5
Y         |/           |/
| Z       0------------1
|/
+---X
Edge numbering:
            +-----10-----+
           5|           6|
          +-----2------+ 11
          | 9          | |
          1 +-----8----3-+
          |4           |7
          +-----0------+

*/

static int etab[15][15] = {
	{-1},
	{0,4,1,-1},
	{4,3,7, 1,3,4,-1},
	{1,0,4, 2,6,3,-1},
	{10,11,6, 1,0,4,-1},
	{4,0,3, 9,4,3, 11,9,3,-1},
	{4,3,7, 1,3,4, 10,11,6,-1},
	{10,11,6, 7,0,3, 2,1,5,-1},
	{9,1,3, 9,3,11,-1},
	{5,1,0, 5,0,10, 10,0,7, 10,7,11,-1},
	{5,2,4, 4,2,0, 8,6,10, 6,8,7,-1},
	{9,1,0, 9,0,7, 9,7,6, 10,9,6,-1},
	{4,0,1, 11,9,3, 9,2,3, 9,5,2,-1},
	{1,0,4, 11,8,7, 2,6,3, 5,9,10,-1},
	{1,2,9, 6,9,2, 6,8,9, 6,7,8,-1}
};

static int vtab[15][8] = {
	{0,0,0,0, 0,0,0,0},
	{1,0,0,0, 0,0,0,0},
	{1,1,0,0, 0,0,0,0},
	{1,0,0,1, 0,0,0,0},
	{1,0,0,0, 0,0,0,1},
	{0,1,0,0, 1,1,0,0},
	{1,1,0,0, 0,0,0,1},
	{0,1,1,0, 0,0,0,1},
	{1,1,0,0, 1,1,0,0},
	{1,0,0,0, 1,1,1,0},
	{1,0,1,0, 0,1,0,1},
	{1,0,0,0, 1,1,0,1},
	{0,1,1,0, 1,1,0,0},
	{1,0,0,1, 0,1,1,0},
	{1,1,0,1, 1,0,0,0}   /*isomer of 1,0,0,0 1,1,0,1 */
};

void	RotZ(long int *v,long int *e);
void	RotY(long int *v,long int *e);
void	RotX(long int *v,long int *e);
int	which(long int *e,long int i);

int	which(long int *e,long int i)
{
	int j;
	for (j=0;j<12;j++) {
		if (e[j] == i) return(j);
	}
	return(-1);
}

void	RotX(long int *v,long int *e) 
{
	long int temp[20],i;
/* rotate verticies first */
	for(i=0;i<8;i++) temp[i] = v[i];
	v[2] = temp[0];
	v[3] = temp[1];
	v[6] = temp[2];
	v[7] = temp[3];
	v[4] = temp[6];
	v[5] = temp[7];
	v[0] = temp[4];
	v[1] = temp[5];
/* rotate edge numbers */
	for(i=0;i<12;i++) temp[i] = e[i];
	e[2] = temp[0];
	e[10] = temp[2];
	e[8] = temp[10];
	e[0] = temp[8];
	e[5] = temp[1];
	e[9] = temp[5];
	e[4] = temp[9];
	e[1] = temp[4];
	e[6] = temp[3];
	e[11] = temp[6];
	e[7] = temp[11];
	e[3] = temp[7];

	return;
}
void	RotY(long int *v,long int *e) 
{
	long int temp[20],i;
/* rotate verticies first */
	for(i=0;i<8;i++) temp[i] = v[i];
	v[3] = temp[2];
	v[1] = temp[0];
	v[5] = temp[1];
	v[7] = temp[3];
	v[6] = temp[7];
	v[4] = temp[5];
	v[2] = temp[6];
	v[0] = temp[4];
/* rotate edge numbers */
	for(i=0;i<12;i++) temp[i] = e[i];
	e[2] = temp[5];
	e[6] = temp[2];
	e[10] = temp[6];
	e[5] = temp[10];
	e[0] = temp[4];
	e[7] = temp[0];
	e[8] = temp[7];
	e[4] = temp[8];
	e[3] = temp[1];
	e[11] = temp[3];
	e[9] = temp[11];
	e[1] = temp[9];

	return;
}
void	RotZ(long int *v,long int *e) 
{
	long int temp[20],i;
/* rotate verticies first */
	for(i=0;i<8;i++) temp[i] = v[i];
	v[0] = temp[2];
	v[2] = temp[3];
	v[3] = temp[1];
	v[1] = temp[0];
	v[5] = temp[4];
	v[7] = temp[5];
	v[6] = temp[7];
	v[4] = temp[6];
/* rotate edge numbers */
	for(i=0;i<12;i++) temp[i] = e[i];
	e[3] = temp[0];
	e[2] = temp[3];
	e[1] = temp[2];
	e[0] = temp[1];
	e[11] = temp[8];
	e[10] = temp[11];
	e[9] = temp[10];
	e[8] = temp[9];
	e[5] = temp[6];
	e[4] = temp[5];
	e[7] = temp[4];
	e[6] = temp[7];

	return;
}

int main(int argc, char **argv)
{
	long int v[8],e[12];
	long int i,j,l1,l2,l3,b;
	long int chk[256];
	
	int outtab[256][15];

	for(i=0;i<256;i++) chk[i] = -1;
	for(i=0;i<15;i++) {
		for(j=0;j<8;j++) v[j] = vtab[i][j];
		for(j=0;j<12;j++) e[j] = j;
		for(l1=0;l1<4;l1++) {
			RotX(v,e);
		for(l2=0;l2<4;l2++) {
			RotY(v,e);
		for(l3=0;l3<4;l3++) {
			RotZ(v,e);
/* check the PLUS phase */
			b=v[0]+(v[1]<<1)+(v[2]<<2)+(v[3]<<3)+(v[4]<<4)+
				(v[5]<<5)+(v[6]<<6)+(v[7]<<7);
			if (chk[b] == -1) {
				chk[b] = i;
				j = 0;
				while(etab[i][j] != -1) {
					outtab[b][j] = which(e,etab[i][j]);
					outtab[b][j+1] = which(e,etab[i][j+1]);
					outtab[b][j+2] = which(e,etab[i][j+2]);
					j += 3;
				}
				outtab[b][j] = -1;
			} else if ((chk[b] != i) && (chk[b] != i+256)) {
				fprintf(stderr,"Warning: %ld roted to %ld\n",chk[b],i);
			}
/* check the MINUS phase */
			j = ~b & 0xff;
			b=(1-v[0]);
			b+=((1-v[1]) << 1);
			b+=((1-v[2]) << 2);
			b+=((1-v[3]) << 3);
			b+=((1-v[4]) << 4);
			b+=((1-v[5]) << 5);
			b+=((1-v[6]) << 6);
			b+=((1-v[7]) << 7);
			if (j != b) {
				fprintf(stderr,"j=%ld  b=%ld\n",j,b);
			}
			if (chk[b] == -1) {
				chk[b] = i+256;
				j = 0;
				while(etab[i][j] != -1) {
/* switch the normal direction */
					outtab[b][j+2] = which(e,etab[i][j]);
					outtab[b][j+1] = which(e,etab[i][j+1]);
					outtab[b][j] = which(e,etab[i][j+2]);
					j += 3;
				}
				outtab[b][j] = -1;
			} else if ((chk[b] != i) && (chk[b] != i+256)) {
			    fprintf(stderr,"Warning: %ld roted to %ld\n",chk[b],i+256);
			}
		}
		}
		}
	}
	printf("\n");

	printf("static short iso_tab[256][15] = {\n");
	b=0;
	for(i=0;i<256;i++) {
		if (chk[i] == -1) {
			fprintf(stderr,"Warning: codes for %ld are undefined.\n",i);
			b++;
		} else {
			printf("/* %ld */ {",i);
			j = 0;
			while (outtab[i][j] != -1) {
				printf("%d,",outtab[i][j]);
				j++;
				if ((j % 3) == 0) printf("  ");
			}	
			if (i != 255) {
				printf("-1},\n");
			} else {
				printf("-1}\n");
			}

		}
	}
	printf("};\n");
	if (b != 0) fprintf(stderr,"Undefined count=%ld\n",b);
	exit(0);
}
