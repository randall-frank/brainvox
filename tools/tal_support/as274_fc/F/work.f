
	program test7
c
c	Test the calculation of the diagonal elements of the hat matrix
c	using the Cloud seeding example on page 4 of Cook & Weisberg.
c	The data are contained in the file: woodley.dat
c	The elements of the diagonal of the hat matrix are on page 127.
c	The model is basically that given in (1.1.3) on page 5, but with
c	the cube root transformation of variable P.
c
	integer np, nrbar, ier, row, in, col,i
	double precision x(15,6), y(15), xrow(4), d(4), rbar(55) 
	double precision tol(4), hii(15), press,
     +		thetab(4), sserr, wt, wk(4), one, zero, sum, third,coef(4)
	real	sst,tmp
c
	logical seeded
	data in/8/, wt/1.0/, one/1.0/, zero/0.0/
c
	np = 4
	nrbar = 55
	nobs = 15
c
c	Read in the data.
c
	open(in, file='rjf.data', status='old')
	Do row = 1, nobs
   		read(in, *)(x(row, col), col=1,3), y(row)
		write(*,*) row,x(row,1),x(row,2),x(row,3),y(row)
	Enddo
c
	call clear(np, nrbar, d, rbar, thetab, sserr, ier)
	do 10 row = 1, nobs
	  xrow(1) = one
	  xrow(2) = x(row, 1)
	  xrow(3) = x(row, 2)
	  xrow(4) = x(row, 3)
	  call includ(np, nrbar, wt, xrow, y(row), d, rbar, thetab,
     +			sserr, ier)
   10	continue
c
	call tolset(np, nrbar, d, rbar, tol, wk, ier)
	call regcf(np,nrbar,d,rbar,thetab,tol,coef,np,ier)
	write(*,*) coef(1),coef(2),coef(3),coef(4)
	sum = zero
	press = zero
	do 20 row = 1, nobs
	  xrow(1) = one
	  xrow(2) = x(row, 1)
	  xrow(3) = x(row, 2)
	  xrow(4) = x(row, 3)
C residual
	  tmp = y(row) - (coef(1)*xrow(1) + coef(2)*xrow(2) + 
     +			coef(3)*xrow(3) + coef(4)*xrow(4))
	  write(*,*) y(row),tmp
C Hii
	  call hdiag(xrow, np, nrbar, d, rbar, tol, np, hii(row), wk, ier)

	  press = press + (tmp/(1-hii(row)))**2
	  sum = sum + hii(row)
   20	continue
	write(*, *)'Diagonal elements of Hat matrix:'
	write(*, '(1x, 6f10.4)') hii
	write(*, *)
	write(*, *)'Sum of diagonal elements = ', sum

	sst = 0
	tmp = 0
	Do i=1,nobs
		tmp = tmp + y(i)
	Enddo
	tmp = tmp/float(nobs)
	Do i=1,nobs
		sst = sst + (y(i)-tmp)**2
	Enddo

	call ss(np,d,thetab,sser,wk,ier)

	write(*,*) "SSres =", sserr
	write(*,*) "SStotal =",sst,wk(1)
        write(*,*) "PRESS = ", press
        write(*,*) "R2 = ", 1.0 - (sserr/sst)
        write(*,*) "R2(adj) = ",1.0-((sserr/(nobs-np))/(sst/(nobs-1)))
	write(*,*) "R2(pred) = ", 1.0 - press/sst
	write(*,*) "S2 = ", sserr/(nobs-np)
	end

