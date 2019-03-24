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
C    MODULE NAME    : vt_arb_mode_io
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
C    DESCRIPTION    :  	Arbitrary mode Tagfile I/O
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
C	Read an ARB tagfile
C
	Subroutine Tagfile_Arb_IO_file(filename,cmd,err)

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

	Integer*4	i,update,j,k,strlen
	Record /FTag/   tags((max_rgns+1)*10)
	Character*256	filename,note
	Character*100	roi,dir
	External strlen
C
C       I=i*4,F=R*4,W=word,S=string
C
C	Global
C
        i = 0
	j = 1
	i = i + 1
        tags(i).name = "I_BV_ARBMODE"//char(0)
        Call get_ref(arb_mode,tags(i).data)
        i = i + 1
        tags(i).name = "F_BV_ISLICE"//char(0)
        Call get_ref(cur_settings(j).islice,tags(i).data)
        i = i + 1
        tags(i).name = "I_BV_ROTX"//char(0)
        Call get_ref(cur_settings(j).rots(1),tags(i).data)
        i = i + 1
        tags(i).name = "I_BV_ROTY"//char(0)
        Call get_ref(cur_settings(j).rots(2),tags(i).data)
        i = i + 1
        tags(i).name = "I_BV_ROTZ"//char(0)
        Call get_ref(cur_settings(j).rots(3),tags(i).data)
        i = i + 1
        tags(i).name = "I_BV_BASESLICE"//char(0)
        Call get_ref(cur_settings(j).base_slice,tags(i).data)
        i = i + 1
        tags(i).name = "I_BV_NUMSLICES"//char(0)
        Call get_ref(cur_settings(j).num_slices,tags(i).data)
        i = i + 1
        tags(i).name = "I_BV_MATCHSLICE"//char(0)
        Call get_ref(cur_settings(j).match_slice,tags(i).data)
        i = i + 1
        tags(i).name = "W_BV_NAME"//char(0)
	k = strlen(cur_settings(j).name)
	cur_settings(j).name(k+1:k+1) = char(0)
        Call get_ref(cur_settings(j).name,tags(i).data)
C
C	Each sub_arb_mode
C
	Do j=2,max_rgns
		write(roi,10) j
10	Format(I2.2)
        	i = i + 1
        	tags(i).name = "F_BV_ISLICE"//roi(1:2)//char(0)
        	Call get_ref(cur_settings(j).islice,tags(i).data)
       	 	i = i + 1
        	tags(i).name = "I_BV_ROTX"//roi(1:2)//char(0)
        	Call get_ref(cur_settings(j).rots(1),tags(i).data)
        	i = i + 1
        	tags(i).name = "I_BV_ROTY"//roi(1:2)//char(0)
        	Call get_ref(cur_settings(j).rots(2),tags(i).data)
        	i = i + 1
        	tags(i).name = "I_BV_ROTZ"//roi(1:2)//char(0)
        	Call get_ref(cur_settings(j).rots(3),tags(i).data)
        	i = i + 1
        	tags(i).name = "I_BV_BASESLICE"//roi(1:2)//char(0)
        	Call get_ref(cur_settings(j).base_slice,tags(i).data)
        	i = i + 1
        	tags(i).name = "I_BV_NUMSLICES"//roi(1:2)//char(0)
        	Call get_ref(cur_settings(j).num_slices,tags(i).data)
        	i = i + 1
        	tags(i).name = "I_BV_MATCHSLICE"//roi(1:2)//char(0)
        	Call get_ref(cur_settings(j).match_slice,tags(i).data)
        	i = i + 1
        	tags(i).name = "W_BV_NAME"//roi(1:2)//char(0)
		k = strlen(cur_settings(j).name)
		cur_settings(j).name(k+1:k+1) = char(0)
        	Call get_ref(cur_settings(j).name,tags(i).data)
	Enddo
C
C	do the operation 
C
	If (cmd .eq. IPC_READ) Then
		Call read_tagged_file(filename,i,tags,err)
	Else if (cmd .eq. IPC_WRITE) Then
		note = "Brainvox_arbitrary_settings"//char(0)
	        update = 0
        	Call write_tagged_file(filename,note,update,i,tags,err)
	Endif
C
C	Cleanup name fields (remove zeros)
C
	Do j=1,max_rgns
		k = index(cur_settings(j).name,char(0))
		If (k .ne. 0) then
		    cur_settings(j).name(k:k)=' '
		Endif
	Enddo
C
	Return
	End
C
C	Routine to match an ROIname up with an arb setting
C
	Subroutine roiname_to_arbmode(roi,arb)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'  
        Include '../libs/vt_inc.f'
        Include '../libs/vt_protos.f'
        Include '../libs/tag_inc.f'
        Include 'tr_inc.f'
        Include 'vt_arb_inc.f'

	Integer*4	arb
	Character*(*)	roi

	Integer*4	i,j,strlen
	External strlen

	arb = 1  !default

	j = strlen(roi)

	Do i=2,max_rgns
		If (cur_settings(i).name(1:j) .eq. roi(1:j)) Then
			arb = i
			Return
		Endif
	Enddo

	Return
	End
C
C	Routine to convert an ROI template name to an ROI index
C
	Subroutine name_to_rgnindex(name,roin)
	
	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'  
        Include '../libs/vt_inc.f'
        Include '../libs/vt_protos.f'
        Include '../libs/tag_inc.f'
        Include 'tr_inc.f'
        Include 'vt_arb_inc.f'

	Integer*4	arb

	Character*(*) 	name
	Integer*4	roin

	Integer*4	i

	roin = 0  !default

	Do i=1,max_rgns
		If (name .eq. rgnnames(i)) Then
			roin = i
			Return
		Endif
	Enddo

	Return
	End
