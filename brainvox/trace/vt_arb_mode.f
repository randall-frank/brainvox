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
C    MODULE NAME    : vt_arb_mode
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
C    DATE           : 4 Aug 95       
C    LINE-BY-LINER  :    
C    APPROVAL       : 
C
C    DESCRIPTION    :  	Routines to control allow the slices to come from
C			arbitrary planes.
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
	Subroutine Set_Arb_Window(show)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'  
        Include '../libs/vt_inc.f'
        Include '../libs/vt_protos.f'
        Include 'tr_inc.f'
        Include 'vt_arb_inc.f'
        Include 'traceuif.inc'
	Include 'tr_windows.inc'

	Integer*4	show,i

	i = show
	If (show .eq. -1) Then
		If (arb_wid .ne. 0) Then
			i = 0
		Else
			i = 1
		Endif
	Endif
	If ((i .ne. 0) .and. (arb_wid .eq. 0)) Then

		If (arb_mode .eq. 0) Then
			new_settings = cur_settings(1)
			undo_settings = new_settings
		Else
			new_settings = cur_settings(arb_mode)
			undo_settings = new_settings
		Endif
		Call prefsi(320,155)
		arb_wid=winope('Slice mode',10)
C		Call prefsi(320,155)
        	Call rgbmod
        	Call doubleb
        	Call Gconfi
		Call Update_Arb_Window(1)

	Else if ((i .eq. 0) .and. (arb_wid .ne. 0)) Then

		Call winclo(arb_wid)
		arb_wid = 0

	Endif
	
	Return
	End
C
	Subroutine Switch_Arb_Mode

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'  
        Include '../libs/vt_inc.f'
        Include '../libs/vt_protos.f'
        Include 'tr_inc.f'
        Include 'vt_arb_inc.f'
        Include 'traceuif.inc'
	Include 'tr_windows.inc'
C
	Integer*4	message(3)
C
C	Cannot change when cutting
C
	If (cut_dir .eq. 0) Then
		arb_mode = 0
		Return
	Endif
C
C	may need to load additional ROIs
C
	Call Readall
C
C	slider bounds could change
C
	Call Set_Image_Scroll_Limits
C
C	Force a redraw (indirectly)
C
	message(1) = IPC_TRACE_PRIVATE
	message(2) = IPC_T_MODECHANGE
	Call send_ipc(2,message)
C
	Return
	End
C
	Subroutine Handle_Arb_Window(device,data,state)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'  
        Include '../libs/vt_inc.f'
        Include '../libs/vt_protos.f'
        Include 'tr_inc.f'
        Include 'vt_arb_inc.f'
        Include 'traceuif.inc'
C
C	Params
C
	Integer*4	device,state
	Integer*2	data
C
C	local vars
C
	Integer*4	i,error
	Character*100	tstr
C
C	event handlers
C	
	If (device .eq. DLG_BUTEVNT) then
		If (data .eq. tr_arb_close) Then
C
C	Close window
C
			Call Set_Arb_Window(0)
		Else if (data .eq. tr_arb_remove) Then
C
C	Remove sub-mode
C
			If (state .eq. 2) Then
				Call bv_error(
     +				"Cannot change mode while tracing.}",0)
				Return
			Endif
C
C	Remove the current one 
C
			cur_settings(arb_mode).name = "<> "
C
C	set to global
C
			arb_mode = 1
			new_settings = cur_settings(arb_mode)
			undo_settings = new_settings
			Call Tagfile_Arb_IO(IPC_WRITE,error)
			If (error .ne. 0) Then
				Call bv_error(
     +			"Error:unable to save slice mode setings.}",0)
			Endif
			Call Switch_Arb_Mode
			Call Update_Arb_Window(1)
C
		Else if (data .eq. tr_arb_revert) Then
C
C	Recall old settings...
C
			new_settings = undo_settings
			Call Update_Arb_Window(1)

C
		Else if (data .eq. tr_arb_grab) Then
C
C	grab values from brainvox_voxel
C
			Call send_ipc(1,IPC_T_ARBREQUEST)
C
C	trace.f handles the receive end...
C
		Endif
C
	Else if (device .eq. DLG_POPEVNT) then
C
	    If (data .eq. tr_arb_submode) Then
C
C	Change arb submode
C
		i = arb_uif(data).tpos
		If (state .eq. 2) Then
			Call bv_error(
     +				"Cannot change mode while tracing.}",0)
			Return
		Endif
		If (i .eq. 1) Then
			arb_mode = 1
		Else
			Call roiname_to_arbmode(rgnnames(i),arb_mode)
			If (arb_mode .eq. 1) Then
C
C	New arb mode
C
			Call roiname_to_arbmode("<> ",arb_mode)
			If (arb_mode .ne. 1) Then
				cur_settings(arb_mode).name = rgnnames(i)
			Endif

			Endif
		Endif
		new_settings = cur_settings(arb_mode)
		undo_settings = new_settings
		Call Tagfile_Arb_IO(IPC_WRITE,error)
		If (error .ne. 0) Then
			Call bv_error(
     +			"Error:unable to save slice mode setings.}",0)
		Endif
		Call Switch_Arb_Mode
		Call Update_Arb_Window(1)
C
	    Else if (data .eq. tr_arb_mode) Then
C
C	1=slice,2=arb,3=update
C
		i = arb_uif(data).tpos
C
C	Cannot change while tracing
C
		If ((state .eq. 2).and.(i .le. 2)) Then
			Call bv_error(
     +				"Cannot change mode while tracing.}",0)
			Return
		Endif
		If (i .eq. 1) Then
C
C	Change mode
C
			If (arb_mode .ne. 0) Then
C
C	Change to slice mode (discard new settings)
C
				arb_mode = 0
				Call Tagfile_Arb_IO(IPC_WRITE,error)
				If (error .ne. 0) Then
					Call bv_error(
     +				"Error:unable to save slice mode setings.}",0)
				Endif
				Call Switch_Arb_Mode
			Endif
			Call Update_Arb_Window(1)
C
		Else if (i .eq. 2) Then
C
			If (arb_mode .eq. 0) Then
C
C	Change to arbitrary mode (new_settings = cur_settings)
C
				arb_mode = 1
				new_settings = cur_settings(arb_mode)
				undo_settings = new_settings
				Call Tagfile_Arb_IO(IPC_WRITE,error)
				If (error .ne. 0) Then
					Call bv_error(
     +				"Error:unable to save slice mode setings.}",0)
				Endif
				Call Switch_Arb_Mode
			Endif
			Call Update_Arb_Window(1)
C
		Else if (i .eq. 3) Then
C
C	Cannot change while tracing
C
			If (state .eq. 2) Then
				Call bv_error(
     +				"Cannot update settings while tracing.}",0)
				Return
			Endif
C
C	Update the settings
C
			If (arb_mode .ne. 0) Then
				cur_settings(arb_mode) = new_settings
			Endif
			Call Tagfile_Arb_IO(IPC_WRITE,error)
			If (error .ne. 0) Then
				Call bv_error(
     +				"Error:unable to save slice mode setings.}",0)
			Endif
			if (arb_mode .ne. 0) Call Switch_Arb_Mode
		Endif
C
	    Endif
C
	Else if (device .eq.  DLG_EDTEVNT) then
		i = arb_uif(data).tlength
		tstr = arb_uif(data).text(1:i)//" "
		If (data .eq. tr_arb_islice) Then
			Read(tstr,*,err=10,end=10) new_settings.islice
10			If (new_settings.islice .eq. 0.0) Then
				new_settings.islice = 1.0
			Endif
			Write(arb_uif(data).text,11) new_settings.islice
			arb_uif(data).tlength = 7
			Call Update_Arb_Window(0)
11	Format(F7.2)
		Else if (data .eq. tr_arb_nslices) Then
			Read(tstr,*,err=20,end=20) new_settings.num_slices
20			If (new_settings.num_slices .gt. max_images) Then
				new_settings.num_slices = max_images
			Endif
			If (new_settings.num_slices .le. 0) Then
				new_settings.num_slices = 1
			Endif
			Write(arb_uif(data).text,21) new_settings.num_slices
			arb_uif(data).tlength = 7
			Call Update_Arb_Window(0)
		Else if (data .eq. tr_arb_matchslice) Then
			Read(tstr,*,err=20,end=20) new_settings.match_slice
30			Write(arb_uif(data).text,21) new_settings.match_slice
			arb_uif(data).tlength = 7
			Call Update_Arb_Window(0)
21	Format(I7)
		Endif
	Endif

	Return
	End
C
	Subroutine Update_Arb_Window(all)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'  
        Include '../libs/vt_inc.f'
        Include '../libs/vt_protos.f'
        Include 'tr_inc.f'
        Include 'vt_arb_inc.f'
        Include 'traceuif.inc'
	Include 'tr_windows.inc'

	Integer*4	all,i,strlen
	External	strlen
C
C	Update the dialog items
C
	Write(arb_uif(tr_arb_text2).text,10) new_settings.rots(1),
     +		new_settings.rots(2),new_settings.rots(3)
10	Format('Rx= ',I5,' Ry= ',I5,' Rz= ',I5,' ')
	Write(arb_uif(tr_arb_text3).text,11) new_settings.base_slice
11	Format('Base slice:',I4)
C
	If (arb_mode .ne. 0) Then
		arb_uif(tr_arb_mode).text = 'Arbitrary'
		arb_uif(tr_arb_mode).tlength = 9
		arb_uif(tr_arb_submode).text=cur_settings(arb_mode).name
		i = strlen(arb_uif(tr_arb_submode).text)
		if (i .gt. 12) i = 12
		arb_uif(tr_arb_submode).tlength = i
	Else 
		arb_uif(tr_arb_mode).text = 'Slices'
		arb_uif(tr_arb_mode).tlength = 6
	Endif
C
	If (all .eq. 1) Then
		Write(arb_uif(tr_arb_nslices).text,21) 
     +			new_settings.num_slices
		arb_uif(tr_arb_nslices).tlength = 7
		Write(arb_uif(tr_arb_matchslice).text,21) 
     +			new_settings.match_slice
		arb_uif(tr_arb_matchslice).tlength = 7
21	Format(I7)
		Write(arb_uif(tr_arb_islice).text,31) 
     +			new_settings.islice
		arb_uif(tr_arb_islice).tlength = 7
31	Format(F7.2)
	Endif
C
C	Disable/enable items if not in arb mode
C
	If (arb_mode .eq. 0) Then
		arb_uif(tr_arb_islice).dtype = DLG_NOP
		arb_uif(tr_arb_nslices).dtype = DLG_NOP
		arb_uif(tr_arb_matchslice).dtype = DLG_NOP
		arb_uif(tr_arb_grab).dtype = DLG_NOP
		arb_uif(tr_arb_submode).dtype = DLG_NOP
	Else
		arb_uif(tr_arb_islice).dtype = DLG_EDIT
		arb_uif(tr_arb_nslices).dtype = DLG_EDIT
		arb_uif(tr_arb_matchslice).dtype = DLG_EDIT
		arb_uif(tr_arb_grab).dtype = DLG_BUTTON
		arb_uif(tr_arb_submode).dtype = DLG_MENU
	Endif
C
	arb_uif(tr_arb_remove).dtype = DLG_NOP
	If (arb_mode .gt. 1) Then
		arb_uif(tr_arb_remove).dtype = DLG_BUTTON
	Endif
C
C	Force a redraw
C
	Call qenter(REDRAW,arb_wid)
C
	Return
	End
C
	Subroutine Init_Arb_Window

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'  
        Include '../libs/vt_inc.f'
        Include '../libs/vt_protos.f'
        Include 'tr_inc.f'
        Include 'vt_arb_inc.f'
        Include 'traceuif.inc'
	Include 'tr_windows.inc'
C
	Integer*4	error,i,j
C
C	Init to defaults
C
	Call arb_init_defaults
C
C	Try to read values from disk
C
	Call Tagfile_Arb_IO(IPC_READ,error)
C
C	If cutting, we cannot be in arb mode 
C
	If (cut_dir .eq. 0) arb_mode = 0
C
C	Does the original ROI exist?
C
	Do i=2,max_rgns
		If (cur_settings(i).name .ne. "<>") Then
			Call name_to_rgnindex(cur_settings(i).name,j)
			If (j .lt. 2) Then
C
C	Remove the setting (that does not match an ROI)
C
				cur_settings(i).name = "<>"
				If (arb_mode .eq. i) arb_mode = 1
			Endif
		Endif
	Enddo
C
	If (arb_mode .ne. 0) Call Switch_Arb_Mode
C
	Return
	End
C
C	Get the arb tagfile name
C
	Subroutine Tagfile_Arb_IO(cmd,err)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'  
        Include '../libs/vt_inc.f'
        Include '../libs/vt_protos.f'
        Include '../libs/tag_inc.f'
        Include 'tr_inc.f'
        Include 'vt_arb_inc.f'

	Integer*4	cmd,err

	Character*256   filename
	Character*100   roi,dir
	
	Integer*4	i,strlen
	External	strlen
C
C       Get the filename
C
        Call indexnum_to_roi(cut_dir,roi,i)
        Call roi_to_dir(roi,dir)
	filename=dir(1:strlen(dir))//"/Trace_Slicing.tag"//char(0)

	Call Tagfile_Arb_IO_file(filename,cmd,err)

	Call Reface_Arb_Submode

	Return
	End
C
C	Routine to set the image scollbar bounds
C
	Subroutine Set_Image_Scroll_Limits

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'  
        Include '../libs/vt_inc.f'
        Include 'tr_inc.f'
        Include 'vt_arb_inc.f'

	Integer*4	i,in_slice
C
	in_slice = cur_slice
C
	i = realimages
	If (arb_mode .ne. 0) i = cur_settings(arb_mode).num_slices 
        If (skip .eq. 2) then
                state1(9).tlength=(i/2)-1
        Else   ! single stepping...
                state1(9).tlength=(i)-1
        Endif
C
C	Set current slice (may need to truncate)
C
	If (cur_slice .gt. ((state1(9).tlength*skip)+1)) Then
		cur_slice = (state1(9).tlength*skip)+1
	Endif
	state1(9).aux=((cur_slice-1)/skip)

	Call tr_read_images(cur_slice,PD,T2)
	If (arb_mode .eq. 0) Then
		write(state1(17).text,8) cur_slice
		state1(17).tlength=18
	Else
		write(state1(17).text,9) cur_slice + 
     +			cur_settings(arb_mode).match_slice - 1
		state1(17).tlength=19
	Endif
8       Format('Original slice:',I3.3)
9       Format('Arbitrary slice:',I3.3)
C
	Do I=1,numrgns
		Call tr_read_roi(I)
	Enddo
C
	Return
	End
C
	Subroutine arb_init_defaults

	Implicit none

        Include 'fgl.h'
        Include 'fdevice.h'
        Include 'sgiuif.h' 
        Include '../libs/vt_inc.f'
        Include '../libs/vt_protos.f'
        Include 'tr_inc.f'
        Include 'vt_arb_inc.f'
        Include 'traceuif.inc'
        Include 'tr_windows.inc'

	Integer*4	i

	Do i=1,max_rgns
		cur_settings(i).islice = interslice
		cur_settings(i).base_slice = -(realimages/2)
		cur_settings(i).num_slices = realimages
		cur_settings(i).rots(1) = 0.0
		cur_settings(i).rots(2) = 0.0
		cur_settings(i).rots(3) = 0.0
		cur_settings(i).match_slice = 1
		cur_settings(i).name="<>"
	Enddo
	cur_settings(1).name="Global"
	new_settings = cur_settings(1)
	undo_settings = cur_settings(1)
	arb_mode = 0
        oldslices = 0

	Return
	End
C
C	Routine to "reface" the arb mode popup menu items
C
	Subroutine	Reface_Arb_Submode

	Implicit None

        Include 'fgl.h'
        Include 'fdevice.h'
        Include 'sgiuif.h' 
        Include '../libs/vt_inc.f'
        Include '../libs/vt_protos.f'
        Include 'tr_inc.f'
        Include 'vt_arb_inc.f'
        Include 'traceuif.inc'

	Character*100	text,out
	Integer*4	i,j,arb,strlen
	External strlen

	Do i=2,numrgns
		text = rgnnames(i)//" "
		j = strlen(text)
		Call roiname_to_arbmode(text,arb)
		If (arb .gt. 1) Then
			out = "<"//text(1:j)//"> "
		Else
			out = text
		Endif
		j = strlen(out)
		write(out(j+1:j+4),664) i
664     Format("%x",I2.2)
		Call change(arb_uif(tr_arb_submode).aux,i,out,j+4,0)
	Enddo

	Return
	End
C
C	Save all slices to disk
C
	Subroutine Save_Slices(dir)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'  
        Include '../libs/vt_inc.f'
        Include 'tr_inc.f'
        Include 'vt_arb_inc.f'

	Character*(*) dir
	Character*256 tmp,file
	Integer*4	j,i,save_slice,num
	Logical		error,found
C
	Integer*2	I1(512*512),I2(512*512)
C
	i = index(dir,char(0))
        i = i - 1
C
	j = 0
        found = .true.
	tmp = dir(1:i)//"/slices_%%%_001.pic"
	Do while (found)
		j = j + 1
		Call name_changer(tmp,j,file,error)
		If (error .or. (j .gt. 999)) Then
			write(*,*) "Unable to find valid filename template"
			Return
		Endif
		inquire(exist=found,file=file)
		write(*,*) found
	Enddo
C
	tmp = dir(1:i)//"/slices_%%%"
	Call name_changer(tmp,j,file,error)
	tmp = file(1:i+11)//"_%%%.pic"//char(0)
C
	i = index(tmp,char(0))
        i = i - 1
C
	save_slice = cur_slice
	num = realimages
	If (arb_mode .ne. 0) num = cur_settings(arb_mode).num_slices 
C
	Do cur_slice=1,num
C
		Call draw_status_bar(float(cur_slice)/float(num),
     +					"Saving images...}");
		Call name_changer(tmp,cur_slice,file,error)
		write(*,*) "Saving:",file(1:i)
C
C	Set current slice
C
		Call tr_read_images(cur_slice,I1,I2)
C
		If (PD_l) Then ! I1
			Call save256(I1,%val(512),file)
		Else ! I2
			Call save256(I2,%val(512),file)
		Endif
C
	Enddo
C
	cur_slice = save_slice

	Call qenter(REDRAW,0)

	Return
	End

