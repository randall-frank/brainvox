C
      SUBROUTINE CONFND (NP,NRBAR,J,RBAR,CONTRA,IFAULT)
C 
C     Algorithm AS 75.2 Appl. Statist. (1974) Vol. 23, No. 3, P448
C 
C     Calling this subroutine obtains the contrast which could not
C     be estimated if D(j) were assumed to be zero, that is, obtains the
C     linear combination of the first j columns which would be zero.  Th
C     obtained by setting the first j-1 elements of contra to the soluti
C     of the triangular system formed by the first j-1 rows and columns
C     rbar with the first j-1 elements of the jth column as right hand
C     side, setting the jth element of contra to -1, and setting the
C     remaining elements of contra to zero.
C 
      DOUBLE PRECISION RBAR(NRBAR),CONTRA(NP),ZERO,ONE
C 
      DATA ZERO/0.0D0/,ONE/1.0D0/
C 
C         check input parameters
C 
      IFAULT=1
      IF (NP.LT.1.OR.NRBAR.LE.NP*(NP-1)/2) RETURN
      IFAULT=0
C 
      JM=J-1
      IF (J.EQ.NP) GO TO 20
      JP=J+1
      DO 10 I=JP,NP
 10     CONTRA(I)=ZERO
 20   CONTRA(J)=-ONE
      IF (J.EQ.1) RETURN
      DO 40 IJ=1,JM
        I=J-IJ
        NEXTR=(I-1)*(NP+NP-I)/2+1
        K=NEXTR+J-I-1
        CONTRA(I)=RBAR(K)
        IF (I.EQ.JM) GO TO 40
        IP=I+1
        DO 30 K=IP,JM
          CONTRA(I)=CONTRA(I)-RBAR(NEXTR)*CONTRA(K)
          NEXTR=NEXTR+1
 30     CONTINUE
 40   CONTINUE
      RETURN
      END
C
C
      SUBROUTINE SSDCMP (NP,D,THETAB,SS,IFAULT)
C 
C     Algorithm AS75.3  Appl. Statist. (1974) Vol.23, No. 3, P448
C 
C     Calling this subroutine computes the np components of the sum
C     of squares decomposition from D and thetab.
C 
      DOUBLE PRECISION D(NP),THETAB(NP),SS(NP)
C 
C        check input parameters
C 
      IFAULT=1
      IF (NP.LT.1) RETURN
      IFAULT=0
C 
      DO 10 I=1,NP
 10     SS(I)=D(I)*THETAB(I)**2
      RETURN
      END
C
C
      SUBROUTINE REGRSS (NP,NRBAR,RBAR,THETAB,BETA,IFAULT)
C 
C     Algorithm AS 75.4  Appl. Statist. (1974), Vol. 23, No. 3, p448
C 
C     Calling this subroutine obtains beta by back-substitution in
C     the triangular system rbar and thetab.
C 
      DOUBLE PRECISION RBAR(NRBAR),THETAB(NP),BETA(NP)
C 
C        check input parameters
C 
      IFAULT=1
      IF (NP.LT.1.OR.NRBAR.LE.NP*(NP-1)/2) RETURN
      IFAULT=0
C 
      DO 20 J=1,NP
        I=NP-J+1
        BETA(I)=THETAB(I)
        NEXTR=(I-1)*(NP+NP-I)/2+1
        IP=I+1
        DO 10 K=IP,NP
          BETA(I)=BETA(I)-RBAR(NEXTR)*BETA(K)
          NEXTR=NEXTR+1
 10     CONTINUE
 20   CONTINUE
      RETURN
      END
