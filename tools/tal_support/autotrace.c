/*
 * Copyright (c), 1990-1998 Randall Frank and Thomas Grabowski
 *
 * $Id: autotrace.c 1250 2005-09-16 15:51:42Z dforeman $
 *
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

#include "autotrace.h"

#define MALLOC malloc
#define FREE free

#define ABS(a)   (((a)<0)?-(a):(a))

/* routines for implementing the volumetric dataset segmenting scheme */
/* which reserves the high order bit for a connectivity tag boolean */

/* generally visible routines */
/* prepare the volume by dividing it by 2 with optional histogram computation */
/* fills in hist[0-255] with a histogram of values if hist != 0L */
void volume_shift(Dvol *set,long int *hist);

/* clear all the high oder bits in the volume */
void volume_clear(Dvol *set,long int *hist);

/* recompute the volume histogram */
void volume_hist(Dvol *set,long int *hist);

/* perform 3D flood fill from a set of preselected seeds */
void volume_autotrace(Dvol *set,long int low,long int high,
	Point3D *seeds,long int nseeds,long int *count);

/* Set the high order bits in a cubic region around a list of points */
void volume_tag(Dvol *set,Point3D *seeds,long int npts,long int t_size,
	long int i);

/* perform 2D isovalue contour following */
void autotrace_2d(unsigned char *image,long int dx,long int dy,long int dz,
	long int val,long int sx,long int sy,long int *out,long int *num);

/* local subroutines for stack handling and 2D image grid access */
int stack_push(long int *stackptr,Point3D *stack,Point3D *add,
	unsigned char *dat,long int *mult);
int stack_pull(long int *stackptr,Point3D *stack,Point3D *ret);
long int grid(long int i,long int j,long int dx,long int dy,long int dz,
	unsigned char *image);

/* routine to divide the voxel volume by 2 */
void volume_shift(Dvol *set,long int *hist)
{
	register unsigned char *dat;
	register long int i;

	if (set->data == 0L) return;

	dat = (unsigned char *)set->data;
	if (hist == 0L) {
/* without a histogram */
		for(i=0;i < ((set->d[0])*(set->d[1])*(set->d[2]));i++) {
			*dat = (*dat >> 1);
			dat++;
		}
	} else {
/* with a histogram */
		for(i=0;i<256;i++) hist[i] = 0;
		for(i=0;i < ((set->d[0])*(set->d[1])*(set->d[2]));i++) {
			*dat = (*dat >> 1);
			hist[*dat]++;
			dat++;
		}
	}
}

/* routine to clear the volume high bit */
void volume_clear(Dvol *set,long int *hist)
{
	register unsigned char *dat;
	register long int i;

	if (set->data == 0L) return;

	dat = (unsigned char *)set->data;
	if (hist == 0L) {
/* without a histogram */
		for(i=0;i < ((set->d[0])*(set->d[1])*(set->d[2]));i++) {
			*dat = (*dat & 0x7f);
			dat++;
		}
	} else {
/* with a histogram */
		for(i=0;i<256;i++) hist[i] = 0;
		for(i=0;i < ((set->d[0])*(set->d[1])*(set->d[2]));i++) {
			*dat = (*dat & 0x7f);
			hist[*dat]++;
			dat++;
		}
	}
}

/* routine to set/clear highbits in cubic regions around a list of points */
void volume_tag(Dvol *set,Point3D *seeds,long int npts,long int t_size,
	long int val)
{
	register unsigned char *dat;
	register long int i,j,x,y,z;
	long int	mult[3],st,en;

	if (set->data == 0L) return;

	dat = (unsigned char *)set->data;

/* get planar mults */
        mult[0] = 1;
        mult[1] = set->d[0];
        mult[2] = (set->d[0])*(set->d[1]);

	st = -(t_size/2);
	en = st + t_size;
/* tag the points */
	for(i=0;i<npts;i++) {
		for(x=st;x<en;x++) {
		for(y=st;y<en;y++) {
		for(z=st;z<en;z++) {
/* make sure the point is legal */
		if ((seeds[i].x+x >= 0) && (seeds[i].x+x < set->d[0])
		   && (seeds[i].y+y >= 0) && (seeds[i].y+y < set->d[1])
		   && (seeds[i].z+z >= 0) && (seeds[i].z+z < set->d[2])) {
/* compute the index */
			j = ((seeds[i].x+x)*mult[0])+((seeds[i].y+y)*mult[1])+
				((seeds[i].z+z)*mult[2]);
/* (un)tag the point */
			dat[j] = ((dat[j] & 0x7f) | (val & 0x80));
		}
		}
		}
		}
	}
	return;
}

/* routine to compute the volume histogram */
void volume_hist(Dvol *set,long int *hist)
{
	register unsigned char *dat;
	register long int i;

	if ((set->data == 0L) || (hist == 0L)) return;

	dat = (unsigned char *)set->data;
	for(i=0;i<256;i++) hist[i] = 0;
	for(i=0;i < ((set->d[0])*(set->d[1])*(set->d[2]));i++) {
		hist[*dat]++;
		dat++;
	}
	return;
}

/* routines to handle a point stack with automatic stack thinning */

/* the stack is fixed in size */
#define MAX_STACK 50000

int stack_push(long int *stackptr,Point3D *stack,Point3D *add,
	unsigned char *dat,long int *mult)
{
	long int i,j;

	if (*stackptr < (MAX_STACK-1)) {
/* plenty of open space, so add the point */
		(*stackptr)++;
		stack[*stackptr] = *add;
		return(0);
	} else {
/* look for an open spot (one where a seed has been externally filled) */
		for(j=0;j<(*stackptr);j++) {
			i = (stack[j].x*mult[0])+(stack[j].y*mult[1])
				+(stack[j].z*mult[2]);
			if ((dat[i] & 0x80) == 0x80) {
				stack[j] = *add;
				return(0);
			}
		}
/* complete overflow */
		vl_puts("3D autotrace warning: stack overflow encountered\n");
		return(1);
	}
/* execution cannot reach here */
}

int stack_pull(long int *stackptr,Point3D *stack,Point3D *ret)
{
	if (*stackptr >= 0) {
		*ret = stack[*stackptr];
		(*stackptr)--;
		return(0);
	} else {
		return(1);
	}
/* execution cannot reach here */
}

/* routine to detect and "tag" all the four connected neighbors from
	a set of 3D seed points within two voxel values */
void volume_autotrace(Dvol *set,long int low,long int high,
	Point3D *seeds,long int nseeds,long int *count)
{
	register unsigned char *dat;
	register long int i,flag;
	long int	startx,endx,starti,err;
	long int 	mult[3],stackptr;
	Point3D		*stack,origin,pt;

/* no points filled */
	*count = 0;

/* is there valid data */
	if ((set->data == 0L) || (nseeds < 1)) return;
	dat = (unsigned char *)set->data;

/* allocate the stack */
	stack = (Point3D *)MALLOC(MAX_STACK*sizeof(Point3D));
	if (stack == 0L) return;

/* get planar mults */
        mult[0] = 1;
        mult[1] = set->d[0];
        mult[2] = (set->d[0])*(set->d[1]);

/* init the stack to the 3D seeds */
	stackptr = -1;
	for(i = 0; i<nseeds;i++) {
		stack_push(&stackptr,stack,&(seeds[i]),dat,mult);
	}

#ifdef DEBUG
	printf("low,high,stack: %d %d %d\n",low,high,stackptr);
#endif
	err = 0;
/* do the fill from the stack */
	while((stack_pull(&stackptr,stack,&origin) == 0)&&(err == 0)) {
/* allow the user to abort 
		if (vl_rcallback(-1L,set)) {
			vl_puts("User aborted autosegmentation.\n");
			err += 1;
		}
*/
#ifdef DEBUG
		printf("point pulled: %d %d %d\n",origin.x,origin.y,origin.x);
		printf("Points left: %d\n",stackptr);
#endif
/* check for out of bounds */
		if ((origin.x < (set->start[0])) || (origin.x > (set->end[0])) 
		  || (origin.y < (set->start[1])) || (origin.y > (set->end[1])) 
		  || (origin.z < (set->start[2])) || (origin.z > (set->end[2]))){
			flag = 0;
		} else {
			flag = 1;
		}
/* get the data pointer */
		i = (origin.x*mult[0])+(origin.y*mult[1])+(origin.z*mult[2]);
/* only do work if we have not been here before */
		if (((dat[i] & 0x80) == 0x00) && (flag == 1)) {
/* skid along the -X axis as far as we can */
			flag = 1;
			while (flag) {
				if ((origin.x >= (set->start[0])) && 
					(dat[i] <= high) && 
					(dat[i] >= low)) {
					origin.x--;
					i = i - mult[0];
				} else {
					flag = 0;
					origin.x++;
					i = i + mult[0];
				}
			}
/* save the starting x coordinate */
			startx = origin.x;
#ifdef DEBUG
			printf("Start of line at %d\n",startx);
#endif
			starti = i;
/* the current point is the first point in the scan line */
			while ((origin.x <= (set->end[0])) && 
				(dat[i] <= high) &&
				(dat[i] >= low)) {
/* fill in a point */
				dat[i] = dat[i] | 0x80;
				(*count)++;
				i = i + mult[0];
				origin.x++;
			}
/* save the ending x coordinate */
			endx = origin.x;
#ifdef DEBUG
			printf("End of line at %d\n",endx);
#endif
/* look for points to be pushed */
/* check y-1 */
			if (origin.y > (set->start[1])) {
				i = starti - mult[1];
				pt.y = origin.y - 1;
				pt.z = origin.z;
				flag = 0;
				for(pt.x=startx;pt.x<endx;pt.x++) {
					if (flag) {
/* we have a 1 to 0 transition */
						if (((dat[i] & 0x7f) > high) ||
						   ((dat[i] & 0x7f) < low)) { 
							flag = 0;
						}
					} else {
/* push the point of a 0 to 1 transition */
						if ((dat[i] <= high) &&
						   (dat[i] >= low)) {

				err+=stack_push(&stackptr,stack,&(pt),dat,mult);

							flag = 1;
						}
					}
					i = i + mult[0];
				}
			}
/* check y+1 */
			if ((origin.y) < (set->end[1])) {
				i = starti + mult[1];
				pt.y = origin.y + 1;
				pt.z = origin.z;
				flag = 0;
				for(pt.x=startx;pt.x<endx;pt.x++) {
					if (flag) {
/* we have a 1 to 0 transition */
						if (((dat[i] & 0x7f) > high) ||
						   ((dat[i] & 0x7f) < low)) { 
							flag = 0;
						}
					} else {
/* push the point of a 0 to 1 transition */
						if ((dat[i] <= high) &&
						   (dat[i] >= low)) {

				err+=stack_push(&stackptr,stack,&(pt),dat,mult);

							flag = 1;
						}
					}
					i = i + mult[0];
				}
			}
/* check z-1 */
			if (origin.z > (set->start[2])) {
				i = starti - mult[2];
				pt.y = origin.y;
				pt.z = origin.z - 1;
				flag = 0;
				for(pt.x=startx;pt.x<endx;pt.x++) {
					if (flag) {
/* we have a 1 to 0 transition */
						if (((dat[i] & 0x7f) > high) ||
						   ((dat[i] & 0x7f) < low)) { 
							flag = 0;
						}
					} else {
/* push the point of a 0 to 1 transition */
						if ((dat[i] <= high) &&
						   (dat[i] >= low)) {

				err+=stack_push(&stackptr,stack,&(pt),dat,mult);

							flag = 1;
						}
					}
					i = i + mult[0];
				}
			}
/* check z+1 */
			if (origin.z < (set->end[2])) {
				i = starti + mult[2];
				pt.y = origin.y;
				pt.z = origin.z + 1;
				flag = 0;
				for(pt.x=startx;pt.x<endx;pt.x++) {
					if (flag) {
/* we have a 1 to 0 transition */
						if (((dat[i] & 0x7f) > high) ||
						   ((dat[i] & 0x7f) < low)) { 
							flag = 0;
						}
					} else {
/* push the point of a 0 to 1 transition */
						if ((dat[i] <= high) &&
						   (dat[i] >= low)) {

				err+=stack_push(&stackptr,stack,&(pt),dat,mult);

							flag = 1;
						}
					}
					i = i + mult[0];
				}
			}
/* end of neighbor scanning */
			
		} /* if point was not hit previously */

	} /* pop a seed from the stack */

/* free up the stack */
	FREE(stack);
	return;
}

/* image access function (makes a image with a bounding box) */
long int grid(long int i,long int j,long int dx,long int dy,long int dz,
	unsigned char *image)
{
	long int	v;

/* check out of bounds */
	if ((i < 0) || (j < 0) || (i >= dx) || (j >= dy)) {
		if (dz > 0) {
			return(1000000L);
		} else {
			return(-1000000L);
		}
	}
	
	switch (ABS(dz)) {
		case 1:
			v = ((unsigned char *)image)[(j*dx)+i];
			break;
		case 2:
			v = ((unsigned short *)image)[(j*dx)+i];
			break;
		default:
			v = 1000000L;
			if (dz < 0) v *= -1;
			break;
	}
	return(v);
}

/* routine to compute a list of points at a given isocontour value 
	on an image slice */
void autotrace_2d(unsigned char *image,long int dx,long int dy,long int dz,
	long int dose,long int iin,long int jin,long int *out,long int *num)
{
	long int	i,j,numpts;
	long int	stx,sty,dir,sdir;
	long int	p[4],k;
	long int	v[4];
	long int	d[2][2][2];

/* init the directional array */
	d[0][0][0]= 1;
	d[0][0][1]=  0;
	d[0][1][0]= -1;
	d[0][1][1]= -1;
	d[1][0][0]= -1;
	d[1][0][1]= 1;
	d[1][1][0]=  0;
	d[1][1][1]= 1;
/*
	locations.   2   3    directions.  1
					 4-+-2
		     0   1                 3
	cases:
               -   -          - | +
	0,0,0    +--    0,0,1   | 
         +1    - | +     +0   - | +

               +   -          +   +
	0,1,0  --+      0,1,1 --+ 
         -1    - | +     -1   - | +
 
               -   -          -   +
 	1,0,0  --+      1,0,1   +--
         -1    + | -     +1   + | -
 
               + | -          +   +
 	1,1,0    |      1,1,1   +--
         +0    + | -     +1   + | -
 
 	If the point pair specified by (I,J)-(I+1,J) dose not bracket DOSE
 	the routine is to return NUM=0
*/
	numpts = (*num) - 2;
  	*num = 0;
	i=iin;
	j=jin;
/* check for point in image...  */
	while (1) {
		if ((i<1) || (i>(dx-1)) || (j<1) || (j>(dy-1))) return;
/* 	test to see if we even start... (if the dose is equal consider it
	to be greater than dose)
	If both are greater or less then slide to the left...
*/
		if ((grid(i,j,dx,dy,dz,image) < dose) &&
		    (grid(i+1,j,dx,dy,dz,image) < dose)) {
			i=i-1;
			continue;
		}
		if ((grid(i,j,dx,dy,dz,image) >= dose) && 
		    (grid(i+1,j,dx,dy,dz,image) >= dose)){
			i=i-1;
			continue;
		}
		break;
	}
/*
	set the starting location and begin...

 	directions...                     1
 					4-+-2
 					  3
*/
	stx=i;
	sty=j;
	sdir=1;
/* choose to start upward...  */
	dir=1;
/*
 	start the loop...                       v2   v3
 	Pick the values and evaluate them...       ^   
                                                v0 | v1
*/
	while (1) {
		if (dir == 1) {
			v[0]=grid(i,j,dx,dy,dz,image);
			v[1]=grid(i+1,j,dx,dy,dz,image);
			v[2]=grid(i,j+1,dx,dy,dz,image);
			v[3]=grid(i+1,j+1,dx,dy,dz,image);
		} else if (dir == 2) {   
			v[0]=grid(i,j+1,dx,dy,dz,image);
			v[1]=grid(i,j,dx,dy,dz,image);
			v[2]=grid(i+1,j+1,dx,dy,dz,image);
			v[3]=grid(i+1,j,dx,dy,dz,image);
		} else if (dir == 3) {   
			v[0]=grid(i+1,j+1,dx,dy,dz,image);
			v[1]=grid(i,j+1,dx,dy,dz,image);
			v[2]=grid(i+1,j,dx,dy,dz,image);
			v[3]=grid(i,j,dx,dy,dz,image);
		} else if (dir == 4) {   
			v[0]=grid(i+1,j,dx,dy,dz,image);
			v[1]=grid(i+1,j+1,dx,dy,dz,image);
			v[2]=grid(i,j,dx,dy,dz,image);
			v[3]=grid(i,j+1,dx,dy,dz,image);
		}
/* compare v0,v1,v2,v3 */
		for(k=0;k<4;k++) {
			if (v[k] < dose) {
				p[k]=0;
			} else {
				p[k]=1;
			}
		}
/* 	add the point between v(0) and v(1) to the list...  */
		*num = (*num) + 2;
/* 	if too trace is big then close and return */
		if ((*num) > numpts) {
			out[(*num)-2]=out[0];
			out[(*num)-1]=out[1];
			return;
		}
/*
	set the value..Based on direction (Left hand corner for P(1)=1 
		or right hand corner for p(1)=0)
*/
		if (dir == 1) {    
		    if (p[1] ==   2) {    
			out[(*num)-2]=i;
			out[(*num)-1]=j;
		    } else {
			out[(*num)-2]=i+1;
			out[(*num)-1]=j;
		    }
		} else if (dir ==   2) {    
		    if (p[1] ==   1) {    
			out[(*num)-2]=i;
			out[(*num)-1]=j+1;
		    } else {
			out[(*num)-2]=i;
			out[(*num)-1]=j;
		    }
		} else if (dir ==   3) {    
		    if (p[1] ==   1) {    
			out[(*num)-2]=i+1;
			out[(*num)-1]=j+1;
		    } else {
			out[(*num)-2]=i;
			out[(*num)-1]=j+1;
		    }
		} else if (dir ==   4) {    
		    if (p[1] == 1) {    
			out[(*num)-2]=i+1;
			out[(*num)-1]=j;
		    } else {
			out[(*num)-2]=i+1;
			out[(*num)-1]=j+1;
		    }
		}
/* If the point is a repeat then do not add it....  */
		if ((*num) > 3) {
			if ((out[(*num)-1] == out[(*num)-3]) && 
			    (out[(*num)-2] == out[(*num)-4])) {
				*num = (*num) - 2;
			}
		}
/*
 	Move along according to the table...Move only into a wall
 	where one corner is + and the other -
 	Note that by definition p(1)=1-p(0) and is not used...
*/
		dir=dir+d[p[0]][p[2]][p[3]];
/* bracket...  */
		if (dir >= 5) dir=1;
		if (dir <= 0) dir=4;
/* 	bump I and J along...  */
		if (dir == 1) {   
			j=j+1;
		} else if (dir == 2) {   
			i=i+1;
		} else if (dir == 3) {   
			j=j-1;
		} else if (dir == 4) {   
			i=i-1;
		}
/* 	test for end...  */
		if ((dir == sdir) && (i == stx) && (j == sty)) {
/* 	replicate first point and return */
			*num = (*num) + 2;
			out[(*num)-2] = out[0];
			out[(*num)-1] = out[1];
			return;
		}
	} /* this loop never terminates */
}

