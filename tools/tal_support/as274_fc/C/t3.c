#include <math.h>
#include <stdio.h>
#include "as274c.h"
#include "misc.h"

int main()
{
    double *d, *rbar, *thetab, *x, y, sserr, *tol, *rss, *beta;
    int i, error, *vorder, *lindep;

    d = dvector(0, 6);
    rbar = dvector(0, 20);
    thetab = dvector(0, 6);
    x = dvector(0, 6);
    tol = dvector(0, 6);
    rss = dvector(0, 6);
    beta = dvector(0, 6);
    vorder = ivector(0, 6);
    lindep = ivector(0, 6);

    randc.ix = 777;
    randc.iy = 777;
    randc.iz = 777;

    error = clear(7, 21, d, rbar, thetab, &sserr);
    if (error) printf("clear() returned %d\n", error);
    for (i=0; i<7; i++) vorder[i] = i;
    for (i=0; i<12; i++) {
        x[0] = ranwm();
        x[2] = ranwm();
        x[3] = ranwm();
        x[4] = ranwm();
        x[6] = ranwm();
        x[1] = x[3] - x[4];
        x[5] = x[0] - x[2];
        y = x[0] + x[2] + x[3] + x[4] + x[6];
        error = includ(7, 21, 1, x, y, d, rbar, thetab, &sserr);
        if (error) printf("includ() returned %d\n", error);
    }

#define PRINTTHREE \
    printf("\n"); \
    putdvec("d", d, 0, 6); \
    pr_utdm_v(rbar, 6, 12, 6); \
    putdvec("thetab", thetab, 0, 6); \
    printf("\n")

    PRINTTHREE;

    error = tolset(7, 21, d, rbar, tol);
    if (error) printf("tolset() returned %d\n", error);
    error = ss(7, d, thetab, &sserr, rss);
    if (error) printf("ss() returned %d\n", error);
    putdvec("RSS", rss, 0, 6);

    error = sing(7, 21, d, rbar, thetab, &sserr, tol, lindep);
    if (error) printf("sing() returned %d\n", error);
    printf("After processing by sing():\n");
    PRINTTHREE;
    error = ss(7, d, thetab, &sserr, rss);
    if (error) printf("ss() returned %d\n", error);
    putdvec("RSS", rss, 0, 6);

    /* Swap rows 4 and 5, and 6 and 7 */
    error = vmove(7, 21, vorder, d, rbar, thetab, rss, 4, 5, tol);
    error = vmove(7, 21, vorder, d, rbar, thetab, rss, 6, 7, tol);
    PRINTTHREE;
    error = regcf(7, 21, d, rbar, thetab, tol, beta, 7);
    putdvec("Beta:", beta, 0, 6);
    error = ss(7, d, thetab, &sserr, rss);
    if (error) printf("ss() returned %d\n", error);
    putdvec("RSS", rss, 0, 6);
    return 0;
}
