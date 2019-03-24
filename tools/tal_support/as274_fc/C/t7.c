#include <stdio.h>
#include <math.h>
#include "as274c.h"
#include "misc.h"

int main()
{
    int error, row, seeded;
    const int np=11, nrbar=55, nobs=24;
    double **x, *y, *xrow, *d, *rbar, *tol, *hii, *thetab, sserr;
    double sum, third=1.0/3.0;

    x = dmatrix(0, 23, 0, 5);
    y = dvector(0, 23);
    xrow = dvector(0, 10);
    d = dvector(0, 10);
    rbar = dvector(0, 54);
    tol = dvector(0, 10);
    hii = dvector(0, 23);
    thetab = dvector(0, 10);
#include "woodley.h"            /* the data */
    error = clear(np, nrbar, d, rbar, thetab, &sserr);
    if (error) printf("clear() returned %d\n", error);

    xrow[0] = 1;
    for (row=0; row<nobs; row++) {
        xrow[2] = x[row][1];
        xrow[3] = x[row][2];
        xrow[4] = x[row][3];
        xrow[5] = pow(x[row][4], third);
        xrow[6] = x[row][5];
        seeded = x[row][0] == 1;
        if (seeded) {
            xrow[1] = 1;
            xrow[7] = x[row][2];
            xrow[8] = x[row][3];
            xrow[9] = xrow[5];
            xrow[10] = x[row][5];
        } else {
            xrow[1] = xrow[7] = xrow[8] = xrow[9] = xrow[10] = 0;
        }
        error = includ(np, nrbar, 1, xrow, y[row], d, rbar, thetab, &sserr);
        if (error) printf("At row %d, includ() returned %d\n",
                          row, error);
    }

    error = tolset(np, nrbar, d, rbar, tol);
    if (error) printf("tolset() returned %d\n", error);
    for (sum = 0, row=0; row<nobs; row++) {
        xrow[2] = x[row][1];
        xrow[3] = x[row][2];
        xrow[4] = x[row][3];
        xrow[5] = pow(x[row][4], third);
        xrow[6] = x[row][5];
        seeded = x[row][0] == 1;
        if (seeded) {
            xrow[1] = 1;
            xrow[7] = x[row][2];
            xrow[8] = x[row][3];
            xrow[9] = xrow[5];
            xrow[10] = x[row][5];
        } else {
            xrow[1] = xrow[7] = xrow[8] = xrow[9] = xrow[10] = 0;
        }
        error = hdiag(xrow, np, nrbar, d, rbar, tol, np, hii+row);
        sum += hii[row];
    }

    printf("Diagonal elements of Hat matrix:\n");
    putdvec("hii", hii, 0, 23);
    printf("Sum of diagonal elements = %g\n", sum);
    return 0;
}
