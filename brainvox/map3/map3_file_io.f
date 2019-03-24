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
C    MODULE NAME    : map3_file_io
C    VERSION        : 1.0 
C    LOCATION       :  
C    LAST EDIT      :  
C    STATUS         :  (Development,Testing,Production,Obsolete)
C    LANGUAGE       : FORTRAN
C    PACKAGE        : brainvox_map3
C
C
C    -- CREATION --------------                                                 
C                                                                               
C    PROGRAMMER     : rjf
C    DATE           : 1 Mar 96        
C    LINE-BY-LINER  :    
C    APPROVAL       : 
C
C    DESCRIPTION    :  	Routines to read/write the text files needed by
C			MAP3
C				1) Tag file R/W for setups
C				2) Parsing of external values files
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
C	Read/Write a settings file ".m3set"
C
	Subroutine MAP3_settings_IO(filename,cmd,err,map)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'  
	Include '../libs/vt_inc.f'
        Include '../libs/vt_protos.f'
        Include 'map3uif.inc'
        Include 'map3.inc'

	Integer*4       cmd,err
	Character*256   filename,tstr
	Integer*4       i,j,k
	Record /MAP3/	map
	logical		found
C
C	Check for previous existance
C
	Inquire(file=filename,exist=found)
C
C	One error case
C
	If ((.not. found) .and. (cmd .eq. IPC_READ)) Then
		Call bv_error("Unable to open the settings file.|"//
     +			"Check file permissions.}",0)

		Return
	Endif
C
C	Open the file
C
	If (found) Then
	    Open(66,file=filename,form='formatted',status='old',err=999)
	Else
	    Open(66,file=filename,form='formatted',status='new',err=999)
	Endif
C
C	Read/Write the data
C
	If (cmd .eq. IPC_READ) Then
C
C	Read the file
C
		read(66,5,err=997,end=997) tstr
		If (tstr .ne. "MAP3_Settings_file") Then
			Close(66)
			Call bv_error("Invalid MAP-3 settings file.}",0)
			Return
		Endif
C
		read(66,*,err=998,end=998) map.n_image
		read(66,*,err=998,end=998) map.offset
		read(66,*,err=998,end=998) map.exp
		read(66,*,err=998,end=998) map.num
		Do I=1,map.num
			read(66,5,err=998,end=998) map.roi(i)
			read(66,5,err=998,end=998) map.dir(i)
			read(66,*,err=998,end=998) map.value(i)
			read(66,*,err=998,end=998) map.arb_mode(i)
		Enddo
C
	Elseif (cmd .eq. IPC_WRITE) Then
C
C	Write the file
C
		write(66,5,err=998) "MAP3_Settings_file"
		write(66,10,err=998) map.n_image
		write(66,20,err=998) map.offset
		write(66,20,err=998) map.exp
		write(66,10,err=998) map.num
		Do I=1,map.num
			write(66,5,err=998) map.roi(i)
			write(66,5,err=998) map.dir(i)
			write(66,20,err=998) map.value(i)
			write(66,10,err=998) map.arb_mode(i)
		Enddo
	Endif
C
5	Format(A)
10	Format(I)
20	Format(F15.5)
C
C	Done
C
	Close(66)
C
	Return
C
C	Errors
C
997	Close(66)
C
	Call bv_error("Unable to read the MAP-3 settings file.|"//
     +			"Check disk space and file permissions.}",0)
C
	Return
C
998	Close(66)
C
	Call bv_error("Unable to write the MAP-3 settings file.|"//
     +			"Check disk space and file permissions.}",0)
C
	Return
C
999	Continue
C
	Call bv_error("Unable to open the MAP-3 settings file.|"//
     +			"Check disk space and file permissions.}",0)
C
	Return
	End
C
C	Routine to read a values file
C
	Subroutine MAP3_external_values(filename,map)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'  
	Include '../libs/vt_inc.f'
        Include '../libs/vt_protos.f'
        Include 'map3uif.inc'
        Include 'map3.inc'

	Character*256	filename
	Record /MAP3/	map

	logical		found

	Character*256	line,idnum
	Real*4		v
	Integer*4	i,j
C
C	is it there??
C	
	inquire(file=filename,exist=found)
	If (.not. found) return
C
C	Open the file
C
	Open(66,file=filename,form='formatted',status='old',err=999)
C
C	Read lines one by one
C
	Do while (.true.)
C
C	Read a non-comment line
C
		Read(66,10,err=100,end=100) line
		If (line(1:1) .ne. "#") Then
10	Format(A)
C
C	line is: [whitespace](IDNUM)(whitespace)(value)
C
C	whitespace
C
			i = 1
			Do while (line(i:i) .lt. "!") 
				i = i + 1
				if (i .gt. 255) goto 300
			Enddo
C
			If (line(i:i) .eq. "#") goto 300  ! comment
C
C	IDNUM
C
			j = 0
			Do while (line(i:i) .ge. "!") 
				j = j + 1
				idnum(j:j) = line(i:i)
				i = i + 1
				if (i .gt. 255) goto 300
			Enddo
C
C	whitespace
C
			Do while (line(i:i) .lt. "!")
				i = i + 1
				if (i .gt. 255) goto 300
			Enddo
C
C	value
C
			Read(line(i:),*,err=300,end=300) v
			If (v .lt. 0.0) v = 0.0
			If (v .gt. 1.0) v = 1.0
C
C	find the match
C	
			Do i=1,map.num
				If (index(map.roi(i),idnum(1:j)) 
     +					.ne. 0) Then
					map.value(i) = v
				Endif
			Enddo
			
300			Continue
		Endif
	Enddo
C
C	done
C
100	Close(66)

	Return

999	Type 998,filename(1:index(filename," ")-1)
998	Format("Error:Unable to open the values file:",A)

	Call bv_error("Unable to open values file|"//
     +		"Check file permissions.}",0)

	Return
	End
