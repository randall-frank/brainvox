#include <stdio.h>
#include "as274c.h"
#include "misc.h"

int main()
{
    double *xrow, y, *d, *rbar, *thetab, sserr, *tol, *work, *beta;
    double pt1=.1, eps;
    int error, k, i, j, *lindep;

    lindep = ivector(0, 8);
    xrow = dvector(0, 8);
    d = dvector(0, 8);
    rbar = dvector(0, 35);
    thetab = dvector(0, 8);
    tol = dvector(0, 8);
    work = dvector(0, 8);
    beta = dvector(0, 8);

    eps = 1e-5;
    for (k=0; k<5; k++) {
        error = clear(9, 36, d, rbar, thetab, &sserr);
        for (j=0; j<9; j++) xrow[j] = 1;
        y = 9 + eps;
        error = includ(9, 36, 1, xrow, y, d, rbar, thetab, &sserr);
        y = eps;
        for (i=1; i<10; i++) {
            for (j=1; j<9; j++) xrow[j] = 0;
            xrow[0] = 1;
            if (i < 9) xrow[i] = eps;
            error = includ(9, 36, 1, xrow, y, d, rbar, thetab, &sserr);
            if (i == 8) y = 9 - eps;
        }

        /* set tol and look for singularities */
        error = tolset(9, 36, d, rbar, tol);
        error = sing(9, 36, d, rbar, thetab, &sserr, tol, lindep);
        printf("eps = %g\n", eps);

        if (error) printf("Error = %d\n", error);
        else {
            error = regcf(9, 36, d, rbar, thetab, tol, beta, 9);
            for (i=0; i<9; i++) work[i] = beta[i] - 1;
            printf("Errors in regression coefficients:\n");
            putdvec("work ", work, 0, 8);
        }

        eps *= pt1;
    }
    return 0;
}
