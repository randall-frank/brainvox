#include <stdio.h>
#include <stdlib.h>
#include "voxel.h"

#ifndef SEEK_SET
#define SEEK_SET 0
#endif

void vl_read_images_( char *temp,long int x,long int y, long int st,
	long int ed,long int step,long int interp,unsigned char *data)
{
/* pass it off as a read with a header size of 0 */
	vl_read_images_h_(temp,x,y,st,ed,step,interp,data,0L);
}

void vl_read_pal_(char *fname,unsigned char *data)
{
	FILE *fid;
	long int i;
	char t1[256];

	sprintf(t1,"Reading palette file:%s\n",fname);  
	vl_puts(t1);
	if ((fid = fopen(fname,"rb")) != NULL)  {
		(void) fread(&(data[0]),1,768,fid);
		(void) fclose(fid);
	} else {
		vl_puts("Using a linear ramp\n");
		for(i=0; i<256; i++) {
			data[i] = i;
			data[i+256] = i;
			data[i+512] = i;
		};
	};
}

void vl_read_images_h_( char *temp,long int x,long int y, long int st,
	long int ed,long int step,long int interp,unsigned char *data,
	long int header)
{
	long int i,error,count,ocount;
	char fname[255],t1[255];
	unsigned char *tbuf;
	FILE *fid;
	
	count = 0;
	tbuf = 0;
	if (header != 0) tbuf = malloc(header);

	for(i=st; i<=ed; i = i + step) {
		(void) vl_name_changer(temp,i,&error,fname);
		if (error != 0) {
			vl_puts("Invalid filename template.\n");
			goto errout;
		}
		sprintf(t1,"Reading:%s\n",fname);
		vl_puts(t1);
#ifdef __unix
		sprintf(t1,"zcat %s.Z",fname);
#endif
		if ((fid = fopen(fname,"rb")) != NULL)  {
			if (header != 0) (void) fseek(fid,header,SEEK_SET);
			(void) fread(&(data[count]),1,x*y,fid);
			(void) fclose(fid);
#ifdef __unix
/* attempt to read compressed images (zcat file.Z...) */
		} else if ((fid = popen(t1,"rb")) != NULL)  {
			if ((header != 0) && (tbuf != 0)) {
				(void) fread(tbuf,1,header,fid);
			}
			(void) fread(&(data[count]),1,x*y,fid);
			(void) pclose(fid);
#endif
		} else {
			sprintf(t1,"Unable to read:%s\n",fname);
			vl_puts(t1);
		};
		if ((i != st) && (interp > 0)) {
			/* do interpolations from previous image to current */
			(void) vl_interp_images_(&(data[ocount]),
				&(data[count]),x,y,interp,
				&(data[ocount+(x*y)]));
		};
	/* bump pointer past last image */
		ocount = count;
		count = count + ((interp+1)*(x*y));
	}
errout:
	if (tbuf != 0) free(tbuf);
}
