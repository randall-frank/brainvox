
	program test7
c
c	Test the calculation of the diagonal elements of the hat matrix
c	using the Cloud seeding example on page 4 of Cook & Weisberg.
c	The data are contained in the file: woodley.dat
c	The elements of the diagonal of the hat matrix are on page 127.
c	The model is basically that given in (1.1.3) on page 5, but with
c	the cube root transformation of variable P.
c
	integer np, nrbar, ier, row, in, col
	double precision x(24,6), y(24), xrow(11), d(11), rbar(55) 
	double precision tol(11), hii(24), 
     +		thetab(11), sserr, wt, wk(11), one, zero, sum, third
c
	logical seeded
	data in/8/, wt/1.0/, one/1.0/, zero/0.0/
c
	third = one / 3
	np = 11
	nrbar = 55
	nobs = 24
c
c	Read in the data.
c
	open(in, file='woodley.data', status='old')
	do 5 row = 1, nobs
    5	read(in, *)(x(row, col), col=1,6), y(row)
c
	call clear(np, nrbar, d, rbar, thetab, sserr, ier)
	xrow(1) = one
	do 10 row = 1, nobs
	  xrow(3) = x(row, 2)
	  xrow(4) = x(row, 3)
	  xrow(5) = x(row, 4)
	  xrow(6) = x(row, 5)**third
	  xrow(7) = x(row, 6)
	  seeded = (x(row, 1) .eq. 1)
	  if (seeded) then
	    xrow(2) = one
	    xrow(8) = x(row, 3)
	    xrow(9) = x(row, 4)
	    xrow(10) = xrow(6)
	    xrow(11) = x(row, 6)
	  else
	    xrow(2) = zero
	    xrow(8) = zero
	    xrow(9) = zero
	    xrow(10) = zero
	    xrow(11) = zero
	  end if
	  call includ(np, nrbar, wt, xrow, y(row), d, rbar, thetab,
     +			sserr, ier)
   10	continue
c
	call tolset(np, nrbar, d, rbar, tol, wk, ier)
	sum = zero
	do 20 row = 1, nobs
	  xrow(3) = x(row, 2)
	  xrow(4) = x(row, 3)
	  xrow(5) = x(row, 4)
	  xrow(6) = x(row, 5)**third
	  xrow(7) = x(row, 6)
	  seeded = (x(row, 1) .eq. 1)
	  if (seeded) then
	    xrow(2) = one
	    xrow(8) = x(row, 3)
	    xrow(9) = x(row, 4)
	    xrow(10) = xrow(6)
	    xrow(11) = x(row, 6)
	  else
	    xrow(2) = zero
	    xrow(8) = zero
	    xrow(9) = zero
	    xrow(10) = zero
	    xrow(11) = zero
	  end if
	  call hdiag(xrow, np, nrbar, d, rbar, tol, np, hii(row), wk, ier)
	  sum = sum + hii(row)
   20	continue
	write(*, *)'Diagonal elements of Hat matrix:'
	write(*, '(1x, 6f10.4)') hii
	write(*, *)
	write(*, *)'Sum of diagonal elements = ', sum
	end

