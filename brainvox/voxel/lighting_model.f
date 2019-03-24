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
C    MODULE NAME    : Lighting model control dialog
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
C    DATE           : 4 Mar 94        
C    LINE-BY-LINER  :    
C    APPROVAL       : 
C
C    DESCRIPTION    :  	Routines to handle the Lighting model control
C				dialog
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
	Subroutine lighting_model_control(data_id)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'  
	Include '../libs/vt_inc.f'
	Include 'voxeluif.inc'
	
	Integer*4	data_id,status
	Record /ditem/  dialog(100)

C	Box definition

	Integer*4	XC,YC,XW,YW
	Parameter	(XC=260)
	Parameter	(YC=90)
	Parameter	(XW=35)
	Parameter	(YW=35)
C
C       IPC defines
C
        Integer*4       size,message(60)
C
	Character*256	tstr,tempstr
	Integer*4	device,i,j,wid,lon
	Integer*2	data
	Real*4		vals(5)  ! dx,dy,dz,l1,l2
	Integer*4	filters(4)
	Integer*4	x,y,xo,yo,wleft,wbot

	data	filters/10,11,15,16/
C
C       I am NOT the shared memory owner!!! (there can be only one!!)
C
	shm_data = data_id
        shmem_owner = 0
        Call vt_init_shmem
C
C	Defaults
C
	vals(1) = 0.
	vals(2) = 0.
	vals(3) = 1.
	vals(4) = 1.
	vals(5) = 1.
C
C       get the window
C
        call prefsi(300,130)
        wid=winope("Lighting model",14)
        Call doubleb
        Call RGBmod
        Call gconfi
        Call qdevic(WINQUI)
        Call INIT_DLG(status)
        If (status .ne. 0) Call bv_exit(1)
        Call GET_DLG_COMP('voxeluif.r',4,100,dialog,status)
        If (status .ne. 0) Call bv_exit(1)
C
C       Menus for the filters
C
	dialog(lmod_menu).aux = 0
	dialog(lmod_menu).aux = newpup()
     	tstr = "Image median %x1|Image average %x2%l|"//
     +	  "Zbuffer average %x3|Zbuffer median %x4}"
        Call addtop(dialog(lmod_menu).aux,tstr,index(tstr,"}")-1,0)
C
	lon = 0
	dialog(lmod_intrinsic).aux = lon
C
	Call qdevic(MOUSE1)
	Call qdevic(MOUSE2)
	Call qdevic(MOUSE3)
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
	If  ((device .ge. MOUSE1) .and. (device .le. MOUSE3) 
     +          .and. (data .eq. 1)) Then
C       get window pos
                call getori(wleft,wbot)
C       get window x,y
                xo = getval(CURSRX)-wleft
                yo = getval(CURSRY)-wbot
                if ((xo .lt. XC-XW) .or. (yo .lt. YC-YW) .or.
     +                  (xo .gt. XC+XW) .or. (yo .gt. YC+YW)) goto 250
C
                Do while (getbut(device))
C       get new x,y
                        x = getval(CURSRX)-wleft
                        y = getval(CURSRY)-wbot
			If ((x .ne. xo) .or. (y .ne. yo)) Then
				vals(1) = float(x - XC)/float(XW)
				vals(2) = float(y - YC)/float(YW)
				if (vals(1) .lt. -1.0) vals(1) = -1.0
				if (vals(2) .lt. -1.0) vals(2) = -1.0
				if (vals(1) .gt. 1.0) vals(1) = 1.0
				if (vals(2) .gt. 1.0) vals(2) = 1.0
				vals(3) = 1.0 - (vals(1)**2 + vals(2)**2)
				If (vals(3) .gt. 0.0) Then
					vals(3) = sqrt(vals(3))
				Else 
					vals(3) = 0.0
				Endif
				Call frontb(.true.)
				Call draw_l_vector(vals)
				Call gfflush
				Call frontb(.false.)
			Endif
			xo = x
			yo = y
		Enddo
		If (lon .eq. 1) Then
			Call send_l_message(IPC_L_INTRINSIC,lon,1,vals,5)
		Endif
		goto 10
	Endif
C
C       handle redraw
C
250     If (device .eq. REDRAW) then    ! redraw screen
C
300	Format("Voxel intensity:",F4.2)
301	Format("Light intensity:",F4.2)
		write(tstr,300) vals(5)
		dialog(lmod_vi_label).text = tstr
		write(tstr,301) vals(4)
		dialog(lmod_li_label).text = tstr
C
                Call RESHAP     ! do not forget !!!
                Call ortho2(0.0,300.0,0.0,130.0)
                Call DRAW_DLG(dialog,status)
                If (status .ne. 0) then
                         goto 999
                Endif
		Call draw_l_vector(vals)
		Call swapbu
	Endif
        Call HANDLE_DLG(device,data,dialog,status)
        If (status .ne. 0) goto 999
        If (device .eq.  DLG_NOPEVNT) goto 10
C
C       handle buttons
C
        If (device .eq.  DLG_BUTEVNT) then
		If (data .eq. lmod_postlight) then
			Call send_l_message(IPC_L_POSTLIGHT,i,0,vals,4)
		Else if (data .eq. lmod_done) then
			goto 999
		Endif
		Call qenter(REDRAW,wid)
		goto 10
	Endif
C
C	handle check box
C
	If (device .eq.  DLG_CHKEVNT) then
		If (data .eq. lmod_intrinsic) then
			lon = dialog(lmod_intrinsic).aux
			Call send_l_message(IPC_L_INTRINSIC,lon,1,vals,5)
		Endif
		goto 10
	Endif
C
C       handle menu events
C
        If (device .eq. DLG_POPEVNT) then
		If (data .eq. lmod_menu) Then
			j = dialog(data).tpos
			Call send_l_message(IPC_L_FILTER,filters(j),1,i,0)
		Endif
		goto 10
	Endif
C
C       handle scrolling events
C
        If (device .eq. DLG_BAREVNT) then
		i = dialog(data).aux
		If (data .eq. lmod_VI_bar) Then
			vals(5) = float(i)/100.0
		Else if (data .eq. lmod_LI_bar) Then
			vals(4) = float(i)/100.0
		Endif
		If (lon .eq. 1) Then
			Call send_l_message(IPC_L_INTRINSIC,lon,1,vals,5)
		Endif
		Call qenter(REDRAW,wid)
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
C
C	Send The IPC message
C
	Subroutine	send_l_message(cmd,a1,n1,a2,n2)

	Implicit None

	Include '../libs/vt_inc.f'

	Integer*4	cmd
	Integer*4	a1(*),a2(*),n1,n2

	Integer*4	msg(65),I,J

	i = 1
	msg(i) = cmd
	Do j=1,n1
		i = i + 1
		msg(i) = a1(j)
	Enddo
	Do j=1,n2
		i = i + 1
		msg(i) = a2(j)
	Enddo
	Call send_ipc(i,msg)

	Return
	End
C
C	Routine to draw the vector
C
	Subroutine	draw_l_vector(v)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	
	Real*4		v(*)
	Integer*4	p1(2),p2(2),p3(2),p4(2),h

C	Box definition

	Integer*4	XC,YC,XW,YW
	Parameter	(XC=260)
	Parameter	(YC=90)
	Parameter	(XW=35)
	Parameter	(YW=35)

	p1(1) = XC-XW
	p1(2) = YC-YW
	p2(1) = XC+XW
	p2(2) = YC-YW
	p3(1) = XC+XW
	p3(2) = YC+YW
	p4(1) = XC-XW
	p4(2) = YC+YW

	h = '00545454'X
	Call cpack(h)

	Call bgnpol
	Call v2i(p1)	
	Call v2i(p2)	
	Call v2i(p3)	
	Call v2i(p4)	
	Call endpol

	h = '00ffffff'X
	Call cpack(h)

	Call bgnclo
	Call v2i(p1)	
	Call v2i(p2)	
	Call v2i(p3)	
	Call v2i(p4)	
	Call endclo
	
	p1(1) = XC + v(1)*XW
	p1(2) = YC + v(2)*YW
	p2(1) = XC
	p2(2) = YC

	Call bgnlin
	Call v2i(p1)
	Call v2i(p2)
	Call endlin

	Return
	End
