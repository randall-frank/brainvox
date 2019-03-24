/* svd.f -- translated by f2c (version 19970805).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)
*/

#include "f2c.h"

/* Table of constant values */

static integer c__1 = 1;
static real c_b44 = -1.f;
static real c_b142 = 1.f;


/* 	Routines from LINPACK for singular value decomposition */

/* Subroutine */ int ssvdc_(real *x, integer *ldx, integer *n, integer *p, 
	real *s, real *e, real *u, integer *ldu, real *v, integer *ldv, real *
	work, integer *job, integer *info)
{
    /* System generated locals */
    integer x_dim1, x_offset, u_dim1, u_offset, v_dim1, v_offset, i__1, i__2, 
	    i__3;
    real r__1, r__2, r__3, r__4, r__5, r__6, r__7;

    /* Builtin functions */
    double r_sign(real *, real *), sqrt(doublereal);

    /* Local variables */
    static integer kase, jobu, iter;
    extern doublereal sdot_(integer *, real *, integer *, real *, integer *);
    static real test;
    extern /* Subroutine */ int srot_(integer *, real *, integer *, real *, 
	    integer *, real *, real *);
    static integer nctp1;
    extern doublereal snrm2_(integer *, real *, integer *);
    static real b, c__;
    static integer nrtp1;
    static real f, g;
    static integer i__, j, k, l, m;
    static real t, scale;
    extern /* Subroutine */ int sscal_(integer *, real *, real *, integer *);
    static real shift;
    static integer maxit;
    extern /* Subroutine */ int sswap_(integer *, real *, integer *, real *, 
	    integer *);
    static logical wantu, wantv;
    extern /* Subroutine */ int srotg_(real *, real *, real *, real *), 
	    saxpy_(integer *, real *, real *, integer *, real *, integer *);
    static real t1, ztest, el;
    static integer kk;
    static real cs;
    static integer ll, mm, ls;
    static real sl;
    static integer lu;
    static real sm, sn;
    static integer lm1, mm1, lp1, mp1, nct, ncu, lls, nrt;
    static real emm1, smm1;



/*     SSVDC IS A SUBROUTINE TO REDUCE A REAL NXP MATRIX X BY */
/*     ORTHOGONAL TRANSFORMATIONS U AND V TO DIAGONAL FORM.  THE */
/*     DIAGONAL ELEMENTS S(I) ARE THE SINGULAR VALUES OF X.  THE */
/*     COLUMNS OF U ARE THE CORRESPONDING LEFT SINGULAR VECTORS, */
/*     AND THE COLUMNS OF V THE RIGHT SINGULAR VECTORS. */

/*     ON ENTRY */

/*         X         REAL(LDX,P), WHERE LDX.GE.N. */
/*                   X CONTAINS THE MATRIX WHOSE SINGULAR VALUE */
/*                   DECOMPOSITION IS TO BE COMPUTED.  X IS */
/*                   DESTROYED BY SSVDC. */

/*         LDX       INTEGER. */
/*                   LDX IS THE LEADING DIMENSION OF THE ARRAY X. */

/*         N         INTEGER. */
/*                   N IS THE NUMBER OF COLUMNS OF THE MATRIX X. */

/*         P         INTEGER. */
/*                   P IS THE NUMBER OF ROWS OF THE MATRIX X. */

/*         LDU       INTEGER. */
/*                   LDU IS THE LEADING DIMENSION OF THE ARRAY U. */
/*                   (SEE BELOW). */

/*         LDV       INTEGER. */
/*                   LDV IS THE LEADING DIMENSION OF THE ARRAY V. */
/*                   (SEE BELOW). */

/*         WORK      REAL(N). */
/*                   WORK IS A SCRATCH ARRAY. */

/*         JOB       INTEGER. */
/*                   JOB CONTROLS THE COMPUTATION OF THE SINGULAR */
/*                   VECTORS.  IT HAS THE DECIMAL EXPANSION AB */
/*                   WITH THE FOLLOWING MEANING */

/*                        A.EQ.0    DO NOT COMPUTE THE LEFT SINGULAR */
/*                                  VECTORS. */
/*                        A.EQ.1    RETURN THE N LEFT SINGULAR VECTORS */
/*                                  IN U. */
/*                        A.GE.2    RETURN THE FIRST MIN(N,P) SINGULAR */
/*                                  VECTORS IN U. */
/*                        B.EQ.0    DO NOT COMPUTE THE RIGHT SINGULAR */
/*                                  VECTORS. */
/*                        B.EQ.1    RETURN THE RIGHT SINGULAR VECTORS */
/*                                  IN V. */

/*     ON RETURN */

/*         S         REAL(MM), WHERE MM=MIN(N+1,P). */
/*                   THE FIRST MIN(N,P) ENTRIES OF S CONTAIN THE */
/*                   SINGULAR VALUES OF X ARRANGED IN DESCENDING */
/*                   ORDER OF MAGNITUDE. */

/*         E         REAL(P). */
/*                   E ORDINARILY CONTAINS ZEROS.  HOWEVER SEE THE */
/*                   DISCUSSION OF INFO FOR EXCEPTIONS. */

/*         U         REAL(LDU,K), WHERE LDU.GE.N.  IF JOBA.EQ.1 THEN */
/*                                   K.EQ.N, IF JOBA.GE.2 THEN */
/*                                   K.EQ.MIN(N,P). */
/*                   U CONTAINS THE MATRIX OF RIGHT SINGULAR VECTORS. */
/*                   U IS NOT REFERENCED IF JOBA.EQ.0.  IF N.LE.P */
/*                   OR IF JOBA.EQ.2, THEN U MAY BE IDENTIFIED WITH X */
/*                   IN THE SUBROUTINE CALL. */

/*         V         REAL(LDV,P), WHERE LDV.GE.P. */
/*                   V CONTAINS THE MATRIX OF RIGHT SINGULAR VECTORS. */
/*                   V IS NOT REFERENCED IF JOB.EQ.0.  IF P.LE.N, */
/*                   THEN V MAY BE IDENTIFIED WITH X IN THE */
/*                   SUBROUTINE CALL. */

/*         INFO      INTEGER. */
/*                   THE SINGULAR VALUES (AND THEIR CORRESPONDING */
/*                   SINGULAR VECTORS) S(INFO+1),S(INFO+2),...,S(M) */
/*                   ARE CORRECT (HERE M=MIN(N,P)).  THUS IF */
/*                   INFO.EQ.0, ALL THE SINGULAR VALUES AND THEIR */
/*                   VECTORS ARE CORRECT.  IN ANY EVENT, THE MATRIX */
/*                   B = TRANS(U)*X*V IS THE BIDIAGONAL MATRIX */
/*                   WITH THE ELEMENTS OF S ON ITS DIAGONAL AND THE */
/*                   ELEMENTS OF E ON ITS SUPER-DIAGONAL (TRANS(U) */
/*                   IS THE TRANSPOSE OF U).  THUS THE SINGULAR */
/*                   VALUES OF X AND B ARE THE SAME. */

/*     LINPACK. THIS VERSION DATED 03/19/79 . */
/*     G.W. STEWART, UNIVERSITY OF MARYLAND, ARGONNE NATIONAL LAB. */

/*     ***** USES THE FOLLOWING FUNCTIONS AND SUBPROGRAMS. */

/*     EXTERNAL SROT */
/*     BLAS SAXPY,SDOT,SSCAL,SSWAP,SNRM2,SROTG */
/*     FORTRAN ABS,AMAX1,MAX0,MIN0,MOD,SQRT */

/*     INTERNAL VARIABLES */



/*     SET THE MAXIMUM NUMBER OF ITERATIONS. */

    /* Parameter adjustments */
    x_dim1 = *ldx;
    x_offset = x_dim1 + 1;
    x -= x_offset;
    --s;
    --e;
    u_dim1 = *ldu;
    u_offset = u_dim1 + 1;
    u -= u_offset;
    v_dim1 = *ldv;
    v_offset = v_dim1 + 1;
    v -= v_offset;
    --work;

    /* Function Body */
    maxit = 30;

/*     DETERMINE WHAT IS TO BE COMPUTED. */

    wantu = FALSE_;
    wantv = FALSE_;
    jobu = *job % 100 / 10;
    ncu = *n;
    if (jobu > 1) {
	ncu = min(*n,*p);
    }
    if (jobu != 0) {
	wantu = TRUE_;
    }
    if (*job % 10 != 0) {
	wantv = TRUE_;
    }

/*     REDUCE X TO BIDIAGONAL FORM, STORING THE DIAGONAL ELEMENTS */
/*     IN S AND THE SUPER-DIAGONAL ELEMENTS IN E. */

    *info = 0;
/* Computing MIN */
    i__1 = *n - 1;
    nct = min(i__1,*p);
/* Computing MAX */
/* Computing MIN */
    i__3 = *p - 2;
    i__1 = 0, i__2 = min(i__3,*n);
    nrt = max(i__1,i__2);
    lu = max(nct,nrt);
    if (lu < 1) {
	goto L170;
    }
    i__1 = lu;
    for (l = 1; l <= i__1; ++l) {
	lp1 = l + 1;
	if (l > nct) {
	    goto L20;
	}

/*           COMPUTE THE TRANSFORMATION FOR THE L-TH COLUMN AND */
/*           PLACE THE L-TH DIAGONAL IN S(L). */

	i__2 = *n - l + 1;
	s[l] = snrm2_(&i__2, &x[l + l * x_dim1], &c__1);
	if (s[l] == 0.f) {
	    goto L10;
	}
	if (x[l + l * x_dim1] != 0.f) {
	    s[l] = r_sign(&s[l], &x[l + l * x_dim1]);
	}
	i__2 = *n - l + 1;
	r__1 = 1.f / s[l];
	sscal_(&i__2, &r__1, &x[l + l * x_dim1], &c__1);
	x[l + l * x_dim1] += 1.f;
L10:
	s[l] = -s[l];
L20:
	if (*p < lp1) {
	    goto L50;
	}
	i__2 = *p;
	for (j = lp1; j <= i__2; ++j) {
	    if (l > nct) {
		goto L30;
	    }
	    if (s[l] == 0.f) {
		goto L30;
	    }

/*              APPLY THE TRANSFORMATION. */

	    i__3 = *n - l + 1;
	    t = -sdot_(&i__3, &x[l + l * x_dim1], &c__1, &x[l + j * x_dim1], &
		    c__1) / x[l + l * x_dim1];
	    i__3 = *n - l + 1;
	    saxpy_(&i__3, &t, &x[l + l * x_dim1], &c__1, &x[l + j * x_dim1], &
		    c__1);
L30:

/*           PLACE THE L-TH ROW OF X INTO  E FOR THE */
/*           SUBSEQUENT CALCULATION OF THE ROW TRANSFORMATION. */

	    e[j] = x[l + j * x_dim1];
/* L40: */
	}
L50:
	if (! wantu || l > nct) {
	    goto L70;
	}

/*           PLACE THE TRANSFORMATION IN U FOR SUBSEQUENT BACK */
/*           MULTIPLICATION. */

	i__2 = *n;
	for (i__ = l; i__ <= i__2; ++i__) {
	    u[i__ + l * u_dim1] = x[i__ + l * x_dim1];
/* L60: */
	}
L70:
	if (l > nrt) {
	    goto L150;
	}

/*           COMPUTE THE L-TH ROW TRANSFORMATION AND PLACE THE */
/*           L-TH SUPER-DIAGONAL IN E(L). */

	i__2 = *p - l;
	e[l] = snrm2_(&i__2, &e[lp1], &c__1);
	if (e[l] == 0.f) {
	    goto L80;
	}
	if (e[lp1] != 0.f) {
	    e[l] = r_sign(&e[l], &e[lp1]);
	}
	i__2 = *p - l;
	r__1 = 1.f / e[l];
	sscal_(&i__2, &r__1, &e[lp1], &c__1);
	e[lp1] += 1.f;
L80:
	e[l] = -e[l];
	if (lp1 > *n || e[l] == 0.f) {
	    goto L120;
	}

/*              APPLY THE TRANSFORMATION. */

	i__2 = *n;
	for (i__ = lp1; i__ <= i__2; ++i__) {
	    work[i__] = 0.f;
/* L90: */
	}
	i__2 = *p;
	for (j = lp1; j <= i__2; ++j) {
	    i__3 = *n - l;
	    saxpy_(&i__3, &e[j], &x[lp1 + j * x_dim1], &c__1, &work[lp1], &
		    c__1);
/* L100: */
	}
	i__2 = *p;
	for (j = lp1; j <= i__2; ++j) {
	    i__3 = *n - l;
	    r__1 = -e[j] / e[lp1];
	    saxpy_(&i__3, &r__1, &work[lp1], &c__1, &x[lp1 + j * x_dim1], &
		    c__1);
/* L110: */
	}
L120:
	if (! wantv) {
	    goto L140;
	}

/*              PLACE THE TRANSFORMATION IN V FOR SUBSEQUENT */
/*              BACK MULTIPLICATION. */

	i__2 = *p;
	for (i__ = lp1; i__ <= i__2; ++i__) {
	    v[i__ + l * v_dim1] = e[i__];
/* L130: */
	}
L140:
L150:
/* L160: */
	;
    }
L170:

/*     SET UP THE FINAL BIDIAGONAL MATRIX OR ORDER M. */

/* Computing MIN */
    i__1 = *p, i__2 = *n + 1;
    m = min(i__1,i__2);
    nctp1 = nct + 1;
    nrtp1 = nrt + 1;
    if (nct < *p) {
	s[nctp1] = x[nctp1 + nctp1 * x_dim1];
    }
    if (*n < m) {
	s[m] = 0.f;
    }
    if (nrtp1 < m) {
	e[nrtp1] = x[nrtp1 + m * x_dim1];
    }
    e[m] = 0.f;

/*     IF REQUIRED, GENERATE U. */

    if (! wantu) {
	goto L300;
    }
    if (ncu < nctp1) {
	goto L200;
    }
    i__1 = ncu;
    for (j = nctp1; j <= i__1; ++j) {
	i__2 = *n;
	for (i__ = 1; i__ <= i__2; ++i__) {
	    u[i__ + j * u_dim1] = 0.f;
/* L180: */
	}
	u[j + j * u_dim1] = 1.f;
/* L190: */
    }
L200:
    if (nct < 1) {
	goto L290;
    }
    i__1 = nct;
    for (ll = 1; ll <= i__1; ++ll) {
	l = nct - ll + 1;
	if (s[l] == 0.f) {
	    goto L250;
	}
	lp1 = l + 1;
	if (ncu < lp1) {
	    goto L220;
	}
	i__2 = ncu;
	for (j = lp1; j <= i__2; ++j) {
	    i__3 = *n - l + 1;
	    t = -sdot_(&i__3, &u[l + l * u_dim1], &c__1, &u[l + j * u_dim1], &
		    c__1) / u[l + l * u_dim1];
	    i__3 = *n - l + 1;
	    saxpy_(&i__3, &t, &u[l + l * u_dim1], &c__1, &u[l + j * u_dim1], &
		    c__1);
/* L210: */
	}
L220:
	i__2 = *n - l + 1;
	sscal_(&i__2, &c_b44, &u[l + l * u_dim1], &c__1);
	u[l + l * u_dim1] += 1.f;
	lm1 = l - 1;
	if (lm1 < 1) {
	    goto L240;
	}
	i__2 = lm1;
	for (i__ = 1; i__ <= i__2; ++i__) {
	    u[i__ + l * u_dim1] = 0.f;
/* L230: */
	}
L240:
	goto L270;
L250:
	i__2 = *n;
	for (i__ = 1; i__ <= i__2; ++i__) {
	    u[i__ + l * u_dim1] = 0.f;
/* L260: */
	}
	u[l + l * u_dim1] = 1.f;
L270:
/* L280: */
	;
    }
L290:
L300:

/*     IF IT IS REQUIRED, GENERATE V. */

    if (! wantv) {
	goto L350;
    }
    i__1 = *p;
    for (ll = 1; ll <= i__1; ++ll) {
	l = *p - ll + 1;
	lp1 = l + 1;
	if (l > nrt) {
	    goto L320;
	}
	if (e[l] == 0.f) {
	    goto L320;
	}
	i__2 = *p;
	for (j = lp1; j <= i__2; ++j) {
	    i__3 = *p - l;
	    t = -sdot_(&i__3, &v[lp1 + l * v_dim1], &c__1, &v[lp1 + j * 
		    v_dim1], &c__1) / v[lp1 + l * v_dim1];
	    i__3 = *p - l;
	    saxpy_(&i__3, &t, &v[lp1 + l * v_dim1], &c__1, &v[lp1 + j * 
		    v_dim1], &c__1);
/* L310: */
	}
L320:
	i__2 = *p;
	for (i__ = 1; i__ <= i__2; ++i__) {
	    v[i__ + l * v_dim1] = 0.f;
/* L330: */
	}
	v[l + l * v_dim1] = 1.f;
/* L340: */
    }
L350:

/*     MAIN ITERATION LOOP FOR THE SINGULAR VALUES. */

    mm = m;
    iter = 0;
L360:

/*        QUIT IF ALL THE SINGULAR VALUES HAVE BEEN FOUND. */

/*     ...EXIT */
    if (m == 0) {
	goto L620;
    }

/*        IF TOO MANY ITERATIONS HAVE BEEN PERFORMED, SET */
/*        FLAG AND RETURN. */

    if (iter < maxit) {
	goto L370;
    }
    *info = m;
/*     ......EXIT */
    goto L620;
L370:

/*        THIS SECTION OF THE PROGRAM INSPECTS FOR */
/*        NEGLIGIBLE ELEMENTS IN THE S AND E ARRAYS.  ON */
/*        COMPLETION THE VARIABLES KASE AND L ARE SET AS FOLLOWS. */

/*           KASE = 1     IF S(M) AND E(L-1) ARE NEGLIGIBLE AND L.LT.M */
/*           KASE = 2     IF S(L) IS NEGLIGIBLE AND L.LT.M */
/*           KASE = 3     IF E(L-1) IS NEGLIGIBLE, L.LT.M, AND */
/*                        S(L), ..., S(M) ARE NOT NEGLIGIBLE (QR STEP). */
/*           KASE = 4     IF E(M-1) IS NEGLIGIBLE (CONVERGENCE). */

    i__1 = m;
    for (ll = 1; ll <= i__1; ++ll) {
	l = m - ll;
/*        ...EXIT */
	if (l == 0) {
	    goto L400;
	}
	test = (r__1 = s[l], dabs(r__1)) + (r__2 = s[l + 1], dabs(r__2));
	ztest = test + (r__1 = e[l], dabs(r__1));
	if (ztest != test) {
	    goto L380;
	}
	e[l] = 0.f;
/*        ......EXIT */
	goto L400;
L380:
/* L390: */
	;
    }
L400:
    if (l != m - 1) {
	goto L410;
    }
    kase = 4;
    goto L480;
L410:
    lp1 = l + 1;
    mp1 = m + 1;
    i__1 = mp1;
    for (lls = lp1; lls <= i__1; ++lls) {
	ls = m - lls + lp1;
/*           ...EXIT */
	if (ls == l) {
	    goto L440;
	}
	test = 0.f;
	if (ls != m) {
	    test += (r__1 = e[ls], dabs(r__1));
	}
	if (ls != l + 1) {
	    test += (r__1 = e[ls - 1], dabs(r__1));
	}
	ztest = test + (r__1 = s[ls], dabs(r__1));
	if (ztest != test) {
	    goto L420;
	}
	s[ls] = 0.f;
/*           ......EXIT */
	goto L440;
L420:
/* L430: */
	;
    }
L440:
    if (ls != l) {
	goto L450;
    }
    kase = 3;
    goto L470;
L450:
    if (ls != m) {
	goto L460;
    }
    kase = 1;
    goto L470;
L460:
    kase = 2;
    l = ls;
L470:
L480:
    ++l;

/*        PERFORM THE TASK INDICATED BY KASE. */

    switch (kase) {
	case 1:  goto L490;
	case 2:  goto L520;
	case 3:  goto L540;
	case 4:  goto L570;
    }

/*        DEFLATE NEGLIGIBLE S(M). */

L490:
    mm1 = m - 1;
    f = e[m - 1];
    e[m - 1] = 0.f;
    i__1 = mm1;
    for (kk = l; kk <= i__1; ++kk) {
	k = mm1 - kk + l;
	t1 = s[k];
	srotg_(&t1, &f, &cs, &sn);
	s[k] = t1;
	if (k == l) {
	    goto L500;
	}
	f = -sn * e[k - 1];
	e[k - 1] = cs * e[k - 1];
L500:
	if (wantv) {
	    srot_(p, &v[k * v_dim1 + 1], &c__1, &v[m * v_dim1 + 1], &c__1, &
		    cs, &sn);
	}
/* L510: */
    }
    goto L610;

/*        SPLIT AT NEGLIGIBLE S(L). */

L520:
    f = e[l - 1];
    e[l - 1] = 0.f;
    i__1 = m;
    for (k = l; k <= i__1; ++k) {
	t1 = s[k];
	srotg_(&t1, &f, &cs, &sn);
	s[k] = t1;
	f = -sn * e[k];
	e[k] = cs * e[k];
	if (wantu) {
	    srot_(n, &u[k * u_dim1 + 1], &c__1, &u[(l - 1) * u_dim1 + 1], &
		    c__1, &cs, &sn);
	}
/* L530: */
    }
    goto L610;

/*        PERFORM ONE QR STEP. */

L540:

/*           CALCULATE THE SHIFT. */

/* Computing MAX */
    r__6 = (r__1 = s[m], dabs(r__1)), r__7 = (r__2 = s[m - 1], dabs(r__2)), 
	    r__6 = max(r__6,r__7), r__7 = (r__3 = e[m - 1], dabs(r__3)), r__6 
	    = max(r__6,r__7), r__7 = (r__4 = s[l], dabs(r__4)), r__6 = max(
	    r__6,r__7), r__7 = (r__5 = e[l], dabs(r__5));
    scale = dmax(r__6,r__7);
    sm = s[m] / scale;
    smm1 = s[m - 1] / scale;
    emm1 = e[m - 1] / scale;
    sl = s[l] / scale;
    el = e[l] / scale;
/* Computing 2nd power */
    r__1 = emm1;
    b = ((smm1 + sm) * (smm1 - sm) + r__1 * r__1) / 2.f;
/* Computing 2nd power */
    r__1 = sm * emm1;
    c__ = r__1 * r__1;
    shift = 0.f;
    if (b == 0.f && c__ == 0.f) {
	goto L550;
    }
/* Computing 2nd power */
    r__1 = b;
    shift = sqrt(r__1 * r__1 + c__);
    if (b < 0.f) {
	shift = -shift;
    }
    shift = c__ / (b + shift);
L550:
    f = (sl + sm) * (sl - sm) - shift;
    g = sl * el;

/*           CHASE ZEROS. */

    mm1 = m - 1;
    i__1 = mm1;
    for (k = l; k <= i__1; ++k) {
	srotg_(&f, &g, &cs, &sn);
	if (k != l) {
	    e[k - 1] = f;
	}
	f = cs * s[k] + sn * e[k];
	e[k] = cs * e[k] - sn * s[k];
	g = sn * s[k + 1];
	s[k + 1] = cs * s[k + 1];
	if (wantv) {
	    srot_(p, &v[k * v_dim1 + 1], &c__1, &v[(k + 1) * v_dim1 + 1], &
		    c__1, &cs, &sn);
	}
	srotg_(&f, &g, &cs, &sn);
	s[k] = f;
	f = cs * e[k] + sn * s[k + 1];
	s[k + 1] = -sn * e[k] + cs * s[k + 1];
	g = sn * e[k + 1];
	e[k + 1] = cs * e[k + 1];
	if (wantu && k < *n) {
	    srot_(n, &u[k * u_dim1 + 1], &c__1, &u[(k + 1) * u_dim1 + 1], &
		    c__1, &cs, &sn);
	}
/* L560: */
    }
    e[m - 1] = f;
    ++iter;
    goto L610;

/*        CONVERGENCE. */

L570:

/*           MAKE THE SINGULAR VALUE  POSITIVE. */

    if (s[l] >= 0.f) {
	goto L580;
    }
    s[l] = -s[l];
    if (wantv) {
	sscal_(p, &c_b44, &v[l * v_dim1 + 1], &c__1);
    }
L580:

/*           ORDER THE SINGULAR VALUE. */

L590:
    if (l == mm) {
	goto L600;
    }
/*           ...EXIT */
    if (s[l] >= s[l + 1]) {
	goto L600;
    }
    t = s[l];
    s[l] = s[l + 1];
    s[l + 1] = t;
    if (wantv && l < *p) {
	sswap_(p, &v[l * v_dim1 + 1], &c__1, &v[(l + 1) * v_dim1 + 1], &c__1);
    }
    if (wantu && l < *n) {
	sswap_(n, &u[l * u_dim1 + 1], &c__1, &u[(l + 1) * u_dim1 + 1], &c__1);
    }
    ++l;
    goto L590;
L600:
    iter = 0;
    --m;
L610:
    goto L360;
L620:
    return 0;
} /* ssvdc_ */

doublereal sasum_(integer *n, real *sx, integer *incx)
{
    /* System generated locals */
    integer i__1, i__2;
    real ret_val, r__1, r__2, r__3, r__4, r__5, r__6;

    /* Local variables */
    static integer i__, m, nincx;
    static real stemp;
    static integer mp1;


/*     TAKES THE SUM OF THE ABSOLUTE VALUES. */
/*     USES UNROLLED LOOPS FOR INCREMENT EQUAL TO ONE. */
/*     JACK DONGARRA, LINPACK, 3/11/78. */


    /* Parameter adjustments */
    --sx;

    /* Function Body */
    ret_val = 0.f;
    stemp = 0.f;
    if (*n <= 0) {
	return ret_val;
    }
    if (*incx == 1) {
	goto L20;
    }

/*        CODE FOR INCREMENT NOT EQUAL TO 1 */

    nincx = *n * *incx;
    i__1 = nincx;
    i__2 = *incx;
    for (i__ = 1; i__2 < 0 ? i__ >= i__1 : i__ <= i__1; i__ += i__2) {
	stemp += (r__1 = sx[i__], dabs(r__1));
/* L10: */
    }
    ret_val = stemp;
    return ret_val;

/*        CODE FOR INCREMENT EQUAL TO 1 */


/*        CLEAN-UP LOOP */

L20:
    m = *n % 6;
    if (m == 0) {
	goto L40;
    }
    i__2 = m;
    for (i__ = 1; i__ <= i__2; ++i__) {
	stemp += (r__1 = sx[i__], dabs(r__1));
/* L30: */
    }
    if (*n < 6) {
	goto L60;
    }
L40:
    mp1 = m + 1;
    i__2 = *n;
    for (i__ = mp1; i__ <= i__2; i__ += 6) {
	stemp = stemp + (r__1 = sx[i__], dabs(r__1)) + (r__2 = sx[i__ + 1], 
		dabs(r__2)) + (r__3 = sx[i__ + 2], dabs(r__3)) + (r__4 = sx[
		i__ + 3], dabs(r__4)) + (r__5 = sx[i__ + 4], dabs(r__5)) + (
		r__6 = sx[i__ + 5], dabs(r__6));
/* L50: */
    }
L60:
    ret_val = stemp;
    return ret_val;
} /* sasum_ */

/* Subroutine */ int saxpy_(integer *n, real *sa, real *sx, integer *incx, 
	real *sy, integer *incy)
{
    /* System generated locals */
    integer i__1;

    /* Local variables */
    static integer i__, m, ix, iy, mp1;


/*     CONSTANT TIMES A VECTOR PLUS A VECTOR. */
/*     USES UNROLLED LOOP FOR INCREMENTS EQUAL TO ONE. */
/*     JACK DONGARRA, LINPACK, 3/11/78. */


    /* Parameter adjustments */
    --sy;
    --sx;

    /* Function Body */
    if (*n <= 0) {
	return 0;
    }
    if (*sa == 0.f) {
	return 0;
    }
    if (*incx == 1 && *incy == 1) {
	goto L20;
    }

/*        CODE FOR UNEQUAL INCREMENTS OR EQUAL INCREMENTS */
/*          NOT EQUAL TO 1 */

    ix = 1;
    iy = 1;
    if (*incx < 0) {
	ix = (-(*n) + 1) * *incx + 1;
    }
    if (*incy < 0) {
	iy = (-(*n) + 1) * *incy + 1;
    }
    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
	sy[iy] += *sa * sx[ix];
	ix += *incx;
	iy += *incy;
/* L10: */
    }
    return 0;

/*        CODE FOR BOTH INCREMENTS EQUAL TO 1 */


/*        CLEAN-UP LOOP */

L20:
    m = *n % 4;
    if (m == 0) {
	goto L40;
    }
    i__1 = m;
    for (i__ = 1; i__ <= i__1; ++i__) {
	sy[i__] += *sa * sx[i__];
/* L30: */
    }
    if (*n < 4) {
	return 0;
    }
L40:
    mp1 = m + 1;
    i__1 = *n;
    for (i__ = mp1; i__ <= i__1; i__ += 4) {
	sy[i__] += *sa * sx[i__];
	sy[i__ + 1] += *sa * sx[i__ + 1];
	sy[i__ + 2] += *sa * sx[i__ + 2];
	sy[i__ + 3] += *sa * sx[i__ + 3];
/* L50: */
    }
    return 0;
} /* saxpy_ */

doublereal sdot_(integer *n, real *sx, integer *incx, real *sy, integer *incy)
{
    /* System generated locals */
    integer i__1;
    real ret_val;

    /* Local variables */
    static integer i__, m;
    static real stemp;
    static integer ix, iy, mp1;


/*     FORMS THE DOT PRODUCT OF TWO VECTORS. */
/*     USES UNROLLED LOOPS FOR INCREMENTS EQUAL TO ONE. */
/*     JACK DONGARRA, LINPACK, 3/11/78. */


    /* Parameter adjustments */
    --sy;
    --sx;

    /* Function Body */
    stemp = 0.f;
    ret_val = 0.f;
    if (*n <= 0) {
	return ret_val;
    }
    if (*incx == 1 && *incy == 1) {
	goto L20;
    }

/*        CODE FOR UNEQUAL INCREMENTS OR EQUAL INCREMENTS */
/*          NOT EQUAL TO 1 */

    ix = 1;
    iy = 1;
    if (*incx < 0) {
	ix = (-(*n) + 1) * *incx + 1;
    }
    if (*incy < 0) {
	iy = (-(*n) + 1) * *incy + 1;
    }
    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
	stemp += sx[ix] * sy[iy];
	ix += *incx;
	iy += *incy;
/* L10: */
    }
    ret_val = stemp;
    return ret_val;

/*        CODE FOR BOTH INCREMENTS EQUAL TO 1 */


/*        CLEAN-UP LOOP */

L20:
    m = *n % 5;
    if (m == 0) {
	goto L40;
    }
    i__1 = m;
    for (i__ = 1; i__ <= i__1; ++i__) {
	stemp += sx[i__] * sy[i__];
/* L30: */
    }
    if (*n < 5) {
	goto L60;
    }
L40:
    mp1 = m + 1;
    i__1 = *n;
    for (i__ = mp1; i__ <= i__1; i__ += 5) {
	stemp = stemp + sx[i__] * sy[i__] + sx[i__ + 1] * sy[i__ + 1] + sx[
		i__ + 2] * sy[i__ + 2] + sx[i__ + 3] * sy[i__ + 3] + sx[i__ + 
		4] * sy[i__ + 4];
/* L50: */
    }
L60:
    ret_val = stemp;
    return ret_val;
} /* sdot_ */

doublereal snrm2_(integer *n, real *sx, integer *incx)
{
    /* Initialized data */

    static real zero = 0.f;
    static real one = 1.f;
    static real cutlo = 4.441e-16f;
    static real cuthi = 1.304e19f;

    /* Format strings */
    static char fmt_30[] = "";
    static char fmt_40[] = "";
    static char fmt_70[] = "";
    static char fmt_80[] = "";

    /* System generated locals */
    integer i__1, i__2;
    real ret_val, r__1;

    /* Builtin functions */
    double sqrt(doublereal);

    /* Local variables */
    static real xmax;
    static integer next, i__, j, nn;
    static real hitest, sum;

    /* Assigned format variables */
    static char *next_fmt;

    /* Parameter adjustments */
    --sx;

    /* Function Body */

/*     EUCLIDEAN NORM OF THE N-VECTOR STORED IN SX() WITH STORAGE */
/*     INCREMENT INCX . */
/*     IF    N .LE. 0 RETURN WITH RESULT = 0. */
/*     IF N .GE. 1 THEN INCX MUST BE .GE. 1 */

/*           C.L.LAWSON, 1978 JAN 08 */

/*     FOUR PHASE METHOD     USING TWO BUILT-IN CONSTANTS THAT ARE */
/*     HOPEFULLY APPLICABLE TO ALL MACHINES. */
/*         CUTLO = MAXIMUM OF  SQRT(U/EPS)  OVER ALL KNOWN MACHINES. */
/*         CUTHI = MINIMUM OF  SQRT(V)      OVER ALL KNOWN MACHINES. */
/*     WHERE */
/*         EPS = SMALLEST NO. SUCH THAT EPS + 1. .GT. 1. */
/*         U   = SMALLEST POSITIVE NO.   (UNDERFLOW LIMIT) */
/*         V   = LARGEST  NO.            (OVERFLOW  LIMIT) */

/*     BRIEF OUTLINE OF ALGORITHM.. */

/*     PHASE 1    SCANS ZERO COMPONENTS. */
/*     MOVE TO PHASE 2 WHEN A COMPONENT IS NONZERO AND .LE. CUTLO */
/*     MOVE TO PHASE 3 WHEN A COMPONENT IS .GT. CUTLO */
/*     MOVE TO PHASE 4 WHEN A COMPONENT IS .GE. CUTHI/M */
/*     WHERE M = N FOR X() REAL AND M = 2*N FOR COMPLEX. */

/*     VALUES FOR CUTLO AND CUTHI.. */
/*     FROM THE ENVIRONMENTAL PARAMETERS LISTED IN THE IMSL CONVERTER */
/*     DOCUMENT THE LIMITING VALUES ARE AS FOLLOWS.. */
/*     CUTLO, S.P.   U/EPS = 2**(-102) FOR  HONEYWELL.  CLOSE SECONDS ARE 
*/
/*                   UNIVAC AND DEC AT 2**(-103) */
/*                   THUS CUTLO = 2**(-51) = 4.44089E-16 */
/*     CUTHI, S.P.   V = 2**127 FOR UNIVAC, HONEYWELL, AND DEC. */
/*                   THUS CUTHI = 2**(63.5) = 1.30438E19 */
/*     CUTLO, D.P.   U/EPS = 2**(-67) FOR HONEYWELL AND DEC. */
/*                   THUS CUTLO = 2**(-33.5) = 8.23181D-11 */
/*     CUTHI, D.P.   SAME AS S.P.  CUTHI = 1.30438D19 */
/*     DATA CUTLO, CUTHI / 8.232D-11,  1.304D19 / */
/*     DATA CUTLO, CUTHI / 4.441E-16,  1.304E19 / */

    if (*n > 0) {
	goto L10;
    }
    ret_val = zero;
    goto L140;

L10:
    next = 0;
    next_fmt = fmt_30;
    sum = zero;
    nn = *n * *incx;
/*                                                 BEGIN MAIN LOOP */
    i__ = 1;
L20:
    switch (next) {
	case 0: goto L30;
	case 1: goto L40;
	case 2: goto L70;
	case 3: goto L80;
    }
L30:
    if ((r__1 = sx[i__], dabs(r__1)) > cutlo) {
	goto L110;
    }
    next = 1;
    next_fmt = fmt_40;
    xmax = zero;

/*                        PHASE 1.  SUM IS ZERO */

L40:
    if (sx[i__] == zero) {
	goto L130;
    }
    if ((r__1 = sx[i__], dabs(r__1)) > cutlo) {
	goto L110;
    }

/*                                PREPARE FOR PHASE 2. */
    next = 2;
    next_fmt = fmt_70;
    goto L60;

/*                                PREPARE FOR PHASE 4. */

L50:
    i__ = j;
    next = 3;
    next_fmt = fmt_80;
    sum = sum / sx[i__] / sx[i__];
L60:
    xmax = (r__1 = sx[i__], dabs(r__1));
    goto L90;

/*                   PHASE 2.  SUM IS SMALL. */
/*                             SCALE TO AVOID DESTRUCTIVE UNDERFLOW. */

L70:
    if ((r__1 = sx[i__], dabs(r__1)) > cutlo) {
	goto L100;
    }

/*                     COMMON CODE FOR PHASES 2 AND 4. */
/*                     IN PHASE 4 SUM IS LARGE.  SCALE TO AVOID OVERFLOW. 
*/

L80:
    if ((r__1 = sx[i__], dabs(r__1)) <= xmax) {
	goto L90;
    }
/* Computing 2nd power */
    r__1 = xmax / sx[i__];
    sum = one + sum * (r__1 * r__1);
    xmax = (r__1 = sx[i__], dabs(r__1));
    goto L130;

L90:
/* Computing 2nd power */
    r__1 = sx[i__] / xmax;
    sum += r__1 * r__1;
    goto L130;


/*                  PREPARE FOR PHASE 3. */

L100:
    sum = sum * xmax * xmax;


/*     FOR REAL OR D.P. SET HITEST = CUTHI/N */
/*     FOR COMPLEX      SET HITEST = CUTHI/(2*N) */

L110:
    hitest = cuthi / (real) (*n);

/*                   PHASE 3.  SUM IS MID-RANGE.  NO SCALING. */

    i__1 = nn;
    i__2 = *incx;
    for (j = i__; i__2 < 0 ? j >= i__1 : j <= i__1; j += i__2) {
	if ((r__1 = sx[j], dabs(r__1)) >= hitest) {
	    goto L50;
	}
/* Computing 2nd power */
	r__1 = sx[j];
	sum += r__1 * r__1;
/* L120: */
    }
    ret_val = sqrt(sum);
    goto L140;

L130:
    i__ += *incx;
    if (i__ <= nn) {
	goto L20;
    }

/*              END OF MAIN LOOP. */

/*              COMPUTE SQUARE ROOT AND ADJUST FOR SCALING. */

    ret_val = xmax * sqrt(sum);
L140:
    return ret_val;
} /* snrm2_ */

/* Subroutine */ int srot_(integer *n, real *sx, integer *incx, real *sy, 
	integer *incy, real *c__, real *s)
{
    /* System generated locals */
    integer i__1;

    /* Local variables */
    static integer i__;
    static real stemp;
    static integer ix, iy;


/*     APPLIES A PLANE ROTATION. */
/*     JACK DONGARRA, LINPACK, 3/11/78. */


    /* Parameter adjustments */
    --sy;
    --sx;

    /* Function Body */
    if (*n <= 0) {
	return 0;
    }
    if (*incx == 1 && *incy == 1) {
	goto L20;
    }

/*       CODE FOR UNEQUAL INCREMENTS OR EQUAL INCREMENTS NOT EQUAL */
/*         TO 1 */

    ix = 1;
    iy = 1;
    if (*incx < 0) {
	ix = (-(*n) + 1) * *incx + 1;
    }
    if (*incy < 0) {
	iy = (-(*n) + 1) * *incy + 1;
    }
    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
	stemp = *c__ * sx[ix] + *s * sy[iy];
	sy[iy] = *c__ * sy[iy] - *s * sx[ix];
	sx[ix] = stemp;
	ix += *incx;
	iy += *incy;
/* L10: */
    }
    return 0;

/*       CODE FOR BOTH INCREMENTS EQUAL TO 1 */

L20:
    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
	stemp = *c__ * sx[i__] + *s * sy[i__];
	sy[i__] = *c__ * sy[i__] - *s * sx[i__];
	sx[i__] = stemp;
/* L30: */
    }
    return 0;
} /* srot_ */

/* Subroutine */ int srotg_(real *sa, real *sb, real *c__, real *s)
{
    /* System generated locals */
    real r__1, r__2;

    /* Builtin functions */
    double sqrt(doublereal), r_sign(real *, real *);

    /* Local variables */
    static real r__, scale, z__, roe;


/*     CONSTRUCT GIVENS PLANE ROTATION. */
/*     JACK DONGARRA, LINPACK, 3/11/78. */


    roe = *sb;
    if (dabs(*sa) > dabs(*sb)) {
	roe = *sa;
    }
    scale = dabs(*sa) + dabs(*sb);
    if (scale != 0.f) {
	goto L10;
    }
    *c__ = 1.f;
    *s = 0.f;
    r__ = 0.f;
    goto L20;
L10:
/* Computing 2nd power */
    r__1 = *sa / scale;
/* Computing 2nd power */
    r__2 = *sb / scale;
    r__ = scale * sqrt(r__1 * r__1 + r__2 * r__2);
    r__ = r_sign(&c_b142, &roe) * r__;
    *c__ = *sa / r__;
    *s = *sb / r__;
L20:
    z__ = 1.f;
    if (dabs(*sa) > dabs(*sb)) {
	z__ = *s;
    }
    if (dabs(*sb) >= dabs(*sa) && *c__ != 0.f) {
	z__ = 1.f / *c__;
    }
    *sa = r__;
    *sb = z__;
    return 0;
} /* srotg_ */

/* Subroutine */ int sscal_(integer *n, real *sa, real *sx, integer *incx)
{
    /* System generated locals */
    integer i__1, i__2;

    /* Local variables */
    static integer i__, m, nincx, mp1;


/*     SCALES A VECTOR BY A CONSTANT. */
/*     USES UNROLLED LOOPS FOR INCREMENT EQUAL TO 1. */
/*     JACK DONGARRA, LINPACK, 3/11/78. */


    /* Parameter adjustments */
    --sx;

    /* Function Body */
    if (*n <= 0) {
	return 0;
    }
    if (*incx == 1) {
	goto L20;
    }

/*        CODE FOR INCREMENT NOT EQUAL TO 1 */

    nincx = *n * *incx;
    i__1 = nincx;
    i__2 = *incx;
    for (i__ = 1; i__2 < 0 ? i__ >= i__1 : i__ <= i__1; i__ += i__2) {
	sx[i__] = *sa * sx[i__];
/* L10: */
    }
    return 0;

/*        CODE FOR INCREMENT EQUAL TO 1 */


/*        CLEAN-UP LOOP */

L20:
    m = *n % 5;
    if (m == 0) {
	goto L40;
    }
    i__2 = m;
    for (i__ = 1; i__ <= i__2; ++i__) {
	sx[i__] = *sa * sx[i__];
/* L30: */
    }
    if (*n < 5) {
	return 0;
    }
L40:
    mp1 = m + 1;
    i__2 = *n;
    for (i__ = mp1; i__ <= i__2; i__ += 5) {
	sx[i__] = *sa * sx[i__];
	sx[i__ + 1] = *sa * sx[i__ + 1];
	sx[i__ + 2] = *sa * sx[i__ + 2];
	sx[i__ + 3] = *sa * sx[i__ + 3];
	sx[i__ + 4] = *sa * sx[i__ + 4];
/* L50: */
    }
    return 0;
} /* sscal_ */

/* Subroutine */ int svd_(integer *nm, integer *m, integer *n, real *a, real *
	w, logical *matu, real *u, logical *matv, real *v, integer *ierr, 
	real *rv1)
{
    /* System generated locals */
    integer a_dim1, a_offset, u_dim1, u_offset, v_dim1, v_offset, i__1, i__2, 
	    i__3;
    real r__1, r__2, r__3, r__4;

    /* Builtin functions */
    double sqrt(doublereal), r_sign(real *, real *);

    /* Local variables */
    static real c__, f, g, h__;
    static integer i__, j, k, l;
    static real s, x, y, z__, scale;
    static integer i1, k1, l1, ii, kk, ll, mn;
    static real machep, eps;
    static integer its;



/*     THIS SUBROUTINE IS A TRANSLATION OF THE ALGOL PROCEDURE SVD, */
/*     NUM. MATH. 14, 403-420(1970) BY GOLUB AND REINSCH. */
/*     HANDBOOK FOR AUTO. COMP., VOL II-LINEAR ALGEBRA, 134-151(1971). */

/*     THIS SUBROUTINE DETERMINES THE SINGULAR VALUE DECOMPOSITION */
/*          T */
/*     A=USV  OF A REAL M BY N RECTANGULAR MATRIX.  HOUSEHOLDER */
/*     BIDIAGONALIZATION AND A VARIANT OF THE QR ALGORITHM ARE USED. */

/*     ON INPUT- */

/*        NM MUST BE SET TO THE ROW DIMENSION OF TWO-DIMENSIONAL */
/*          ARRAY PARAMETERS AS DECLARED IN THE CALLING PROGRAM */
/*          DIMENSION STATEMENT.  NOTE THAT NM MUST BE AT LEAST */
/*          AS LARGE AS THE MAXIMUM OF M AND N, */

/*        M IS THE NUMBER OF ROWS OF A (AND U), */

/*        N IS THE NUMBER OF COLUMNS OF A (AND U) AND THE ORDER OF V, */

/*        A CONTAINS THE RECTANGULAR INPUT MATRIX TO BE DECOMPOSED, */

/*        MATU SHOULD BE SET TO .TRUE. IF THE U MATRIX IN THE */
/*          DECOMPOSITION IS DESIRED, AND TO .FALSE. OTHERWISE, */

/*        MATV SHOULD BE SET TO .TRUE. IF THE V MATRIX IN THE */
/*          DECOMPOSITION IS DESIRED, AND TO .FALSE. OTHERWISE. */

/*     ON OUTPUT- */

/*        A IS UNALTERED (UNLESS OVERWRITTEN BY U OR V), */

/*        W CONTAINS THE N (NON-NEGATIVE) SINGULAR VALUES OF A (THE */
/*          DIAGONAL ELEMENTS OF S).  THEY ARE UNORDERED.  IF AN */
/*          ERROR EXIT IS MADE, THE SINGULAR VALUES SHOULD BE CORRECT */
/*          FOR INDICES IERR+1,IERR+2,...,N, */

/*        U CONTAINS THE MATRIX U (ORTHOGONAL COLUMN VECTORS) OF THE */
/*          DECOMPOSITION IF MATU HAS BEEN SET TO .TRUE.  OTHERWISE */
/*          U IS USED AS A TEMPORARY ARRAY.  U MAY COINCIDE WITH A. */
/*          IF AN ERROR EXIT IS MADE, THE COLUMNS OF U CORRESPONDING */
/*          TO INDICES OF CORRECT SINGULAR VALUES SHOULD BE CORRECT, */

/*        V CONTAINS THE MATRIX V (ORTHOGONAL) OF THE DECOMPOSITION IF */
/*          MATV HAS BEEN SET TO .TRUE.  OTHERWISE V IS NOT REFERENCED. */
/*          V MAY ALSO COINCIDE WITH A IF U IS NOT NEEDED.  IF AN ERROR */
/*          EXIT IS MADE, THE COLUMNS OF V CORRESPONDING TO INDICES OF */
/*          CORRECT SINGULAR VALUES SHOULD BE CORRECT, */

/*        IERR IS SET TO */
/*          ZERO       FOR NORMAL RETURN, */
/*          K          IF THE K-TH SINGULAR VALUE HAS NOT BEEN */
/*                     DETERMINED AFTER 30 ITERATIONS, */

/*        RV1 IS A TEMPORARY STORAGE ARRAY. */

/*     QUESTIONS AND COMMENTS SHOULD BE DIRECTED TO B. S. GARBOW, */
/*     APPLIED MATHEMATICS DIVISION, ARGONNE NATIONAL LABORATORY */

/*     ------------------------------------------------------------------ 
*/

/*     ********** MACHEP IS A MACHINE DEPENDENT PARAMETER SPECIFYING */
/*                THE RELATIVE PRECISION OF FLOATING POINT ARITHMETIC. */

/*                ********** */
    /* Parameter adjustments */
    --rv1;
    v_dim1 = *nm;
    v_offset = v_dim1 + 1;
    v -= v_offset;
    u_dim1 = *nm;
    u_offset = u_dim1 + 1;
    u -= u_offset;
    --w;
    a_dim1 = *nm;
    a_offset = a_dim1 + 1;
    a -= a_offset;

    /* Function Body */
    machep = 1.4901161193847656e-8f;

    *ierr = 0;

    i__1 = *m;
    for (i__ = 1; i__ <= i__1; ++i__) {

	i__2 = *n;
	for (j = 1; j <= i__2; ++j) {
	    u[i__ + j * u_dim1] = a[i__ + j * a_dim1];
/* L10: */
	}
/* L20: */
    }
/*     ********** HOUSEHOLDER REDUCTION TO BIDIAGONAL FORM ********** */
    g = 0.f;
    scale = 0.f;
    x = 0.f;

    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
	l = i__ + 1;
	rv1[i__] = scale * g;
	g = 0.f;
	s = 0.f;
	scale = 0.f;
	if (i__ > *m) {
	    goto L100;
	}

	i__2 = *m;
	for (k = i__; k <= i__2; ++k) {
	    scale += (r__1 = u[k + i__ * u_dim1], dabs(r__1));
/* L30: */
	}

	if (scale == 0.f) {
	    goto L100;
	}

	i__2 = *m;
	for (k = i__; k <= i__2; ++k) {
	    u[k + i__ * u_dim1] /= scale;
/* Computing 2nd power */
	    r__1 = u[k + i__ * u_dim1];
	    s += r__1 * r__1;
/* L40: */
	}

	f = u[i__ + i__ * u_dim1];
	r__1 = sqrt(s);
	g = -r_sign(&r__1, &f);
	h__ = f * g - s;
	u[i__ + i__ * u_dim1] = f - g;
	if (i__ == *n) {
	    goto L80;
	}

	i__2 = *n;
	for (j = l; j <= i__2; ++j) {
	    s = 0.f;

	    i__3 = *m;
	    for (k = i__; k <= i__3; ++k) {
		s += u[k + i__ * u_dim1] * u[k + j * u_dim1];
/* L50: */
	    }

	    f = s / h__;

	    i__3 = *m;
	    for (k = i__; k <= i__3; ++k) {
		u[k + j * u_dim1] += f * u[k + i__ * u_dim1];
/* L60: */
	    }
/* L70: */
	}

L80:
	i__2 = *m;
	for (k = i__; k <= i__2; ++k) {
	    u[k + i__ * u_dim1] = scale * u[k + i__ * u_dim1];
/* L90: */
	}

L100:
	w[i__] = scale * g;
	g = 0.f;
	s = 0.f;
	scale = 0.f;
	if (i__ > *m || i__ == *n) {
	    goto L190;
	}

	i__2 = *n;
	for (k = l; k <= i__2; ++k) {
	    scale += (r__1 = u[i__ + k * u_dim1], dabs(r__1));
/* L110: */
	}

	if (scale == 0.f) {
	    goto L190;
	}

	i__2 = *n;
	for (k = l; k <= i__2; ++k) {
	    u[i__ + k * u_dim1] /= scale;
/* Computing 2nd power */
	    r__1 = u[i__ + k * u_dim1];
	    s += r__1 * r__1;
/* L120: */
	}

	f = u[i__ + l * u_dim1];
	r__1 = sqrt(s);
	g = -r_sign(&r__1, &f);
	h__ = f * g - s;
	u[i__ + l * u_dim1] = f - g;

	i__2 = *n;
	for (k = l; k <= i__2; ++k) {
	    rv1[k] = u[i__ + k * u_dim1] / h__;
/* L130: */
	}

	if (i__ == *m) {
	    goto L170;
	}

	i__2 = *m;
	for (j = l; j <= i__2; ++j) {
	    s = 0.f;

	    i__3 = *n;
	    for (k = l; k <= i__3; ++k) {
		s += u[j + k * u_dim1] * u[i__ + k * u_dim1];
/* L140: */
	    }

	    i__3 = *n;
	    for (k = l; k <= i__3; ++k) {
		u[j + k * u_dim1] += s * rv1[k];
/* L150: */
	    }
/* L160: */
	}

L170:
	i__2 = *n;
	for (k = l; k <= i__2; ++k) {
	    u[i__ + k * u_dim1] = scale * u[i__ + k * u_dim1];
/* L180: */
	}

L190:
/* Computing MAX */
	r__3 = x, r__4 = (r__1 = w[i__], dabs(r__1)) + (r__2 = rv1[i__], dabs(
		r__2));
	x = dmax(r__3,r__4);
/* L200: */
    }
/*     ********** ACCUMULATION OF RIGHT-HAND TRANSFORMATIONS ********** */
    if (! (*matv)) {
	goto L290;
    }
/*     ********** FOR I=N STEP -1 UNTIL 1 DO -- ********** */
    i__1 = *n;
    for (ii = 1; ii <= i__1; ++ii) {
	i__ = *n + 1 - ii;
	if (i__ == *n) {
	    goto L270;
	}
	if (g == 0.f) {
	    goto L250;
	}

	i__2 = *n;
	for (j = l; j <= i__2; ++j) {
/*     ********** DOUBLE DIVISION AVOIDS POSSIBLE UNDERFLOW *****
***** */
	    v[j + i__ * v_dim1] = u[i__ + j * u_dim1] / u[i__ + l * u_dim1] / 
		    g;
/* L210: */
	}

	i__2 = *n;
	for (j = l; j <= i__2; ++j) {
	    s = 0.f;

	    i__3 = *n;
	    for (k = l; k <= i__3; ++k) {
		s += u[i__ + k * u_dim1] * v[k + j * v_dim1];
/* L220: */
	    }

	    i__3 = *n;
	    for (k = l; k <= i__3; ++k) {
		v[k + j * v_dim1] += s * v[k + i__ * v_dim1];
/* L230: */
	    }
/* L240: */
	}

L250:
	i__2 = *n;
	for (j = l; j <= i__2; ++j) {
	    v[i__ + j * v_dim1] = 0.f;
	    v[j + i__ * v_dim1] = 0.f;
/* L260: */
	}

L270:
	v[i__ + i__ * v_dim1] = 1.f;
	g = rv1[i__];
	l = i__;
/* L280: */
    }
/*     ********** ACCUMULATION OF LEFT-HAND TRANSFORMATIONS ********** */
L290:
    if (! (*matu)) {
	goto L410;
    }
/*     **********FOR I=MIN(M,N) STEP -1 UNTIL 1 DO -- ********** */
    mn = *n;
    if (*m < *n) {
	mn = *m;
    }

    i__1 = mn;
    for (ii = 1; ii <= i__1; ++ii) {
	i__ = mn + 1 - ii;
	l = i__ + 1;
	g = w[i__];
	if (i__ == *n) {
	    goto L310;
	}

	i__2 = *n;
	for (j = l; j <= i__2; ++j) {
	    u[i__ + j * u_dim1] = 0.f;
/* L300: */
	}

L310:
	if (g == 0.f) {
	    goto L370;
	}
	if (i__ == mn) {
	    goto L350;
	}

	i__2 = *n;
	for (j = l; j <= i__2; ++j) {
	    s = 0.f;

	    i__3 = *m;
	    for (k = l; k <= i__3; ++k) {
		s += u[k + i__ * u_dim1] * u[k + j * u_dim1];
/* L320: */
	    }
/*     ********** DOUBLE DIVISION AVOIDS POSSIBLE UNDERFLOW *****
***** */
	    f = s / u[i__ + i__ * u_dim1] / g;

	    i__3 = *m;
	    for (k = i__; k <= i__3; ++k) {
		u[k + j * u_dim1] += f * u[k + i__ * u_dim1];
/* L330: */
	    }
/* L340: */
	}

L350:
	i__2 = *m;
	for (j = i__; j <= i__2; ++j) {
	    u[j + i__ * u_dim1] /= g;
/* L360: */
	}

	goto L390;

L370:
	i__2 = *m;
	for (j = i__; j <= i__2; ++j) {
	    u[j + i__ * u_dim1] = 0.f;
/* L380: */
	}

L390:
	u[i__ + i__ * u_dim1] += 1.f;
/* L400: */
    }
/*     ********** DIAGONALIZATION OF THE BIDIAGONAL FORM ********** */
L410:
    eps = machep * x;
/*     ********** FOR K=N STEP -1 UNTIL 1 DO -- ********** */
    i__1 = *n;
    for (kk = 1; kk <= i__1; ++kk) {
	k1 = *n - kk;
	k = k1 + 1;
	its = 0;
/*     ********** TEST FOR SPLITTING. */
/*                FOR L=K STEP -1 UNTIL 1 DO -- ********** */
L420:
	i__2 = k;
	for (ll = 1; ll <= i__2; ++ll) {
	    l1 = k - ll;
	    l = l1 + 1;
	    if ((r__1 = rv1[l], dabs(r__1)) <= eps) {
		goto L470;
	    }
/*     ********** RV1(1) IS ALWAYS ZERO, SO THERE IS NO EXIT */
/*                THROUGH THE BOTTOM OF THE LOOP ********** */
	    if ((r__1 = w[l1], dabs(r__1)) <= eps) {
		goto L440;
	    }
/* L430: */
	}
/*     ********** CANCELLATION OF RV1(L) IF L GREATER THAN 1 ********
** */
L440:
	c__ = 0.f;
	s = 1.f;

	i__2 = k;
	for (i__ = l; i__ <= i__2; ++i__) {
	    f = s * rv1[i__];
	    rv1[i__] = c__ * rv1[i__];
	    if (dabs(f) <= eps) {
		goto L470;
	    }
	    g = w[i__];
	    h__ = sqrt(f * f + g * g);
	    w[i__] = h__;
	    c__ = g / h__;
	    s = -f / h__;
	    if (! (*matu)) {
		goto L460;
	    }

	    i__3 = *m;
	    for (j = 1; j <= i__3; ++j) {
		y = u[j + l1 * u_dim1];
		z__ = u[j + i__ * u_dim1];
		u[j + l1 * u_dim1] = y * c__ + z__ * s;
		u[j + i__ * u_dim1] = -y * s + z__ * c__;
/* L450: */
	    }

L460:
	    ;
	}
/*     ********** TEST FOR CONVERGENCE ********** */
L470:
	z__ = w[k];
	if (l == k) {
	    goto L530;
	}
/*     ********** SHIFT FROM BOTTOM 2 BY 2 MINOR ********** */
	if (its == 30) {
	    goto L560;
	}
	++its;
	x = w[l];
	y = w[k1];
	g = rv1[k1];
	h__ = rv1[k];
	f = ((y - z__) * (y + z__) + (g - h__) * (g + h__)) / (h__ * 2.f * y);
	g = sqrt(f * f + 1.f);
	f = ((x - z__) * (x + z__) + h__ * (y / (f + r_sign(&g, &f)) - h__)) /
		 x;
/*     ********** NEXT QR TRANSFORMATION ********** */
	c__ = 1.f;
	s = 1.f;

	i__2 = k1;
	for (i1 = l; i1 <= i__2; ++i1) {
	    i__ = i1 + 1;
	    g = rv1[i__];
	    y = w[i__];
	    h__ = s * g;
	    g = c__ * g;
	    z__ = sqrt(f * f + h__ * h__);
	    rv1[i1] = z__;
	    c__ = f / z__;
	    s = h__ / z__;
	    f = x * c__ + g * s;
	    g = -x * s + g * c__;
	    h__ = y * s;
	    y *= c__;
	    if (! (*matv)) {
		goto L490;
	    }

	    i__3 = *n;
	    for (j = 1; j <= i__3; ++j) {
		x = v[j + i1 * v_dim1];
		z__ = v[j + i__ * v_dim1];
		v[j + i1 * v_dim1] = x * c__ + z__ * s;
		v[j + i__ * v_dim1] = -x * s + z__ * c__;
/* L480: */
	    }

L490:
	    z__ = sqrt(f * f + h__ * h__);
	    w[i1] = z__;
/*     ********** ROTATION CAN BE ARBITRARY IF Z IS ZERO ********
** */
	    if (z__ == 0.f) {
		goto L500;
	    }
	    c__ = f / z__;
	    s = h__ / z__;
L500:
	    f = c__ * g + s * y;
	    x = -s * g + c__ * y;
	    if (! (*matu)) {
		goto L520;
	    }

	    i__3 = *m;
	    for (j = 1; j <= i__3; ++j) {
		y = u[j + i1 * u_dim1];
		z__ = u[j + i__ * u_dim1];
		u[j + i1 * u_dim1] = y * c__ + z__ * s;
		u[j + i__ * u_dim1] = -y * s + z__ * c__;
/* L510: */
	    }

L520:
	    ;
	}

	rv1[l] = 0.f;
	rv1[k] = f;
	w[k] = x;
	goto L420;
/*     ********** CONVERGENCE ********** */
L530:
	if (z__ >= 0.f) {
	    goto L550;
	}
/*     ********** W(K) IS MADE NON-NEGATIVE ********** */
	w[k] = -z__;
	if (! (*matv)) {
	    goto L550;
	}

	i__2 = *n;
	for (j = 1; j <= i__2; ++j) {
	    v[j + k * v_dim1] = -v[j + k * v_dim1];
/* L540: */
	}

L550:
	;
    }

    goto L570;
/*     ********** SET ERROR -- NO CONVERGENCE TO A */
/*                SINGULAR VALUE AFTER 30 ITERATIONS ********** */
L560:
    *ierr = k;
L570:
    return 0;
/*     ********** LAST CARD OF SVD ********** */
} /* svd_ */


/*     ------------------------------------------------------------------ */

/* Subroutine */ int sswap_(integer *n, real *sx, integer *incx, real *sy, 
	integer *incy)
{
    /* System generated locals */
    integer i__1;

    /* Local variables */
    static integer i__, m;
    static real stemp;
    static integer ix, iy, mp1;


/*     INTERCHANGES TWO VECTORS. */
/*     USES UNROLLED LOOPS FOR INCREMENTS EQUAL TO 1. */
/*     JACK DONGARRA, LINPACK, 3/11/78. */


    /* Parameter adjustments */
    --sy;
    --sx;

    /* Function Body */
    if (*n <= 0) {
	return 0;
    }
    if (*incx == 1 && *incy == 1) {
	goto L20;
    }

/*       CODE FOR UNEQUAL INCREMENTS OR EQUAL INCREMENTS NOT EQUAL */
/*         TO 1 */

    ix = 1;
    iy = 1;
    if (*incx < 0) {
	ix = (-(*n) + 1) * *incx + 1;
    }
    if (*incy < 0) {
	iy = (-(*n) + 1) * *incy + 1;
    }
    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
	stemp = sx[ix];
	sx[ix] = sy[iy];
	sy[iy] = stemp;
	ix += *incx;
	iy += *incy;
/* L10: */
    }
    return 0;

/*       CODE FOR BOTH INCREMENTS EQUAL TO 1 */


/*       CLEAN-UP LOOP */

L20:
    m = *n % 3;
    if (m == 0) {
	goto L40;
    }
    i__1 = m;
    for (i__ = 1; i__ <= i__1; ++i__) {
	stemp = sx[i__];
	sx[i__] = sy[i__];
	sy[i__] = stemp;
/* L30: */
    }
    if (*n < 3) {
	return 0;
    }
L40:
    mp1 = m + 1;
    i__1 = *n;
    for (i__ = mp1; i__ <= i__1; i__ += 3) {
	stemp = sx[i__];
	sx[i__] = sy[i__];
	sy[i__] = stemp;
	stemp = sx[i__ + 1];
	sx[i__ + 1] = sy[i__ + 1];
	sy[i__ + 1] = stemp;
	stemp = sx[i__ + 2];
	sx[i__ + 2] = sy[i__ + 2];
	sy[i__ + 2] = stemp;
/* L50: */
    }
    return 0;
} /* sswap_ */

