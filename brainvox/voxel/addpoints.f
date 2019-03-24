C    ======================================================================     
C    THIS SOFTWARE IS THE PROPERTY OF:                                          
C         The Image Analysis Facility                                     
C         71 HBRF     
C         The University of Iowa, Iowa City, Iowa                               
C    ======================================================================     
C       Copyright (C) 1989 Image Analysis Facility, University of Iowa
C    ======================================================================     
C                                                                               
C    -- OVERVIEW --------------                                                 
C                                                                               
C    MODULE NAME    : addpoints
C    VERSION        : 1.0 
C    LOCATION       :  
C    LAST EDIT      :  
C    STATUS         :  (Development,Testing,Production,Obsolete)
C    LANGUAGE       : FORTRAN
C    PACKAGE        : brainvox_voxel
C
C
C    -- CREATION --------------                                                 
C                                                                               
C    PROGRAMMER     : rjf
C    DATE           : 20 Sep 89       
C    LINE-BY-LINER  :    
C    APPROVAL       : 
C
C    DESCRIPTION    :  	
C                                                                               
C    -- REVISION HISTORY --------------                                         
C
C                                                                               
C    -- GENERAL --------------                                                  
C                                                                               
C    SYNTAX         :     
C    INPUTS         :          
C    OUTPUTS        : 
C
C    LIMITATIONS    :      
C    NOTES          :      
C                 
C                                                                               
C    ====================================================================== 
C	Add a point if possible
C


	Subroutine  addpt(x,y,num,list,slice,icol,myset,wx,wy,retvals)

	Implicit None

	Include	'sgiuif.h'
	Include '../libs/vt_inc.f'
	Include 'windows.inc'

	Integer*4	x,y,num,list(4096,4),slice,icol,myset(*),wx,wy
C
	Integer*4	I,p2d(3),p3d(3),p(3),thres,retvals(4)
C
	Integer*4	fat,clippingon,tal_over,clipshade
C
        common  /fat_com/fat,clippingon,tal_over,clipshade
C
	thres = retvals(1)
C
	If (num .gt. 4094) then
		write(*, 10)
10	Format("Unable to add further points to list.")
		Return
	Endif
C
	retvals(1) = x
	retvals(2) = y
	retvals(3) = slice
	retvals(4) = icol
C
C	Choose alg
C	use arbitrary slicing 
C
	if ((slice .ne. -1).and.(wind2d(curwin).mode .ne. 0)) then
C
	p(1) = x
	p(2) = y
C DEBUG
C	write(*, *)"Sending:",x,y," to vl_cutplaneto3d"
	Call vl_cutplaneto3d(p(1),p2d(1),i)
C
	retvals(1) = p2d(1)
	retvals(2) = p2d(2)
	retvals(3) = p2d(3)
C DEBUG
C	write(*, *)"Output:",retvals(1),retvals(2),retvals(3)
C
C	check for error
C
	If (i .eq. 1) then
Cifdef DEBUG
C		write(*, *)"Error found from vl_cutplaneto3d"
		return
	Endif
C
C	project the point into the voxel image
C
	Call p2dtop3d(p2d,p,myset,wx,wy)
C
C	p(3) = -1
C
C	use 2d slice (orthogonal)
C
	Else if (slice .ne. -1) then
C
Cifdef DEBUG
C	write(*, *)"point:",x,y,slice," as orthoplane input"
C
C	check bounds on inputs
C
		if (x .ge. imagesize) return
		if (y .ge. imagesize) return
		if (x .lt. 0) return
		if (y .lt. 0) return
		if (slice .lt. 0) return
Cifdef DEBUG
C	write(*, *)"Output:",retvals(1),retvals(2),retvals(3)
C
C	convert 2d + Z into 3d
C
		p2d(1) = x 
		p2d(2) = y 
		p2d(3) = slice
		Call p2dtop3d(p2d,p,myset,wx,wy)
C
C		write(*,102) p(1),p(2),p(3)
102	Format("Ouptut: X=",I4," Y=",I4," Z=",I4)
C
C	this one always works...
C		
C		p(3) = -1   ! no Z value
	Else
C
C		write(*,101) x,y,slice
101	Format("3D into 2D+Z in: X=",I4," Y=",I4," slice=",I4)
C
C	convert 3d point into 2d + Z
C
C	Comes from retvals(1) and -999 == map to Z=0
C		thres = 5
C
		p3d(1) = x 
		p3d(2) = y 
		Call p3dtop2d(p3d,p,myset,wx,wy,thres)
		retvals(3) = p3d(3)  ! get the intersection
C
C	check for error
C
		if (p(1) .eq. -1) return
C
C	convert Z into slice number (not needed anymore, rjf)
C
C		type *,"P3 in = ",p(3)
C
C		type 103,p(1),p(2),p(3)
103	Format("Ouptut: X=",I4," Y=",I4," slice=",I4)
C
	Endif
C
C	add the point
C
	num = num + 1
	list(num,1) = p(1)
	list(num,2) = p(2)
	list(num,3) = p(3)
	list(num,4) = icol

	Return
	End

C
C	Routine to interpolate between two points in 3D
C	
	Subroutine interp_pts(num,pts,interp)

	Implicit None

	Integer*4	num,pts(4096,4),interp
	Integer*4	i,en(4),st(4)
	Real*4		d,f(3),inc(3)
C
C	Can we do anything?
C
	if (num .le. 1) return  ! enough points ?
	if (pts(num,4) .ne. pts(num-1,4)) return ! same color?
C
C	get endpoints 
C
	Do i=1,4
		en(i) = pts(num,i)
		st(i) = pts(num-1,i)
	Enddo
	d = sqrt(float(en(1)-st(1))**2 + float(en(2)-st(2))**2 + 
     +		float(en(3)-st(3))**2)
C
C	estimate the number of points to be inserted
C
	If (interp .eq. 0) Then
		interp = d - 1
	Endif
C
C	place interp points bwteen st and en
C
	d = interp + 1
	num = num - 1
	Do i=1,3
		inc(i) = float(en(i) - st(i))/d
		f(i) = pts(num,i)
	Enddo
	Do i=1,interp
		num = num + 1
		f(1) = f(1) + inc(1)
		f(2) = f(2) + inc(2)
		f(3) = f(3) + inc(3)
		pts(num,1) = f(1)
		pts(num,2) = f(2)
		pts(num,3) = f(3)
		pts(num,4) = pts(num-1,4)
		if (num .ge. 4094) goto 100
	Enddo
C
C	put the end back on
C
100	continue
	num = num + 1
	Do i=1,4
		pts(num,i) = en(i)
	Enddo
C
C	That's all
C
	Return
	End
C
C	Routine to sort points by color and place a given color on top
C
	Subroutine pointsort(pts1,num1,pts2,num2,color)

	Implicit None

	Integer*4	num1,pts1(4096,4),color,num2,pts2(4096,4)

	Integer*4	tmp1(4096,4),tmp2(4096,4),k,i,j

	if (num1 .ne. num2) return
C
C	sort (pack) all colors except COLOR first
C
	k = 0
	Do I=0,20  ! 13 + more
	    If (I .ne. color) then
		Do j = 1,num1
		    If (pts1(j,4) .eq. I) then
			k = k + 1
			tmp1(k,1) = pts1(j,1)
			tmp1(k,2) = pts1(j,2)
			tmp1(k,3) = pts1(j,3)
			tmp1(k,4) = pts1(j,4)
			tmp2(k,1) = pts2(j,1)
			tmp2(k,2) = pts2(j,2)
			tmp2(k,3) = pts2(j,3)
			tmp2(k,4) = pts2(j,4)
		    Endif
		Enddo
	    Endif
	Enddo
C
C	color goes on the table last
C
	Do j = 1,num1
	    If (pts1(j,4) .eq. color) then
		k = k + 1
		tmp1(k,1) = pts1(j,1)
		tmp1(k,2) = pts1(j,2)
		tmp1(k,3) = pts1(j,3)
		tmp1(k,4) = pts1(j,4)
		tmp2(k,1) = pts2(j,1)
		tmp2(k,2) = pts2(j,2)
		tmp2(k,3) = pts2(j,3)
		tmp2(k,4) = pts2(j,4)
	    Endif
	Enddo
C
C	Copy it back for output
C
	Do j=1,num1
		pts1(j,1) = tmp1(j,1)
		pts1(j,2) = tmp1(j,2)
		pts1(j,3) = tmp1(j,3)
		pts1(j,4) = tmp1(j,4)
		pts2(j,1) = tmp2(j,1)
		pts2(j,2) = tmp2(j,2)
		pts2(j,3) = tmp2(j,3)
		pts2(j,4) = tmp2(j,4)
	Enddo
C
	Return
	End
C
C	INCOMPLETE!!!  WORK IN PROGRESS...
C	Subroutine to bump a point in some direction according to
C	some view plane
C
	Subroutine	bump_point(num2d,w2dpts,num3d,w3dpts,dx,dy,
     +				wx,wy,myset,view)

	Implicit None

	Include	'sgiuif.h'
	Include '../libs/vt_inc.f'
	Include 'windows.inc'

	Integer*4	num2d,num3d,dx,dy,wx,wy,view
	Integer*4	w2dpts(4096,4),w3dpts(4096,4),myset(*)
	Integer*4	i,p2d(3),p3d(3)
C
	Integer*4	fat,clippingon,tal_over,clipshade
C
        common  /fat_com/fat,clippingon,tal_over,clipshade
C
C	is there anything to do?
C
	If ((num2d .eq. 0) .or. (num3d .eq. 0)) return
C
	If (view .eq. 3) then
C
C	motion in 3d projection window
C
	Else 
C
C	motion in 2d projection window
C
		If (wind2d(curwin).mode .eq. 0) Then
C
C	slice view
C
			p2d(1) = w2dpts(num2d,1) + dx
			p2d(2) = w2dpts(num2d,2) + dy
			p2d(3) = w2dpts(num2d,3)
			Call p2dtop3d(p2d,p3d,myset,wx,wy)
			Do i=1,3
				w2dpts(num2d,i) = p2d(i)
				w3dpts(num3d,i) = p3d(i)
			Enddo
		Else 
C
C	Arb or MP_FIT view
C
		Endif
	Endif

	Return
	End
