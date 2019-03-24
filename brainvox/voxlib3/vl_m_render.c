#include "voxel.h"
#include "vl_util.h"
#include <stdio.h>
#include <stdlib.h>

#include "vl_parallel.h"

void vl_m_render_(VLINT32 npin,VLINT32 flags, VLINT32 interrpt ,
	VLINT32 *ret, vset *set,VLINT32 *rect)
{
	VLINT32 order[3],step[3];
	VLINT32	mult[3];
	VLINT32 st[3],en[3],ri[3],le[3],sp[3];
	VLINT32 vflag,nprocs;
	VLINT32 killflag;
	VLINT32 nlines;
	VLINT32	i;

	VLINT32 lines[MAX_CPUS];
	par_quant pars[MAX_CPUS];

/* get the number of procs */
	nprocs = npin;
	if (nprocs <= 0) nprocs = 1;
	if (nprocs > MAX_CPUS) nprocs = MAX_CPUS;

/* get the axis starts/ends and steps */
	for(i=0;i<3;i++) {
		le[i] = set->start[i];
		ri[i] = set->end[i];
		st[i] = set->start[i];
		en[i] = set->end[i];
	};
	sp[0] = ((set->speed >> 16) & 0xff);
	sp[1] = ((set->speed >> 8) & 0xff);
	sp[2] = ((set->speed) & 0xff);
	(void) axisorder(order, step, set);
/* fudge speed so slowest axis has the skip */
	if ((set->speed & 0x00ffff00) == 0L) {
		sp[order[2]] = 0;
		sp[order[1]] = 0;
		sp[order[0]] = set->speed;
	}
/* apply the skip factor and swap the start.stop */
	for(i=0;i<3;i++) {
		if (step[i] < 0) {
			SWAP(st[i],en[i]);
		};
		if (sp[i] == 0) sp[i] = 1;
		step[i] = step[i] * (sp[i]);
	};
/* get the planar multiplies */
	mult[0] = 1;
	mult[1] = set->d[0];
	mult[2] = (set->d[0])*(set->d[1]);

/* assume no errors */
	*ret = VL_NOERR;
	if ((set->data) == 0) {
		vl_puts("This vset has no data assigned to it!.");
		*ret = VL_MEMERR;
	 	return;
	};
	if (flags & VL_RENDER16BIT) {
		if ((set->aux_data[0] == 0) || (set->aux_data[1] == 0)) {
	vl_puts("Two volumes must be specified for 16bit rendering.");
			*ret = VL_MEMERR;
	 		return;
		}
	};
	if ((flags & VL_RENDER24BIT) || (flags & VL_RENDERYIQ)) {
		if ((set->aux_data[0] == 0) || (set->aux_data[1] == 0)
			|| (set->aux_data[2]  == 0)) {
	vl_puts("Three volumes must be specified for 24bit rendering.");
			*ret = VL_MEMERR;
	 		return;
		}
	};
	if ((flags & VL_BACKGROUND) == VL_BACKGROUND) *ret = VL_BACKGROUND;

/* set up the vflag rendering type */
	vflag = (flags & 0xfffffffe);
	if (set->plane != 0) vflag = (vflag | VL_RENDERPLANE);

/*	vl_puts("Using parallel rendering...\n"); */

/* need to 1) get and clear the image buffer */
/*         2) do the same for the zbuffer */
/*         3) handle the cutting plane */

	killflag = 0;
	nlines = set->imagey/nprocs;
        pars[0].set = set;
        pars[0].st = st;
        pars[0].en = en;
        pars[0].le = le;
        pars[0].ri = ri;
        pars[0].step = step;
        pars[0].mult = mult;
        pars[0].order = order;
	pars[0].flag = vflag & (~VL_RENDER3DTEX);
        pars[0].inter = interrpt;
        pars[0].ret = *ret;
	pars[0].abort = &killflag;
        pars[0].mynum = 0;
        pars[0].cbnum = -1;
	pars[0].rect[0] = 0;
	pars[0].rect[1] = 0;
	pars[0].rect[2] = set->imagex;
	pars[0].rect[3] = (set->imagey-(nlines*(nprocs-1)))-1;

/* init the buffers */
	vl_m_init(&(pars[0]));
	if (pars[0].ret != *ret) {
		*ret = pars[0].ret;
		return;
	}
/* build the procs */
	vl_calc_even_rects(set,nprocs,lines,rect);

	if (nprocs == 1) {
/* single CPU */
		int retry = VL_CANNOT_3DTEX;
        	pars[0].cbnum = 0;
		pars[0].rect[3] = set->imagey;
		/* can try HW volume rendering */
		if (flags & VL_RENDER3DTEX) {
			retry = vl_3dtexrender(&(pars[0]));
                }
		/* if HW fails, or not asked for */
                if (retry == VL_CANNOT_3DTEX) {
		    if (flags & VL_RENDER16BIT) {
			vl_m_pirend16(&(pars[0]));
		    } else if ((flags & VL_RENDER24BIT)||(flags & VL_RENDERYIQ)){
			vl_m_pirend24(&(pars[0]));
		    } else {
			vl_m_pirender(&(pars[0]));
		    }
                }
	} else {
/* startup threads */
		vl_init_threads(nprocs);

#ifdef DEBUG
	printf("Line demarking for CPUs: (%ld lines)\n",set->imagey);
	for(i=0;i<nprocs;i++) printf("CPU %ld : linestop %ld\n",i,lines[i]);
#endif

        	for(i=0;i<nprocs; i++) {
                	if (i != 0) {
                        	pars[i] = pars[i-1]; /* copy from previous */
                        	pars[i].mynum = i;
/* bump rectangle along */
/* There seems to be a bug in clipping 
   such that it is not inclusive of y1 */
				pars[i].rect[1] = pars[i].rect[3]-1;  /* use +1 if not contig */
                	}
/* use the lines arrary !! */
			pars[i].rect[3] = (lines[i]-1);
/* RJF:fixed major bug (was pars[0]!!) race condition as a thread's boundary
	was changed after the rendering had started!!! */
			if (i == nprocs-1) pars[i].rect[3] = set->imagey;

#ifdef DEBUG
	fprintf(stderr,"Sprocing %ld for %ld %ld %ld %ld\n",
		i,pars[i].rect[0],pars[i].rect[1],pars[i].rect[2],
		pars[i].rect[3]);
#endif
			if (flags & VL_RENDER16BIT) {
				vl_start_thread((p_sproc)vl_m_pirend16,
					&(pars[i]),i,nprocs);
			} else 
			if ((flags & VL_RENDER24BIT) ||
			    (flags & VL_RENDERYIQ)){
				vl_start_thread((p_sproc)vl_m_pirend24,
					&(pars[i]),i,nprocs);
			} else {
				vl_start_thread((p_sproc)vl_m_pirender,
					&(pars[i]),i,nprocs);
			}
        	}

		vl_gather_threads(set,*ret,&killflag,nprocs,pars);
	}

/* free up the extra buffers (if any) */
	vl_m_done(&(pars[0]));

	*ret = pars[0].ret;

/* free up any malloced buffers */
#if 0
        for(i=0;i<set->imagex;i++)   {
           printf("%d ",set->zbuffer[i+(set->imagey/2)*set->imagex]);
/*
           int *image = (int *)set->image;
           printf("%x %d ",image[i+(set->imagey/2)*set->imagex],
                   set->zbuffer[i+(set->imagey/2)*set->imagex]);
*/
        }
        printf("\n");
#endif
	return;
}
