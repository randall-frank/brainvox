#include <math.h>
#include <stdio.h>

#ifdef CAP_NOUS
#define rgb_to_hsl_ RGB_TO_HSL
#define hsl_to_rgb_ HSL_TO_RGB
#define rgb_to_hsv_ RGB_TO_HSV
#define hsv_to_rgb_ HSV_TO_RGB
#endif

/*
 * RGB-HSL transforms.
 */

#define MIN(a,b)        (((a)<(b))?(a):(b))
#define MAX(a,b)        (((a)>(b))?(a):(b))

/*
 * given r,g,b on [0 ... 1],
 * return (h,s,l) on [0 ... 1]
 */

void rgb_to_hsl_(float r,float g,float b,float *h,float *s,float *l)
{
    double v;
    double m;
    double vm;
    double r2, g2, b2;

    v = MAX(r,g);
    v = MAX(v,b);
    m = MIN(r,g);
    m = MIN(m,b);

    if ((*l = (m + v) / 2.0) <= 0.0) return;
    if ((*s = vm = v - m) > 0.0) {
			*s /= (*l <= 0.5)
	    	? (v + m )
	    	: (2.0 - v - m)
	    	;
    } else
	return;


    r2 = (v - r) / vm;
    g2 = (v - g) / vm;
    b2 = (v - b) / vm;

    if (r == v)
		*h = (g == m ? 5.0 + b2 : 1.0 - g2);
    else if (g == v)
		*h = (b == m ? 1.0 + r2 : 3.0 - b2);
    else
		*h = (r == m ? 3.0 + g2 : 5.0 - r2);

    	*h /= 6;
	}

    /*
     * given h,s,l on [0..1],
     * return r,g,b on [0..1]
     */
void hsl_to_rgb_(float h,float sl,float l,float *r,float *g,float *b)
{
    double v;

    v = (l <= 0.5) ? (l * (1.0 + sl)) : (l + sl - l * sl);
    if (v <= 0) {
		*r = *g = *b = 0.0;
    } else {
		double m;
		double sv;
		int sextant;
		double fract, vsf, mid1, mid2;

		m = l + l - v;
		sv = (v - m ) / v;
		h *= 6.0;
		sextant = h;	
		fract = h - sextant;
		vsf = v * sv * fract;
		mid1 = m + vsf;
		mid2 = v - vsf;
		switch (sextant) {
			case 0: *r = v; *g = mid1; *b = m; break;
			case 1: *r = mid2; *g = v; *b = m; break;
			case 2: *r = m; *g = v; *b = mid1; break;
			case 3: *r = m; *g = mid2; *b = v; break;
			case 4: *r = mid1; *g = m; *b = v; break;
			case 5: *r = v; *g = m; *b = mid2; break;
		}
    }
}

void rgb_to_hsv_(float r,float g,float b,float *h,float *s,float *v)
{
        double  ax,in,d;

        ax = MAX(r,g);
	ax = MAX(ax,b);
        in = MIN(r,g);
	in = MIN(in,b);

        *v = ax;

        if (ax != 0.0) {
                *s = (ax-in)/ax;
        } else {
                *s = 0;
	}
        if (*s == 0.0) {
                *h = 0.0;
        } else {
                d = ax - in;
                if (r == ax) {
                        *h = (g - b)/d;
                } else if (g == ax) {
                        *h = 2.0 + (b - r)/d;
                } else if (b == ax) {
                        *h = 4.0 + (r - g)/d;
                }     
                *h = (*h) / 6.0;
                if (*h < 0) *h = (*h) + 1.0;
        }
}

void hsv_to_rgb_(float h1,float s1,float v1,float *r,float *g,float *b)
{
        double  h,s,v,f,q,p,t;
        int 	i;

        h = h1;
        s = s1;
        v = v1;

        if (s == 0.0) {
                *r = v;
                *g = v;
                *b = v;
        } else {
                if (h == 1.0) h = 0.0;
                h = h * 6.0;
                i = floor(h);
                f = h - (double)i;
                p = v * (1 - s);
                q = v * (1 - (s * f));
                t = v * (1 - (s * (1 - f)));
		switch (i) {
			case 0:
                        	*r = v;
                        	*g = t;
                        	*b = p;
				break;
			case 1:
                        	*r = q;
                        	*g = v;
                        	*b = p;
				break;
			case 2:
                        	*r = p;
                        	*g = v;
                        	*b = t;
				break;
			case 3:
                        	*r = p;
                        	*g = q;
                        	*b = v;
				break;
			case 4:
                        	*r = t;
                        	*g = p;
                        	*b = v;
				break;
			case 5:
                        	*r = v;
                        	*g = p;
                        	*b = q;
				break;
                }    
	}
}
