#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <float.h>

/*
 * $Id: stats.c 1787 2006-06-03 21:11:31Z rjfrank $
 */

#include "stats_inc.h"


/* local prototypes */
static int dcheck(const void *d1,const void *d2);
static double binv(double p,double q);

double lngamma(double z);
double ibeta(double a,double  b,double  x);
double igamma(double a,double  x);

#define EPS 1.0e-18
#define MACHEPS 1.0E-08
#define ITMAX 100000

/* Normally I would use routines from Numerical Recipes in C,
	but since I cannot distribute the routines, I have come up 
	with replacements for the ones I use */
/* This define would select the NRC versions of the routines */
#ifdef USE_NRC

#include "nrc.c"

#else

/* replacement routines from the public domain */

double betai(double a,double b,double x)
{
	return(ibeta(a,b,x));
}
double gammln(double x)
{
	return(lngamma(x));
}
double gammp(double a,double x)
{
	return(igamma(a,x));
}
double gammq(double a,double x)
{
	return(1.0 - igamma(a,x));
}
void    chstwo(double *R,double *S,long int n,long int constrain,
        double *df,double *chsq,double *p)
{
	double		tmp = 0.0;
	long int	i;
	
	*df = n - 1 - constrain;
	for(i=0;i<n;i++) {
/* check divide by zero */		
		if ((R[i] == 0.0) && (S[i] == 0.0)) {
			*df -= 1;  /* empty bins do not count */
		} else {
			tmp += ((R[i] - S[i])*(R[i] - S[i]))/
				(R[i] + S[i]);
		}
	}
	*chsq = tmp;
/* return the probability */
	*p = 1.0 - CDF_chi(tmp,df);
}
void    kstwo(double *d1,long int n1,double *d2,long int n2,
                double *ks,double *p)
{
	double		tmp;
	long int	i1,i2;

	i1 = 0;
	i2 = 0;
/* sort the event times (ie histogramming "sort of") */
	qsort(d1,n1,sizeof(double),dcheck);
	qsort(d2,n2,sizeof(double),dcheck);
/* add from one or the other dist, forming the step-wise cululative fun */
	while((i1 < n1) && (i2 < n2)) {
		if (d1[i1] <= d2[i2]) {
			i1 += 1;
		} else {
			i2 += 1;
		}
		tmp = fabs(((double)(i2)/(double)(n2)) - 
			((double)(i1)/(double)(n1)));
		if (tmp > *ks) *ks = tmp;
	}
	*p = 0.0;
}

double probks(double ks)
{
	double		tmp,p,inc1,inc2;
	long int	i;

	tmp = 0.0;
	p = -2.0*ks*ks;

	for(i=0;i<(ITMAX/2);i+=2) {
		inc1 = 2.0*exp(p*(double)(i*i));
		inc2 = 2.0*exp(p*(double)((i+1)*(i+1)));
		tmp = tmp + inc1 - inc2;
		if (fabs(inc1) < tmp*MACHEPS) return(tmp);
		if (fabs(inc1) > MACHEPS*fabs(inc2)) return(tmp);
	}

	return(tmp);
}

void    moment(double *data,long int n,double *ave,double *adev,double *sdev,
                double *var,double *skew,double *kurt)
{
	double		mean,tmp;
	long int	i;

/* clear */
	*ave = 0.0;
	*adev = 0.0;
	*sdev = 0.0;
	*var = 0.0;
	*skew = 0.0;
	*kurt = 0.0;
	if (n < 2) return;

/* get the mean */
	mean = 0.0;
	for(i=0;i<n;i++) mean += data[i];
	mean = mean/(double)(n);
	*ave = mean;
/* get the moments */
	for(i=0;i<n;i++) {
		tmp = (data[i] - mean);
		*adev += fabs(tmp);
		*var += (tmp*tmp);
		*skew += (tmp*tmp*tmp);
		*kurt += (tmp*tmp*tmp*tmp);
	}
/* normalize them */
	*adev = (*adev)/(double)(n);
	*var = (*var)/(double)(n-1.0);
	*sdev = sqrt(*var);
	if (*sdev == 0) return;
	*skew = (*skew)/((double)(n)*(*sdev)*(*sdev)*(*sdev));
	*kurt = (*kurt)/((double)(n)*(*sdev)*(*sdev)*(*sdev)*(*sdev));
	*kurt -= 3.0;

	return;
}

#endif

static int dcheck(const void *d1,const void *d2)
{
	double	v1,v2;
	v1 = *((double *)d1);
	v2 = *((double *)d2);

	if (v1 > v2) return(1);
	if (v2 > v1) return(-1);

	return(0);
}

#if NEVER

static int fcheck(const void *d1,const void *d2);

static int fcheck(const void *d1,const void *d2)
{
	float	v1,v2;
	v1 = *((float *)d1);
	v2 = *((float *)d2);

	if (v1 > v2) return(1);
	if (v2 > v1) return(-1);

	return(0);
}

/* this routine needs to be rewritten!!!  */
float	median(float *x,long int n)
{
	long int	i;

	qsort(x,n,sizeof(float),fcheck);
	i = n/2;

	if (n & 1) return(x[i]);
	return(0.5*(x[i]+x[i+1]));
}
#else
/* ACM algorithm #489, Select, a partial quicksort routine */
float	median(float *x,long int n)
{
	long int	med,l,r,i,j;
	float		tmp1,tmp2;

	med = n/2;
	l = 0;
	r = n-1;
	while(r>l) {
		tmp1 = x[med];
		i = l;
		j = r;
		tmp2 = x[l];
		x[l] = x[med];
		x[med] = tmp2;
		if (x[r] > tmp1) {
			tmp2 = x[l];
			x[l] = x[r];
			x[r] = tmp2;
		}
		while(i<j) {   /* until corss-over */
			tmp2 = x[i];
			x[i] = x[j];
			x[j] = tmp2;
			i += 1;
			while(x[i] < tmp1) i += 1;
			j -= 1;
			while(x[j] > tmp1) j -= 1;
		}
		if (x[l] == tmp1) {
			tmp2 = x[l];
			x[l] = x[j];
			x[j] = tmp2;
		} else {
			j += 1;
			tmp2 = x[j];
			x[j] = x[r];
			x[r] = tmp2;
		}
		if (j <= med) l = j + 1;
		if (j >= med) r = j - 1;
	}
	return(x[med]);
}
#endif


double	factorial(long int n)
{
	static long int	cachemax = 1;
	static double	cache[30] = {1.0};
	long int	i;

	if (n < 0) {
		fprintf(stderr,"Warning: attempt to take factorial of %ld\n",n);
		return(1.0);
	}
	if (n >= 30) return(exp(gammln((double)(n)+1.0)));
	if (n < cachemax) return(cache[n]);
	for(i=cachemax;i<=n;i++) {
		cache[i] = cache[i-1]*i;
	}
	return(cache[n]);
}

#ifndef __APPLE__
double	erf(double x)
{
	return (x < 0.0 ? -gammp(0.5,x*x) : gammp(0.5,x*x));
}

double	erfc(double x)
{
	return (x < 0.0 ? 1.0 + gammp(0.5,x*x) : 1.0 - gammp(0.5,x*x));
}
#endif

static double binv(double p,double q)
{
	return( exp(gammln(p+q)-gammln(p)-gammln(q)) );
}

double PDF_normal(double x,double *v)
{
	double	invsqrt2pi = 1.0/sqrt(2*M_PI);
	double	sigma = 1.0;
	double	mu = 0.0;

	return( invsqrt2pi/sigma*exp(-0.5*pow((x-mu)/sigma,2.0)) );
}
double CDF_normal(double x,double *v)
{
	double	sigma = 1.0;
	double	mu = 0.0;

	return( 0.5+0.5*erf((x-mu)/sigma/sqrt(2.0)) );
}

double PDF_t(double x,double *v)
{
	double df = v[0];

	return( binv(0.5*df,0.5)/sqrt(df)*pow((1.0+(x*x)/df),-0.5*(df+1.0)) );
}
double CDF_t(double x,double *v)
{
	double df = v[0];

	double	t;

	t = 0.5*betai(0.5*df,0.5,df/(df+x*x));
	if (x > 0.0) {
		t = 1.0 - t;
	} 
	return(t);
}
double PDF_f(double x,double *v)
{
	double df1 = v[0];
	double df2 = v[1];
	double	t;

	t = binv(0.5*df1,0.5*df2)*pow(df1/df2,0.5*df1)*pow(x,(0.5*df1)-1.0);
	t = t / pow(1.0+df1/df2*x,0.5*(df1+df2));

	return(t);
}
double CDF_f(double x,double *v)
{
	double df1 = v[0];
	double df2 = v[1];

	return( 1.0-betai(0.5*df2,0.5*df1,df2/(df2+df1*x)) );
}
double PDF_chi(double x,double *v)
{
	double	log2 = log(2.0);
	double df = v[0];

	return( exp((0.5*df-1.0)*log(x)-0.5*x-gammln(0.5*df)-df*0.5*log2) );
}
double CDF_chi(double x,double *v)
{
	double df = v[0];

	return( gammp(0.5*df,0.5*x) );
}


#ifdef NEVER
int main(int argc, char **argv)
{
	double	d,x,v,inc,df,ox,on,n;
	long int	stuck;

	printf("Normal distribution\n");
	for(d=-10;d<10;d+=1.0) {
		printf("d=%lf : PDF:%lf CDF:%lf\n",d,
			PDF_normal(d,NULL),CDF_normal(d,NULL));
	}

	for(df=5;df<21;df+=10) {
		if (df == 20.0) df = 1000.0;
		printf("T for df= %lf\n",df);
	for(d=-10;d<10;d+=1.0) {
		printf("d=%lf : PDF:%lf CDF:%lf\n",d,
			PDF_t(d,&df),CDF_t(d,&df));
	}
	}
		
	df = 100;
	printf("DF=%lf\n",df);
	for(d=-15.0;d<=15.0;d+=0.50) {
		v = CDF_t(d,&df);
		inc = 0.01;
		x = 0.0;  /* start in the middle */
		ox = x+inc;
		on = CDF_normal(ox,NULL);
		stuck = 0;
		while (1) {
			n = CDF_normal(x,NULL);
			if (fabs(n-v) < 5.0e-16) break;
			inc = ox - ((on-v)/(on-n))*(ox-x);
			ox = x;
			on = n;
			x = inc;
			stuck++;
			if (stuck > ITMAX) break;
		}
		printf("d,df,t=%9le,%9lg,%9le  Z=%9le,%9le\n",d,df,v,x,CDF_normal(x,NULL));
	}
	
	exit(0);
}
#endif


/* functions borrowed from the GNUPLOT software package */

#define GAMMA lngamma
#define OFLOW   FLT_MAX
#define MINEXP  -88.0
#define XBIG    2.55E+305
#define MACHEPS 1.0E-08

#define MIN(a,b) ((a) < (b) ? (a) : (b))

static double   lgamneg(double z);
static double   lgampos(double z);
static double   confrac(double a,double b,double x);

/* from Ray Toy */
static double a[] = {
        .99999999999980993227684700473478296744476168282198,
     676.52036812188509856700919044401903816411251975244084,
   -1259.13921672240287047156078755282840836424300664868028,
     771.32342877765307884865282588943070775227268469602500,
    -176.61502916214059906584551353999392943274507608117860,
      12.50734327868690481445893685327104972970563021816420,
       -.13857109526572011689554706984971501358032683492780,
        .00000998436957801957085956266828104544089848531228,
        .00000015056327351493115583383579667028994545044040,
};

static double   lgamneg(double z)
{
    double          tmp;

    /* Use reflection formula, then call lgampos() */
    tmp = sin(z * M_PI);

    if (fabs(tmp) < MACHEPS) {
        tmp = 0.0;
    } else if (tmp < 0.0) {
        tmp = -tmp;
#ifndef WIN32
        signgam = -1;
#endif
    }
    return(LN_PI - lgampos(1.0 - z) - log(tmp));

}

static double   lgampos(double z)
{
    double          sum;
    double          tmp;
    int             i;

    sum = a[0];
    for (i = 1, tmp = z; i < 9; i++) {
        sum += a[i] / tmp;
        tmp++;
    }

    return (log(sum) + LNSQRT_2PI - z - 6.5 + (z - 0.5) * log(z + 6.5));
}

double lngamma(double z)
{
    if (z <= 0.0)
        return lgamneg(z);
    else
        return lgampos(z);
}

/** ibeta.c
 *
 *   DESCRIB   Approximate the incomplete beta function Ix(a, b).
 *
 *                           _
 *                          |(a + b)     /x  (a-1)         (b-1)
 *             Ix(a, b) = -_-------_--- * |  t     * (1 - t)     dt (a,b > 0)
 *                        |(a) * |(b)   /0
 *
 *
 *
 *   CALL      p = ibeta(a, b, x)
 *
 *             double    a    > 0
 *             double    b    > 0
 *             double    x    [0, 1]
 *
 *   WARNING   none
 *
 *   RETURN    double    p    [0, 1]
 *                            -1.0 on error condition
 *
 *   XREF      lngamma()
 *
 *   BUGS      none
 *
 *   REFERENCE The continued fraction expansion as given by
 *             Abramowitz and Stegun (1964) is used.
 *
 * Permission granted to distribute freely for non-commercial purposes only
 * Copyright (c) 1992 Jos van der Woude, jvdwoude@hut.nl
 */


double ibeta(double a,double  b,double  x)
{
    /* Test for admissibility of arguments */
    if (a <= 0.0 || b <= 0.0)
        return -1.0;
    if (x < 0.0 || x > 1.0)
        return -1.0;;

    /* If x equals 0 or 1, return x as prob */
    if (x == 0.0 || x == 1.0)
        return x;

    /* Swap a, b if necessarry for more efficient evaluation */
    return a < x * (a + b) ? 1.0 - confrac(b, a, 1.0 - x) : confrac(a, b, x);
}

static double   confrac(double a,double b,double x)
{
    double          Alo = 0.0;
    double          Ahi;
    double          Aev;
    double          Aod;
    double          Blo = 1.0;
    double          Bhi = 1.0;
    double          Bod = 1.0;
    double          Bev = 1.0;
    double          f;
    double          fold;
    double          Apb = a + b;
    double          d;
    int             i;
    int             j;

    /* Set up continued fraction expansion evaluation. */
    Ahi = exp(GAMMA(Apb) + a * log(x) + b * log(1.0 - x) -
              GAMMA(a + 1.0) - GAMMA(b));

    /*
     * Continued fraction loop begins here. Evaluation continues until
     * maximum iterations are exceeded, or convergence achieved.
     */
    for (i = 0, j = 1, f = Ahi; i <= ITMAX; i++, j++) {
        d = a + j + i;
        Aev = -(a + i) * (Apb + i) * x / d / (d - 1.0);
        Aod = j * (b - j) * x / d / (d + 1.0);
        Alo = Bev * Ahi + Aev * Alo;
        Blo = Bev * Bhi + Aev * Blo;
        Ahi = Bod * Alo + Aod * Ahi;
        Bhi = Bod * Blo + Aod * Bhi;

        if (fabs(Bhi) < MACHEPS)
            Bhi = 0.0;

        if (Bhi != 0.0) {
            fold = f;
            f = Ahi / Bhi;
            if (fabs(f - fold) < fabs(f) * MACHEPS)
                return f;
        }
    }

    return -1.0;
}

/** igamma.c
 *
 *   DESCRIB   Approximate the incomplete gamma function P(a, x).
 *
 *                         1     /x  -t   (a-1)
 *             P(a, x) = -_--- * |  e  * t     dt      (a > 0)
 *                       |(a)   /0
 *
 *   CALL      p = igamma(a, x)
 *
 *             double    a    >  0
 *             double    x    >= 0
 *
 *   WARNING   none
 *
 *   RETURN    double    p    [0, 1]
 *                            -1.0 on error condition
 *
 *   XREF      lngamma()
 *
 *   BUGS      Values 0 <= x <= 1 may lead to inaccurate results.
 *
 *   REFERENCE ALGORITHM AS239  APPL. STATIST. (1988) VOL. 37, NO. 3
 *
 * Permission granted to distribute freely for non-commercial purposes only
 * Copyright (c) 1992 Jos van der Woude, jvdwoude@hut.nl
 */

/* Global variables, not visible outside this file */
static double   pn1, pn2, pn3, pn4, pn5, pn6;

double igamma(double a,double  x)
{
    double          arg;
    double          aa;
    double          an;
    double          b;
    int             i;

    /* Check that we have valid values for a and x */
    if (x < 0.0 || a <= 0.0)
        return -1.0;

    /* Deal with special cases */
    if (x == 0.0)
        return 0.0;
    if (x > XBIG)
        return 1.0;

    /* Check value of factor arg */
    arg = a * log(x) - x - GAMMA(a + 1.0);
    if (arg < MINEXP)
        return -1.0;
    arg = exp(arg);

    /* Choose infinite series or continued fraction. */

    if ((x > 1.0) && (x >= a + 2.0)) {
        /* Use a continued fraction expansion */

        double          rn;
        double          rnold;

        aa = 1.0 - a;
        b = aa + x + 1.0;
        pn1 = 1.0;
        pn2 = x;
        pn3 = x + 1.0;
        pn4 = x * b;
        rnold = pn3 / pn4;

        for (i = 1; i <= ITMAX; i++) {

            aa++;
            b += 2.0;
            an = aa * (double) i;

            pn5 = b * pn3 - an * pn1;
            pn6 = b * pn4 - an * pn2;

            if (pn6 != 0.0) {

                rn = pn5 / pn6;
                if (fabs(rnold - rn) <= MIN(MACHEPS, MACHEPS * rn))
                    return 1.0 - arg * rn * a;

                rnold = rn;
            }
            pn1 = pn3;
            pn2 = pn4;
            pn3 = pn5;
            pn4 = pn6;

            /* Re-scale terms in continued fraction if terms are large */
            if (fabs(pn5) >= OFLOW) {

                pn1 /= OFLOW;
                pn2 /= OFLOW;
                pn3 /= OFLOW;
                pn4 /= OFLOW;
            }
        }
    } else {
        /* Use Pearson's series expansion. */

        for (i = 0, aa = a, an = b = 1.0; i <= ITMAX; i++) {

            aa++;
            an *= x / aa;
            b += an;
            if (an < b * MACHEPS)
                return arg * b;
        }
    }
    return -1.0;
}

