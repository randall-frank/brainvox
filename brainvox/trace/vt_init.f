C    ======================================================================     
C    THIS SOFTWARE IS THE PROPERTY OF:                                          
C         The Image Analysis Facility                                     
C         133 MRC     
C         The University of Iowa, Iowa City, Iowa                               
C    ======================================================================     
C                                                                               
C    -- OVERVIEW --------------                                                 
C                                                                               
C    MODULE NAME    : tr_init
C    VERSION        : 1.0 
C    LOCATION       :  
C    LAST EDIT      :  
C    STATUS         :  (Development,Testing,Production,Obsolete)
C    LANGUAGE       : FORTRAN
C    PACKAGE        : brainvox_trace
C
C
C    -- CREATION --------------                                                 
C                                                                               
C    PROGRAMMER     : rjf
C    DATE           : 21 sep 89       
C    LINE-BY-LINER  :    
C    APPROVAL       : 
C
C    DESCRIPTION    :  	this routine sets up the graphics screen/hardware
C			and fills out menus
C                                                                               
C    -- REVISION HISTORY --------------                                         
C
C                                                                               
C    -- GENERAL --------------                                                  
C                                                                               
C    SYNTAX         :     Call tr_init
C    INPUTS         :          none
C    OUTPUTS        : all *_menus are returned menu handles
C
C    LIMITATIONS    :      
C    NOTES          :      
C                 
C                                                                               
C    ====================================================================== 
	Subroutine tr_init(wid,wid2)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'
	Include '../libs/vt_inc.f'
	Include '../libs/vt_protos.f'
	Include 'tr_inc.f'
	Include 'vt_arb_inc.f'
	Include 'traceuif.inc'

	Integer*4	wid,wid2
	Integer*4	I,iargc,month,day,year,J,JJ
	Character*200	filename
	logical		found,error
        Integer*4       tptrs(max_images,max_rgns)
	character*40	temp,username
C
	Integer*4	gamma_menu,patinfo_menu,strlen
	Character*200	text

        common  /virt/tptrs
C
	external	iargc,strlen
C
C	get command line
C
	If ((iargc() .lt. 1).or.(bv_test_option(OPTS_TESTING))) Then
        write(*, 1)version,com_date
1       Format(" Welcome to BRAINVOX_TRACE version:",A15,/,
     +         " Copyright (C) ",A15," University of Iowa,",
     +         " Image Analysis Facility")
	Endif
C
	wx=768
	wy=768
	I=iargc()
C
	If (I .lt. 2) then
2		write(*, 10)
10      Format("This program must be launched from within brainvox.") 
		Call bv_exit(1)
	Endif
C
	Call getarg_nq(iargc(),text)
	read(unit=text,fmt=*,err=2) shm_data
C
C       I am NOT the shared memory owner!!! (there can be only one!!)
C
        shmem_owner = 0
        Call vt_init_shmem
C
C	t2/pd pairs...
C
	skip=2 
C
C	handle the '-s' flag for 512x512 images
C
	call getarg(2,filename)
	if (filename(1:2) .eq. '-s') then
		wx=512
		wy=512
	Endif
C
	Call getarg_nq(1,text)   
	numrgns=0  
C
C	read the rgnsdef file
C
	call read_rgns(text,error)
C
C       default is to display all loaded rois...
C
	Do I=1,numrgns
                background(I)=.true.
        Enddo
C
C	also the wd will always be .
C
	skip=1    !t2/pd only
	Units="Pixels"
C
C	init vars...
C
	exact_tracing = 0
	pd_l=.true.
	cur_slice=1
	cur_roi=0
	zoom=1
	if (wx .gt. 512) zoom=2 
C
C	open 'SQUARE'  1000 (-200) , 800 window
C	double buffered color map mode
C
	Call foregr 
C
C	open the dialog window
C
	i = (wx+15) + 16
	j = 315 + 14
	Call prefpo(i,i+512,j,j+250)
	Wid=winope('Trace',5)
	Call rgbmod
	Call doubleb
	Call gconfi
C
C	open the image window
C
	i = 0
	j = 0
C	Call prefpo(i,i+wx-1,j,j+wy-1)
	Call minsiz(512,512)
	Call prefpo2(i,j+wy-1)
	wid2 = winope('Trace:Image',11)
	Call minsiz(512,512)
	Call wincon
	Call doubleb
	Call rgbmod
	Call gconfi
C
C	set all queue events
C
	Call Qdevic(REDRAW)
C
	Call Qdevic(MOUSE3)
	Call Qdevic(MOUSE1)
	Call Qdevic(MOUSE2)
	Call Qdevic(WINQUI)
C
C	queue the function keys...
C
	Do I=F1KEY,F12KEY
		Call Qdevic(I)
	Enddo
C
C	queue the scrolling keys and keyboard aliases
C
	call qdevic(PAD0)
	call qdevic(PAD2)
	call qdevic(PAD4)
	call qdevic(PAD6)
	call qdevic(PAD8)
	Call qdevic(PADAST)
C
	Call init_dlg(i)
	If (I .ne. 0) call trace_exit(wid,wid2)
C
	Call get_dlg_comp('trace.res',1,50,state1,i)
	If (I .ne. 0) Call trace_exit(wid,wid2)
	Call get_dlg_comp('trace.res',2,50,state2,i)
	If (I .ne. 0) Call trace_exit(wid,wid2)
C
C       setup the initial stuff... (into dialog records)
C
	Call getarg(1,text)   
        write(state1(16).text,6) text(1:strlen(text))
6       Format('Region:',A)
        state1(16).tlength=7+strlen(text)
        state2(16).tlength=state1(16).tlength
        state2(16).text=state1(16).text
C
C	set up the colors popup menu
C
	state2(34).aux=newpup()
C
        state1(37).aux=newpup()
        state2(37).aux=state1(37).aux
        state2(13).aux=newpup()
        text = 'Red %s%r1|Green %I%r1|Blue%r1|Yellow%r1|Cyan%r1'
     +                  //'|Violet%r1|White%r1|Black%r1}'
        Call addtop(state1(37).aux,text,index(text,'}')-1,0)
        text = 'Red %s%I%r1|Green %r1|Blue%r1|Yellow%r1|Cyan%r1'
     +                  //'|Violet%r1|White%r1|Black%r1}'
        Call addtop(state2(13).aux,text,index(text,'}')-1,0)
C
C	set up the options popup menu
C
	state1(38).aux=newpup()
	state2(38).aux=newpup()
	state2(39).aux=state2(34).aux
	gamma_menu=newpup()
C
C	first the rollover...
C
	text='Gamma settings %t%s|0.8 %x80|1.0 %x100|1.2 %x120|'//
     +		'1.4 %x140|1.6 %x160|1.7 %x170|1.8 %x180|2.0 %x200|'//
     +		'2.2 %x220}'
	call addtop(gamma_menu,text,index(text,'}')-1,0)
C
C	now the real mccoy..
C
	text='Options %t|Traces on %x1|Traces off %x2|'//
     +		'Exact Tracing %i%x4}'
	Call addtop(state1(38).aux,text,index(text,'}')-1,0)
	Call addtop(state2(38).aux,text,index(text,'}')-1,0)
C	text='Gamma %m}'
C	Call addtop(state1(38).aux,text,index(text,'}')-1,gamma_menu)
C	Call addtop(state2(38).aux,text,index(text,'}')-1,gamma_menu)
	if (cut_dir .ne. 0) Then
	   text = 'Slice mode... %x5}'
	   Call addtop(state1(38).aux,text,index(text,'}')-1,0)
	   Call addtop(state2(38).aux,text,index(text,'}')-1,0)
	Endif

	text = "Thin current ROI %x3}"
	Call addtop(state2(38).aux,text,index(text,"}")-1,0)

	text = "Auto-split Preview %x6|Auto-split Hemis %x7}"
	Call addtop(state1(38).aux,text,index(text,"}")-1,0)
C
	text = "Save tracing images... %x8}"
	Call addtop(state1(38).aux,text,index(text,"}")-1,0)
C
C	patient info pup
C
	patinfo_menu=newpup()
	Do j=1,3
		i=200
		do while((patientinfo(j)(i:i) .eq. ' ').and.(i .gt. 1))
			i= i - 1
		Enddo
		text = patientinfo(j)(1:i)//' %x0}'
		Call addtop(patinfo_menu,text,index(text,"}")-1,0)
	Enddo
	text = "Dataset data %m}"
	Call addtop(state2(38).aux,text,index(text,"}")-1,patinfo_menu)
	Call addtop(state1(38).aux,text,index(text,"}")-1,patinfo_menu)
C
C	The copy a trace popup item
C
	Do i = 1, numrgns
		text = rgnnames(i)//" "
		j = index(text," ")
		write(text(j+1:j+4),664) i
664	Format("%x",I2.2)
		Call addtop(state2(34).aux,text,j+4,0)
	Enddo
C
C	If single slices the disable the PD T2 buttons
C

	If ((skip .eq. 1).and.(.false.)) then
		state1(6).dtype=12 ! NOP
		state1(7).dtype=12
		state2(6).dtype=12
		state2(7).dtype=12
	Endif
C
C	Arb dialog
C
	Call get_dlg_comp('trace.res',3,30,arb_uif,i)
	If (I .ne. 0) Call trace_exit(wid,wid2)
        arb_uif(tr_arb_mode).aux=newpup()
        text = 'Slices %x1|Arbitrary %l%x2|'//
     +			'Update arb mode %x3}'
        Call addtop(arb_uif(tr_arb_mode).aux,text,index(text,'}')-1,0)
C
        arb_uif(tr_arb_submode).aux=newpup()
        text = 'Global %x1}'
        Call addtop(arb_uif(tr_arb_submode).aux,text,
     +		index(text,'}')-1,0)
	Do i = 2, numrgns
		text = rgnnames(i)//" "
		j = index(text," ")
		write(text(j+1:j+4),664) i
		Call addtop(arb_uif(tr_arb_submode).aux,text,j+4,0)
	Enddo
C
C       init the vset (both 8 bit volumes)
C
        Call    vl_init(myset)
        i = realimages+((realimages - 1) * interpimages)
        Call    vl_data(%val(imagesize),%val(imagesize),%val(i),
     +                  %val(images_addr),myset)
        Call    vl_aux_data(%val(1),%val(texture_addr),myset)
C
C       set the squeeze factor
C
        If (realimages .gt. 2) then
                squeezefactor = (float(realimages)*interslice)/
     +    (float(realimages + (interpimages*(realimages-1)))*interpixel)

        	Call vl_setsqu(squeezefactor,myset)
		Call vl_setinterp(%val(interpimages),myset)
        Endif
C	
	Call Init_Arb_Window
C
	oldslices = 0
C
C	Note This is an obsolete call but in one place they say to do
C	it and in another they say it is no longer needed???
C
	Call resetl(.true.)
C
C	zero out dynamic memory pointers...
C
	Do I=1,max_images
		Do J=1,max_rgns
			tptrs(I,J)=0
		Enddo
	Enddo
C
	call winset(wid2)
C
	Call qenter(REDRAW,0)
C
	Return
C
	End
C
C 	Routine to translate a keystroke into a button event...
C
	Subroutine HotKeys(device,data)

	Implicit None

	Include 'fdevice.h'
	Include 'fgl.h'
	Include 'sgiuif.h'

	Integer*4	device
	Integer*2	data

	If (device .ne. KEYBD) return

	If ((data .eq. ICHAR('e')).or.(data .eq. ICHAR('E'))) Then
		device = DLG_BUTEVNT
		data = 8
		Return
	Endif
	If ((data .eq. ICHAR('a')).or.(data .eq. ICHAR('A'))) Then
		device = DLG_BUTEVNT
		data = 35
		Return
	Endif
	If ((data .eq. ICHAR('r')).or.(data .eq. ICHAR('R'))) Then
		device = DLG_BUTEVNT
		data = 11
		Return
	Endif
	If ((data .eq. ICHAR('n')).or.(data .eq. ICHAR('N'))) Then
		device = DLG_BUTEVNT
		data = 14
		Return
	Endif
	If ((data .eq. ICHAR('s')).or.(data .eq. ICHAR('S'))) Then
		device = DLG_BUTEVNT
		data = 15
		Return
	Endif
	If ((data .eq. ICHAR('c')).or.(data .eq. ICHAR('C'))) Then
		device = DLG_BUTEVNT
		data = 9
		Return
	Endif

	Return
	End
C
