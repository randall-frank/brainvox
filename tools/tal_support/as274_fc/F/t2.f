
      program test2
c
c     Test vmove using the Longley data.
c
      double precision x(16,6), y(16), d(7), rbar(21), thetab(7)
	  double precision xrow(7), yelem,
     +   sserr, beta(7), work(7), tol(7), rss(7)
      integer i, j, ifault, vorder(7)
      logical lindep(7)
c
      data vorder/0, 1, 2, 3, 4, 5, 6/
c
      write(*, *)'Using the Longley data'
      write(*, *)

      open(9, file='longley.data', status='old')
      do 10 i = 1, 16
   10 read(9, *)(x(i,j),j=1,6), y(i)
c
c     Use includ to form the orthogonal reduction.
c
      do 20 i = 1, 7
	d(i) = 0.0
	thetab(i) = 0.0
   20 continue
      do 30 i = 1, 21
   30 rbar(i) = 0.0
      sserr = 0.0

      do 50 i = 1, 16
	xrow(1) = 1.0
	do 40 j = 1, 6
   40   xrow(j+1) = x(i,j)
	yelem = y(i)
	call includ(7, 21, 1.0d0, xrow, yelem, d, rbar, thetab, sserr,
     +      ifault)
   50 continue
c
c     Call sing to detect singularities.
c
      call tolset(7, 21, d, rbar, tol, work, ifault)
      call sing(7, 21, d, rbar, thetab, sserr, tol, lindep, work,
     +    ifault)
      write(*, *)'sserr = ', sserr
      call ss(7, d, thetab, sserr, rss, ifault)
c
c     Calculate regression coefficients, using vmove to cycle through
c     the order of the variables.
c
      do 60 i = 1, 7
	call regcf(7, 21, d, rbar, thetab, tol, beta, 7, ifault)
	write(*, 900) beta
  900   format(' Regn. coeffs.:'/ 1x, 5g15.7/ 1x, 2g15.7)
	write(*, 910) d, rbar, thetab
  910   format(' d: ', 5g13.5/1x,2g13.5
     +       / ' rbar:'/ 1x,6g13.5/ 14x,5g13.5/ 27x,4g13.5/
     +      40x,3g13.5/ 53x,2g13.5/ 66x,g13.5/
     +       ' thetab: '/1x, 5g13.5/ 1x, 2g13.5//)
c
c     Move variable in 1st position to the last.
c
	call vmove(7, 21, vorder, d, rbar, thetab, rss, 2, 7, tol,
     +      ifault)
   60 continue
      end
