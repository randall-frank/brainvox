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
C    MODULE NAME    : measure 
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
C    DATE           : 24 Jun 91       
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
	Subroutine init_measure

	Implicit None

	Include 'measure.inc'

	Integer*4	I
	
	Do I=1,max_linedata
		linedata.data(i) = 0.0
	Enddo
	linedata.color = 2

	Return
	End
C
C	bump to next layer
C
	Subroutine next_meas_layer

	Implicit None

	Include 'measure.inc'

	data_layer = data_layer + 1
	If (data_layer .gt. 1) data_layer = 0

	Return
	End
C
C	routine to calc the data
C
	Subroutine do_measure(points,numpts,pts2,numpts2,
     +		myset,dialog,sa_flag)

	Implicit None

        Include 'sgiuif.h'
        Include '../libs/vt_inc.f'

	Include 'voxeluif.inc'
	Include 'measure.inc'

	Integer*4	I,points(4096,4),numpts,myset(*)
	Integer*4	zdx,zdy,pts2(4096,4),numpts2
	Integer*4	zbuff_ptr,image_ptr
        Record /ditem/  dialog(*)
	Real*4		dx,dy,dz,squeeze,dist
	logical		flag,flag2
	Integer*4	sa_flag,pixel
	Real*4		v1(3),v2(3),d1,d2,ang,fval(6)


C	type *,"do_measure:",numpts,numpts2,sa_flag
C
C	init to zero
C
	Do I=1,max_linedata
		linedata.data(i) = 0.0
	Enddo
        Call vl_getsqu(squeeze,myset)
	Call vl_getbufs(image_ptr,zbuff_ptr,zdx,zdy,myset)
C
	If ((numpts .eq. 1).and.
     +		(points(numpts,4) .eq. linedata.color)) Then
		linedata.data(4) = points(numpts,1)*interpixel
		linedata.data(5) = points(numpts,2)*interpixel
		linedata.data(6) = points(numpts,3)*squeeze*interpixel
		If (pointtype .eq. 3) Then
		Call point2disk(points(numpts,1),points(numpts,2),
     +			points(numpts,3),linedata.data(7))
		Else
		Call tal_point2tal(points(numpts,1),points(numpts,2),
     +			points(numpts,3),linedata.data(7))
		Endif
	Endif
C
	If ((numpts .gt. 0).and.
     +       	(points(numpts,4) .eq. linedata.color)) Then
		call vl_getvoxel(myset,%val(points(numpts,1)),
     +			           %val(points(numpts,2)),
     +			           %val(points(numpts,3)),%val(0),pixel)
		linedata.data(13) = pixel
		call vl_getvoxel(myset,%val(points(numpts,1)),
     +			           %val(points(numpts,2)),
     +			           %val(points(numpts,3)),%val(1),pixel)
		linedata.data(14) = pixel
	Endif
C
	flag = .true.  ! flag turns false when the first linear line is found
	flag2= .true.  ! flag2 turns false when the first angle is found
C
	Do I=numpts,2,-1
C
C	is the segment the correct color?
C
		If ((points(i,4) .eq. linedata.color).and.
     +			(points(i-1,4) .eq. linedata.color)) then
C
C	Compute the distance
C
			dx = points(i,1) - points(i-1,1)
			dy = points(i,2) - points(i-1,2)
			dz = points(i,3) - points(i-1,3)
			dz = dz * squeeze
			dist = sqrt(dx*dx + dy*dy + dz*dz)
			dist = dist * interpixel
C
C	last angle? (next point past last segment 
C
			If ((flag .eq. .false.).and.
     +				(flag2 .eq. .true.)) Then
				v1(1) = points(i+1,1) - points(i,1)
				v1(2) = points(i+1,2) - points(i,2)
				v1(3) = points(i+1,3) - points(i,3)
				v1(3) = v1(3) *squeeze
				v2(1) = points(i-1,1) - points(i,1)
				v2(2) = points(i-1,2) - points(i,2)
				v2(3) = points(i-1,3) - points(i,3)
				v2(3) = v2(3) *squeeze
				ang = v1(3)*v2(3)+v1(1)*v2(1)+v1(2)*v2(2)
				d1 = sqrt(v1(1)*v1(1) +  v1(2)*v1(2) +
     +					v1(3)*v1(3))
				d2 = sqrt(v2(1)*v2(1) +  v2(2)*v2(2) +
     +					v2(3)*v2(3))
				if (d1 .le. 0.0) d1 = 1.0
				if (d2 .le. 0.0) d2 = 1.0
				ang = ang / (d1*d2)
				If (abs(ang-(-1.)) .lt. 0.0001) Then
					ang = 3.14159
				Else 
					ang = acos(ang)
				Endif
				linedata.data(10)=(ang/3.14159)*180.0
				flag2 = .false.
			Endif
C
C	is is the last segment?
C
			If (flag) then
				linedata.data(2) = dist
				linedata.data(4) = points(i,1)*interpixel
				linedata.data(5) = points(i,2)*interpixel
				linedata.data(6) = points(i,3)*squeeze*
     +					interpixel
				If (pointtype .eq. 3) Then
				Call point2disk(points(i,1),
     +					points(i,2),points(i,3),
     +					linedata.data(7))
				Else
				Call tal_point2tal(points(i,1),
     +					points(i,2),points(i,3),
     +					linedata.data(7))
				Endif
				flag = .false.
			Endif
C
C	add to length
C
			linedata.data(3) = linedata.data(3) + dist
C
		Endif
	Enddo
C
C	Calc Surface area
C
	If (sa_flag .ne. 0) Then

		If ((zbuff_ptr .ne. 0) .and. (image_ptr .ne. 0)) then
		    Call calc_sa(pts2,numpts2,linedata.color,%val(zbuff_ptr),
     +			%val(image_ptr),zdx,zdy,linedata.data(1),sa_flag)
		Endif
		Call fit_cutting_plane(linedata.color,points,numpts,
     +			myset,fval)  !1=error 2=area
		linedata.data(11) = fval(1)
		linedata.data(12) = fval(2)
	Endif
C
C	convert from pixels to UNITS squared
C
	linedata.data(1) = linedata.data(1) * interpixel * interpixel
	linedata.data(12) = linedata.data(12) * interpixel * interpixel
	linedata.data(11) = linedata.data(11) * interpixel
C
C	fill out the dialog
C
	Do i=0,4
	    dialog(measuretext+i).text = "                         "
	Enddo
C
C  0
C  4
C  3
C  2
C  1
C
	If (data_layer .eq. 0) Then

	    write(unit=dialog(measuretext+3).text,fmt=10) linedata.data(1)
10	Format("Area:",F14.1," mm2")
		dialog(measuretext+3).tlength = 23
	    write(unit=dialog(measuretext+1).text,fmt=20) linedata.data(10)
20	Format("Angl:",F6.1)
		dialog(measuretext+1).tlength = 11
	    write(unit=dialog(measuretext+2).text,fmt=30) linedata.data(3),
     +		linedata.data(2)
30	Format("Tot:",F7.1," Last:",F6.1,"mm")
		dialog(measuretext+2).tlength = 25
	    write(unit=dialog(measuretext).text,fmt=40) linedata.data(4),
     +		linedata.data(5),linedata.data(6)
40	Format("Pos:",F7.1,F7.1,F7.1)
	    dialog(measuretext).tlength = 25
	    If (pointtype .eq. 3) Then
		write(unit=dialog(measuretext+4).text,fmt=50) "Dsk:",
     +			linedata.data(7),linedata.data(8),linedata.data(9)
	    Else
		write(unit=dialog(measuretext+4).text,fmt=50) "Tal:",
     +			linedata.data(7),linedata.data(8),linedata.data(9)
50	Format(A4,F7.1,F7.1,F7.1)
	    Endif
	    dialog(measuretext+4).tlength = 25
	Elseif (data_layer .eq. 1) Then
	    write(unit=dialog(measuretext).text,fmt=60) linedata.data(12),
     +		linedata.data(11)
	    dialog(measuretext).tlength = 25
60      Format("2DA:",F9.1," Err:",F5.1,"mm")
	    write(unit=dialog(measuretext+4).text,fmt=70) linedata.data(13),
     +		linedata.data(14)
	    dialog(measuretext+4).tlength = 23
70      Format("Voxel:",F4.0," Texture:",F4.0)
	Endif
C
	Return
	End

	Subroutine fit_cutting_plane(col,points,numpts,myset,fval)

	Implicit None
        Include 'sgiuif.h'
        Include '../libs/vt_inc.f'

	Include 'voxeluif.inc'
	Include 'measure.inc'

	Real*8		verts(4096*3)
	Real*8		error,params(4),area  ! params(x,y,z,D)
	Integer*4	nverts,i,col,inum
	Integer*4	points(4096,4),numpts,myset(*)
	Real*4		squeeze,fval(*)

        Call vl_getsqu(squeeze,myset)
	inum = realimages + (realimages-1)*interpimages
	area = 0.0

	nverts = 0
	Do i=numpts,1,-1
		If (points(i,4) .eq. col) Then
			verts(1+nverts*3)=points(i,1)-(0.5*imagesize)
			verts(2+nverts*3)=points(i,2)-(0.5*imagesize)
			verts(3+nverts*3)=(points(i,3)-(0.5*inum))*squeeze
			nverts = nverts + 1
		Endif
	Enddo

	if (nverts .lt. 3) return

	Call fit_plane(verts,%val(nverts),error,params,area)

	fval(1) = error
	fval(2) = area
	fval(3) = params(1)
	fval(4) = params(2)
	fval(5) = params(3)
	fval(6) = params(4)
	
	Return
	End
