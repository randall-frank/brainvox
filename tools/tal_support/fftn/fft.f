C added some do { } while comments to help untangle the code
      
* ======================================================================
* NIST Guide to Available Math Software.
* Source for module FFT from package GO.
* Retrieved from NETLIB on Wed Jul  5 11:50:07 1995.
* ======================================================================
      subroutine fft(a,b,ntot,n,nspan,isn)
c  multivariate complex fourier transform, computed in place
c    using mixed-radix fast fourier transform algorithm.
c  by r. c. singleton, stanford research institute, sept. 1968
c  arrays a and b originally hold the real and imaginary
c    components of the data, and return the real and
c    imaginary components of the resulting fourier coefficients.
c  multivariate data is indexed according to the fortran
c    array element successor function, without limit
c    on the number of implied multiple subscripts.
c    the subroutine is called once for each variate.
c    the calls for a multivariate transform may be in any order.
c  ntot is the total number of complex data values.
c  n is the dimension of the current variable.
c  nspan/n is the spacing of consecutive data values
c    while indexing the current variable.
c  the sign of isn determines the sign of the complex
c    exponential, and the magnitude of isn is normally one.
c  a tri-variate transform with a(n1,n2,n3), b(n1,n2,n3)
c    is computed by
c      call fft(a,b,n1*n2*n3,n1,n1,1)
c      call fft(a,b,n1*n2*n3,n2,n1*n2,1)
c      call fft(a,b,n1*n2*n3,n3,n1*n2*n3,1)
c  for a single-variate transform,
c    ntot = n = nspan = (number of complex data values), e.g.
c      call fft(a,b,n,n,n,1)
c  the data can alternatively be stored in a single complex array c
c    in standard fortran fashion, i.e. alternating real and imaginary
c    parts. then with most fortran compilers, the complex array c can
c    be equivalenced to a real array a, the magnitude of isn changed
c    to two to give correct indexing increment, and a(1) and a(2) used
c    to pass the initial addresses for the sequences of real and
c    imaginary values, e.g.
c       complex c(ntot)
c       real    a(2*ntot)
c       equivalence (c(1),a(1))
c       call fft(a(1),a(2),ntot,n,nspan,2)
c  arrays at(maxf), ck(maxf), bt(maxf), sk(maxf), and np(maxp)
c    are used for temporary storage.  if the available storage
c    is insufficient, the program is terminated by a stop.
c    maxf must be .ge. the maximum prime factor of n.
c    maxp must be .gt. the number of prime factors of n.
c    in addition, if the square-free portion k of n has two or
c    more prime factors, then maxp must be .ge. k-1.
      dimension a(1),b(1)
c  array storage in nfac for a maximum of 15 prime factors of n.
c  if n has more than one square-free factor, the product of the
c    square-free factors must be .le. 210
      dimension nfac(11),np(209)
c  array storage for maximum prime factor of 23
      dimension at(23),ck(23),bt(23),sk(23)
      equivalence (i,ii)
c  the following two constants should agree with the array dimensions.
      maxp=209
      maxf=23
      if(n .lt. 2) return
      inc=isn
      c72=0.30901699437494742
      s72=0.95105651629515357
      s120=0.86602540378443865
      rad=6.2831853071796
      if(isn .lt. 0) then
        s72=-s72
        s120=-s120
        rad=-rad
        inc=-inc
      endif
 10   continue
      nt=inc*ntot
      ks=inc*nspan
      kspan=ks
      nn=nt-inc
      jc=ks/n
      radf=rad*float(jc)*0.5
      i=0
      jf=0
c  determine the factors of n
      m=0
      k=n
      
c      goto 20
c  15  continue
c      m=m+1
c      nfac(m)=4
c      k=k/16
c  20  if(k-(k/16)*16 .eq. 0) goto 15

C  while ((k - (k / 16) << 4) == 0) {
 20   if(k-(k/16)*16 .eq. 0) then
        m=m+1
        nfac(m)=4
        k=k/16
        goto 20
      endif
C }
            
      j=3
      jj=9

c      goto 30
c  25  continue
c      m=m+1
c      nfac(m)=j
c      k=k/jj
c  30  if(mod(k,jj) .eq. 0) goto 25      
      
 30   if(mod(k,jj) .eq. 0) then
C do {
C while (k % jj == 0) {
        m=m+1
        nfac(m)=j
        k=k/jj
        goto 30
      endif
C }      
      j=j+2
      jj=j*j       
      if(jj .le. k) goto 30
C } while (jj <= k);
      
c      if(k .gt. 4) goto 40
c      kt=m
c      nfac(m+1)=k
c      if(k .ne. 1) m=m+1
c      goto 80
c   40 continue
c      if(k-(k/4)*4 .ne. 0) goto 50
c      m=m+1
c      nfac(m)=2
c      k=k/4
c   50 continue

      if (k .le. 4) then
        kt=m
        nfac(m+1)=k
        if(k .ne. 1) m=m+1
      else     
        if(k-(k/4)*4 .eq. 0) then
          m=m+1
          nfac(m)=2
          k=k/4
        endif

        kt=m
        j=2
      
c   60 if(mod(k,j) .ne. 0) goto 70
c      m=m+1
c      nfac(m)=j
c      k=k/j
c   70 continue
        
C do {      
 60     if(mod(k,j) .eq. 0) then
          m=m+1
          nfac(m)=j
          k=k/j
        endif
        
        j=((j+1)/2)*2+1
        if(j .le. k) goto 60
C } while (j <= k);
      endif
      
 80   continue
C      if(kt .eq. 0) goto 100
      if(kt .ne. 0) then
        j=kt
c do {
 90     m=m+1
        nfac(m)=nfac(j)
        j=j-1
        if(j .ne. 0) goto 90
c } while (j);
      endif
c  compute fourier transform
 100  continue
      sd=radf/float(kspan)
      cd=sin(sd)
      cd=2.0*cd*cd
      sd=sin(sd+sd)
      kk=1
      i=i+1
      if(nfac(i) .ne. 2) goto 400
c  transform for factor of 2 (including rotation factor)
      kspan=kspan/2
      k1=kspan+2

C do {
C do {
 210  k2=kk+kspan
      ak=a(k2)
      bk=b(k2)
      a(k2)=a(kk)-ak
      b(k2)=b(kk)-bk
      a(kk)=a(kk)+ak
      b(kk)=b(kk)+bk
      kk=k2+kspan
      if(kk .le. nn) goto 210
C } while (kk <= nn);
      kk=kk-nn
      if(kk .le. jc) goto 210
C } while (kk <= jc);
      if(kk .gt. kspan) goto 800
      
C do {
 220  c1=1.0-cd
      s1=sd
C do {
C do {
C do {
 230  k2=kk+kspan
      ak=a(kk)-a(k2)
      bk=b(kk)-b(k2)
      a(kk)=a(kk)+a(k2)
      b(kk)=b(kk)+b(k2)
      a(k2)=c1*ak-s1*bk
      b(k2)=s1*ak+c1*bk
      kk=k2+kspan
      if(kk .lt. nt) goto 230
C } while (kk < nt);
      k2=kk-nt
      c1=-c1
      kk=k1-k2
      if(kk .gt. k2) goto 230
C } while (kk > k2);
      ak=c1-(cd*c1+sd*s1)
      s1=(sd*c1-cd*s1)+s1
      c1=2.0-(ak*ak+s1*s1)
      s1=c1*s1
      c1=c1*ak
      kk=kk+jc
      if(kk .lt. k2) goto 230
C } while (kk < k2);
      k1=k1+inc+inc
      kk=(k1-kspan)/2+jc
      if(kk .le. jc+jc) goto 220
C } while (kk <= jc+jc);
      goto 100

      
c  transform for factor of 3 (optional code)
C do {      
C do {      
 320  continue
      k1=kk+kspan
      k2=k1+kspan
      ak=a(kk)
      bk=b(kk)
      aj=a(k1)+a(k2)
      bj=b(k1)+b(k2)
      a(kk)=ak+aj
      b(kk)=bk+bj
      ak=ak-0.5*aj
      bk=bk-0.5*bj
      aj=(a(k1)-a(k2))*s120
      bj=(b(k1)-b(k2))*s120
      a(k1)=ak-bj
      b(k1)=bk+aj
      a(k2)=ak+bj
      b(k2)=bk-aj
      kk=k2+kspan
      if(kk .lt. nn) goto 320
C } while (kk < nn);
      kk=kk-nn
      if(kk .le. kspan) goto 320
C } while (kk <= kspan);
      goto 700

      
c  transform for factor of 4
 400  continue
      if(nfac(i) .ne. 4) goto 600
      kspnn=kspan
      kspan=kspan/4
 410  continue
      c1=1.0
      s1=0
 420  continue
      k1=kk+kspan
      k2=k1+kspan
      k3=k2+kspan
      akp=a(kk)+a(k2)
      akm=a(kk)-a(k2)
      ajp=a(k1)+a(k3)
      ajm=a(k1)-a(k3)
      a(kk)=akp+ajp
      ajp=akp-ajp
      bkp=b(kk)+b(k2)
      bkm=b(kk)-b(k2)
      bjp=b(k1)+b(k3)
      bjm=b(k1)-b(k3)
      b(kk)=bkp+bjp
      bjp=bkp-bjp
      if(isn .lt. 0) then
        goto 450
      else        
        akp=akm-bjm
        akm=akm+bjm
        bkp=bkm+ajm
        bkm=bkm-ajm
      endif
      if(s1 .eq. 0) goto 460
 430  continue
      a(k1)=akp*c1-bkp*s1
      b(k1)=akp*s1+bkp*c1
      a(k2)=ajp*c2-bjp*s2
      b(k2)=ajp*s2+bjp*c2
      a(k3)=akm*c3-bkm*s3
      b(k3)=akm*s3+bkm*c3
      kk=k3+kspan
      if(kk .le. nt) goto 420
 440  continue
      c2=c1-(cd*c1+sd*s1)
      s1=(sd*c1-cd*s1)+s1
      c1=2.0-(c2*c2+s1*s1)
      s1=c1*s1
      c1=c1*c2
      c2=c1*c1-s1*s1
      s2=2.0*c1*s1
      c3=c2*c1-s2*s1
      s3=c2*s1+s2*c1
      kk=kk-nt+jc
      if(kk .le. kspan) goto 420
      kk=kk-kspan+inc
      if(kk .le. jc) goto 410
      if(kspan .eq. jc) goto 800
      goto 100
      
 450  continue
      akp=akm+bjm
      akm=akm-bjm
      bkp=bkm-ajm
      bkm=bkm+ajm
      if(s1 .ne. 0) goto 430
 460  continue
      a(k1)=akp
      b(k1)=bkp
      a(k2)=ajp
      b(k2)=bjp
      a(k3)=akm
      b(k3)=bkm
      kk=k3+kspan
      if(kk .le. nt) goto 420
      goto 440
      
c  transform for factor of 5 (optional code)
 510  continue
      c2=c72*c72-s72*s72
      s2=2.0*c72*s72
C do {
 520  k1=kk+kspan
      k2=k1+kspan
      k3=k2+kspan
      k4=k3+kspan
      akp=a(k1)+a(k4)
      akm=a(k1)-a(k4)
      bkp=b(k1)+b(k4)
      bkm=b(k1)-b(k4)
      ajp=a(k2)+a(k3)
      ajm=a(k2)-a(k3)
      bjp=b(k2)+b(k3)
      bjm=b(k2)-b(k3)
      aa=a(kk)
      bb=b(kk)
      a(kk)=aa+akp+ajp
      b(kk)=bb+bkp+bjp
      ak=akp*c72+ajp*c2+aa
      bk=bkp*c72+bjp*c2+bb
      aj=akm*s72+ajm*s2
      bj=bkm*s72+bjm*s2
      a(k1)=ak-bj
      a(k4)=ak+bj
      b(k1)=bk+aj
      b(k4)=bk-aj
      ak=akp*c2+ajp*c72+aa
      bk=bkp*c2+bjp*c72+bb
      aj=akm*s2-ajm*s72
      bj=bkm*s2-bjm*s72
      a(k2)=ak-bj
      a(k3)=ak+bj
      b(k2)=bk+aj
      b(k3)=bk-aj
      kk=k4+kspan
      if(kk .lt. nn) goto 520
C } while (kk < nn);
      kk=kk-nn
      if(kk .le. kspan) goto 520
C } while (kk <= kspan);
      goto 700
      
c  transform for odd factors
 600  continue
      k=nfac(i)
      kspnn=kspan
      kspan=kspan/k
      if(k .eq. 3) then
        goto 320
      endif
      if(k .eq. 5) then
        goto 510
      endif
      
      if(k .eq. jf) goto 640
      jf=k
      s1=rad/float(k)
      c1=cos(s1)
      s1=sin(s1)
      if(jf .gt. maxf) goto 998
      ck(jf)=1.0
      sk(jf)=0.0
      j=1
C do {
 630  ck(j)=ck(k)*c1+sk(k)*s1
      sk(j)=ck(k)*s1-sk(k)*c1
      k=k-1
      ck(k)=ck(j)
      sk(k)=-sk(j)
      j=j+1
      if(j .lt. k) goto 630
C } while (j < k);
C do {
C do {
 640  continue
      k1=kk
      k2=kk+kspnn
      aa=a(kk)
      bb=b(kk)
      ak=aa
      bk=bb
      j=1
      k1=k1+kspan
C do {
 650  k2=k2-kspan
      j=j+1
      at(j)=a(k1)+a(k2)
      ak=at(j)+ak
      bt(j)=b(k1)+b(k2)
      bk=bt(j)+bk
      j=j+1
      at(j)=a(k1)-a(k2)
      bt(j)=b(k1)-b(k2)
      k1=k1+kspan
      if(k1 .lt. k2) goto 650
C } while (k1 < k2);
      a(kk)=ak
      b(kk)=bk
      k1=kk
      k2=kk+kspnn
      j=1
C do {
 660  k1=k1+kspan
      k2=k2-kspan
      jj=j
      ak=aa
      bk=bb
      aj=0.0
      bj=0.0
      k=1
C do {
 670  k=k+1
      ak=at(k)*ck(jj)+ak
      bk=bt(k)*ck(jj)+bk
      k=k+1
      aj=at(k)*sk(jj)+aj
      bj=bt(k)*sk(jj)+bj
      jj=jj+j
      if(jj .gt. jf) jj=jj-jf
      if(k .lt. jf) goto 670
C } while (k < jf);
      k=jf-j
      a(k1)=ak-bj
      b(k1)=bk+aj
      a(k2)=ak+bj
      b(k2)=bk-aj
      j=j+1
      if(j .lt. k) goto 660
C } while (j < k);
      kk=kk+kspnn
      if(kk .le. nn) goto 640
C } while (kk <= nn);
      kk=kk-nn
      if(kk .le. kspan) goto 640
C } while (kk <= kspan);
c  multiply by rotation factor (except for factors of 2 and 4)
 700  continue
      if(i .eq. m) goto 800
      kk=jc+1
C do {
 710  c2=1.0-cd
      s1=sd
C do {
 720  c1=c2
      s2=s1
      kk=kk+kspan
C do {
C do {
 730  ak=a(kk)
      a(kk)=c2*ak-s2*b(kk)
      b(kk)=s2*ak+c2*b(kk)
      kk=kk+kspnn
      if(kk .le. nt) goto 730
C } while (kk <= nt);
      ak=s1*s2
      s2=s1*c2+c1*s2
      c2=c1*c2-ak
      kk=kk-nt+kspan
      if(kk .le. kspnn) goto 730
C } while (kk <= kspnn);
      c2=c1-(cd*c1+sd*s1)
      s1=s1+(sd*c1-cd*s1)
      c1=2.0-(c2*c2+s1*s1)
      s1=c1*s1
      c2=c1*c2
      kk=kk-kspnn+jc
      if(kk .le. kspan) goto 720
C } while (kk <= kspan);
      kk=kk-kspan+jc+inc
      if(kk .le. jc+jc) goto 710
C } while (kk <= jc + jc);
      goto 100
c  permute the results to normal order---done in two stages
c  permutation for square factors of n
 800  continue
      np(1)=ks
      if(kt .eq. 0) goto 890
      k=kt+kt+1
      if(m .lt. k) k=k-1
      j=1
      np(k+1)=jc
C do {
 810  np(j+1)=np(j)/nfac(j)
      np(k)=np(k+1)*nfac(j)
      j=j+1
      k=k-1
      if(j .lt. k) goto 810
C } while (j < k);
      k3=np(k+1)
      kspan=np(2)
      kk=jc+1
      k2=kspan+1
      j=1
      if(n .ne. ntot) goto 850
c  permutation for single-variate transform (optional code)
 820  continue
C do {
 825  ak=a(kk)
      a(kk)=a(k2)
      a(k2)=ak
      bk=b(kk)
      b(kk)=b(k2)
      b(k2)=bk
      kk=kk+inc
      k2=k2+kspan
      if(k2 .lt. ks) goto 825
C } while (k2 < ks);
 830  continue
C do {
C do {
 835  continue
      k2=k2-np(j)
      j=j+1
      k2=k2+np(j+1)
      if(k2 .gt. np(j)) goto 835
C } while (k2 > np[j - 1]);
      j=1
C do {
 840  if(kk .lt. k2) goto 820
      kk=kk+inc
      k2=k2+kspan
      if(k2 .lt. ks) goto 840
C } while (k2 < ks);
      if(kk .lt. ks) goto 835
C } while (kk < ks);
      jc=k3
      goto 890
      
c  permutation for multivariate transform
 850  continue
C do {
C do {
      k=kk+jc
C do {
 860  ak=a(kk)
      a(kk)=a(k2)
      a(k2)=ak
      bk=b(kk)
      b(kk)=b(k2)
      b(k2)=bk
      kk=kk+inc
      k2=k2+inc
      if(kk .lt. k) goto 860
C } while (kk < k);
      kk=kk+ks-jc
      k2=k2+ks-jc
      if(kk .lt. nt) goto 850
C } while (kk < nt);
      k2=k2-nt+kspan
      kk=kk-nt+jc
      if(k2 .lt. ks) goto 850
C } while (k2 < ks);
C do {
 870  continue
C do {
 875  continue
      k2=k2-np(j)
      j=j+1
      k2=k2np(j+1)
      if(k2 .gt. np(j)) goto 875
C } while (k2 > np[j - 1]);
      j=1
C do {
 880  continue
      if(kk .lt. k2) goto 850
      kk=kk+jc
      k2=k2+kspan
      if(k2 .lt. ks) goto 880
C } while (k2 < ks);
      if(kk .lt. ks) goto 870
C } while (kk < ks);
      jc=k3
 890  continue
      if(2*kt+1 .ge. m) return
      kspnn=np(kt+1)
c  permutation for square-free factors of n
      j=m-kt
      nfac(j+1)=1
C do {
 900  nfac(j)=nfac(j)*nfac(j+1)
      j=j-1
      if(j .ne. kt) goto 900
C } while (j != kt);
      kt=kt+1
      nn=nfac(kt)-1
      if(nn .gt. maxp) goto 998
      jj=0
      j=0
      goto 906
      
 904  continue
      jj=jj+kk
      if(jj .ge. k2) then
        jj=jj-k2
        k2=kk
        k=k+1
        kk=nfac(k)
      else
        goto 903
      endif
 903  continue
      np(j)=jj
      goto 906
      
 906  continue
      k2=nfac(kt)
      k=kt+1
      kk=nfac(k)
      j=j+1
      if(j .le. nn) goto 904
c  determine the permutation cycles of length greater than 1
      j=0
      goto 914
      
 910  continue
C do {
      k=kk
      kk=np(k)
      np(k)=-kk
      if(kk .ne. j) goto 910
C } while (kk != j);
      k3=kk
 914  continue
C do {
      j=j+1
      kk=np(j)
      if(kk .lt. 0) goto 914
C } while (kk < 0);
      if(kk .ne. j) goto 910
      np(j)=-j
      if(j .ne. nn) goto 914
      maxf=inc*maxf
c  reorder a and b, following the permutation cycles
      goto 920

C for (;;) {
 920 continue      
      j=k3+1
      nt=nt-kspnn
      ii=nt-inc+1
      if(nt .lt. 0) return
      
C do {
C do {
 924  j=j-1
      if(np(j) .lt. 0) goto 924
C } while (np[j - 1] < 0);
      jj=jc
C do {
 926  kspan=jj
      if(jj .gt. maxf) kspan=maxf
      jj=jj-kspan
      k=np(j)
      kk=jc*k+ii+jj
      k1=kk+kspan
      k2=0
C do {
 928  k2=k2+1
      at(k2)=a(k1)
      bt(k2)=b(k1)
      k1=k1-inc
      if(k1 .ne. kk) goto 928
C } while (k1 != kk);
C do {
 932  k1=kk+kspan
      k2=k1-jc*(k+np(k))
      k=-np(k)
C do {
 936  a(k1)=a(k2)
      b(k1)=b(k2)
      k1=k1-inc
      k2=k2-inc
      if(k1 .ne. kk) goto 936
C } while (k1 != kk);
      kk=k2
      if(k .ne. j) goto 932
C } while (k != j);
      k1=kk+kspan
      k2=0
C do {
 940  k2=k2+1
      a(k1)=at(k2)
      b(k1)=bt(k2)
      k1=k1-inc
      if(k1 .ne. kk) goto 940
C } while (k1 != kk);
      if(jj .ne. 0) goto 926
C } while (jj);
      if(j .ne. 1) goto 924
C } while (j != 1);
      goto 920
C }
      
c  error finish, insufficient array storage
 998  continue
      isn=0
      print 999
      stop
 999  format('array bounds exceeded within subroutine fft')
      end

