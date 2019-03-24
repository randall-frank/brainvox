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
C    MODULE NAME    : rmaker   
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
C    DESCRIPTION    :  	This program converts a resource file into
C			a fortran callable  subroutine which  has the
C			name get_dlg_comp and the same parameters as
C			the SGIUIF routine get_dlg.  (get_dlg_comp replaces
C			get_dlg in the final build...) 
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
	Program rmaker

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'  
	Include 'sgiuif.f'  

	Integer*4	Num_args,status,hunit
	Integer		iargc
	Character*200	infile,outfile,hfile
	Logical		Found,syntax,extra
	Record	/ditem/	item
C
	Integer*4	num_dlgs,I,J
	Integer*4	count(40)
C
	Character*24	stdate
	Character*24	fdate
	External	fdate,iargc
C
	Character*10	list(14)
C	
	data	list/  'DLG_HEADER',
     +			'DLG_COLOR',
     +			'DLG_END',
     +			'DLG_LINE',
     +			'DLG_BOX',
     +			'DLG_TEXT',
     +			'DLG_EDIT',
     +			'DLG_SBAR',
     +			'DLG_MENU',
     +			'DLG_CHECK',
     +			'DLG_BUTTON',
     +			'DLG_NOP', 
     +			'DLG_RADIO',
     +			'unknown'/ 
C
C	assume no include file
C
	hunit = -1
C
C	get command line
C
	stdate=fdate()
	num_args=Iargc()
	If ((num_args .ne. 2).and.(num_args .ne. 3)) then
		write(*,10)
		Call Exit(1)
10	Format(' rmaker - Program to compile resource files into FORTRAN.'/
     +	       ' usage: rmaker infile outfile {incfile}'/ 
     +     '    or: rmaker infile -ss or -s (resfile syntax checking)')
	Else
		call getarg(1,infile)
		call getarg(2,outfile)
		if (num_args .eq. 3) then
			call getarg(3,hfile)
			hunit = 105
		Endif
	Endif

	Inquire(file=infile,Exist=found)
	
	If (.not. found) then
		write(*,11)
		Call exit(1)
11	Format(' rmaker - Unable to find input file.')
	Endif

	Open(unit=100,file=infile,form='formatted',err=999,status='old')
C
	syntax=.false.
	extra =.false.
C
	If (outfile(1:2) .eq. '-s') then
		syntax = .true.
		hunit = -1
	Endif
	If (outfile(1:3) .eq. '-ss') then
		extra = .true.
		hunit = -1
	Endif
C
C	begin processing... (for syntax checking...)
C
	If (syntax) then
		Status=0
		I=1
		J=1
		Do While (status .eq. 0)
			write(*,5) I,J
5 	Format('Dialog #:',I5,' Item #:',I5)
			Call readarec(item,status,extra,-1)
			If (extra .eq. .false.) write(*,6)list(item.dtype)
6	Format('Item type:',A10)
			J=J+1
			If (item.dtype .eq. DLG_END) then
				I=I+1
				J=1
			Endif
		Enddo
		Close(100)
C	
		call exit(0)
	Endif
C
C	do it for real here...
C
	inquire(exist=found,file=outfile)
	If (found) then
		Open(101,file=outfile,form='formatted',err=997,status='old')
	Else
		Open(101,file=outfile,form='formatted',err=997,status='new')
	Endif
	If (hunit .ne. -1) then
		inquire(exist=found,file=hfile)
		If (found) then
		Open(hunit,file=hfile,form='formatted',err=997,status='old')
		Else
		Open(hunit,file=hfile,form='formatted',err=997,status='new')
		Endif
	Endif
C
C	parse the input file for number of dialogs and number of elements
C	also serves as syntax checker...
C
	num_dlgs=0
	status=0
C
C	until EOF
C
	Do while (status .eq. 0)
		Call readarec(item,status,syntax,-1)
		If ((item.dtype .eq. DLG_HEADER)
     +				.and.(status .eq. 0)) then
C
C	next dialog please...
C
			num_dlgs=num_dlgs+1
			If (num_dlgs .gt. 40) then
				write(*,995)
995	Format('rmaker - Error, more than 40 dialogs in input file.')
				Call exit(1)
			Endif
			count(num_dlgs)=0
		Endif
C
C	count entries
C
		If ((num_dlgs .ne. 0).and.(status .eq. 0)) then
			count(num_dlgs)=count(num_dlgs)+1
		Endif
	Enddo
C
C	back to start
C
	rewind (100)
	status=0
C
C	write include file header if needed
C
	If (hunit .ne. -1) then
		write(hunit,400) infile(1:33),num_dlgs,stdate
400	Format('***************************************************',/,
     +	       '*    Resource file converted to include by rmaker *',/,
     +         '*    Source file:',A33,'*',/,
     +         '*    File contained:',I6,' dialog(s).             *',/,
     +         '*    Compiled at:',A24,'         *',/,
     +	       '**Includes*****************************************')
	Endif
C
C	write out the header portion...
C
	Write(101,500) infile(1:33),num_dlgs,stdate
	Write(101,531) 
	Write(101,532) num_dlgs
500	Format('***************************************************',/,
     +	       '*    Resource file converted to FORTRAN by rmaker *',/,
     +         '*    Source file:',A33,'*',/,
     +         '*    File contained:',I6,' dialog(s).             *',/,
     +         '*    Compiled at:',A24,'         *',/,
     +	       '**interface****************************************')
531     Format('       Subroutine get_dlg_comp(file,num,arrlen,',
     +         'arr,stat)',/,
     +         '       Implicit None',/,
     +         '**includes**************',/,
     +         '       Include "sgiuif.h"',/,
     +         '**declarations**********',/,
     +         '       Character*200 file   !ignored by this routine',/,
     +         '       Integer*4     num    !dlg # to read',/,
     +         '       Integer*4     arrlen !size of arr',/,
     +         '       Record/ditem/ arr(*) !place to store dlg',/,
     +         '       Integer*4     stat   !returned status',/,
     +         '**code******************') 
532     Format('       stat=0           ',/,
     +         '       If (num .gt. ',I5,') then ',/,
     +         '            stat=-1',/,
     +         '            Return',/,
     +         '       Endif')
C
C	repeat for each dialog...
C
	Do I=1,num_dlgs
C
C	do the IF statment header...
C
		If (I .eq. 1) then
			write(101,501) I,count(I)
501	Format('       If ((num .eq. ',I5,').and.(arrlen .ge. ',I5,
     +         ')) then')
		Else
			write(101,502) I,count(I)
502	Format('       elseif ((num .eq. ',I5,').and.(arrlen .ge. ',I5,
     +         ')) then')
		Endif
		Write(101,541) I,count(I)
541	Format('**Dialog ',I5,' with ',I5,' items*****')
C
C	write out the assignment statements...
C	
		Do J=1,count(I)
			status = j
			Call readarec(item,status,syntax,hunit)
C
C	id line
C
			Write(101,542) J,count(I),list(item.dtype)
542	Format('**DlgItem ',I5,' of ',I5,' type=',A10,'*****')
C
C	write out the item
C
			Write(101,540) J,item.dtype,J,item.tlength,
     +			   J,item.tpos,J,item.text(1:40),
     +			   item.text(41:80),item.text(81:120),
     +			   item.text(121:160),item.text(161:200),
     +                     J,item.rect(1),
     +			   J,item.rect(2),J,item.rect(3),J,item.rect(4),
     +			   J,item.aux
540	Format('       arr(',I5,').dtype  =',I5,/,
     +         '       arr(',I5,').tlength=',I5,/,
     +         '       arr(',I5,').tpos   =',I5,/,
     +         '       arr(',I5,').text   ="',A40,'"//',/,
     +         '     +           "',A40,'"//',/,
     +         '     +           "',A40,'"//',/,
     +         '     +           "',A40,'"//',/,
     +         '     +           "',A40,'"',/,
     +         '       arr(',I5,').rect(1)=',I5,/,
     +         '       arr(',I5,').rect(2)=',I5,/,
     +         '       arr(',I5,').rect(3)=',I5,/,
     +         '       arr(',I5,').rect(4)=',I5,/,
     +         '       arr(',I5,').aux    =',I5)
		Enddo
C
C	and the return
C
		Write(101,503)
503	Format('             Return')
	Enddo
C
C	trailer...
C
	stdate = fdate()
	Write(101,504) stdate
504	Format('       Endif',/
     +         '       stat=-1',/
     +         '       Return',/
     +         '       End',/
     +         '** End of file :',A24,'*****',/)
C
C	done
C
	Close(101)
	Close(100)
	if (hunit .ne. -1) close(hunit)
	Call exit(0)
C
C	error
C
997	write(*,996)
996	Format(' rmaker : Error in opening output file.')
	Call exit(1)
999	write(*,998)
998	Format(' rmaker : Error in input file i/o.')
	Call exit(1)
	End
C
C
C	subroutines local to this routine...
C
C	this routine reads a record from the resource file...
C
	Subroutine	readarec(item,status,syntax,hunit)

	Implicit None

	Include		'sgiuif.h'
	Include		'sgiuif.f'

	Record	/ditem/	item
	Integer*4	status,I,hunit,linenum
	Character*200	line
	Logical		syntax
	Character*10	list(14)
C	
	data	list/  'DLG_HEADER',
     +			'DLG_COLOR',
     +			'DLG_END',
     +			'DLG_LINE',
     +			'DLG_BOX',
     +			'DLG_TEXT',
     +			'DLG_EDIT',
     +			'DLG_SBAR',
     +			'DLG_MENU',
     +			'DLG_CHECK',
     +			'DLG_BUTTON',
     +			'DLG_NOP', 
     +			'DLG_RADIO',
     +			'unknown'/ 
C
C	Passed status is the current item number
C	used for creating the include file
C
	linenum = status
C
C	first the type...
C
	Call readaline(line,status,syntax,linenum,hunit)
	If (status .ne. 0) goto 999
	read(line,*,err=999) item.dtype
C
C	the tlength        
C
	Call readaline(line,status,syntax,linenum,hunit)
	If (status .ne. 0) goto 999
	read(line,*,err=999) item.tlength
C
C	the tpos           
C
	Call readaline(line,status,syntax,linenum,hunit)
	If (status .ne. 0) goto 999
	read(line,*,err=999) item.tpos
C
C	the text
C
	Call readaline(line,status,syntax,linenum,hunit)
	If (status .ne. 0) goto 999
	item.text=line
C
C	the rectangle      
C
	Call readaline(line,status,syntax,linenum,hunit)
	If (status .ne. 0) goto 999
	read(line,*,err=999) (item.rect(I),I=1,4)
C
C	the aux            
C
	Call readaline(line,status,syntax,linenum,hunit)
	If (status .ne. 0) goto 999
	read(line,*,err=999) item.aux 
C
	If ((item.rect(3) .ne. -1).and.(item.dtype .ne. DLG_LINE)) then
C
C	reorder the points for upperleft to lower right
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
	Status=0
C
C	check for bad item type...
C
	If ((item.dtype .gt. DLG_RADIO).or.(item.dtype .lt. DLG_HEADER)) then
		item.dtype=DLG_RADIO+1
		Status=-1  
	Endif
C
	If (syntax) then
		write(*,100)list(item.dtype),item.tlength,item.tpos,
     +		item.text(1:60),
     +		item.rect(1),item.rect(2),item.rect(3),item.rect(4),
     +		Item.aux
100	Format('-----------------------------------------------',/
     +	       ' Dialog item:',A10,/
     +	       '         len:',I6,/
     +	       '         pos:',I6,/
     +	       '        text:',A60,/
     +	       '        rect:',I8,I8,I8,I8,/
     +         '         aux:',I6,/
     +	       '-----------------------------------------------')
	Endif
C
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
C	if hunit <> -1 then look for include file stuff and write to
C	unit=hunit using linenum as the item number
C
	subroutine	readaline(line,status,syntax,linenum,hunit)

	Implicit None

	Include		'sgiuif.h'
	Include		'sgiuif.f'

	Integer*4	status,I,linenum,hunit,j,k
	Character*200	line,tstr
	Logical		syntax
C
C	assume success
C
	status = 0
C
C	read the line
C
20	line = ""
	read(100,10,err=999,end=999) line
10	Format(A200)
11	Format(A70)
	if (syntax) write(*,11)line(1:70)
C
C	check for include file processing if need be...
C
	If (hunit .ne. -1) then
		I = index(line,';#')
		if (i .ne. 0) then
C
C	find first char past the ;#
C
			j = i+2
			Do while ((line(j:j) .eq. ' ').or.
     +				(line(j:j) .eq. char(9)))
				j = j +1
			Enddo
C
C	find last non-blank char in string
C
			k = j+1
			Do while ((line(k:k) .ne. ' ') .and.
     +				(line(k:k) .ne. char(9)))
				k = k + 1
			Enddo
			tstr = line(j:k-1)
C
C	send it to the include file unit
C
			write(hunit,15,err=999) tstr(1:20)
15	Format('       Integer*4 ',A20)
			write(hunit,16,err=999) tstr(1:20),linenum
16	Format('       Parameter (',A20,' = ',I5,') ')
		Endif
	Endif
C
C	walk along string looking for first non blank character
C
	I=1
30	If ((line(I:I) .eq. ' ').or.(line(i:i) .eq. char(9))) then
		I=I+1
		If (I .eq. 200) goto 20  !blank line
		goto 30
	Endif
C
C	standard comment checking
C
	If (line(I:I) .eq. ';') goto 20  !ignore comment lines
C
	Return
C
C	error... 
C
999	status=-1
	Return
	End
