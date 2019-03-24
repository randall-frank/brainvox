/* --------------------------------------------------------------------------
|| CHRTBL - Stroke character table on 6x9 matrix. 3-dimensional array. Each
|| element consists of V, dX, dY where V = 3 for invisible, V=2 for
|| visible, and V=0 for end of list. Array accessing is
|| chrtbl[chr,strokenumber,which] where which=0 gets V, =1 gets dX, and
|| =2 gets dY
------------------------------------------------------------------------- */
#include "voxel.h"
#include "vl_util.h"
#include "vl_polygon.h"
#include "vl_putil.h"
#include <stdio.h>
#include <string.h>
#include <math.h>


extern VLFLOAT64 cen_tab[18];


void vl_pick_string(VLCHAR *s,VLINT32 type,vl_text_attrib *a,vl_int_vertex *pnt,vset *set)
{
	VLINT32	dx,dy,x,y,flag,color,i;
	void		*data;
	VLUCHAR	*d;
	VLINT16		zval;

	if (vl_polygon_globals.text_proc == 0L) {
		draw_string_vector(s,type,a,pnt,set,vl_p_rendpickline);
	} else {
/* get the pixmap */
		data = 0;
		(*vl_polygon_globals.text_proc)(s,a,&dx,&dy,&data);
		if (data == 0L) return;

/* setup for drawing */
		x = pnt->screen[0];
		y = pnt->screen[1];

/* need to shift to proper corner */
    		x = x + (cen_tab[2*(a->center)]*(dx));
    		y = y + (cen_tab[(2*(a->center))+1]*(dy));

/* get zvalue and flags */
		zval = pnt->screen[2];
		flag = (type  & VL_P_OPTSMASK);

		i = pnt->color[0]*255;
		color = i;
		i = pnt->color[1]*255;
		color |= (i << 8);
		i = pnt->color[2]*255;
		color |= (i << 16);

/* alpha should come from color[3] !!!! */
		d = data;
		for(i=0;i<(dx*dy);i++) {
			if (d[i] != 0) d[i] = pnt->color[3]*255;
		}
/* render it */
		vl_pick_pixmap_(data,&zval,dx,dy,x,y,flag,color,set);
/* allow for cleanup */
		(*vl_polygon_globals.text_proc)(0L,a,&dx,&dy,&data);
	}
	return;
}


