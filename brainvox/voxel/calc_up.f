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
C    MODULE NAME    : calc_up.f
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
C    DESCRIPTION    :  	routines to compute the centroid and normal of the
C			user positioned plane in pixel space from the
C			fixed plane data and the trans/rots
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
	Subroutine calc_up(rots,interpixel)

	Implicit None

	Include 'rotsdef.inc'

        Record  /rotsrec/       rots

	Integer*4	I
	Real*4		transx(3),transy(3),transz(3),temp
	Real*4		interpixel
	Real*4		pv(3),pp(3)  ! vector from plane to rot centroid

	Real*4		XO,YO,ZO

	parameter	(XO = (1.36-0.5))
	parameter	(YO = (3.42-0.5))
	parameter	(ZO = (0.00))
C
C
C	Initially the translation axis coincide
C
	Do I=1,3
C
C	Translation axis
C
		transz(i) = rots.fpnorm(i)
		transx(i) = rots.fpxaxis(i)
		transy(i) = rots.fpyaxis(i)
C
C	Actual cutting plane axis
C
		rots.upnorm(i) = rots.fpnorm(i)
		rots.upxaxis(i) = rots.fpxaxis(i)
		rots.upyaxis(i) = rots.fpyaxis(i)
	Enddo
C
C	Apply the slew and tilt to the translation vectors and the
C	actual cutting plane
C
C	handle slew (for plane:use additional  for trans axis:use calib)
C
	temp = rots.slew(2) 
	temp = -temp  ! slew is recorded backwards
	Call rotaxis(rots.upyaxis,temp,rots.upnorm,rots.upnorm)
	Call rotaxis(rots.upyaxis,temp,rots.upxaxis,rots.upxaxis)
	temp = -rots.slew(1) ! calibration slew is subtracted
	temp = -temp  ! slew is recorded backwards
	Call rotaxis(transy,temp,transz,transz)
	Call rotaxis(transy,temp,transx,transx)
C
C	handle tilt (for plane:use additional  for trans axis:use calib)
C
	temp = rots.tilt(2)  ! additional tilt is OK
	Call rotaxis(rots.upxaxis,temp,rots.upnorm,rots.upnorm)
	Call rotaxis(rots.upxaxis,temp,rots.upyaxis,rots.upyaxis)
	temp = -rots.tilt(1)  ! calibration tilt is subtracted
	Call rotaxis(transx,temp,transz,transz)
	Call rotaxis(transx,temp,transy,transy)
C
C	Set the rotation centroid (along upnorm, then rotate to make pp)
C
C An earlier (incorrect) release of the code used these instead:
C	pv(i) = (48.9/interpixel)*rots.upnorm(i)
C	Call rotaxis(rots.upyaxis,temp,pv,pp)
C	Call rotaxis(rots.upxaxis,temp,pp,pp)
C	Which are wrong as the motion should be derived from the fixed plane.
C
	Do i=1,3
		pv(i) = (48.9/interpixel)*rots.fpnorm(i)
	Enddo
	temp = rots.slew(2) 
	temp = -temp  ! slew is recorded backwards
	Call rotaxis(rots.fpyaxis,temp,pv,pp)
	temp = rots.tilt(2)  ! additional tilt is OK
	Call rotaxis(rots.fpxaxis,temp,pp,pp)
C
C	Lastly add the translations from the fp centroid by adding along
C	the FIXED PLANE AXIS the uptrans values
C
C	Start at the fp center (shifted slightly by the Xoffset)
C
	rots.upcenter(1) = rots.fpcenter(1) + 
     +			(rots.fpxaxis(1)*(rots.x_off/interpixel))
	rots.upcenter(2) = rots.fpcenter(2) +
     +			(rots.fpxaxis(2)*(rots.x_off/interpixel))
	rots.upcenter(3) = rots.fpcenter(3) +
     +			(rots.fpxaxis(3)*(rots.x_off/interpixel))
C
C	X Axis
C
	Do i=1,3
		rots.upcenter(i) = rots.upcenter(i) + 
     +			(-rots.uptrans(1))*transx(i)
	Enddo
C
C	Y Axis
C
	Do i=1,3
		rots.upcenter(i) = rots.upcenter(i) + 
     +			(-rots.uptrans(2))*transy(i)
	Enddo
C
C	Z Axis
C
	Do i=1,3
		rots.upcenter(i) = rots.upcenter(i) + 
     +			(-rots.uptrans(3))*transz(i)
	Enddo
C
C	Correct the UPc for the additional slew/tilt about a point
C	that is PV from the base plane
C
	Do i=1,3
		rots.upcenter(i) = rots.upcenter(i) - pv(i)
	Enddo
C
C	now shoot it out along the rotated pp vector
C
	Do i=1,3
		rots.upcenter(i) = rots.upcenter(i) + pp(i)
	Enddo
C
C	write(*, *)"Upcent:",rots.upcenter(1),rots.upcenter(2),rots.upcenter(3)
C	write(*, *)"Upnorm:",rots.upnorm(1),rots.upnorm(2),rots.upnorm(3)
C
C	One final correction:  The PET image reconstruction matrix is
C	not centered on the laser lines.  Everything to this point
C	simulated the laser lines assuming the center of first plane 
C	is where the lasers meet.  We have found that the lasers 
C	cross at a point offset -1.36,-3.42MM from the image center.
C	Signs are swapped to correct the error, and the YO sign is
C	swapped a second time to correct for the fact that in Brainvox
C	images are flipped over the Y axis w/respect to mtrace where
C	these numbers were obtained.  The ZO number is probably 0.5
C	but is not used here as it is too difficult a measure to
C	ascertain.  The 0.5 added corrects for the center of the MP_FIT
C	matrix being at 128,128 instead of 128.5,128.5
C
	Do i=1,3
		rots.upcenter(i)=rots.upcenter(i)+(XO*rots.upxaxis(i))
C
C	add xshift value
C
		rots.upcenter(i)=rots.upcenter(i)+
     +			(rots.xshift*rots.upxaxis(i))
	Enddo
	Do i=1,3
		rots.upcenter(i)=rots.upcenter(i)-(YO*rots.upyaxis(i))
	Enddo
	Do i=1,3
		rots.upcenter(i)=rots.upcenter(i)+(ZO*rots.upnorm(i))
	Enddo
C
	Return
	End
C
C	routine to compute all the planes and save them to disk
C
	Subroutine save_planes(rots,myset,imagesize,interpixel,
     +			patinfo,what)

	Implicit None

	Include 'rotsdef.inc'
	Include 'tal_space.inc'

        Record  /rotsrec/       rots,rots_saved
	Record	/tal_conv/	saved_tal
	Integer*4		myset(*),imagesize,what
	Integer*2		Image(512*512)
	Byte			bimage(512*512)
	logical			error,found
	Integer*4		i,j,k,tmp,h
	Character*200		filename,tstr,setname
	Character*200		patinfo(3)
	Real*4			pdiff,interpixel,pt(3),ratio,dist
	Real*4			squf,tnorm(6)
	Integer*4		data(4096,4),num,dout(4096,4)
	Integer*4		in(3),out(3),err,interpolated
	Integer*4		dz_size

	Character*24		fdate
	External		fdate
	Integer*4		bv_mkdir,strlen
	External        	bv_mkdir,strlen
C
C	Make the MP_FIT subdirectory
C
        inquire(exist=error,file="MP_FIT.dir")
        filename = "MP_FIT.dir"//char(0)
	h = '000041ff'X
        if (.not. error) i = bv_mkdir(filename,h)
C
C	compute distance between planes in pixels
C
	pdiff = rots.sthickness/interpixel
C
C	Ratio from PET pixels to MRI pixels
C
	ratio = rots.ipixel/interpixel
C
C	Compute the number of interpolated slices in the resampled dataset
C
        squf = 10.0
        interpolated = -1
        do while ((squf .gt. 1.00).and.(interpolated .lt. 20))
                interpolated = interpolated + 1
                squf = (float(rots.n_slices(1))*rots.sthickness)/
     +   	 	(float(rots.n_slices(1) + 
     +			(interpolated*(rots.n_slices(1)-1)))*rots.ipixel)
        Enddo
	dz_size = rots.n_slices(1) + (interpolated*(rots.n_slices(1)-1))
C
C	Write ascii annotation file
C
	j = 200
	Do while ((rots.outname(j:j) .eq. ' ').and.(j .gt. 1))
		j = j - 1
	Enddo
	tstr = "MP_FIT.dir/"//rots.outname(1:j)
	filename = "MP_FIT.dir/"//rots.outname(1:j)//".txt"
	Inquire(file=filename,exist=found)
	If (found) Then
		open(66,file=filename,form="formatted",
     +			status="old",err=999)
	Else
		open(66,file=filename,form="formatted",
     +			status="new",err=999)
	Endif
	write(66,1,err=999) fdate(),rots.outname(1:j),
     +			rots.n_slices(1),rots.sthickness
1	Format("MP_FIT - ",A24,/,"Filenames:",A,/,I3," slices",
     +		" with a thickness of ",F8.4," mm")
	write(66,2,err=999)rots.ipixel,patinfo(1)(1:79),patinfo(2)(1:79),
     +			patinfo(3)(1:79)
	write(66,3,err=999)rots.slew(1),rots.tilt(1)
	write(66,4,err=999)rots.slew(1)+rots.slew(2),rots.tilt(1)+rots.tilt(2)
	write(66,5,err=999)rots.x_off,rots.uptrans(2)*interpixel,
     +		rots.uptrans(3)*interpixel
3	Format("Calibration: Slew:",F8.3," Tilt:",F8.3)
4	Format("Total: Slew:",F8.3," Tilt:",F8.3)
5	Format("Table: Xoff:",F8.3," mm Y:",F8.3," mm Z:",F8.3," mm")
2	Format("Interpixel spacing:",F10.5," mm",/,"Patient information:",/,
     +		">",A79,/,">",A79,/,">",A79)
C
C	Was there an AIR file adjustment?
C
	Call air_get_params(j,tnorm,filename)
	If (j .ne. 0) Then
		j = index(filename,char(0)) - 1
		write(66,500,err=999) filename(1:j)
		write(66,501,err=999) "Rotations:",tnorm(1),
     +				tnorm(2),tnorm(3)
		write(66,501,err=999) "Translations:",tnorm(4),
     +				tnorm(5),tnorm(6)
500	Format("AIR restart file:",A)
501	Format("AIR ",A,F8.3," ",F8.3," ",F8.3)
	Endif
C
C	Done
C
999	Close(66,err=998)
998	Continue
C
C	Perform AIR adjustment
C
C	Saved old
C
	rots_saved = rots
C
	Call AIR_xform(rots.upcenter,rots.upxaxis,rots.upyaxis,rots.upnorm)
C
C	Sample the planes!!
C
C	For each plane
C
	If (what .eq. 0) Then
	j = 1
	Do i=0,rots.n_slices(1)-1
C
C	Get the filename
C
		call name_changer(tstr,j,filename,error)
C
		Write(*,20) j,rots.n_slices(1)
20	Format("Computing sampled image:",I3," of ",I3)
C
C	Get the center point
C
		Do k=1,3
			pt(k)=rots.upcenter(k) +
     +				float(-i)*pdiff*rots.upnorm(k)
		Enddo
C
C	bump onto the plane
C
		Call vl_resample_axis(%val(imagesize),
     +  		%val(imagesize),Image,
     +  		rots.upxaxis(1),rots.upyaxis(1),
     +  		rots.upnorm(1),pt(1),myset,ratio) 
C
C	Convert to byte image
C
		Do k=1,imagesize*imagesize
			tmp = image(k) - 256
			if (tmp .ge. 128) tmp = tmp - 256
			bimage(k) = tmp
		Enddo
C
C	And flip the image over the X axis
C
        	Call flipbyteimage(bimage)
C
C	write the image to disk
C
		Write(*,10) filename(1:strlen(filename))
10	Format("Writing sampled file:",A)
		Call write_image(filename,bimage,error)
C
		j = j + 1
C
	Enddo
	Endif
C
C	Resample the pointsets
C
	Do j=0,19 
C
C	For each of the pointsets
C	Read the pointset
C
		setname = " "
		num = 0   ! pre-clear the input buffer
		Call point_io(j,setname,0,data,num,'./ ')
		tmp = 0   ! accumulate the points into dout
C
C	Map the points into MP_FIT space
C
		Do k=1,num
			in(1) = data(k,1)
			in(2) = data(k,2)
			in(3) = data(k,3)
			Call vl_3dpttomrpet(%val(imagesize),
     +				%val(imagesize),%val(dz_size),
     +				rots.upxaxis(1),rots.upyaxis(1),
     +                  	rots.upnorm(1),rots.upcenter(1),
     +				myset,ratio,squf,
     +				in(1),out(1),err)	
C
C	If a transformation is found, record it
C
			If (err .eq. 0) Then
				tmp = tmp + 1
				dout(tmp,1) = out(1)
				dout(tmp,2) = out(2)
				dout(tmp,3) = out(3)
				dout(tmp,4) = data(k,4)
			Endif
		Enddo
C
C	add offset (cdist)
C
C	Write out the new points
C
		If (tmp .ne. 0) Call point_io(j,setname,1,dout,tmp,
     +			'MP_FIT.dir/ ')
C
	Enddo
C
C	Convert talairach coords (if possible)
C	1) save off current space
C
	saved_tal = tspace
C
C	if invalid, try to load from disk, if still invalid abort
C
	If (tspace.valid .eq. 0) Then
		Call tal_file_io("_talairach ",0,interpixel)
		If (tspace.valid .ne. 1) goto 1000
	Endif
C
C	2) Xform current
C		2.1) offset from the centroid (upcenter)
C	Tnorm is the centroid of the resampled space (in cur volume space)
C
	Do i=1,3
		tnorm(i) = rots.upcenter(i) + 
     +		(float(-(rots.n_slices(1)-1))/2.0)*pdiff*rots.upnorm(i)
	Enddo
	Do i=1,3
		tspace.points(i,PT_CA) = tspace.points(i,PT_CA) - 
     +			tnorm(i)
		tspace.points(i,PT_CP) = tspace.points(i,PT_CP) - 
     +			tnorm(i)
		tspace.points(i,PT_PLANE) = tspace.points(i,PT_PLANE) - 
     +			tnorm(i)
	Enddo
C
C	Since we use (-i) above, we must use -(norm) here
C
	tnorm(1) = -rots.upnorm(1)
	tnorm(2) = -rots.upnorm(2)
	tnorm(3) = -rots.upnorm(3)
C
C		2.2) apply rotations to axis vectors and points
Cpoints first
	Do i=1,3
		pt(1) = tspace.points(1,i)
		pt(2) = tspace.points(2,i)
		pt(3) = tspace.points(3,i)
		Call dotprod(pt,rots.upxaxis,dist)
		tspace.points(1,i) = dist
		Call dotprod(pt,rots.upyaxis,dist)
		tspace.points(2,i) = dist
		Call dotprod(pt,tnorm,dist)
		tspace.points(3,i) = dist
	Enddo
CXaxis
	Do i=1,3
		pt(i) = tspace.xaxis(i)
	Enddo
	Call dotprod(pt,rots.upxaxis,tspace.xaxis(1))
	Call dotprod(pt,rots.upyaxis,tspace.xaxis(2))
	Call dotprod(pt,tnorm,tspace.xaxis(3))
CYaxis
	Do i=1,3
		pt(i) = tspace.yaxis(i)
	Enddo
	Call dotprod(pt,rots.upxaxis,tspace.yaxis(1))
	Call dotprod(pt,rots.upyaxis,tspace.yaxis(2))
	Call dotprod(pt,tnorm,tspace.yaxis(3))
CZaxis
	Do i=1,3
		pt(i) = tspace.zaxis(i)
	Enddo
	Call dotprod(pt,rots.upxaxis,tspace.zaxis(1))
	Call dotprod(pt,rots.upyaxis,tspace.zaxis(2))
	Call dotprod(pt,tnorm,tspace.zaxis(3))
C
C		2.3) scaling to MM is handled automatically by the IO routine
C
C	3) save off new
C
	Call tal_file_io('MP_FIT.dir/_talairach ',1,interpixel)
C
C	4) restore saved (leave things as we found them)
C
	tspace = saved_tal
C
C	Recover pre-AIR parameters
C
1000	Continue
	rots = rots_saved
C
	Return
C
	End
C
C	Routine to draw the MP_FIT graphics overlays
C
	Subroutine draw_mp_fit(rots,myset,imagesize,interpixel,which)

	Implicit None

	Include 'rotsdef.inc'
	Include 'windows.inc'

        Record  /rotsrec/       rots
	Integer*4		myset(*),imagesize
	Integer*4		i,j,k,which,solid,h
	real*4			pdiff,interpixel,pt(12)
	logical			flag

        Real*4  rt_pnt(3),rt_norm(3)
        Real*4  re_pnt(3),re_norm(3)
        Real*4  lt_pnt(3),lt_norm(3)
        Real*4  le_pnt(3),le_norm(3)
        Real*4  p1(3),p2(3)

        Common /junkfit/rt_pnt,rt_norm,re_pnt,re_norm,
     +                  lt_pnt,lt_norm,le_pnt,le_norm
C
C	compute distance between planes in pixels
C
	pdiff = rots.sthickness/interpixel
C
C	For each plane
C
	j = 1
	Do i=0,rots.n_slices(1)-1
C
C	Get the center point
C
		Do k=1,3
			pt(k)=rots.upcenter(k) +
     +				float(-i)*pdiff*rots.upnorm(k)
		Enddo
C
C	bump onto the plane
C
		solid = 0
		If (i .ne. 0) Then
			If (i .eq. wind2d(curwin).n_slice) Then
				h = '00000080'X
				Call cpack(h)
				If (iand(which,16) .eq. 16) solid = 1
			Else 
				Call cpack(0)
			Endif
		Else 
			h = '000000FF'X
			Call cpack(h)
			If (i .eq. wind2d(curwin).n_slice) Then
				If (iand(which,16) .eq. 16) solid = 1
			Endif
		Endif
		flag = .false.
		if ((iand(which,1) .eq. 1) .and. (i .eq. 0)) flag = .true.
		if ((iand(which,2) .eq. 2) .and. (i .ne. 0)) flag = .true.
		if (i .eq. wind2d(curwin).n_slice) flag = .true.
		If (flag) then
                Call vl_mpfit_uplane(%val(imagesize),%val(imagesize),
     +                  rots.upxaxis(1),rots.upyaxis(1),
     +                  rots.upnorm(1),pt(1),myset,%val(solid))
		Endif
C
	Enddo
        k = 1
        Do i=1,4
                Do j=1,3
                        pt(k) = rots.fppoints(i,j)
                        k = k + 1
                Enddo
        Enddo
	h = '00ff0000'X
        Call cpack(h)
        if (iand(which,4) .eq. 4) Call vl_mpfit_fplane(pt(1),myset)
C
	If (iand(which,32) .eq. 32) Then
		h = '00ff0080'X
		Call cpack(h)  ! BlueViolet
		Do i=1,3
			p1(i) = rt_pnt(i) + 50*rt_norm(i)
			p2(i) = rt_pnt(i) - 50*rt_norm(i)
		Enddo
		Call vl_mpfit_line(p1,p2,myset)
		Do i=1,3
			p1(i) = re_pnt(i) + 50*re_norm(i)
			p2(i) = re_pnt(i) - 50*re_norm(i)
		Enddo
		Call vl_mpfit_line(p1,p2,myset)
		Do i=1,3
			p1(i) = lt_pnt(i) + 50*lt_norm(i)
			p2(i) = lt_pnt(i) - 50*lt_norm(i)
		Enddo
		Call vl_mpfit_line(p1,p2,myset)
		Do i=1,3
		p1(i) = le_pnt(i) + 50*le_norm(i)
			p2(i) = le_pnt(i) - 50*le_norm(i)
		Enddo
		Call vl_mpfit_line(p1,p2,myset)
	Endif
C
	Return
	End
C
C	Subroutine to setup the ortho params and draw the MP_FIT
C	graphics into the voxel image window
C
	Subroutine draw_mp_fit_voxel(rots,myset,imagesize,interpixel,
     +		which,offset,zoom)

	Implicit None

	Include 'rotsdef.inc'

        Record  /rotsrec/       rots
	Integer*4		myset(*),imagesize
	Integer*4		which,offset(2),zoom
	real*4			interpixel
C
	Integer*4		a,b
	Real*4			dx,dy
	Real*4			oth(6)
C
C	Get windowsize/2
C
	Call getsiz(a,b)
	dx = float(a)/float(zoom)
	dy = float(b)/float(zoom)
	a = 445/2
	b = 445/2
C
C	Generate the ortho parameters
C
	oth(1) = (-a) + offset(1)
	oth(2) = oth(1) + dx
	oth(3) = (-b) + offset(2)
	oth(4) = oth(3) + dy
C
C	Z depths are fixed
C
	oth(5) = -(445/2)
	oth(6) = (445/2)
C
	Call ortho(oth(1),oth(2),oth(3),oth(4),oth(5),oth(6))
C
C	draw the graphics (if needed)
C
	If (iand(which,8) .eq. 8) then
		Call draw_mp_fit(rots,myset,imagesize,interpixel,which)
	Endif
C	
	Return
	End
