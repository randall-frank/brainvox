c    ======================================================================     
C    THIS SOFTWARE IS THE PROPERTY OF:                                          
C         The Image Analysis Facility                                     
C         71 HBRF     
C         The University of Iowa, Iowa City, Iowa                               
C    ======================================================================     
C	Copyright (C) 1989 Image Analysis Facility, University of Iowa
C    ======================================================================     
C                                                                               
C    -- OVERVIEW --------------                                                 
C                                                                               
C    MODULE NAME    : fileio  
C    VERSION        : 1.0 
C    LOCATION       :  
C    LAST EDIT      :  
C    STATUS         :  (Testing,Production,Obsolete)
C    LANGUAGE       : FORTRAN
C    PACKAGE        : brainvox
C
C
C    -- CREATION --------------                                                 
C                                                                               
C    PROGRAMMER     : rjf
C    DATE           : 27 Nov 90      
C    LINE-BY-LINER  :    
C    APPROVAL       : 
C
C    DESCRIPTION    :  	this file contains routines for reading and writing
C			palettes, region lists, and patient info.
C                                                                               
C    -- REVISION HISTORY --------------                                         
C
C                                                                               
C    -- GENERAL --------------                                                  
C
C    LIMITATIONS    :      
C    NOTES          :    See individual routines
C                 
C    ====================================================================== 
C
C	routine to read a palette file
C
	Subroutine	read_pal(filename,pal,error)
C
C	INPUTS:		filename = filename to read
C	OUTPUTS:	pal = i*4(768) read RGB palette
C			error = logical (true on error)
C
	Implicit None

	logical		error
	character*(*)	filename
	integer*4	pal(768)
C
	Character*200	tfile
	Integer*4	fid,status,I
	byte		rpal(768)

	Integer*4	bv_open,bv_read,bv_close,strlen
	external	bv_open,bv_read,bv_close,strlen
C
C	assume no error
C
	error = .false.
C
C	open the file
C
	tfile = filename(1:strlen(filename))//char(0)
	fid = bv_open(tfile,"r",0)
	if (fid .eq. -1) then
		error = .true.
		return
	Endif
c
C	read the raw pallette
C
        status=bv_read(fid,rpal,768)
	if (status .ne. 768) error = .true.
C
C	close the file
C
        status=bv_close(fid)
C
C	convert into I*4
C
	Do I=1,768    ! rrr...ggg...bbb...
		pal(i) = rpal(i)
		if (pal(i) .lt. 0) pal(i) = 256 + pal(i)
	Enddo
C
C	done!
C
	return
	End
C
C	routine to write a palette
C
	Subroutine	write_pal(filename,pal,error)
C
C	INPUTS:		filename = filename to read
C			pal = i*4(768) read RGB palette
C	OUTPUTS:	error = logical (true on error)
C
	Implicit None

	logical		error
	character*(*)	filename
	integer*4	pal(768)
C
	Character*200	tfile
	Integer*4	fid,status,i
	byte		rpal(768)

	Integer*4	bv_open,bv_write,bv_close,strlen
	external	bv_open,bv_write,bv_close,strlen
C
C	assume no error
C
	error = .false.
C
C	open the file
C
	tfile = filename(1:strlen(filename))//char(0)
C
C	convert from I*4
C
	Do I=1,768    ! rrr...ggg...bbb...
		if (pal(i) .ge. 128) then
			rpal(i) = pal(i)-256
		Else
			rpal(i) = pal(i)
		Endif
	Enddo
c
C	write the raw palette
C	666 octal => 438 decimal
C
        fid=bv_open(tfile,"cw",438)
        If (fid .ne. -1) then
                status=bv_write(fid,rpal,768)
                status=bv_close(fid)
	Else
		error = .true.
        Endif
C
C	done!
C
	return
	End
C
C------- rgndefs routines -------------
C
C	routine to read a region definition file
C
	subroutine	read_rgns(filename,error)
C
C	INPUTS:		filename = filename to read
C	OUTPUTS:	error = logical (true on error)
C			into common block
C

	Implicit None

	Include 'vt_inc.f'

	Character*(*)	filename
	Character*(200) tmp,tmp2
	logical		error

	logical		found
	Integer*4	I,bv_get_app_path
	External bv_get_app_path
C
C	assume OK
C
	error = .false.
C
C	is it there?
C
	inquire(exist=found,file=filename)
	if (.not. found) goto 999
	open(unit=66,file=filename,form="formatted",status="old",
     +			err=999)
C
C	read the number of entries
C
	read(66,*,err=999) numrgns
	if (numrgns .gt. max_rgns) then
		write(*, 10)max_rgns,numrgns
		numrgns = max_rgns
10	Format(" Warning, only ",I4," of ",I4," regions used.")
	Endif
C
C	read the entries
C
	do I=1,numrgns
		read(66,11,err=999) rgnnames(i),rgntypes(i)
11	Format(A40,I9)
	enddo
C
	close(66)
C
	return
C
C	error
C
999	error = .true.
	close(66)
	return

	end
C
C	routine to write a region definition file
C
	subroutine	write_rgns(filename,error)
C
C	INPUTS:		filename = filename to read
C			from common block
C	OUTPUTS:	error = logical (true on error)
C

	Implicit None

	Include 'vt_inc.f'

	Character*(*)	filename
	logical		error,found

	Integer*4	I
C
C	assume OK
C
	error = .false.
C
C	is it there?
C
	inquire(exist=found,file=filename)
	if (found) then
		open(unit=66,file=filename,form="formatted",status="old",
     +			err=999)
	Else
		open(unit=66,file=filename,form="formatted",status="new",
     +			err=999)
	Endif
C
C	write the count
C
	write(66,10,err=999) numrgns
10	Format(I9)
C
C	and the rgns
C
	do I=1,numrgns
		write(66,11,err=999) rgnnames(i),rgntypes(i)
	Enddo
11	Format(A40,I9)
C
	close(66,err=998)
C
	return
C
C	error
C
999	continue
	close(66,err=998)
998	error = .true.

	return
	end
C
C------- patientinfo routines -------------
C
C	routine to read the patientinfo file
C
	subroutine	read_patient(filename,error)
C
C	INPUTS:		filename = filename to read
C	OUTPUTS:	error = logical (true on error)
C			into common block
C

	Implicit None

	Include 'vt_inc.f'

	Character*(*)	filename
	logical		error,found
	real*4		squf,ip,is
	Integer*4	dz
C
C	assume OK
C
	error = .false.
C
C	is it there?
C
	inquire(exist=found,file=filename)
	if (.not. found) goto 999
	open(unit=66,file=filename,form="formatted",status="old",
     +			err=999)
C
C	read the stuff
C
	read(66,10,err=999) imagename
10	format(A200)
	read(66,10,err=999) patientinfo(1)
	read(66,10,err=999) patientinfo(2)
	read(66,10,err=999) patientinfo(3)
C
	read(66,*,err=999)  interpixel
	read(66,*,err=999)  interslice
C
	close(66)
C
	Call read_tagged_values(filename)
C
	open(unit=66,file="_."//filename,form="formatted",status="new",
     +			err=109)
	write(66,*,err=109) "1.0"
	close(66,err=109)
C
C	attempt to check file integrity
C
109	Call image_check(imagename,realimages,imagesize,dz,ip,is)
	write(*,100)realimages,volumedepth,imagesize,imagesize,imagename
100	Format(" Found ",I5,"x",I1," byte images of size ",I5,"x",I5,
     +		" using template:",/,A200)
C
C	should set the number of interpolated images via the interpixel
C	and interslice spacings
C
	if (interpixel .le. 0) then
		interpixel = 1.0
		write(*, 101)
101	Format(" Warning, interpixel spacing invalid. Setting to 1.0")
	Endif
C
C	More sanity checks...
C
	if (interpixel .ne. ip) then
		write(*,*) "Warning: file interpixel spacing:",ip,
     +			" does not match _patient value:",interpixel
	endif
	if (interslice .ne. is) then
		write(*,*) "Warning: file interslice spacing:",is,
     +			" does not match _patient value:",interslice
	endif
	if (dz .ne. volumedepth) then
		write(*,*) "Warning: file voxel size:",dz,
     +			" does not match _patient value:",volumedepth
	endif
C
C	start with zero interpolated
C
	if (realimages .lt. 2) then
		interpimages = 0
		return
	endif
	squf = 10.0
	interpimages = -1
	do while ((squf .gt. 1.00).and.(interpimages .lt. 20))
		interpimages = interpimages +1
        	squf = (float(realimages)*interslice)/
     +    (float(realimages + (interpimages*(realimages-1)))*interpixel)
	Enddo
C
	squeezefactor = squf
C
C	add interpolated slices until squf .le. 1.0
C
C	interpimages = jnint(((interslice/interpixel)-1.0)+0.5)
C
C	just in case the interpixel is far greater than interslice
C
	if (interpimages .lt. 0) interpimages = 0
	if (interpimages .gt. 20) then
		write(*, 102)
102	Format(" Warning, number of interpolated images very high.",/,
     +	       " Please check the interpixel and interslice distances.",/,
     +	       " Interpolation set to 3.") 
		interpimages = 3
	Endif
C
	return
C
C	error
C
999	error = .true.
	close(66)

	return
	end
C
C	routine to write the patientinfo file
C
	subroutine	write_patient(filename,error)
C
C	INPUTS:		filename = filename to read
C			from common block
C	OUTPUTS:	error = logical (true on error)
C

	Implicit None

	Include 'vt_inc.f'

	Character*(*)	filename
	logical		error,found
	Character*200	dum(10)
	Integer*4	i,j
C
C	assume OK
C
	error = .false.
C
C	is it there?
C
	inquire(exist=found,file=filename)
	if (found) then
C
C	attempt to read lines past the normal stuff
C
		open(unit=66,file=filename,form="formatted",status="old",
     +			err=999)
		Do i=1,6
			read(66,10,err=100,end=100) dum(1)
		Enddo
C	Read until error 
		i = 0
		Do while (i .lt. 10)
			read(66,10,err=100,end=100) dum(i+1)
			i = i + 1
		Enddo
100		close(66)
		open(unit=66,file=filename,form="formatted",status="old",
     +			err=999)
	Else
		open(unit=66,file=filename,form="formatted",status="new",
     +			err=999)
		i = 0
	Endif
C
C	write the stuff
C
	write(66,10,err=999) imagename
10	format(A200)
	write(66,10,err=999) patientinfo(1)
	write(66,10,err=999) patientinfo(2)
	write(66,10,err=999) patientinfo(3)
C
	write(66,11,err=999)  interpixel
	write(66,11,err=999)  interslice
11	Format(f15.8)
C
C	write out the preserved appended lines
C
	Do j=1,i
		write(66,10,err=999) dum(j)
	Enddo
C
	close(66,err=998)
C
	Call write_tagged_values(filename)
C
	open(unit=66,file="_."//filename,form="formatted",status="new",
     +			err=109)
	write(66,*,err=109) "1.0"
	close(66,err=109)
C
109	return
C
C	error
C
999	continue
	close(66,err=998)

998	error = .true.

	Call write_tagged_values(filename)

	return
	end
C
C	Routine to read any PET data added to the _patients file
C
	subroutine	read_patient_pet(filename,inj_temp,pet_min,
     +				pet_max,pet_avg,error)
C
C	INPUTS:		filename = filename to read
C	OUTPUTS:	error = logical (true on error)
C
	Implicit None

	Include 'vt_inc.f'

	Character*(*)	filename,inj_temp
	Integer*4	pet_min,pet_max,pet_avg
	logical		error,found
	real*4		squf
	Character*200	junk_str
C
C	assume OK
C
	error = .false.
C
C	is it there?
C
	inquire(exist=found,file=filename)
	if (.not. found) goto 999
	open(unit=66,file=filename,form="formatted",status="old",
     +			err=999)
C
C	read and skip the normal stuff
C
	read(66,10,err=999) junk_str ! image template
10	format(A200)
	read(66,10,err=999) junk_str ! patinfo1
	read(66,10,err=999) junk_str ! patinfo2
	read(66,10,err=999) junk_str ! patinfo3
C
	read(66,10,err=999) junk_str ! islice
	read(66,10,err=999) junk_str ! ipixel
C
C	Read the PET data
C
	read(66,10,err=999,end=999) inj_temp
	read(66,*,err=999,end=999) pet_min
	read(66,*,err=999,end=999) pet_max
	read(66,*,err=999,end=999) pet_avg
C
	close(66)
	return
C
999	error = .true.
	close(66)
	return
	end
C
C---------- check images ----------
C
C	This routine counts the number of images available
C	and determines their size
C
	Subroutine	image_check(tempname,num,size,dz,ip,is)
C
C	INPUTS:		tempname = filename template
C	OUTPUTS:	num,size = I*4 number of images and side length
C
	
	Implicit None
	
	Include 'vt_inc.f'

	Integer*4	num,size
	character*(*)	tempname

	Integer*4	I,ndy,dz,flip,slice
	Real*4		ip,is
	Character*200	tname,zname
	logical		error,found

	Integer*4	status,strlen,bv_nifti_io
	External strlen,bv_nifti_io
C
C	try nifti-1 (query)
C
	If (bv_nifti_io(tempname(1:strlen(tempname)),2,0,
     +		size,ndy,num,slice,dz,flip,ip,is) .eq. 0) Then
		return
	Endif
C
C	try filenames starting at 1
C	
	ip = interpixel
	is = interslice
	dz = volumedepth
	i=0
	found = .true.
	Do while (found)
		i = i + 1
		Call name_changer(tempname,i,tname,error)
		if (error) then
			found = .false.
		Else
			inquire(exist=found,file=tname)
		Endif
	Enddo
C
C	there are i-1 files available
C
	num = i - 1
	size = 256
	If (num .lt. 1) return
	If (num .gt. max_images) then
		 num = max_images
		 write(*,*)"Warning, only ",max_images," will be used."
	Endif
C
C	read the first one and check its size
C
	Call name_changer(tempname,1,tname,error)
	zname = tname(1:strlen(tname))//char(0)
C
	Call size_bin_file(zname,size,status)
	If (status .eq. 0) Then
		size = size / volumedepth
		size = size**0.5
	Else 
		size = 0
	Endif
C
	Return
	End

C
C	routine to read an image file 
C
	Subroutine	read_image(filename,img,error)
C
C	INPUTS:		filename = filename to use
C	OUTPUTS:	error = logical (true if error occurred)
C			img = array of bytes at least (imagesize*imagesize)
C

	Implicit None
	Include 'vt_inc.f'

	logical		error
	character*(*)	filename
	byte		img(*)
	Integer*2	img2(512*512)
	Real*4		imgf(512*512)
	Real*4		d
C
	Character*200	tfile
	Integer*4	status,I,isize
	Integer*4	trace(4096,2)
	Integer*4	strips(8192),ptr,x,y,jj,num
	Real*4		fact
	Integer*4 strlen
	External strlen
C
C	image size
C
	isize = imagesize*imagesize
C
C	assume error
C
	error = .true.
C
C	Check for ROI
C
	If (index(filename,'.roi ') .ne. 0) Then
C
C	read the ROI
C

		Call read_roi_file(filename,trace,num,error)
		If (error) return
C
C	scale the ROI
C
		fact = 512.0/float(imagesize)
		Do i=1,num
			trace(i,1) = trace(i,1)/fact
			if (trace(i,1) .lt. 0) trace(i,1)=0 
			if (trace(i,1) .gt. imagesize-1) trace(i,1)=imagesize-1
			trace(i,2) = trace(i,2)/fact
			if (trace(i,2) .lt. 0) trace(i,2)=0 
			if (trace(i,2) .gt. imagesize-1) trace(i,2)=imagesize-1
		Enddo
C
C       call polyfill
C
        	ptr=8192
        	Call polyfill(trace,num,strips,ptr)
        	if (ptr .le. -1) then
                	write(*, 13)
13      Format("Unable to complete roi fill operation.")
                	return
        	Endif
C
C	Clear the image
C
		Do i=1,imagesize*imagesize
			img(i) = 0
		Enddo
C
C	Paint the image
C
		Do i=1,ptr-2,3
C
C       and each pixel  (note: images from shared mem have been flipped)
C
                	Y=(imagesize-1) - strips(i+2)
                	Do X=strips(i),strips(i+1)
				jj = y*imagesize+x
				img(jj+1) = 255

			Enddo
		Enddo

		error = .false.

		Return
	Endif
C
C	open the file
C
	tfile = filename(1:strlen(filename))//char(0)
C
	If (volumedepth .eq. 1) Then
		Call read_bin_file(tfile,%ref(img(1)),isize,status)
	Else if (volumedepth .eq. 2) Then
		isize = isize*2
		Call read_bin_file(tfile,%ref(img2(1)),isize,status)
		Call cnvt_16_to_8(img2,img,imagesize,imagesize,
     +			volume_lims,volume_bswap)

	Else if (volumedepth .eq. 4) Then
		isize = isize*4
		Call read_bin_file(tfile,%ref(imgf(1)),isize,status)
		Call cnvt_f_to_8(imgf,img,imagesize,imagesize,
     +			volume_lims,volume_bswap)

	Endif
C
        If (status .eq. 0) error = .false.
C
	Return
	End
C
C	routine to write an image file 
C
	Subroutine	write_image(filename,img,error)
C
C	INPUTS:		filename = filename to use
C			img = array of bytes at least (imagesize*imagesize)
C	OUTPUTS:	error = logical (true if error occurred)
C

	Implicit None
	Include 'vt_inc.f'

	logical		error
	character*(*)	filename
	byte		img(*)
C
	Character*200	tfile
	Integer*4	status,I,isize,strlen
	External strlen
C
C	image size
C
	isize = imagesize*imagesize
C
C	assume error
C
	error = .true.
C
C	open the file
C
	tfile = filename(1:strlen(filename))//char(0)
C
	Call write_bin_file(tfile,%ref(img(1)),isize,status)
C
        If (status .eq. 0) error = .false.
C
	Return	
	End
C
C       routine to read a generic binary file into an array
C
        Subroutine      readbinfile(filename,lut,size,error)

        Implicit None

        Character*(*)   filename
        Character*150   fname
        Integer*4       status,i,size,strlen
        byte            lut(*)
	External	strlen

        logical         error
C
C       assume Bad
C
        error = .true.
C
C       get filename
C
        fname = filename
        i=index(fname,char(0))
        if (i .eq. 0) then
                status = strlen(fname)
                fname(status:status)=char(0)
        Endif
	Call read_bin_file(fname,%ref(lut(1)),size,status)
C
        If (status .eq. 0) error = .false.
C
        Return
        End
C
C       routine to write a generic binary file file
C
        Subroutine      writebinfile(filename,lut,size,error)

        Implicit None

        Character*(*)   filename
        Character*150   fname
        Integer*4       status,i,size,strlen
        byte            lut(*)
	External	strlen

        logical         error
C
C       assume Bad
C
        error = .true.
C
C       get filename
C
        fname = filename
        i=index(fname,char(0))
        if (i .eq. 0) then
                status = strlen(fname)
                fname(status:status)=char(0)
        Endif
	Call write_bin_file(fname,%ref(lut(1)),size,status)
C
        If (status .eq. 0) error = .false.
C
        error = .false.
C
        Return
        End   

C
C	Routine to read tagged data values from a text file
C
	Subroutine read_tagged_values(filename)
	
	Implicit None

	Include 'vt_inc.f'
	Include 'tag_inc.f'
	
	Character*(*) 	filename
	Integer*4	err
	Character*256	tstr
	Integer*4	i,strlen
	External	strlen
	Record /FTag/	tags(20)

	i = strlen(filename)
	tstr = filename(1:i)//".tag"//char(0)
C
C	I=i*4,F=R*4,W=word,S=string
C
	i = 1
	tags(i).name = "I_BV_MRIDEPTH"//char(0)
	Call get_ref(volumedepth,tags(i).data)
	i = i + 1
	tags(i).name = "F_BV_MRI_MIN"//char(0)
	Call get_ref(volume_lims(1),tags(i).data)
	i = i + 1
	tags(i).name = "F_BV_MRI_MAX"//char(0)
	Call get_ref(volume_lims(2),tags(i).data)
	i = i + 1
	tags(i).name = "I_BV_ZSWAP"//char(0)
	Call get_ref(volume_zswap,tags(i).data)
	i = i + 1
	tags(i).name = "I_BV_BSWAP"//char(0)
	Call get_ref(volume_bswap,tags(i).data)
	
	Call read_tagged_file(tstr,i,tags,err)

	Return
	End
C
C	Routine to append tagged data values to a text file
C
	Subroutine write_tagged_values(filename)
	
	Implicit None

	Include 'vt_inc.f'
	Include 'tag_inc.f'

	Character*(*) 	filename
	Character*256	tstr,note
	Integer*4	i,err,update,strlen
	Record /FTag/	tags(20)

	i = strlen(filename)
	tstr = filename(1:i)//".tag"//char(0)

	i = 1
	tags(i).name = "I_BV_MRIDEPTH"//char(0)
	Call get_ref(volumedepth,tags(i).data)
	i = i + 1
	tags(i).name = "F_BV_MRI_MIN"//char(0)
	Call get_ref(volume_lims(1),tags(i).data)
	i = i + 1
	tags(i).name = "F_BV_MRI_MAX"//char(0)
	Call get_ref(volume_lims(2),tags(i).data)
	i = i + 1
	tags(i).name = "I_BV_ZSWAP"//char(0)
	Call get_ref(volume_zswap,tags(i).data)
	i = i + 1
	tags(i).name = "I_BV_BSWAP"//char(0)
	Call get_ref(volume_bswap,tags(i).data)

	note = "Brainvox_patient"//char(0)
	update = 1
	Call write_tagged_file(tstr,note,update,i,tags,err)

	Return
	End
C
C	Routine to convert filenames in case they are swapped
C

	Subroutine get_slice_num(in,out)

	Implicit None

	Include 'vt_inc.f'

	Integer*4	in,out

	If (volume_zswap .eq. 0) Then
		out = in
	Else	
		out = realimages - in + 1
	Endif

	Return
	End
C
C	Routine to read an ROI
C

	Subroutine read_roi_file(filename,list,num,error)

	Implicit None

	Character*(*)	filename
	Integer*4	list(4096,2)
	Integer*4	num
	Logical		error

	Integer*4	i
	Logical		found

	error = .true.
	num = 0

	inquire(file=filename,exist=found)	
	if (.not. found) return
C
C	Open the file
C
	Open(66,file=filename,form='formatted',status='old',
     +		err=999,readonly)
C
C       read number of points..
C
        read(66,*,err=999,end=998) num
C
	num=num/2
C
        Do i=1,num
                read(66,*,err=999,end=998) list(i,1)
                read(66,*,err=999,end=998) list(i,2)
        Enddo
C
        close(66)
C
C	Done
C
	error = .false.

        Return
C
998	close(66)
999     num=0

	Return
	End
C
C   Get a fortran string length
C
	Integer*4       function        slength(s,l)

	Implicit None

	Character*(*)   s
	Integer*4       l,i

	slength = 0

	i = l
	Do while (i .gt. 0)
                If (s(i:i) .ne. ' ') Then
                        slength = i
                        return
                Endif
                i = i - 1
	Enddo

	Return
	End
C
C  Or implicitly...
C
        integer*4       function strlen(s)

        Implicit None

        character*(*)   s
        integer*4       i
C
C       assume total string length
C
        i = len(s)
C
C       walk backward through string to first non-space character
C
        Do while(((s(i:i) .eq. ' ').or.(s(i:i) .eq. char(0)))
     +			.and.(i .gt. 1))
                i = i - 1
        Enddo
C
        strlen = i
        return
        end
C
C	Get a positional argument, but remove quotes...
C
	Subroutine getarg_nq(n,s)

	Implicit None

	Character*(*)	s
	Character*1000	tmp
	Integer*4	n,strlen,i,j

	External	strlen

	Call getarg(n,s)

	If ((s(1:1) .eq. "'") .or. (s(1:1) .eq. '"')) Then
		tmp = s
		Do i=1,strlen(s)
			s(i:i) = ' '
		Enddo
		i = 2
		j = 1
		Do while((tmp(i:i) .ne. tmp(1:1)).and.(i .lt. len(s)))
			s(j:j) = tmp(i:i)
			i = i + 1
			j = j + 1
		Enddo
	Endif

	Return
	End

