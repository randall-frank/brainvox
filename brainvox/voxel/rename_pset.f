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
C    MODULE NAME    : Rename pset
C    VERSION        : 1.0 
C    LOCATION       :  
C    LAST EDIT      :  
C    STATUS         :  (Development,Testing,Production,Obsolete)
C    LANGUAGE       : FORTRAN
C    PACKAGE        : brainvox
C
C
C    -- CREATION --------------                                                 
C                                                                               
C    PROGRAMMER     : rjf
C    DATE           : 12 Jan 93       
C    LINE-BY-LINER  :    
C    APPROVAL       : 
C
C    DESCRIPTION    :  	Routines to handle the rename a pointset dialog
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
	Subroutine rename_pointsets(data_id)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'  
	Include '../libs/vt_inc.f'
	Include 'voxeluif.inc'
	
	Integer*4	data_id,status
	Record /ditem/  dialog(100)
C
C       IPC defines
C
        Integer*4       size,message(60)
C
	Character*255	tstr,tempstr
	Integer*4	cur_set
	Integer*4	device,i,j,wid,st
	Integer*2	data
C
C       I am NOT the shared memory owner!!! (there can be only one!!)
C
	shm_data = data_id
        shmem_owner = 0
        Call vt_init_shmem
C
C       get the window
C
        call prefsi(300,130)
        wid=winope("Pointsets",9)
        Call doubleb
        Call RGBmod
        Call gconfi
        Call qdevic(WINQUI)
        Call INIT_DLG(status)
        If (status .ne. 0) Call bv_exit(1)
        Call GET_DLG_COMP('voxeluif.r',2,100,dialog,status)
        If (status .ne. 0) Call bv_exit(1)
C
C       Menus for the pointsets
C
	dialog(pset_menu).aux = newpup()
        Do I=0,19
                Call get_point_name(i,tstr,'./ ')
                j = 200
                Do while (tstr(j:j) .eq. ' ')
                        j = j - 1
                Enddo
		st = 1
		if (tstr(1:1) .eq. '#') st = 2
		If (i .eq. 0) then
			cur_set = 0
			dialog(pset_edit).text = tstr(st:j)
			dialog(pset_edit).tlength = j-st+1
			dialog(pset_edit).tpos = j-st+1
			write(tempstr,9) tstr(st:j)
9	Format("Pointset:",A)
			dialog(pset_menu).text = tempstr
			dialog(pset_menu).tlength = j+9-st+1
			dialog(pset_locked).aux = st-1
		Endif
                write(tempstr,8) tstr(st:j),I+1000
                Call addtop(dialog(pset_menu).aux,tempstr,
     +			index(tempstr,"}")-1,0)
8       Format(A," %x",I4.4,"}")
        Enddo
C
	Call qdevic(TIMER0)
	Call noise(TIMER0,20)  !every 1/3 seconds
	Call qenter(REDRAW,wid)
C
C       redraw comes from setup_dialog
C
10      device=qread(data)      ! top of event loop
C
C       timer stuff
C
        if (device .eq. TIMER0) then
                size = 60
                call check_ipc(size,message)
                if (message(1) .eq. IPC_QUIT) goto 999
                goto 10
        Endif
C
C       handle redraw
C
        If (device .eq. REDRAW) then    ! redraw screen
                Call RESHAP     ! do not forget !!!
                Call ortho2(0.0,300.0,0.0,130.0)
                Call DRAW_DLG(dialog,status)
                If (status .ne. 0) then
                         goto 999
                Endif
		Call swapbu
	Endif
        Call HANDLE_DLG(device,data,dialog,status)
        If (status .ne. 0) goto 999
        If (device .eq.  DLG_NOPEVNT) goto 10
C
C       handle buttons
C
        If (device .eq.  DLG_BUTEVNT) then
		If (data .eq. pset_doit) then
			st = dialog(pset_locked).aux
			j = dialog(pset_edit).tlength
			tstr = dialog(pset_edit).text(1:j)

			write(tempstr,9) tstr(1:j)
			dialog(pset_menu).text = tempstr
			dialog(pset_menu).tlength = j+9
			if (st .eq. 1) tstr = "#"//tstr
			Call set_point_name(cur_set,tstr,'./ ')
			Call update_pset_menu(
     +				dialog(pset_menu).aux,1000,'./ ',0)
                        Call send_ipc(1,IPC_PNAMES)
			Call qenter(REDRAW,wid)
		Else if (data .eq. pset_done) then
			goto 999
		Endif
		goto 10
	Endif
C
C       handle radio events
C
        If (device .eq. DLG_CHKEVNT) then
                If (data .eq. pset_locked) then  ! left side
			j = dialog(pset_locked).aux
		Endif
	Endif
C
C	handle edit box
C
	If (device .eq.  DLG_EDTEVNT) then
		If (data .eq. pset_edit) then
			i = dialog(pset_edit).tlength
		Endif
		goto 10
	Endif
C
C       handle menu events
C
        If (device .eq. DLG_POPEVNT) then
		If (data .eq. pset_menu) Then
			i = dialog(data).tpos
			If ((i .lt. 1000) .or. (i .gt. 1020)) goto 10
			i = i - 1000
                	Call get_point_name(i,tstr,'./ ')
                	j = 200
                	Do while (tstr(j:j) .eq. ' ')
                        	j = j - 1
                	Enddo
			st = 1
			if (tstr(1:1) .eq. '#') st = 2
                        cur_set = i
                        dialog(pset_edit).text = tstr(st:j)
                        dialog(pset_edit).tlength = j-st+1
			dialog(pset_edit).tpos = j-st+1
			write(tempstr,9) tstr(st:j)
			dialog(pset_menu).text = tempstr
			dialog(pset_menu).tlength = j+9-st+1
			dialog(pset_locked).aux = st-1
			Call qenter(REDRAW,wid)
		Endif
		goto 10
	Endif
C
C	get next event
C
	goto 10
C
C	Exit 
C
999	Call winclo(wid)
	Call vt_quit_shmem
	Call bv_exit(0)

	Return
	End
