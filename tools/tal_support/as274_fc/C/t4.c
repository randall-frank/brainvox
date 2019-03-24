#include <stdio.h>
#include "as274c.h"
#include "misc.h"

int main()
{
    double *rbar, *rinv, rho=.6;
    int pos=0, row, col;

    rbar = dvector(0, 20);
    rinv = dvector(0, 14);

    for (row=0; row<6; row++)
      for (col=row+1; col<7; col++)
        rbar[pos++] = col == row+1 ? rho : 0;

    inv(7, 21, rbar, 6, rinv);
    pr_utdm_v(rinv, 5, 13, 7);
    return 0;
}
