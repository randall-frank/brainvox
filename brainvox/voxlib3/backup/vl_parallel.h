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
	long int *st;
	long int *en;
	long int *le;
	long int *ri;
	long int *step;
	long int *mult;
	long int *order;
	long int flag;

	long int rect[4];  /* xmin,ymin,xmax,ymax */
	long int *abort;
	long int ret;
	long int inter;
	long int mynum;  /* thread number */
	long int cbnum;	 /* thread to make callbacks on (parent = -1) */

	/* externallly initialized buffers */
	unsigned long int	*image;
	long int		*pbuffer;
	short			*zbuf;

} par_quant;

typedef void (*p_sproc)(void *);

/* parallel versions of routines */
void vl_m_pirender(par_quant *par);
void vl_m_pirend16(par_quant *par);
void vl_m_init(par_quant *par);
void vl_m_done(par_quant *par);

void vl_calc_even_rects(vset *set,long int n,long int *lines,long int *rect);

/* thread primitives (encapsulated) */
long int vl_init_threads(long int ncpus);
long int vl_start_thread(p_sproc proc,void *data,long int n,long int ncpus);
void vl_exit_thread(long int n);
long int vl_gather_threads(vset *set,long int ret,long int *kill,long int ncpus
	,void *data);

#ifdef __cplusplus
}
#endif

#endif
