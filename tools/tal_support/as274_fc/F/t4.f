      program test4
c
c     Test of subroutine INV.
c
      double precision rbar(21), rinv(15), rho, zero
      integer pos, row, col
c
      data rho/0.6/, zero/0.0/
c
      pos = 1
      do 20 row = 1, 6
	do 10 col = row+1, 7
	  if (col .eq. row+1) then
	    rbar(pos) = rho
	  else
	    rbar(pos) = zero
	  end if
	  pos = pos + 1
   10   continue
   20 continue
      call inv(7, 21, rbar, 6, rinv)
C I added the "21" argument.  It's supposed to be nrbar.
C     - ajay shah
      write(*, 900) rinv
  900 format(1x, 5g14.6/ 15x, 4g14.6/ 29x, 3g14.6/ 43x, 2g14.6/
     +      57x, g14.6)
      end
