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
C    MODULE NAME    : get_dlg 
C    VERSION        : 1.0 
C    LOCATION       :  
C    LAST EDIT      :  
C    STATUS         :  (Development,Testing,Production,Obsolete)
C    LANGUAGE       : FORTRAN
C    PACKAGE        : sgiuif
C
C
C    -- CREATION --------------                                                 
C                                                                               
C    PROGRAMMER     : rjf
C    DATE           : 08 Aug 89       
C    LINE-BY-LINER  :    
C    APPROVAL       : 
C
C    DESCRIPTION    :  	This routine reads a dialog from a  resource file
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
	Subroutine get_dlg(filename,num,dlglen,dialog,status)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'  
	Include 'sgiuif.f'  

	Character*200	filename
	Integer*4	num
	Integer*4	dlglen
	Integer*4	status
	Record	/ditem/	dialog(*)
C
	Logical		found   ! file exists???

	Record /ditem/	temp	! reading a ditem record
	Integer*4	count	! counting the number of headers

	Status=0
C
C	if filenames differ open a new file (if the old file had been open)
C
	If ((filename .ne. dlg_resfile).and.(dlg_resunit .ne. 0)) then   
		Close(dlg_resunit)
		dlg_resunit=0
	Endif
C
C	open a new file if none has been opened
C
	If (dlg_resunit .eq. 0) then
		dlg_resunit=100
		dlg_resfile=filename
		Inquire(file=filename,exist=found)
		If (.not. found) then
C
C	file cannot be found
C
			dlg_resunit=0
			status=-1
			Return
		Endif
		Open(unit=dlg_resunit,file=dlg_resfile,err=20,
     +			form='formatted',status='old')
		Goto 30
C
C	file cannot be opened
C
20		dlg_resunit=0
		status=-1
		Return
C	
	Endif
C
C	get the requested dialog template
C
30	Continue
C
C	point at start
C
	rewind(dlg_resunit)
	Count=0
C
C	read until the header number of EOF is reached
C
	Do while (count .lt. num)
		Call readarec(temp,status)
		If (status .ne. 0) then
			status=-1
			Return
		Endif
C
C	count number of headers...
C
		If (temp.dtype .eq. DLG_HEADER) then
			Count=Count+1
		Endif
	Enddo
C
C	count is now a placement index
C
	Count=1
C
C	is there room??
C
	If (dlglen .lt. 1) Then
		status=-1
		Return
	Endif
C
C	get the header
C
	dialog(1).dtype=temp.dtype
	dialog(1).tpos=temp.tpos 
	dialog(1).tlength=temp.tlength
	dialog(1).text=temp.text   
	dialog(1).rect(1)=temp.rect(1)
	dialog(1).rect(2)=temp.rect(2)
	dialog(1).rect(3)=temp.rect(3)
	dialog(1).rect(4)=temp.rect(4)
	dialog(1).aux=temp.aux    
C
C	loop until DLG_END
C
	Do while(dialog(count).dtype .ne. DLG_END)
C
C	bump the count and test if at end of array
C
		Count=count+1
		If (dlglen .lt. count) then
			status=-1
			Return
		Endif
C
C	get the next record
C
		Call readarec(dialog(count),status)
		If (status .ne. 0) then
			status=-1
			Return
		Endif
C
C	next record
C
	Enddo
C
C	return success...
C
	status=0
C
	Return
C
	End
C
C	subroutines local to this routine...
C
C	this routine reads a record from the resource file...
C
	Subroutine	readarec(item,status)

	Implicit None

	Include		'sgiuif.h'
	Include		'sgiuif.f'

	Record	/ditem/	item
	Integer*4	status,I
	Character*200	line
C
C	first the type...
C
	Call readaline(line,status)
	If (status .ne. 0) goto 999
	read(unit=line,fmt=*,err=999) item.dtype
C
C	the tlength        
C
	Call readaline(line,status)
	If (status .ne. 0) goto 999
	read(unit=line,fmt=*,err=999) item.tlength
C
C	the tpos           
C
	Call readaline(line,status)
	If (status .ne. 0) goto 999
	read(unit=line,fmt=*,err=999) item.tpos
C
C	the text
C
	Call readaline(line,status)
	If (status .ne. 0) goto 999
	item.text=line
C
C	the rectangle      
C
	Call readaline(line,status)
	If (status .ne. 0) goto 999
	read(unit=line,fmt=*,err=999) (item.rect(I),I=1,4)
C
C	if there is not a -1,-1 fix the rect coords...
C
	If ((item.rect(3) .ne. -1).and.(item.dtype .ne. DLG_LINE)) then
C
C	x1,y1 must be upper,left
C
		If (item.rect(1) .gt. item.rect(3)) then
			status=item.rect(3)
			item.rect(3)=item.rect(1)
			item.rect(1)=status
		Endif
		If (item.rect(4) .gt. item.rect(2)) then
			status=item.rect(4)
			item.rect(4)=item.rect(2)
			item.rect(2)=status
		Endif
	Endif
C
C	the aux            
C
	Call readaline(line,status)
	If (status .ne. 0) goto 999
	read(unit=line,fmt=*,err=999) item.aux 
C
	Status=0
	Return
C
C	error... 
C
999	status=-1
	Return
	End
C
C	this routine reads lines from the resfile looking for
C	non-comment lines...
C	it returns status=0 and the line in LINE when it finds one
C
	subroutine	readaline(line,status)

	Implicit None

	Include		'sgiuif.h'
	Include		'sgiuif.f'

	Integer*4	status,I
	Character*200	line
C
C	assume success
C
	status = 0
C
C	read the line
C
20	read(dlg_resunit,10,err=999,end=999) line
10	format(A200)
C
C	look past leading spaces...
C
	I=1
30	If (line(I:I) .eq. ' ') then
		I=I+1
		If (I .eq. 200) goto 20  !blank line
		goto 30
	Endif
	If (line(I:I) .eq. ';') goto 20  !ignore comment lines
C
	Return
C
C	error... 
C
999	status=-1
	Return
	End
