/* prototypes from the Regression code in 274.f */
/* AS274 */

void remove_(long int *np, long int *nrbar, double *weight, double *xrow, double *yelem, double *d__, double *rbar, 
	double *thetab, double *sserr, long int *ier);

/*
      SUBROUTINE SS(NP, D, THETAB, SSERR, RSS, IER)
      INTEGER NP, IER
      DOUBLE PRECISION D(NP), THETAB(NP), SSERR, RSS(NP)
*/
void ss_(long int *np,double *d,double *thetab,double *sserr,double *rss,
	long int *ier);

/*
      SUBROUTINE HDIAG(XROW, NP, NRBAR, D, RBAR, TOL, NREQ, HII, WK,
     +     IFAULT)
      INTEGER NP, NRBAR, NREQ, IFAULT
      DOUBLE PRECISION XROW(NP), D(NP), RBAR(*), TOL(NP), HII, WK(NP)
*/
void hdiag_(double *XROW,long int *NP,long int *NRBAR,double *D,double *RBAR,
	double *TOL,long int *NREQ, double *HII, double *WK, long int *IFAULT);

/*
      SUBROUTINE VMOVE(NP, NRBAR, VORDER, D, RBAR, THETAB, RSS, FROM,
     +    TO, TOL, IER)
      INTEGER NP, NRBAR, VORDER(NP), FROM, TO, IER
      DOUBLE PRECISION D(NP), RBAR(*), THETAB(NP), RSS(NP), TOL(NP)
*/
void vmove_(long int *NP, long int *NRBAR, long int *VORDER, double *D,
	double *RBAR, double *THETAB, double *RSS, long int *FROM,
	long int *TO, double *TOL, long int *IER);

/*
      SUBROUTINE INCLUD(NP, NRBAR, WEIGHT, XROW, YELEM, D, RBAR, THETAB,
     +      SSERR, IER)
      INTEGER NP, NRBAR, IER
      DOUBLE PRECISION WEIGHT, XROW(NP), YELEM, D(NP), RBAR(*),
     +       THETAB(NP), SSERR
*/
void includ_(long int *NP, long int *NRBAR, double *WEIGHT, double *XROW,
	double *YELEM, double *D, double *RBAR, double *THETAB, double *SSERR,
	long int *IER);

/*
      SUBROUTINE TOLSET(NP, NRBAR, D, RBAR, TOL, WORK, IER)
      INTEGER NP, NRBAR, IER
      DOUBLE PRECISION D(NP), RBAR(*), TOL(NP), WORK(NP)
*/
void tolset_(long int *NP, long int *NRBAR, double *D, double *RBAR, 
	double *TOL, double *WORK, long int *IER);

/*
      SUBROUTINE SING(NP, NRBAR, D, RBAR, THETAB, SSERR, TOL, LINDEP,
     +   WORK, IER)
      INTEGER NP, NRBAR, IER
      DOUBLE PRECISION D(NP), RBAR(NRBAR), THETAB(NP), SSERR, TOL(NP),
     +        WORK(NP)
      LOGICAL LINDEP(NP)
*/
void sing_(long int *NP, long int *NRBAR, double *D, double *RBAR,
	double *THETAB, double *SSERR, double *TOL, long int *LINDEP,
	double *WORK, long int *IER);

/*
      SUBROUTINE REGCF(NP, NRBAR, D, RBAR, THETAB, TOL, BETA, NREQ,
     +     IER)
      INTEGER NP, NRBAR, NREQ, IER
      DOUBLE PRECISION D(NP), RBAR(*), THETAB(NP), TOL(NP), BETA(NP)
*/
void regcf_(long int *NP, long int *NRBAR, double *D, double *RBAR,
	double *THETAB, double *TOL, double *BETA, long int *NREQ,
	long int *IER);

/*
      SUBROUTINE CLEAR(NP, NRBAR, D, RBAR, THETAB, SSERR, IER)
      INTEGER NP, NRBAR, IER
      DOUBLE PRECISION D(NP), RBAR(*), THETAB(NP), SSERR
*/
void clear_(long int *NP, long int *NRBAR, double *D, double *RBAR,
	double *THETAB, double *SSERR, long int *IER);

/*
      SUBROUTINE COV(NP, NRBAR, D, RBAR, NREQ, RINV, VAR, COVMAT,
     +      DIMCOV, STERR, IER)
      INTEGER NP, NRBAR, NREQ, DIMCOV, IER
      DOUBLE PRECISION D(NP), RBAR(*), RINV(*), VAR, COVMAT(DIMCOV),
     +       STERR(NP)
*/
void cov_(long int *NP, long int *NRBAR, double *D, double *RBAR,
	long int *NREQ, double *RINV, double *VAR, double *COVMAT,
	long int *DIMCOV, double *STERR, long int *IER);

/*
      SUBROUTINE INV(NP, NRBAR, RBAR, NREQ, RINV)
      INTEGER NP, NRBAR, NREQ
      DOUBLE PRECISION RBAR(*), RINV(*)
*/
void inv_(long int *NP, long int *NRBAR, double *RBAR, long int *NREQ,
	double *RINV);

/*
      SUBROUTINE PCORR(NP, NRBAR, D, RBAR, THETAB, SSERR, IN, WORK,
     +      CORMAT, DIMC, YCORR, IER)
      INTEGER NP, NRBAR, IN, DIMC, IER
      DOUBLE PRECISION D(NP), RBAR(*), THETAB(NP), SSERR, WORK(NP),
     +        CORMAT(*), YCORR
*/
void pcorr_(long int *NP, long int *NRBAR, double *D, double *RBAR,
	double *THETAB, double *SSERR, long int *IN, double *WORK,
	double *CORMAT, long int *DIMC, double *YCORR, long int *IER);

/*
      SUBROUTINE COR(NP, D, RBAR, THETAB, SSERR, WORK, CORMAT, YCORR)
      INTEGER NP
      DOUBLE PRECISION D(NP), RBAR(*), THETAB(NP), SSERR, WORK(NP),
     +      CORMAT(*), YCORR(NP)
*/
void cor_(long int *NP, double *D, double *RBAR, double *THETAB,
	double *SSERR, double *WORK, double *CORMAT, double *YCORR);

/*
      SUBROUTINE REORDR(NP, NRBAR, VORDER, D, RBAR, THETAB, RSS, TOL,
     +      LIST, N, POS1, IER)
      INTEGER NP, NRBAR, VORDER(NP), N, LIST(N), POS1, IER
      DOUBLE PRECISION D(NP), RBAR(NRBAR), THETAB(NP), RSS(NP), TOL(NP)
*/
void reordr_(long int *NP, long int *NRBAR, long int *VORDER, double *D,
	double *RBAR, double *THETAB, double *RSS, double *TOL,
	long int *LIST, long int *N, long int *POS1, long int *IER);

