#include <stdio.h>
#include "as274c.h"
#include "misc.h"

int main()
{
    double **x, *y, *d, *rbar, *thetab, *xrow, sserr, *beta;
    double *tol, *rss;
    int i, j, *vorder, *lindep, error;

    x = dmatrix(0, 10, 0, 4);
    y = dvector(0, 10);
    d = dvector(0, 4);
    rbar = dvector(0, 9);
    thetab = dvector(0, 4);
    xrow = dvector(0, 4);
    beta = dvector(0, 4);
    tol = dvector(0, 4);
    rss = dvector(0, 4);
    vorder = ivector(0, 4);
    lindep = ivector(0, 4);

    y[0] = 65647.;
    y[1] =  70638.;
    y[2] =  75889.;
    y[3] =  81399.;
    y[4] =  87169.;
    y[5] =  93202.;
    y[6] =  99503.;
    y[7] = 106079.;
    y[8] =  112939.;
    y[9] =  120094.;
    y[10] =  127557.;
    for (i=0; i<5; i++) vorder[i] = i;

    printf("Fitting nasty cubic; 1st 4 betas should be 1\n\n");
    for (i=0; i<11; i++) {
        double tmp = i+40;
        x[i][0] = tmp;
        x[i][1] = tmp*tmp;
        x[i][2] = x[i][0] * x[i][1];
        x[i][3] = x[i][0] + x[i][1];
    }
    error = clear(5, 10, d, rbar, thetab, &sserr);
    if (error) printf("clear() returned %d\n", error);

    for (i=0; i<11; i++) {
        xrow[0] = 1;
        for (j=0; j<4; j++) xrow[j+1] = x[i][j];
        error = includ(5, 10, 1.0, xrow, y[i], d, rbar, thetab, &sserr);
        if (error) printf("includ() returned %d at row %d\n",
                          error, i);
    }

    error = tolset(5, 10, d, rbar, tol);
    if (error) printf("tolset() returned %d\n", error);
    error = sing(5, 10, d, rbar, thetab, &sserr, tol, lindep);
    if (error) printf("sing() returned %d\n", error);
    printf("SSerr = %17g, expect 286.\n", sserr);

    for (i=0; i<6; i++) {
        error = regcf(5, 10, d, rbar, thetab, tol, beta, 5);
        if (error) printf("regcf() returned %d\n", error);
        printf("\n\nVariable order: ");
        for (j=0; j<5; j++) printf("%d ", vorder[j]); printf("\n");
        putdvec("Beta", beta, 0, 4);
        putdvec("d", d, 0, 4);
        printf("rbar matrix:\n"); pr_utdm_v(rbar, 4, 14, 6);
        putdvec("thetab", thetab, 0, 4);

        /* rotate through 1 */
        error = ss(5, d, thetab, &sserr, rss);
        if (error) printf("ss() returned %d\n", error);
        error = vmove(5, 10, vorder, d, rbar, thetab, rss, 1, 5, tol);
        if (error) printf("vmove() returned %d\n", error);
    }

    return 0;
}
