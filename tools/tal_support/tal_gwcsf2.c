#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "proto.h"
#include "autotrace.h"
#include "skeleton.h"

extern long int verbose;

void do_rim_cleanup(unsigned char *data,unsigned char *mask,
	unsigned char *Gb,unsigned char *Wb,unsigned char *Cb,
	unsigned char *Fb,long int *dd,long int *size,
	char *G_O_temp,char *W_O_temp,char *C_O_temp,
	long int istart,long istep,long int swab,long int options);

#define MAX_DEPTH 8

void do_rim_cleanup(unsigned char *data,unsigned char *mask,
	unsigned char *Gb,unsigned char *Wb,unsigned char *Cb,
	unsigned char *Fb,long int *dd,long int *size,
	char *G_O_temp,char *W_O_temp,char *C_O_temp,
	long int istart,long istep,long int swab,long int options)
{
	long int	hist[256];
	long int	i,j,k,v,err,z;
	char            tstr[256];
	Dvol		thevol;
	Point3D         seeds[10];
	float		percent[MAX_DEPTH]={0.0,0.7,0.7,0.2,0.0,0.0,0.0,0.0};
	float		g,w,c,d;
	
	thevol.data = data;
	for(i=0;i<3;i++) thevol.d[i] = size[i];
        for(i=0;i<3;i++) {
                thevol.start[i] = 0;
                thevol.end[i] = thevol.d[i] - 1;
        }
        seeds[0].x = 1;
        seeds[0].y = 1;
        seeds[0].z = 1;
        seeds[0].col = 1;

	/* 1 - EDM the mask */
	fprintf(stderr,"Computing original mask EDM...\n");
	for(i=0;i<256;i++) hist[i] = 0;
	memcpy(data,mask,size[0]*size[1]*size[2]);
	calc_edm_3d_(data,size[0],size[1],size[2],1,hist);

	/* 2 - compute new mask = ((mask>0)-((csf>128)*(edm<10)))*255 */
	/*     mask = mask-(csf within 10 mm of surface) */
	fprintf(stderr,"Computing new mask...\n");
	k = 0;
	z = istart;
	for(i=0;i<size[2];i++) {
/* read the CSF */
                name_changer(C_O_temp,z,&err,tstr);
                fprintf(stderr,"Reading the CSF file:%s\n",tstr);
                bin_io(tstr,'r',Cb,size[0],size[1],1,1,swab,0L);
/* apply formula */
		for(j=0;j<(size[0]*size[1]);j++) {
			v = 0;
			if (mask[k]) {
				v = 127;
				if ((Cb[j] > 127) && (data[k] < 10)) {
					v = 0;
				};
			};
			data[k] = v;
			k++;
		};
		z += istep;
	};

	/* 3 - remask the new mask and EDM it */

	fprintf(stderr,"Remasking the new mask...\n"); 
	volume_autotrace(&thevol,0,10,seeds,1,&i);
	for(i=0;i<(size[0]*size[1]*size[2]);i++) {
                if (data[i] >= 128) {
                        data[i] = 0;
                } else {
                        data[i] = 255;
                }
        }

	fprintf(stderr,"Computing new mask EDM...\n");
	calc_edm_3d_(data,size[0],size[1],size[2],1,hist);

	/* 4 - apply the new mask to the GRAY matter rim */
	fprintf(stderr,"Applying new mask EDM to Gray probs...\n");
	k = 0;
	z = istart;
	for(i=0;i<(size[2]);i++) {
/* read the GRAY */
                name_changer(G_O_temp,z,&err,tstr);
                fprintf(stderr,"Reading the Gray file:%s\n",tstr);
                bin_io(tstr,'r',Gb,size[0],size[1],1,1,swab,0L);
/* read the WHITE */
                name_changer(W_O_temp,z,&err,tstr);
                fprintf(stderr,"Reading the White file:%s\n",tstr);
                bin_io(tstr,'r',Wb,size[0],size[1],1,1,swab,0L);
/* read the CSF */
                name_changer(C_O_temp,z,&err,tstr);
                fprintf(stderr,"Reading the CSF file:%s\n",tstr);
                bin_io(tstr,'r',Cb,size[0],size[1],1,1,swab,0L);
/* apply formula */
		for(j=0;j<(size[0]*size[1]);j++) {
			v = data[k];
			if ((v > 0) && (v < MAX_DEPTH)) {
				g = (float)(Gb[j]) / 255.0;
				w = (float)(Wb[j]) / 255.0;
				c = (float)(Cb[j]) / 255.0;
				d = percent[v];
				if (d > g) {
					g = (g + (2.0*d))/3.0;
					d = g + w + c;
					if (d == 0.0) d = 1.0;
					g /= d;
					w /= d;
					c /= d;
					Gb[j] = g*255.0;
					Wb[j] = w*255.0;
					Cb[j] = c*255.0;
				}
			}
			k++;
		}
/* write the GRAY */
                name_changer(G_O_temp,z,&err,tstr);
                fprintf(stderr,"Writing the Gray file:%s\n",tstr);
                bin_io(tstr,'w',Gb,size[0],size[1],1,1,swab,0L);
/* write the WHITE */
                name_changer(W_O_temp,z,&err,tstr);
                fprintf(stderr,"Writing the White file:%s\n",tstr);
                bin_io(tstr,'w',Wb,size[0],size[1],1,1,swab,0L);
/* write the CSF */
                name_changer(C_O_temp,z,&err,tstr);
                fprintf(stderr,"Writing the CSF file:%s\n",tstr);
                bin_io(tstr,'w',Cb,size[0],size[1],1,1,swab,0L);

		z += istep;
	}

	return;
}

