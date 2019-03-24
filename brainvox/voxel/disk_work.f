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
C    MODULE NAME    : disk_work
C    VERSION        : 1.0 
C    LOCATION       :  
C    LAST EDIT      :  
C    STATUS         :  (Development,Testing,Production,Obsolete)
C    LANGUAGE       : FORTRAN
C    PACKAGE        : Brainvox
C
C
C    -- CREATION --------------                                                 
C                                                                               
C    PROGRAMMER     : rjf
C    DATE           : 23 May 94       
C    LINE-BY-LINER  :    
C    APPROVAL       : 
C
C    DESCRIPTION    :  	Routines to implement Matt Howard disk space
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

	Subroutine init_dspace()

	Implicit None

	Include 'disk_space.inc'

	dspace.valid = 0
	dspace.display = 0

	Return
	End
C
C	Routine to handle the Howard disk menus
C
	Subroutine handle_dspace_menu(i,point,num,myset)

	Implicit None

	Include 'disk_space.inc'
	Include 'palettes.inc'
	Include '../libs/vt_inc.f'

	Integer*4	num,point(4096,4)
	Integer*4	myset(*)

	Integer*4	i

	If (i .eq. 500) Then
C
C	Calibrate
C
		Call disk_init_space(point,num,opacity,myset)

	Else if (i .eq. 501) Then
C
C	Toggle lines display
C
		dspace.display = ieor(dspace.display,1)

	Else if (i .eq. 502) Then
C
C	Toggle plane display
C
		dspace.display = ieor(dspace.display,2)

	Else if (i .eq. 503) Then
C
C	Save dspace
C
		Call disk_file_io("_Dspace ",IPC_WRITE,interpixel)

	Else if (i .eq. 504) Then
C
C	Load dspace
C
		Call disk_file_io("_Dspace ",IPC_READ,interpixel)

	Endif

	Return
	End
C
C	Given the point for the center and two points on the surface
C	compute the disk location and orientation
C
	Subroutine disk_init_space(point,num,opacity,myset)

	Implicit None

	Include '../libs/vt_inc.f'
	Include 'disk_space.inc'

	Integer*4	num,point(4096,4),opacity(768)
	Integer*4	myset(*),size
	Real*4		pt(3),ratio,numimg
	Real*4		d0,d1,z(3),x(3),y(3),bestval,p(3)
	Real*4		d2,d3,save(3)

	Integer*4	i,j,k
C
	ratio = 1.0
	size = D_RADIUS*2*interpixel
C
C	Calc number of images
C
	numimg = realimages+((realimages - 1) * interpimages)
C
C	Compute the clusterings for three points
C
	If (num .lt. 3) then
		write(*, *)"More points needed to form Disk space."
		Return
	Endif
	j = 1
	Do i=1,3
C
C	Get the initial point
C
		dspace.points(1,i) = float(point(j,1))-(imagesize/2)
		dspace.points(2,i) = float(point(j,2))-(imagesize/2)
		dspace.points(3,i) = (float(point(j,3))-
     +			(numimg/2))*squeezefactor
		k = j
C
C	Skip all locals
C
		Do While ((j .le. num) .and. (sqrt(
     +			float(point(k,1)-point(j,1))**2 +
     +			float(point(k,2)-point(j,2))**2 +
     +			float(point(k,3)-point(j,3))**2) .lt. TOLERANCE))
			j = j + 1
		Enddo
		If ((j .gt. num).and.(i .ne. 3)) Then
		write(*, *)"More points needed to form Disk space."
			Return
		Endif
	Enddo
C
C	We got the points.  Now compute the Vectors
C
	Do i = 1,3
		dspace.yaxis(i) = dspace.points(i,PT_P2) - 
     +			dspace.points(i,PT_C)
		dspace.xaxis(i) = dspace.points(i,PT_P1) - 
     +			dspace.points(i,PT_C)
	Enddo
	Call Normalize(dspace.yaxis)
	Call Normalize(dspace.xaxis)
C
C	Get Z vector  (X cross Y)
C
	Call Xprod(dspace.xaxis,dspace.yaxis,dspace.zaxis)
	Call Normalize(dspace.zaxis)
C
C	Y is Z cross X
C
	Call Xprod(dspace.zaxis,dspace.xaxis,dspace.yaxis)
	Call Normalize(dspace.yaxis)
C
C	And finally X axis is Y cross Z
C
	Call Xprod(dspace.yaxis,dspace.zaxis,dspace.xaxis)
	Call Normalize(dspace.xaxis)
C
C#ifdef DEBUG
C	type *,"Disk calibration params in:"
C	type *,"Points:"
C	type *,"C:",(dspace.points(i,PT_C),i=1,3)
C	type *,"P1:",(dspace.points(i,PT_P1),i=1,3)
C	type *,"P2:",(dspace.points(i,PT_P2),i=1,3)
C	type *,"Xaxis:",(dspace.xaxis(i),i=1,3)
C	type *,"Yaxis:",(dspace.yaxis(i),i=1,3)
C	type *,"Zaxis:",(dspace.zaxis(i),i=1,3)
C#endif
C
C	Here is how the minimization is done:
C	1) perturb the Z axis over a number of possibilities
C	2) start at PT_C and go + until a plane is < 5%
C	3) start at PT_C and go - until a plane is > 95%
C		accumulate weights*x and weights*y
C	4) PT_C' is the midway between 2 and 3
C		along Zaxis' which is the weights/n
C	5) fit "quality" is how close the un-translated PT_C' is
C		from the original PT_C
C	6) return the new PT_C and axis values
C

C#define I_START -10
C#define I_END 10

	Do i=-10,10
	Do j=-10,10
C
C	Perturb the normal
C
		d0 = float(i)*0.05
		d1 = float(j)*0.05
C#ifdef DEBUG
C	type *,"Normal delta d0,d1=",d0,d1
C#endif
		Do k=1,3
			Z(k) = dspace.zaxis(k) + d0*dspace.xaxis(k) +
     +				d1*dspace.yaxis(k)
		Enddo
		Call Normalize(Z)
C
C	Y' = Z' cross xaxis
C
		Call Xprod(Z,dspace.xaxis,Y)
		Call Normalize(Y)
C
C	X' = Y' cross Z'
C
		Call Xprod(Y,Z,X)
		Call Normalize(X)
C
		Do k=1,3
			p(k) = dspace.points(k,PT_C)
		Enddo
C
C	sample the volume here and return the weighted centroid
C
		Call calc_disk_vol(x,y,z,p,d2,myset,ratio,size,opacity,d3)
C
C	record the best result
C
		if ((i .eq. -10) .and. (j .eq. -10)) Then
			bestval = d2
			save(1) = d0
			save(2) = d1
			save(3) = d3
C#ifdef DEBUG
C			type *,"Keep the first one.",d2,d3
C#endif
		Else if (d2 .lt. bestval) Then
			bestval = d2
			save(1) = d0
			save(2) = d1
			save(3) = d3
C#ifdef DEBUG
C			type *,"Keep this one.",d2,d3
C#endif
		Endif
C
	Enddo
	Enddo
C
C	recall the best one
C
	d0 = save(1)
	d1 = save(2)
	d3 = save(3)
C
	Do k=1,3
		Z(k) = dspace.zaxis(k) + d0*dspace.xaxis(k) +
     +			d1*dspace.yaxis(k)
	Enddo
	Call Normalize(Z)
C
C	Y' = Z' cross xaxis
C
	Call Xprod(Z,dspace.xaxis,Y)
	Call Normalize(Y)
C
C	X' = Y' cross Z'
C
	Call Xprod(Y,Z,X)
	Call Normalize(X)
C
	Do k=1,3
		P(k)=dspace.points(k,PT_C) + d3*z(k)
	Enddo
C
C	replace the values
C
	Do k=1,3
		dspace.points(k,PT_C) = p(k)
		dspace.xaxis(k) = x(k)
		dspace.yaxis(k) = y(k)
		dspace.zaxis(k) = z(k)
	Enddo
C
C#ifdef DEBUG
C	type *,"Disk calibration params out:"
C	type *,"Points:"
C	type *,"C:",(dspace.points(i,PT_C),i=1,3)
C	type *,"P1:",(dspace.points(i,PT_P1),i=1,3)
C	type *,"P2:",(dspace.points(i,PT_P2),i=1,3)
C	type *,"Xaxis:",(dspace.xaxis(i),i=1,3)
C	type *,"Yaxis:",(dspace.yaxis(i),i=1,3)
C	type *,"Zaxis:",(dspace.zaxis(i),i=1,3)
C#endif
C
	dspace.valid = 1
C
	Return
	End
C
C	File I/O for reading/writing the settings
C
	Subroutine disk_file_io(filename,op,ip)

	Implicit None

	Include '../libs/vt_inc.f'
	Include 'disk_space.inc'

	Character*(*)	filename

	logical		found
	Integer*4	i,op
	Real*4		ip

	If ((dspace.valid .eq. 0).and.(op .eq. IPC_WRITE)) Then
		write(*, 1)
1	Format("Disk space must be calibrated before ",
     +			"it can be saved.")
		Return
	Endif

	Inquire(file=filename,exist=found)
	If (op .eq. IPC_READ) then
		If (.not. found) return
		write(*, 300)
300	Format("Reading saved Disk space")
		open(66,file=filename,form="formatted",status="old",
     +			readonly,err=999)
		Do i=1,3
			Read(66,*,err=997,end=997) dspace.points(1,i),
     +				dspace.points(2,i),dspace.points(3,i)
		Enddo
		Do i=1,3
		  dspace.points(1,i) = dspace.points(1,i)/ip
		  dspace.points(2,i) = dspace.points(2,i)/ip
		  dspace.points(3,i) = dspace.points(3,i)/ip
		Enddo
		Read(66,*,err=997,end=997) dspace.xaxis(1),
     +				dspace.xaxis(2),dspace.xaxis(3)
		Read(66,*,err=997,end=997) dspace.yaxis(1),
     +				dspace.yaxis(2),dspace.yaxis(3)
		Read(66,*,err=997,end=997) dspace.zaxis(1),
     +				dspace.zaxis(2),dspace.zaxis(3)
		dspace.valid = 1
	Else if (op .eq. IPC_WRITE) then
20	Format(2F15.6)
30	Format(3F15.6)
		write(*, 301)
301	Format("Writing Disk space to disk")
		If (.not. found) Then
			open(66,file=filename,form="formatted",
     +				status="new",err=999)
		Else
			open(66,file=filename,form="formatted",
     +				status="old",err=999)
		Endif
		Do i=1,3
			Write(66,30,err=997)
     +		 		dspace.points(1,i)*ip,
     +				dspace.points(2,i)*ip,
     +				dspace.points(3,i)*ip
		Enddo
		Write(66,30,err=997) dspace.xaxis(1),
     +				dspace.xaxis(2),dspace.xaxis(3)
		Write(66,30,err=997) dspace.yaxis(1),
     +				dspace.yaxis(2),dspace.yaxis(3)
		Write(66,30,err=997) dspace.zaxis(1),
     +				dspace.zaxis(2),dspace.zaxis(3)
	Endif
	Close(66,err=995)

	Return

997	Close(66,err=995)
995	write(*, 996)filename(1:index(filename," ")-1)
996	Format("I/O Error in using the file:",A)
	Return

999	write(*, 998)filename(1:index(filename," ")-1)
998	Format("Error in opening Disk space file:",A)

	Return
	End
C
C	conversion routines
C
	Subroutine disk2point(disk,x,y,z)

	Implicit None

	Include '../libs/vt_inc.f'
	Include 'disk_space.inc'

	Integer*4	x,y,z,i
	Real*4		disk(3),tmp(3),vec(3),numimg
C
	If (dspace.valid .eq. 0) return
C
C	Convert the disk space coords into the BV pixel space
C
C	Calc number of images
C
	numimg = realimages+((realimages - 1) * interpimages)
C
C	Disk coords are in plateang,upang,depth order
C
C	rotate upwards by disk(2) degrees (start from along X axis)
C
	vec(1) = cosd(disk(2))
	vec(2) = 0
	vec(3) = sind(disk(2))
C
C	rotate in the plane by disk(1) degrees (adjust only X and Y)
C
	tmp(1) = vec(1)*cosd(disk(1)) - vec(2)*sind(disk(1))
	tmp(2) = vec(1)*sind(disk(1)) + vec(2)*cosd(disk(1))
	tmp(3) = vec(3)
	Call normalize(tmp)
C
C	make axis become real space axis
C
	Do i=1,3
		vec(i) = (tmp(1)*dspace.xaxis(i)) +
     +			(tmp(2)*dspace.yaxis(i)) +
     +			(tmp(3)*dspace.zaxis(i))
	Enddo
C
C	translate from origin point by distance
C
	Do i=1,3
		vec(i) = dspace.points(i,PT_C) - vec(i)*disk(3)
	Enddo
C
C	Convert the point back into a BV point (tracing)
C
	x = vec(1) + (imagesize/2)
	y = vec(2) + (imagesize/2)
	z = (vec(3)/squeezefactor) + (numimg/2)
C#ifdef DEBUG
C	type *,"Input:",disk(1),disk(2),disk(3)
C	type *,"tmp:",tmp(1),tmp(2),tmp(3)
C	type *,"Vec:",vec(1),vec(2),vec(3)
C	type *,"Output:",x,y,z                
C	type *,"Numimg,squ:",numimg,squeezefactor
C	Call point2disk(x,y,z,tmp)
C	type *,"Backcheck:",tmp(1),tmp(2),tmp(3)
C#endif
C
	Return
	End
C
	Subroutine point2disk(x,y,z,disk)

	Implicit None

	Include '../libs/vt_inc.f'
	Include 'disk_space.inc'

	Integer*4	x,y,z,i
	Real*4		disk(3),vec(3),tmp(3),numimg
C
	If (dspace.valid .eq. 0) return
C
C	Calc number of images
C
	numimg = realimages+((realimages - 1) * interpimages)
C
C	make it volume centered
C
	tmp(1) = float(x)-(imagesize/2)
	tmp(2) = float(y)-(imagesize/2)
	tmp(3) = (float(z)-(numimg/2))*squeezefactor
C
C	create the vector from origin to the point
C	
	Do i=1,3
		vec(i) = tmp(i) - dspace.points(i,PT_C)
	Enddo
C
C	Get the projections onto the disk vectors
C
	Call Dotprod(vec,dspace.xaxis,tmp(1))
	Call Dotprod(vec,dspace.yaxis,tmp(2))
	Call Dotprod(vec,dspace.zaxis,tmp(3))
C
C	The entry point is on the opposite side
C
	Do i=1,3
		tmp(i) = -tmp(i)
	Enddo
C
C	get the length 
C
	Call Dotprod(tmp,tmp,disk(3))
	If (disk(3) .ne. 0) disk(3) = sqrt(disk(3))
C
C	find the direction
C
	Call normalize(tmp)
C
C	project onto X,Y plane
C
	vec(1) = tmp(1)
	vec(2) = tmp(2)
	vec(3) = 0
	Call normalize(vec) ! vector in X,Y plane 
C
C	disk(1) = angle in X,Y plane
C
	disk(1) = atan2d(vec(2),vec(1))
C
C	disk(2) = angle between vec and tmp
C
	Call Dotprod(vec,tmp,disk(2))
	disk(2) = acosd(disk(2))
C
	Return
	End
C
C	Routine to calc the centroid of a local disk volume
C
	Subroutine calc_disk_vol(x,y,z,p,fval,myset,ratio,size,opacity,d)

	Implicit None

	Include 'disk_space.inc'

C#ifdef DUMPFILE
C        Include         'fdevice.h'
C        Include         'fgl.h'
C        Logical         err
C#endif

	Real*4		x(3),y(3),z(3),p(3),fval,pt(3)
	Integer*4	myset(*),size,opacity(768),i
	Real*4		ratio,hits,d,sx,sy,sw,dd
	Integer*2	image(DI_RADIUS*DI_RADIUS*2*2*10)
C
C	remember that Z+ is outside the brain and Z- is inside
C
	d = 0.0
	sx = 0.0
	sy = 0.0
	sw = 0.0
C
	Do i=1,3
		pt(i) = p(i) + d*z(i)
	Enddo
C
C	Get the starting location
C
	Call vl_resample_axis(%val(size),%val(size),image,x,y,z,pt,
     +		myset,ratio)
	Call calc_hits(hits,image,size,opacity,sx,sy,sw)

C#ifdef DUMPFILE
C        Do while ((getbut(PAD4) .eq. .false.).and.
C     +		(getbut(PAD6) .eq. .false.))
C        Enddo
C	If (getbut(PAD6)) Then
C        Call writebinfile("/usr/tmp/tmp.bin ",image,
C     +                  size*size*2,err)
C        Call system("vbin -16 /usr/tmp/tmp.bin")
C	Endif
C        Do while (getbut(PAD6) .eq. .true.)
C        Enddo
C#endif
C
C	walk outward until outside, or very far off
C
	d = 0
	dd = hits
	Do while ((hits .gt. 0.05).and.(d .le. 10))
C
		d = d + 1.0
		Do i=1,3
			pt(i) = p(i) + d*z(i)
		Enddo
C
		Call vl_resample_axis(%val(size),%val(size),image,
     +			x,y,z,pt,myset,ratio)
		Call calc_hits(hits,image,size,opacity,sx,sy,sw)
C
	Enddo
C
C	reset hits to number at d=0, save the outward d, and reset d
C
	hits = dd
	dd = d
	d = 0.0
C
C	walk inward until inside, or very far in
C
	Do while ((hits .lt. 0.95).and.(d .ge. -10))
C
		d = d - 1.0
		Do i=1,3
			pt(i) = p(i) + d*z(i)
		Enddo
C
		Call vl_resample_axis(%val(size),%val(size),image,
     +			x,y,z,pt,myset,ratio)
		Call calc_hits(hits,image,size,opacity,sx,sy,sw)
C
	Enddo
C
C	compute the centroid of the disk
C
	sx = sx / sw
	sy = sy / sw
	d = (d+dd)/2  ! average of the outsides
C
C	error is equal to the radius the sx,sy point misses
C
	fval = (sx*sx) + (sy*sy)
C#ifdef DEBUG
C	type *,"Hits=",hits,"Val=",fval,"sx,sy=",sx,sy
C#endif
C
	Return
	End
C
C	Compute hits % from an image
C
	Subroutine calc_hits(hits,image,size,opacity,sx,sy,sw)

	Implicit None

	Real*4		hits,sx,sy,sw,radiuss,x,y
	Integer*4	size,opacity(768)
	Integer*2	image(*)
	Integer*4	i,j,n,k,ptr

	hits = 0.0
	n = 0
C
	radiuss = (float(size)/2)**2
C
	ptr = 1
	Do j=0,size-1
	Do i=0,size-1
C
C	get real location
C
		x = float(i) - (float(size)/2)
		y = float(j) - (float(size)/2)

		If (((x*x)+(y*y)) .le. radiuss) Then
C
C	voxel value and opacity
C
			k = image(ptr) - 255
			If (opacity(k) .gt. 2) then
C
C	Sum the number of hits in the disk
C
				hits = hits + 1
C
C	and weight the centroid acculmulators
C
				sx = sx + x*opacity(k)
				sy = sy + y*opacity(k)
				sw = sw + opacity(k)
C
C	Count the voxels
C
			Endif
			n = n + 1
		Endif
C
		ptr = ptr + 1
C
	Enddo
	Enddo

	hits = hits/float(n)

	Return
	End
C
C	Routines for displaying the disk
C
        Subroutine draw_dspace_voxel(rots,myset,imagesize,interpixel,
     +          offset,zoom)

        Implicit None

        Include 'rotsdef.inc'
	Include 'disk_space.inc'

        Record  /rotsrec/       rots
        Integer*4               myset(*),imagesize
        Integer*4               offset(2),zoom
        real*4                  interpixel
C
        Integer*4               a,b,solid,size,i,h
        Real*4                  dx,dy
        Real*4                  oth(6),p1(3),p2(3)
C
C	Is it Ok?
C
	If (dspace.valid .eq. 0) return
C
C       Get windowsize/2
C
        Call getsiz(a,b)
        dx = float(a)/float(zoom)
        dy = float(b)/float(zoom)
        a = 445/2
        b = 445/2
C
C       Generate the ortho parameters
C
        oth(1) = (-a) + offset(1)
        oth(2) = oth(1) + dx
        oth(3) = (-b) + offset(2)
        oth(4) = oth(3) + dy
C
C       Z depths are fixed
C
        oth(5) = -(445/2)
        oth(6) = (445/2)
C
        Call ortho(oth(1),oth(2),oth(3),oth(4),oth(5),oth(6))
C
C       draw the graphics (if needed)
C
C	A square for the moment
C
	If (iand(dspace.display,2) .eq. 2) Then
		size = D_RADIUS*2*interpixel
		solid = 1
		Do i=1,3
			p1(i) = dspace.points(i,PT_C)
		Enddo
		h = '00ff00ff'X
		Call cpack(h)
	        Call vl_mpfit_uplane(%val(size),%val(size),
     +                  dspace.xaxis,dspace.yaxis,
     +                  dspace.zaxis,p1,myset,%val(solid))

	Endif
C
C	Lines  (X+,Z-)
C
	If (iand(dspace.display,1) .eq. 1) Then
		Do i=1,3
			p1(i) = dspace.points(i,PT_C) + 10*dspace.zaxis(i)
			p2(i) = dspace.points(i,PT_C) - 50*dspace.zaxis(i)
		Enddo
		h = '00ffff00'X
		Call cpack(h)
		Call vl_mpfit_line(p1,p2,myset)
		Do i=1,3
			p1(i) = dspace.points(i,PT_C) - 10*dspace.xaxis(i)
			p2(i) = dspace.points(i,PT_C) + 50*dspace.xaxis(i)
		Enddo
		h = '00ffff00'X
		Call cpack(h)
		Call vl_mpfit_line(p1,p2,myset)
	Endif
C
        Return
        End
