/* 274.f -- translated by f2c (version 19990326).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)
*/

#include "f2c.h"

/* ----------------------------------------------------------------------- */
/* Subroutine */ int remove_(np, nrbar, weight, xrow, yelem, d__, rbar, 
	thetab, sserr, ier)
integer *np, *nrbar;
doublereal *weight, *xrow, *yelem, *d__, *rbar, *thetab, *sserr;
integer *ier;
{
    /* Initialized data */

    static doublereal zero = 0.;

    /* System generated locals */
    integer i__1, i__2;

    /* Local variables */
    static doublereal cbar, sbar;
    static integer i__, k;
    static doublereal w, y;
    static integer nextr;
    static doublereal di, xi, xk, dpi, wxi;

/*     *** WARNING ***   The elements of XROW are over-written. */


/*     Local variables */


    /* Parameter adjustments */
    --thetab;
    --d__;
    --xrow;
    --rbar;

    /* Function Body */

/*     Some checks. */

    *ier = 0;
    if (*np < 1) {
	*ier = 1;
    }
    if (*nrbar < *np * (*np - 1) / 2) {
	*ier += 2;
    }
    if (*ier != 0) {
	return 0;
    }

    w = *weight;
    y = *yelem;
    nextr = 1;
    if (w == zero) {
	return 0;
    }
    i__1 = *np;
    for (i__ = 1; i__ <= i__1; ++i__) {
	xi = xrow[i__];
	dpi = d__[i__];
	wxi = w * xi;
	d__[i__] -= wxi * xi;
	di = d__[i__];
	cbar = di / dpi;
	sbar = wxi / dpi;
	w = cbar * w;
	if (i__ == *np) {
	    goto L20;
	}
	i__2 = *np;
	for (k = i__ + 1; k <= i__2; ++k) {
	    xk = xrow[k];
	    rbar[nextr] = (rbar[nextr] - sbar * xk) / cbar;
	    xrow[k] = xk - xi * rbar[nextr];
	    ++nextr;
/* L10: */
	}
L20:
	xk = y;
	thetab[i__] = (thetab[i__] - sbar * xk) / cbar;
	y = xk - xi * thetab[i__];
/* L30: */
    }

    *sserr -= w * y * y;

    return 0;
} /* remove_ */

/* ----------------------------------------------------------------------- */
/* Subroutine */ int includ_(np, nrbar, weight, xrow, yelem, d__, rbar, 
	thetab, sserr, ier)
integer *np, *nrbar;
doublereal *weight, *xrow, *yelem, *d__, *rbar, *thetab, *sserr;
integer *ier;
{
    /* Initialized data */

    static doublereal zero = 0.;

    /* System generated locals */
    integer i__1, i__2;

    /* Local variables */
    static doublereal cbar, sbar;
    static integer i__, k;
    static doublereal w, y;
    static integer nextr;
    static doublereal di, xi, xk, dpi, wxi;


/*     ALGORITHM AS274.1  APPL. STATIST. (1992) VOL 41, NO. 2 */

/*     DOUBLE PRECISION VERSION */

/*     Calling this routine updates d, rbar, thetab and sserr by the */
/*     inclusion of xrow, yelem with the specified weight. */
/*     This version has been modified to make it slightly faster when the */
/*     early elements of XROW are not zeroes. */

/*     *** WARNING ***   The elements of XROW are over-written. */


/*     Local variables */


    /* Parameter adjustments */
    --thetab;
    --d__;
    --xrow;
    --rbar;

    /* Function Body */

/*     Some checks. */

    *ier = 0;
    if (*np < 1) {
	*ier = 1;
    }
    if (*nrbar < *np * (*np - 1) / 2) {
	*ier += 2;
    }
    if (*ier != 0) {
	return 0;
    }

    w = *weight;
    y = *yelem;
    nextr = 1;
    i__1 = *np;
    for (i__ = 1; i__ <= i__1; ++i__) {

/*     Skip unnecessary transformations.   Test on exact zeroes must be */
/*     used or stability can be destroyed. */

	if (w == zero) {
	    return 0;
	}
	xi = xrow[i__];
	if (xi == zero) {
	    nextr = nextr + *np - i__;
	    goto L30;
	}
	di = d__[i__];
	wxi = w * xi;
	dpi = di + wxi * xi;
	cbar = di / dpi;
	sbar = wxi / dpi;
	w = cbar * w;
	d__[i__] = dpi;
	if (i__ == *np) {
	    goto L20;
	}
	i__2 = *np;
	for (k = i__ + 1; k <= i__2; ++k) {
	    xk = xrow[k];
	    xrow[k] = xk - xi * rbar[nextr];
	    rbar[nextr] = cbar * rbar[nextr] + sbar * xk;
	    ++nextr;
/* L10: */
	}
L20:
	xk = y;
	y = xk - xi * thetab[i__];
	thetab[i__] = cbar * thetab[i__] + sbar * xk;
L30:
	;
    }

/*     Y * SQRT(W) is now equal to the Brown, Durbin & Evans recursive */
/*     residual. */

    *sserr += w * y * y;

    return 0;
} /* includ_ */


/* Subroutine */ int clear_(np, nrbar, d__, rbar, thetab, sserr, ier)
integer *np, *nrbar;
doublereal *d__, *rbar, *thetab, *sserr;
integer *ier;
{
    /* Initialized data */

    static doublereal zero = 0.;

    /* System generated locals */
    integer i__1;

    /* Local variables */
    static integer i__;


/*     ALGORITHM AS274.2  APPL. STATIST. (1992) VOL.41, NO.2 */

/*     Sets arrays to zero prior to calling AS75.1 */


/*     Local variables */


    /* Parameter adjustments */
    --thetab;
    --d__;
    --rbar;

    /* Function Body */

/*     Some checks. */

    *ier = 0;
    if (*np < 1) {
	*ier = 1;
    }
    if (*nrbar < *np * (*np - 1) / 2) {
	*ier += 2;
    }
    if (*ier != 0) {
	return 0;
    }

    i__1 = *np;
    for (i__ = 1; i__ <= i__1; ++i__) {
	d__[i__] = zero;
	thetab[i__] = zero;
/* L10: */
    }
    i__1 = *nrbar;
    for (i__ = 1; i__ <= i__1; ++i__) {
/* L20: */
	rbar[i__] = zero;
    }
    *sserr = zero;
    return 0;
} /* clear_ */


/* Subroutine */ int regcf_(np, nrbar, d__, rbar, thetab, tol, beta, nreq, 
	ier)
integer *np, *nrbar;
doublereal *d__, *rbar, *thetab, *tol, *beta;
integer *nreq, *ier;
{
    /* Initialized data */

    static doublereal zero = 0.;

    /* System generated locals */
    integer i__1;

    /* Builtin functions */
    double sqrt();

    /* Local variables */
    static integer i__, j, nextr;


/*     ALGORITHM AS274.3  APPL. STATIST. (1992) VOL 41, NO.2 */

/*     Modified version of AS75.4 to calculate regression coefficients */
/*     for the first NREQ variables, given an orthogonal reduction from */
/*     AS75.1. */


/*     Local variables */


    /* Parameter adjustments */
    --beta;
    --tol;
    --thetab;
    --d__;
    --rbar;

    /* Function Body */

/*     Some checks. */

    *ier = 0;
    if (*np < 1) {
	*ier = 1;
    }
    if (*nrbar < *np * (*np - 1) / 2) {
	*ier += 2;
    }
    if (*nreq < 1 || *nreq > *np) {
	*ier += 4;
    }
    if (*ier != 0) {
	return 0;
    }

    for (i__ = *nreq; i__ >= 1; --i__) {
	if (sqrt(d__[i__]) < tol[i__]) {
	    beta[i__] = zero;
	    d__[i__] = zero;
	    goto L20;
	}
	beta[i__] = thetab[i__];
	nextr = (i__ - 1) * (*np + *np - i__) / 2 + 1;
	i__1 = *nreq;
	for (j = i__ + 1; j <= i__1; ++j) {
	    beta[i__] -= rbar[nextr] * beta[j];
	    ++nextr;
/* L10: */
	}
L20:
	;
    }

    return 0;
} /* regcf_ */


/* Subroutine */ int tolset_(np, nrbar, d__, rbar, tol, work, ier)
integer *np, *nrbar;
doublereal *d__, *rbar, *tol, *work;
integer *ier;
{
    /* Initialized data */

    static doublereal eps = 1e-15;

    /* System generated locals */
    integer i__1, i__2;
    doublereal d__1;

    /* Builtin functions */
    double sqrt();

    /* Local variables */
    static integer col, pos;
    static doublereal sum;
    static integer row;


/*     ALGORITHM AS274.4  APPL. STATIST. (1992) VOL 41, NO.2 */

/*     Sets up array TOL for testing for zeroes in an orthogonal */
/*     reduction formed using AS75.1. */


/*     Local variables. */


/*     EPS is a machine-dependent constant.   For compilers which use */
/*     the IEEE format for floating-point numbers, recommended values */
/*     are 1.E-06 for single precision and 1.E-15 for double precision. */

    /* Parameter adjustments */
    --work;
    --tol;
    --d__;
    --rbar;

    /* Function Body */

/*     Some checks. */

    *ier = 0;
    if (*np < 1) {
	*ier = 1;
    }
    if (*nrbar < *np * (*np - 1) / 2) {
	*ier += 2;
    }
    if (*ier != 0) {
	return 0;
    }

/*     Set TOL(I) = sum of absolute values in column I of RBAR after */
/*     scaling each element by the square root of its row multiplier. */

    i__1 = *np;
    for (col = 1; col <= i__1; ++col) {
/* L10: */
	work[col] = sqrt(d__[col]);
    }
    i__1 = *np;
    for (col = 1; col <= i__1; ++col) {
	pos = col - 1;
	sum = work[col];
	i__2 = col - 1;
	for (row = 1; row <= i__2; ++row) {
	    sum += (d__1 = rbar[pos], abs(d__1)) * work[row];
	    pos = pos + *np - row - 1;
/* L20: */
	}
	tol[col] = eps * sum;
/* L30: */
    }

    return 0;
} /* tolset_ */


/* Subroutine */ int sing_(np, nrbar, d__, rbar, thetab, sserr, tol, lindep, 
	work, ier)
integer *np, *nrbar;
doublereal *d__, *rbar, *thetab, *sserr, *tol;
logical *lindep;
doublereal *work;
integer *ier;
{
    /* Initialized data */

    static doublereal zero = 0.;

    /* System generated locals */
    integer i__1, i__2;
    doublereal d__1;

    /* Builtin functions */
    double sqrt();

    /* Local variables */
    static doublereal temp;
    extern /* Subroutine */ int includ_();
    static integer np2, col, pos, row, pos2;


/*     ALGORITHM AS274.5  APPL. STATIST. (1992) VOL 41, NO.2 */

/*     Checks for singularities, reports, and adjusts orthogonal */
/*     reductions produced by AS75.1. */


/*     Local variables */


    /* Parameter adjustments */
    --work;
    --lindep;
    --tol;
    --thetab;
    --d__;
    --rbar;

    /* Function Body */

/*     Check input parameters */

    *ier = 0;
    if (*np <= 0) {
	*ier = 1;
    }
    if (*nrbar < *np * (*np - 1) / 2) {
	*ier += 2;
    }
    if (*ier != 0) {
	return 0;
    }

    i__1 = *np;
    for (col = 1; col <= i__1; ++col) {
/* L10: */
	work[col] = sqrt(d__[col]);
    }

    i__1 = *np;
    for (col = 1; col <= i__1; ++col) {

/*     Set elements within RBAR to zero if they are less than TOL(COL) in */
/*     absolute value after being scaled by the square root of their row */
/*     multiplier. */

	temp = tol[col];
	pos = col - 1;
	i__2 = col - 1;
	for (row = 1; row <= i__2; ++row) {
	    if ((d__1 = rbar[pos], abs(d__1)) * work[row] < temp) {
		rbar[pos] = zero;
	    }
	    pos = pos + *np - row - 1;
/* L30: */
	}

/*     If diagonal element is near zero, set it to zero, set appropriate */
/*     element of LINDEP, and use INCLUD to augment the projections in */
/*     the lower rows of the orthogonalization. */

	lindep[col] = FALSE_;
	if (work[col] < temp) {
	    lindep[col] = TRUE_;
	    --(*ier);
	    if (col < *np) {
		np2 = *np - col;
		pos2 = pos + *np - col + 1;
		i__2 = np2 * (np2 - 1) / 2;
		includ_(&np2, &i__2, &d__[col], &rbar[pos + 1], &thetab[col], 
			&d__[col + 1], &rbar[pos2], &thetab[col + 1], sserr, 
			ier);
	    } else {
/* Computing 2nd power */
		d__1 = thetab[col];
		*sserr += d__[col] * (d__1 * d__1);
	    }
	    d__[col] = zero;
	    work[col] = zero;
	    thetab[col] = zero;
	}
/* L40: */
    }
    return 0;
} /* sing_ */


/* Subroutine */ int ss_(np, d__, thetab, sserr, rss, ier)
integer *np;
doublereal *d__, *thetab, *sserr, *rss;
integer *ier;
{
    /* System generated locals */
    doublereal d__1;

    /* Local variables */
    static integer i__;
    static doublereal sum;


/*     ALGORITHM AS274.6  APPL. STATIST. (1992) VOL 41, NO.2 */

/*     Calculates partial residual sums of squares from an orthogonal */
/*     reduction from AS75.1. */


/*     Local variables */


/*     Some checks. */

    /* Parameter adjustments */
    --rss;
    --thetab;
    --d__;

    /* Function Body */
    *ier = 0;
    if (*np < 1) {
	*ier = 1;
    }
    if (*ier != 0) {
	return 0;
    }

    sum = *sserr;
    rss[*np] = *sserr;
    for (i__ = *np; i__ >= 2; --i__) {
/* Computing 2nd power */
	d__1 = thetab[i__];
	sum += d__[i__] * (d__1 * d__1);
	rss[i__ - 1] = sum;
/* L10: */
    }
    return 0;
} /* ss_ */


/* Subroutine */ int cov_(np, nrbar, d__, rbar, nreq, rinv, var, covmat, 
	dimcov, sterr, ier)
integer *np, *nrbar;
doublereal *d__, *rbar;
integer *nreq;
doublereal *rinv, *var, *covmat;
integer *dimcov;
doublereal *sterr;
integer *ier;
{
    /* Initialized data */

    static doublereal zero = 0.;
    static doublereal one = 1.;

    /* System generated locals */
    integer i__1, i__2, i__3;

    /* Builtin functions */
    double sqrt();

    /* Local variables */
    static integer k, start, col;
    extern /* Subroutine */ int inv_();
    static integer pos;
    static doublereal sum;
    static integer row, pos1, pos2;


/*     ALGORITHM AS274.7  APPL. STATIST. (1992) VOL 41, NO.2 */

/*     Calculate covariance matrix for regression coefficients for the */
/*     first NREQ variables, from an orthogonal reduction produced from */
/*     AS75.1. */

/*     Auxiliary routine called: INV */


/*     Local variables. */


    /* Parameter adjustments */
    --sterr;
    --d__;
    --rbar;
    --rinv;
    --covmat;

    /* Function Body */

/*     Some checks. */

    *ier = 0;
    if (*np < 1) {
	*ier = 1;
    }
    if (*nrbar < *np * (*np - 1) / 2) {
	*ier += 2;
    }
    if (*dimcov < *nreq * (*nreq + 1) / 2) {
	*ier += 4;
    }
    i__1 = *nreq;
    for (row = 1; row <= i__1; ++row) {
	if (d__[row] == zero) {
	    *ier = -row;
	}
/* L10: */
    }
    if (*ier != 0) {
	return 0;
    }

    inv_(np, nrbar, &rbar[1], nreq, &rinv[1]);
    pos = 1;
    start = 1;
    i__1 = *nreq;
    for (row = 1; row <= i__1; ++row) {
	pos2 = start;
	i__2 = *nreq;
	for (col = row; col <= i__2; ++col) {
	    pos1 = start + col - row;
	    if (row == col) {
		sum = one / d__[col];
	    } else {
		sum = rinv[pos1 - 1] / d__[col];
	    }
	    i__3 = *nreq;
	    for (k = col + 1; k <= i__3; ++k) {
		sum += rinv[pos1] * rinv[pos2] / d__[k];
		++pos1;
		++pos2;
/* L20: */
	    }
	    covmat[pos] = sum * *var;
	    if (row == col) {
		sterr[row] = sqrt(covmat[pos]);
	    }
	    ++pos;
/* L30: */
	}
	start = start + *nreq - row;
/* L40: */
    }

    return 0;
} /* cov_ */


/* Subroutine */ int inv_(np, nrbar, rbar, nreq, rinv)
integer *np, *nrbar;
doublereal *rbar;
integer *nreq;
doublereal *rinv;
{
    /* Initialized data */

    static doublereal zero = 0.;

    /* System generated locals */
    integer i__1, i__2;

    /* Local variables */
    static integer k, start, col, pos;
    static doublereal sum;
    static integer row, pos1, pos2;


/*     ALGORITHM AS274.8  APPL. STATIST. (1992) VOL 41, NO.2 */

/*     Invert first NREQ rows and columns of Cholesky factorization */
/*     produced by AS75.1. */


/*     Local variables. */


    /* Parameter adjustments */
    --rinv;
    --rbar;

    /* Function Body */

/*     Invert RBAR ignoring row multipliers, from the bottom up. */

    pos = *nreq * (*nreq - 1) / 2;
    for (row = *nreq - 1; row >= 1; --row) {
	start = (row - 1) * (*np + *np - row) / 2 + 1;
	i__1 = row + 1;
	for (col = *nreq; col >= i__1; --col) {
	    pos1 = start;
	    pos2 = pos;
	    sum = zero;
	    i__2 = col - 1;
	    for (k = row + 1; k <= i__2; ++k) {
		pos2 = pos2 + *nreq - k;
		sum -= rbar[pos1] * rinv[pos2];
		++pos1;
/* L10: */
	    }
	    rinv[pos] = sum - rbar[pos1];
	    --pos;
/* L20: */
	}
/* L30: */
    }

    return 0;
} /* inv_ */


/* Subroutine */ int pcorr_(np, nrbar, d__, rbar, thetab, sserr, in, work, 
	cormat, dimc, ycorr, ier)
integer *np, *nrbar;
doublereal *d__, *rbar, *thetab, *sserr;
integer *in;
doublereal *work, *cormat;
integer *dimc;
doublereal *ycorr;
integer *ier;
{
    /* Initialized data */

    static doublereal zero = 0.;

    /* System generated locals */
    integer i__1;

    /* Local variables */
    static integer i__, start, in1;
    extern /* Subroutine */ int cor_();


/*     ALGORITHM AS274.9  APPL. STATIST. (1992) VOL 41, NO.2 */

/*     Calculate partial correlations after the first IN variables */
/*     have been forced into the regression. */

/*     Auxiliary routine called: COR */


/*     Local variables. */


    /* Parameter adjustments */
    --work;
    --thetab;
    --d__;
    --rbar;
    --cormat;

    /* Function Body */

/*     Some checks. */

    *ier = 0;
    if (*np < 1) {
	*ier = 1;
    }
    if (*nrbar < *np * (*np - 1) / 2) {
	*ier += 2;
    }
    if (*in < 0 || *in > *np - 1) {
	*ier += 4;
    }
    if (*dimc < (*np - *in) * (*np - *in - 1) / 2) {
	*ier += 8;
    }
    if (*ier != 0) {
	return 0;
    }

    start = *in * (*np + *np - *in - 1) / 2 + 1;
    in1 = *in + 1;
    i__1 = *np - *in;
    cor_(&i__1, &d__[in1], &rbar[start], &thetab[in1], sserr, &work[1], &
	    cormat[1], ycorr);

/*     Check for zeroes. */

    i__1 = *np - *in;
    for (i__ = 1; i__ <= i__1; ++i__) {
	if (work[i__] <= zero) {
	    *ier = -i__;
	}
/* L10: */
    }

    return 0;
} /* pcorr_ */


/* Subroutine */ int cor_(np, d__, rbar, thetab, sserr, work, cormat, ycorr)
integer *np;
doublereal *d__, *rbar, *thetab, *sserr, *work, *cormat, *ycorr;
{
    /* Initialized data */

    static doublereal zero = 0.;

    /* System generated locals */
    integer i__1, i__2;
    doublereal d__1;

    /* Builtin functions */
    double sqrt();

    /* Local variables */
    static integer diff;
    static doublereal sumy;
    static integer pos;
    static doublereal sum;
    static integer row, col1, col2, pos1, pos2;


/*     ALGORITHM AS274.10  APPL. STATIST. (1992) VOL 41, NO.2 */

/*     Calculate correlations from an orthogonal reduction.   This */
/*     routine will usually be called from PCORR, which will have */
/*     removed the appropriate number of rows at the start. */


/*     Local variables. */


    /* Parameter adjustments */
    --ycorr;
    --work;
    --thetab;
    --d__;
    --rbar;
    --cormat;

    /* Function Body */

/*     Process by columns, including the projections of the dependent */
/*     variable (THETAB). */

    sumy = *sserr;
    i__1 = *np;
    for (row = 1; row <= i__1; ++row) {
/* L10: */
/* Computing 2nd power */
	d__1 = thetab[row];
	sumy += d__[row] * (d__1 * d__1);
    }
    sumy = sqrt(sumy);
    pos = *np * (*np - 1) / 2;
    for (col1 = *np; col1 >= 1; --col1) {

/*     Calculate the length of column COL1. */

	sum = d__[col1];
	pos1 = col1 - 1;
	i__1 = col1 - 1;
	for (row = 1; row <= i__1; ++row) {
/* Computing 2nd power */
	    d__1 = rbar[pos1];
	    sum += d__[row] * (d__1 * d__1);
	    pos1 = pos1 + *np - row - 1;
/* L20: */
	}
	work[col1] = sqrt(sum);

/*     If SUM = 0, set all correlations with this variable to zero. */

	if (sum == zero) {
	    ycorr[col1] = zero;
	    i__1 = col1 + 1;
	    for (col2 = *np; col2 >= i__1; --col2) {
		cormat[pos] = zero;
		--pos;
/* L30: */
	    }
	    goto L70;
	}

/*     Form cross-products, then divide by product of column lengths. */

	sum = d__[col1] * thetab[col1];
	pos1 = col1 - 1;
	i__1 = col1 - 1;
	for (row = 1; row <= i__1; ++row) {
	    sum += d__[row] * rbar[pos1] * thetab[row];
	    pos1 = pos1 + *np - row - 1;
/* L40: */
	}
	ycorr[col1] = sum / (sumy * work[col1]);

	i__1 = col1 + 1;
	for (col2 = *np; col2 >= i__1; --col2) {
	    if (work[col2] > zero) {
		pos1 = col1 - 1;
		pos2 = col2 - 1;
		diff = col2 - col1;
		sum = zero;
		i__2 = col1 - 1;
		for (row = 1; row <= i__2; ++row) {
		    sum += d__[row] * rbar[pos1] * rbar[pos2];
		    pos1 = pos1 + *np - row - 1;
		    pos2 = pos1 + diff;
/* L50: */
		}
		sum += d__[col1] * rbar[pos2];
		cormat[pos] = sum / (work[col1] * work[col2]);
	    } else {
		cormat[pos] = zero;
	    }
	    --pos;
/* L60: */
	}
L70:
	;
    }

    return 0;
} /* cor_ */


/* Subroutine */ int vmove_(np, nrbar, vorder, d__, rbar, thetab, rss, from, 
	to, tol, ier)
integer *np, *nrbar, *vorder;
doublereal *d__, *rbar, *thetab, *rss;
integer *from, *to;
doublereal *tol;
integer *ier;
{
    /* Initialized data */

    static doublereal zero = 0.;
    static doublereal one = 1.;

    /* System generated locals */
    integer i__1, i__2, i__3;
    doublereal d__1;

    /* Builtin functions */
    double sqrt();

    /* Local variables */
    static doublereal cbar, sbar;
    static integer last;
    static doublereal d1new, d2new;
    static integer m;
    static doublereal x, y, d1, d2;
    static integer first, m1, m2, mp1, inc, col, pos, row;


/*     ALGORITHM AS274.11 APPL. STATIST. (1992) VOL 41, NO.2 */

/*     Move variable from position FROM to position TO in an */
/*     orthogonal reduction produced by AS75.1. */


/*     Local variables */


    /* Parameter adjustments */
    --tol;
    --rss;
    --thetab;
    --d__;
    --vorder;
    --rbar;

    /* Function Body */

/*     Check input parameters */

    *ier = 0;
    if (*np <= 0) {
	*ier = 1;
    }
    if (*nrbar < *np * (*np - 1) / 2) {
	*ier += 2;
    }
    if (*from < 1 || *from > *np) {
	*ier += 4;
    }
    if (*to < 1 || *to > *np) {
	*ier += 8;
    }
    if (*ier != 0) {
	return 0;
    }

    if (*from == *to) {
	return 0;
    }

    if (*from < *to) {
	first = *from;
	last = *to - 1;
	inc = 1;
    } else {
	first = *from - 1;
	last = *to;
	inc = -1;
    }
    i__1 = last;
    i__2 = inc;
    for (m = first; i__2 < 0 ? m >= i__1 : m <= i__1; m += i__2) {

/*     Find addresses of first elements of RBAR in rows M and (M+1). */

	m1 = (m - 1) * (*np + *np - m) / 2 + 1;
	m2 = m1 + *np - m;
	mp1 = m + 1;
	d1 = d__[m];
	d2 = d__[mp1];

/*     Special cases. */

	if (d1 == zero && d2 == zero) {
	    goto L40;
	}
	x = rbar[m1];
	if (abs(x) * sqrt(d1) < tol[mp1]) {
	    x = zero;
	}
	if (d1 == zero || x == zero) {
	    d__[m] = d2;
	    d__[mp1] = d1;
	    rbar[m1] = zero;
	    i__3 = *np;
	    for (col = m + 2; col <= i__3; ++col) {
		++m1;
		x = rbar[m1];
		rbar[m1] = rbar[m2];
		rbar[m2] = x;
		++m2;
/* L10: */
	    }
	    x = thetab[m];
	    thetab[m] = thetab[mp1];
	    thetab[mp1] = x;
	    goto L40;
	} else if (d2 == zero) {
/* Computing 2nd power */
	    d__1 = x;
	    d__[m] = d1 * (d__1 * d__1);
	    rbar[m1] = one / x;
	    i__3 = *np;
	    for (col = m + 2; col <= i__3; ++col) {
		++m1;
		rbar[m1] /= x;
/* L20: */
	    }
	    thetab[m] /= x;
	    goto L40;
	}

/*     Planar rotation in regular case. */

/* Computing 2nd power */
	d__1 = x;
	d1new = d2 + d1 * (d__1 * d__1);
	cbar = d2 / d1new;
	sbar = x * d1 / d1new;
	d2new = d1 * cbar;
	d__[m] = d1new;
	d__[mp1] = d2new;
	rbar[m1] = sbar;
	i__3 = *np;
	for (col = m + 2; col <= i__3; ++col) {
	    ++m1;
	    y = rbar[m1];
	    rbar[m1] = cbar * rbar[m2] + sbar * y;
	    rbar[m2] = y - x * rbar[m2];
	    ++m2;
/* L30: */
	}
	y = thetab[m];
	thetab[m] = cbar * thetab[mp1] + sbar * y;
	thetab[mp1] = y - x * thetab[mp1];

/*     Swap columns M and (M+1) down to row (M-1). */

L40:
	if (m == 1) {
	    goto L60;
	}
	pos = m;
	i__3 = m - 1;
	for (row = 1; row <= i__3; ++row) {
	    x = rbar[pos];
	    rbar[pos] = rbar[pos - 1];
	    rbar[pos - 1] = x;
	    pos = pos + *np - row - 1;
/* L50: */
	}

/*     Adjust variable order (VORDER), the tolerances (TOL) and */
/*     the vector of residual sums of squares (RSS). */

L60:
	m1 = vorder[m];
	vorder[m] = vorder[mp1];
	vorder[mp1] = m1;
	x = tol[m];
	tol[m] = tol[mp1];
	tol[mp1] = x;
/* Computing 2nd power */
	d__1 = thetab[mp1];
	rss[m] = rss[mp1] + d__[mp1] * (d__1 * d__1);
/* L70: */
    }

    return 0;
} /* vmove_ */


/* Subroutine */ int reordr_(np, nrbar, vorder, d__, rbar, thetab, rss, tol, 
	list, n, pos1, ier)
integer *np, *nrbar, *vorder;
doublereal *d__, *rbar, *thetab, *rss, *tol;
integer *list, *n, *pos1, *ier;
{
    /* System generated locals */
    integer i__1;

    /* Local variables */
    static integer next, i__, j, l;
    extern /* Subroutine */ int vmove_();


/*     ALGORITHM AS274.12  APPL. STATIST. (1992) VOL 41, NO.2 */

/*     Re-order the variables in an orthogonal reduction produced by */
/*     AS75.1 so that the N variables in LIST start at position POS1, */
/*     though will not necessarily be in the same order as in LIST. */
/*     Any variables in VORDER before position POS1 are not moved. */

/*     Auxiliary routine called: VMOVE */


/*     Local variables. */


/*     Check N. */

    /* Parameter adjustments */
    --tol;
    --rss;
    --thetab;
    --d__;
    --vorder;
    --rbar;
    --list;

    /* Function Body */
    *ier = 0;
    if (*np < 1) {
	*ier = 1;
    }
    if (*nrbar < *np * (*np - 1) / 2) {
	*ier += 2;
    }
    if (*n < 1 || *n >= *np + 1 - *pos1) {
	*ier += 4;
    }
    if (*ier != 0) {
	return 0;
    }

/*     Work through VORDER finding variables which are in LIST. */

    next = *pos1;
    i__ = *pos1;
L10:
    l = vorder[i__];
    i__1 = *n;
    for (j = 1; j <= i__1; ++j) {
	if (l == list[j]) {
	    goto L40;
	}
/* L20: */
    }
L30:
    ++i__;
    if (i__ <= *np) {
	goto L10;
    }

/*     If this point is reached, one or more variables in LIST has not */
/*     been found. */

    *ier = 8;
    return 0;

/*     Variable L is in LIST; move it up to position NEXT if it is not */
/*     already there. */

L40:
    if (i__ > next) {
	vmove_(np, nrbar, &vorder[1], &d__[1], &rbar[1], &thetab[1], &rss[1], 
		&i__, &next, &tol[1], ier);
    }
    ++next;
    if (next < *n + *pos1) {
	goto L30;
    }

    return 0;
} /* reordr_ */


/* Subroutine */ int hdiag_(xrow, np, nrbar, d__, rbar, tol, nreq, hii, wk, 
	ifault)
doublereal *xrow;
integer *np, *nrbar;
doublereal *d__, *rbar, *tol;
integer *nreq;
doublereal *hii, *wk;
integer *ifault;
{
    /* Initialized data */

    static doublereal zero = 0.;

    /* System generated locals */
    integer i__1, i__2;
    doublereal d__1;

    /* Builtin functions */
    double sqrt();

    /* Local variables */
    static integer col, pos;
    static doublereal sum;
    static integer row;


/*     ALGORITHM AS274.13  APPL. STATIST. (1992) VOL.41, NO.2 */


/*     Local variables */


    /* Parameter adjustments */
    --wk;
    --tol;
    --d__;
    --xrow;
    --rbar;

    /* Function Body */

/*     Some checks */

    *ifault = 0;
    if (*np < 1) {
	*ifault = 1;
    }
    if (*nrbar < *np * (*np - 1) / 2) {
	*ifault += 2;
    }
    if (*nreq > *np) {
	*ifault += 4;
    }
    if (*ifault != 0) {
	return 0;
    }

/*     The elements of XROW.inv(RBAR).sqrt(D) are calculated and stored */
/*     in WK. */

    *hii = zero;
    i__1 = *nreq;
    for (col = 1; col <= i__1; ++col) {
	if (sqrt(d__[col]) <= tol[col]) {
	    wk[col] = zero;
	    goto L20;
	}
	pos = col - 1;
	sum = xrow[col];
	i__2 = col - 1;
	for (row = 1; row <= i__2; ++row) {
	    sum -= wk[row] * rbar[pos];
	    pos = pos + *np - row - 1;
/* L10: */
	}
	wk[col] = sum;
/* Computing 2nd power */
	d__1 = sum;
	*hii += d__1 * d__1 / d__[col];
L20:
	;
    }

    return 0;
} /* hdiag_ */

