
      program test8
c
c     Test handling of a case with more variables than cases.
c     X1, .., X4 from random number generator; Y = X1 + X2 + X3 + X4
c     X5 = X1 + X2
c     X6 =      X2 + X3
c     X7 =           X3 + X4
c     X8 = X1           + X4   Hence Y = (X5 + X6 + X7 + X8)/2
c
c     ncases = 5
c
c     The matrix RBAR should look like:
c		(1)   1   2   3   4   5   6   7
c		     (1)  8   9  10  11  12  13
c			 (1) 14  15  16  17  18
c			     (1) 19  20  21  22
c				 (1) 23  24  25
c                                    (1) 26  27
c                                        (1) 28
c     where (1) indicates an implicit 1.0 and the other numbers are the
c     locations within array RBAR.   Elements 26-28 should be zero.
c
      integer np, nrbar
      parameter (np=8, nrbar=28)
      integer ix, iy, iz, ier, case, i, vorder(np), list(np)
      real rand
      double precision d(np), rbar(28), thetab(np), rss(np),
     +    wt, tol(np), sserr, x(8), y, work(np), beta(np)
      logical lindep(np)
      common /randc/ ix, iy, iz
      data wt/1.0d0/

      ix = 777
      iy = 777
      iz = 777
      call clear(np, nrbar, d, rbar, thetab, sserr, ier)
c
c     Generate 5 lines of data
c
      do 20 case = 1, 5
	do 10 i = 1, 4
   10   x(i) = rand()
	x(5) = x(1) + x(2)
	x(6) = x(2) + x(3)
	x(7) = x(3) + x(4)
	x(8) = x(4) + x(1)
	y = x(5) + x(7)
	call includ(np, nrbar, wt, x, y, d, rbar, thetab,
     +		sserr, ier)
   20 continue
c
c     Now look at the factorization
c
      write(*, 900) d
  900 format(' D = ', 8f9.5)
      write(*, 910)(rbar(i),i=1,7)
  910 format(' RBAR = '/ 1x, 'Row 1: (1) ', 7f9.5)
      write(*, 920)(rbar(i),i=8,13)
  920 format(1x, 'Row 2:', 9x, ' (1) ', 6f9.5)
      write(*, 930)(rbar(i),i=14,18)
  930 format(1x, 'Row 3:', 18x, ' (1) ', 5f9.5)
      write(*, 940)(rbar(i),i=19,22)
  940 format(1x, 'Row 4:', 27x, ' (1) ', 4f9.5)
      write(*, 950)(rbar(i),i=23,25)
  950 format(1x, 'Row 5:', 36x, ' (1) ', 3f9.5)
      write(*, 960)(rbar(i),i=26,28)
  960 format(1x, 'Row 6:', 45x, ' (1) ', 2f9.5/
     +       1x, 'Row 7:', 54x, ' (1) ', f9.5)

      call tolset(np, nrbar, d, rbar, tol, work, ier)
      call sing(np, nrbar, d, rbar, thetab, sserr, tol,
     +		lindep, work, ier)
      write(*, *)' IER = ', ier
      write(*, *)' LINDEP = ', lindep
      call ss(np, d, thetab, sserr, rss, ier)
c
c     Calculate regression coefficients for first 4 variables.
c
      call regcf(np, nrbar, d, rbar, thetab, tol, beta,
     +		4, ier)
      write(*, 970) (beta(i),i=1,4)
  970 format(' Regression coefficients:'/ 1x, 4f10.5/)
c
c     Re-order variables as X5 .. X8, X1 .. X4
c     N.B.  X5 + X7 = X6 + X8
c
      new = 5
      do 30 i = 1, 8
	vorder(i) = i
	list(i) = new
	if (i .eq. 4) then
	  new = 1
	else
	  new = new + 1
	end if
   30 continue
      call reordr(np, nrbar, vorder, d, rbar, thetab, rss,
     +		tol, list, 4, 1, ier)
c
c     Now look at the factorization again
c
      write(*, 900) d
      write(*, 910)(rbar(i),i=1,7)
      write(*, 920)(rbar(i),i=8,13)
      write(*, 930)(rbar(i),i=14,18)
      write(*, 940)(rbar(i),i=19,22)
      write(*, 950)(rbar(i),i=23,25)
      write(*, 960)(rbar(i),i=26,28)
      call sing(np, nrbar, d, rbar, thetab, sserr, tol,
     +		lindep, work, ier)
      write(*, *)' IER = ', ier
      write(*, *)' LINDEP = ', lindep
c
c     Calculate regression coefficients for first 4 variables.
c
      call regcf(np, nrbar, d, rbar, thetab, tol, beta, 4, ier)
      write(*, 970) (beta(i),i=1,4)

      end
