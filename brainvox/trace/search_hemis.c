#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "opt_search.h"

void hemi_search_(long int dx,long int dy,long int icount,long int skip,
	unsigned char *tex,unsigned char *mri,long int *depth)
{
	FILE		*fp;
	long int	width = 20;
	long int	midline = 128;

	long int	i,j,k,x,p,xx,yy,m,size[3];
	float		*cost,v;
        long int        range[3] = {1,1,1};
        long int        wrap[3] = {0,0,0};

#ifdef DEBUG
	printf("%ld %ld %ld %ld\n",dx,dy,icount,skip);
	printf("%ld %ld %ld %ld\n",tex,mri,midline,width);
#endif

/* clear depth buffer */
	for(i=0;i<dy*icount;i++) depth[i] = 0;

/* get cost buffer */
	cost = (float *)malloc(icount*dy*2*width*sizeof(float));
	if (cost == 0) return;

/* clear buffers */
	for(i=0;i<dy*icount;i++) depth[i] = 0;

	fprintf(stderr," Building the cost matrix...\n");
	x = 0;
	for(k=0;k<2*width;k++) {
	for(j=0;j<dy;j++) {
	for(i=0;i<icount;i++) {
		v = 0;
		p = (i*(skip+1)*dx*dy) + ((dy-j-1)*dx) + (k+midline-width);
		if ((tex[p] > 0) && (i > 0) && (i < icount-1) && (j>0)
			 && (j<dy-1)) {
			for(xx=-1;xx<=1;xx++) {
			for(yy=-1;yy<=1;yy++) {
				m = p + (yy*dx) + (xx*(skip+1)*dx*dy);
				v = v + tex[m] + tex[m+1] + tex[m-1] - 3;
			}
			}
			v = (v/27.0);
			v += mri[p];
		} 
		v = v/2;
		if (v > 255) v = 255;
		if (v < 0) v = 0;
		cost[x++] = v;
	}
	}
	}

	fprintf(stderr," Searching the cost matrix...\n");

#ifdef NEVER
	fp = fopen("cost124x256x40.flt","w");
	fwrite(cost,icount*dy*width*2,sizeof(float),fp);
	fclose(fp);
#endif

/* compute the result */
        size[0] = icount;
        size[1] = dy;
        size[2] = width*2;
        if (opt_search(3,size,cost,depth,range,wrap,OPT_NORM|OPT_TRUNC)==1) {
                fprintf(stderr,"Error in search routine...");
        }

	fprintf(stderr," Done.\n");

/* place it in the texture volume? */
	for(i=0;i<dx*dy*(icount+((icount-1)*skip));i++) tex[i] = 0;

	for(j=0;j<dy;j++) {
	for(i=0;i<icount;i++) {
		p = (i*(skip+1)*dx*dy) + ((dy-j-1)*dx);
		x = i+(j*icount);
		for(k=0;k<2*width;k++) {
			if (k != depth[x]) {
				xx = i+(j*icount)+k*(dy*icount);
				tex[p+midline-width+k] = cost[xx];
			} else {
				tex[p+midline-width+k] = 255;
			}
		}
	}
	}

/* all done */
	free(cost);
	return;
}
void HEMI_SEARCH(long int dx,long int dy,long int icount,long int skip,
	unsigned char *tex,unsigned char *mri,long int *depth)
{
hemi_search_(dx,dy,icount,skip,tex,mri,depth);
}
