/* 75_base.f -- translated by f2c (version 19970805).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)
*/

#include "f2c.h"


/* Subroutine */ int confnd_(integer *np, integer *nrbar, integer *j, 
	doublereal *rbar, doublereal *contra, integer *ifault)
{
    /* Initialized data */

    static doublereal zero = 0.;
    static doublereal one = 1.;

    /* System generated locals */
    integer i__1, i__2;

    /* Local variables */
    static integer i__, k, nextr, ij, jm, ip, jp;


/*     Algorithm AS 75.2 Appl. Statist. (1974) Vol. 23, No. 3, P448 */

/*     Calling this subroutine obtains the contrast which could not */
/*     be estimated if D(j) were assumed to be zero, that is, obtains the 
*/
/*     linear combination of the first j columns which would be zero.  Th 
*/
/*     obtained by setting the first j-1 elements of contra to the soluti 
*/
/*     of the triangular system formed by the first j-1 rows and columns 
*/
/*     rbar with the first j-1 elements of the jth column as right hand */
/*     side, setting the jth element of contra to -1, and setting the */
/*     remaining elements of contra to zero. */


    /* Parameter adjustments */
    --contra;
    --rbar;

    /* Function Body */

/*         check input parameters */

    *ifault = 1;
    if (*np < 1 || *nrbar <= *np * (*np - 1) / 2) {
	return 0;
    }
    *ifault = 0;

    jm = *j - 1;
    if (*j == *np) {
	goto L20;
    }
    jp = *j + 1;
    i__1 = *np;
    for (i__ = jp; i__ <= i__1; ++i__) {
/* L10: */
	contra[i__] = zero;
    }
L20:
    contra[*j] = -one;
    if (*j == 1) {
	return 0;
    }
    i__1 = jm;
    for (ij = 1; ij <= i__1; ++ij) {
	i__ = *j - ij;
	nextr = (i__ - 1) * (*np + *np - i__) / 2 + 1;
	k = nextr + *j - i__ - 1;
	contra[i__] = rbar[k];
	if (i__ == jm) {
	    goto L40;
	}
	ip = i__ + 1;
	i__2 = jm;
	for (k = ip; k <= i__2; ++k) {
	    contra[i__] -= rbar[nextr] * contra[k];
	    ++nextr;
/* L30: */
	}
L40:
	;
    }
    return 0;
} /* confnd_ */



/* Subroutine */ int ssdcmp_(integer *np, doublereal *d__, doublereal *thetab,
	 doublereal *ss, integer *ifault)
{
    /* System generated locals */
    integer i__1;
    doublereal d__1;

    /* Local variables */
    static integer i__;


/*     Algorithm AS75.3  Appl. Statist. (1974) Vol.23, No. 3, P448 */

/*     Calling this subroutine computes the np components of the sum */
/*     of squares decomposition from D and thetab. */


/*        check input parameters */

    /* Parameter adjustments */
    --ss;
    --thetab;
    --d__;

    /* Function Body */
    *ifault = 1;
    if (*np < 1) {
	return 0;
    }
    *ifault = 0;

    i__1 = *np;
    for (i__ = 1; i__ <= i__1; ++i__) {
/* L10: */
/* Computing 2nd power */
	d__1 = thetab[i__];
	ss[i__] = d__[i__] * (d__1 * d__1);
    }
    return 0;
} /* ssdcmp_ */



/* Subroutine */ int regrss_(integer *np, integer *nrbar, doublereal *rbar, 
	doublereal *thetab, doublereal *beta, integer *ifault)
{
    /* System generated locals */
    integer i__1, i__2;

    /* Local variables */
    static integer i__, j, k, nextr, ip;


/*     Algorithm AS 75.4  Appl. Statist. (1974), Vol. 23, No. 3, p448 */

/*     Calling this subroutine obtains beta by back-substitution in */
/*     the triangular system rbar and thetab. */


/*        check input parameters */

    /* Parameter adjustments */
    --beta;
    --thetab;
    --rbar;

    /* Function Body */
    *ifault = 1;
    if (*np < 1 || *nrbar <= *np * (*np - 1) / 2) {
	return 0;
    }
    *ifault = 0;

    i__1 = *np;
    for (j = 1; j <= i__1; ++j) {
	i__ = *np - j + 1;
	beta[i__] = thetab[i__];
	nextr = (i__ - 1) * (*np + *np - i__) / 2 + 1;
	ip = i__ + 1;
	i__2 = *np;
	for (k = ip; k <= i__2; ++k) {
	    beta[i__] -= rbar[nextr] * beta[k];
	    ++nextr;
/* L10: */
	}
/* L20: */
    }
    return 0;
} /* regrss_ */

