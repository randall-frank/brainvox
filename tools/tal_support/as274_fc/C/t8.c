#include <math.h>
#include <stdio.h>
#include "as274c.h"
#include "misc.h"

int main()
{
    const int np=8, nrbar=28;
    int error, icase, i, *vorder, *list, *lindep, new;
    double *d, *rbar, *thetab, *rss, *tol, sserr, *x, y, *beta;

    vorder = ivector(0, np-1);
    list = ivector(0, np-1);
    d = dvector(0, np-1);
    rbar = dvector(0, nrbar-1);
    thetab = dvector(0, np-1);
    rss = dvector(0, np-1);
    tol = dvector(0, np-1);
    x = dvector(0, np-1);
    beta = dvector(0, np-1);
    lindep = ivector(0, np-1);
    
    randc.ix = randc.iy = randc.iz = 777;
    error = clear(np, nrbar, d, rbar, thetab, &sserr);
    if (error) printf("clear() returned %d\n", error);

    for (icase = 0; icase<5; icase++) {
        for (i=0; i<4; i++) x[i] = ranwm();
        x[4] = x[0] + x[1];
        x[5] = x[1] + x[2];
        x[6] = x[2] + x[3];
        x[7] = x[3] + x[0];
        y = x[4] + x[6];
        error = includ(np, nrbar, 1, x, y, d, rbar, thetab, &sserr);
        if (error) printf("At row %d, includ() returned %d\n", icase, error);
    }

    /* Now look at factorisation */
    putdvec("D", d, 0, 7);
    pr_utdm_v(rbar, 7, 11, 3);

    error = tolset(np, nrbar, d, rbar, tol);
    if (error) printf("tolset() returned %d\n", error);
    error = sing(np, nrbar, d, rbar, thetab, &sserr, tol, lindep);
    if (error) printf("sing() returned %d\n", error);
    printf("lindep vector is: ");
    for (i=0; i<np; i++) printf("%d ", lindep[i]); printf("\n");

    error = ss(np, d, thetab, &sserr, rss);
    if (error) printf("ss() returned %d\n", error);
    error = regcf(np, nrbar, d, rbar, thetab, tol, beta, 4);
    printf("betas: %f %f %f %f\n", beta[0], beta[1], beta[2], beta[3]);

    /* reorder vars as x5..x8, x1..x4.  Note xt+x7 == x6+x8 */
    new = 4;
    for (i=0; i<8; i++) {
        vorder[i] = i;
        list[i] = new;
        new = i == 3 ? 0 : new+1;
    }
    error = reordr(np, nrbar, vorder, d, rbar, thetab, rss, tol, list, 4, 1);
    if (error) printf("reordr() returned %d\n", error);

    /* Look at factorisation again: */
    putdvec("D", d, 0, 7);
    pr_utdm_v(rbar, 7, 11, 3);
    error = sing(np, nrbar, d, rbar, thetab, &sserr, tol, lindep);
    if (error) printf("sing() returned %d\n", error);
    printf("lindep vector is: ");
    for (i=0; i<np; i++) printf("%d ", lindep[i]); printf("\n");

    error = regcf(np, nrbar, d, rbar, thetab, tol, beta, 4);
    printf("betas: %f %f %f %f\n", beta[0], beta[1], beta[2], beta[3]);

    return 0;
}
