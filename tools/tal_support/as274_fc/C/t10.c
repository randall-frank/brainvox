#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "misc.h"
#include "as274c.h"

int main(int argc, char **argv)
{
    int n, np, nrbar, b, block, row, col, nreq, pos, *lindep;
    int error;
    double eps, gamma, delta, *x, y, rhs_const,
       mult_factor, *d, *rbar, *thetab, sserr, weight,
       *tol, *work, *beta, sum, temp;

    x = dvector(0, 49);
    d = dvector(0, 49);
    rbar = dvector(0, 1224);
    thetab = dvector(0, 49);
    tol = dvector(0, 49);
    work = dvector(0, 49);
    beta = dvector(0, 49);
    lindep = ivector(0, 49);

    if (argc != 6) {
        printf("Usage: %s n b eps gamma delta\n", argv[0]);
        return 1;
    }
    n = atoi(argv[1]);
    np = n-1;
    nrbar = np*(np-1)/2;
    b = atoi(argv[2]);
    eps = atof(argv[3]);
    gamma = atof(argv[4]);
    delta = atof(argv[5]);

    error = clear(np, nrbar, d, rbar, thetab, &sserr);
    if (error) printf("error() returned %d\n", error);

    rhs_const = (n-2)*gamma + eps;
    mult_factor = 1.0 + eps/gamma;
    weight = 1.0;
    for (block=0; block<b; block++) {
        x[0] = 1;
        x[1] = (1+gamma)*delta;
        for (col=2; col<n-1; col++) x[col] = x[col-1] * delta;
        y = (n-1) + rhs_const;
        error = includ(np, nrbar, 1, x, y, d, rbar, thetab, &sserr);
        if (error) printf("includ() returned %d\n", error);

        for (row=1; row<n; row++) {
            x[0] = 1;
            x[1] = gamma*delta;
            for (col=2; col<n-1; col++) x[col] = x[col-1]*delta;
            if (row < n) x[row] *= mult_factor;
            y = rhs_const;
            if (row == n-1) y += n-1 - 2*eps;
            error = includ(np, nrbar, 1, x, y, d, rbar, thetab, &sserr);
            if (error) printf("includ() returned %d\n", error);
        }
    }

    error = tolset(np, nrbar, d, rbar, tol);
    if (error) printf("tolset() returned %d\n", error);
    error = regcf(np, nrbar, d, rbar, thetab, tol, beta, np);
    if (error) printf("regcf() returned %d\n", error);
    printf("Reg coeffs when sing() was not used:\n");
    putdvec("beta", beta, 0, np-1);

    /* Now call sing() to tidy up singularities */
    error = sing(np, nrbar, d, rbar, thetab, &sserr, tol, lindep);
    if (error) printf("sing() returned %d\n", error);
    nreq = np + error;
    error = regcf(np, nrbar, d, rbar, thetab, tol, beta, nreq);
    printf("After sing was used:\n");
    putdvec("beta", beta, 0, nreq-1);

    /* Show diagonal elements of cholesky factorisation */
    printf("Column   Col.length   Cholesky diagonal   Tolerance\n");
    --d; --rbar; --tol;
    for (col=1; col<=np; col++) {
        sum = d[col];
        temp = sqrt(sum);
        pos = col-1;
        for (row=1; row<=col-1; row++) {
            sum += d[row] * rbar[pos] * rbar[pos];
            pos += np - row - 1;
        }
        printf("%d %g %g %g\n", col, sqrt(sum), temp, tol[col]);
    }

    return 0;
}
