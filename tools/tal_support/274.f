
C-----------------------------------------------------------------------

      SUBROUTINE REMOVE(NP, NRBAR, WEIGHT, XROW, YELEM, D, RBAR, THETAB,
     +      SSERR, IER)

C     *** WARNING ***   The elements of XROW are over-written.
C
      INTEGER NP, NRBAR, IER
      DOUBLE PRECISION WEIGHT, XROW(NP), YELEM, D(NP), RBAR(*),
     +       THETAB(NP), SSERR
C
C     Local variables
C
      INTEGER I, K, NEXTR
      DOUBLE PRECISION ZERO, W, Y, XI, DI, WXI, DPI, CBAR, SBAR, XK
C
      DATA ZERO/0.D0/
C
C     Some checks.
C
      IER = 0
      IF (NP .LT. 1) IER = 1
      IF (NRBAR .LT. NP*(NP-1)/2) IER = IER + 2
      IF (IER .NE. 0) RETURN
C
      W = WEIGHT
      Y = YELEM
      NEXTR = 1
      IF (W .EQ. ZERO) RETURN
      DO 30 I =1, NP
        XI = XROW(I)
	DPI = D(I)
        WXI = W * XI
        D(I) = D(I) - WXI*XI
        DI = D(I)
        CBAR = DI / DPI
        SBAR = WXI / DPI
        W = CBAR * W
        IF (I .EQ. NP) GO TO 20
        DO 10 K = I+1, NP
          XK = XROW(K)
          RBAR(NEXTR) = (RBAR(NEXTR) - (SBAR * XK))/CBAR
          XROW(K) = XK - XI * RBAR(NEXTR)
          NEXTR = NEXTR + 1
   10   CONTINUE
   20   XK = Y
        THETAB(I) = (THETAB(I) - (SBAR * XK))/CBAR
        Y = XK - XI * THETAB(I)
   30 CONTINUE
C
      SSERR = SSERR - (W * Y * Y)
C
      RETURN
      END

C-----------------------------------------------------------------------

      SUBROUTINE INCLUD(NP, NRBAR, WEIGHT, XROW, YELEM, D, RBAR, THETAB,
     +      SSERR, IER)
C
C     ALGORITHM AS274.1  APPL. STATIST. (1992) VOL 41, NO. 2
C
C     DOUBLE PRECISION VERSION
C
C     Calling this routine updates d, rbar, thetab and sserr by the
C     inclusion of xrow, yelem with the specified weight.
C     This version has been modified to make it slightly faster when the
C     early elements of XROW are not zeroes.
C
C     *** WARNING ***   The elements of XROW are over-written.
C
      INTEGER NP, NRBAR, IER
      DOUBLE PRECISION WEIGHT, XROW(NP), YELEM, D(NP), RBAR(*),
     +       THETAB(NP), SSERR
C
C     Local variables
C
      INTEGER I, K, NEXTR
      DOUBLE PRECISION ZERO, W, Y, XI, DI, WXI, DPI, CBAR, SBAR, XK
C
      DATA ZERO/0.D0/
C
C     Some checks.
C
      IER = 0
      IF (NP .LT. 1) IER = 1
      IF (NRBAR .LT. NP*(NP-1)/2) IER = IER + 2
      IF (IER .NE. 0) RETURN
C
      W = WEIGHT
      Y = YELEM
      NEXTR = 1
      DO 30 I = 1, NP
C
C     Skip unnecessary transformations.   Test on exact zeroes must be
C     used or stability can be destroyed.
C
	IF (W .EQ. ZERO) RETURN
	XI = XROW(I)
	IF (XI .EQ. ZERO) THEN
	  NEXTR = NEXTR + NP - I
	  GO TO 30
	END IF
	DI = D(I)
	WXI = W * XI
	DPI = DI + WXI*XI
	CBAR = DI / DPI
	SBAR = WXI / DPI
	W = CBAR * W
	D(I) = DPI
	IF (I .EQ. NP) GO TO 20
	DO 10 K = I+1, NP
	  XK = XROW(K)
	  XROW(K) = XK - XI * RBAR(NEXTR)
	  RBAR(NEXTR) = CBAR * RBAR(NEXTR) + SBAR * XK
	  NEXTR = NEXTR + 1
   10   CONTINUE
   20   XK = Y
	Y = XK - XI * THETAB(I)
	THETAB(I) = CBAR * THETAB(I) + SBAR * XK
   30 CONTINUE
C
C     Y * SQRT(W) is now equal to the Brown, Durbin & Evans recursive
C     residual.
C
      SSERR = SSERR + W * Y * Y
C
      RETURN
      END
C
      SUBROUTINE CLEAR(NP, NRBAR, D, RBAR, THETAB, SSERR, IER)
C
C     ALGORITHM AS274.2  APPL. STATIST. (1992) VOL.41, NO.2
C
C     Sets arrays to zero prior to calling AS75.1
C
      INTEGER NP, NRBAR, IER
      DOUBLE PRECISION D(NP), RBAR(*), THETAB(NP), SSERR
C
C     Local variables
C
      INTEGER I
      DOUBLE PRECISION ZERO
C
      DATA ZERO/0.D0/
C
C     Some checks.
C
      IER = 0
      IF (NP .LT. 1) IER = 1
      IF (NRBAR .LT. NP*(NP-1)/2) IER = IER + 2
      IF (IER .NE. 0) RETURN
C
      DO 10 I = 1, NP
	D(I) = ZERO
	THETAB(I) = ZERO
   10 CONTINUE
      DO 20 I = 1, NRBAR
   20 RBAR(I) = ZERO
      SSERR = ZERO
      RETURN
      END
C
      SUBROUTINE REGCF(NP, NRBAR, D, RBAR, THETAB, TOL, BETA, NREQ,
     +     IER)
C
C     ALGORITHM AS274.3  APPL. STATIST. (1992) VOL 41, NO.2
C
C     Modified version of AS75.4 to calculate regression coefficients
C     for the first NREQ variables, given an orthogonal reduction from
C     AS75.1.
C
      INTEGER NP, NRBAR, NREQ, IER
      DOUBLE PRECISION D(NP), RBAR(*), THETAB(NP), TOL(NP), BETA(NP)
C
C     Local variables
C
      INTEGER I, J, NEXTR
      DOUBLE PRECISION ZERO
C
      DATA ZERO/0.D0/
C
C     Some checks.
C
      IER = 0
      IF (NP .LT. 1) IER = 1
      IF (NRBAR .LT. NP*(NP-1)/2) IER = IER + 2
      IF (NREQ .LT. 1 .OR. NREQ .GT. NP) IER = IER + 4
      IF (IER .NE. 0) RETURN
C
      DO 20 I = NREQ, 1, -1
	IF (SQRT(D(I)) .LT. TOL(I)) THEN
	  BETA(I) = ZERO
	  D(I) = ZERO
	  GO TO 20
	END IF
	BETA(I) = THETAB(I)
	NEXTR = (I-1) * (NP+NP-I)/2 + 1
	DO 10 J = I+1, NREQ
	  BETA(I) = BETA(I) - RBAR(NEXTR) * BETA(J)
	  NEXTR = NEXTR + 1
   10   CONTINUE
   20 CONTINUE
C
      RETURN
      END
C
      SUBROUTINE TOLSET(NP, NRBAR, D, RBAR, TOL, WORK, IER)
C
C     ALGORITHM AS274.4  APPL. STATIST. (1992) VOL 41, NO.2
C
C     Sets up array TOL for testing for zeroes in an orthogonal
C     reduction formed using AS75.1.
C
      INTEGER NP, NRBAR, IER
      DOUBLE PRECISION D(NP), RBAR(*), TOL(NP), WORK(NP)
C
C     Local variables.
C
      INTEGER COL, ROW, POS
      DOUBLE PRECISION EPS, SUM
C
C     EPS is a machine-dependent constant.   For compilers which use
C     the IEEE format for floating-point numbers, recommended values
C     are 1.E-06 for single precision and 1.E-15 for double precision.
C
      DATA EPS/1.D-15/
C
C     Some checks.
C
      IER = 0
      IF (NP .LT. 1) IER = 1
      IF (NRBAR .LT. NP*(NP-1)/2) IER = IER + 2
      IF (IER .NE. 0) RETURN
C
C     Set TOL(I) = sum of absolute values in column I of RBAR after
C     scaling each element by the square root of its row multiplier.
C
      DO 10 COL = 1, NP
   10 WORK(COL) = SQRT(D(COL))
      DO 30 COL = 1, NP
	POS = COL - 1
	SUM = WORK(COL)
	DO 20 ROW = 1, COL-1
	  SUM = SUM + ABS(RBAR(POS)) * WORK(ROW)
	  POS = POS + NP - ROW - 1
  20    CONTINUE
	TOL(COL) = EPS * SUM
   30 CONTINUE
C
      RETURN
      END
C
      SUBROUTINE SING(NP, NRBAR, D, RBAR, THETAB, SSERR, TOL, LINDEP,
     +   WORK, IER)
C
C     ALGORITHM AS274.5  APPL. STATIST. (1992) VOL 41, NO.2
C
C     Checks for singularities, reports, and adjusts orthogonal
C     reductions produced by AS75.1.
C
      INTEGER NP, NRBAR, IER
      DOUBLE PRECISION D(NP), RBAR(NRBAR), THETAB(NP), SSERR, TOL(NP),
     +        WORK(NP)
      LOGICAL LINDEP(NP)
C
C     Local variables
C
      DOUBLE PRECISION ZERO, TEMP
      INTEGER COL, POS, ROW, NP2, POS2
C
      DATA ZERO/0.D0/
C
C     Check input parameters
C
      IER = 0
      IF (NP .LE. 0) IER = 1
      IF (NRBAR .LT. NP*(NP-1)/2) IER = IER + 2
      IF (IER .NE. 0) RETURN
C
      DO 10 COL = 1, NP
   10 WORK(COL) = SQRT(D(COL))
C
      DO 40 COL = 1, NP
C
C     Set elements within RBAR to zero if they are less than TOL(COL) in
C     absolute value after being scaled by the square root of their row
C     multiplier.
C
	TEMP = TOL(COL)
	POS = COL - 1
	DO 30 ROW = 1, COL-1
	  IF (ABS(RBAR(POS)) * WORK(ROW) .LT. TEMP) RBAR(POS) = ZERO
	  POS = POS + NP - ROW - 1
   30   CONTINUE
C
C     If diagonal element is near zero, set it to zero, set appropriate
C     element of LINDEP, and use INCLUD to augment the projections in
C     the lower rows of the orthogonalization.
C
	LINDEP(COL) = .FALSE.
	IF (WORK(COL) .LT. TEMP) THEN
	  LINDEP(COL) = .TRUE.
	  IER = IER - 1
	  IF (COL .LT. NP) THEN
	    NP2 = NP - COL
	    POS2 = POS + NP - COL + 1
	    CALL INCLUD(NP2, NP2*(NP2-1)/2, D(COL), RBAR(POS+1),
     +            THETAB(COL), D(COL+1), RBAR(POS2), THETAB(COL+1),
     +            SSERR, IER)
	  ELSE
	    SSERR = SSERR + D(COL) * THETAB(COL)**2
	  END IF
	  D(COL) = ZERO
	  WORK(COL) = ZERO
	  THETAB(COL) = ZERO
	END IF
   40 CONTINUE
      RETURN
      END
C
      SUBROUTINE SS(NP, D, THETAB, SSERR, RSS, IER)
C
C     ALGORITHM AS274.6  APPL. STATIST. (1992) VOL 41, NO.2
C
C     Calculates partial residual sums of squares from an orthogonal
C     reduction from AS75.1.
C
      INTEGER NP, IER
      DOUBLE PRECISION D(NP), THETAB(NP), SSERR, RSS(NP)
C
C     Local variables
C
      INTEGER I
      DOUBLE PRECISION SUM
C
C     Some checks.
C
      IER = 0
      IF (NP .LT. 1) IER = 1
      IF (IER .NE. 0) RETURN
C
      SUM = SSERR
      RSS(NP) = SSERR
      DO 10 I = NP, 2, -1
	SUM = SUM + D(I) * THETAB(I)**2
	RSS(I-1) = SUM
   10 CONTINUE
      RETURN
      END
C
      SUBROUTINE COV(NP, NRBAR, D, RBAR, NREQ, RINV, VAR, COVMAT,
     +      DIMCOV, STERR, IER)
C
C     ALGORITHM AS274.7  APPL. STATIST. (1992) VOL 41, NO.2
C
C     Calculate covariance matrix for regression coefficients for the
C     first NREQ variables, from an orthogonal reduction produced from
C     AS75.1.
C
C     Auxiliary routine called: INV
C
      INTEGER NP, NRBAR, NREQ, DIMCOV, IER
      DOUBLE PRECISION D(NP), RBAR(*), RINV(*), VAR, COVMAT(DIMCOV),
     +       STERR(NP)
C
C     Local variables.
C
      INTEGER POS, ROW, START, POS2, COL, POS1, K
      DOUBLE PRECISION ZERO, ONE, SUM
C
      DATA ZERO/0.D0/, ONE/1.D0/
C
C     Some checks.
C
      IER = 0
      IF (NP .LT. 1) IER = 1
      IF (NRBAR .LT. NP*(NP-1)/2) IER = IER + 2
      IF (DIMCOV .LT. NREQ*(NREQ+1)/2) IER = IER + 4
      DO 10 ROW = 1, NREQ
	IF (D(ROW) .EQ. ZERO) IER = -ROW
   10 CONTINUE
      IF (IER .NE. 0) RETURN
C
      CALL INV(NP, NRBAR, RBAR, NREQ, RINV)
      POS = 1
      START = 1
      DO 40 ROW = 1, NREQ
	POS2 = START
	DO 30 COL = ROW, NREQ
	  POS1 = START + COL - ROW
	  IF (ROW .EQ. COL) THEN
	    SUM = ONE / D(COL)
	  ELSE
	    SUM = RINV(POS1-1) / D(COL)
	  END IF
	  DO 20 K = COL+1, NREQ
	    SUM = SUM + RINV(POS1) * RINV(POS2) / D(K)
	    POS1 = POS1 + 1
	    POS2 = POS2 + 1
   20     CONTINUE
	  COVMAT(POS) = SUM * VAR
	  IF (ROW .EQ. COL) STERR(ROW) = SQRT(COVMAT(POS))
	  POS = POS + 1
   30   CONTINUE
	START = START + NREQ - ROW
   40 CONTINUE
C
      RETURN
      END
C
      SUBROUTINE INV(NP, NRBAR, RBAR, NREQ, RINV)
C
C     ALGORITHM AS274.8  APPL. STATIST. (1992) VOL 41, NO.2
C
C     Invert first NREQ rows and columns of Cholesky factorization
C     produced by AS75.1.
C
      INTEGER NP, NRBAR, NREQ
      DOUBLE PRECISION RBAR(*), RINV(*)
C
C     Local variables.
C
      INTEGER POS, ROW, COL, START, K, POS1, POS2
      DOUBLE PRECISION SUM, ZERO
C
      DATA ZERO/0.D0/
C
C     Invert RBAR ignoring row multipliers, from the bottom up.
C
      POS = NREQ * (NREQ-1)/2
      DO 30 ROW = NREQ-1, 1, -1
	START = (ROW-1) * (NP+NP-ROW)/2 + 1
	DO 20 COL = NREQ, ROW+1, -1
	  POS1 = START
	  POS2 = POS
	  SUM = ZERO
	  DO 10 K = ROW+1, COL-1
	    POS2 = POS2 + NREQ - K
	    SUM = SUM - RBAR(POS1) * RINV(POS2)
	    POS1 = POS1 + 1
   10     CONTINUE
	  RINV(POS) = SUM - RBAR(POS1)
	  POS = POS - 1
   20   CONTINUE
   30 CONTINUE
C
      RETURN
      END
C
      SUBROUTINE PCORR(NP, NRBAR, D, RBAR, THETAB, SSERR, IN, WORK,
     +      CORMAT, DIMC, YCORR, IER)
C
C     ALGORITHM AS274.9  APPL. STATIST. (1992) VOL 41, NO.2
C
C     Calculate partial correlations after the first IN variables
C     have been forced into the regression.
C
C     Auxiliary routine called: COR
C
      INTEGER NP, NRBAR, IN, DIMC, IER
      DOUBLE PRECISION D(NP), RBAR(*), THETAB(NP), SSERR, WORK(NP),
     +        CORMAT(*), YCORR
C
C     Local variables.
C
      INTEGER START, IN1, I
      DOUBLE PRECISION ZERO
C
      DATA ZERO/0.D0/
C
C     Some checks.
C
      IER = 0
      IF (NP .LT. 1) IER = 1
      IF (NRBAR .LT. NP*(NP-1)/2) IER = IER + 2
      IF (IN .LT. 0 .OR. IN .GT. NP-1) IER = IER + 4
      IF (DIMC .LT. (NP-IN)*(NP-IN-1)/2) IER = IER + 8
      IF (IER .NE. 0) RETURN
C
      START = IN * (NP+NP-IN-1)/2 + 1
      IN1 = IN + 1
      CALL COR(NP-IN, D(IN1), RBAR(START), THETAB(IN1), SSERR, WORK,
     +      CORMAT, YCORR)
C
C     Check for zeroes.
C
      DO 10 I = 1, NP-IN
	IF (WORK(I) .LE. ZERO) IER = -I
   10 CONTINUE
C
      RETURN
      END
C
      SUBROUTINE COR(NP, D, RBAR, THETAB, SSERR, WORK, CORMAT, YCORR)
C
C     ALGORITHM AS274.10  APPL. STATIST. (1992) VOL 41, NO.2
C
C     Calculate correlations from an orthogonal reduction.   This
C     routine will usually be called from PCORR, which will have
C     removed the appropriate number of rows at the start.
C
      INTEGER NP
      DOUBLE PRECISION D(NP), RBAR(*), THETAB(NP), SSERR, WORK(NP),
     +      CORMAT(*), YCORR(NP)
C
C     Local variables.
C
      INTEGER ROW, POS, COL1, POS1, COL2, POS2, DIFF
      DOUBLE PRECISION SUMY, SUM, ZERO
C
      DATA ZERO/0.D0/
C
C     Process by columns, including the projections of the dependent
C     variable (THETAB).
C
      SUMY = SSERR
      DO 10 ROW = 1, NP
   10 SUMY = SUMY + D(ROW) * THETAB(ROW)**2
      SUMY = SQRT(SUMY)
      POS = NP*(NP-1)/2
      DO 70 COL1 = NP, 1, -1
C
C     Calculate the length of column COL1.
C
	SUM = D(COL1)
	POS1 = COL1 - 1
	DO 20 ROW = 1, COL1-1
	  SUM = SUM + D(ROW) * RBAR(POS1)**2
	  POS1 = POS1 + NP - ROW - 1
   20   CONTINUE
	WORK(COL1) = SQRT(SUM)
C
C     If SUM = 0, set all correlations with this variable to zero.
C
	IF (SUM .EQ. ZERO) THEN
	  YCORR(COL1) = ZERO
	  DO 30 COL2 = NP, COL1+1, -1
	    CORMAT(POS) = ZERO
	    POS = POS - 1
   30     CONTINUE
	  GO TO 70
	END IF
C
C     Form cross-products, then divide by product of column lengths.
C
	SUM = D(COL1) * THETAB(COL1)
	POS1 = COL1 - 1
	DO 40 ROW = 1, COL1-1
	  SUM = SUM + D(ROW) * RBAR(POS1) * THETAB(ROW)
	  POS1 = POS1 + NP - ROW - 1
   40   CONTINUE
	YCORR(COL1) = SUM / (SUMY * WORK(COL1))
C
	DO 60 COL2 = NP, COL1+1, -1
	  IF (WORK(COL2) .GT. ZERO) THEN
	    POS1 = COL1 - 1
	    POS2 = COL2 - 1
	    DIFF = COL2 - COL1
	    SUM = ZERO
	    DO 50 ROW = 1, COL1-1
	      SUM = SUM + D(ROW) * RBAR(POS1) * RBAR(POS2)
	      POS1 = POS1 + NP - ROW - 1
	      POS2 = POS1 + DIFF
   50       CONTINUE
	    SUM = SUM + D(COL1) * RBAR(POS2)
	    CORMAT(POS) = SUM / (WORK(COL1) * WORK(COL2))
	  ELSE
	    CORMAT(POS) = ZERO
	  END IF
	  POS = POS - 1
   60   CONTINUE
   70 CONTINUE
C
      RETURN
      END
C
      SUBROUTINE VMOVE(NP, NRBAR, VORDER, D, RBAR, THETAB, RSS, FROM,
     +    TO, TOL, IER)
C
C     ALGORITHM AS274.11 APPL. STATIST. (1992) VOL 41, NO.2
C
C     Move variable from position FROM to position TO in an
C     orthogonal reduction produced by AS75.1.
C
      INTEGER NP, NRBAR, VORDER(NP), FROM, TO, IER
      DOUBLE PRECISION D(NP), RBAR(*), THETAB(NP), RSS(NP), TOL(NP)
C
C     Local variables
C
      DOUBLE PRECISION ZERO, D1, D2, X, ONE, D1NEW, D2NEW, CBAR, SBAR, Y
      INTEGER M, FIRST, LAST, INC, M1, M2, MP1, COL, POS, ROW
C
      DATA ZERO/0.D0/, ONE/1.D0/
C
C     Check input parameters
C
      IER = 0
      IF (NP .LE. 0) IER = 1
      IF (NRBAR .LT. NP*(NP-1)/2) IER = IER + 2
      IF (FROM .LT. 1 .OR. FROM .GT. NP) IER = IER + 4
      IF (TO .LT. 1 .OR. TO .GT. NP) IER = IER + 8
      IF (IER .NE. 0) RETURN
C
      IF (FROM .EQ. TO) RETURN
C
      IF (FROM .LT. TO) THEN
	FIRST = FROM
	LAST = TO - 1
	INC = 1
      ELSE
	FIRST = FROM - 1
	LAST = TO
	INC = -1
      END IF
      DO 70 M = FIRST, LAST, INC
C
C     Find addresses of first elements of RBAR in rows M and (M+1).
C
	M1 = (M-1)*(NP+NP-M)/2 + 1
	M2 = M1 + NP - M
	MP1 = M + 1
	D1 = D(M)
	D2 = D(MP1)
C
C     Special cases.
C
	IF (D1 .EQ. ZERO .AND. D2 .EQ. ZERO) GO TO 40
	X = RBAR(M1)
	IF (ABS(X) * SQRT(D1) .LT. TOL(MP1)) THEN
	  X = ZERO
	END IF
	IF (D1 .EQ. ZERO .OR. X .EQ. ZERO) THEN
	  D(M) = D2
	  D(MP1) = D1
	  RBAR(M1) = ZERO
	  DO 10 COL = M+2, NP
	    M1 = M1 + 1
	    X = RBAR(M1)
	    RBAR(M1) = RBAR(M2)
	    RBAR(M2) = X
	    M2 = M2 + 1
   10     CONTINUE
	  X = THETAB(M)
	  THETAB(M) = THETAB(MP1)
	  THETAB(MP1) = X
	  GO TO 40
	ELSE IF (D2 .EQ. ZERO) THEN
	  D(M) = D1 * X**2
	  RBAR(M1) = ONE / X
	  DO 20 COL = M+2, NP
	    M1 = M1 + 1
	    RBAR(M1) = RBAR(M1) / X
   20     CONTINUE
	  THETAB(M) = THETAB(M) / X
	  GO TO 40
	END IF
C
C     Planar rotation in regular case.
C
	D1NEW = D2 + D1*X**2
	CBAR = D2 / D1NEW
	SBAR = X * D1 / D1NEW
	D2NEW = D1 * CBAR
	D(M) = D1NEW
	D(MP1) = D2NEW
	RBAR(M1) = SBAR
	DO 30 COL = M+2, NP
	  M1 = M1 + 1
	  Y = RBAR(M1)
	  RBAR(M1) = CBAR*RBAR(M2) + SBAR*Y
	  RBAR(M2) = Y - X*RBAR(M2)
	  M2 = M2 + 1
   30   CONTINUE
	Y = THETAB(M)
	THETAB(M) = CBAR*THETAB(MP1) + SBAR*Y
	THETAB(MP1) = Y - X*THETAB(MP1)
C
C     Swap columns M and (M+1) down to row (M-1).
C
   40   IF (M .EQ. 1) GO TO 60
	POS = M
	DO 50 ROW = 1, M-1
	  X = RBAR(POS)
	  RBAR(POS) = RBAR(POS-1)
	  RBAR(POS-1) = X
	  POS = POS + NP - ROW - 1
   50   CONTINUE
C
C     Adjust variable order (VORDER), the tolerances (TOL) and
C     the vector of residual sums of squares (RSS).
C
   60   M1 = VORDER(M)
	VORDER(M) = VORDER(MP1)
	VORDER(MP1) = M1
	X = TOL(M)
	TOL(M) = TOL(MP1)
	TOL(MP1) = X
	RSS(M) = RSS(MP1) + D(MP1) * THETAB(MP1)**2
   70 CONTINUE
C
      RETURN
      END
C
      SUBROUTINE REORDR(NP, NRBAR, VORDER, D, RBAR, THETAB, RSS, TOL,
     +      LIST, N, POS1, IER)
C
C     ALGORITHM AS274.12  APPL. STATIST. (1992) VOL 41, NO.2
C
C     Re-order the variables in an orthogonal reduction produced by
C     AS75.1 so that the N variables in LIST start at position POS1,
C     though will not necessarily be in the same order as in LIST.
C     Any variables in VORDER before position POS1 are not moved.
C
C     Auxiliary routine called: VMOVE
C
      INTEGER NP, NRBAR, VORDER(NP), N, LIST(N), POS1, IER
      DOUBLE PRECISION D(NP), RBAR(NRBAR), THETAB(NP), RSS(NP), TOL(NP)
C
C     Local variables.
C
      INTEGER NEXT, I, L, J
C
C     Check N.
C
      IER = 0
      IF (NP .LT. 1) IER = 1
      IF (NRBAR .LT. NP*(NP-1)/2) IER = IER + 2
      IF (N .LT. 1 .OR. N .GE. NP+1-POS1) IER = IER + 4
      IF (IER .NE. 0) RETURN
C
C     Work through VORDER finding variables which are in LIST.
C
      NEXT = POS1
      I = POS1
   10 L = VORDER(I)
      DO 20 J = 1, N
	IF (L .EQ. LIST(J)) GO TO 40
   20 CONTINUE
   30 I = I + 1
      IF (I .LE. NP) GO TO 10
C
C     If this point is reached, one or more variables in LIST has not
C     been found.
C
      IER = 8
      RETURN
C
C     Variable L is in LIST; move it up to position NEXT if it is not
C     already there.
C
   40 IF (I .GT. NEXT) CALL VMOVE(NP, NRBAR, VORDER, D, RBAR, THETAB,
     +      RSS, I, NEXT, TOL, IER)
      NEXT = NEXT + 1
      IF (NEXT .LT. N+POS1) GO TO 30
C
      RETURN
      END
C
      SUBROUTINE HDIAG(XROW, NP, NRBAR, D, RBAR, TOL, NREQ, HII, WK,
     +     IFAULT)
C
C     ALGORITHM AS274.13  APPL. STATIST. (1992) VOL.41, NO.2
C
      INTEGER NP, NRBAR, NREQ, IFAULT
      DOUBLE PRECISION XROW(NP), D(NP), RBAR(*), TOL(NP), HII, WK(NP)
C
C     Local variables
C
      INTEGER COL, ROW, POS
      DOUBLE PRECISION ZERO, SUM
C
      DATA ZERO /0.0D0/
C
C     Some checks
C
      IFAULT = 0
      IF (NP .LT. 1) IFAULT = 1
      IF (NRBAR .LT. NP*(NP-1)/2) IFAULT = IFAULT + 2
      IF (NREQ .GT. NP) IFAULT = IFAULT + 4
      IF (IFAULT .NE. 0) RETURN
C
C     The elements of XROW.inv(RBAR).sqrt(D) are calculated and stored
C     in WK.
C
      HII = ZERO
      DO 20 COL = 1, NREQ
	IF (SQRT(D(COL)) .LE. TOL(COL)) THEN
	  WK(COL) = ZERO
	  GO TO 20
	END IF
	POS = COL - 1
	SUM = XROW(COL)
	DO 10 ROW = 1, COL-1
	  SUM = SUM - WK(ROW)*RBAR(POS)
	  POS = POS + NP - ROW - 1
   10   CONTINUE
	WK(COL) = SUM
	HII = HII + SUM**2 / D(COL)
   20 CONTINUE
C
      RETURN
      END
