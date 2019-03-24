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
C    MODULE NAME    : Talairach
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
C    DATE           : 8 Jun 92        
C    LINE-BY-LINER  :    
C    APPROVAL       : 
C
C    DESCRIPTION    :  	Routines to implement Talairach space conversions
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
C
	Subroutine init_tal

	Implicit None

	Include 'tal_space.inc'

	tspace.valid = 0

	Return
	End
C
C	Given the points for CA CP and the plane get the vectors and limits
C	In the order: CA,CP,plane
C
	Subroutine tal_init_space(point,num,opacity,myset)

	Implicit None

	Include '../libs/vt_inc.f'
	Include 'tal_space.inc'

	Integer*4	num,point(4096,4),opacity(768)
	Integer*4	myset(*),size
	Integer*2	image(512*512)
	Real*4		pt(3),ratio,numimg

	Integer*4	i,j,k
C
C	Calc number of images
C
	numimg = realimages+((realimages - 1) * interpimages)
C
C	Compute the clusterings for three points
C
	If (num .lt. 3) then
		write(*, *)"More points needed to form Talairach space."
		Return
	Endif
	j = 1
	Do i=1,3
C
C	Get the initial point
C
		tspace.points(1,i) = float(point(j,1))-(imagesize/2)
		tspace.points(2,i) = float(point(j,2))-(imagesize/2)
		tspace.points(3,i) = (float(point(j,3))-
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
		write(*, *)"More points needed to form Talairach space."
			Return
		Endif
	Enddo
C
C	We got the points.  Now compute the Vectors
C
	Do i = 1,3
		tspace.yaxis(i) = tspace.points(i,PT_CA) - 
     +			tspace.points(i,PT_CP)
	Enddo
	Call Normalize(tspace.yaxis)
C
C	Get temporary Z vector
C
	Do i = 1,3
		tspace.zaxis(i) = tspace.points(i,PT_PLANE) - 
     +			tspace.points(i,PT_CP)
	Enddo
	Call Normalize(tspace.zaxis)
C
C	X axis is Y cross Z
C
	Call Xprod(tspace.yaxis,tspace.zaxis,tspace.xaxis)
	Call Normalize(tspace.xaxis)
C
C	And TRUE Z axis is X cross Y
C
	Call Xprod(tspace.xaxis,tspace.yaxis,tspace.zaxis)
	Call Normalize(tspace.zaxis)
C
C	Find the threshold
C
	j = 0
	Do i=256,1,-1
		if (opacity(i) .gt. 2) j = i
	Enddo
C	
C	Now, Scan along the Y axis until you find the last slices w/brain
C
	write(*, 10)"Scanning for front..."
	Call vol_scanner(tspace.points(1,PT_CA),tspace.xaxis(1),
     +		tspace.zaxis(1),tspace.yaxis(1),myset,1.0,j,
     +		tspace.CA_front)
	tspace.CA_front = abs(tspace.CA_front)
C
C	Scan backward along Y axis for last slice w/brain
C
	write(*, 10)"Scanning for back..."
	Call vol_scanner(tspace.points(1,PT_CP),tspace.xaxis(1),
     +		tspace.zaxis(1),tspace.yaxis(1),myset,-1.0,j,
     +		tspace.CP_back)
	tspace.CP_back = abs(tspace.CP_back)
C
	tspace.CA_CP = sqrt(
     +		(tspace.points(1,PT_CP)-tspace.points(1,PT_CA))**2+
     +		(tspace.points(2,PT_CP)-tspace.points(2,PT_CA))**2+
     +		(tspace.points(3,PT_CP)-tspace.points(3,PT_CA))**2)
C
C	Scan Right along X axis for right side of brain
C
	write(*, 10)"Scanning for right..."
	Call vol_scanner(tspace.points(1,PT_CA),tspace.yaxis(1),
     +		tspace.zaxis(1),tspace.xaxis(1),myset,1.0,j,
     +		tspace.CA_right)
	tspace.CA_right = abs(tspace.CA_right)
C
C	Scan Left along X axis for left side of brain
C
	write(*, 10)"Scanning for left..."
	Call vol_scanner(tspace.points(1,PT_CA),tspace.yaxis(1),
     +		tspace.zaxis(1),tspace.xaxis(1),myset,-1.0,j,
     +		tspace.CA_left)
	tspace.CA_left = abs(tspace.CA_left)
C
C	Scan up along Z axis for top of brain
C
	write(*, 10)"Scanning for top..."
	Call vol_scanner(tspace.points(1,PT_CA),tspace.xaxis(1),
     +		tspace.yaxis(1),tspace.zaxis(1),myset,1.0,j,
     +		tspace.CA_top)
	tspace.CA_top = abs(tspace.CA_top)
C
C	Scan down along Z axis for bottom of brain
C
	write(*, 10)"Scanning for bottom..."
	Call vol_scanner(tspace.points(1,PT_CA),tspace.xaxis(1),
     +		tspace.yaxis(1),tspace.zaxis(1),myset,-1.0,j,
     +		tspace.CA_bottom)
	tspace.CA_bottom = abs(tspace.CA_bottom)
C
10	Format("Talairach calibration:",A)
Cifdef DEBUG
Ctype *,"Talairach calibration params:"
Ctype *,"Points:"
Ctype *,"CA:",(tspace.points(i,PT_CA),i=1,3)
Ctype *,"CP:",(tspace.points(i,PT_CP),i=1,3)
Ctype *,"PLANE:",(tspace.points(i,PT_PLANE),i=1,3)
Ctype *,"Xaxis:",(tspace.xaxis(i),i=1,3)
Ctype *,"Yaxis:",(tspace.yaxis(i),i=1,3)
Ctype *,"Zaxis:",(tspace.zaxis(i),i=1,3)
Ctype *,"CA_front,CA_CP,CP_back:",tspace.CA_front,
C    +			tspace.CA_CP,tspace.CP_back
Ctype *,"CA_top,CA_bottom:",tspace.CA_top,tspace.CA_bottom
Ctype *,"CA_right,CA_left:",tspace.CA_right,tspace.CA_left
Ctype *,"Threshold:",j
C
	tspace.valid = 1
C
	Return
	End
C
C	Routine to modify the current Talairach space (calibrated)
C	to match the St. Louis method from Tom Videen (6 May 94 message)
C
	Subroutine  tal_2_stlouis

	Implicit None

	Include '../libs/vt_inc.f'
	Include 'tal_space.inc'

	Real*4		d,shift,d2,d3,p(3)
	Integer*4	i
C
C	I believe the original BV tal space is better, but this is for
C	comparision purposes.
C
	If (tspace.valid .ne. 1) Then
		write(*, 1)
1	Format("Talairach space must be calibrated before ",
     +		"the St Louis modifications can be applied.")
		Return
	Endif
C
C	Differences:
C	1) R/L is evenly spaced (move the AC,PC points to correct)
C
	d = tspace.CA_right + tspace.CA_left
	d = d/2
	shift = tspace.CA_right - d
	Do i=1,3
		tspace.points(i,PT_CA) = tspace.points(i,PT_CA) +
     +			tspace.xaxis(i)*shift
		tspace.points(i,PT_CP) = tspace.points(i,PT_CP) +
     +			tspace.xaxis(i)*shift
		tspace.points(i,PT_PLANE) = tspace.points(i,PT_PLANE) +
     +			tspace.xaxis(i)*shift
	Enddo
	tspace.CA_right = d
	tspace.CA_left = d
C
C	2) bottom is scaled same as top (move bottom distance to
C			compensate)
C
	d = tspace.CA_top/TAL_TOP
	tspace.CA_bottom = d * TAL_BOTTOM
C
C	3) Y axis is different (move AC,PC Y points to compensate)
C		(This one is fun :)
C	
C	get the distance in use now
C
	d = tspace.CA_front + tspace.CA_CP + tspace.CP_back
C
C	since they use linear scaling, CA is at 68/172 of d from front
C	and CP is scaled to fit (ratio of middle over middle+back times
C	remaining length of d)
C
	d2 = d*(68.0/172.0)
	d3 = (d-d2)*(TAL_MIDDLE/(TAL_MIDDLE+TAL_BACK))
C
C 	Place CA (d2) from the front and CP (d3) from CA along Y axis
C	
C	start at CA and move forward
C
	Do i=1,3
		p(i) = tspace.points(i,PT_CA) + 
     +			tspace.yaxis(i)*tspace.CA_front
	Enddo
C
C	move back d2
C
	Do i=1,3
		p(i) = p(i) - tspace.yaxis(i)*d2
		tspace.points(i,PT_CA) = p(i)
	Enddo
C
C	move back d3
C
	Do i=1,3
		p(i) = p(i) - tspace.yaxis(i)*d3
		tspace.points(i,PT_CP) = p(i)
	Enddo
C
C	change the distance values
C
	tspace.CA_front = d2
	tspace.CA_CP = d3
	tspace.CP_back = d - (d2+d3)
C
	write(*, 2)
2	Format("Talairach space has been modified to mimic ",
     +		"the St Louis construction method.")

	Return
	End
C
C	File I/O for reading/writing the settings
C
	Subroutine tal_file_io(filename,op,ip)

	Implicit None

	Include '../libs/vt_inc.f'
	Include 'tal_space.inc'

	Character*(*)	filename

	logical		found
	Integer*4	i,op,strlen
	Real*4		ip
	External strlen

	If ((tspace.valid .eq. 0).and.(op .eq. IPC_WRITE)) Then
		write(*, 1)
1	Format("Talairach space must be calibrated before ",
     +			"it can be saved.")
		Return
	Endif

	Inquire(file=filename,exist=found)
	If (op .eq. IPC_READ) then
		If (.not. found) return
		write(*, 300)
300	Format("Reading saved Talairach space")
		open(66,file=filename,form="formatted",status="old",
     +			readonly,err=999)
		Do i=1,3
			Read(66,*,err=997,end=997) tspace.points(1,i),
     +				tspace.points(2,i),tspace.points(3,i)
		Enddo
		Do i=1,3
		  tspace.points(1,i) = tspace.points(1,i)/ip
		  tspace.points(2,i) = tspace.points(2,i)/ip
		  tspace.points(3,i) = tspace.points(3,i)/ip
		Enddo
		Read(66,*,err=997,end=997) tspace.CA_front,
     +				tspace.CP_back,tspace.CA_CP
		tspace.CA_front = tspace.CA_front/ip
		tspace.CP_back = tspace.CP_back/ip
		tspace.CA_CP = tspace.CA_CP/ip
		Read(66,*,err=997,end=997) tspace.CA_top,
     +				tspace.CA_bottom
		tspace.CA_top = tspace.CA_top/ip
		tspace.CA_bottom = tspace.CA_bottom/ip
		Read(66,*,err=997,end=997) tspace.CA_right,
     +				tspace.CA_left
		tspace.CA_right = tspace.CA_right/ip
		tspace.CA_left = tspace.CA_left/ip
		Read(66,*,err=997,end=997) tspace.xaxis(1),
     +				tspace.xaxis(2),tspace.xaxis(3)
		Read(66,*,err=997,end=997) tspace.yaxis(1),
     +				tspace.yaxis(2),tspace.yaxis(3)
		Read(66,*,err=997,end=997) tspace.zaxis(1),
     +				tspace.zaxis(2),tspace.zaxis(3)
		tspace.valid = 1
	Else if (op .eq. IPC_WRITE) then
20	Format(2F15.6)
30	Format(3F15.6)
		write(*, 301)
301	Format("Writing Talairach space to disk")
		If (.not. found) Then
			open(66,file=filename,form="formatted",
     +				status="new",err=999)
		Else
			open(66,file=filename,form="formatted",
     +				status="old",err=999)
		Endif
		Do i=1,3
			Write(66,30,err=997)
     +		 		tspace.points(1,i)*ip,
     +				tspace.points(2,i)*ip,
     +				tspace.points(3,i)*ip
		Enddo
		Write(66,30,err=997) tspace.CA_front*ip,
     +				tspace.CP_back*ip,
     +				tspace.CA_CP*ip
		Write(66,20,err=997) tspace.CA_top*ip,
     +				tspace.CA_bottom*ip
		Write(66,20,err=997) tspace.CA_right*ip,
     +				tspace.CA_left*ip
		Write(66,30,err=997) tspace.xaxis(1),
     +				tspace.xaxis(2),tspace.xaxis(3)
		Write(66,30,err=997) tspace.yaxis(1),
     +				tspace.yaxis(2),tspace.yaxis(3)
		Write(66,30,err=997) tspace.zaxis(1),
     +				tspace.zaxis(2),tspace.zaxis(3)
	Endif
	Close(66,err=995)

	Return

997	Close(66,err=995)
995	write(*, 996)filename(1:strlen(filename))
996	Format("I/O Error in using the file:",A)
	Return

999	write(*,998)filename(1:strlen(filename))
998	Format("Error in opening Taraiach space file:",A)

	Return
	End
C
C	conversion routines
C
	Subroutine tal_tal2point(tal,x,y,z)

	Implicit None

	Include '../libs/vt_inc.f'
	Include 'tal_space.inc'

	Integer*4	x,y,z,i
	Real*4		tal(3),tmp(3),vec(3),numimg
C
	If (tspace.valid .eq. 0) return
C
C	Convert the tal space coords into the BV pixel space
C
C	Z (top,bottom)
C
	If (tal(3) .ge. 0.0) Then
		tmp(3) = (tal(3)/TAL_TOP)*(tspace.CA_top)
	Else
		tmp(3) = (tal(3)/TAL_BOTTOM)*(tspace.CA_bottom)
	Endif
C
C	X  (right,left)
C
	If (tal(1) .ge. 0.0) Then
		tmp(1) = (tal(1)/TAL_RIGHT)*(tspace.CA_right)
	Else
		tmp(1) = (tal(1)/TAL_LEFT)*(tspace.CA_left) 
	Endif
C
C	Y (front,middle,back)
C
	If (tal(2) .ge. 0.0) Then
		tmp(2) = (tal(2)/TAL_FRONT)*(tspace.CA_front)
	Elseif (abs(tal(2)) .le. (TAL_MIDDLE)) Then
		tmp(2) = (tal(2)/TAL_MIDDLE)*(tspace.CA_CP)
	Else  
		tmp(2) = (tal(2)+TAL_MIDDLE)/TAL_BACK
		tmp(2) = (tmp(2)*(tspace.CP_back))-(tspace.CA_CP)
	Endif
C
C remember tal(2) is Neg in last case
C
C	The point is actually CA + (tmp*axis)
C
	Do i=1,3
		vec(i) = (tmp(1)*(tspace.xaxis(i))) + 
     +			(tmp(2)*(tspace.yaxis(i))) +
     +			(tmp(3)*(tspace.zaxis(i)))
	Enddo
	Do i=1,3
		vec(i) = vec(i) + (tspace.points(i,PT_CA))
	Enddo
C
C	Calc number of images
C
	numimg = realimages+((realimages - 1) * interpimages)
C
C	Convert the point back into a BV point (tracing)
C
	x = vec(1) + (imagesize/2)
	y = vec(2) + (imagesize/2)
	z = (vec(3)/squeezefactor) + (numimg/2)
Cifdef DEBUG
Ctype *,"Input:",tal(1),tal(2),tal(3)
Ctype *,"tmp:",tmp(1),tmp(2),tmp(3)
Ctype *,"Vec:",vec(1),vec(2),vec(3)
Ctype *,"Output:",x,y,z                
Ctype *,"Numimg,squ:",numimg,squeezefactor
CCall tal_point2tal(x,y,z,tmp)
Ctype *,"Backcheck:",tmp(1),tmp(2),tmp(3)
C
	Return
	End
C
	Subroutine tal_point2tal(x,y,z,tal)

	Implicit None

	Include '../libs/vt_inc.f'
	Include 'tal_space.inc'

	Integer*4	x,y,z,i
	Real*4		tal(3),vec(3),tmp(3),numimg
C
	If (tspace.valid .eq. 0) return
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
C	create the vector from CA to the point
C	
	Do i=1,3
		vec(i) = tmp(i) - tspace.points(i,PT_CA)
	Enddo
C
C	Get the projections onto the talairach vectors
C
	Call Dotprod(vec,tspace.xaxis,tmp(1))
	Call Dotprod(vec,tspace.yaxis,tmp(2))
	Call Dotprod(vec,tspace.zaxis,tmp(3))
C
C	Scale the point into Tspace
C
C	Z (top,bottom)
C
	If (tmp(3) .ge. 0.0) Then
		tal(3) = (tmp(3)/(tspace.CA_top))*TAL_TOP
	Else
		tal(3) = (tmp(3)/(tspace.CA_bottom))*TAL_BOTTOM
	Endif
C
C	X  (right,left)
C
	If (tmp(1) .ge. 0.0) Then
		tal(1) = (tmp(1)/(tspace.CA_right))*TAL_RIGHT
	Else
		tal(1) = (tmp(1)/(tspace.CA_left))*TAL_LEFT
	Endif
C
C	Y (front,middle,back)
C
	If (tmp(2) .ge. 0.0) Then
		tal(2) = (tmp(2)/(tspace.CA_front))*TAL_FRONT
	Elseif (abs(tmp(2)) .le. (tspace.CA_CP)) Then
		tal(2) = (tmp(2)/(tspace.CA_CP))*TAL_MIDDLE
	Else  
		tal(2) = (tmp(2)+(tspace.CA_CP))/(tspace.CP_back)
		tal(2) = (tal(2)*TAL_BACK) - TAL_MIDDLE
	Endif
C
C remember tmp(2) is neg in last case
C
	Return
	End
C
C	Routine to find the last slice in a direction
C
	Subroutine	vol_scanner(pnt,x,y,n,myset,inc,THRES,d)
	
	Implicit None

	Real*4		pt(3),pnt(3),x(3),y(3),n(3),ratio,inc,d
	Real*4		step,prev_list(50,2)
	Integer*4	myset(*),size,i,k,num,THRES
	Integer*2	image(512*512)

	ratio = 1.0
	size = 256
	k = 1
	d = 0
	num = 0
C
C	initial step size is quite large
C
	step = inc * 32.0
	Do while (k .ne. 0)
		write(*, *)"scanning:",d
C
C	Look to see if we've been there before
C
		i = 0
		Do k=1,num
			if (prev_list(k,1) .eq. d) i = k
		Enddo
		If (i .eq. 0) then
C
C	If not, resample
C
			Do i=1,3
				pt(i) = pnt(i) + d*n(i)
			Enddo
			Call vl_resample_axis(%val(size),%val(size),
     +				image,x(1),y(1),n(1),pt(1),myset,ratio)
			k = 0
			Do i=1,size*size
				if (image(i) .gt. (THRES+256)) k = 1
			Enddo
			If (num .lt. 50) then
				num = num + 1
				prev_list(num,1) = d
				prev_list(num,2) = k
			Endif
		Else
			k = prev_list(i,2)
		Endif
C
C	Reduce the step size, backup, and cont on (if step is small & blank)
C
		If ((k .eq. 0) .and. (abs(step) .gt. abs(inc))) Then
			d = d - step
			step = step / 2.0
			k = 1
		Endif

		if (k .ne. 0) d = d + step
	Enddo

	d = d - step  ! return the last plane with brain on it

	Return
	End
C
C	Routine to resample a dataset into a 1.0mm Talairach dataset
C
	Subroutine	tal_resample(tempname,inc)

	Implicit None

	Include '../libs/vt_inc.f'
	Include 'tal_space.inc'

	Character*(*)	tempname

	Integer*4	i,p(3),x,y,z,j,dx,dy,inc,inum,tmp,k
	Real*4		zoff,tal(3)
	Byte		out(512*512)
	Integer*4       data(4096,4),num,dout(4096,4)
	logical		error
	Character*200	filename,tstr,setname

	Integer*4	bv_mkdir,strlen
	External	bv_mkdir,strlen
C
	If (tspace.valid .eq. 0) return
C
C       Make the TAL_SAMP subdirectory
C
	Inquire(exist=error,file="TAL_SAMP.dir")
	filename = "TAL_SAMP.dir"//char(0)
	tmp = '000041ff'X
	If (.not. error) i = bv_mkdir(filename,tmp)
C
C	write _taliarach file
C
	filename = "TAL_SAMP.dir/_talairach"
	open(66,file=filename,form="formatted",status="unknown",err=320)
C	Zoff should be -15.5
	zoff = TAL_BOTTOM - ((TAL_TOP + TAL_BOTTOM + 0.5)/2.0)
	write(66,301,err=310) 0.0,0.0,zoff
	write(66,301,err=310) 0.0,-(TAL_MIDDLE),zoff
	write(66,301,err=310) 0.0,0.0,30.0
	write(66,301,err=310) TAL_FRONT,TAL_BACK,TAL_MIDDLE
	write(66,302,err=310) TAL_TOP,TAL_BOTTOM
	write(66,302,err=310) TAL_RIGHT,TAL_LEFT
	write(66,301,err=310) -1.0,0.0,0.0   ! this gets swapped ie 128-x
	write(66,301,err=310) 0.0,1.0,0.0
	write(66,301,err=310) 0.0,0.0,1.0
301	format(3F15.6)
302	format(2F15.6)
310	close(66,err=320)
	goto 300
320	write(*, *)"Error, Unable to save the file:",filename
	Call bv_error("Unable to save the file|"//
     +		filename(1:strlen(filename))//
     +		"|Check diskspace and file permissions}",0)
300	continue
C
	tstr = "TAL_SAMP.dir/"//tempname
C
C	get some vars
C
	dx = imagesize
	dy = imagesize
	inum = realimages + (realimages-1)*interpimages
	j = 1
C
	Do z=-(TAL_BOTTOM),(TAL_TOP),inc
C
C	Special case for cthead work
C
C	Do z=-(TAL_BOTTOM)-160,(TAL_TOP)+50,inc
C
C	for each level
C	Clear it
C
		Do i=1,dx*dy
			out(i) = 0
		Enddo
C
C	Resample it
C
C		Do y=-(TAL_BACK+TAL_MIDDLE),(TAL_FRONT),1
C			Do x=-(TAL_LEFT),(TAL_RIGHT),1
C	Completely
C
		Do y=-(dy/2)+1,(dy/2)-1,1
			Do x=-(dx/2)+1,(dx/2)-1,1
C
C	Transform Talairach into point space
C
			   tal(1) = x
			   tal(2) = y
			   tal(3) = z
			   Call tal_tal2point(tal,p(1),p(2),p(3))
C				type *,"Tal=",tal(1),tal(2),tal(3)
C				type *,"p=",p(1),p(2),p(3)
C
C	Get the pixel at i,j,k
C
			   i = 0
                if ((p(1) .lt. 0) .or. (p(1) .ge. imagesize)) goto 100
                if ((p(2) .lt. 0) .or. (p(2) .ge. imagesize)) goto 100
                if ((p(3) .lt. 0) .or. (p(3) .ge. inum)) goto 100
			   Call get_pix(i,p,%val(images_addr),dx,dy)
100		           if (i .ge. 128) i = i - 256	
C
C	Store into image
C
			   out((dx*(y+(dy/2)))+((dx/2)-x))=i
C
			Enddo
		Enddo
C
		call name_changer(tstr,j,filename,error)
C
C       And flip the image over the X axis
C
                Call flipbyteimage(out)
C
C       write the image to disk
C
                Write(*,10) filename(1:strlen(filename))
10      Format("Writing sampled file:",A)
                Call write_image(filename,out,error)
C
                j = j + 1
C
	Enddo
C
C	Now the pointsets
C
	Do j=0,19
C
C       For each of the pointsets
C       Read the pointset
C
                setname = " "
                num = 0   ! pre-clear the input buffer
                Call point_io(j,setname,0,data,num,'./ ')
                tmp = 0   ! accumulate the points into dout
C
C       Map the points into _tal space
C
                Do k=1,num
			p(1) = data(k,1)
			p(2) = data(k,2)
			p(3) = data(k,3)
			Call tal_point2tal(p(1),p(2),p(3),tal)
C
C	add the tal point to the output list
C
			tmp = tmp + 1
			dout(tmp,1) = (imagesize/2) - tal(1)
			dout(tmp,2) = tal(2) + (imagesize/2)
			dout(tmp,3) = tal(3) + TAL_BOTTOM
			dout(tmp,4) = data(k,4)
		Enddo
C
C	write out the pointset
C
                If (tmp .ne. 0) Call point_io(j,setname,1,dout,tmp,
     +                  'TAL_SAMP.dir/ ')
C
	Enddo

	Return
	End
C
C	Routine to read in a list of Tal points and enter them into
C	brainvox space with some color codeing scheme
C
	Subroutine read_tal_points(num,points,filename)

	Implicit None

	Include '../libs/vt_inc.f'
	Include 'tal_space.inc'

	Character*(*)	filename
	Integer*4	num,points(4096,4),strlen
	Integer*4	color
	Real*4		tpoint(4)
	Character*200	string
	External strlen
C
C	Check for valid Tal space
C
	If (tspace.valid .eq. 0) Then
		write(*, 1)
1	Format("Talairach space must be calibrated before ",
     +			"points can be loaded.")
		Return
	Endif
C
C	Open the file
C
	Open(66,file=filename,form="formatted",status="old",
     +			readonly,err=999)
C
C	read the file, looking for lines like : Tal: xxx.x yyy.y zzz.z zscore
C	
	write(*, 200)filename(1:strlen(filename))
200	Format("Reading points from the file:",A)
	Do while (.true.)
		Read(66,10,err=998,end=998) string
10	Format(A200)
C
C	If we get the Tal: match then parse the line
C
		If (string(1:4) .eq. 'Tal:') Then
C
			color = 1
			Read(string(5:200),*,err=100,end=120) tpoint(1),
     +				tpoint(2),tpoint(3),tpoint(4)
C
C	get the proper color
C	Black = 0
C	Colors:Black,Red,Green,Yellow,Blue,Magenta,Cyan,White,Coral,Chartre,
C		SpGreen,SlateBl,BlueVio,OrangeR
C	
C	4.00 - Red
C	3.40 - Coral
C	2.90 - Yellow
C	2.58 - Chartre
C	1.96 - SpGreen
C	0.00 - White
C	-1.96 - Cyan
C	-2.58 - SlateBl
C	-2.90 - Blue
C	-3.40 - BlueVio
C	-4.00 - Magenta
C
			If (tpoint(4) .ge. 4.0) then
				color = 1  ! Red
			Else if (tpoint(4) .ge. 3.4) then
				color = 8  ! Coral
			Else if (tpoint(4) .ge. 2.9) then
				color = 3  ! Yellow
			Else if (tpoint(4) .ge. 2.58) then
				color = 9  ! Chartre
			Else if (tpoint(4) .ge. 1.96) then
				color = 10 ! SpGreen
			Else if (tpoint(4) .le. -4.0) then
				color = 5  ! Magenta
			Else if (tpoint(4) .le. -3.4) then
				color = 12 ! BlueVio
			Else if (tpoint(4) .le. -2.9) then
				color = 4  ! Blue
			Else if (tpoint(4) .le. -2.58) then
				color = 11 ! SlateBl
			Else if (tpoint(4) .le. -1.96) then
				color = 6  ! Cyan
			Else
				color = 7  ! White
			Endif
C
C	add the point
C	
120			Continue
C
C	Add the point to the list
C
			num = num + 1
			If (num .gt. 4095) num = 4095
C
			write(*, 20)tpoint(1),tpoint(2),tpoint(3),tpoint(4)
20	Format("Adding point:",F10.3,F10.3,F10.3,F10.3)
C
C	Convert to brainvox coords
C
			Call tal_tal2point(tpoint,points(num,1),
     +				points(num,2),points(num,3))
			points(num,4) = color
		Endif
100		Continue
	Enddo

998	Close(66)
999	Return
C
	End

