      program test1
c
c     Test vmove.
c     The LS fit to the data is:  Y = 1 + X + X**2 + X**3
c     i.e. all of the regression coefficients should be exactly 1.0.
c     An extra variable equal to X + X**2 is inserted to give a singularity.
c
      double precision x(11,4), y(11), d(5), rbar(10), thetab(5)
	  double precision xrow(5), yelem,
     +   sserr, beta(5), work(5), tol(5), rss(5)
      integer i, j, ifault, vorder(5)
      logical lindep(5)

      data y/65647., 70638., 75889., 81399., 87169., 93202., 99503.,
     +   106079., 112939., 120094., 127557./
      data vorder/0, 1, 2, 3, 4/

      write(*, *)'Fitting nasty cubic'
      write(*, *)'1st 4 regression coefficients should equal 1.0'
      write(*, *)

      do 10 i = 1, 11
	x(i,1) = i + 39
	x(i,2) = x(i,1)**2
	x(i,3) = x(i,1) * x(i,2)
	x(i,4) = x(i,1) + x(i,2)
   10 continue
c
c     Use includ to form the orthogonal reduction.
c
      do 20 i = 1, 5
	d(i) = 0.0
	thetab(i) = 0.0
   20 continue
      do 30 i = 1, 10
   30 rbar(i) = 0.0
      sserr = 0.0

      do 50 i = 1, 11
	xrow(1) = 1.0
	do 40 j = 1, 4
   40   xrow(j+1) = x(i,j)
	yelem = y(i)
	call includ(5, 10, 1.0d0, xrow, yelem, d, rbar, thetab, sserr,
     +      ifault)
   50 continue
c
c     Call tolset to set tolerances, then sing to detect singularities.
c
      call tolset(5, 10, d, rbar, tol, work, ifault)
      call sing(5, 10, d, rbar, thetab, sserr, tol, lindep, work,
     +    ifault)
      write(*, *)'sserr = ', sserr, '   Should be 286.000'
c
c     Calculate regression coefficients, using vmove to cycle through
c     the order of the variables.
c
      do 60 i = 1, 6
	call regcf(5, 10, d, rbar, thetab, tol, beta, 5, ifault)
	write(*, *)'Variable order:', vorder
	write(*, 900) beta
  900   format(' Regn. coeffs.:'/ 1x, 5g15.7)
	write(*, 910) d, rbar, thetab
  910   format(' d: ', 5g15.7/ ' rbar:'/1x,4g15.7/16x,3g15.7/31x,2g15.7/
     +      46x,g15.7/ ' thetab: '/1x, 5g15.7//)
c
c     Calculate residual sums of squares (RSS).
c
	call ss(5, d, thetab, sserr, rss, ifault)
c
c     Move variable in 1st position to the last.
c
	call vmove(5, 10, vorder, d, rbar, thetab, rss, 1, 5, tol,
     +          ifault)
   60 continue
      end

