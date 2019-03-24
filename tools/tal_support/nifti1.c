#include <stdio.h>
#include <stdlib.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "nifti1.h"
#include "proto.h"

/*
 * Interface for reading Analyze/NIFTI1 format files
 * Used by tal_programs and Brainvox
 *
 * {basename.hdr|basename.nii}%%%@nz={numslices}@t={n}@c={n}
 *
 * t= is the time point to sample (def = 0)
 * c= is the component to sample (def = 0)
 * nz= number of slices (used on creation)
 * ip= interpixel spacing (used on creation)
 * is= interslice spacing (used on creation)
 * it= intertime sample spacing (used on creation)
 * nt= number of timesteps (used on creation)
 * nc= number of components (used on creation)
 *
 */

/*
 * TODO:
 *    Brainvox?
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
   char mode;
   int time;
   int component;
} NIFTIcache;

int iDataSpatialDims = 3;
int iDataHasTime = 0;

#define MAX_CACHE 100
NIFTIcache cache[MAX_CACHE];
int iCacheSize = 0;

/* local routines */
static void SwapNIFTIHdr(nifti_1_header *hdr);
static void SwapBytes(void *pin,int size,int num);
static NIFTIcache *check_cache(char *file, char mode, long int dx, 
                        long int dy, long int dz, long int *slice);

static int scan_value_int(char *file,char *value,int def)
{
   char *p = strstr(file,value);
   if (!p) return(def);
   p += strlen(value);
   return(atoi(p));
}
static float scan_value_float(char *file,char *value,float def)
{
   char *p = strstr(file,value);
   if (!p) return(def);
   p += strlen(value);
   return((float)atof(p));
}

static NIFTIcache *check_cache(char *file, char mode, long int dx, 
                        long int dy, long int dz, long int *slice)
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
   *slice = atoi(p+4)-1;
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
   if (mode == 'r') {
      int single;
      nc->fp = fopen(tmp,"rb");
      if (!nc->fp) return(NULL);
      fread(&nc->hdr,sizeof(nc->hdr),1,nc->fp);
      nc->swap = NIFTI_hdrcheck(&nc->hdr, &single, &nc->offset);
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
   } else if (mode == 'w') {
      /* build a header */
      memset(nc,0,sizeof(*nc));
      nc->hdr.sizeof_hdr = sizeof(nc->hdr);
      strcpy(nc->hdr.db_name,"tal_program");
      nc->hdr.dim[0] = 3;
      nc->hdr.dim[1] = dx;
      nc->hdr.dim[2] = dy;
      nc->hdr.dim[3] = scan_value_int(file,"@nz=",1);
      nc->hdr.dim[4] = scan_value_int(file,"@nt=",1);
      if (nc->hdr.dim[4] > 1) nc->hdr.dim[0] = 4;
      nc->hdr.dim[5] = scan_value_int(file,"@nc=",1);
      if (nc->hdr.dim[5] > 1) nc->hdr.dim[0] = 5;
      if ((dx <= 0) || (dy <= 0) || (nc->hdr.dim[3] <= 0)) return(NULL);
      nc->hdr.datatype = DT_UINT8;
      if (dz == 2) nc->hdr.datatype = DT_INT16;
      if (dz == 4) nc->hdr.datatype = DT_FLOAT32;
      nc->hdr.bitpix = dz*8;
      nc->hdr.pixdim[0] = 1.0;
      nc->hdr.pixdim[1] = scan_value_float(file,"@ip=",1.0);
      nc->hdr.pixdim[2] = scan_value_float(file,"@ip=",1.0);
      nc->hdr.pixdim[3] = scan_value_float(file,"@is=",1.0);
      nc->hdr.pixdim[4] = scan_value_float(file,"@it=",1.0);
      nc->hdr.pixdim[5] = 1.0;
      if (strstr(tmp,".nii")) nc->offset = 352;
      strcpy(nc->hdr.magic,"ni1");
      if (nc->offset == 352) strcpy(nc->hdr.magic,"n+1");
      nc->hdr.vox_offset = nc->offset;
      strcpy(nc->hdr.descrip,"tal_program output");
      /* if header already exists, compare values!!! */
      nc->fp = fopen(tmp,"rb");
      if (nc->fp) {
         nifti_1_header hdr;
         int sing,off;
         /* read header */
         fread(&hdr,sizeof(hdr),1,nc->fp);
         fclose(nc->fp);
         /* is it valid? */
         nc->swap = NIFTI_hdrcheck(&hdr, &sing, &off);
         if (nc->swap == -1) {
            fclose(nc->fp);
            return(NULL);
         }
         /* compare datatype and dimensions to computed one, if bad match, error */
         if ((nc->hdr.datatype != hdr.datatype) || 
             (nc->hdr.dim[0] != hdr.dim[0]) || 
             (nc->hdr.dim[1] != hdr.dim[1]) || 
             (nc->hdr.dim[2] != hdr.dim[2]) || 
             (nc->hdr.dim[3] != hdr.dim[3]) || 
             (nc->hdr.dim[4] != hdr.dim[4])) {
            fprintf(stderr,"Error: NIFTI parameters do not match existing file\n");
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

   nc->time = scan_value_int(file,"@t=",0);
   nc->component = scan_value_int(file,"@c=",0);
   nc->filename = strdup(tmp);
   nc->mode = mode;
#ifdef DEBUG
   printf("Adding to cache: %s\n",tmp);
#endif

   iCacheSize += 1;
   return(nc);
}

void NIFTI_cleanup(void)
{
   int i;
   for(i=0;i<iCacheSize;i++) {
      if (cache[i].fp) fclose(cache[i].fp);
      if (cache[i].filename) free(cache[i].filename); 
   }
   iCacheSize = 0;
   return;
}

int NIFTI_io(char *file,char type,void *data,long int dx,long int dy,
             long int dz,long int flip) 
{
   long int slice,volume_size,iOffset;
   NIFTIcache *nc;

   /* update the cache */
   nc = check_cache(file,type,dx,dy,dz,&slice);
   if (!nc) return(-1);

   /* some checks */
   if ((dx != nc->hdr.dim[1]) || (dy != nc->hdr.dim[2])) {
      fprintf(stderr,"Error: Volume size does not match NIFTI1 header\n");
      return(-1);
   }
   if ((slice < 0) || (slice >= nc->hdr.dim[3])) {
      fprintf(stderr,"Error: Slice number out of range from NIFTI1 header\n");
      return(-1);
   }
   if (((dz == 1) && (nc->hdr.datatype != DT_UINT8)) ||
       ((dz == 2) && (nc->hdr.datatype != DT_INT16)) ||
       ((dz == 4) && (nc->hdr.datatype != DT_FLOAT32))) {
      fprintf(stderr,"Error: Volume data type does not match NIFTI1 header\n");
      return(-1);
   }
   /* perform the operation */
   volume_size = nc->hdr.dim[1]*nc->hdr.dim[2]*nc->hdr.dim[3]*dz;
   /* 1=dx,2=dy,3=dz,4=time,5=component */
   iOffset = nc->component*volume_size*nc->hdr.dim[4];
   iOffset += nc->time*volume_size;
   iOffset += slice*dx*dy*dz + nc->offset;
   fseek(nc->fp,iOffset,SEEK_SET);
   if (type == 'r') {
      fread(data,dx*dy*dz,1,nc->fp);
      if (nc->swap) SwapBytes(data,dz,dx*dy);
      if (flip) yflip(data,dx,dy,dz);
   } else if (type == 'w') {
      if (nc->swap) SwapBytes(data,dz,dx*dy);
      if (flip) yflip(data,dx,dy,dz);
      fwrite(data,dx*dy*dz,1,nc->fp);
      if (flip) yflip(data,dx,dy,dz);
      if (nc->swap) SwapBytes(data,dz,dx*dy);
      fflush(nc->fp);
   } else return(-1);

   return(0);
}

#define MAGIC_LEN 	0x0000015c
#define MAGIC_LEN_SWAP	0x5c010000

int NIFTI_hdrcheck(nifti_1_header *hdr, int *iSingle, int *iOff)
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
           hdr->dim[0] = 5;
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
           hdr->dim[0] = 5;
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

