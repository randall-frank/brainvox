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
C    MODULE NAME    : Texture options control dialog
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
C    DATE           : 16 Nov 94        
C    LINE-BY-LINER  :    
C    APPROVAL       : 
C
C    DESCRIPTION    :  	Routines to handle the Texture control	dialog
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
	Subroutine texture_control(data_id,rgnsfile)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'  
	Include '../filer/filer_inc.inc'
	Include '../libs/vt_inc.f'
	Include 'voxeluif.inc'
	
	Integer*4	data_id,status
	Record /ditem/  dialog(100)
C
C       IPC defines
C
        Integer*4       size,message(IPC_SHM_SIZE)
C
	Character*256	tstr,tempstr
	Character*(*)	rgnsfile
	Integer*4	device,i,j,wid,tag_value1,tag_value2
	Real*4		r_tag_value1,r_tag_value2
	Integer*2	data
	Integer*4	x,y,xo,yo,wleft,wbot
	Integer*4	interp_load,num_map1,num_map2
	Integer*4	flags(10)
	Character*256	dir_name,temp_name,lut_map1,lut_map2
	logical		error,found
	Integer*4	selected_roiname,target_volume,target_bits
	Integer*4	target_autoscale
	Integer*4	arblist(max_rgns),strlen
	External strlen
C
C       I am NOT the shared memory owner!!! (there can be only one!!)
C
	shm_data = data_id
        shmem_owner = 0
        Call vt_init_shmem
C
C       get the window
C
        call prefsi(450,385)
        wid=winope("Texture option",14)
        Call doubleb
        Call RGBmod
        Call gconfi
        Call qdevic(WINQUI)
        Call INIT_DLG(status)
        If (status .ne. 0) Call bv_exit(1)
        Call GET_DLG_COMP('voxeluif.r',5,100,dialog,status)
        If (status .ne. 0) Call bv_exit(1)
C
C	Read the rgns names
C
	numrgns=0
	Call read_rgns(rgnsfile,error)
C
C       Menus 
C
	Call qdevic(MOUSE2)
	Call qdevic(TIMER0)
	Call noise(TIMER0,20)  !every 1/3 seconds
	Call qenter(REDRAW,wid)
C
C	Defaults
C
	selected_roiname = 1
	tag_value1 = 255
	r_tag_value1 = 255.0
	tag_value2 = 255
	r_tag_value2 = 255.0
	interp_load = 1
	dir_name = "."
	temp_name = "vol_%%%.pic"
	Call texture_fun_io(IPC_READ,lut_map1,lut_map2)
	Call texfunc_to_num(lut_map1,num_map1,0,tstr)
	Call texfunc_to_num(lut_map2,num_map2,0,tstr)
C
C	Build clipping menus
C
	dialog(tex_cl_clip).aux = newpup()
	dialog(tex_cl_fill).aux = newpup()
	tstr = "Clip region %t|to 0 %x1|to 255 %x2|to value1 %x3%l|"
     +       //"Set bit 7 %x4| Clear bit 7 %x5}"
	Call addtop(dialog(tex_cl_clip).aux,tstr,index(tstr,"}")-1,0)
	tstr = "Fill volume %t|to 0 %x1|to 255 %x2|to value1 %x3%l|"
     +       //"Set bit 7 %x4| Clear bit 7 %x5}"
	Call addtop(dialog(tex_cl_fill).aux,tstr,index(tstr,"}")-1,0)
C
C	Build volume type menus
C
	dialog(bv_depth_menu).aux = newpup()
	tstr = "8bit %x1|16bit %x2|Float %x4}"
	Call addtop(dialog(bv_depth_menu).aux,tstr,
     +		index(tstr,"}")-1,0)
        dialog(bv_depth_menu).text = "Type:8bit "
        If (volumedepth .eq. 1) Then
                dialog(bv_min_edit).dtype = DLG_NOP
                dialog(bv_max_edit).dtype = DLG_NOP
        Else
                dialog(bv_min_edit).dtype = DLG_EDIT
                dialog(bv_max_edit).dtype = DLG_EDIT
        Endif
        If (volumedepth .eq. 2) Then
                dialog(bv_depth_menu).text = "Type:16bit"
        Else if (volumedepth .eq. 4) Then
                dialog(bv_depth_menu).text = "Type:float"
        Endif
599     format(f10.5)
        write(dialog(bv_min_edit).text,599) volume_lims(1)
        dialog(bv_min_edit).tlength = 10
        write(dialog(bv_max_edit).text,599) volume_lims(2)
        dialog(bv_max_edit).tlength = 10
c
        dialog(bv_zswap_chk).aux = volume_zswap
        dialog(bv_bswap_chk).aux = volume_bswap
C
C	More menus
C
	dialog(tex_map1).aux = newpup()
	dialog(tex_map2).aux = newpup()
	Do i=1,5
		Call texfunc_to_num(tstr,i,1,tempstr)
		j = 200
		Call str_len(tempstr,j)
		write(tstr,30) tempstr(1:j),i+100
30	Format(A,"%x",I3.3,"}")
		Call addtop(dialog(tex_map1).aux,tstr,index(tstr,"}")-1,0)
		Call addtop(dialog(tex_map2).aux,tstr,index(tstr,"}")-1,0)
	Enddo
	Call	new_lut_funcs(lut_map1,num_map1,lut_map2,num_map2,dialog)
C
C	Volume I/O options
C
	target_volume = 1
	dialog(tex_loadoptions).aux = newpup()
	tstr = "Texture Volume %I%x1%r1|Image Volume %i%r1%x2%l}"
	Call addtop(dialog(tex_loadoptions).aux,tstr,index(tstr,"}")-1,0)
C
	target_bits = 8
	tstr = "Load as 8bits %I%x11%r2|Load as upper 1bit %i%r2%x4}"
	Call addtop(dialog(tex_loadoptions).aux,tstr,index(tstr,"}")-1,0)
	Do i=2,7
		write(tstr,31) i,i+3
31	Format("Load as upper ",I1.1,"bits %i%r2%x",I3.3,"}")
		Call addtop(dialog(tex_loadoptions).aux,tstr,
     +			index(tstr,"}")-1,0)
	Enddo
C
	target_autoscale = 0
	tstr = "Bitslice autoscale %i%x15}"
	Call addtop(dialog(tex_loadoptions).aux,tstr,index(tstr,"}")-1,0)
C
C	volume OPs menu
C
	dialog(tex_cl_options).aux = newpup()
	tstr = "Volume Operations %t|Divide Texture Volume by 2 %x1|"
     +	     //"Multiply Texture and Image Volumes %x2|"
     +	     //"Multiply Texture Volume by Value1 %x4}"
	Call addtop(dialog(tex_cl_options).aux,tstr,index(tstr,"}")-1,0)
	tstr = "Add Texture and Image Volumes %x5|"
     +	     //"Add Value1 to Texture Volume %l%x6}"
	Call addtop(dialog(tex_cl_options).aux,tstr,index(tstr,"}")-1,0)
	tstr = "Copy Image to Texture Volume %x7|"
     +	     //"Copy Texture to Image Volume %x8|"
     +	     //"Swap Texture and Image Volumes %l%x3|"
     +	     //"Flood Fill Texture Volume %l%x9|"
     +	     //"Recompute Volume Extents %x10}"
	Call addtop(dialog(tex_cl_options).aux,tstr,index(tstr,"}")-1,0)
C
C	2D Trace volume opts menu (tex_cl_trace_opts)
C
	Call Check_arb_mode(arblist)
	x = newpup()
        Do i = 1, numrgns
		j = index(rgnnames(i)," ")
		If (arblist(i) .ne. 0) Then
                	tstr = "<"//rgnnames(i)(1:j-1)//"> "
		Else 
                	tstr = rgnnames(i)(1:j-1)//" "
		Endif
                j = index(tstr," ")
		If (i .eq. selected_roiname) Then
                	write(tstr(j+1:j+11),665) i+1000
		Else
                	write(tstr(j+1:j+11),664) i+1000
		Endif
664     Format("%i%r1%x",I4.4)
665     Format("%I%r1%x",I4.4)
                Call addtop(x,tstr,j+11,0)
        Enddo
C	
	dialog(tex_cl_trace_opts).aux = newpup()
	tstr = "2D Trace Operations %t|Select ROI name %l%m|"//
     +	       "Fill with 0 %x1|Fill with 255 %x2|"//
     +	       "Fill to value1 %l%x3|Set bit 7 %x4|Clear bit 7 %l%x5|"//
     +	       "Increment volume %x6|Decrement volume %l%x7}"
	Call addtop(dialog(tex_cl_trace_opts).aux,tstr,index(tstr,"}")-1,x)
	tstr = "MAP-3 controls... %x8}"
	Call addtop(dialog(tex_cl_trace_opts).aux,tstr,index(tstr,"}")-1,0)

C
C	Top of the event loop
C
10      device=qread(data)      ! top of event loop
C
C       timer stuff
C
        if (device .eq. TIMER0) then
                size = IPC_SHM_SIZE
                call check_ipc(size,message)
                If (message(1) .eq. IPC_QUIT) Then
			goto 999
		Else if (message(1) .eq. IPC_F_RETURN) Then
			If (message(2) .eq. FLR_TEX_VOLUME) Then
				Call get_filename(tstr,message(3),i)
				Call send_ipc(1,IPC_OK)
				If (i .eq. 0) goto 10
				j = 1
				x = 0
				Do While (j .le. i)
					If (tstr(j:j) .eq. '/') Then
						x = j + 1
					Endif
					j = j + 1
				Enddo
				If (x .ne. 0) Then
					dialog(tex_template).text =
     +						tstr(x:i)
					dialog(tex_template).tlength = i-x+1
					dialog(tex_template).tpos = i-x+1
	
					dialog(tex_directory).text = 
     +						tstr(1:x-2)
					dialog(tex_directory).tlength=x-2
					dialog(tex_directory).tpos=x-2
				Else
					dialog(tex_template).text = tstr
					dialog(tex_template).tlength = i
					dialog(tex_template).tpos = i

					dialog(tex_directory).text = "."
					dialog(tex_directory).tlength=1
					dialog(tex_directory).tpos=1
				Endif
C
C	Can we find "%%%.pic"?
C
				tstr = dialog(tex_template).text
				Call find_template(tstr)
				dialog(tex_template).text = tstr
				i = strlen(tstr)
				if (i .lt. 0) i = 0
				dialog(tex_template).tlength = i
				dialog(tex_template).tpos = i
				temp_name = tstr
				dir_name = dialog(tex_directory).text
C
				Call qenter(REDRAW,wid)
				goto 10
C
			Else if (message(2) .eq. FLR_REQ_TEX_BASE) Then
				Call get_filename(tstr,message(3),i)
				Call send_ipc(1,IPC_OK)
				If (i .eq. 0) goto 10
				If (tstr(1:7) .eq. 'Replace') then
C
C	Save the texture volume to disk
C
			i = strlen(dir_name)
			j = strlen(temp_name)
			If (j .eq. 0) Then
				Call ringbe
				goto 10
			Endif
			If (i .eq. 0) Then
				tstr = "./"//temp_name(1:j)
			Else
				tstr = dir_name(1:i)//"/"//temp_name(1:j)
			Endif
			Call volume_saveimages(tstr,error,wid,target_volume)
			Call qenter(REDRAW,wid)

				Endif
			Endif	
		Endif
                goto 10
        Endif
C
C       handle redraw
C
        If (device .eq. REDRAW) then    ! redraw screen
C
                Call RESHAP     ! do not forget !!!
                Call ortho2(0.0,450.0,0.0,385.0)
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
100     Format("brainvox_filer ",I," ",I," ",A," . -hide -f ",A," &")
        If (device .eq.  DLG_BUTEVNT) then
		If (data .eq. tex_done) then
			goto 999
		Else if (data .eq. tex_select) then
			write(tstr,100) shm_data,FLR_TEX_VOLUME,
     +				'"Select first file in the volume"',
     +				'"*001.*"'
			Call bv_system_path(tstr)
		Else if (data .eq. tex_loadvol) then
C
C	Load the volume
C
			i = strlen(dir_name)
			j = strlen(temp_name)
			If (j .eq. 0) Then
				Call ringbe
				goto 10
			Endif
			If (i .eq. 0) Then
				tstr = "./"//temp_name(1:j)
			Else
				tstr = dir_name(1:i)//"/"//temp_name(1:j)
			Endif
			error = .false.
C
C	flags 1=flags  2=in_min 3=in_max 4=out_min 5=out_max
C
			flags(1) = target_bits+target_autoscale
			Call volume_loadimages(tstr,interp_load,error,wid,
     +				target_volume,flags)
			If (error) Then
		write(*, *)"An error occurred during texture volume loading."
			Else
				message(1) = IPC_P_BITSLICE
				message(2) = target_volume
				message(3) = target_bits+target_autoscale
				message(4) = flags(4)
				message(5) = flags(5)
				Call send_ipc(5,message)
				Call send_ipc(1,IPC_TEXVOLUME)
			Endif
C
		Else if (data .eq. tex_savevol) then
C
C	Save the volume
C
			i = strlen(dir_name)
			j = strlen(temp_name)
			If (j .eq. 0) Then
				Call ringbe
				goto 10
			Endif
			If (i .eq. 0) Then
				tstr = "./"//temp_name(1:j)
			Else
				tstr = dir_name(1:i)//"/"//temp_name(1:j)
			Endif
C
C	check to see if the files exist already...
C
			Call name_changer(tstr,1,tempstr,error)
			If (error) Then
			Call bv_error("The current filename template "//
     +				"is not valid.}",.false.)
				goto 10
			Endif
			Inquire(file=tempstr,exist=found)
			If (found) then
				Write(unit=tempstr,fmt=1000) shm_data,
     +			FLR_REQ_TEX_BASE,"Replace existing image files?"
1000	Format("brainvox_filer ",I," ",I,' "',A,
     +				'" "Replace|Cancel" -c -b &')
				Call bv_system_path(tempstr)
				goto 10
			Endif
C
C	Save the texture volume to disk
C
			Call volume_saveimages(tstr,error,wid,target_volume)
			Call qenter(REDRAW,wid)
			goto 10
		Endif
		Call qenter(REDRAW,wid)
		goto 10
	Endif
C
C       handle edit text
C
        If (device .eq. DLG_EDTEVNT) then
                i = dialog(data).tlength
		If (data .eq. tex_template) Then
			temp_name = dialog(data).text(1:i)
		Else if (data .eq. tex_directory) Then
			dir_name = dialog(data).text(1:i)
		Else if (data .eq. tex_cl_val1) Then
			read(dialog(data).text(1:i),*,end=603,err=603) 
     +				tag_value1
603			continue
			read(dialog(data).text(1:i),*,end=604,err=604) 
     +				r_tag_value1
604			continue
		Else if (data .eq. tex_cl_val2) Then
			read(dialog(data).text(1:i),*,end=603,err=605) 
     +				tag_value2
605			continue
			read(dialog(data).text(1:i),*,end=604,err=606) 
     +				r_tag_value2
606			continue
		Else if ((data .eq. bv_min_edit).or.(data .eq.
     +			bv_max_edit)) then
			read(dialog(data).text(1:i),*,end=602,err=602)
     +				volume_lims(data-bv_min_edit+1)
602                     write(dialog(data).text,599)
     +				volume_lims(data-bv_min_edit+1)
			dialog(data).tlength = 10
		Endif
		Call qenter(REDRAW,wid)
		goto 10
	Endif
C
C	handle check box
C
	If (device .eq.  DLG_CHKEVNT) then
		If (data .eq. tex_interp) then
			interp_load = 1 - interp_load 
		Else If (data .eq. bv_zswap_chk) then
			volume_zswap = dialog(bv_zswap_chk).aux
		Else If (data .eq. bv_bswap_chk) then
			volume_bswap = dialog(bv_bswap_chk).aux
		Endif
		goto 10
	Endif
C
C       handle menu events
C
        If (device .eq. DLG_POPEVNT) then
		i = dialog(data).tpos
		If (i .le. 0) goto 10
		If (data .eq. tex_cl_clip) Then
			message(1) = IPC_TEXCLIP
			message(2) = IPC_TEX_CL_PAINT
			message(3) = tag_value1
			if (i .eq. 1) Then
				message(3) = 0
			Else if (i .eq. 2) Then
				message(3) = 255
			Else if (i .eq. 4) Then
				message(2) = IPC_TEX_CL_TAG
			Else if (i .eq. 5) Then
				message(2) = IPC_TEX_CL_UNTAG
			Endif
			Call send_ipc(3,message)
			Call qenter(REDRAW,wid)
			goto 10
		Else If (data .eq. tex_cl_fill) Then
			message(1) = IPC_TEXFILL
			message(2) = IPC_TEX_CL_PAINT
			message(3) = tag_value1
			if (i .eq. 1) Then
				message(3) = 0
			Else if (i .eq. 2) Then
				message(3) = 255
			Else if (i .eq. 4) Then
				message(2) = IPC_TEX_CL_TAG
			Else if (i .eq. 5) Then
				message(2) = IPC_TEX_CL_UNTAG
			Endif
			Call send_ipc(3,message)
			Call qenter(REDRAW,wid)
			goto 10
C
C	Volume operations
C
		Else if (data .eq. tex_cl_options) Then
			If (i .eq. 1) Then
				message(1) = IPC_TEXUTIL
				message(2) = IPC_TEX_UTIL_MULTC
				message(3) = 0.5*256.0
				message(4) = r_tag_value2*256.0
				Call send_ipc(4,message)
			Else if (i .eq. 2) then
				message(1) = IPC_TEXUTIL
				message(2) = IPC_TEX_UTIL_MULTV
				message(3) = tag_value1
				message(4) = tag_value2
				Call send_ipc(4,message)
			Else if (i .eq. 3) then
				message(1) = IPC_TEXUTIL
				message(2) = IPC_TEX_UTIL_SWAP
				message(3) = tag_value1
				message(4) = tag_value2
				Call send_ipc(4,message)
			Else if (i .eq. 4) then
				message(1) = IPC_TEXUTIL
				message(2) = IPC_TEX_UTIL_MULTC
				message(3) = r_tag_value1*256.0
				message(4) = r_tag_value2*256.0
				Call send_ipc(4,message)
			Else if (i .eq. 5) then
				message(1) = IPC_TEXUTIL
				message(2) = IPC_TEX_UTIL_ADDV
				message(3) = tag_value1
				message(4) = tag_value2
				Call send_ipc(4,message)
			Else if (i .eq. 6) then
				message(1) = IPC_TEXUTIL
				message(2) = IPC_TEX_UTIL_ADDC
				message(3) = r_tag_value1*256.0
				message(4) = r_tag_value2*256.0
				Call send_ipc(4,message)
			Else if (i .eq. 7) then
				message(1) = IPC_TEXUTIL
				message(2) = IPC_TEX_UTIL_VOL2TEX
				message(3) = tag_value1
				message(4) = tag_value2
				Call send_ipc(4,message)
			Else if (i .eq. 8) then
				message(1) = IPC_TEXUTIL
				message(2) = IPC_TEX_UTIL_TEX2VOL
				message(3) = tag_value1
				message(4) = tag_value2
				Call send_ipc(4,message)
			Else if (i .eq. 9) then
				message(1) = IPC_TEXUTIL
				message(2) = IPC_TEX_UTIL_FFILL
				message(3) = tag_value1
				message(4) = tag_value2
				Call send_ipc(4,message)
			Else if (i .eq. 10) then
				message(1) = IPC_TEXUTIL
				message(2) = IPC_TEX_UTIL_REBOUND
				message(3) = tag_value1
				message(4) = tag_value2
				Call send_ipc(4,message)
			Endif
			goto 10
C
C	Volume I/O options
C
		Else if (data .eq. tex_loadoptions) Then
			If (i .eq. 1) Then
				target_volume = 1
			Else if (i .eq. 2) Then
				target_volume = 0
			Else if ((i .ge. 4).and.(i.le.11)) Then
				target_bits = i-3
			Else if (i .eq. 15) Then
				target_autoscale = 256-target_autoscale
			Endif
			goto 10
C
C	2D Trace options
C
		Else if (data .eq. tex_cl_trace_opts) Then
			If (i .gt. 1000) Then
C	New current ROIname
				selected_roiname = i - 1000
				goto 10
			Else if ((i .ge. 1) .and. (i .le. 7)) Then
C
C	2D ROI texture operation
C
				message(1) = IPC_2D_TEXFILL
				message(2) = IPC_TEX_CL_PAINT
				message(3) = tag_value1
				if (i .eq. 1) Then
					message(3) = 0
				Else if (i .eq. 2) Then
					message(3) = 255
				Else if (i .eq. 4) Then
					message(2) = IPC_TEX_CL_TAG
				Else if (i .eq. 5) Then
					message(2) = IPC_TEX_CL_UNTAG
				Else if (i .eq. 6) Then
					message(2) = IPC_TEX_CL_INCR
				Else if (i .eq. 7) Then
					message(2) = IPC_TEX_CL_DECR
				Endif
				message(4) = selected_roiname
				Call send_ipc(4,message)
				Call qenter(REDRAW,wid)
				goto 10
C
			Else if (i .eq. 8) Then
C
C	MAP-3 interface
C
600	Format("brainvox_map3 ",I," &")
				write(tstr,600) shm_data
				Call bv_system_path(tstr)
				goto 10
			Endif
C
C	Changing the mapping function
C
		Else If (data .eq. tex_map1) Then
			num_map1 = i - 100
			i = 1
		Else if (data .eq. tex_map2) Then
			num_map2 = i - 100
			i = 1
		Else If (data .eq. bv_depth_menu) Then
			volumedepth = i
                dialog(bv_depth_menu).text = "Type:8bit "
                If (volumedepth .eq. 1) Then
                        dialog(bv_min_edit).dtype = DLG_NOP
                        dialog(bv_max_edit).dtype = DLG_NOP
                Else
                        dialog(bv_min_edit).dtype = DLG_EDIT
                        dialog(bv_max_edit).dtype = DLG_EDIT
                Endif
                If (volumedepth .eq. 2) Then
                        dialog(bv_depth_menu).text = "Type:16bit"
                Else if (volumedepth .eq. 4) Then
                        dialog(bv_depth_menu).text = "Type:float"
                Endif
			i = 0
		Endif
		if (i .eq. 1) Then
			Call new_lut_funcs(lut_map1,num_map1,
     +				lut_map2,num_map2,dialog)
			Call texture_fun_io(IPC_WRITE,lut_map1,lut_map2)
			Call send_ipc(1,IPC_TEXFUNCS)
		Endif
		Call qenter(REDRAW,wid)
		goto 10
	Endif
C
C       handle scrolling events
C
        If (device .eq. DLG_BAREVNT) then
		i = dialog(data).aux
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
C	Read/Write the texture functions file
C
	Subroutine texture_fun_io(oper,map1,map2)

	Implicit None

	Integer*4	oper,num
	Character*200	map1,map2
	logical		found
	Character*100	tstr,file
C
C	oper=0=read   oper=1=write
C
	file = "_texturefuncs"
	If (oper .eq. 0) Then
C
C	Assume the worst
C
		map1="VL"
		map2="TL"
		Inquire(exist=found,file=file)
		If (found) Then
C
C	Read the lines
C
			Open(66,file=file,form="formatted",
     +				status="old",err=990)
			Read(66,1,err=992,end=992) tstr
1	Format(A200)
			Read(66,*,err=992,end=990) num
			If (num .ge. 2) Then
				Read(66,1,err=992,end=992) map1
				Read(66,1,err=992,end=992) map2
			Endif
			Close(66,err=990)
		Endif

	Else if (oper .eq. 1) Then
C
C	Write the file
C
		Inquire(exist=found,file=file)
		If (found) Then
		Open(66,file=file,form="formatted",status="old",err=990)
		Else
		Open(66,file=file,form="formatted",status="new",err=990)
		Endif
		Write(66,2,err=994)
2	Format("TextureFunctionFile 1.0")
		Write(66,3,err=994) 2
3	Format(I4)
		Write(66,4,err=994) map1
		Write(66,4,err=994) map2
4	Format(A200)
		Close(66,err=990)

	Else

		write(*, 900)oper
900	Format("Error: undefined texture function operation:",I)

	Endif

	Return

990	write(*,991)
991	Format("Unable to open the texture functions file.")
	Return

992	write(*, 993)
993	Format("Unable to read the texture functions file.")
	Close(66)
	Return

994	Close(66,err=996)
996	write(*, 995)
995	Format("Unable to write the texture functions file.")
	Return
	
	End
C
C	Routine to find the end of a FORTRAN string
C
	Subroutine	str_len(str,len)

	Implicit None

	Character*(*)	str
	Integer*4	i,len

	i = len
	Do while (i .gt. 0)
		If (str(i:i) .ne. ' ') Then
			len = i
			return
		Endif
		i = i - 1
	Enddo

	len = i

	Return
	End
C
C	Find strings like "001.pic" and convert to "%%%.pic"
C	Find "10xxxxxx.roi" and convert to "*xxxxxx.roi"
C
	Subroutine	find_template(str)

	Implicit None

	Character*(*)	str
	Integer*4	i,j
C
C	Check for ".roi" suffix
C
	i = index(str,".roi")
	if (i .ne. 0) Then
C
C	count leading digits
C
		j = 1
		Do While ((str(j:j) .ge. '0') .and. 
     +				(str(j:j) .le. '9'))
			j = j + 1
		Enddo
C
C	If no leading digits, treat like "%%%.pic"
C
		if (j .eq. 1) goto 100
C
C	Replace leading string with '*'
C
		str = "*"//str(j:i+3)//" "
C
		Return
	Endif
C
C	Find ".pic" (or ".flt")
C
	i = index(str,".pic")
	If (i .eq. 0) Then
		i = index(str,".flt")
		if (i .eq. 0) Return
	Endif
C
C	Replace numbers before the suffix with '%'
C
100	continue
	i = i - 1
	Do While (i .gt. 0)
		If ((str(i:i) .ge. '0').and.(str(i:i) .le. '9')) Then
			str(i:i) = '%'
		Else
			Return
		Endif
		i = i - 1
	Enddo
	
	Return

	End
C
C	Convert texturestring to number and vice versa
C
	Subroutine	texfunc_to_num(str,num,which,name)

	Implicit None

	Integer*4	MAX_FUNCS
	Parameter	(MAX_FUNCS = 5)
	Character*(*)	str,name
	Character*200	funcs(MAX_FUNCS),sname(MAX_FUNCS)
	Integer*4	i,which,num

	data	funcs/'IL','TL','IL*(IO)+TL*(255-IO)',
     +			'IL*(TO)+TL*(255-TO)','IL*TL/255'/
	data	sname/'Image Colored','Texture Colored',
     +			'Image Voxel Opacity Weighted',
     +			'Texture Voxel Opacity Weighted',
     +			'Image/Texture Blended'/

	If (which .eq. 0) Then
C
C	from string to number
C
		Do i=1,MAX_FUNCS
			If (index(str,funcs(i)) .ne. 0) Then
				num = i
				Return
			Endif
		Enddo
		num = 1
	Else 
C
C	from number to string
C
		name = sname(1)
		str = funcs(1)
		If (num .GT. MAX_FUNCS) Return
		name = sname(num)
		str = funcs(num)
	Endif

	Return
	End
C
C	Setup the menu text and lut strings from new lut numbers
C
	Subroutine	new_lut_funcs(lut_map1,num_map1,lut_map2,num_map2,
     +				dialog)

	Implicit 	None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'  
	Include '../filer/filer_inc.inc'
	Include '../libs/vt_inc.f'
	Include 'voxeluif.inc'
	
	Record /ditem/  dialog(100)

	Integer*4	j
	Character*255	tempstr
	Integer*4	num_map1,num_map2
	Character*(*)	lut_map1,lut_map2
	
	Call texfunc_to_num(lut_map1,num_map1,1,tempstr)
	j = 200
	Call str_len(tempstr,j)
	dialog(tex_map1).text = tempstr(1:j)
	dialog(tex_map1).tlength = j
	Call texfunc_to_num(lut_map2,num_map2,1,tempstr)
	j = 200
	Call str_len(tempstr,j)
	dialog(tex_map2).text = tempstr(1:j)
	dialog(tex_map2).tlength = j

	Return
	End
C
C	Routine to figure out if an ROI has an arb_mode
C
	Subroutine Check_arb_mode(arblist)

	Implicit None
	
        Include 'fgl.h'
        Include 'fdevice.h'
        Include 'sgiuif.h'
        Include '../libs/vt_inc.f'
        Include '../libs/vt_protos.f'
        Include '../libs/tag_inc.f'

	Integer*4	arblist(max_rgns)
	Record /FTag/   tags(max_rgns+1)

	Character*256   filename
	Character*100   roi,dir,names(max_rgns)
	Integer*4	i,err,j,k,strlen
	External	strlen
C
C	Get the filename
C
	Call indexnum_to_roi(cut_dir,roi,i)
	Call roi_to_dir(roi,dir)
	filename=dir(1:strlen(dir))//"/Trace_Slicing.tag"//char(0)
C
C	define the tags we want
C
	i = 0
	i = i + 1
	tags(i).name = "W_BV_NAME"//char(0)
	Call get_ref(names(i),tags(i).data)
	Do j=2,max_rgns
		write(roi,10) j
10      Format(I2.2)
		i = i + 1
		tags(i).name = "W_BV_NAME"//roi(1:2)//char(0)
		Call get_ref(names(i),tags(i).data)
	Enddo
	Do j=1,max_rgns
		arblist(j) = 0
	Enddo
	Call read_tagged_file(filename,i,tags,err)
	If (err .ne. 0) Return
C
C	look for the roinames
C
	Do i=1,numrgns
		Do j=1,max_rgns
			k = strlen(rgnnames(i))
			If (index(names(j),rgnnames(i)(1:k)) .ne. 0) then
				arblist(i)=1
			Endif
		Enddo
	Enddo
C
C	done
C
	Return
	End
