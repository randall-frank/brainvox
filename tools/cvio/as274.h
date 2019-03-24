/* prototypes from the Regression code in 274.f */
/* AS274 */

void remove_(int *np, int *nrbar, double *weight, double *xrow, double *yelem, double *d__, double *rbar, 
	double *thetab, double *sserr, int *ier);

/*
      SUBROUTINE SS(NP, D, THETAB, SSERR, RSS, IER)
      INTEGER NP, IER
      DOUBLE PRECISION D(NP), THETAB(NP), SSERR, RSS(NP)
*/
void ss_(int *np,double *d,double *thetab,double *sserr,double *rss,
	int *ier);

/*
      SUBROUTINE HDIAG(XROW, NP, NRBAR, D, RBAR, TOL, NREQ, HII, WK,
     +     IFAULT)
      INTEGER NP, NRBAR, NREQ, IFAULT
      DOUBLE PRECISION XROW(NP), D(NP), RBAR(*), TOL(NP), HII, WK(NP)
*/
void hdiag_(double *XROW,int *NP,int *NRBAR,double *D,double *RBAR,
	double *TOL,int *NREQ, double *HII, double *WK, int *IFAULT);

/*
      SUBROUTINE VMOVE(NP, NRBAR, VORDER, D, RBAR, THETAB, RSS, FROM,
     +    TO, TOL, IER)
      INTEGER NP, NRBAR, VORDER(NP), FROM, TO, IER
      DOUBLE PRECISION D(NP), RBAR(*), THETAB(NP), RSS(NP), TOL(NP)
*/
void vmove_(int *NP, int *NRBAR, int *VORDER, double *D,
	double *RBAR, double *THETAB, double *RSS, int *FROM,
	int *TO, double *TOL, int *IER);

/*
      SUBROUTINE INCLUD(NP, NRBAR, WEIGHT, XROW, YELEM, D, RBAR, THETAB,
     +      SSERR, IER)
      INTEGER NP, NRBAR, IER
      DOUBLE PRECISION WEIGHT, XROW(NP), YELEM, D(NP), RBAR(*),
     +       THETAB(NP), SSERR
*/
void includ_(int *NP, int *NRBAR, double *WEIGHT, double *XROW,
	double *YELEM, double *D, double *RBAR, double *THETAB, double *SSERR,
	int *IER);

/*
      SUBROUTINE TOLSET(NP, NRBAR, D, RBAR, TOL, WORK, IER)
      INTEGER NP, NRBAR, IER
      DOUBLE PRECISION D(NP), RBAR(*), TOL(NP), WORK(NP)
*/
void tolset_(int *NP, int *NRBAR, double *D, double *RBAR, 
	double *TOL, double *WORK, int *IER);

/*
      SUBROUTINE SING(NP, NRBAR, D, RBAR, THETAB, SSERR, TOL, LINDEP,
     +   WORK, IER)
      INTEGER NP, NRBAR, IER
      DOUBLE PRECISION D(NP), RBAR(NRBAR), THETAB(NP), SSERR, TOL(NP),
     +        WORK(NP)
      LOGICAL LINDEP(NP)
*/
void sing_(int *NP, int *NRBAR, double *D, double *RBAR,
	double *THETAB, double *SSERR, double *TOL, int *LINDEP,
	double *WORK, int *IER);

/*
      SUBROUTINE REGCF(NP, NRBAR, D, RBAR, THETAB, TOL, BETA, NREQ,
     +     IER)
      INTEGER NP, NRBAR, NREQ, IER
      DOUBLE PRECISION D(NP), RBAR(*), THETAB(NP), TOL(NP), BETA(NP)
*/
void regcf_(int *NP, int *NRBAR, double *D, double *RBAR,
	double *THETAB, double *TOL, double *BETA, int *NREQ,
	int *IER);

/*
      SUBROUTINE CLEAR(NP, NRBAR, D, RBAR, THETAB, SSERR, IER)
      INTEGER NP, NRBAR, IER
      DOUBLE PRECISION D(NP), RBAR(*), THETAB(NP), SSERR
*/
void clear_(int *NP, int *NRBAR, double *D, double *RBAR,
	double *THETAB, double *SSERR, int *IER);

/*
      SUBROUTINE COV(NP, NRBAR, D, RBAR, NREQ, RINV, VAR, COVMAT,
     +      DIMCOV, STERR, IER)
      INTEGER NP, NRBAR, NREQ, DIMCOV, IER
      DOUBLE PRECISION D(NP), RBAR(*), RINV(*), VAR, COVMAT(DIMCOV),
     +       STERR(NP)
*/
void cov_(int *NP, int *NRBAR, double *D, double *RBAR,
	int *NREQ, double *RINV, double *VAR, double *COVMAT,
	int *DIMCOV, double *STERR, int *IER);

/*
      SUBROUTINE INV(NP, NRBAR, RBAR, NREQ, RINV)
      INTEGER NP, NRBAR, NREQ
      DOUBLE PRECISION RBAR(*), RINV(*)
*/
void inv_(int *NP, int *NRBAR, double *RBAR, int *NREQ,
	double *RINV);

/*
      SUBROUTINE PCORR(NP, NRBAR, D, RBAR, THETAB, SSERR, IN, WORK,
     +      CORMAT, DIMC, YCORR, IER)
      INTEGER NP, NRBAR, IN, DIMC, IER
      DOUBLE PRECISION D(NP), RBAR(*), THETAB(NP), SSERR, WORK(NP),
     +        CORMAT(*), YCORR
*/
void pcorr_(int *NP, int *NRBAR, double *D, double *RBAR,
	double *THETAB, double *SSERR, int *IN, double *WORK,
	double *CORMAT, int *DIMC, double *YCORR, int *IER);

/*
      SUBROUTINE COR(NP, D, RBAR, THETAB, SSERR, WORK, CORMAT, YCORR)
      INTEGER NP
      DOUBLE PRECISION D(NP), RBAR(*), THETAB(NP), SSERR, WORK(NP),
     +      CORMAT(*), YCORR(NP)
*/
void cor_(int *NP, double *D, double *RBAR, double *THETAB,
	double *SSERR, double *WORK, double *CORMAT, double *YCORR);

/*
      SUBROUTINE REORDR(NP, NRBAR, VORDER, D, RBAR, THETAB, RSS, TOL,
     +      LIST, N, POS1, IER)
      INTEGER NP, NRBAR, VORDER(NP), N, LIST(N), POS1, IER
      DOUBLE PRECISION D(NP), RBAR(NRBAR), THETAB(NP), RSS(NP), TOL(NP)
*/
void reordr_(int *NP, int *NRBAR, int *VORDER, double *D,
	double *RBAR, double *THETAB, double *RSS, double *TOL,
	int *LIST, int *N, int *POS1, int *IER);

