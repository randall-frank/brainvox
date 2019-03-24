#include <stdio.h>
#include <stdlib.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "nifti1.h"

/*
 * Interface for reading Analyze/NIFTI1 format files
 * Used by Brainvox
 */

/*
 * TODO:
 * long run: 
 *    read/write compressed (sequential?)
 *    flipping/transforms
 *    intents
 *    
 */

typedef struct {
   nifti_1_header hdr;
   FILE *fp;
   char *filename;
   int offset;  /* if non-zero, we have a "single file" case */
   int swap;
   int mode;
} NIFTIcache;

int iDataSpatialDims = 3;
int iDataHasTime = 0;

#define MAX_CACHE 100
NIFTIcache cache[MAX_CACHE];
int iCacheSize = 0;

/* local routines */
static void SwapNIFTIHdr(nifti_1_header *hdr);
static void SwapBytes(void *pin,int size,int num);
static NIFTIcache *check_cache(char *file, int mode, int dx, 
                int dy, int nslices, int dz, float ip, float is);

static NIFTIcache *check_cache(char *file, int mode, int dx, 
                int dy, int nslices, int dz, float ip, float is)
{
   int i;
   char tmp[1024],*p,*q,dfile[1024];
   NIFTIcache *nc;

   /* does the filename end in .hdr or .nii */
   strcpy(tmp,file);
   p = strrchr(tmp,'/');
   if (!p) p = tmp;
   q = strrchr(p,'\\');
   if (!q) q = p;
   p = strstr(q,".hdr");
   if (!p) p = strstr(q,".nii");
   if (!p) return(NULL);
   p[4] = '\0';

   /* is it in the cache */
   for(i=0;i<iCacheSize;i++) {
      if ((strcmp(tmp,cache[i].filename) == 0) && (mode == cache[i].mode)) {
#ifdef DEBUG
         printf("Found in cache: %s\n",tmp);
#endif
         return(&(cache[i]));
      }
   }

   /* make space (drop first read only) */
   if (iCacheSize >= MAX_CACHE-1) {
      int j;
      for(i=0;i<iCacheSize;i++) {
         if (cache[i].mode == 'r') {
            break;
         }
      }
      if (i == iCacheSize) {
         i = 0;
         fprintf(stderr,"Warning: Analyze write file %s closed\n",
                 cache[i].filename);
      }
      if (cache[i].fp) fclose(cache[i].fp);
      if (cache[i].filename) free(cache[i].filename); 
      for(j=i+1;j<iCacheSize;j++) cache[j-1] = cache[j];
      iCacheSize -= 1;
   }
   nc = &(cache[iCacheSize]);

   /* check the header or create the file(s) */
   if (mode == 0) {
      int single;
      nc->fp = fopen(tmp,"rb");
      if (!nc->fp) return(NULL);
      fread(&nc->hdr,sizeof(nc->hdr),1,nc->fp);
      nc->swap = bv_nifti_hdrcheck(&nc->hdr, &single, &nc->offset);
      if (nc->swap == -1) {
         fclose(nc->fp);
         return(NULL);
      }
      /* open the data file */
      if (!single) {
         fclose(nc->fp);
         strcpy(dfile,tmp);
         p = strrchr(dfile,'.');
         strcpy(p,".img");
         nc->fp = fopen(dfile,"rb");
         if (!nc->fp) return(NULL);
         nc->offset = 0;
      }
   } else if (mode == 1) {
      /* build a header */
      memset(nc,0,sizeof(*nc));
      nc->hdr.sizeof_hdr = sizeof(nc->hdr);
      strcpy(nc->hdr.db_name,"Brainvox");
      nc->hdr.dim[0] = 3;
      nc->hdr.dim[1] = dx;
      nc->hdr.dim[2] = dy;
      nc->hdr.dim[3] = nslices;
      nc->hdr.dim[4] = 1;
      nc->hdr.dim[5] = 1;
      nc->hdr.datatype = DT_UINT8;
      if (dz == 2) nc->hdr.datatype = DT_INT16;
      if (dz == 4) nc->hdr.datatype = DT_FLOAT32;
      nc->hdr.bitpix = dz*8;
      nc->hdr.pixdim[0] = 1.0;
      nc->hdr.pixdim[1] = ip;
      nc->hdr.pixdim[2] = ip;
      nc->hdr.pixdim[3] = is;
      nc->hdr.pixdim[4] = 1.0;
      nc->hdr.pixdim[5] = 1.0;
      if (strstr(tmp,".nii")) nc->offset = 352;
      strcpy(nc->hdr.magic,"ni1");
      if (nc->offset == 352) strcpy(nc->hdr.magic,"n+1");
      nc->hdr.vox_offset = nc->offset;
      strcpy(nc->hdr.descrip,"Brainvox output");
      /* if header already exists, compare values!!! */
      nc->fp = fopen(tmp,"rb");
      if (nc->fp) {
         nifti_1_header hdr;
         int sing,off;
         /* read header */
         fread(&hdr,sizeof(hdr),1,nc->fp);
         fclose(nc->fp);
         /* is it valid? */
         nc->swap = bv_nifti_hdrcheck(&hdr, &sing, &off);
         if (nc->swap == -1) {
            fclose(nc->fp);
            return(NULL);
         }
         /* compare datatype and dimensions to computed one, if bad match, err */
         if ((nc->hdr.datatype != hdr.datatype) || 
             (nc->hdr.dim[0] != hdr.dim[0]) || 
             (nc->hdr.dim[1] != hdr.dim[1]) || 
             (nc->hdr.dim[2] != hdr.dim[2]) || 
             (nc->hdr.dim[3] != hdr.dim[3])) {
            fprintf(stderr,"Error: NIFTI params do not match existing file\n");
            return(NULL);
         }
         /* set up the offset and open the proper file */
         nc->offset = off;
         if (sing) {
            nc->fp = fopen(tmp,"r+b");
            if (!nc->fp) return(NULL);
         } else {
            nc->offset = 0;
            strcpy(dfile,tmp);
            p = strrchr(dfile,'.');
            strcpy(p,".img");
            nc->fp = fopen(dfile,"r+b");
            if (!nc->fp) return(NULL);
         }
      } else {
         /* write header file */
         nc->fp = fopen(tmp,"wb");
         if (!nc->fp) return(NULL);
         fwrite(&nc->hdr,sizeof(nc->hdr),1,nc->fp);
         if (nc->offset) {
            i = sizeof(nc->hdr);
            while(i<nc->offset) {
               fwrite(&i,1,1,nc->fp);
               i += 1;
            }
         } else {
            fclose(nc->fp);
            strcpy(dfile,tmp);
            p = strrchr(dfile,'.');
            strcpy(p,".img");
            nc->fp = fopen(dfile,"wb");
            if (!nc->fp) return(NULL);
         }
         /* fill the data file with zeros (include time and components)... */
         i = nc->hdr.dim[1]*nc->hdr.dim[2]*nc->hdr.dim[3]*
             nc->hdr.dim[4]*nc->hdr.dim[5];
         while(i) {
            char buf[4096] = {0};
            int j = i;
            if (j > sizeof(buf)) j = sizeof(buf);
            fwrite(buf,j,1,nc->fp);
            i -= j;
         }
         nc->swap = 0;
      }
   } else return(NULL);

   nc->filename = strdup(tmp);
   nc->mode = mode;
#ifdef DEBUG
   printf("Adding to cache: %s\n",tmp);
#endif

   iCacheSize += 1;
   return(nc);
}

void bv_nifti_cleanup_(void)
{
   int i;
   for(i=0;i<iCacheSize;i++) {
      if (cache[i].fp) fclose(cache[i].fp);
      if (cache[i].filename) free(cache[i].filename); 
   }
   iCacheSize = 0;
   return;
}

int bv_nifti_io_(char *infile,int *iwrite, void *data,int *dx,int *dy,
     int *nslice, int *slice, int *dz,int *flip, float *ip, float *is, int slen) 
{
   char file[1024] = {0};
   int iOffset,iSlice;
   NIFTIcache *nc;
   nifti_1_header hdr;

   memcpy(file,infile,slen);
   if (strlen(file) < 5) return(-1);
   if ((strcmp(file+strlen(file)-4,".nii") != 0) &&
       (strcmp(file+strlen(file)-4,".hdr") != 0)) {
      return(-1);
   }
   if (*iwrite == 3) return(0);
   
   if (*iwrite == 2) {
      int single,swap,offset;
      FILE *fp;
      fp = fopen(file,"rb");
      if (!fp) return(-1);
      fread(&hdr,sizeof(hdr),1,fp);
      fclose(fp);
      swap = bv_nifti_hdrcheck(&hdr, &single, &offset);
      if (swap == -1) return(-1);
      *dx = hdr.dim[1];
      *dy = hdr.dim[2];
      *nslice = hdr.dim[3];
      *dz = 1;
      if (hdr.datatype == DT_UINT8) *dz = 1;
      if (hdr.datatype == DT_INT16) *dz = 2;
      if (hdr.datatype == DT_FLOAT32) *dz = 4;
      *ip = hdr.pixdim[1];
      *is = hdr.pixdim[3];
      *flip = 0;
      return(0);
   }

   /* update the cache */
   nc = check_cache(file,*iwrite,*dx,*dy,*nslice,*dz,*ip,*is);
   if (!nc) return(-1);

   /* some checks */
   if ((*dx != nc->hdr.dim[1]) || (*dy != nc->hdr.dim[2]) ||
       (*nslice != nc->hdr.dim[3])) {
      fprintf(stderr,"Error: Volume size does not match NIFTI1 header\n");
      return(-1);
   }
   if ((*slice < 1) || (*slice > nc->hdr.dim[3])) {
      fprintf(stderr,"Error: Slice number out of range from NIFTI1 header\n");
      return(-1);
   }
   if (((*dz == 1) && (nc->hdr.datatype != DT_UINT8)) ||
       ((*dz == 2) && (nc->hdr.datatype != DT_INT16)) ||
       ((*dz == 4) && (nc->hdr.datatype != DT_FLOAT32))) {
      fprintf(stderr,"Error: Volume data type does not match NIFTI1 header\n");
      return(-1);
   }
   /* perform the operation */
   /* 1=dx,2=dy,3=dz,4=time,5=component */
   iSlice = nc->hdr.dim[1]*nc->hdr.dim[2]*(*dz);
   iOffset = ((*slice)-1)*iSlice + nc->offset;
   fseek(nc->fp,iOffset,SEEK_SET);
   if (*iwrite == 0) {
      fread(data,iSlice,1,nc->fp);
      if (nc->swap) SwapBytes(data,*dz,nc->hdr.dim[1]*nc->hdr.dim[2]);
      if (flip) bv_yflip_(data,dx,dy,dz);
   } else if (*iwrite == 1) {
      if (nc->swap) SwapBytes(data,*dz,nc->hdr.dim[1]*nc->hdr.dim[2]);
      if (flip) bv_yflip_(data,dx,dy,dz);
      fwrite(data,iSlice,1,nc->fp);
      if (flip) bv_yflip_(data,dx,dy,dz);
      if (nc->swap) SwapBytes(data,*dz,nc->hdr.dim[1]*nc->hdr.dim[2]);
      fflush(nc->fp);
   } else return(-1);

   return(0);
}

#define MAGIC_LEN 	0x0000015c
#define MAGIC_LEN_SWAP	0x5c010000

/* Note 3D only!! */
int bv_nifti_hdrcheck(nifti_1_header *hdr, int *iSingle, int *iOff)
{
	int i;
	int iSwap = 0;

/* coming out of here, dims are:  x,y,z,t,num_vars */
	if (NIFTI_VERSION(*hdr) == 1) {
           iSwap = NIFTI_NEEDS_SWAP(*hdr);
           *iSingle = NIFTI_ONEFILE(*hdr);
           if (iSwap) SwapNIFTIHdr(hdr);
           for(i=hdr->dim[0]+1;i<8;i++) {
              hdr->dim[i] = 1;
              if (hdr->pixdim[i] <= 0.0) hdr->pixdim[i] = 1.0; 
           }
           hdr->dim[0] = 3;
        } else {
           int tmp[5] = {1,1,1,1,1};
           iSwap = NIFTI_NEEDS_SWAP(*hdr);
           if (iSwap) SwapNIFTIHdr(hdr);
           *iSingle = 0;
           *iOff = 0;
           for(i=hdr->dim[0]+1;i<8;i++) {
              hdr->dim[i] = 1;
              if (hdr->pixdim[i] <= 0.0) hdr->pixdim[i] = 1.0; 
           }
           hdr->dim[0] = 3;
           /* make the header look like an NIFTI header */
           /* map any extra dimensions */
           for(i=0;i<iDataSpatialDims;i++) tmp[i] = hdr->dim[i+1];
           if (iDataHasTime) {
              tmp[3] = hdr->dim[iDataSpatialDims+1];
              tmp[4] = hdr->dim[iDataSpatialDims+2];
           } else {
              tmp[3] = 1;
              tmp[4] = hdr->dim[iDataSpatialDims+1];
           }
           for(i=0;i<5;i++) hdr->dim[i+1] = tmp[i];
           /* fill in any other fields we look at with good defaults */
           hdr->vox_offset = 0.0;
           hdr->intent_code = NIFTI_INTENT_NONE;
           hdr->qform_code = NIFTI_XFORM_UNKNOWN;
           hdr->sform_code = NIFTI_XFORM_UNKNOWN;
           hdr->scl_slope = 0.0;  /* do not use scaling */
           hdr->scl_inter = 0.0;
           hdr->cal_min = 0.0;
           hdr->cal_max = 0.0;
           hdr->toffset = 0.0;
        }
        *iOff = (int)(hdr->vox_offset);

#if OLDMETHOD
	if (hdr->hk.sizeof_hdr == MAGIC_LEN) return(0);
	if (hdr->hk.sizeof_hdr != MAGIC_LEN_SWAP) return(-1);
	SwapAnalyzeHdr(hdr);
	return(1);
#endif
	return(iSwap);
}

static void SwapNIFTIHdr(nifti_1_header *hdr)
{

	SwapBytes(&(hdr->sizeof_hdr),sizeof(int),1);
	SwapBytes(&(hdr->extents),sizeof(int),1);
	SwapBytes(&(hdr->session_error),sizeof(short int),1);

	SwapBytes(hdr->dim,sizeof(short int),8);
	SwapBytes(&(hdr->intent_p1),sizeof(float),1);
	SwapBytes(&(hdr->intent_p2),sizeof(float),1);
	SwapBytes(&(hdr->intent_p3),sizeof(float),1);
	SwapBytes(&(hdr->intent_code),sizeof(short int),1);
	SwapBytes(&(hdr->datatype),sizeof(short int),1);
	SwapBytes(&(hdr->bitpix),sizeof(short int),1);
	SwapBytes(&(hdr->slice_start),sizeof(short int),1);
	SwapBytes(hdr->pixdim,sizeof(float),8);
	SwapBytes(&(hdr->vox_offset),sizeof(float),1);
	SwapBytes(&(hdr->scl_slope),sizeof(float),1);
	SwapBytes(&(hdr->scl_inter),sizeof(float),1);
	SwapBytes(&(hdr->slice_end),sizeof(short int),1);
	SwapBytes(&(hdr->cal_max),sizeof(float),1);
	SwapBytes(&(hdr->cal_min),sizeof(float),1);
	SwapBytes(&(hdr->slice_duration),sizeof(float),1);
	SwapBytes(&(hdr->glmin),sizeof(int),1);
	SwapBytes(&(hdr->glmax),sizeof(int),1);

	SwapBytes(&(hdr->qform_code),sizeof(short int),1);
	SwapBytes(&(hdr->sform_code),sizeof(short int),1);
	SwapBytes(&(hdr->quatern_b),sizeof(float),1);
	SwapBytes(&(hdr->quatern_c),sizeof(float),1);
	SwapBytes(&(hdr->quatern_d),sizeof(float),1);
	SwapBytes(&(hdr->qoffset_x),sizeof(float),1);
	SwapBytes(&(hdr->qoffset_y),sizeof(float),1);
	SwapBytes(&(hdr->qoffset_z),sizeof(float),1);
	SwapBytes(hdr->srow_x,sizeof(float),4);
	SwapBytes(hdr->srow_y,sizeof(float),4);
	SwapBytes(hdr->srow_z,sizeof(float),4);
}

static void SwapBytes(void *pin,int size,int num)
{
	int		i;
	unsigned char	*p = (unsigned char *)pin;
	unsigned char	c;

	switch(size) {
		case 2:
			for(i=0;i<num;i++) {
				c = p[1]; p[1] = p[0]; p[0] = c;
				p += size;
			}
			break;
		case 4:
			for(i=0;i<num;i++) {
				c = p[3]; p[3] = p[0]; p[0] = c;
				c = p[2]; p[2] = p[1]; p[1] = c;
				p += size;
			}
			break;
		case 8:
			for(i=0;i<num;i++) {
				c = p[7]; p[7] = p[0]; p[0] = c;
				c = p[6]; p[6] = p[1]; p[1] = c;
				c = p[5]; p[5] = p[2]; p[2] = c;
				c = p[4]; p[4] = p[3]; p[3] = c;
				p += size;
			}
			break;
	}
	return;
}
void    bv_yflip_(unsigned char *data,int *dx,int *dy,int *dz)
{
        int x,y;
        unsigned char *p1,*p2,t;

        for(y=0;y<(*dy/2);y++) {
                p1 = data + (y*(*dx)*(*dz));
                p2 = data + ((*dy-y-1)*(*dx)*(*dz));
                for(x=0;x<((*dx)*(*dz));x++) {
                        t = *p2;
                        *p2 = *p1;
                        *p1 = t;
                        p1++; p2++;
                }
        }
        return;
}

void    bv_xflip_(unsigned char *data,int *dx,int *dy,int *dz)
{
        int x,y,i;
        unsigned char *p1,*p2,t;

        for(y=0;y<*dy;y++) {
                p1 = data + (y*(*dx)*(*dz));
                p2 = p1 + (((*dx)-1)*(*dz));
                for(x=0;x<(*dx)/2;x++) {
                        for(i=0;i<(*dz);i++) {
                                t = p2[i];
                                p2[i] = p1[i];
                                p1[i] = t;
                        }
                        p1+=(*dz); p2-=(*dz);
                }
        }
        return;
}

