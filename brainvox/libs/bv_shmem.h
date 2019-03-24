
/* brainvox defines */
/*
C       shared mem allocation sizes
*/
#define BASE_SHM_KEY (0x41366)
#define LUT_SHM_SIZE (3*256*4 + 2*256*256)
#define LUT_SHM_OFFSET 4596
#define DATA_SHM_SIZE  (LUT_SHM_OFFSET+LUT_SHM_SIZE)
#define PTS_SHM_SIZE  (4097*4*4)
/* this one is now the maximum message size, NOT the size of the shm block */
#define IPC_SHM_SIZE  2048
/*
C       IPC declares
*/
#define IPC_READ   0
#define IPC_WRITE  1

#define IPC_QUIT  -1
#define IPC_OK     0
#define IPC_POINT  1
#define IPC_CLEAR  2
#define IPC_VLUTS  3
#define IPC_ILUTS  4
#define IPC_PNAMES  5
#define IPC_MPIOSAVE  6
#define IPC_MPIOLOAD  7
/*
C       Added for lighting model dialog
*/
#define IPC_L_INTRINSIC 8
#define IPC_L_FILTER  9
#define IPC_L_POSTLIGHT 10
/*
C       Added for new palette editor model
*/
#define IPC_V_VLUT 11
#define IPC_V_ILUT 12
#define IPC_T_ILUT 13
#define IPC_V_TLUT 14
/*
C       Added for the filer interface
*/
#define IPC_F_RETURN    15

/*
C       Added for the histogram passing interface
*/
#define IPC_I_HISTO     16

#define IPC_H_MRIVOL    0
#define IPC_H_MRICUT    1
#define IPC_H_TEXTUREVOL        2
#define IPC_H_TEXTURECUT        3

/*
C       Added for texture functions
*/
#define IPC_TEXFUNCS    17
#define IPC_TEXVOLUME   18
#define IPC_TEXCLIP     19
#define IPC_TEXFILL     20
#define IPC_TEXUTIL     21

/* clip/fill options */
#define IPC_TEX_CL_PAINT        0
#define IPC_TEX_CL_TAG  1
#define IPC_TEX_CL_UNTAG        2

/* util operations */
#define IPC_TEX_UTIL_DIV        0
#define IPC_TEX_UTIL_MULT       0
#define IPC_TEX_UTIL_SWAP       0

/* DEBUGING options */
#define OPTS_NONE 0
#define OPTS_TESTING 1
#define OPTS_ADVANCED 2
#define OPTS_RJF 4

#define MAX_RGNS 60
#define MAX_IMAGES 256

/* The external interface specific defines */
#define BV_INTERP_LINEAR 1
#define BV_INTERP_IMMEDIATE 2

/* brainvox shared globals structure */
typedef struct {
        int shm_ipc;
        int shm_data;
        int shm_pts;
        int shm_images;
        int shm_texture;

        int realimages;
        int imagesize;
        int interpimages;
        int numrgns;
        int rgntypes[MAX_RGNS];
        char rgnnames[MAX_RGNS][40];

        char imagename[200];
        char patientinfo[3][200];

        float   interslice;
        float   interpixel;
        float   squeezefactor;

        int     cut_dir;  /* 0-cutting other=subvolume index number */

        int     volumedepth;
        int     volume_zswap;
        int     volume_bswap;
        float   volume_lims[2];

        int     brainvox_options;
/*
C       memory address of mount points (0 if segment is not attached)
*/
        void *ipc_addr;
        void *data_addr;
        void *images_addr;
        void *pts_addr;
        void *texture_addr;
        void *lut_addr;

        int shmem_owner;  /* 0=client 1=owner */
        int bv_key;

/* External interface specific stuff */
	long int	interpolate;

} Brainvox_Globals;

/* IPC routines */
void init_ipc_(long int *first,void *ipc_addr);
void send_ipc_(long int *size,long int *msg);
void check_ipc_(long int *size,long int *msg);

/* shared memory access wrappers */
void get_next_key_(int *key,int inc);
void create_shared_mem_(int key, int size, int *shmid);
void status_shared_mem_(int shmid,int *size,int *key,int *numusers,
	int *corenumusers,int *lasttime, int *status);
void delete_shared_mem_(int shmid,int *status);
void attach_shared_mem_(int shmid,int *address);
void detach_shared_mem_(int address,int *status);

/* general brainvox access routines */
int bv_index_open_(int index,Brainvox_Globals *vars);
int bv_shmem_open_(int shmem,Brainvox_Globals *vars);
int bv_shmem_close_(Brainvox_Globals *vars);
int bv_realimage_IO_(Brainvox_Globals *vars,long int slice,long int chan,
        long int cmd,unsigned char *data);
void bv_interpolate_(Brainvox_Globals *vars,long int slice,long int chan);
