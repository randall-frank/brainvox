/* Common chunks for all rendering routines */
/* Included multiple times because of the case structure and caching */

#define REND_START \
for(;(loc_v < loc_ri); loc_v+=1) {  \
/* get the voxel value */  \
	i = *loc_dptr; 

#define ORIG_REND_START \
for(loc_v=st[a]; ((loc_v <= loc_ri) && (loc_v >= loc_le));  \
	loc_v=loc_v+loc_st) {  \
/* get the voxel value */  \
	i = *loc_dptr; 

#define REND_END \
	loc_dptr = loc_dptr + lastinc;  \
	vid[0] += vidd[0]; \
        vid[1] += vidd[1]; \
        vid[2] += vidd[2]; \
}

#define REND_START16 \
for(;(loc_v < loc_ri); loc_v+=1) {  \
/* get the voxel value */  \
	i = *loc_dptr;  \
	i2 = *loc_d2ptr; 

#define REND_END16 \
	loc_dptr = loc_dptr + lastinc;  \
	loc_d2ptr = loc_d2ptr + lastinc;  \
	vid[0] += vidd[0]; \
        vid[1] += vidd[1]; \
        vid[2] += vidd[2]; \
}

#define REND_START24 \
for(;(loc_v < loc_ri); loc_v+=1) {  \
/* get the voxel value */  \
	i = *loc_dptr;  \
	i2 = *loc_d2ptr; \
	i3 = *loc_d3ptr; 

#define REND_END24 \
	loc_dptr = loc_dptr + lastinc;  \
	loc_d2ptr = loc_d2ptr + lastinc;  \
	loc_d3ptr = loc_d3ptr + lastinc;  \
	vid[0] += vidd[0]; \
        vid[1] += vidd[1]; \
        vid[2] += vidd[2]; \
}

/* -------------------------------------------------------------- */

#define NORMAL \
/* if it is visible plot it */  \
	if ((loc_rop = ropacity[i]) < 255) {  \
/* get the X,Y point */  \
        	vx[0] = vid[0] >> 16;  \
        	vx[1] = vid[1] >> 16;  \
        	vx[2] = vid[2] >> 13;  \
/* run through the buffer */  \
		COLORIN(vx,loc_rop,RBrlut[i],Grlut[i],image,zbuf); \
        }; 

/* -------------------------------------------------------------- */

#define PLANE \
        loc_rop = ropacity[i];  \
        loc_lop = lopacity[i];  \
/* if it is visible plot it */  \
        if ((loc_lop < 255) || (loc_rop < 255)) {  \
/* get the X,Y point */  \
                vx[0] = vid[0] >> 16;  \
                vx[1] = vid[1] >> 16;  \
                vx[2] = vid[2] >> 13;  \
/* if the Z point is <= the plane buffer use rlut */  \
                if (vx[2] <= pbuffer[(vx[0] + (vx[1]*imagex))])  \
                {  \
                	if (loc_rop < 255)  \
/* run through the buffer  w/rlut */  \
                	COLORIN(vx,loc_rop,RBrlut[i],Grlut[i],image,zbuf);  \
        	} else {  \
                	if (loc_lop < 255)  \
/* run through the buffer  w/llut */  \
                	COLORIN(vx,loc_lop,RBllut[i],Gllut[i],image,zbuf);  \
                };  \
        }; 

/* -------------------------------------------------------------- */

#define OVER \
/* if it is visible plot it */  \
	if ((loc_rop = ropacity[i]) < 255) {  \
/* get the X,Y point */  \
        	vx[0] = vid[0] >> 16;  \
        	vx[1] = vid[1] >> 16;  \
        	vx[2] = vid[2] >> 13;  \
/* run through the buffer */  \
		ZCOLORIN(vx,loc_rop,RBrlut[i],Grlut[i],image,zbuf); \
        }; 

/* -------------------------------------------------------------- */

#define LIGHT \
/* if it is visible plot it */  \
        if ((loc_rop = ropacity[i]) < 255) {  \
/* get the X,Y point */  \
                vx[0] = vid[0] >> 16;  \
                vx[1] = vid[1] >> 16;  \
                vx[2] = vid[2] >> 13;  \
/* LIGHT the point */  \
                wx = *(loc_dptr + mult[0]);  \
                wx -= *(loc_dptr - mult[0]);  \
                wy = *(loc_dptr + mult[1]);  \
                wy -= *(loc_dptr - mult[1]);  \
                wz = *(loc_dptr + mult[2]);  \
                wz -= *(loc_dptr - mult[2]);  \
                wi = RBrlut[i];  \
                wj = Grlut[i];  \
                vl_get_inten(wx,wy,wz,&wi,&wj);  \
                wi = wi * (255-loc_rop);  \
                wj = wj * (255-loc_rop);  \
/* run through the buffer */  \
                COLORIN(vx,loc_rop,wi,wj,image,zbuf);  \
        }; 

/* -------------------------------------------------------------- */

#define PLANEOVER \
        loc_rop = ropacity[i];  \
        loc_lop = lopacity[i];  \
/* if it is visible plot it */  \
        if ((loc_lop < 255) || (loc_rop < 255)) {  \
/* get the X,Y point */  \
                vx[0] = vid[0] >> 16;  \
                vx[1] = vid[1] >> 16;  \
                vx[2] = vid[2] >> 13;  \
/* if the Z point is <= the plane buffer use rlut */  \
                if (vx[2] <= pbuffer[(vx[0] + (vx[1]*imagex))])  \
                {  \
                	if (loc_rop < 255)  \
/* run through the buffer  w/rlut */  \
                	ZCOLORIN(vx,loc_rop,RBrlut[i],Grlut[i],image,zbuf);  \
        	} else {  \
                	if (loc_lop < 255)  \
/* run through the buffer  w/llut */  \
                	ZCOLORIN(vx,loc_lop,RBllut[i],Gllut[i],image,zbuf);  \
                };  \
        }; 

/* -------------------------------------------------------------- */

#define PLANELIGHT \
        loc_rop = ropacity[i];  \
        loc_lop = lopacity[i];  \
/* if it is visible plot it */  \
        if ((loc_lop < 255) || (loc_rop < 255)) {  \
/* get the X,Y point */  \
                vx[0] = vid[0] >> 16;  \
                vx[1] = vid[1] >> 16;  \
                vx[2] = vid[2] >> 13;  \
/* if the Z point is <= the plane buffer use rlut */  \
                if (vx[2] <= pbuffer[(vx[0] + (vx[1]*imagex))])  \
                {  \
                	if (loc_rop < 255) {  \
/* LIGHT the point */  \
                		wx = *(loc_dptr + mult[0]);  \
                		wx -= *(loc_dptr - mult[0]);  \
                		wy = *(loc_dptr + mult[1]);  \
                		wy -= *(loc_dptr - mult[1]);  \
                		wz = *(loc_dptr + mult[2]);  \
                		wz -= *(loc_dptr - mult[2]);  \
                		wi = RBrlut[i];  \
                		wj = Grlut[i];  \
               		 	vl_get_inten(wx,wy,wz,&wi,&wj);  \
                		wi = wi * (255-loc_rop);  \
                		wj = wj * (255-loc_rop);  \
/* run through the buffer  w/rlut */  \
                		COLORIN(vx,loc_rop,wi,wj,image,zbuf);  \
			}  \
        	} else {  \
                	if (loc_lop < 255) {  \
                		wx = *(loc_dptr + mult[0]);  \
                		wx -= *(loc_dptr - mult[0]);  \
                		wy = *(loc_dptr + mult[1]);  \
                		wy -= *(loc_dptr - mult[1]);  \
                		wz = *(loc_dptr + mult[2]);  \
                		wz -= *(loc_dptr - mult[2]);  \
                		wi = RBllut[i];  \
                		wj = Gllut[i];  \
               		 	vl_get_inten(wx,wy,wz,&wi,&wj);  \
                		wi = wi * (255-loc_lop);  \
                		wj = wj * (255-loc_lop);  \
/* run through the buffer  w/llut */  \
                		COLORIN(vx,loc_lop,wi,wj,image,zbuf);  \
			}  \
                };  \
        }; 

/* -------------------------------------------------------------- */

#define PLANEOVERLIGHT \
        loc_rop = ropacity[i];  \
        loc_lop = lopacity[i];  \
/* if it is visible plot it */  \
        if ((loc_lop < 255) || (loc_rop < 255)) {  \
/* get the X,Y point */  \
                vx[0] = vid[0] >> 16;  \
                vx[1] = vid[1] >> 16;  \
                vx[2] = vid[2] >> 13;  \
/* if the Z point is <= the plane buffer use rlut */  \
                if (vx[2] <= pbuffer[(vx[0] + (vx[1]*imagex))])  \
                {  \
                	if (loc_rop < 255) {  \
/* LIGHT the point */  \
                		wx = *(loc_dptr + mult[0]);  \
                		wx -= *(loc_dptr - mult[0]);  \
                		wy = *(loc_dptr + mult[1]);  \
                		wy -= *(loc_dptr - mult[1]);  \
                		wz = *(loc_dptr + mult[2]);  \
                		wz -= *(loc_dptr - mult[2]);  \
                		wi = RBrlut[i];  \
                		wj = Grlut[i];  \
               		 	vl_get_inten(wx,wy,wz,&wi,&wj);  \
                		wi = wi * (255-loc_rop);  \
                		wj = wj * (255-loc_rop);  \
/* run through the buffer  w/rlut */  \
                		ZCOLORIN(vx,loc_rop,wi,wj,image,zbuf);  \
			}  \
        	} else {  \
                	if (loc_lop < 255) {  \
                		wx = *(loc_dptr + mult[0]);  \
                		wx -= *(loc_dptr - mult[0]);  \
                		wy = *(loc_dptr + mult[1]);  \
                		wy -= *(loc_dptr - mult[1]);  \
                		wz = *(loc_dptr + mult[2]);  \
                		wz -= *(loc_dptr - mult[2]);  \
                		wi = RBllut[i];  \
                		wj = Gllut[i];  \
               		 	vl_get_inten(wx,wy,wz,&wi,&wj);  \
                		wi = wi * (255-loc_lop);  \
                		wj = wj * (255-loc_lop);  \
/* run through the buffer  w/llut */  \
                		ZCOLORIN(vx,loc_lop,wi,wj,image,zbuf);  \
			}  \
                };  \
        };  

/* -------------------------------------------------------------- */

#define OVERLIGHT \
/* if it is visible plot it */  \
        if ((loc_rop = ropacity[i]) < 255) {  \
/* get the X,Y point */  \
                vx[0] = vid[0] >> 16;  \
                vx[1] = vid[1] >> 16;  \
                vx[2] = vid[2] >> 13;  \
/* LIGHT the point */  \
                wx = *(loc_dptr + mult[0]);  \
                wx -= *(loc_dptr - mult[0]);  \
                wy = *(loc_dptr + mult[1]);  \
                wy -= *(loc_dptr - mult[1]);  \
                wz = *(loc_dptr + mult[2]);  \
                wz -= *(loc_dptr - mult[2]);  \
                wi = RBrlut[i];  \
                wj = Grlut[i];  \
                vl_get_inten(wx,wy,wz,&wi,&wj);  \
                wi = wi * (255-loc_rop);  \
                wj = wj * (255-loc_rop);  \
/* run through the buffer */  \
                ZCOLORIN(vx,loc_rop,wi,wj,image,zbuf);  \
        }; 

/* -------------------------------------------------------------- */

#define OVER16 \
/* if it is visible plot it */  \
        loc_rop = (ropacity[i]*lopacity[i2])/255 ; \
	if (loc_rop > 0) {  \
/* get the X,Y point */  \
        	vx[0] = vid[0] >> 16;  \
        	vx[1] = vid[1] >> 16;  \
        	vx[2] = vid[2] >> 13;  \
/* run through the buffer */  \
		i2 = i + (256*i2); \
                wi = RBlut[i2]*loc_rop; \
                wj = Glut[i2]*loc_rop; \
		loc_rop = 255 - loc_rop; \
		ZCOLORIN(vx,loc_rop,wi,wj,image,zbuf); \
        }; 

/* -------------------------------------------------------------- */

#define OVERLIGHT16 \
/* if it is visible plot it */  \
	loc_rop = (ropacity[i]*lopacity[i2]) >> 8; \
        if (loc_rop > 0) {  \
/* get the X,Y point */  \
                vx[0] = vid[0] >> 16;  \
                vx[1] = vid[1] >> 16;  \
                vx[2] = vid[2] >> 13;  \
/* LIGHT the point */  \
                wx = *(loc_dptr + mult[0]);  \
                wx -= *(loc_dptr - mult[0]);  \
                wy = *(loc_dptr + mult[1]);  \
                wy -= *(loc_dptr - mult[1]);  \
                wz = *(loc_dptr + mult[2]);  \
                wz -= *(loc_dptr - mult[2]);  \
		i2 = i + (256*i2); \
                wi = RBlut[i2]; \
                wj = Glut[i2]; \
                vl_get_inten(wx,wy,wz,&wi,&wj);  \
                wi = wi * (loc_rop);  \
                wj = wj * (loc_rop);  \
		loc_rop = 255 - loc_rop; \
/* run through the buffer */  \
                ZCOLORIN(vx,loc_rop,wi,wj,image,zbuf);  \
        }; 

/* -------------------------------------------------------------- */

#define PLANEOVER16 \
/* if it is visible plot it */  \
        loc_rop = (ropacity[i]*lopacity[i2])/255 ; \
	if (loc_rop > 0) {  \
/* get the X,Y point */  \
        	vx[0] = vid[0] >> 16;  \
        	vx[1] = vid[1] >> 16;  \
        	vx[2] = vid[2] >> 13;  \
/* run through the buffer */  \
		i2 = i + (256*i2); \
                wi = RBlut[i2]*loc_rop; \
                wj = Glut[i2]*loc_rop; \
		loc_rop = 255 - loc_rop; \
		if (vx[2] <= pbuffer[(vx[0] + (vx[1]*imagex))]) { \
			ZCOLORIN(vx,loc_rop,wi,wj,image,zbuf); \
		}; \
        }; 

/* -------------------------------------------------------------- */

#define PLANEOVERLIGHT16 \
/* if it is visible plot it */  \
	loc_rop = (ropacity[i]*lopacity[i2]) >> 8; \
        if (loc_rop > 0) {  \
/* get the X,Y point */  \
                vx[0] = vid[0] >> 16;  \
                vx[1] = vid[1] >> 16;  \
                vx[2] = vid[2] >> 13;  \
/* LIGHT the point */  \
                wx = *(loc_dptr + mult[0]);  \
                wx -= *(loc_dptr - mult[0]);  \
                wy = *(loc_dptr + mult[1]);  \
                wy -= *(loc_dptr - mult[1]);  \
                wz = *(loc_dptr + mult[2]);  \
                wz -= *(loc_dptr - mult[2]);  \
		i2 = i + (256*i2); \
                wi = RBlut[i2]; \
                wj = Glut[i2]; \
                vl_get_inten(wx,wy,wz,&wi,&wj);  \
                wi = wi * (loc_rop);  \
                wj = wj * (loc_rop);  \
		loc_rop = 255 - loc_rop; \
/* run through the buffer */  \
		if (vx[2] <= pbuffer[(vx[0] + (vx[1]*imagex))]) { \
                	ZCOLORIN(vx,loc_rop,wi,wj,image,zbuf);  \
		}; \
        }; 

