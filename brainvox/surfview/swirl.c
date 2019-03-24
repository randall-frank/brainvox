/*
 *	A couple of routines that make swirl work
 */
#include "flip.h"

#define SECTIONS 19

#define SWIRLINC 40

#define PolyOrLine(fl)	if (fl == POLYGONS) { \
				bgnpolygon(); \
			} else { \
				bgnclosedline(); \
			}

#define EndPolyOrLine(fl) if (fl == POLYGONS) { \
				endpolygon(); \
			} else { \
				endclosedline(); \
			}
/*
 *	How does this work?  It draws SECTIONS sections of polygons,
 * rotating between drawing them, the magnitude of the rotation
 * defined by how big the 'swirl' paramater is.
 */
draw_swirl(obj)
flipobj *obj;
{
	register int i;
	register float *p, *end;
	int ixr, iyr;

	if (obj->swirl > 0)
	{
		srand(0);
		for (i = 0 ; i < SECTIONS ; i++)
		{
			ixr = rand() % 3600;
			iyr = rand() % 3600;

			pushmatrix();
			rotate(ixr, 'x');
			rotate(iyr, 'y');
			rotate(obj->swirl, 'x');
			rotate(obj->swirl, 'y');
			rotate(-iyr, 'y');
			rotate(-ixr, 'x');
			p = obj->swirldata + (32*i) ;
			end = obj->swirldata + 8*obj->npoints ;

			while (p < end)
			{
			    PolyOrLine(obj->type);
			    n3f(p);
			    v3f(p+4);
			    n3f(p+8);
			    v3f(p+12);
			    n3f(p+16);
			    v3f(p+20);
			    n3f(p+24);
			    v3f(p+28);
			    EndPolyOrLine(obj->type);
			    p += (32*SECTIONS) ;
			}
			popmatrix();
		}
	}
	else
	{
		drawflipobj(obj);
	}
	obj->swirl += SWIRLINC;
	if (obj->swirl > 3600)
	{
		obj->swirl = -60 * SWIRLINC + 1 ;
/* Note:  add 1 above so that obj->swirl doesn't hit zero.  Start at
 * negative so there is a time when it isn't swirling.
 */
	}
}

swirl_randomize(obj)
flipobj *obj;
{
    int		i, j;
    int		*ip, *ip1;
    int		temp[32];
    int		*t1, *t2;
    int		off1, off2;

    /*
     *	Randomize the polygons for swirl.
     */

	/* First, copy data over... */
	obj->swirldata = (float *)malloc(sizeof(float) * 8 * obj->npoints);
	memcpy(obj->swirldata, obj->data, sizeof(float) * 8 * obj->npoints);

    for (i = 0; i < obj->npoints / 8; i++)
	{
		off1 =(rand() % (obj->npoints / 4)) * 32;
		off2 =(rand() % (obj->npoints / 4)) * 32;
		ip = ip1 = (int *)obj->swirldata;
		ip += off1;
		ip1 += off2;
		if (ip != ip1)
		{
		    for (t1 = temp, t2 = ip, j=0; j < 32; j++)
			*t1++ = *t2++;
		    for (t1 = ip, t2 = ip1, j=0; j < 32; j++)
			*t1++ = *t2++;
		    for (t1 = ip1, t2 = temp, j=0; j < 32; j++)
			*t1++ = *t2++;
		}
    }
}
