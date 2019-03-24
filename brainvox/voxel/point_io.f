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
C    MODULE NAME    : point_io.f
C    VERSION        : 1.0 
C    LOCATION       :  
C    LAST EDIT      :  
C    STATUS         :  (Development,Testing,Production,Obsolete)
C    LANGUAGE       : FORTRAN
C    PACKAGE        : sgitrace
C
C
C    -- CREATION --------------                                                 
C                                                                               
C    PROGRAMMER     : rjf
C    DATE           : 20 Sep 89       
C    LINE-BY-LINER  :    
C    APPROVAL       : 
C
C    DESCRIPTION    :  	a routine to read/save a set of 3d points
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

	Subroutine set_point_name(setnum,setname,prefix)

	Implicit None

        Integer*4       setnum
        Character*(*)   setname,prefix
	Integer*4	data(4096,4),num,h
	Character*255	tmp

	num = 0
	h = '00001000'X
	Call point_io(setnum,tmp,h,data,num,prefix)
	h = '00001001'X
	Call point_io(setnum,setname,h,data,num,prefix)

        Return
        End

	Subroutine get_point_name(setnum,setname,prefix)

	Implicit None

	Integer*4	setnum
	Character*(*)	setname,prefix
	Integer*4	data(4096,4),num,h

	setname(1:) = " "
	num = 0
	h = '00001000'X
	Call point_io(setnum,setname,h,data,num,prefix)

	Return
	End
C
	Subroutine point_io(setnum,setname,cmd,data,num,prefix)

	Implicit None

	Integer*4	setnum,num
	Integer*4 	cmd,data(4096,4)
	logical		error
	Character*255	filename,tmp
	Integer*4	i,j,k,h,strlen
	External strlen
	Character*(*)	prefix,setname
C
C	build filename
C	
	Call name_changer("pointset_%%% ",setnum,tmp,i)
	if (i .ne. 0) return
C
C	add prefix
C
	filename=prefix(1:strlen(prefix))//tmp(1:strlen(tmp)+1)
C
C	if first char is a space, use default name
C
	If (setname(1:1) .eq. ' ') then
		Write(setname,1) setnum
1	Format("Point set ",I2.2)
	Endif
C
C	case of read (cmd=0) or write (cmd=1)
C
	if (iand(cmd,255) .eq. 0) then
C
C	Read in points from file
C
		Inquire(exist=error,file=filename)
		If (error .eq. .false.) then
			h = '00001000'X
			If (iand(cmd,h) .eq. 0) Then
				write(*, 500)filename(1:strlen(filename)) 
500	Format("Point set ",A," is undefined")
			Endif
			Return	
		Endif
		Open(66,file=filename,form="formatted",status="old",err=990)
		Read(66,2,err=999,end=999) tmp
2	Format(A200)
		Read(tmp,*,err=999,end=999) i
		If ((num + i).gt.4093) Then
			close(66)
			write(*,*)"Too many points defined to load the points"
			write(*,*)"clear some current points, and try again"
			return
		Endif
		j = index(tmp,':')
		If (j .ne. 0) then
			k = 200
			Do while(tmp(k:k) .eq. ' ')
				k = k - 1
			Enddo
			if (k .ge. j+1) setname = tmp(j+1:k)
		Endif
C
C	Read the points
C
		Do j=1,i
			num = num+1
			Read(66,*,err=999,end=999) data(num,1),
     +				data(num,2),data(num,3),data(num,4)
		Enddo
		Close(66)
		h = '00001000'X
		If (iand(cmd,h) .eq. 0) Then
			write(*, 501)i,filename(1:strlen(filename)) 
501	Format(I5," points read from the point set:",A)
		Endif
C
	Else if (iand(cmd,255) .eq. 1) then
C
C	write points to a file
C
		Inquire(exist=error,file=filename)
		If (error) then
		Open(66,file=filename,
     +			form="formatted",status="old",err=991)
		Else
		Open(66,file=filename,
     +			form="formatted",status="new",err=991)
		Endif
		j = 200
		Do while (setname(j:j) .eq. ' ')
			j = j - 1
		Enddo
		Write(66,10,err=998) num,setname(1:j)
10	Format(I9," :",A)
20	Format(4I9)
C
C	write the points
C
		Do j=1,num
			write(66,20,err=998) data(j,1),data(j,2),
     +				data(j,3),data(j,4)
		Enddo
		Close(66,err=991)
		h = '00001000'X
		If (iand(cmd,h) .eq. 0) Then
			write(*, 502)num,filename(1:strlen(filename)) 
502	Format(I5," points written into the point set:",A)
		Endif
	Endif
C
	Return
C
C	Errors
C
990	write(*, *)"Error : Unable to open a pointset file for reading."
	write(*, *)"        The operation was not completed as requested."
	Close(66,err=1000)

	Return

991	write(*, *)"Error : Unable to open a pointset file for writing."
	write(*, *)"        The operation was not completed as requested."
	Call bv_error("Unable to write data into a pointset file|"//
     +		"Check file permissions and disk space}",0)
	Close(66,err=1000)

	Return
C
999	write(*, *)"Error : Unable to read data from a pointset file."
	write(*, *)"        The operation was not completed as requested."
	Close(66,err=1000)

	Return

998	write(*, *)"Error : Unable to write data into a pointset file."
	write(*, *)"        The operation was not completed as requested."
	Call bv_error("Unable to write data into a pointset file|"//
     +		"Check file permissions and disk space}",0)
	Close(66,err=1000)
C
1000	Return

	End
