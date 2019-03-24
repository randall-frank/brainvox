#include <stdio.h>
#include "as274c.h"
#include "misc.h"

int main()
{
    double *d, *rbar, *rinv, rho=.6, *covmat, var, *sterr;
    int pos=0, row, col, error;

    d = dvector(0, 6);
    rbar = dvector(0, 20);
    rinv = dvector(0, 14);
    covmat = dvector(0, 20);
    sterr = dvector(0, 5);

    for (row=0; row<6; row++) {
        d[row] = 1;
        for (col=row+1; col<7; col++)
          rbar[pos++] = col == row+1 ? rho : 0;
    }

    var = 1;
    error = cov(7, 21, d, rbar, 6, rinv, &var, covmat, 21, sterr);
    if (error) printf("cov() returned %d\n", error);
    printf("covmat is:\n");
    pr_utdm_v(covmat, 6, 13, 7);
    return 0;
}
