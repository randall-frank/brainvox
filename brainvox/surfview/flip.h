#include <stdio.h>
#include <gl.h>
#include <device.h>
#include "light.h"

#ifndef HZ
#define HZ 100
#endif

#define MODELDIR "/usr/demos/data/models/"

/* drawing data types */

enum DrawType
{
	POLYGONS,
	LINES,
	SUBSMOOTHLINES, 
	NUM_DrawTypes
} ;

#define FASTMAGIC	0x5423

typedef struct {
	float *v0, *v1;
} flipedge;

typedef struct {
    int npoints;
	int display ;	/* Draw me at all ? */
	enum DrawType type ;	/* Polygons? Lines? What? */
	int select ;	/* Am I selected? */
    int material;	/* Material index */
    int ablend;	/* Alpha-blend me? */
	int swirl;	/* Go crazy with swirling? */
	char *fname ;	/* Filename for menus */
    float *data;	/* Raw polygon data */
	float *swirldata;	/* Randomized data for swirling */
	flipedge *edge;	/* Processed edges */
	int nedges;
	float er[4];	/* Cumulative rotation */
	float espin[4];	/* Incremental rotation to spin */
	float trans[3];	/* Translation */
} flipobj;

flipobj *readflipobj();

extern int	SurfView;

