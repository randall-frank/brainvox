#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <sys/types.h>
#ifdef WIN32
#include <windows.h>
#else
#include <sys/ipc.h>
#include <sys/shm.h>
#endif
#include <errno.h>
#include "bv_shmem.h"

/* these routines are assumed to be a language independent interface to
	the shared memory routines */

#ifdef WIN32

#include <signal.h>

#define IPC_RMID  0x1000
#define IPC_STAT  0x1002
#define IPC_CREAT 0x0200
#define IPC_EXCL  0x0400

#define SHM_R 0x01
#define SHM_W 0x02

struct ipc_perm {
   int key;
};

struct shmid_ds {
   struct ipc_perm shm_perm;
   int shm_segsz;
   int shm_nattch;
   int shm_ctime;
};

typedef struct {
   int size;
   int num;
} shm_hdr_;

typedef struct {
   int key;
   HANDLE hmap;
   void *addr;
   int maps;
} win_shm;

int n_shm = 0;
win_shm l_shm[1000];

/* Detach from a shared mem area based on its address */ 
int shmdt(const void *shmaddr) 
{ 
    int i;
    shm_hdr_ *tmp;
    for(i=0;i<n_shm;i++) {
       tmp = (shm_hdr_ *)(((unsigned char *)shmaddr) - sizeof(shm_hdr_));
       if ((void *)tmp == l_shm[i].addr) {
          if (tmp->num) tmp->num -= 1;
          l_shm[i].maps -= 1;
          if (l_shm[i].maps == 0) {
             int j;
             UnmapViewOfFile(l_shm[i].addr);
             CloseHandle(l_shm[i].hmap);
             for(j=i+1;j<n_shm;j++) l_shm[j-1] = l_shm[j];
             n_shm -= 1;
          }
          return -1;
       }
    }
    return -1; 
} 
 
/* on a "crash", clean up the shared memory as best we can... */
static int inst_sig = 0;
static int clean_shm(int in)
{
   while(n_shm) shmctl(l_shm[0].key,IPC_RMID);
   exit(0);
}

/* Attach to an existing area */ 
void *shmat(int memId, void *shmaddr, int flag) 
{ 
   shm_hdr_ *tmp;
   char szShareMem[50]; 
   int i;
   
   if (!inst_sig) {
      signal(SIGABRT,clean_shm);
      signal(SIGILL,clean_shm);
      signal(SIGINT,clean_shm);
      signal(SIGSEGV,clean_shm);
      signal(SIGTERM,clean_shm);
      inst_sig = 1;
   }

   for(i=0;i<n_shm;i++) if (memId == l_shm[i].key) break;
   /* open a new file... */
   if (i == n_shm) {
      sprintf(szShareMem, "bvshmem.%d", memId);
      l_shm[i].hmap = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, szShareMem); 
      if (!l_shm[i].hmap) return (void *)-1;
      l_shm[i].addr = NULL;
      l_shm[i].key = memId;
      l_shm[i].maps = 0;
      n_shm += 1;
   }
   if (!l_shm[i].addr) {
      l_shm[i].addr = (void *)MapViewOfFileEx(l_shm[i].hmap, 
                             FILE_MAP_WRITE | FILE_MAP_READ, 0, 0, 0, NULL); 
      if (!l_shm[i].addr) return  (void *)-1; 
   }
   l_shm[i].maps += 1;
   tmp = (shm_hdr_ *)l_shm[i].addr;
   tmp->num += 1;
 
   return ((unsigned char *)l_shm[i].addr) + sizeof(shm_hdr_);
} 
 
/* Control a shared mem area */ 
int shmctl(int shmid, int flag, struct shmid_ds *out) 
{ 
   int i,j;
   shm_hdr_ *tmp;
   if (flag == IPC_RMID) { 
      for(i=0;i<n_shm;i++) {
         if (shmid == l_shm[i].key) {
            if (l_shm[i].addr) {
               tmp = (shm_hdr_ *)l_shm[i].addr;
               tmp->num -= l_shm[i].maps;
               UnmapViewOfFile(l_shm[i].addr);
            }
            CloseHandle(l_shm[i].hmap);
            for(j=i+1;j<n_shm;j++) l_shm[j-1] = l_shm[j];
            n_shm -= 1;
            return 0; 
         }
      }
      return -1;
    } else if (flag == IPC_STAT) { 
        void *shmaddr = shmat(shmid, NULL, 0);
        if ((int)(shmaddr) < 0) { 
            /* Shared memory does not exist */ 
            errno = EINVAL; 
            return -1; 
        } else { 
            tmp = (shm_hdr_ *)(((unsigned char *)shmaddr) - sizeof(shm_hdr_));
            out->shm_perm.key = shmid;
            out->shm_segsz = tmp->size;
            out->shm_nattch = tmp->num-1;
            out->shm_ctime = 0;
            shmdt(shmaddr);
            errno = 0; 
            return 0; 
        } 
    } 
 
    errno = EINVAL; 
    return -1; 
} 
 
/* Get an area based on the IPC key */ 
int shmget(int memKey, int size, int flag) 
{
   char        szShareMem[50]; 
   DWORD       dwRet; 
   shm_hdr_    *tmp;

   sprintf(szShareMem, "bvshmem.%d", memKey);
   if (flag & IPC_CREAT) { 
      l_shm[n_shm].hmap = CreateFileMapping((HANDLE)INVALID_HANDLE_VALUE, 
                    NULL,PAGE_READWRITE,0L,size+sizeof(shm_hdr_),szShareMem); 
      /* init the header */
      if (l_shm[n_shm].hmap) {
         l_shm[n_shm].key = memKey;
         l_shm[n_shm].maps = 0;
         l_shm[n_shm].addr = (void *)MapViewOfFileEx(l_shm[n_shm].hmap, 
                             FILE_MAP_WRITE | FILE_MAP_READ, 0, 0, 0, NULL); 
         if (l_shm[n_shm].addr) {
            tmp=(shm_hdr_ *)(l_shm[n_shm].addr);
            tmp->size = size;
            tmp->num = 0;
         }
         n_shm += 1;
      } else {
         return -1; 
      }
   } else { 
      l_shm[n_shm].hmap=OpenFileMapping(FILE_MAP_ALL_ACCESS,FALSE,szShareMem); 
      if (!l_shm[n_shm].hmap) { 
          errno = ENOENT; 
          return -1; 
      } 
      l_shm[n_shm].key = memKey;
      l_shm[n_shm].maps = 0;
      l_shm[n_shm].addr = NULL;
   } 
   dwRet = GetLastError(); 
   if (dwRet == ERROR_ALREADY_EXISTS && (flag & IPC_EXCL)) { 
      /* Caller wanted to create the segment -- error if already exists */ 
      CloseHandle(l_shm[n_shm].hmap); 
      errno = EEXIST; 
      return -1; 
   } 
   n_shm += 1;
 
   return memKey; 
} 

#endif

void GET_NEXT_KEY(int *key,int inc)
{
get_next_key_(key,inc);
}
/* routine to find the next unused key value */
void get_next_key_(int *key,int inc)
{
	int flags,shmid,i;

/* try to allocate segments until one fails */
	i = *key;

	while (i < 1000000) {
/* look for existing segments */
		flags = (SHM_R | SHM_W);
		shmid = shmget(i, 10000, flags);
		if (shmid == -1) {
/* if no segment exists error, then we have our man */
			if (errno == ENOENT) {
				*key = i;
				return;
			}
		}
		i += inc;
	}
/* error (generally) */
	return;
}

#define RIGHTS (SHM_R | SHM_W | 0040 | 0020)

void CREATE_SHARED_MEM(int key, int size, int *shmid)
{
	create_shared_mem_(key,size,shmid);
}

/* routine to allocate a shared memory block */
void create_shared_mem_(int key, int size, int *shmid)
{
	int flags;
	flags = (IPC_CREAT | RIGHTS); /* create user RW group RW*/
	*shmid = shmget(key, size, flags);
}

/* routine to get shared memory status */
void STATUS_SHARED_MEM(int shmid,int *size,int *key,int *numusers,
int *corenumusers,int *lasttime, int *status)
{
	status_shared_mem_(shmid,size,key,numusers,corenumusers,lasttime,status);
}
void status_shared_mem_(int shmid,int *size,int *key,int *numusers,
int *corenumusers,int *lasttime, int *status)
{
	struct shmid_ds buf;

	*status = shmctl(shmid, IPC_STAT, &buf);
#ifdef linux
	*key = buf.shm_perm.__key;
#else
	*key = buf.shm_perm.key;
#endif
	*size = buf.shm_segsz;
	*numusers = buf.shm_nattch;
	*corenumusers = buf.shm_nattch;  /* from cnattch in IRIX */
	*lasttime = buf.shm_ctime;
}
void DELETE_SHARED_MEM(int shmid,int *status)
{
	delete_shared_mem_(shmid,status);
}
/* routine to dispose of a shared memory block */
void delete_shared_mem_(int shmid,int *status)
{
	struct shmid_ds buf;

	*status = shmctl(shmid, IPC_RMID, &buf);
}

void ATTACH_SHARED_MEM(int shmid,int *address)
{
	attach_shared_mem_(shmid,address);
}
/* routine to attach to a shared memory segment */
void attach_shared_mem_(int shmid,int *address)
{
	*address = (int)shmat(shmid, 0, 0);
}

void DETACH_SHARED_MEM(int address,int *status)
{
detach_shared_mem_(address,status);
}
/* routine to detach a shared memory segment */
void detach_shared_mem_(int address,int *status)
{
	if (address == 0L) return;
	*status = shmdt((void *)address);
}


/* these routine are used to access the Brainvox shared memory segments
	from external programs */

/* local prototypes */
void	bv_parse_string(long int *data,long int len,char *str);
void	bv_parse_float(long int *data,float *f);
void    parse_shm_data(Brainvox_Globals *vars);
void 	bv_interp_images_(unsigned char *start,unsigned char *end,long int dx,
        	long int dy, long int n, unsigned char *out);

void	bv_parse_string(long int *data,long int len,char *str)
{
	long int	i;

/* copy the data */
	for(i=0;i<len;i++) {
		str[i] = ((char *)(data))[i];
	}
/* find last non space character and null terminate */
	for(i=len-1;i>0;i--) {
		str[i] = '\0';
		if (str[i-1] != ' ') {
			return;
		}
	}
	str[0] = '\0';

	return;
}

void	bv_parse_float(long int *data,float *f)
{

	*f = *((float *)(data));

	return;
}

void	parse_shm_data(Brainvox_Globals *vars)
{
	long int	*addr;
	long int	i,j;

/* fill out Brainvox_Globals structure from the packed shared memory block */

	addr = (long int *)(vars->data_addr);
	
	i = 0;
	vars->shm_ipc = addr[i++];    /*1*/
	vars->shm_data = addr[i++];    /*2*/
	vars->shm_pts = addr[i++];    /*3*/
	vars->shm_images = addr[i++];    /*4*/

	vars->realimages = addr[i++];    /*5*/
	vars->imagesize = addr[i++];    /*6*/
	vars->interpimages = addr[i++];    /*7*/
	vars->numrgns = addr[i++];    /*8*/
	
	for(j=0;j<MAX_RGNS;j++) vars->rgntypes[j] = addr[i++];

	for(j=0;j<MAX_RGNS;j++) {
		bv_parse_string(&(addr[i]),40,vars->rgnnames[j]);
		i = i + (40/4);
	}
	bv_parse_string(&(addr[i]),200,vars->imagename);
	i = i + (200/4);
	bv_parse_string(&(addr[i]),200,vars->patientinfo[0]);
	i = i + (200/4);
	bv_parse_string(&(addr[i]),200,vars->patientinfo[1]);
	i = i + (200/4);
	bv_parse_string(&(addr[i]),200,vars->patientinfo[2]);
	i = i + (200/4);
	bv_parse_float(&(addr[i]),&(vars->interpixel)); i++;
	bv_parse_float(&(addr[i]),&(vars->interslice)); i++;
	bv_parse_float(&(addr[i]),&(vars->squeezefactor)); i++;

	vars->cut_dir = addr[i++];
	vars->volumedepth = addr[i++];
	vars->shm_texture = addr[i++];

	vars->brainvox_options = addr[i++];

	bv_parse_float(&(addr[i]),&(vars->volume_lims[0])); i++;
	bv_parse_float(&(addr[i]),&(vars->volume_lims[1])); i++;

	vars->volume_zswap = addr[i++];
	vars->volume_bswap = addr[i++];

	return;
}

int BV_INDEX_OPEN(int index,Brainvox_Globals *vars)
{
return bv_index_open_(index,vars);
}
int bv_index_open_(int index,Brainvox_Globals *vars)
{
	int	shmid,i,flags;

/* build key number for this brainvox run */
	i = (BASE_SHM_KEY) + (10*index);

/* see if the data shmem segment exists */
	flags = (SHM_R | SHM_W);
	shmid = shmget(i, 100, flags);
	if (shmid == -1) return(1);  /* invalid brainvox index */

/* open it up */
	return(bv_shmem_open_(shmid,vars));
}

int BV_SHMEM_OPEN(int shmem,Brainvox_Globals *vars)
{
return bv_shmem_open_(shmem,vars);
}
int bv_shmem_open_(int shmem,Brainvox_Globals *vars)
{
	long int	i;

	int	size,key,numusers,cnumusers,lasttime,status;

	vars->ipc_addr = 0;
	vars->data_addr = 0;
	vars->images_addr = 0;
	vars->pts_addr = 0;
	vars->texture_addr = 0;
	vars->lut_addr = 0;

	vars->shmem_owner = 0;

	status_shared_mem_(shmem,&size,&key,&numusers,&cnumusers,
		&lasttime,&status);
	vars->bv_key = key; 

/* default to immediate mode interpolation */
	vars->interpolate = (BV_INTERP_LINEAR | BV_INTERP_IMMEDIATE);

/* link up to the main shared memory segment */
	vars->shm_data = shmem;
	attach_shared_mem_(vars->shm_data,(int *)&(vars->data_addr));
	if (vars->data_addr == 0) return(1);
	vars->lut_addr = (void *)((long int)vars->data_addr + LUT_SHM_OFFSET);
/* get the global data out */
	parse_shm_data(vars);

/* attach all shared memory segments */	
	attach_shared_mem_(vars->shm_ipc,(int *)&(vars->ipc_addr));
	attach_shared_mem_(vars->shm_images,(int *)&(vars->images_addr));
	attach_shared_mem_(vars->shm_pts,(int *)&(vars->pts_addr));
	attach_shared_mem_(vars->shm_texture,(int *)&(vars->texture_addr));

	if ((vars->ipc_addr == 0) || (vars->images_addr == 0) ||
		(vars->pts_addr == 0) || (vars->texture_addr == 0)) {
		bv_shmem_close_(vars);
		return(1);
	}

/* connect to the IPC stream */
	i = 0;
	init_ipc_(&i,vars->ipc_addr);

	return(0);
}

int BV_SHMEM_CHECK(Brainvox_Globals *vars)
{
return(bv_shmem_check_(vars));
}
int bv_shmem_check_(Brainvox_Globals *vars)
{
	return(0);
}

int BV_SHMEM_CLOSE(Brainvox_Globals *vars)
{
return(bv_shmem_close_(vars));
}
int bv_shmem_close_(Brainvox_Globals *vars)
{
	int status;

	detach_shared_mem_((long int)vars->ipc_addr,&status);
	detach_shared_mem_((long int)vars->data_addr,&status);
	detach_shared_mem_((long int)vars->images_addr,&status);
	detach_shared_mem_((long int)vars->pts_addr,&status);
	detach_shared_mem_((long int)vars->texture_addr,&status);
	vars->ipc_addr = 0L;
	vars->data_addr = 0L;
	vars->images_addr = 0L;
	vars->pts_addr = 0L;
	vars->texture_addr = 0L;
	vars->lut_addr = 0L;

	return(0);
}

int BV_READIMAGE_IO(Brainvox_Globals *vars,long int slice,long int chan,
	long int cmd,unsigned char *data) 
{
return(bv_realimage_IO_(vars,slice,chan,cmd,data));
}

int bv_realimage_IO_(Brainvox_Globals *vars,long int slice,long int chan,
	long int cmd,unsigned char *data)
{
	unsigned char	*shmdata;
	long int	i,isize;

/* select volume */
	if ((chan < 0) || (chan > 1)) return(1);
	if (chan == 0) {
		shmdata = (unsigned char *)vars->images_addr;
	} else {
		shmdata = (unsigned char *)vars->texture_addr;
	}
/* valid data? */
	if (shmdata == 0L) return(1);
/* valid slice? */
	if ((slice < 0) || (slice > vars->realimages)) return(1);
/* number of bytes to move */
	isize = (vars->imagesize)*(vars->imagesize);
/* get index to first byte */
	i = ((isize)*(slice-1)*(1+(vars->interpimages)));
	switch(cmd) {
		case IPC_READ:
			memcpy(data,&(shmdata[i]),isize);
			break;
		case IPC_WRITE:
			memcpy(&(shmdata[i]),data,isize);
/* possible immediate mode interpolation */
			bv_interpolate_(vars,slice,chan);
			break;
		default:
			return(1);
			break;
	}
/* done OK */
	return(0);
}

void BV_INTERPOLATE(Brainvox_Globals *vars,long int slice,long int chan)
{
bv_interpolate_(vars,slice,chan);
}

void bv_interpolate_(Brainvox_Globals *vars,long int slice,long int chan)
{
	long int 	i,j,dest;
	long int	st,en;
	long int	isize,start,stepsize,iend;
	unsigned char	*data;

/* select volume */
	if ((chan < 0) || (chan > 1)) return;
	if (chan == 0) {
		data = (unsigned char *)vars->images_addr;
	} else {
		data = (unsigned char *)vars->texture_addr;
	}
/* valid data? */
	if (data == 0L) return;

/* if slice number is real, only work if immediate is selected */
/* is slice number is not real, only work if immed is not selected */
	if (vars->interpolate & BV_INTERP_IMMEDIATE) {
		if (slice <= 0) return;
	} else {
		if (slice > 0) return;
	}
/* if slice number is not real, do for all slices */
	if (slice <= 0) {
		en = vars->realimages-1;
		st = 1;
	} else {
		st = slice - 1;
		en = slice + 1;
		if (st < 1) st = 1;
		if (en > vars->realimages-1) en = vars->realimages-1;
	}
/* do the interpolation */
	isize = (vars->imagesize)*(vars->imagesize);
	stepsize = ((vars->interpimages)+1)*isize;  /* n interps + one real */
	start = (st-1)*stepsize;
	for(i=st;i<=en;i++) {
		dest = start + isize;
		iend = start + stepsize;
		if (vars->interpolate & BV_INTERP_LINEAR) {
/* linear interpolate */
			bv_interp_images_(&(data[start]),&(data[iend]),
				vars->imagesize,vars->imagesize,
				vars->interpimages,&(data[dest]));
		} else {
/* replicate */
			for(j=0;j<vars->interpimages;j++) {
				memcpy(&(data[dest]),&(data[start]),isize);
				dest += isize;
			}
		}
		start = start + stepsize;
	}
	return;
}

void 	BV_INTERP_IMAGES(unsigned char *start,unsigned char *end,long int dx,
	long int dy, long int n, unsigned char *out)
{
bv_interp_images_(start,end,dx,dy,n,out);
}

void bv_interp_images_(unsigned char *start,unsigned char *end,long int dx,
        long int dy, long int n, unsigned char *out)
{
        long int i,dxy;
        long int j;
        unsigned char *ptr;
        double wstart,wend,wadd;

/* weighting to add with each slice */
        wadd = 1.0/(n+1.0);
        wend = wadd;
        wstart = 1.0 - wend;
        dxy = dx*dy;
        ptr = out;
        
        for(j=0; j<n; j++) {
/* for each slice */
                for(i=0;i<dxy;i++) {
                        *ptr++ = start[i]*wstart + end[i]*wend;
                };
                wend = wend + wadd;
                wstart = 1.0 - wend;
        };
}

#if 0
void get_next_key_(int *key,int inc)
{
   int i = *key;
   while(1) {
      HANDLE      hmap;
      char        szShareMem[50];

      sprintf(szShareMem, "bvshmem.%d", i);
      hmap = CreateFileMapping((HANDLE)INVALID_HANDLE_VALUE, /* Use swapfile */
                                NULL,
                                PAGE_READWRITE,      /* Memory is Read/Write */
                                0L,                  /* Size Upper 32 Bits */
                                (DWORD)1000,         /* Size Lower 32 bits */
                                szShareMem);
      if (hmap) {
         if (GetLastError() == ERROR_ALREADY_EXISTS) {
            CloseHandle(hmap);
         } else {
            *key = i;
            CloseHandle(hmap);
            return;
         }
      }
      i += inc;
   }
   return;
}

typedef struct {
   int key;
   HANDLE hmap;
   unsigned char *addr;
   int *nusers;
   int create;
} win_shm;

int n_shm = 0;
win_shm l_shm[1000];

void create_shared_mem_(int key, int size, int *shmid)
{
   HANDLE      hmap;
   char        szShareMem[50];

   sprintf(szShareMem, "bvshmem.%d", key);
   hmap = CreateFileMapping((HANDLE)INVALID_HANDLE_VALUE, /* Use swapfile */
                                NULL,
                                PAGE_READWRITE,      /* Memory is Read/Write */
                                0L,                  /* Size Upper 32 Bits */
                                (DWORD)size,         /* Size Lower 32 bits */
                                szShareMem);
   if (hmap) {
      if (GetLastError() == ERROR_ALREADY_EXISTS) {
         *shmid = -1;
         CloseHandle(hmap);
         return;
      }
      l_shm[n_shm].key = key;
      l_shm[n_shm].hmap = hmap;
      l_shm[n_shm].create = 1;
      l_shm[n_shm].nusers = (int *)MapViewOfFileEx(l_shm[n_shm].hmap, 
                                    FILE_MAP_WRITE | FILE_MAP_READ, 
                                    0, 0, 0, NULL); 
      l_shm[n_shm].addr = (unsigned char *)(l_shm[n_shm].nusers + 2);
      l_shm[n_shm].nusers[0] = 0;
      l_shm[n_shm].nusers[1] = size;
      *shmid = key;
      n_shm += 1;
      return;
   }

   *shmid = -1;
   return;
}
void attach_shared_mem_(int shmid,int *address)
{
   int i;
   HANDLE      hmap;
   char        szShareMem[50];

   *address = -1;
   for(i=0;i<n_shm;i++) {
      if (shmid == l_shm[i].key) {
         /* previously mapped shmid */
         if (l_shm[i].addr) {
            *address = (int)l_shm[i].addr;
            l_shm[i].nusers[0] += 1;
            return;
         }
         /* map this one... */
         l_shm[i].nusers = (int *)MapViewOfFileEx(l_shm[i].hmap, 
                                    FILE_MAP_WRITE | FILE_MAP_READ, 
                                    0, 0, 0, NULL); 
         l_shm[i].addr = (unsigned char *)(l_shm[i].nusers + 2);
         l_shm[i].nusers[0] += 1;
         *address = (int)l_shm[i].addr;
         return;
      }
   }

   /* new shmid, we want to map... */
   sprintf(szShareMem, "bvshmem.%d", shmid);
   hmap = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, szShareMem); 
   if (!hmap) return;

   l_shm[n_shm].key = shmid;
   l_shm[n_shm].hmap = hmap;
   l_shm[n_shm].create = 0;
   l_shm[n_shm].nusers = (int *)MapViewOfFileEx(l_shm[n_shm].hmap, 
                                    FILE_MAP_WRITE | FILE_MAP_READ, 
                                    0, 0, 0, NULL); 
   l_shm[n_shm].addr = (unsigned char *)(l_shm[n_shm].nusers + 2);
   l_shm[n_shm].nusers[0] += 1;
   *address = (int)l_shm[n_shm].addr;
   n_shm += 1;

   return;
}
void detach_shared_mem_(int address,int *status)
{
   int i;
   for(i=0;i<n_shm;i++) {
      if (address == (int)l_shm[i].addr) {
         *status = 0;
         l_shm[i].nusers[0] -= 1;
         return;
      }
   }
   *status = -1;
   return;
}
void status_shared_mem_(int shmid,int *size,int *key,int *numusers,
                        int *corenumusers,int *lasttime, int *status)
{
   int i;
   *status = -1;
   for(i=0;i<n_shm;i++) {
      if (l_shm[i].key == shmid) {
         if (l_shm[i].nusers) {
            *key = l_shm[i].key;
            *numusers = l_shm[i].nusers[0];
            *corenumusers = l_shm[i].nusers[0];
            *size = l_shm[i].nusers[1];
            *lasttime = 0;
            *status = 0;
            return;
         }
      }
   }
   return;
}
void delete_shared_mem_(int shmid,int *status)
{
   int i,j;
   *status = -1;
   for(i=0;i<n_shm;i++) {
      if (l_shm[i].key == shmid) {
         /* if we did not create it, we cannot destroy it */
         if (!l_shm[i].create) return;
         UnmapViewOfFile(l_shm[i].nusers);
         CloseHandle(l_shm[i].hmap);
         for(j=i+1;j<n_shm;j++) l_shm[j-1] = l_shm[j];
         n_shm -= 1;
         *status = 0;
         return;
      }
   }
   return;
}
#endif
