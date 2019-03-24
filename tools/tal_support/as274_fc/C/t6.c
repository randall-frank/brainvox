#include <stdio.h>
#include "as274c.h"
#include "misc.h"

int main()
{
    double *d, *rbar, *thetab, sserr, **x, *y, *cormat, *ycorr;
    int error, i;

    x = dmatrix(0, 24, 0, 9); y = dvector(0, 24);
    d = dvector(0, 9);
    rbar = dvector(0, 44);
    thetab = dvector(0, 9);
    cormat = dvector(0, 35);
    ycorr = dvector(0, 9);

    error = clear(10, 45, d, rbar, thetab, &sserr);
    if (error) printf("clear() returned %d\n", error);
/* Grab data */
#include "steam.h"
    for (i=0; i<25; i++) {
        error = includ(10, 45, 1, x[i], y[i], d, rbar, thetab, &sserr);
        if (error) printf("includ() returned %d\n", error);
    }
    printf("SSerr = %.15g\n", sserr);
    error = pcorr(10, 45, d, rbar, thetab, &sserr, 1, cormat, 36, ycorr);
    if (error) printf("pcorr() returned %d\n", error);

    putdvec("ycorr", ycorr, 0, 9);
    pr_utdm_v(cormat, 8, 9, 4);
    return 0;
}
