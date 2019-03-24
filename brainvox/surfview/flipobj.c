#include <math.h>
#include "stdio.h"
#include "gl.h"
#include "device.h"

#include "flip.h"
#include "hash.h"

flipobj *readflipobj(char *name)
{
	FILE	*inf;
	flipobj	*obj;
	int		i, j;
	int		nlongs;
	int		magic;
	int		*ip;

	inf = fopen(name,"r");
	if(!inf) {
		fprintf(stderr,"readfast: can't open input file %s\n",name);
		exit(1);
	}
	fread(&magic,sizeof(int),1,inf);
	if(magic != FASTMAGIC) {
		fprintf(stderr,"readfast: bad magic in object file\n");
		fclose(inf);
		exit(1);
	}
	obj = (flipobj *)malloc(sizeof(flipobj));
	fread(&obj->npoints,sizeof(int),1,inf);
/*** IGNORE COLORS FIELD ***/
	fread(&magic,sizeof(int),1,inf);

	/*
	 * Insure that the data is quad-word aligned and begins on a page
	 * boundary.  This shields us from the performance loss which occurs 
	 * whenever we try to fetch data which straddles a page boundary  (the OS
	 * has to map in the next virtual page and re-start the DMA transfer).
	 */
	nlongs = 8 * obj->npoints;
	obj->data = (float *) malloc(nlongs*sizeof(int) + 4096);
	obj->data = (float *) (((int)(obj->data)) + 0xfff);
	obj->data = (float *) (((int)(obj->data)) & 0xfffff000);

	for (i = 0, ip = (int *)obj->data;  i < nlongs/4;  i++, ip += 4)
		fread(ip, 3 * sizeof(int), 1, inf);
	fclose(inf);

	return obj;
}

void drawflipobj(flipobj *obj)
{
	register float *p,*end;
	enum DrawType lflag;

	p = obj->data;
	end = p + 8 * obj->npoints;
	lflag = obj->type;

if (SurfView) {
	if (obj->type == POLYGONS) {
		while ( p < end) {
			bgnpolygon();
			c3f(p);
			v3f(p+4);
			c3f(p+8);
			v3f(p+12);
			c3f(p+16);
			v3f(p+20);
			c3f(p+24);
			v3f(p+28);
			endpolygon();
			p += 32;
		}
	} else {
		while ( p < end) {
			bgnclosedline();
			c3f(p);
			v3f(p+4);
			c3f(p+8);
			v3f(p+12);
			c3f(p+16);
			v3f(p+20);
			c3f(p+24);
			v3f(p+28);
			endclosedline();
			p += 32;
		}
	}
} else {
	if (obj->type == POLYGONS) {
		while ( p < end) {
			bgnpolygon();
			n3f(p);
			v3f(p+4);
			n3f(p+8);
			v3f(p+12);
			n3f(p+16);
			v3f(p+20);
			n3f(p+24);
			v3f(p+28);
			endpolygon();
			p += 32;
		}
	} else {
		while ( p < end) {
			bgnclosedline();
			n3f(p);
			v3f(p+4);
			n3f(p+8);
			v3f(p+12);
			n3f(p+16);
			v3f(p+20);
			n3f(p+24);
			v3f(p+28);
			endclosedline();
			p += 32;
		}
	}
}
	return;
}


/*
 * objmaxpoint
 *
 * find the vertex farthest from the origin,
 * so we can set the near and far clipping planes tightly.
 */

#define MAXVERT(v) if ( (len = sqrt(	(*(v))  *  (*(v))  +	  \
					(*(v+1)) * (*(v+1)) +		   \
					(*(v+2)) * (*(v+2)) )) > max)  \
			max = len;

float objmaxpoint(flipobj *obj)
{
	register float *p, *end;
	register int npolys;
	register float len;
	register float max = 0.0;

	p = obj->data;

	end = p + 8 * obj->npoints;
	while ( p < end) {
		MAXVERT(p+4);
		MAXVERT(p+12);
		MAXVERT(p+20);
		MAXVERT(p+28);
		p += 32;
	}

	return max;
}

/*
 *	Use hash functions to find all unique edges
 */
void find_edges(flipobj *obj)
{
	int i, j, v0, v1, n;
	float *p, *end;

	h_init_vertex(obj->npoints * 2);
	h_init_edge(obj->npoints * 4);

/* First run through, to figure out how many there are */
	p = obj->data;
	end = p + 8 * obj->npoints;
	while ( p < end) {
		v0 = h_find_vertex(p+4);
		v1 = h_find_vertex(p+12);
		h_find_edge(v0, v1);
		v0 = h_find_vertex(p+12);
		v1 = h_find_vertex(p+20);
		h_find_edge(v0, v1);
		v0 = h_find_vertex(p+20);
		v1 = h_find_vertex(p+28);
		h_find_edge(v0, v1);
		v0 = h_find_vertex(p+28);
		v1 = h_find_vertex(p+4);
		h_find_edge(v0, v1);
		p += 32;
	}
/* Now malloc enough space */
	obj->nedges = h_get_ne();
	obj->edge = (flipedge *)malloc(sizeof(flipedge)*obj->nedges);

/* And now run through, filling up structure */
	p = obj->data;
	end = p + 8 * obj->npoints;
	while ( p < end) {
		v0 = h_find_vertex(p+4);
		v1 = h_find_vertex(p+12);
		n = h_find_edge(v0, v1);
		obj->edge[n].v0 = p+4;
		obj->edge[n].v1 = p+12;
		
		v0 = h_find_vertex(p+12);
		v1 = h_find_vertex(p+20);
		n = h_find_edge(v0, v1);
		obj->edge[n].v0 = p+12;
		obj->edge[n].v1 = p+20;

		v0 = h_find_vertex(p+20);
		v1 = h_find_vertex(p+28);
		n = h_find_edge(v0, v1);
		obj->edge[n].v0 = p+20;
		obj->edge[n].v1 = p+28;

		v0 = h_find_vertex(p+28);
		v1 = h_find_vertex(p+4);
		n = h_find_edge(v0, v1);
		obj->edge[n].v0 = p+28;
		obj->edge[n].v1 = p+4;

		p += 32;
	}
	h_destroy_vertex();
	h_destroy_edge();
}
