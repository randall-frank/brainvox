#include "voxel.h"
#include "vl_util.h"
#include <stdio.h>
#include <math.h>
#include <limits.h>

void vl_makeplane(long int *pbuf, long int dx, long int dy, vset *set)
{
	double q,n[3],p[3],pt[3];
	long int i,x,y,xp,yp;
	long int ptr;

	for(i=0; i<3; i++) {
		p[i] = (set->planept)[i];
		n[i] = (set->planevec)[i];
	};
/* find q such that the points on the plane match the equation
	(nx*x) + (ny*y) + (nz*z) + q = 0
*/
	q = -(n[0]*p[0] + n[1]*p[1] + n[2]*p[2]);

/* for each point in the buffer */
	ptr = 0;
/* the order is such that the loop works its way through the RAM buffer
	sequentially */
	for(y=0; y<(dy); y++) {
		yp = y - (dy/2);
		xp = -(dx/2);
		for(x=0; x<(dx); x++) {
/* check the special case where the plane is perpendicular to the screen */
			if (fabs(n[2]) < 0.0001) {
				if ((n[0]*xp + n[1]*yp + q) > 0) {
					pbuf[ptr++] = INT_MAX;
				} else {
					pbuf[ptr++] = INT_MIN;
				};
			} else {
				pbuf[ptr++] = ((q + n[0]*xp + n[1]*yp)/n[2]) * 8;
			};
			xp++;
		};
	};

}
