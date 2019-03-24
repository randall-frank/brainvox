#include <math.h>
#include <stdio.h>
#include <stdlib.h>

void pr_utdm_v(double *x, int N, int width, int precision)
     /* "print a upper triangular double matrix stored as a vector"
        The matrix is N x N, the vector has N*(N+1)/2 elements.
        Each element is formatted using width and precision.
        There are no sanity checks at all. */
{
    int pos=0, i, j, leavespace;
    char s[100], fmt[100];      /* will be used in making printf() formats */

    sprintf(fmt, "%%%d.%dg", width, precision);
    for (i=0; i<N; i++) {
        leavespace = i*width;
        sprintf(s, "%%%ds", leavespace); printf(s, "");
        for (j=i; j<N; j++) printf(fmt, x[pos++]);
        printf("\n");
    }
}

double *dvector(int l, int h)
{
    double *block;
    int size;

    size = h - l + 1;
    block = (double *) malloc(sizeof(double)*size);
    if (block == NULL) fprintf(stderr, "malloc failure in dvector()\n");
    return block-l;
}

int *ivector(int l, int h)
{
    int *block, size;

    size = h - l + 1;
    block = (int *) malloc(sizeof(int)*size);
    if (block == NULL) fprintf(stderr, "malloc failure in ivector()\n");
    return block-l;
}

double **dmatrix(int rl, int rh, int cl, int ch)
{
    double *block;
    double **m;
    int size, i, rowsize, numrows;

    rowsize = ch - cl + 1;	/* #locations consumed by 1 row */
    numrows = rh -rl + 1;
    size = numrows*rowsize;
    block = (double *) malloc((unsigned) sizeof(double)*size);
    if (block == NULL) 
      fprintf(stderr, "malloc failure in matrix allocation\n");
    /* so we have the matrix. */

    /* Now for the row pointers */
    m = (double **) malloc((unsigned) sizeof(double *)*numrows);
    if (m == NULL) fprintf(stderr, "malloc failure in matrix allocation\n");
    m -= rl;	/* fixup m pointer so m[rl] == old m[0] */

    /* Finally, setup pointers to rows */
    block -= cl;
    for (i=rl; i<=rh; i++) {
        m[i] = block; block += rowsize;
    }
    return m;
}

void putdvec(const char *s, double *x, int l, int h)
{
    int i;
    printf("Vector %-10s: ", s);
    for (i=l; i<=h; i++) printf("%.4g ", x[i]);
    printf("\n");
}

static double r_mod(float x, float y)    /* used by f2c's version of rand() */
{
    double quotient = (double) x/y;
    quotient = quotient >= 0 ? floor(quotient) : -floor(-quotient);
    return x - y*quotient;
}

struct {
    int ix, iy, iz;
} randc;

double ranwm()
{
    float r;
    randc.ix = randc.ix * 171 % 30269;
    randc.iy = randc.iy * 172 % 30307;
    randc.iz = randc.iz * 170 % 30323;
    r = (float) randc.ix / (float) 30269. + (float) randc.iy / (float)
	    30307. + (float) randc.iz / (float) 30323.;
    return r_mod(r, 1.0);
}
