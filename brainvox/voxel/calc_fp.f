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
C    MODULE NAME    : calc_fp.f
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
C    DATE           : 7 Aug 91        
C    LINE-BY-LINER  :    
C    APPROVAL       : 
C
C    DESCRIPTION    :  	routines to compute the centroid and normal to
C			the current fixed PET plane
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

	Subroutine glasses_io(what,rots)

	Implicit None

	Include 'rotsdef.inc'

        Record  /rotsrec/       rots
	Integer*4		what,I

	Logical			found
	Character*200		str
C
C       case of read (cmd=0) or write (cmd=1)
C
	If (what .eq. 0) Then
C
C	Read
C
		Inquire(exist=found,file="_glasses")
		If (found) Then
			Open(unit=66,file="_glasses",form="formatted",
     +				status="old",err=990) 
			Read(66,2,err=991,end=991) str
2	Format(A200)
			Do I=1,4  !RF,LF,RR,LR
			Read(66,*,err=991,end=991) rots.fppoints(I,1),
     +			rots.fppoints(I,2),rots.fppoints(I,3)
			Enddo
			Close(66)
			write(*, 201)
201	Format("_glasses file read.")
			Return

991			write(*, 102)
102	Format("Error:Unable to read glasses calibration file")
			Close(66)
			Return
990			write(*, 101)
101	Format("Error:Unable to open glasses calibration file")
			Return

		Else
			write(*, 100)
100	Format("Error:No saved glasses calibration exists")
			Return
		Endif

	Else if (what .eq. 1) Then
C
C	Write
C
		Inquire(exist=found,file="_glasses")
		If (found) Then
			Open(unit=66,file="_glasses",form="formatted",
     +				status="old",err=995) 
		Else
			Open(unit=66,file="_glasses",form="formatted",
     +				status="new",err=995) 
		Endif
		write(66,1,err=996) "1.0 Glasses fit:RF,LF,RR,LR"
1	Format(A)
		Do I=1,4  !RF,LF,RR,LR
			Write(66,120,err=996) rots.fppoints(I,1),
     +			rots.fppoints(I,2),rots.fppoints(I,3)
120	Format(F10.3,F10.3,F10.3)
		Enddo
		Close(66,err=994)
		write(*, 200)
200	Format("_glasses file saved.")
		Return
C
995	write(*, 997)
997	Format("Error:Unable to open glasses calibration file")
	Return
996	Close(66,err=994)
994	write(*, 998)
998	Format("Error:Unable to write glasses calibration file")
	Return
C
	Else 
C
C	Error case
C
		write(*, 110)what
110	Format("Warning: Unknown glasses_io operation",I8)

	Endif

	Return
	End
C
C	Routine to compute the rots.fppoints array from current traces
C
	Subroutine	trace2fppoints(num,point,rots)

	Implicit None

	Include 'rotsdef.inc'
	Include '../libs/vt_inc.f'

        Record  /rotsrec/       rots
	Integer*4	num,point(4096,4)

	Real*4		numimg
	Integer*4	I,j,col
	Real*4	x(4096),y(4096),z(4096),err

	Real*4	rt_pnt(3),rt_norm(3)
	Real*4	re_pnt(3),re_norm(3)
	Real*4	lt_pnt(3),lt_norm(3)
	Real*4	le_pnt(3),le_norm(3)
	Real*4	out(3),p1(3),p2(3)

	common /junkfit/rt_pnt,rt_norm,re_pnt,re_norm,
     +			lt_pnt,lt_norm,le_pnt,le_norm
C
C       Calc number of images
C
        numimg = realimages+((realimages - 1) * interpimages)
C
C	Copy Colored points into array in the following order:
C
C	Right temple
C	Right ear
C	Left temple
C	Left ear
C
C	First the right side
C
10	Format("Unable to fit glasses.  Not enought points for ",A,".")
	i = 1
	col = point(i,4)
	j = 0
	Do while ((i .le. num) .and. (col .eq. point(i,4)))
		j = j + 1
		x(j) = float(point(i,1))-(imagesize/2)
		y(j) = float(point(i,2))-(imagesize/2)
		z(j) = (float(point(i,3))-(numimg/2))*squeezefactor
		i = i + 1
	Enddo
	If (j .lt. 2) Then
		write(*, 10)"right temple"
		Return
	Endif
	Call lin_fit3d(x,y,z,j,rt_pnt,rt_norm,err)
	write(*, 20)"right temple",err
20	Format("Average ",A," line fit error:",F8.3)

Cifdef DEBUG
c	type *,"RT:",j,err,rt_pnt(1),rt_pnt(2),rt_pnt(3),rt_norm(1),
c     +		rt_norm(2),rt_norm(3)A

	col = point(i,4)
	j = 0
	Do while ((i .le. num) .and. (col .eq. point(i,4)))
		j = j + 1
		x(j) = float(point(i,1))-(imagesize/2)
		y(j) = float(point(i,2))-(imagesize/2)
		z(j) = (float(point(i,3))-(numimg/2))*squeezefactor
		i = i + 1
	Enddo
	If (j .lt. 2) Then
		write(*, 10)"right ear"
		Return
	Endif
	Call lin_fit3d(x,y,z,j,re_pnt,re_norm,err)
	write(*, 20)"right ear",err
Cifdef DEBUG
C	type *,"RE:",j,err,re_pnt(1),re_pnt(2),re_pnt(3),re_norm(1),
C     +		re_norm(2),re_norm(3)
C
C	Now the left side
C
	col = point(i,4)
	j = 0
	Do while ((i .le. num) .and. (col .eq. point(i,4)))
		j = j + 1
		x(j) = float(point(i,1))-(imagesize/2)
		y(j) = float(point(i,2))-(imagesize/2)
		z(j) = (float(point(i,3))-(numimg/2))*squeezefactor
		i = i + 1
	Enddo
	If (j .lt. 2) Then
		write(*, 10)"left temple"
		Return
	Endif
	Call lin_fit3d(x,y,z,j,lt_pnt,lt_norm,err)
	write(*, 20)"left temple",err
Cifdef DEBUG
c	type *,"LT:",j,err,lt_pnt(1),lt_pnt(2),lt_pnt(3),lt_norm(1),
c     +		lt_norm(2),lt_norm(3)

	col = point(i,4)
	j = 0
	Do while ((i .le. num) .and. (col .eq. point(i,4)))
		j = j + 1
		x(j) = float(point(i,1))-(imagesize/2)
		y(j) = float(point(i,2))-(imagesize/2)
		z(j) = (float(point(i,3))-(numimg/2))*squeezefactor
		i = i + 1
	Enddo
	If (j .lt. 2) Then
		write(*, 10)"left ear"
		Return
	Endif
	Call lin_fit3d(x,y,z,j,le_pnt,le_norm,err)
	write(*, 20)"left ear",err
Cifdef DEBUG
C	type *,"LE:",j,err,le_pnt(1),le_pnt(2),le_pnt(3),le_norm(1),
C     +		le_norm(2),le_norm(3)
C
C	Now, compute the rear corner points (projections)
C
	Do i=1,3
		p1(i) = re_pnt(i) - rt_pnt(i) ! from RTp to REp
	Enddo
	Call dotprod(p1,rt_norm,p2)
	Do i=1,3
		out(i) = rt_pnt(i) + rt_norm(i)*p2(1)
	Enddo
	Do i=1,3
		rots.fppoints(3,i) = out(i)
		re_norm(i) = re_pnt(i) - out(i)
	Enddo
	Call	normalize(re_norm)
Cifdef DEBUG
C	type *,"RR:",err,out(1),out(2),out(3)

	Do i=1,3
		p1(i) = le_pnt(i) - lt_pnt(i) ! from LTp to LEp
	Enddo
	Call dotprod(p1,lt_norm,p2)
	Do i=1,3
		out(i) = lt_pnt(i) + lt_norm(i)*p2(1)
	Enddo
	Do i=1,3
		rots.fppoints(4,i) = out(i)
		le_norm(i) = le_pnt(i) - out(i)
	Enddo
	Call	normalize(le_norm)
Cifdef DEBUG
C	type *,"LR:",err,out(1),out(2),out(3)
C
C	Compute the RF,LF points by walking along a line from the rear
C	points toward the line point
C	a line from the rear point through the temple point
C
	Do i=1,3
		p1(i) = rt_pnt(i) - rots.fppoints(3,i)
		p2(i) = lt_pnt(i) - rots.fppoints(4,i)
	Enddo
	Call Normalize(p1)
	Call Normalize(p2)
	Do i=1,3
		rots.fppoints(1,i) = rots.fppoints(3,i) + (23.5*p1(i))
		rots.fppoints(2,i) = rots.fppoints(4,i) + (23.5*p2(i))
	Enddo
C
C	That's it!!!
C
	Return
	End
C
	Subroutine calc_fp(rots)

	Implicit None

	Include 'rotsdef.inc'

        Record  /rotsrec/       rots

	Integer*4	I,J
C
C	Take the average of REAR two points as the center point
C
	Do I=1,3
		rots.fpcenter(I) = 0.0 
		Do J=3,4
			rots.fpcenter(I) = rots.fpcenter(I) 
     +				+ rots.fppoints(J,I)
		Enddo
		rots.fpcenter(I) = rots.fpcenter(I) / 2.0
	Enddo
C
C	Compute the Normal to the 4 triangles which can be generated
C	from the four points
C
	Do I=1,3
		rots.fpnorm(I) = 0.0
	Enddo
C
C	Get triangle normals (sum into fpnorm)
C
	Call trinorm(rots.fpnorm,rots.fppoints,1,2,4)
	Call trinorm(rots.fpnorm,rots.fppoints,2,4,3)
	Call trinorm(rots.fpnorm,rots.fppoints,4,3,1)
	Call trinorm(rots.fpnorm,rots.fppoints,3,1,2)
C
C	Normalize Z vector
C
	Call normalize(rots.fpnorm)
C
C	Get X vector by mean of LR-RR and LF-RF (4,3) (2,1)
C
	Do I=1,3
		rots.fpxaxis(i) = ((rots.fppoints(4,i) - rots.fppoints(3,i))
     +			+ (rots.fppoints(2,i) - rots.fppoints(1,i)))/2.0
	Enddo
	Call normalize(rots.fpxaxis)
C
C	Y vector is Z cross X
C
	Call Xprod(rots.fpnorm,rots.fpxaxis,rots.fpyaxis)
C
Cifdef DEBUG
Cwrite(*, *)"Cent:",rots.fpcenter(1),rots.fpcenter(2),rots.fpcenter(3)
Cwrite(*, *)"Norm:",rots.fpnorm(1),rots.fpnorm(2),rots.fpnorm(3)
Cwrite(*, *)"X:",rots.fpxaxis(1),rots.fpxaxis(2),rots.fpxaxis(3)
Cwrite(*, *)"Y:",rots.fpyaxis(1),rots.fpyaxis(2),rots.fpyaxis(3)
C
	Return
	End
C
C	Routine to do dot products
C
	Subroutine Dotprod(v1,v2,out)

	Implicit None

	Real*4	v1(3),v2(3),out

	out = (v1(1)*v2(1)) + (v1(2)*v2(2)) + (v1(3)*v2(3))

	Return
	End
C
C	Routine to do cross products
C
	Subroutine Xprod(v1,v2,out)

	Implicit None

	Real*4	v1(3),v2(3),out(3)

	out(1) = (v1(2)*v2(3)) - (v1(3)*v2(2))
	out(2) = (v1(3)*v2(1)) - (v1(1)*v2(3))
	out(3) = (v1(1)*v2(2)) - (v1(2)*v2(1))

	Return
	End
C
C	Routine to normalize a vector
C
	Subroutine normalize(vect)

	Implicit None

	real*4		vect(3),D1
	Integer*4	I

	D1 = sqrt(vect(1)**2.0 + vect(2)**2.0 + vect(3)**2.0)
        If (D1 .eq. 0.0) then
		write(*,*) "Unable to normalize zero length vector"
                Return
        Endif
        Do I=1,3
                vect(I) = vect(I)/D1
        Enddo  

	Return
	End

	Subroutine trinorm(norm,points,p1,p2,p3)

	Implicit None

	Integer*4	p1,p2,p3,I
	Real*4		norm(3),out(3)
	Real*4		points(4,3),v1(3),v2(3)
C
C	Create vectors from p2 to p1 and from p2 to p3
C
	Do I=1,3
		v1(i) = points(p1,i) - points(p2,I)
		v2(i) = points(p3,i) - points(p2,I)
	Enddo
C
C	Compute v1 X v2 and add to norm
C
	Call Xprod(v1,v2,out)
	Do i=1,3
		norm(i) = norm(i) + out(i)
	Enddo
C
	Return
	End
C
C	Routine to rotate a point over an axis (about the origin)
C
C	AXIS is a unit vector about which to rotate
C	ANGLE is the amount of rotation in degrees
C	INPT is the input point in 3d
C	OUTPT is the output point in 3d
C	INPT and OUTPT may be the same array
C
	Subroutine Rotaxis(axis,angle,inpt,outpt)

	Implicit None

	Real*4		axis(3),angle
	Real*4		inpt(3),outpt(3)
	Real*4		mat(3,3),s,c,t,tmp(3)
	Integer*4	I
C
C	Build rotation matrix from angle and axis
C	(See Graphics Gems, Glassner Page 446)
C
	s = sind(angle)
	c = cosd(angle)
	t = 1.0 - cosd(angle)
C
	mat(1,1) = t*axis(1)*axis(1) + c
	mat(2,1) = t*axis(1)*axis(2) + s*axis(3)
	mat(3,1) = t*axis(1)*axis(3) - s*axis(2)
	mat(1,2) = t*axis(1)*axis(2) - s*axis(3)
	mat(2,2) = t*axis(2)*axis(2) + c
	mat(3,2) = t*axis(2)*axis(3) + s*axis(1)
	mat(1,3) = t*axis(1)*axis(3) + s*axis(2)
	mat(2,3) = t*axis(2)*axis(3) - s*axis(1)
	mat(3,3) = t*axis(3)*axis(3) + c
C
C	Multiply the point by the matrix
C
	Do i=1,3
		tmp(i) = mat(1,i)*inpt(1) + mat(2,i)*inpt(2) + 
     +			mat(3,i)*inpt(3)
	Enddo
C
C	Copy the result to the output array (this allows the input and
C	output arrays to be the same)
C
	Do i=1,3
		outpt(i) = tmp(i)
	Enddo
C
	Return
	End
C
C	Subroutine to compute the best fit line through a set of 3D points
C
	Subroutine	lin_fit3d(x,y,z,n,p,norm,err)

	Implicit None
	
	Real*4		x(*),y(*),z(*),p(3),norm(3),err
	Integer*4	n

	Integer*4	i
	Real*4		mx(3),my(3),mz(3),a,m,chi,tmp(3),out(3)
	Real*4		weight(3)
C
	err = -1
C
C	compute the average point
C
	p(1) = 0
	p(2) = 0
	p(3) = 0
	Do i=1,n
		p(1) = p(1) + x(i)
		p(2) = p(2) + y(i)
		p(3) = p(3) + z(i)
	Enddo
	p(1) = p(1)/float(n)
	p(2) = p(2)/float(n)
	p(3) = p(3)/float(n)
C
C	Compute y = xm + a
C
	Call lin_fit2d(x,y,n,a,m,chi)
	norm(1) = 1
	norm(2) = m
	norm(3) = 0
	weight(1) = chi
Cifdef DEBUG
C	type *,"y = ",m,"*x + ",a," chi = ",chi
C
C	Compute z = xm + a
C
	Call lin_fit2d(x,z,n,a,m,chi)
	norm(3) = m
	Call normalize(norm)
	mx(1) = norm(1)
	my(1) = norm(2)
	mz(1) = norm(3)
	weight(1) = weight(1) + chi
Cifdef DEBUG
Ctype *,"z = ",m,"*x + ",a," chi = ",chi
Ctype *,"mx = ",norm(1),"my = ",norm(2)," mz = ",norm(3),
C    +		" 1/w=",1.0/weight(1)
C
C	Compute x = ym + a
C
	Call lin_fit2d(y,x,n,a,m,chi)
	norm(1) = m
	norm(2) = 1
	norm(3) = 0
	weight(2) = chi
Cifdef DEBUG
Ctype *,"x = ",m,"*y + ",a," chi = ",chi
C
C	Compute z = ym + a
C
	Call lin_fit2d(y,z,n,a,m,chi)
	norm(3) = m
	Call normalize(norm)
	mx(2) = norm(1)
	my(2) = norm(2)
	mz(2) = norm(3)
	weight(2) = weight(2) + chi
Cifdef DEBUG
Ctype *,"z = ",m,"*y + ",a," chi = ",chi
Ctype *,"mx = ",norm(1),"my = ",norm(2)," mz = ",norm(3),
C    +		" 1/w=",1.0/weight(2)
C
C	Compute x = zm + a
C
	Call lin_fit2d(z,x,n,a,m,chi)
	norm(1) = m
	norm(2) = 0
	norm(3) = 1
	weight(3) = chi
Cifdef DEBUG
Ctype *,"x = ",m,"*z + ",a," chi = ",chi
C
C	Compute y = zm + a
C
	Call lin_fit2d(z,y,n,a,m,chi)
	norm(2) = m
	Call normalize(norm)
	mx(3) = norm(1)
	my(3) = norm(2)
	mz(3) = norm(3)
	weight(3) = weight(3) + chi
Cifdef DEBUG
Ctype *,"y = ",m,"*z + ",a," chi = ",chi
Ctype *,"mx = ",norm(1),"my = ",norm(2)," mz = ",norm(3),
C    +		" 1/w=",1.0/weight(3)
C
C	Orientate the normals
C
	norm(1) = mx(1)
	norm(2) = my(1)
	norm(3) = mz(1)
	Do i=2,3
		tmp(1) = mx(i)
		tmp(2) = my(i)
		tmp(3) = mz(i)
		Call Dotprod(norm,tmp,err)
		If (err .lt. 0.0) Then
			mx(i) = -mx(i)
			my(i) = -my(i)
			mz(i) = -mz(i)
		Endif
	Enddo
C
C	compute final average normal
C
	err = 0.0
	norm(1) = 0.0
	Do i=1,3
		if (mx(i) .ne. -9999) Then
			err = err + (1.0/weight(i))
			norm(1) = norm(1) + mx(i)*(1.0/weight(i))
		Endif
	Enddo
	norm(1) = norm(1) / err

	err = 0.0
	norm(2) = 0.0
	Do i=1,3
		if (my(i) .ne. -9999) Then
			err = err + (1.0/weight(i))
			norm(2) = norm(2) + my(i)*(1.0/weight(i))
		Endif
	Enddo
	norm(2) = norm(2) / err

	err = 0.0
	norm(3) = 0.0
	Do i=1,3
		if (mz(i) .ne. -9999) Then
			err = err + (1.0/weight(i))
			norm(3) = norm(3) + mz(i)*(1.0/weight(i))
		Endif
	Enddo
	norm(3) = norm(3) / err

	Call normalize(norm)
C
C	compute errors
C
	err = 0.0
	Do i=1,n
C		
C	Compute distance from the point (x,y,z)(i) to the line (p,norm)
C	D = (line vec) X (point to point)
C	
		tmp(1) = x(i) - p(1)
		tmp(2) = y(i) - p(2)
		tmp(3) = z(i) - p(3)
		Call xprod(norm,tmp,out)
		err = err + sqrt((out(1)*out(1)) + (out(2)*out(2)) +
     +			(out(3)*out(3)))
	Enddo
	err = err / float(n)
C
	Return
	End
C
C	Subroutine to compute the best fit line through a set of 2D points
C
	Subroutine	lin_fit2d(x,y,n,a,b,chi)

	Implicit None

	Real*4		x(*),y(*),a,b,chi
	Integer*4	n

	Real*4		ss,sx,sy,sxss,t,stt
	Integer*4	i

	chi = -1.0

	sx = 0.0
	sy = 0.0
	Do i=1,n
		sx = sx + x(i)
		sy = sy + y(i)
	Enddo
	ss = n
	sxss = sx/ss
	stt = 0
	b = 0
	Do i=1,n
		t = x(i) - sxss
		stt = stt + (t*t)
		b = b + (t*y(i))
	Enddo
	if (stt .eq. 0.0) stt = 0.000001
	b = b / stt

	a = (sy-(sx*b))/ss
	chi = 0
	Do i=1,n
		t = y(i) - (a + (b*x(i)))
		chi = chi + (t*t)
	Enddo

	Return
	End
C
C	Subroutine to compute the closest fit to a pair of lines
C
	Subroutine	intersect_lines(p1,p2,q1,q2,out,error)

	Implicit None

	Real*4		p1(3),p2(3),q1(3),q2(3),error,out(3)

	Real*4		v1(3),v2(3),d,s,t,tp(3),tq(3)

	error = -1
C
C	catch a trivial case
C
	If ((p1(1).eq.q1(1)).and.(p1(2).eq.q1(2)).and.(p1(3).eq.q1(3))) Then
		error = 0
		out(1) = p1(1)
		out(2) = p1(2)
		out(3) = p1(3)
		Return
	Endif
C
C	compute the vectors
C
	v1(1) = p2(1) - p1(1)
	v1(2) = p2(2) - p1(2)
	v1(3) = p2(3) - p1(3)
	If ((v1(1) .eq. 0).and.(v1(2) .eq. 0).and.(v1(3) .eq. 0)) Return
	Call normalize(v1)

	v2(1) = q2(1) - q1(1)
	v2(2) = q2(2) - q1(2)
	v2(3) = q2(3) - q1(3)
	If ((v2(1) .eq. 0).and.(v2(2) .eq. 0).and.(v2(3) .eq. 0)) Return
	Call normalize(v2)

C
C	compute determinate of ((p2-p1),v2,v1Xv2)
C
	Call Xprod(v1,v2,out)

	d = (out(1)*out(1)) + (out(2)*out(2)) + (out(3)*out(3))
	If (d .eq. 0) return

	Call Xprod(v2,out,tp)
	t = ((q1(1) - p1(1))*tp(1))
	t = t + ((q1(2) - p1(2))*tp(2))
	t = t + ((q1(3) - p1(3))*tp(3))
	t = t / d
C
C	compute determinate of ((p1-p2),v1,v2Xv1)
C
	Call Xprod(v2,v1,out)

	d = (out(1)*out(1)) + (out(2)*out(2)) + (out(3)*out(3))
	If (d .eq. 0) return

	Call Xprod(v1,out,tp)
	s = ((p1(1) - q1(1))*tp(1))
	s = s + ((p1(2) - q1(2))*tp(2))
	s = s + ((p1(3) - q1(3))*tp(3))
	s = s / d
C
C	compute the points
C	
	tp(1) = p1(1) + (t*v1(1))
	tp(2) = p1(2) + (t*v1(2))
	tp(3) = p1(3) + (t*v1(3))
	tq(1) = q1(1) + (s*v2(1))
	tq(2) = q1(2) + (s*v2(2))
	tq(3) = q1(3) + (s*v2(3))
C
C	and average...
C
	out(1) = (tp(1) + tq(1))/2
	out(2) = (tp(2) + tq(2))/2
	out(3) = (tp(3) + tq(3))/2
C
C	compute error
C
	error = ((tp(1) - tq(1)) * (tp(1) - tq(1)))
	error = error + ((tp(2) - tq(2)) * (tp(2) - tq(2)))
	error = error + ((tp(3) - tq(3)) * (tp(3) - tq(3)))
	if (error .gt. 0.00001) error = sqrt(error)
C
	Return
	End
