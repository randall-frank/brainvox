      program Sim_Les
c
c     Use the test problem from:
c       Simon, S.D. & Lesage, J.P. (1988). Benchmarking numerical
c           accuracy of statistical algorithms.  Comput.Statist. &
c           Data Anal., vol.7, 197-209.
c
c     Parameters to be set by the user:
c     n   number of different rows of data.   The number of columns will
c         be (n-1) of which the first will contain 1s.
c     b   the number of blocks of identical data.   The number of cases
c         generated will be n x b.
c     eps small values give ill-conditioning (try 1.d-04).
c     gamma determines the relative size of the predictors compared with
c         the constant.   A large value, say 1.D+08, makes the constant
c         difficult to determine accurately.
c     delta is the ratio between consecutive variables (approximately).
c
c     But what should the regression coefficients be ?
c     It appears that they should be 1.0 when delta = 1, but that is
c     from observation - the paper by Simon & Lesage does not say, nor
c     does another later reference which I have used.
c
      implicit none           ! This is not standard Fortran yet

	integer n, np, nrbar, b, block, row, col, ier, nreq, pos
      double precision eps, gamma, delta, x(50), y, one, rhs_const,
     +    mult_factor, d(50), rbar(1225), thetab(50), sserr, weight,
     +    tol(50), work(50), beta(50), sum, temp
      logical lindep(50)

      data one /1.d0/

   10 write(*, *)'Enter number of different rows (range 2-51): '
      read(*, *) n
      np = n - 1
      nrbar = np * (np-1) / 2
      write(*, *)'Enter the number of blocks (at least 1): '
      read(*, *) b
      write(*, *)'Ill-conditioning parameter'
      write(*, *)'Enter small +ve value for eps: '
      read(*, *) eps
      write(*, *)'Importance of constant parameter'
      write(*, *)'Enter value for gamma: '
      read(*, *) gamma
      write(*, *)'Scaling parameter'
      write(*, *)'Enter value for delta: '
      read(*, *) delta
c
c     Generate the artificial data
c
      call CLEAR(NP, NRBAR, D, RBAR, THETAB, SSERR, IER)
      rhs_const = (n-2)*gamma + eps
      mult_factor = one + eps/gamma
      weight = one
      do 50 block = 1, b
c
c     Special case for first row
c
        x(1) = one
        x(2) = (one + gamma) * delta
        do 20 col = 3, n-1
   20   x(col) = x(col-1) * delta
        y = (n-1) + rhs_const
        call INCLUD(NP, NRBAR, WEIGHT, X, Y, D, RBAR, THETAB, SSERR,
     +       IER)
c
c     Rows 2 to n
c
        do 40 row = 2, n
          x(1) = one
          x(2) = gamma * delta
          do 30 col = 3, n-1
   30     x(col) = x(col-1) * delta
          if (row .lt. n) x(row) = x(row) * mult_factor
          y = rhs_const
	    if (row .eq. n) y = y + (n-1) - 2.d0*eps
          call INCLUD(NP, NRBAR, WEIGHT, X, Y, D, RBAR, THETAB, SSERR,
     +         IER)
   40   continue
   50 continue
c
c     Calculate regression coefficients without taking the precaution
c     of calling SING first.
c
      call TOLSET(NP, NRBAR, D, RBAR, TOL, WORK, IER)
      call REGCF(NP, NRBAR, D, RBAR, THETAB, TOL, BETA, NP, IER)
      write(*, 900) (beta(col),col=1,np)
  900 format(1x, 'Regression coefficients when SING has not been used',
     +    10(/1x, 5g15.6))
c
c     Now call SING to tidy up any singularities
c
      call SING(NP, NRBAR, D, RBAR, THETAB, SSERR, TOL, LINDEP, WORK,
     +    IER)
      if (ier .ne. 0) write(*, *) -ier, ' singularities reported'
      nreq = np + ier
      call REGCF(NP, NRBAR, D, RBAR, THETAB, TOL, BETA, NREQ, IER)
      write(*, 910) (beta(col),col=1,nreq)
  910 format(1x, 'Regression coefficients after SING has been used',
     +    10(/1x, 5g15.6))
c
c     Show sizes of diagonal elements of the Cholesky factorization,
c     i.e. sqrt(d(i)), compared with the length of the column for the
c     corresponding variable.   If the Cholesky diagonal element is
c     small compared with the length of the column, it is an indicator
c     of ill-conditioning.
c
	write(*, *)'Column   Col.length   Cholesky diagonal   Tolerance'
	do 70 col = 1, np
	  sum = d(col)
	  temp = sqrt(sum)
	  pos = col - 1
	  do 60 row = 1, col-1
	    sum = sum + d(row) * rbar(pos)**2
	    pos = pos + np - row - 1
   60   continue
	  write(*, 920) col, sqrt(sum), temp, tol(col)
  920   format(1x, i3, 3(4x, g13.4))
   70 continue
      end

