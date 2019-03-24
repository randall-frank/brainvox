
      program test5
c
c     Test of subroutine COV.
c
      double precision d(7), rbar(21), rinv(15), one, rho
      double precision zero, covmat(21), var,
     +      sterr(6)
      integer pos, row, col, ier
c
      data one/1.0/, rho/0.6/, zero/0.0/
c
      pos = 1
      do 20 row = 1, 6
	d(row) = one
	do 10 col = row+1, 7
	  if (col .eq. row+1) then
	    rbar(pos) = rho
	  else
	    rbar(pos) = zero
	  end if
	  pos = pos + 1
   10   continue
   20 continue
      var = one
      call cov(7, 21, d, rbar, 6, rinv, var, covmat, 21, sterr, ier)
      write(*, 900) covmat
  900 format(1x, 6f12.6/ 13x, 5f12.6/ 25x, 4f12.6/ 37x, 3f12.6/
     +      49x, 2f12.6/ 61x, f12.6)
      end
