
#ifndef IDL_TOOLS_H
#define IDL_TOOLS_H

/* message numbers... */
#define M_TOOLS_ERROR		0
#define M_TOOLS_BADTYPE		-1
#define M_TOOLS_BADAUX		-2
#define M_TOOLS_BADWEIGHTS	-3
#define M_TOOLS_BADSND		-4
#define	M_TOOLS_BADDIBFORMAT	-5
#define	M_TOOLS_BADDIMS		-6
#define	M_TOOLS_BADRANGE	-7
#define M_TOOLS_BADKWDS		-8
#define M_TOOLS_BADPASTE	-9
#define	M_TOOLS_BADPROCNAME	-10
#define	M_TOOLS_BADSIG		-11
#define	M_TOOLS_EXCLKEY		-12

/* Handy macro */
#define ARRLEN(arr) (sizeof(arr)/sizeof(arr[0]))

extern IDL_MSG_BLOCK msg_block;

/* variable encapsulation and manipulation */
/* 
 * Define the token value which leads up the var header.  Its form
 * on a read determines if byteswapping is necessary on the
 * read data.  ('IDLV')
 */
#define	TOKEN		0x49444C56
#define SWAPTOKEN	0x564C4449

/* IDL variable packet header */
typedef struct {
	IDL_LONG	token;
	IDL_LONG	type;
	IDL_LONG	ndims;
	IDL_LONG	len;
	IDL_LONG	nelts;
	IDL_LONG	dims[IDL_MAX_ARRAY_DIM];
} i_var;

extern void byteswap(void *buffer,int len,int swapsize);

/* idl_sock.c */
extern void sock_exit_handler(void);
extern int sock_startup(void);

/* idl_snd.c */
extern void snd_exit_handler(void);
extern int snd_startup(void);

/* idl_gzip.c */
extern void gzip_exit_handler(void);
extern int gzip_startup(void);

/* idl_mesh.c */
extern void mesh_exit_handler(void);
extern int mesh_startup(void);

/* idl_twain.c */
extern void twain_exit_handler(void);
extern int twain_startup(void);

/* idl_sharr.c */
extern void sharr_exit_handler(void);
extern int sharr_startup(void);

/* idl_defproc.c */
extern void extproc_exit_handler(void);
extern int extproc_startup(void);

/* idl_vfw.c */
extern void vfw_exit_handler(void);
extern int vfw_startup(void);

#endif

