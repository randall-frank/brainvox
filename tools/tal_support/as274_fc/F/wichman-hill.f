      REAL FUNCTION RAND()
C
C     The Wichmann & Hill random number generator
C     Algorithm AS183, Appl. Statist., 31, 188-190, 1982.
C     The cycle length is 6.95E+12.
C     This random number generator is very slow compared with most
C     others, but it is dependable, and the results are reproducible.
C
      INTEGER*4 IX, IY, IZ
      COMMON /RANDC/ IX, IY, IZ
C
      IX = MOD(171*IX, 30269)
      IY = MOD(172*IY, 30307)
      IZ = MOD(170*IZ, 30323)
      RAND = MOD(FLOAT(IX)/30269. + FLOAT(IY)/30307. +
     1 FLOAT(IZ)/30323. , 1.0)
      RETURN
      END

