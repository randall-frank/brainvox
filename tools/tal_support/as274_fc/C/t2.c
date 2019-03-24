#include <stdio.h>
#include "as274c.h"
#include "misc.h"

int main()
{
    double **x, *y, *d, *rbar, *thetab, sserr, *beta, *tol, *rss;
    int i, error, *vorder, *lindep;

    vorder = ivector(0, 6); lindep = ivector(0, 6);
    x = dmatrix(0, 15, 0, 6);
    y = dvector(0, 15);
    d = dvector(0, 6);
    rbar = dvector(0, 20);
    thetab = dvector(0, 6);
    beta = dvector(0, 6);
    tol = dvector(0, 6);
    rss = dvector(0, 6);

    for (i=0; i<7; i++) vorder[i] = i;
    printf("Using the famous Longley data:\n");
#include "longley.h"

    error = clear(7, 21, d, rbar, thetab, &sserr);
    if (error) printf("clear() returned %d\n", error);
    for (i=0; i<16; i++) {
        error = includ(7, 21, 1, x[i], y[i], d, rbar, thetab, &sserr);
        if (error) printf("includ() returned %d\n", error);
    }

    error = tolset(7, 21, d, rbar, tol);
    if (error) printf("tolset() returned %d\n", error);
    error = sing(7, 21, d, rbar, thetab, &sserr, tol, lindep);
    if (error) printf("sing() returned %d\n", error);
    printf("sserr = %.15g\n", sserr);
    error = ss(7, d, thetab, &sserr, rss);
    if (error) printf("ss() returned %d\n", error);

    /* Now make reg coeffs, using vmove() to cycle through ordering */
    for (i=0; i<7; i++) {
        printf("\nStarting cycle %d\n", i);
        error = regcf(7, 21, d, rbar, thetab, tol, beta, 7);
        putdvec("beta", beta, 0, 6);
        putdvec("d", d, 0, 6);
        pr_utdm_v(rbar, 6, 12, 5);
        putdvec("thetab", thetab, 0, 6);

        error = vmove(7, 21, vorder, d, rbar, thetab, rss, 2, 7, tol);
        if (error) printf("vmove() returned %d\n", error);
    }
    return 0;
}
