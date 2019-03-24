#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* local prototypes */
long int vl_Clipt(double denom, double num, double *tE, double *tL);
long int InBounds(long int vx,long int vy,long int Xmin, long int Ymin,
	long int Xmax, long int Ymax);
long int base_clip(long int *vid, long int Dx,long int Dy, long int nloops,
	long int Xmin, long int Ymin, long int Xmax, long int Ymax,
	long int *out_t);

long int vl_clipline(long int *vid,long int *vidd,long int nloops,
	long int Xmin, long int Ymin, long int Xmax, long int Ymax,
	long int *out)
{
	long int	j,xp,yp;

/* step one, use Liang-Barsky for find "close" values */
	if (!base_clip(vid,vidd[0]*(nloops-1),vidd[1]*(nloops-1),nloops,
		Xmin,Ymin,Xmax,Ymax,out)) return(0);

/* now it is touchup time */
/* first for tE */
	if (out[0] != 0) {
		j = out[0];
		xp = vid[0]+vidd[0]*j;
		yp = vid[1]+vidd[1]*j;
		if (InBounds(xp,yp,Xmin,Ymin,Xmax,Ymax)) {
			xp -= vidd[0];
			yp -= vidd[1];
			while ((j > 1) && 
				(InBounds(xp,yp,Xmin,Ymin,Xmax,Ymax))) {
				j -= 1;
				xp -= vidd[0];
				yp -= vidd[1];
			}
		} else {
			xp += vidd[0];
			yp += vidd[1];
			while ((j < nloops-2) && 
				(!InBounds(xp,yp,Xmin,Ymin,Xmax,Ymax))) {
				j += 1;
				xp += vidd[0];
				yp += vidd[1];
			}
		}
		out[0] = j;
	}
/* last for tL */
	if (out[1] != (nloops-1)) {
		j = out[1];
		xp = vid[0]+vidd[0]*j;
		yp = vid[1]+vidd[1]*j;
		if (!InBounds(xp,yp,Xmin,Ymin,Xmax,Ymax)) {
			xp -= vidd[0];
			yp -= vidd[1];
			while ((j > 1) && 
				(!InBounds(xp,yp,Xmin,Ymin,Xmax,Ymax))) {
				j -= 1;
				xp -= vidd[0];
				yp -= vidd[1];
			}
		} else {
			xp += vidd[0];
			yp += vidd[1];
			while ((j < nloops-2) && 
				(InBounds(xp,yp,Xmin,Ymin,Xmax,Ymax))) {
				j += 1;
				xp += vidd[0];
				yp += vidd[1];
			}
		}
		out[1] = j;
	}
	return(1);
}

/*--------------------------------------------------------------------------*/
/*  Clip line to upright rectangle using Liang-Barsky parametric algorithm  */
/*--------------------------------------------------------------------------*/
long int InBounds(long int vx,long int vy,long int Xmin, long int Ymin,
	long int Xmax, long int Ymax)
{
	if (vx < Xmin) return(0);
	if (vx > Xmax) return(0);
	if (vy < Ymin) return(0);
	if (vy > Ymax) return(0);
	return(1);
}

long int base_clip(long int *vid, long int Dx,long int Dy, long int nloops,
	long int Xmin, long int Ymin, long int Xmax, long int Ymax,
	long int *out_t)
{
	register	double	dx = Dx,dy = Dy;
	register	double	x0 = (vid[0]);
	register	double  y0 = (vid[1]);
	double		xmin = Xmin,ymin = Ymin;
	double		xmax = Xmax,ymax = Ymax;
	double		tE = 0,tL = 1;
	long int 	visible;
	 			
	visible = 0;
	
	if ((dx == 0) && (dy == 0) && 
		InBounds(vid[0],vid[1],Xmin,Ymin,Xmax,Ymax)) {
	
		visible = 1;
		
	}  else  {
	
		if (vl_Clipt(dx,xmin - x0,&tE,&tL))  {	
		
			if (vl_Clipt(-dx,x0 - xmax,&tE,&tL))  {
						
				if (vl_Clipt(dy,ymin - y0,&tE,&tL))  {	
							
					if (vl_Clipt(-dy,y0 - ymax,&tE,&tL))  {
					
						visible = 1;
						
					}
					
				}
				
			}
			
		}
					
	}	

	out_t[0] = tE*(double)(nloops-1);
	out_t[1] = tL*(double)(nloops-1);

	return visible;
}

long int vl_Clipt(double denom, double num, double *tE, double *tL)
{
	double		t;
	long int	accept;
	
	accept = 1;
	
	if (denom > 0)  {
	
		t=num/denom;
		
		if (t > *tL)  {
		
			accept = 0;
			
		}
		
		else if (t > *tE)  {
		
			*tE = t;
			
		}
		
	}
	
	else if (denom < 0)  {
	
		t=num/denom;
		
		if (t < *tE)  {
		
			accept = 0;
			
		}
		
		else if (t < *tL)  {
		
			*tL = t;
			
		}
		
	}
	
	else  {
	
		if (num > 0)  {
		
			accept = 0;
		
		}
		
	}
	
	return accept;	
}
