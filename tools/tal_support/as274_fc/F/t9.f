	program wampler
c
c	Test of includ, tolset, sing and regcf, using the 10x9 test
c	matrices from:
c	Wampler, R.H. (1980) Test procedures and test problems for
c	     least squares algorithms. J. of Econometrics, 12, 3-22.
c
	double precision weight, xrow(9), y, d(9), rbar(36)
	double precision thetab(9), sserr,
     +		tol(9), work(9), beta(9), zero, one, pt1, nine, eps
	integer np, nrbar, ier, k, i, j
	logical lindep(9)
	data weight/1.0/, zero/0.0/, one/1.0/, pt1/0.1/, nine/9.0/,
     +          np/9/, nrbar/36/
c
c       Single precision, starting eps = 0.1
c       Double precision, starting eps = 1.d-05
c
	eps = 1.0d-5
c
	do 50 k = 1, 5
	  call clear(np, nrbar, d, rbar, thetab, sserr, ier)
	  do 10 j = 1, 9
   10	  xrow(j) = one
	  y = nine + eps
	  call includ(np, nrbar, weight, xrow, y, d, rbar, thetab,
     +          sserr, ier)
	  y = eps
	  do 30 i = 2, 10
	    do 20 j = 2, 9
   20       xrow(j) = zero
	    xrow(1) = one
	    if (i .lt. 10) xrow(i) = eps
	    call includ(np, nrbar, weight, xrow, y, d, rbar, thetab,
     +		sserr, ier)
	    if (i .eq. 9) y = nine - eps
   30	  continue
c
c	Set tol & then test for singularities
c
	  call tolset(np, nrbar, d, rbar, tol, work, ier)
	  call sing(np, nrbar, d, rbar, thetab, sserr, tol, lindep,
     +          work, ier)
	  write(*, *)'eps = ', eps
c
c	Calculate regression coefficients if non-singular
c
	  if (ier .eq. 0) then
	    call regcf(np, nrbar, d, rbar, thetab, tol, beta, np, ier)
	    do 40 i = 1, 9
   40	    work(i) = beta(i) - one
	    write(*, 900)(work(i),i=1,9)
  900       format(' Errors in regression coefficients:', 2(/1x,5e11.3))
	  else
	    write(*, *)'ier = ', ier
	  end if
	  write(*, *)
	  eps = eps * pt1
   50	continue
	end
