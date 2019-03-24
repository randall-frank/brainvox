      program test6
c     Use the STEAM data from Draper & Smith to test PCORR.

      integer ier, case, col
      double precision d(10), rbar(45), thetab(10), sserr 
      double precision x(10), y, one, work(10), cormat(36), ycorr(10)
      data one/1.0/

      open(9, file='steam.data', status='old')
      call clear(10, 45, d, rbar, thetab, sserr, ier)

      x(1) = one
      do case = 1, 25
         read(9, *) (x(col),col=2,10), y
         call includ(10, 45, one, x, y, d, rbar, thetab, sserr, ier)
      end do
      write(*, *) 'ier = ', ier, ' and sserr = ', sserr

      call pcorr(10, 45, d, rbar, thetab, sserr, 1, work, cormat, 36,
     +      ycorr, ier)
      if (ier .ne. 0) write(*, *) 'pcorr() returned ', ier

      write(*, 900) (ycorr(col),col=1,10)
  900 format(' Correlations with the dependent variable:'/ 1x, 10f7.4/)
      write(*, 910) cormat
  910 format(' Correlations amongst the predictors:'/
     +       1x, '1.0', 3x, 8f8.4/
     +       9x, '1.0', 3x, 7f8.4/
     +      17x, '1.0', 3x, 6f8.4/
     +      25x, '1.0', 3x, 5f8.4/
     +      33x, '1.0', 3x, 4f8.4/
     +      41x, '1.0', 3x, 3f8.4/
     +      49x, '1.0', 3x, 2f8.4/
     +      57x, '1.0', 3x, f8.4/
     +      65x, '1.0')
      end
