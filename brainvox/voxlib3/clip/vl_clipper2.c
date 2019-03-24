#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "2dpolygon.h"

#define DEBUG 1
#define FF (65536.0)

long int main(long int argc,char **argv)
{
	char		str[256];
	long int	x1,y1,x2,y2,l,t,r,b;
	long int	j;
	long int	out[2],nloops;
	long int	vidd[2],vid[2];
	double		dx1,dx2,dy1,dy2;

	l = (100 << 16);
	r = (200 << 16);
	t = (100 << 16);
	b = (200 << 16);
	while(fgets(str,256,stdin) != 0) {
		if (str[0] == 'R') {
			sscanf(&(str[1]),"%ld %ld %ld %ld\n",&l,&t,&r,&b);
			l = (l << 16);
			r = (r << 16);
			t = (t << 16);
			b = (b << 16);
		} else if (str[0] == 'V') {
			if (sscanf(&(str[1]),"%lf %lf %lf %lf %ld\n",&dx1,&dy1,&dx2,&dy2,&nloops) == 5) {
				vid[0] = dx1*FF;
				vid[1] = dy1*FF;
				vidd[0] = dx2*FF;
				vidd[1] = dy2*FF;
				printf("Clipping: %lf,%lf %lf,%lf to %lf,%lf %lf,%lf\n",
					vid[0]/FF,vid[1]/FF,
					(vid[0]+(nloops*vidd[0]))/FF,(vid[1]+(nloops*vidd[1]))/FF,
					l/FF,t/FF,r/FF,b/FF);
				j = vl_clipline(vid,vidd,nloops,l,t,r,b,out);
				if (j == 0) {
					printf("Line outside the rect\n");
				} else {
					printf("Clipped to: %lf,%lf %lf,%lf\n",
						(vid[0]+(out[0]*vidd[0]))/FF,(vid[1]+(out[0]*vidd[1]))/FF,
						(vid[0]+(out[1]*vidd[0]))/FF,(vid[1]+(out[1]*vidd[1]))/FF);
				}		
			} else {
				printf("Wrong number of V values\n");
			}
		} else if (sscanf(str,"%ld %ld %ld %ld\n",&x1,&y1,&x2,&y2) == 4) {
			nloops = sqrt(((x1-x2)*(x1-x2)) + ((y1-y2)*(y1-y2)));
			nloops *= 2;
			vid[0] = (x1<<16);
			vid[1] = (y1<<16);
			vidd[0] = ((x2 - x1)<<16)/(nloops);
			vidd[1] = ((y2 - y1)<<16)/(nloops);
			x1 = vid[0] >> 16;
			y1 = vid[1] >> 16;
			x2 = (vid[0] + (vidd[0]*(nloops))) >> 16;
			y2 = (vid[1] + (vidd[1]*(nloops))) >> 16;
			printf("Clipping: %ld,%ld %ld,%ld to %ld,%ld %ld,%ld\n",
				x1,y1,x2,y2,l>>16,t>>16,r>>16,b>>16);
			j = vl_clipline(vid,vidd,nloops,l,t,r,b,out);
			if (j == 0) {
				printf("Line outside the rect\n");
			} else {
				x1 = (vid[0] + (vidd[0]*out[0])) >> 16;
				y1 = (vid[1] + (vidd[1]*out[0])) >> 16;
				x2 = (vid[0] + (vidd[0]*out[1])) >> 16;
				y2 = (vid[1] + (vidd[1]*out[1])) >> 16;
				printf("Line clipped to:%ld,%ld %ld,%ld\n",
					x1,y1,x2,y2);
			}
		} else {
				printf("Wrong number of values\n");
		}
	}

	exit(0);
}

/* taken liberally from Foley and VanDam (2) pg 122 */
long int vl_CLIPt(double d,double n,double *tE,double *tL) 
{
	double		t;
	long int	accept;
	
	accept = 1;
	if (d > 0.0) {
		t = n/d;
		if (t > (*tL)) {
			accept = 0;
		} else if (t > (*tE)) {
			*tE = t;
		}
	} else if (d < 0.0) {
		t = n/d;
		if (t < (*tE)) {
			accept = 0;
		} else if (t < (*tL)) {
			*tL = t;
		}
	} else {
		if (n > 0.0) accept = 0;
	}
	return(accept);
}

/* return 1 if inside the rect */
long int vl_CLIPp(long int x,long int y,
	long int Xmin,long int Ymin,long int Xmax,long int Ymax)
{

#ifdef DEBUG
	printf("vl_CLIPp %lf,%lf : %lf,%lf %lf,%lf\n",x/FF,y/FF,Xmin/FF,Ymin/FF,Xmax/FF,Ymax/FF);
#endif

	if (x < Xmin) return(0);
	if (x >= Xmax) return(0);
	if (y < Ymin) return(0);
	if (y >= Ymax) return(0);
	
	return(1);
}

/* all input values are 16.16 fixed point values */
long int vl_clipline(long int *vid,long int *vidd,long int nloops,
	long int Xmin, long int Ymin, long int Xmax, long int Ymax,
	long int *out)
{
/* vid = start point, vidd = deltas, nloops = steps */
/* v1=vid; v2=vid+(nloops*vidd); */
/* clips to Xmin,Ymin,Xmax,Ymax */
/* out[0] = nloops start, out[1] = nloops end */
/* returns 0 if clipped out */

	long int	visible,j;
	long int	xp,yp;
	double		tE,tL;
	double		dx,dy;
	
	dx = vidd[0]*(nloops);
	dy = vidd[1]*(nloops);
	visible = 0;
	out[0] = 0;
	out[1] = (nloops);
#ifdef DEBUG
	printf("vl_clipline x,y:%lf,%lf dx,dy:%lf,%lf: %lf,%lf %lf,%lf\n",
		vid[0]/FF,vid[1]/FF,dx/FF,dy/FF,Xmin/FF,Ymin/FF,Xmax/FF,Ymax/FF);
	printf("nloops=%ld\n",nloops);
#endif
	
	if ((dx == 0) && (dy == 0) && vl_CLIPp(vid[0],vid[1],Xmin,Ymin,Xmax,Ymax)) {
		visible = 1;
	} else {
		tE = 0.0;
		tL = 1.0;
		if (vl_CLIPt(dx,(double)(Xmin-vid[0]),&tE,&tL)) {
			if (vl_CLIPt(-dx,(double)(vid[0]-Xmax),&tE,&tL)) {
				if (vl_CLIPt(dy,(double)(Ymin-vid[1]),&tE,&tL)) {
					if (vl_CLIPt(-dy,(double)(vid[1]-Ymax),&tE,&tL)) {
							visible = 1;
							if (tE > 0.0) out[0] = tE*(double)(nloops);
							if (tL < 1.0) out[1] = tL*(double)(nloops);
					}
				}
			}
		}
	}
	if (!visible) return(visible);
	
#ifdef DEBUG
	printf("out[0] = %ld, out[1] = %ld\n",out[0],out[1]);
#endif
/* now it is touchup time */
/* first for tE */
/*	if (out[0] != 0) { */
	if (1) {
		j = out[0];
		xp = vid[0]+vidd[0]*j;
		yp = vid[1]+vidd[1]*j;
		if (vl_CLIPp(xp,yp,Xmin,Ymin,Xmax,Ymax)) {
			xp -= vidd[0];
			yp -= vidd[1];
			while ((j > 0) && 
				(vl_CLIPp(xp,yp,Xmin,Ymin,Xmax,Ymax))) {
				j -= 1;
				xp -= vidd[0];
				yp -= vidd[1];
			}
		} else {
			xp += vidd[0];
			yp += vidd[1];
			j += 1;
			while ((j < nloops) && 
				(!vl_CLIPp(xp,yp,Xmin,Ymin,Xmax,Ymax))) {
				j += 1;
				xp += vidd[0];
				yp += vidd[1];
			}
		}
		out[0] = j;
	}
/* last for tL */
/*	if (out[1] != (nloops)) { */
	if (1) {
		j = out[1];
		xp = vid[0]+vidd[0]*j;
		yp = vid[1]+vidd[1]*j;
		if (!vl_CLIPp(xp,yp,Xmin,Ymin,Xmax,Ymax)) {
			j -= 1;
			xp -= vidd[0];
			yp -= vidd[1];
			while ((j > 0) && 
				(!vl_CLIPp(xp,yp,Xmin,Ymin,Xmax,Ymax))) {
				j -= 1;
				xp -= vidd[0];
				yp -= vidd[1];
			}
		} else {
			xp += vidd[0];
			yp += vidd[1];
			while ((j < nloops) && 
				(vl_CLIPp(xp,yp,Xmin,Ymin,Xmax,Ymax))) {
				j += 1;
				xp += vidd[0];
				yp += vidd[1];
			}
		}
		out[1] = j;
	}
#ifdef DEBUG
	printf("out[0] = %ld, out[1] = %ld\n",out[0],out[1]);
#endif		
	return(visible);
}