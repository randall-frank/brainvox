      program test3
c
c     Test vmove for treatment of singularities.
c
      real rand
      double precision d(7), rbar(21), thetab(7), x(7), y 
      double precision sserr, work(7),
     +       tol(7), rss(7), beta(7)
      integer ix, iy, iz, i, ier, vorder(7)
      logical lindep(7)
      common /randc/ ix, iy, iz

      ix = 777
      iy = 777
      iz = 777

c     Initialize orthogonal reduction.
      call clear(7, 21, d, rbar, thetab, sserr, ier)
      do 10 i = 1, 7
	vorder(i) = i
   10 continue

c     Generate data such that:
c             x2 = x4 - x5
c             x6 = x1 - x3
c             y  = x1 + x3 + x4 + x5 + x7

      do 30 i = 1, 12
	x(1) = rand()
	x(3) = rand()
	x(4) = rand()
	x(5) = rand()
	x(7) = rand()
	x(2) = x(4) - x(5)
	x(6) = x(1) - x(3)
	y = x(1) + x(3) + x(4) + x(5) + x(7)
	call includ(7, 21, 1.0d0, x, y, d, rbar, thetab, sserr, ier)
   30 continue
      write(*, *)'As output from includ:'
      write(*, 900) d, rbar, thetab
  900 format(' d:'/1x,7f11.6/
     +       ' rbar:'/1x,6f11.6/ 12x,5f11.6/ 23x,4f11.6/ 34x,3f11.6/
     +             45x,2f11.6/ 56x,f11.6/
     +       ' thetab:'/1x,7f11.6//)
c
c     Set up arrays TOL & RSS.
c
      call tolset(7, 21, d, rbar, tol, work, ier)
      call ss(7, d, thetab, sserr, rss, ier)
      write(*, 920) rss
  920 format(' RSS:'/ 1x, 7f11.5/)
c
c     Use SING to set near zeroes to zero.
c
      call sing(7, 21, d, rbar, thetab, sserr, tol, lindep, work, ier)
      write(*, *)'After being processed by sing:'
      write(*, *)'IER = ', ier
      write(*, 900) d, rbar, thetab
      call ss(7, d, thetab, sserr, rss, ier)
      write(*, 920) rss
c
c       Swap rows 4 & 5 and rows 6 & 7.
c
      call vmove(7, 21, vorder, d, rbar, thetab, rss, 4, 5, tol, ier)
      call vmove(7, 21, vorder, d, rbar, thetab, rss, 6, 7, tol, ier)
      write(*, *)'After interchange of variables 4 & 5, and 6 & 7'
      write(*, 900) d, rbar, thetab
      call regcf(7, 21, d, rbar, thetab, tol, beta, 7, ier)
      write(*, 910) beta
  910 format(' Regression coefficients:'/ 1x,7f11.6)
      call ss(7, d, thetab, sserr, rss, ier)
      write(*, 920) rss
      end
