/* header for parallel version of voxlib */

/* it works like this:
	(Multi-CPU? getenv...)
	parent -> build chunks to be rendered.
	sproc off chunks.
	wait for children to die (finish) while
	calling the callback proc. If the callback proc is an ABORT, kill
	the children.
*/

#ifndef __VL_PARALLEL__
#define __VL_PARALLEL__

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_CPUS 36

/* define a "chunk" to be rendered via sproc to vl_m_pirender */
typedef struct {
	vset *set;
	VLINT32 *st;
	VLINT32 *en;
	VLINT32 *le;
	VLINT32 *ri;
	VLINT32 *step;
	VLINT32 *mult;
	VLINT32 *order;
	VLINT32 flag;

	VLINT32 rect[4];  /* xmin,ymin,xmax,ymax */
	VLINT32 *abort;
	VLINT32 ret;
	VLINT32 inter;
	VLINT32 mynum;  /* thread number */
	VLINT32 cbnum;	 /* thread to make callbacks on (parent = -1) */
#if (defined(WIN32) || defined(__WIN32__))
	VLVOID *hThread;  /* the thread handle for WaitThread */
#endif


	/* externallly initialized buffers */
	VLUINT32	*image;
	VLINT32		*pbuffer;
	VLINT16			*zbuf;

} par_quant;

typedef void (*p_sproc)(void *);

/* parallel versions of routines */
void vl_m_pirender(par_quant *par);
void vl_m_pirend16(par_quant *par);
void vl_m_pirend24(par_quant *par);
void vl_m_init(par_quant *par);
void vl_m_done(par_quant *par);
int vl_3dtexrender(par_quant *par);

#define VL_CANNOT_3DTEX 100

void vl_calc_even_rects(vset *set,VLINT32 n,VLINT32 *lines,VLINT32 *rect);

/* thread primitives (encapsulated) */
VLINT32 vl_init_threads(VLINT32 ncpus);
VLINT32 vl_start_thread(p_sproc proc, VLVOID *data,VLINT32 n,VLINT32 ncpus);
void vl_exit_thread(VLINT32 n);
VLINT32 vl_gather_threads(vset *set,VLINT32 ret,VLINT32 *kill,VLINT32 ncpus
	,void *data);

#ifdef __cplusplus
}
#endif

#endif
