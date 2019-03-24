C
C	Routines from LINPACK for singular value decomposition
C
      SUBROUTINE SSVDC(X, LDX, N, P, S, E, U, LDU, V, LDV, WORK, JOB,  
     + INFO)
      INTEGER LDX, N, P, LDU, LDV, JOB, INFO
      REAL X(LDX,1), S(1), E(1), U(LDU,1), V(LDV,1), WORK(1)
C
C
C     SSVDC IS A SUBROUTINE TO REDUCE A REAL NXP MATRIX X BY
C     ORTHOGONAL TRANSFORMATIONS U AND V TO DIAGONAL FORM.  THE
C     DIAGONAL ELEMENTS S(I) ARE THE SINGULAR VALUES OF X.  THE
C     COLUMNS OF U ARE THE CORRESPONDING LEFT SINGULAR VECTORS,
C     AND THE COLUMNS OF V THE RIGHT SINGULAR VECTORS.
C
C     ON ENTRY
C
C         X         REAL(LDX,P), WHERE LDX.GE.N.
C                   X CONTAINS THE MATRIX WHOSE SINGULAR VALUE
C                   DECOMPOSITION IS TO BE COMPUTED.  X IS
C                   DESTROYED BY SSVDC.
C
C         LDX       INTEGER.
C                   LDX IS THE LEADING DIMENSION OF THE ARRAY X.
C
C         N         INTEGER.
C                   N IS THE NUMBER OF COLUMNS OF THE MATRIX X.
C
C         P         INTEGER.
C                   P IS THE NUMBER OF ROWS OF THE MATRIX X.
C
C         LDU       INTEGER.
C                   LDU IS THE LEADING DIMENSION OF THE ARRAY U.
C                   (SEE BELOW).
C
C         LDV       INTEGER.
C                   LDV IS THE LEADING DIMENSION OF THE ARRAY V.
C                   (SEE BELOW).
C
C         WORK      REAL(N).
C                   WORK IS A SCRATCH ARRAY.
C
C         JOB       INTEGER.
C                   JOB CONTROLS THE COMPUTATION OF THE SINGULAR
C                   VECTORS.  IT HAS THE DECIMAL EXPANSION AB
C                   WITH THE FOLLOWING MEANING
C
C                        A.EQ.0    DO NOT COMPUTE THE LEFT SINGULAR
C                                  VECTORS.
C                        A.EQ.1    RETURN THE N LEFT SINGULAR VECTORS
C                                  IN U.
C                        A.GE.2    RETURN THE FIRST MIN(N,P) SINGULAR
C                                  VECTORS IN U.
C                        B.EQ.0    DO NOT COMPUTE THE RIGHT SINGULAR
C                                  VECTORS.
C                        B.EQ.1    RETURN THE RIGHT SINGULAR VECTORS
C                                  IN V.
C
C     ON RETURN
C
C         S         REAL(MM), WHERE MM=MIN(N+1,P).
C                   THE FIRST MIN(N,P) ENTRIES OF S CONTAIN THE
C                   SINGULAR VALUES OF X ARRANGED IN DESCENDING
C                   ORDER OF MAGNITUDE.
C
C         E         REAL(P).
C                   E ORDINARILY CONTAINS ZEROS.  HOWEVER SEE THE
C                   DISCUSSION OF INFO FOR EXCEPTIONS.
C
C         U         REAL(LDU,K), WHERE LDU.GE.N.  IF JOBA.EQ.1 THEN
C                                   K.EQ.N, IF JOBA.GE.2 THEN
C                                   K.EQ.MIN(N,P).
C                   U CONTAINS THE MATRIX OF RIGHT SINGULAR VECTORS.
C                   U IS NOT REFERENCED IF JOBA.EQ.0.  IF N.LE.P
C                   OR IF JOBA.EQ.2, THEN U MAY BE IDENTIFIED WITH X
C                   IN THE SUBROUTINE CALL.
C
C         V         REAL(LDV,P), WHERE LDV.GE.P.
C                   V CONTAINS THE MATRIX OF RIGHT SINGULAR VECTORS.
C                   V IS NOT REFERENCED IF JOB.EQ.0.  IF P.LE.N,
C                   THEN V MAY BE IDENTIFIED WITH X IN THE
C                   SUBROUTINE CALL.
C
C         INFO      INTEGER.
C                   THE SINGULAR VALUES (AND THEIR CORRESPONDING
C                   SINGULAR VECTORS) S(INFO+1),S(INFO+2),...,S(M)
C                   ARE CORRECT (HERE M=MIN(N,P)).  THUS IF
C                   INFO.EQ.0, ALL THE SINGULAR VALUES AND THEIR
C                   VECTORS ARE CORRECT.  IN ANY EVENT, THE MATRIX
C                   B = TRANS(U)*X*V IS THE BIDIAGONAL MATRIX
C                   WITH THE ELEMENTS OF S ON ITS DIAGONAL AND THE
C                   ELEMENTS OF E ON ITS SUPER-DIAGONAL (TRANS(U)
C                   IS THE TRANSPOSE OF U).  THUS THE SINGULAR
C                   VALUES OF X AND B ARE THE SAME.
C
C     LINPACK. THIS VERSION DATED 03/19/79 .
C     G.W. STEWART, UNIVERSITY OF MARYLAND, ARGONNE NATIONAL LAB.
C
C     ***** USES THE FOLLOWING FUNCTIONS AND SUBPROGRAMS.
C
C     EXTERNAL SROT
C     BLAS SAXPY,SDOT,SSCAL,SSWAP,SNRM2,SROTG
C     FORTRAN ABS,AMAX1,MAX0,MIN0,MOD,SQRT
C
C     INTERNAL VARIABLES
C
      INTEGER I, ITER, J, JOBU, K, KASE, KK, L, LL, LLS, LM1, LP1, LS,
     + LU, M, MAXIT, MM, MM1, MP1, NCT, NCTP1, NCU, NRT, NRTP1
      REAL SDOT, T
      REAL B, C, CS, EL, EMM1, F, G, SNRM2, SCALE, SHIFT, SL, SM, SN,
     + SMM1, T1, TEST, ZTEST
      LOGICAL WANTU, WANTV
C
C
C     SET THE MAXIMUM NUMBER OF ITERATIONS.
C
      MAXIT = 30
C
C     DETERMINE WHAT IS TO BE COMPUTED.
C
      WANTU = .FALSE.
      WANTV = .FALSE.
      JOBU = MOD(JOB,100)/10
      NCU = N
      IF (JOBU.GT.1) NCU = MIN0(N,P)
      IF (JOBU.NE.0) WANTU = .TRUE.
      IF (MOD(JOB,10).NE.0) WANTV = .TRUE.
C
C     REDUCE X TO BIDIAGONAL FORM, STORING THE DIAGONAL ELEMENTS
C     IN S AND THE SUPER-DIAGONAL ELEMENTS IN E.
C
      INFO = 0
      NCT = MIN0(N-1,P)
      NRT = MAX0(0,MIN0(P-2,N))
      LU = MAX0(NCT,NRT)
      IF (LU.LT.1) GO TO 170
      DO 160 L=1,LU
        LP1 = L + 1
        IF (L.GT.NCT) GO TO 20
C
C           COMPUTE THE TRANSFORMATION FOR THE L-TH COLUMN AND
C           PLACE THE L-TH DIAGONAL IN S(L).
C
        S(L) = SNRM2(N-L+1,X(L,L),1)
        IF (S(L).EQ.0.0E0) GO TO 10
        IF (X(L,L).NE.0.0E0) S(L) = SIGN(S(L),X(L,L))
        CALL SSCAL(N-L+1, 1.0E0/S(L), X(L,L), 1)
        X(L,L) = 1.0E0 + X(L,L)
   10   CONTINUE
        S(L) = -S(L)
   20   CONTINUE
        IF (P.LT.LP1) GO TO 50
        DO 40 J=LP1,P
          IF (L.GT.NCT) GO TO 30
          IF (S(L).EQ.0.0E0) GO TO 30
C
C              APPLY THE TRANSFORMATION.
C
          T = -SDOT(N-L+1,X(L,L),1,X(L,J),1)/X(L,L)
          CALL SAXPY(N-L+1, T, X(L,L), 1, X(L,J), 1)
   30     CONTINUE
C
C           PLACE THE L-TH ROW OF X INTO  E FOR THE
C           SUBSEQUENT CALCULATION OF THE ROW TRANSFORMATION.
C
          E(J) = X(L,J)
   40   CONTINUE
   50   CONTINUE
        IF (.NOT.WANTU .OR. L.GT.NCT) GO TO 70
C
C           PLACE THE TRANSFORMATION IN U FOR SUBSEQUENT BACK
C           MULTIPLICATION.
C
        DO 60 I=L,N
          U(I,L) = X(I,L)
   60   CONTINUE
   70   CONTINUE
        IF (L.GT.NRT) GO TO 150
C
C           COMPUTE THE L-TH ROW TRANSFORMATION AND PLACE THE
C           L-TH SUPER-DIAGONAL IN E(L).
C
        E(L) = SNRM2(P-L,E(LP1),1)
        IF (E(L).EQ.0.0E0) GO TO 80
        IF (E(LP1).NE.0.0E0) E(L) = SIGN(E(L),E(LP1))
        CALL SSCAL(P-L, 1.0E0/E(L), E(LP1), 1)
        E(LP1) = 1.0E0 + E(LP1)
   80   CONTINUE
        E(L) = -E(L)
        IF (LP1.GT.N .OR. E(L).EQ.0.0E0) GO TO 120
C
C              APPLY THE TRANSFORMATION.
C
        DO 90 I=LP1,N
          WORK(I) = 0.0E0
   90   CONTINUE
        DO 100 J=LP1,P
          CALL SAXPY(N-L, E(J), X(LP1,J), 1, WORK(LP1), 1)
  100   CONTINUE
        DO 110 J=LP1,P
          CALL SAXPY(N-L, -E(J)/E(LP1), WORK(LP1), 1, X(LP1,J), 1)
  110   CONTINUE
  120   CONTINUE
        IF (.NOT.WANTV) GO TO 140
C
C              PLACE THE TRANSFORMATION IN V FOR SUBSEQUENT
C              BACK MULTIPLICATION.
C
        DO 130 I=LP1,P
          V(I,L) = E(I)
  130   CONTINUE
  140   CONTINUE
  150   CONTINUE
  160 CONTINUE
  170 CONTINUE
C
C     SET UP THE FINAL BIDIAGONAL MATRIX OR ORDER M.
C
      M = MIN0(P,N+1)
      NCTP1 = NCT + 1
      NRTP1 = NRT + 1
      IF (NCT.LT.P) S(NCTP1) = X(NCTP1,NCTP1)
      IF (N.LT.M) S(M) = 0.0E0
      IF (NRTP1.LT.M) E(NRTP1) = X(NRTP1,M)
      E(M) = 0.0E0
C
C     IF REQUIRED, GENERATE U.
C
      IF (.NOT.WANTU) GO TO 300
      IF (NCU.LT.NCTP1) GO TO 200
      DO 190 J=NCTP1,NCU
        DO 180 I=1,N
          U(I,J) = 0.0E0
  180   CONTINUE
        U(J,J) = 1.0E0
  190 CONTINUE
  200 CONTINUE
      IF (NCT.LT.1) GO TO 290
      DO 280 LL=1,NCT
        L = NCT - LL + 1
        IF (S(L).EQ.0.0E0) GO TO 250
        LP1 = L + 1
        IF (NCU.LT.LP1) GO TO 220
        DO 210 J=LP1,NCU
          T = -SDOT(N-L+1,U(L,L),1,U(L,J),1)/U(L,L)
          CALL SAXPY(N-L+1, T, U(L,L), 1, U(L,J), 1)
  210   CONTINUE
  220   CONTINUE
        CALL SSCAL(N-L+1, -1.0E0, U(L,L), 1)
        U(L,L) = 1.0E0 + U(L,L)
        LM1 = L - 1
        IF (LM1.LT.1) GO TO 240
        DO 230 I=1,LM1
          U(I,L) = 0.0E0
  230   CONTINUE
  240   CONTINUE
        GO TO 270
  250   CONTINUE
        DO 260 I=1,N
          U(I,L) = 0.0E0
  260   CONTINUE
        U(L,L) = 1.0E0
  270   CONTINUE
  280 CONTINUE
  290 CONTINUE
  300 CONTINUE
C
C     IF IT IS REQUIRED, GENERATE V.
C
      IF (.NOT.WANTV) GO TO 350
      DO 340 LL=1,P
        L = P - LL + 1
        LP1 = L + 1
        IF (L.GT.NRT) GO TO 320
        IF (E(L).EQ.0.0E0) GO TO 320
        DO 310 J=LP1,P
          T = -SDOT(P-L,V(LP1,L),1,V(LP1,J),1)/V(LP1,L)
          CALL SAXPY(P-L, T, V(LP1,L), 1, V(LP1,J), 1)
  310   CONTINUE
  320   CONTINUE
        DO 330 I=1,P
          V(I,L) = 0.0E0
  330   CONTINUE
        V(L,L) = 1.0E0
  340 CONTINUE
  350 CONTINUE
C
C     MAIN ITERATION LOOP FOR THE SINGULAR VALUES.
C
      MM = M
      ITER = 0
  360 CONTINUE
C
C        QUIT IF ALL THE SINGULAR VALUES HAVE BEEN FOUND.
C
C     ...EXIT
      IF (M.EQ.0) GO TO 620
C
C        IF TOO MANY ITERATIONS HAVE BEEN PERFORMED, SET
C        FLAG AND RETURN.
C
      IF (ITER.LT.MAXIT) GO TO 370
      INFO = M
C     ......EXIT
      GO TO 620
  370 CONTINUE
C
C        THIS SECTION OF THE PROGRAM INSPECTS FOR
C        NEGLIGIBLE ELEMENTS IN THE S AND E ARRAYS.  ON
C        COMPLETION THE VARIABLES KASE AND L ARE SET AS FOLLOWS.
C
C           KASE = 1     IF S(M) AND E(L-1) ARE NEGLIGIBLE AND L.LT.M
C           KASE = 2     IF S(L) IS NEGLIGIBLE AND L.LT.M
C           KASE = 3     IF E(L-1) IS NEGLIGIBLE, L.LT.M, AND
C                        S(L), ..., S(M) ARE NOT NEGLIGIBLE (QR STEP).
C           KASE = 4     IF E(M-1) IS NEGLIGIBLE (CONVERGENCE).
C
      DO 390 LL=1,M
        L = M - LL
C        ...EXIT
        IF (L.EQ.0) GO TO 400
        TEST = ABS(S(L)) + ABS(S(L+1))
        ZTEST = TEST + ABS(E(L))
        IF (ZTEST.NE.TEST) GO TO 380
        E(L) = 0.0E0
C        ......EXIT
        GO TO 400
  380   CONTINUE
  390 CONTINUE
  400 CONTINUE
      IF (L.NE.M-1) GO TO 410
      KASE = 4
      GO TO 480
  410 CONTINUE
      LP1 = L + 1
      MP1 = M + 1
      DO 430 LLS=LP1,MP1
        LS = M - LLS + LP1
C           ...EXIT
        IF (LS.EQ.L) GO TO 440
        TEST = 0.0E0
        IF (LS.NE.M) TEST = TEST + ABS(E(LS))
        IF (LS.NE.L+1) TEST = TEST + ABS(E(LS-1))
        ZTEST = TEST + ABS(S(LS))
        IF (ZTEST.NE.TEST) GO TO 420
        S(LS) = 0.0E0
C           ......EXIT
        GO TO 440
  420   CONTINUE
  430 CONTINUE
  440 CONTINUE
      IF (LS.NE.L) GO TO 450
      KASE = 3
      GO TO 470
  450 CONTINUE
      IF (LS.NE.M) GO TO 460
      KASE = 1
      GO TO 470
  460 CONTINUE
      KASE = 2
      L = LS
  470 CONTINUE
  480 CONTINUE
      L = L + 1
C
C        PERFORM THE TASK INDICATED BY KASE.
C
      GO TO (490, 520, 540, 570), KASE
C
C        DEFLATE NEGLIGIBLE S(M).
C
  490 CONTINUE
      MM1 = M - 1
      F = E(M-1)
      E(M-1) = 0.0E0
      DO 510 KK=L,MM1
        K = MM1 - KK + L
        T1 = S(K)
        CALL SROTG(T1, F, CS, SN)
        S(K) = T1
        IF (K.EQ.L) GO TO 500
        F = -SN*E(K-1)
        E(K-1) = CS*E(K-1)
  500   CONTINUE
        IF (WANTV) CALL SROT(P, V(1,K), 1, V(1,M), 1, CS, SN)
  510 CONTINUE
      GO TO 610
C
C        SPLIT AT NEGLIGIBLE S(L).
C
  520 CONTINUE
      F = E(L-1)
      E(L-1) = 0.0E0
      DO 530 K=L,M
        T1 = S(K)
        CALL SROTG(T1, F, CS, SN)
        S(K) = T1
        F = -SN*E(K)
        E(K) = CS*E(K)
        IF (WANTU) CALL SROT(N, U(1,K), 1, U(1,L-1), 1, CS, SN)
  530 CONTINUE
      GO TO 610
C
C        PERFORM ONE QR STEP.
C
  540 CONTINUE
C
C           CALCULATE THE SHIFT.
C
      SCALE = AMAX1(ABS(S(M)),ABS(S(M-1)),ABS(E(M-1)),ABS(S(L)),ABS(E(L)
     * ))
      SM = S(M)/SCALE
      SMM1 = S(M-1)/SCALE
      EMM1 = E(M-1)/SCALE
      SL = S(L)/SCALE
      EL = E(L)/SCALE
      B = ((SMM1+SM)*(SMM1-SM)+EMM1**2)/2.0E0
      C = (SM*EMM1)**2
      SHIFT = 0.0E0
      IF (B.EQ.0.0E0 .AND. C.EQ.0.0E0) GO TO 550
      SHIFT = SQRT(B**2+C)
      IF (B.LT.0.0E0) SHIFT = -SHIFT
      SHIFT = C/(B+SHIFT)
  550 CONTINUE
      F = (SL+SM)*(SL-SM) - SHIFT
      G = SL*EL
C
C           CHASE ZEROS.
C
      MM1 = M - 1
      DO 560 K=L,MM1
        CALL SROTG(F, G, CS, SN)
        IF (K.NE.L) E(K-1) = F
        F = CS*S(K) + SN*E(K)
        E(K) = CS*E(K) - SN*S(K)
        G = SN*S(K+1)
        S(K+1) = CS*S(K+1)
        IF (WANTV) CALL SROT(P, V(1,K), 1, V(1,K+1), 1, CS, SN)
        CALL SROTG(F, G, CS, SN)
        S(K) = F
        F = CS*E(K) + SN*S(K+1)
        S(K+1) = -SN*E(K) + CS*S(K+1)
        G = SN*E(K+1)
        E(K+1) = CS*E(K+1)
        IF (WANTU .AND. K.LT.N) CALL SROT(N, U(1,K), 1, U(1,K+1), 1,
     *   CS, SN)
  560 CONTINUE
      E(M-1) = F
      ITER = ITER + 1
      GO TO 610
C
C        CONVERGENCE.
C
  570 CONTINUE
C
C           MAKE THE SINGULAR VALUE  POSITIVE.
C
      IF (S(L).GE.0.0E0) GO TO 580
      S(L) = -S(L)
      IF (WANTV) CALL SSCAL(P, -1.0E0, V(1,L), 1)
  580 CONTINUE
C
C           ORDER THE SINGULAR VALUE.
C
  590 IF (L.EQ.MM) GO TO 600
C           ...EXIT
      IF (S(L).GE.S(L+1)) GO TO 600
      T = S(L)
      S(L) = S(L+1)
      S(L+1) = T
      IF (WANTV .AND. L.LT.P) CALL SSWAP(P, V(1,L), 1, V(1,L+1), 1)
      IF (WANTU .AND. L.LT.N) CALL SSWAP(N, U(1,L), 1, U(1,L+1), 1)
      L = L + 1
      GO TO 590
  600 CONTINUE
      ITER = 0
      M = M - 1
  610 CONTINUE
      GO TO 360
  620 CONTINUE
      RETURN
      END

      REAL FUNCTION SASUM(N, SX, INCX)                                  
C
C     TAKES THE SUM OF THE ABSOLUTE VALUES.
C     USES UNROLLED LOOPS FOR INCREMENT EQUAL TO ONE.
C     JACK DONGARRA, LINPACK, 3/11/78.
C
      REAL SX(1), STEMP
      INTEGER I, INCX, M, MP1, N, NINCX
C
      SASUM = 0.0E0
      STEMP = 0.0E0
      IF (N.LE.0) RETURN
      IF (INCX.EQ.1) GO TO 20
C
C        CODE FOR INCREMENT NOT EQUAL TO 1
C
      NINCX = N*INCX
      DO 10 I=1,NINCX,INCX
        STEMP = STEMP + ABS(SX(I))
   10 CONTINUE
      SASUM = STEMP
      RETURN
C
C        CODE FOR INCREMENT EQUAL TO 1
C
C
C        CLEAN-UP LOOP
C
   20 M = MOD(N,6)
      IF (M.EQ.0) GO TO 40
      DO 30 I=1,M
        STEMP = STEMP + ABS(SX(I))
   30 CONTINUE
      IF (N.LT.6) GO TO 60
   40 MP1 = M + 1
      DO 50 I=MP1,N,6
        STEMP = STEMP + ABS(SX(I)) + ABS(SX(I+1)) + ABS(SX(I+2)) +
     *   ABS(SX(I+3)) + ABS(SX(I+4)) + ABS(SX(I+5))
   50 CONTINUE
   60 SASUM = STEMP
      RETURN
      END

      SUBROUTINE SAXPY(N, SA, SX, INCX, SY, INCY)                      
C
C     CONSTANT TIMES A VECTOR PLUS A VECTOR.
C     USES UNROLLED LOOP FOR INCREMENTS EQUAL TO ONE.
C     JACK DONGARRA, LINPACK, 3/11/78.
C
      REAL SX(1), SY(1), SA
      INTEGER I, INCX, INCY, IX, IY, M, MP1, N
C
      IF (N.LE.0) RETURN
      IF (SA.EQ.0.0) RETURN
      IF (INCX.EQ.1 .AND. INCY.EQ.1) GO TO 20
C
C        CODE FOR UNEQUAL INCREMENTS OR EQUAL INCREMENTS
C          NOT EQUAL TO 1
C
      IX = 1
      IY = 1
      IF (INCX.LT.0) IX = (-N+1)*INCX + 1
      IF (INCY.LT.0) IY = (-N+1)*INCY + 1
      DO 10 I=1,N
        SY(IY) = SY(IY) + SA*SX(IX)
        IX = IX + INCX
        IY = IY + INCY
   10 CONTINUE
      RETURN
C
C        CODE FOR BOTH INCREMENTS EQUAL TO 1
C
C
C        CLEAN-UP LOOP
C
   20 M = MOD(N,4)
      IF (M.EQ.0) GO TO 40
      DO 30 I=1,M
        SY(I) = SY(I) + SA*SX(I)
   30 CONTINUE
      IF (N.LT.4) RETURN
   40 MP1 = M + 1
      DO 50 I=MP1,N,4
        SY(I) = SY(I) + SA*SX(I)
        SY(I+1) = SY(I+1) + SA*SX(I+1)
        SY(I+2) = SY(I+2) + SA*SX(I+2)
        SY(I+3) = SY(I+3) + SA*SX(I+3)
   50 CONTINUE
      RETURN
      END

      REAL FUNCTION SDOT(N, SX, INCX, SY, INCY)                         
C
C     FORMS THE DOT PRODUCT OF TWO VECTORS.
C     USES UNROLLED LOOPS FOR INCREMENTS EQUAL TO ONE.
C     JACK DONGARRA, LINPACK, 3/11/78.
C
      REAL SX(1), SY(1), STEMP
      INTEGER I, INCX, INCY, IX, IY, M, MP1, N
C
      STEMP = 0.0E0
      SDOT = 0.0E0
      IF (N.LE.0) RETURN
      IF (INCX.EQ.1 .AND. INCY.EQ.1) GO TO 20
C
C        CODE FOR UNEQUAL INCREMENTS OR EQUAL INCREMENTS
C          NOT EQUAL TO 1
C
      IX = 1
      IY = 1
      IF (INCX.LT.0) IX = (-N+1)*INCX + 1
      IF (INCY.LT.0) IY = (-N+1)*INCY + 1
      DO 10 I=1,N
        STEMP = STEMP + SX(IX)*SY(IY)
        IX = IX + INCX
        IY = IY + INCY
   10 CONTINUE
      SDOT = STEMP
      RETURN
C
C        CODE FOR BOTH INCREMENTS EQUAL TO 1
C
C
C        CLEAN-UP LOOP
C
   20 M = MOD(N,5)
      IF (M.EQ.0) GO TO 40
      DO 30 I=1,M
        STEMP = STEMP + SX(I)*SY(I)
   30 CONTINUE
      IF (N.LT.5) GO TO 60
   40 MP1 = M + 1
      DO 50 I=MP1,N,5
        STEMP = STEMP + SX(I)*SY(I) + SX(I+1)*SY(I+1) + SX(I+2)*SY(I+2)
     *   + SX(I+3)*SY(I+3) + SX(I+4)*SY(I+4)
   50 CONTINUE
   60 SDOT = STEMP
      RETURN
      END

      REAL FUNCTION SNRM2(N, SX, INCX)                                  
      INTEGER NEXT
      REAL SX(1), CUTLO, CUTHI, HITEST, SUM, XMAX, ZERO, ONE
      DATA ZERO, ONE /0.0E0,1.0E0/
C
C     EUCLIDEAN NORM OF THE N-VECTOR STORED IN SX() WITH STORAGE
C     INCREMENT INCX .
C     IF    N .LE. 0 RETURN WITH RESULT = 0.
C     IF N .GE. 1 THEN INCX MUST BE .GE. 1
C
C           C.L.LAWSON, 1978 JAN 08
C
C     FOUR PHASE METHOD     USING TWO BUILT-IN CONSTANTS THAT ARE
C     HOPEFULLY APPLICABLE TO ALL MACHINES.
C         CUTLO = MAXIMUM OF  SQRT(U/EPS)  OVER ALL KNOWN MACHINES.
C         CUTHI = MINIMUM OF  SQRT(V)      OVER ALL KNOWN MACHINES.
C     WHERE
C         EPS = SMALLEST NO. SUCH THAT EPS + 1. .GT. 1.
C         U   = SMALLEST POSITIVE NO.   (UNDERFLOW LIMIT)
C         V   = LARGEST  NO.            (OVERFLOW  LIMIT)
C
C     BRIEF OUTLINE OF ALGORITHM..
C
C     PHASE 1    SCANS ZERO COMPONENTS.
C     MOVE TO PHASE 2 WHEN A COMPONENT IS NONZERO AND .LE. CUTLO
C     MOVE TO PHASE 3 WHEN A COMPONENT IS .GT. CUTLO
C     MOVE TO PHASE 4 WHEN A COMPONENT IS .GE. CUTHI/M
C     WHERE M = N FOR X() REAL AND M = 2*N FOR COMPLEX.
C
C     VALUES FOR CUTLO AND CUTHI..
C     FROM THE ENVIRONMENTAL PARAMETERS LISTED IN THE IMSL CONVERTER
C     DOCUMENT THE LIMITING VALUES ARE AS FOLLOWS..
C     CUTLO, S.P.   U/EPS = 2**(-102) FOR  HONEYWELL.  CLOSE SECONDS ARE
C                   UNIVAC AND DEC AT 2**(-103)
C                   THUS CUTLO = 2**(-51) = 4.44089E-16
C     CUTHI, S.P.   V = 2**127 FOR UNIVAC, HONEYWELL, AND DEC.
C                   THUS CUTHI = 2**(63.5) = 1.30438E19
C     CUTLO, D.P.   U/EPS = 2**(-67) FOR HONEYWELL AND DEC.
C                   THUS CUTLO = 2**(-33.5) = 8.23181D-11
C     CUTHI, D.P.   SAME AS S.P.  CUTHI = 1.30438D19
C     DATA CUTLO, CUTHI / 8.232D-11,  1.304D19 /
C     DATA CUTLO, CUTHI / 4.441E-16,  1.304E19 /
      DATA CUTLO, CUTHI /4.441E-16,1.304E19/
C
      IF (N.GT.0) GO TO 10
      SNRM2 = ZERO
      GO TO 140
C
   10 ASSIGN 30 TO NEXT
      SUM = ZERO
      NN = N*INCX
C                                                 BEGIN MAIN LOOP
      I = 1
   20 GO TO NEXT, (30, 40, 70, 80)
   30 IF (ABS(SX(I)).GT.CUTLO) GO TO 110
      ASSIGN 40 TO NEXT
      XMAX = ZERO
C
C                        PHASE 1.  SUM IS ZERO
C
   40 IF (SX(I).EQ.ZERO) GO TO 130
      IF (ABS(SX(I)).GT.CUTLO) GO TO 110
C
C                                PREPARE FOR PHASE 2.
      ASSIGN 70 TO NEXT
      GO TO 60
C
C                                PREPARE FOR PHASE 4.
C
   50 I = J
      ASSIGN 80 TO NEXT
      SUM = (SUM/SX(I))/SX(I)
   60 XMAX = ABS(SX(I))
      GO TO 90
C
C                   PHASE 2.  SUM IS SMALL.
C                             SCALE TO AVOID DESTRUCTIVE UNDERFLOW.
C
   70 IF (ABS(SX(I)).GT.CUTLO) GO TO 100
C
C                     COMMON CODE FOR PHASES 2 AND 4.
C                     IN PHASE 4 SUM IS LARGE.  SCALE TO AVOID OVERFLOW.
C
   80 IF (ABS(SX(I)).LE.XMAX) GO TO 90
      SUM = ONE + SUM*(XMAX/SX(I))**2
      XMAX = ABS(SX(I))
      GO TO 130
C
   90 SUM = SUM + (SX(I)/XMAX)**2
      GO TO 130
C
C
C                  PREPARE FOR PHASE 3.
C
  100 SUM = (SUM*XMAX)*XMAX
C
C
C     FOR REAL OR D.P. SET HITEST = CUTHI/N
C     FOR COMPLEX      SET HITEST = CUTHI/(2*N)
C
  110 HITEST = CUTHI/FLOAT(N)
C
C                   PHASE 3.  SUM IS MID-RANGE.  NO SCALING.
C
      DO 120 J=I,NN,INCX
        IF (ABS(SX(J)).GE.HITEST) GO TO 50
        SUM = SUM + SX(J)**2
  120 CONTINUE
      SNRM2 = SQRT(SUM)
      GO TO 140
C
  130 CONTINUE
      I = I + INCX
      IF (I.LE.NN) GO TO 20
C
C              END OF MAIN LOOP.
C
C              COMPUTE SQUARE ROOT AND ADJUST FOR SCALING.
C
      SNRM2 = XMAX*SQRT(SUM)
  140 CONTINUE
      RETURN
      END

      SUBROUTINE SROT(N, SX, INCX, SY, INCY, C, S)                      
C
C     APPLIES A PLANE ROTATION.
C     JACK DONGARRA, LINPACK, 3/11/78.
C
      REAL SX(1), SY(1), STEMP, C, S
      INTEGER I, INCX, INCY, IX, IY, N
C
      IF (N.LE.0) RETURN
      IF (INCX.EQ.1 .AND. INCY.EQ.1) GO TO 20
C
C       CODE FOR UNEQUAL INCREMENTS OR EQUAL INCREMENTS NOT EQUAL
C         TO 1
C
      IX = 1
      IY = 1
      IF (INCX.LT.0) IX = (-N+1)*INCX + 1
      IF (INCY.LT.0) IY = (-N+1)*INCY + 1
      DO 10 I=1,N
        STEMP = C*SX(IX) + S*SY(IY)
        SY(IY) = C*SY(IY) - S*SX(IX)
        SX(IX) = STEMP
        IX = IX + INCX
        IY = IY + INCY
   10 CONTINUE
      RETURN
C
C       CODE FOR BOTH INCREMENTS EQUAL TO 1
C
   20 DO 30 I=1,N
        STEMP = C*SX(I) + S*SY(I)
        SY(I) = C*SY(I) - S*SX(I)
        SX(I) = STEMP
   30 CONTINUE
      RETURN
      END


      SUBROUTINE SROTG(SA, SB, C, S)                                    
C
C     CONSTRUCT GIVENS PLANE ROTATION.
C     JACK DONGARRA, LINPACK, 3/11/78.
C
      REAL SA, SB, C, S, ROE, SCALE, R, Z
C
      ROE = SB
      IF (ABS(SA).GT.ABS(SB)) ROE = SA
      SCALE = ABS(SA) + ABS(SB)
      IF (SCALE.NE.0.0) GO TO 10
      C = 1.0
      S = 0.0
      R = 0.0
      GO TO 20
   10 R = SCALE*SQRT((SA/SCALE)**2+(SB/SCALE)**2)
      R = SIGN(1.0,ROE)*R
      C = SA/R
      S = SB/R
   20 Z = 1.0
      IF (ABS(SA).GT.ABS(SB)) Z = S
      IF (ABS(SB).GE.ABS(SA) .AND. C.NE.0.0) Z = 1.0/C
      SA = R
      SB = Z
      RETURN
      END


      SUBROUTINE SSCAL(N, SA, SX, INCX)                                 
C
C     SCALES A VECTOR BY A CONSTANT.
C     USES UNROLLED LOOPS FOR INCREMENT EQUAL TO 1.
C     JACK DONGARRA, LINPACK, 3/11/78.
C
      REAL SA, SX(1)
      INTEGER I, INCX, M, MP1, N, NINCX
C
      IF (N.LE.0) RETURN
      IF (INCX.EQ.1) GO TO 20
C
C        CODE FOR INCREMENT NOT EQUAL TO 1
C
      NINCX = N*INCX
      DO 10 I=1,NINCX,INCX
        SX(I) = SA*SX(I)
   10 CONTINUE
      RETURN
C
C        CODE FOR INCREMENT EQUAL TO 1
C
C
C        CLEAN-UP LOOP
C
   20 M = MOD(N,5)
      IF (M.EQ.0) GO TO 40
      DO 30 I=1,M
        SX(I) = SA*SX(I)
   30 CONTINUE
      IF (N.LT.5) RETURN
   40 MP1 = M + 1
      DO 50 I=MP1,N,5
        SX(I) = SA*SX(I)
        SX(I+1) = SA*SX(I+1)
        SX(I+2) = SA*SX(I+2)
        SX(I+3) = SA*SX(I+3)
        SX(I+4) = SA*SX(I+4)
   50 CONTINUE
      RETURN
      END


      SUBROUTINE SVD(NM, M, N, A, W, MATU, U, MATV, V, IERR, RV1)       
C
      INTEGER I, J, K, L, M, N, II, I1, KK, K1, LL, L1, MN, NM, ITS,
     + IERR
      REAL A(NM,N), W(N), U(NM,N), V(NM,N), RV1(N)
      REAL C, F, G, H, S, X, Y, Z, EPS, SCALE, MACHEP
      REAL SQRT, AMAX1, ABS, SIGN
      LOGICAL MATU, MATV
C
C     THIS SUBROUTINE IS A TRANSLATION OF THE ALGOL PROCEDURE SVD,
C     NUM. MATH. 14, 403-420(1970) BY GOLUB AND REINSCH.
C     HANDBOOK FOR AUTO. COMP., VOL II-LINEAR ALGEBRA, 134-151(1971).
C
C     THIS SUBROUTINE DETERMINES THE SINGULAR VALUE DECOMPOSITION
C          T
C     A=USV  OF A REAL M BY N RECTANGULAR MATRIX.  HOUSEHOLDER
C     BIDIAGONALIZATION AND A VARIANT OF THE QR ALGORITHM ARE USED.
C
C     ON INPUT-
C
C        NM MUST BE SET TO THE ROW DIMENSION OF TWO-DIMENSIONAL
C          ARRAY PARAMETERS AS DECLARED IN THE CALLING PROGRAM
C          DIMENSION STATEMENT.  NOTE THAT NM MUST BE AT LEAST
C          AS LARGE AS THE MAXIMUM OF M AND N,
C
C        M IS THE NUMBER OF ROWS OF A (AND U),
C
C        N IS THE NUMBER OF COLUMNS OF A (AND U) AND THE ORDER OF V,
C
C        A CONTAINS THE RECTANGULAR INPUT MATRIX TO BE DECOMPOSED,
C
C        MATU SHOULD BE SET TO .TRUE. IF THE U MATRIX IN THE
C          DECOMPOSITION IS DESIRED, AND TO .FALSE. OTHERWISE,
C
C        MATV SHOULD BE SET TO .TRUE. IF THE V MATRIX IN THE
C          DECOMPOSITION IS DESIRED, AND TO .FALSE. OTHERWISE.
C
C     ON OUTPUT-
C
C        A IS UNALTERED (UNLESS OVERWRITTEN BY U OR V),
C
C        W CONTAINS THE N (NON-NEGATIVE) SINGULAR VALUES OF A (THE
C          DIAGONAL ELEMENTS OF S).  THEY ARE UNORDERED.  IF AN
C          ERROR EXIT IS MADE, THE SINGULAR VALUES SHOULD BE CORRECT
C          FOR INDICES IERR+1,IERR+2,...,N,
C
C        U CONTAINS THE MATRIX U (ORTHOGONAL COLUMN VECTORS) OF THE
C          DECOMPOSITION IF MATU HAS BEEN SET TO .TRUE.  OTHERWISE
C          U IS USED AS A TEMPORARY ARRAY.  U MAY COINCIDE WITH A.
C          IF AN ERROR EXIT IS MADE, THE COLUMNS OF U CORRESPONDING
C          TO INDICES OF CORRECT SINGULAR VALUES SHOULD BE CORRECT,
C
C        V CONTAINS THE MATRIX V (ORTHOGONAL) OF THE DECOMPOSITION IF
C          MATV HAS BEEN SET TO .TRUE.  OTHERWISE V IS NOT REFERENCED.
C          V MAY ALSO COINCIDE WITH A IF U IS NOT NEEDED.  IF AN ERROR
C          EXIT IS MADE, THE COLUMNS OF V CORRESPONDING TO INDICES OF
C          CORRECT SINGULAR VALUES SHOULD BE CORRECT,
C
C        IERR IS SET TO
C          ZERO       FOR NORMAL RETURN,
C          K          IF THE K-TH SINGULAR VALUE HAS NOT BEEN
C                     DETERMINED AFTER 30 ITERATIONS,
C
C        RV1 IS A TEMPORARY STORAGE ARRAY.
C
C     QUESTIONS AND COMMENTS SHOULD BE DIRECTED TO B. S. GARBOW,
C     APPLIED MATHEMATICS DIVISION, ARGONNE NATIONAL LABORATORY
C
C     ------------------------------------------------------------------
C
C     ********** MACHEP IS A MACHINE DEPENDENT PARAMETER SPECIFYING
C                THE RELATIVE PRECISION OF FLOATING POINT ARITHMETIC.
C
C                **********
      MACHEP = 2.**(-26)
C
      IERR = 0
C
      DO 20 I=1,M
C
        DO 10 J=1,N
          U(I,J) = A(I,J)
   10   CONTINUE
   20 CONTINUE
C     ********** HOUSEHOLDER REDUCTION TO BIDIAGONAL FORM **********
      G = 0.0
      SCALE = 0.0
      X = 0.0
C
      DO 200 I=1,N
        L = I + 1
        RV1(I) = SCALE*G
        G = 0.0
        S = 0.0
        SCALE = 0.0
        IF (I.GT.M) GO TO 100
C
        DO 30 K=I,M
          SCALE = SCALE + ABS(U(K,I))
   30   CONTINUE
C
        IF (SCALE.EQ.0.0) GO TO 100
C
        DO 40 K=I,M
          U(K,I) = U(K,I)/SCALE
          S = S + U(K,I)**2
   40   CONTINUE
C
        F = U(I,I)
        G = -SIGN(SQRT(S),F)
        H = F*G - S
        U(I,I) = F - G
        IF (I.EQ.N) GO TO 80
C
        DO 70 J=L,N
          S = 0.0
C
          DO 50 K=I,M
            S = S + U(K,I)*U(K,J)
   50     CONTINUE
C
          F = S/H
C
          DO 60 K=I,M
            U(K,J) = U(K,J) + F*U(K,I)
   60     CONTINUE
   70   CONTINUE
C
   80   DO 90 K=I,M
          U(K,I) = SCALE*U(K,I)
   90   CONTINUE
C
  100   W(I) = SCALE*G
        G = 0.0
        S = 0.0
        SCALE = 0.0
        IF (I.GT.M .OR. I.EQ.N) GO TO 190
C
        DO 110 K=L,N
          SCALE = SCALE + ABS(U(I,K))
  110   CONTINUE
C
        IF (SCALE.EQ.0.0) GO TO 190
C
        DO 120 K=L,N
          U(I,K) = U(I,K)/SCALE
          S = S + U(I,K)**2
  120   CONTINUE
C
        F = U(I,L)
        G = -SIGN(SQRT(S),F)
        H = F*G - S
        U(I,L) = F - G
C
        DO 130 K=L,N
          RV1(K) = U(I,K)/H
  130   CONTINUE
C
        IF (I.EQ.M) GO TO 170
C
        DO 160 J=L,M
          S = 0.0
C
          DO 140 K=L,N
            S = S + U(J,K)*U(I,K)
  140     CONTINUE
C
          DO 150 K=L,N
            U(J,K) = U(J,K) + S*RV1(K)
  150     CONTINUE
  160   CONTINUE
C
  170   DO 180 K=L,N
          U(I,K) = SCALE*U(I,K)
  180   CONTINUE
C
  190   X = AMAX1(X,ABS(W(I))+ABS(RV1(I)))
  200 CONTINUE
C     ********** ACCUMULATION OF RIGHT-HAND TRANSFORMATIONS **********
      IF (.NOT.MATV) GO TO 290
C     ********** FOR I=N STEP -1 UNTIL 1 DO -- **********
      DO 280 II=1,N
        I = N + 1 - II
        IF (I.EQ.N) GO TO 270
        IF (G.EQ.0.0) GO TO 250
C
        DO 210 J=L,N
C     ********** DOUBLE DIVISION AVOIDS POSSIBLE UNDERFLOW **********
          V(J,I) = (U(I,J)/U(I,L))/G
  210   CONTINUE
C
        DO 240 J=L,N
          S = 0.0
C
          DO 220 K=L,N
            S = S + U(I,K)*V(K,J)
  220     CONTINUE
C
          DO 230 K=L,N
            V(K,J) = V(K,J) + S*V(K,I)
  230     CONTINUE
  240   CONTINUE
C
  250   DO 260 J=L,N
          V(I,J) = 0.0
          V(J,I) = 0.0
  260   CONTINUE
C
  270   V(I,I) = 1.0
        G = RV1(I)
        L = I
  280 CONTINUE
C     ********** ACCUMULATION OF LEFT-HAND TRANSFORMATIONS **********
  290 IF (.NOT.MATU) GO TO 410
C     **********FOR I=MIN(M,N) STEP -1 UNTIL 1 DO -- **********
      MN = N
      IF (M.LT.N) MN = M
C
      DO 400 II=1,MN
        I = MN + 1 - II
        L = I + 1
        G = W(I)
        IF (I.EQ.N) GO TO 310
C
        DO 300 J=L,N
          U(I,J) = 0.0
  300   CONTINUE
C
  310   IF (G.EQ.0.0) GO TO 370
        IF (I.EQ.MN) GO TO 350
C
        DO 340 J=L,N
          S = 0.0
C
          DO 320 K=L,M
            S = S + U(K,I)*U(K,J)
  320     CONTINUE
C     ********** DOUBLE DIVISION AVOIDS POSSIBLE UNDERFLOW **********
          F = (S/U(I,I))/G
C
          DO 330 K=I,M
            U(K,J) = U(K,J) + F*U(K,I)
  330     CONTINUE
  340   CONTINUE
C
  350   DO 360 J=I,M
          U(J,I) = U(J,I)/G
  360   CONTINUE
C
        GO TO 390
C
  370   DO 380 J=I,M
          U(J,I) = 0.0
  380   CONTINUE
C
  390   U(I,I) = U(I,I) + 1.0
  400 CONTINUE
C     ********** DIAGONALIZATION OF THE BIDIAGONAL FORM **********
  410 EPS = MACHEP*X
C     ********** FOR K=N STEP -1 UNTIL 1 DO -- **********
      DO 550 KK=1,N
        K1 = N - KK
        K = K1 + 1
        ITS = 0
C     ********** TEST FOR SPLITTING.
C                FOR L=K STEP -1 UNTIL 1 DO -- **********
  420   DO 430 LL=1,K
          L1 = K - LL
          L = L1 + 1
          IF (ABS(RV1(L)).LE.EPS) GO TO 470
C     ********** RV1(1) IS ALWAYS ZERO, SO THERE IS NO EXIT
C                THROUGH THE BOTTOM OF THE LOOP **********
          IF (ABS(W(L1)).LE.EPS) GO TO 440
  430   CONTINUE
C     ********** CANCELLATION OF RV1(L) IF L GREATER THAN 1 **********
  440   C = 0.0
        S = 1.0
C
        DO 460 I=L,K
          F = S*RV1(I)
          RV1(I) = C*RV1(I)
          IF (ABS(F).LE.EPS) GO TO 470
          G = W(I)
          H = SQRT(F*F+G*G)
          W(I) = H
          C = G/H
          S = -F/H
          IF (.NOT.MATU) GO TO 460
C
          DO 450 J=1,M
            Y = U(J,L1)
            Z = U(J,I)
            U(J,L1) = Y*C + Z*S
            U(J,I) = -Y*S + Z*C
  450     CONTINUE
C
  460   CONTINUE
C     ********** TEST FOR CONVERGENCE **********
  470   Z = W(K)
        IF (L.EQ.K) GO TO 530
C     ********** SHIFT FROM BOTTOM 2 BY 2 MINOR **********
        IF (ITS.EQ.30) GO TO 560
        ITS = ITS + 1
        X = W(L)
        Y = W(K1)
        G = RV1(K1)
        H = RV1(K)
        F = ((Y-Z)*(Y+Z)+(G-H)*(G+H))/(2.0*H*Y)
        G = SQRT(F*F+1.0)
        F = ((X-Z)*(X+Z)+H*(Y/(F+SIGN(G,F))-H))/X
C     ********** NEXT QR TRANSFORMATION **********
        C = 1.0
        S = 1.0
C
        DO 520 I1=L,K1
          I = I1 + 1
          G = RV1(I)
          Y = W(I)
          H = S*G
          G = C*G
          Z = SQRT(F*F+H*H)
          RV1(I1) = Z
          C = F/Z
          S = H/Z
          F = X*C + G*S
          G = -X*S + G*C
          H = Y*S
          Y = Y*C
          IF (.NOT.MATV) GO TO 490
C
          DO 480 J=1,N
            X = V(J,I1)
            Z = V(J,I)
            V(J,I1) = X*C + Z*S
            V(J,I) = -X*S + Z*C
  480     CONTINUE
C
  490     Z = SQRT(F*F+H*H)
          W(I1) = Z
C     ********** ROTATION CAN BE ARBITRARY IF Z IS ZERO **********
          IF (Z.EQ.0.0) GO TO 500
          C = F/Z
          S = H/Z
  500     F = C*G + S*Y
          X = -S*G + C*Y
          IF (.NOT.MATU) GO TO 520
C
          DO 510 J=1,M
            Y = U(J,I1)
            Z = U(J,I)
            U(J,I1) = Y*C + Z*S
            U(J,I) = -Y*S + Z*C
  510     CONTINUE
C
  520   CONTINUE
C
        RV1(L) = 0.0
        RV1(K) = F
        W(K) = X
        GO TO 420
C     ********** CONVERGENCE **********
  530   IF (Z.GE.0.0) GO TO 550
C     ********** W(K) IS MADE NON-NEGATIVE **********
        W(K) = -Z
        IF (.NOT.MATV) GO TO 550
C
        DO 540 J=1,N
          V(J,K) = -V(J,K)
  540   CONTINUE
C
  550 CONTINUE
C
      GO TO 570
C     ********** SET ERROR -- NO CONVERGENCE TO A
C                SINGULAR VALUE AFTER 30 ITERATIONS **********
  560 IERR = K
  570 RETURN
C     ********** LAST CARD OF SVD **********
      END
C                                                                       
C     ------------------------------------------------------------------
C                                                                       
      SUBROUTINE SSWAP(N, SX, INCX, SY, INCY)                           
C
C     INTERCHANGES TWO VECTORS.
C     USES UNROLLED LOOPS FOR INCREMENTS EQUAL TO 1.
C     JACK DONGARRA, LINPACK, 3/11/78.
C
      REAL SX(1), SY(1), STEMP
      INTEGER I, INCX, INCY, IX, IY, M, MP1, N
C
      IF (N.LE.0) RETURN
      IF (INCX.EQ.1 .AND. INCY.EQ.1) GO TO 20
C
C       CODE FOR UNEQUAL INCREMENTS OR EQUAL INCREMENTS NOT EQUAL
C         TO 1
C
      IX = 1
      IY = 1
      IF (INCX.LT.0) IX = (-N+1)*INCX + 1
      IF (INCY.LT.0) IY = (-N+1)*INCY + 1
      DO 10 I=1,N
        STEMP = SX(IX)
        SX(IX) = SY(IY)
        SY(IY) = STEMP
        IX = IX + INCX
        IY = IY + INCY
   10 CONTINUE
      RETURN
C
C       CODE FOR BOTH INCREMENTS EQUAL TO 1
C
C
C       CLEAN-UP LOOP
C
   20 M = MOD(N,3)
      IF (M.EQ.0) GO TO 40
      DO 30 I=1,M
        STEMP = SX(I)
        SX(I) = SY(I)
        SY(I) = STEMP
   30 CONTINUE
      IF (N.LT.3) RETURN
   40 MP1 = M + 1
      DO 50 I=MP1,N,3
        STEMP = SX(I)
        SX(I) = SY(I)
        SY(I) = STEMP
        STEMP = SX(I+1)
        SX(I+1) = SY(I+1)
        SY(I+1) = STEMP
        STEMP = SX(I+2)
        SX(I+2) = SY(I+2)
        SY(I+2) = STEMP
   50 CONTINUE
      RETURN
      END

