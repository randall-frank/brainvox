	Program brainvox_filer

C    ======================================================================     
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
C    MODULE NAME    : _Main   
C    VERSION        : 1.0 
C    LOCATION       :  
C    LAST EDIT      :  15 Mar 94
C    STATUS         :  (Testing,Production,Obsolete)
C    LANGUAGE       : FORTRAN
C    PACKAGE        : brainvox_filer
C
C
C    -- CREATION --------------                                                 
C                                                                               
C    PROGRAMMER     : rjf
C    DATE           : 15 Mar 94      
C    LINE-BY-LINER  :    
C    APPROVAL       : 
C
C    DESCRIPTION    :  	this program provides an interface to a standard  
C			file selection utility which can be used from
C			within an app or shell script 
C                                                                               
C    -- REVISION HISTORY --------------                                         
C
C                                                                               
C    -- GENERAL --------------                                                  
C                                                                               
C    SYNTAX :  stdfile "text" "startpath" pipe#(or -) [-dir][-any][-s file]
C    INPUTS         :          
C    OUTPUTS        : 
C
C    LIMITATIONS    :      
C    NOTES          :    This program will open a window with TEXT as 
C			annotation.  the file search will start at STARTPATH
C			if a pipe# is given all output will be through this
C			pipe.  if - is given output will be through stdout
C       		[-hide] - do not show files starting with '.'s
C			[-dir] - select only directories
C			[-any] - allow directories or files (defaulut is files
C					only.
C			[-s file] - save (as oppoesed to load) any trailing
C				    data is the initial file name...
C
C			The program returns a fully qualified pathname or
C			"<cancel>"
C                 
C    ====================================================================== 
C
C#define MAX_NAMES 800

	Implicit None

	Include 'fdevice.h'
	Include 'fgl.h'
	Include 'sgiuif.h'
	Include '../libs/vt_inc.f'
	Include 'fileruif.inc'
	Include 'filer_inc.inc'

	Integer*4	status
	Integer*4	device
	Integer*4	wid,iargc,pipenum,ftype,I,bv_chdir
	Integer*4	x,y,xs,ys
	logical		pipe,dotnames,use_filter,centerdlg,bells
	Integer*2	data
	character*200	text,dir,temp,filename,filter,tstr,homedir
	Record /ditem/	dialog(50)
	Integer*4	MAX_NAMES
	Parameter	(MAX_NAMES = 800)
C
	character*200	fnames(MAX_NAMES)
	integer*4	num_files,numargs
	integer*4	topline,selline,retain
C
C       IPC defines
C
        integer*4       size,message(60)
	Integer*4	tag_code,bv_getcwd,strlen,slength
	Integer*4	bv_os,bv_drives

	external	iargc,bv_chdir,bv_getcwd,strlen,slength
	external	bv_os,bv_drives
C
C       Guess the system path
C
        Call getarg(0,tstr)
        Call bv_set_app_name(tstr)
C
	numargs=iargc()
	If (numargs .lt. 4) then
2		write(*, 3)version,com_date
		write(*, 1)
		Call bv_exit(0)
3	Format(" Welcome to BRAINVOX_FILER version:",A15,/,
     +		" Copyright (C) ",A15," University of Iowa,",
     +		" Image Analysis Facility")
1	Format(" Usage : brainvox_filer shm_id tag_code prompt",
     +		" dir [options]",/,
     +    ' Options : [-hide]    Hide dot files',/,
     +    '           [-dir]     Return a directory name',/,
     +    '           [-any]     Return a directory or a filename',/,
     +    '           [-c]       Center the dialog on the screen',/,
     +    '           [-b]       Ring the keyboard bells',/,
     +    '           [-f expr]  Filter filenames by expr',/,
     +    '           [-s file]  Save a file (file is the default)',/,
     +    '           [-noret]   Do not allow the retain button')
	Endif
C
C	get shm_id
C
	call getarg(1,temp)
	read(unit=temp,fmt=*,err=2) shm_data
C
C	get Tag_code
C
	call getarg(2,temp)
	read(unit=temp,fmt=*,err=2) tag_code
C
C	get text
C
	Call getarg(3,text)
C
C	get path start
C
	Call getarg(4,dir)
	if ((tag_code.ne.FLR_WARNING).and.(tag_code.lt.FLR_REQUEST)) Then
		i=bv_chdir(dir(1:strlen(dir))//char(0))
		i=bv_getcwd(dir,%val(200))
		homedir = dir(1:index(dir,char(0))-1)
		dir = homedir
	Endif
C
C	All files
C
	filter = "."
	use_filter = .false.
C
C	get the rest...
C
	ftype=0    !0-files 1-directories 2-either 3-saving
	dotnames=.true.
	bells = .false.
	retain = 0
	centerdlg = .false.
	i=4
	Do while (i .lt. numargs)
		i=i+1
		call getarg(i,temp)
		if (temp(1:2) .eq. '-d') then
			ftype= 1
		else if (temp(1:2) .eq. '-a') then
			ftype= 2
		else if (temp(1:2) .eq. '-h') then
			dotnames=.false.
		else if (temp(1:2) .eq. '-n') then
			retain = -1
		else if (temp(1:2) .eq. '-c') then
			centerdlg = .true.
		else if (temp(1:2) .eq. '-b') then
			bells = .true.
		else if (temp(1:2) .eq. '-f') then
			if (i+1 .gt. numargs) goto 2
			i=i+1
			call getarg(i,filter)
			use_filter = .true.
		else if (temp(1:2) .eq. '-s') then
			if (i+1 .gt. numargs) goto 2
			i=i+1
			call getarg(i,filename)
			ftype= 3
		else
			goto 2
		Endif
	Enddo
C
C       I am NOT the shared memory owner!!! (there can be only one!!)
C
        shmem_owner = 0
        Call vt_init_shmem
C
C	start the GL stuff
C
	If ((tag_code.eq.FLR_WARNING).or.(tag_code.ge.FLR_REQUEST)) Then
		xs = 430
		ys = 105
	Else
		xs = 450
		ys = 305
	Endif
	Call foregr
	If (centerdlg .eq. .false.) Then
		Call prefsi(xs,ys)
	Else
		x = (getgde(GDXPMA)/2) - (xs/2)
		y = (getgde(GDYPMA)/2) - (ys/2)
		Call prefpo(x,x+xs,y,y+ys)
	Endif
	If (tag_code .eq. FLR_WARNING) Then
		wid=winope("Warning",7)
	Else if (tag_code .ge. FLR_REQUEST) Then
		wid=winope("Question",8)
	Else
		wid=winope("Filer",5)
	Endif
	Call rgbmod
	Call doubleb
	Call gconfi
	Call INIT_DLG(status)
	If (status .ne. 0) Call bv_exit(1)
	If ((tag_code.eq.FLR_WARNING).or.(tag_code.ge.FLR_REQUEST)) Then
		Call GET_DLG_COMP('stdfile.res',2,50,dialog,status)
	Else
		Call GET_DLG_COMP('stdfile.res',1,50,dialog,status)
	Endif
	If (status .ne. 0) Call bv_exit(1)
C
C	Beeps...
C
	If (bells) Then
		Call ringbe
		Call ringbe
	Endif
C
	If ((tag_code.eq.FLR_WARNING).or.(tag_code.ge.FLR_REQUEST)) Then
		dialog(uif_btn2).dtype = DLG_NOP
		dialog(uif_btn3).dtype = DLG_NOP
C
C	Parse the buttons string
C
		x = 1
		i = index(dir(x:200),"|")
		If (i .eq. 0) i = slength(dir(x:200),200-x)+1
		i = i + x - 1
		dialog(uif_btn1).text = dir(x:i-1)
		dialog(uif_btn1).tlength = i - x
		x = i+1
		if (x .lt. slength(dir,200)) Then
			i = index(dir(x:200),"|")
			If (i .eq. 0) i = slength(dir(x:200),200-x)+1
			i = i + x - 1
			dialog(uif_btn2).text = dir(x:i-1)
			dialog(uif_btn2).tlength = i - x
			dialog(uif_btn2).dtype = DLG_BUTTON
		Endif
		x = i+1
		if (x .lt. slength(dir,200)) Then
			i = index(dir(x:200-x),"|")
			If (i .eq. 0) i = slength(dir(x:200),200-x)+1
			i = i + x - 1
			dialog(uif_btn3).text = dir(x:i-1)
			dialog(uif_btn3).tlength = i - x
			dialog(uif_btn3).dtype = DLG_BUTTON
		Endif
C
C	Parse the prompt string
C
		x = 1
		i = index(text(x:200),"|")
		If (i .eq. 0) i = slength(text(x:200),200-x)+1
		i = i + x - 1
		dialog(uif_msg1).text = text(x:i-1)
		dialog(uif_msg1).tlength = i - x
		x = i+1
		if (x .lt. slength(text,200)) Then
			i = index(text(x:200),"|")
			If (i .eq. 0) i = slength(text(x:200),200-x)+1
			i = i + x - 1
			dialog(uif_msg2).text = text(x:i-1)
			dialog(uif_msg2).tlength = i - x
		Else
			dialog(uif_msg2).dtype = DLG_NOP
		Endif
		x = i+1
		if (x .lt. slength(text,200)) Then
			i = index(text(x:200-x),"|")
			If (i .eq. 0) i = slength(text(x:200),200-x)+1
			i = i + x - 1
			dialog(uif_msg3).text = text(x:i-1)
			dialog(uif_msg3).tlength = i - x
		Else
			dialog(uif_msg3).dtype = DLG_NOP
		Endif
		
		Call Do_Warning(dialog,wid,tag_code)
		goto 999
	Endif
C
C	setup for get or put
C
	if (ftype .eq. 3) then
C
C	stdput dialog
C
C	feed the edit field
C
		dialog(uif_edit).text=filename
		dialog(uif_edit).tlength=strlen(filename)
		dialog(uif_edit).tpos=strlen(filename)
		dialog(uif_open).text="Save"
	Else
C
C	stdget dialog
C
C	no edit field
C
		dialog(uif_edit).dtype = DLG_NOP
	Endif
C
C	Filter text (on/off)
C
	If (use_filter .eq. .false.) Then
		dialog(uif_filter_text).dtype = DLG_NOP
		dialog(uif_filter).dtype = DLG_NOP
	Else
		dialog(uif_filter).text = filter
		dialog(uif_filter).tlength=strlen(filter)
		dialog(uif_filter).tpos=strlen(filter)
	Endif
C
C	Noret
C
	If (retain .eq. -1) Then
		dialog(uif_retain).dtype = DLG_NOP
		retain = 0
	Endif
C
C	the text display line
C
	dialog(uif_prompt).text=text
	dialog(uif_prompt).tlength=40
	topline=1
	selline=0
C
	Call getdir(dir,num_files,fnames,dialog,dotnames,filter)
	Call setdlg(dir,num_files,fnames,topline,selline,dialog,ftype)
C
C Home/drive menu
C
	i = newpup()
	dialog(uif_home).aux = i
	text = "Initial directory %x1000|Home directory %x1001}"
	Call addtop(i,text,index(text,"}")-1,0)
	If (bv_os() .eq. 0) Then
32	Format(A1,":/ drive %x",I2.2,"}")
		y = bv_drives()
		Do x=0,27
			If (iand(y,2**x) .ne. 0) Then
				write(text,32) char(ichar("A")+x),x
				Call addtop(i,text,index(text,"}")-1,0)
			Endif
		Enddo
	Endif
C
	Call qdevic(WINQUI)
	Call qdevic(TIMER0)
	Call noise(TIMER0,20)  !every 1/3 seconds
	Call qenter(REDRAW,0)	! force a redraw
C
10	device=qread(data)	! top of event loop
	If (device .eq. WINQUI) then	! ignore these...
		goto 10      
	Endif
	If (device .eq. TIMER0) then
		size = 60
		call check_ipc(size,message)
		If (message(1) .eq. IPC_QUIT) Then
			goto 999
		Endif
		goto 10
	Endif
	If (device .eq. REDRAW) then	! redraw screen
		Call RESHAP	! do not forget !!!
		Call DRAW_DLG(dialog,status)
		If (status .ne. 0) Call bv_exit(1)
		Call swapbu
		Goto 10
	Endif
	Call HANDLE_DLG(device,data,dialog,status) 
	If (status .ne. 0) call bv_exit(1)
C
C	check if the event was handled
C
	If (device .eq.  DLG_NOPEVNT) goto 10
11	If (device .eq.  DLG_BUTEVNT) then
C
C	buttons
C
		If (data .eq. uif_cancel) then  !Cancel
			Call stdfiledone('<cancel> ',' ',tag_code)
			goto 999
		Elseif (data .eq. uif_open) then !Open
			if (ftype .eq. 3) then
				filename=
     +				  dialog(uif_edit).text
				i=dialog(uif_edit).tlength
				filename(i+1:i+1)=" "
				if (strlen(filename) .gt. 1) then
					Call stdfiledone(
     +					dir(1:strlen(dir)),      
     +					filename(1:i+1),
     +					tag_code)
				Endif
			Else
				if (selline .ne. 0) then
					Call stdfiledone(
     +					dir(1:strlen(dir)),      
     +					fnames(selline)
     +					(1:strlen(fnames(selline))),
     +					tag_code)
				endif
			Endif
			if (retain .eq. 0) Then
				goto 999
			Else
				goto 10
			Endif
C
C	UP button
C
		Elseif (data .eq. uif_up) then !up   
			i=strlen(dir)
			do while ((dir(i:i) .ne. "/").and.
     +                            (dir(i:i) .ne. "\").and.(i .gt. 0))
				dir(i:i)=" "
				i=i-1
			Enddo
			If (i .ge. 1) dir(i:i)=" "
			topline=1
			selline=0
			If ((index(dir,"/").eq.0).and.
     +				(index(dir,"\").eq.0)) Then
				dir(i:i+1) = "/ "
			Endif
C
	Call getdir(dir,num_files,fnames,dialog,dotnames,filter)
	Call setdlg(dir,num_files,fnames,topline,selline,dialog,ftype)
			Call qenter(REDRAW,wid)
			goto 10
C			
C	Down Button
C
		Elseif (data .eq. uif_down) then !Down
			if (dir(1:2) .eq. "/ ") then
				dir=fnames(selline)
			Else if (dir(2:4) .eq. ":\ ") then
				dir=dir(1:3)//fnames(selline)
			Else
                                i=strlen(dir)
				temp=dir(1:i)//fnames(selline)
				dir=temp
			Endif
			topline=1
			selline=0
C
	Call getdir(dir,num_files,fnames,dialog,dotnames,filter)
	Call setdlg(dir,num_files,fnames,topline,selline,dialog,ftype)
			Call qenter(REDRAW,wid)
			goto 10
C			
C	Home Button
C
		Elseif (data .eq. uif_home) then !Down
C			Call getenv("HOME",dir)
			dir = homedir
			topline=1
			selline=0
C
	Call getdir(dir,num_files,fnames,dialog,dotnames,filter)
	Call setdlg(dir,num_files,fnames,topline,selline,dialog,ftype)
			Call qenter(REDRAW,wid)
			goto 10
		Endif
C		
	Else If (device .eq.  DLG_BAREVNT) then
C
C	scrollbar
C
		topline=dialog(data).aux+1
		Call setdlg(dir,num_files,fnames,topline,selline,
     +				dialog,ftype)
		Call qenter(REDRAW,WID)
		goto 10
C
C       handle checkbox events
C
        Else if (device .eq. DLG_CHKEVNT) then
C
		If (data .eq. uif_retain) Then
			retain = dialog(data).aux
			If (retain .eq. 1) Then
				dialog(uif_cancel).text = "Close"
				dialog(uif_cancel).tlength = 5
			Else
				dialog(uif_cancel).text = "Cancel"
				dialog(uif_cancel).tlength = 6
			Endif
			Call qenter(REDRAW,WID)
			goto 10
		Endif
C		
	Else If (device .eq.  DLG_POPEVNT) then
C
		If (data .eq. uif_home) Then
			If (dialog(data).tpos .eq. 1000) Then
				dir = homedir
			Else If (dialog(data).tpos .eq. 1001) Then
				If (bv_os() .eq. 0) Then
					Call getenv("HOMEDRIVE",dir)
					Call getenv("HOMEPATH",text)
					dir(3:) = text(1:strlen(text))
				Else 
					Call getenv("HOME",dir)
				Endif
			Else 
				dir = "C:/"
				dir(1:1)=char(ichar("A")+dialog(data).tpos)
			Endif
			topline=1
			selline=0
C
			Call getdir(dir,num_files,fnames,dialog,
     +					dotnames,filter)
			Call setdlg(dir,num_files,fnames,topline,
     +					selline,dialog,ftype)
			Call qenter(REDRAW,wid)
			goto 10
		Endif
C
	Else If (device .eq.  DLG_TXTEVNT) then
C
C	text clicked (new selection)
C
		i=data-uif_first_list+topline
		if (i .eq. selline) then  ! double click
C
C	double clicking of a DIR does a DOWN (if the button is ON)
C
			If ((fnames(selline)(1:1) .eq. "/").and.
     +			(dialog(uif_down).dtype .eq. DLG_BUTTON)) then
				data=uif_down
				device=DLG_BUTEVNT
				goto 11
			Endif
C
C	double clicking of a FILE does an OPEN or SAVE (if the button is ON)
C
			If ((fnames(selline)(1:1) .ne. "/").and.
     +			(dialog(uif_open).dtype .eq. DLG_BUTTON)) then
				data=uif_open
				device=DLG_BUTEVNT
				goto 11
			Endif
				
		Else
			selline=i
		Endif
C
C	click copies to EDIT field  (files only)
C
		if ((ftype .eq. 3).and.
     +				(fnames(selline)(1:1) .ne. "/")) then
		dialog(uif_edit).text=fnames(selline)(1:200)
		dialog(uif_edit).tlength=strlen(fnames(selline))
		dialog(uif_edit).tpos=strlen(fnames(selline))
		Endif
		Call setdlg(dir,num_files,fnames,topline,selline,
     +				dialog,ftype)
		Call qenter(REDRAW,WID)
		goto 10
C		
	Else If (device .eq.  DLG_EDTEVNT) then
C
C	editfield
C
		If (data .eq. uif_filter) Then
			tstr = dialog(uif_filter).text
			filter = tstr(1:dialog(uif_filter).tlength)
			topline=1
			selline=0
C
C	Re-read the dir with the new filter
C
	Call getdir(dir,num_files,fnames,dialog,dotnames,filter)
	Call setdlg(dir,num_files,fnames,topline,selline,dialog,ftype)
			Call qenter(REDRAW,wid)
			goto 10
		Endif
	Endif

	Goto 10
C
C       quit routine
C
999     continue
C
C       since we are a lower life form we should not send IPCs
C       cut the umbilical
C
        Call winclo(wid)
        Call vt_quit_shmem
        Call bv_exit(0)
C
	End
C
C-----------------------------------------------------------
C
C	routine to write the selected filename to the proper output
C
	Subroutine	stdfiledone(dirname,fname,tag_code)

	Implicit None

	Integer*4	status,I,J,tag_code,strlen
	Character*(*)	fname
	Character*(*)	dirname
	Character*200	string
	External strlen
C
C	Put together a complete filename
C
	i = strlen(dirname)
	j = strlen(fname)
	if (dirname(1:8) .eq. '<cancel>') then
		string = '<cancel> '
		i = 0
	Else if (fname(1:1) .eq. '/') then
		string = dirname(1:i)//fname(1:j)//' '
		i = i + j
	Else
		string = dirname(1:i)//"/"//fname(1:j)//' '
		i = i + j + 1
	Endif
C
C	Zero terminate
C
	string(i+1:i+1) = char(0)
	string(i+2:) = " "
C
C	Send out via IPC
C
	Call send_string(string,tag_code,i+1)
C
	Return
	End
C
	Subroutine	send_string(string,tag_code,len)

	Implicit None

	Include '../libs/vt_inc.f'

	Integer*4	string(*)
	Integer*4	tag_code,len,i,j
	Integer*4	message(IPC_SHM_SIZE)
C
C	Setup the header
C		
	i = 1
	message(i) = IPC_F_RETURN
	i = i + 1
	message(i) = tag_code
	Do j = 1,(len+5)/4
		i = i + 1
		message(i) = string(j)
	Enddo
	Call send_ipc(i,message)
C
	Return
	End
C
C-----------------------------------------------------------
C
C	routine to read the filenames in DIR and return them
C
	subroutine getdir(dir,num_files,fnames,dialog,dotnames,filter)

	Implicit None

	Include 'fgl.h'
	Include 'sgiuif.h'
	Include 'fileruif.inc'

	Integer*4	MAX_NAMES
	Parameter	(MAX_NAMES = 800)

	character*200	dir,fnames(MAX_NAMES),dname,filter,cfilter
	integer*4	num_files
	byte		buffer(MAX_NAMES*200)
	Integer*4	I,ptr,J,K,strlen
	record/ditem/	dialog(*)
	logical		dotnames
	Integer*2	compar
	External	compar,strlen
C
	Call wait_cursor
C
C	read the directory
C	
	i=strlen(dir)
	if (i .eq. 0) then
		dname="/"//char(0)
	Else
		dname=dir(1:i)//char(0)
	Endif

	i = strlen(filter)
	cfilter = filter(1:i)//char(0)
C
C	call C-routine defined in sgiuif package
C
	j = MAX_NAMES*200
	i = 0
	Call read_filenames(dname,buffer,%val(j),cfilter,i,%val(0))
C	Call readdir(I,buffer,dname)
C
C	parse out the directory names...
C
	num_files=0
	ptr=1
	Do J=1,I
		fnames(num_Files+1)=""
		k=0
		Do while (buffer(ptr) .ne. 0)
			k=k+1
			fnames(num_files+1)(k:k)=char(buffer(ptr))
			ptr=ptr+1
		Enddo
		ptr=ptr+1
C
C	(save none of the '.' files...)
C
		If ((num_files .lt. MAX_NAMES-1 ) .and.
     +			(fnames(num_files+1)(1:2) .ne. "/.")) then
			num_files=num_files+1
		Endif
		if ((.not. dotnames).and.
     +            (fnames(num_files)(1:1).eq.'.')) num_files=num_files-1
	Enddo
C
C	set the scroll bar
C
	dialog(uif_sbar).tlength=max(num_files-1,0)    !0-numfiles-1
	dialog(uif_sbar).aux=0              
C
C	sort the array
C
	Call qsort(fnames,num_files,200,compar)
C
	Call arrow_cursor
C
	return
	End
C
C-------------comparison function---------------
C
	Integer*2	function	compar(f1,f2)

	Implicit None

	Character*200	f1,f2
C
C	return  0 if f1=f2
C	return -1 if f1<f2  (f1 is before f2)
C	return  1 if f2<f1  (f2 is before f1)
C
	compar=0
C
	If (f1 .eq. f2) return
C
	If (llt(f1,f2)) Then  !is f1<f2
		compar=-1
	Else
		compar=1
	Endif
C
	return
	end
C
C-----------------------------------------------------------
C
C	routtine to set the dialog entries for the scrolling list
C
	subroutine setdlg(path,num_files,fnames,topline,selline,
     +		dialog,ftype)
	
	Implicit None

	Include 'fgl.h'
	Include 'sgiuif.h'
	Include 'fileruif.inc'

	Integer*4	MAX_NAMES
	Parameter	(MAX_NAMES = 800)

	character*200	path,fnames(MAX_NAMES)
	integer*4	num_files,topline,selline,I,ftype,strlen
        external strlen
	record/ditem/   dialog(*)
C
	Do I=uif_first_list,uif_last_list
C
C	fill in the text 
C
		If (i-uif_first_list+topline .le. num_files) then
		dialog(i).text=fnames(i-uif_first_list+topline)(1:200)
			dialog(i).tpos=1  ! clickable
			dialog(i).tlength=32
		Else
			dialog(i).text=""
			dialog(i).tpos=0  ! non-clickable
		Endif
C
C	highlight
C
		If (i-uif_first_list+topline .eq. selline) then
			dialog(i).aux=1
		Else
			dialog(i).aux=0
		Endif
	Enddo
C
C	set the path var
C
	i=strlen(path)
	dialog(uif_path).text="Path:"//path(1:74)
	If (i .gt. 38) then
		dialog(uif_path).text="Path:..."//path(i-38:i)
	Endif
	if (i .eq. 0) then
		dialog(uif_path).text="Path:/"
		dialog(uif_up).dtype=DLG_NOP
	Else
		dialog(uif_up).dtype=DLG_BUTTON
	Endif
	dialog(uif_path).tlength=47
C
	If ((selline .ne. 0).and.(fnames(selline)(1:1) .eq. "/")) then
C
C	down available
C
		dialog(uif_down).dtype=DLG_BUTTON
C
	Else
C
C	nothing selected so down disabled..
C
		dialog(uif_down).dtype=DLG_NOP
	Endif
C
C	now the open button
C
C
C ftype=1 = dir
C ftype=0 = files
C
	dialog(uif_open).dtype=DLG_BUTTON
	If ((selline.eq.0).and.(ftype.ne.3))dialog(uif_open).dtype=DLG_NOP
	if (selline .ne. 0) then
		if ((ftype .eq. 1).and.(fnames(selline)(1:1) .ne. "/"))
     +			dialog(uif_open).dtype=DLG_NOP
		if ((ftype .eq. 0).and.(fnames(selline)(1:1) .eq. "/"))
     +			dialog(uif_open).dtype=DLG_NOP
	Endif

	Return
C
	End
C
C	Warning dialog
C
	Subroutine Do_Warning(dialog,wid,tagcode)

	Implicit None

	Include 'fdevice.h'
	Include 'fgl.h'
	Include 'sgiuif.h'
	Include '../libs/vt_inc.f'
	Include 'fileruif.inc'
	Include 'filer_inc.inc'

	Integer*4	wid,tagcode
	Record/ditem/   dialog(*)
	Integer*4	device,status,i
	Integer*2	data
	character*250	tstr
C
C       IPC defines
C
        integer*4       size,message(60)

	Call qdevic(WINQUI)
	Call qdevic(TIMER0)
	Call noise(TIMER0,20)  !every 1/3 seconds
	Call qenter(REDRAW,0)	! force a redraw
C
10	device=qread(data)	! top of event loop
	If (device .eq. WINQUI) then	! ignore these...
		goto 10      
	Endif
	If (device .eq. TIMER0) then
		size = 60
		call check_ipc(size,message)
		If (message(1) .eq. IPC_QUIT) Then
			return
		Endif
		goto 10
	Endif
	If (device .eq. REDRAW) then	! redraw screen
		Call RESHAP	! do not forget !!!
		Call DRAW_DLG(dialog,status)
		If (status .ne. 0) Call bv_exit(1)
		Call swapbu
		Goto 10
	Endif
	Call HANDLE_DLG(device,data,dialog,status) 
	If (status .ne. 0) call bv_exit(1)
C
C	check if the event was handled
C
	If (device .eq.  DLG_NOPEVNT) goto 10
11	If (device .eq.  DLG_BUTEVNT) then
		If (tagcode .eq. FLR_WARNING) then
			Return
		Else
			tstr = dialog(data).text
			i = dialog(data).tlength+1
			tstr(i:i) = " "
			tstr(i+1:i+1) = char(0)
			Call send_string(tstr,tagcode,i+1)
			Return
		Endif
	Endif
	goto 10

	Return
	End
