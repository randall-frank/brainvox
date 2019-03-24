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
C    MODULE NAME    : Texture functions update
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
C    DESCRIPTION    :  	Routines to Change the current texture functions
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
	Subroutine update_texture_functions(why)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'  
	Include '../libs/vt_inc.f'
	Include 'palettes.inc'
	Include 'windows.inc'

	Character*200	map1,map2,tstr
	Integer*4	map1num,map2num
	Integer*4	rtable(65536),gtable(65536),btable(65536)
	Integer*4	i,why
	Integer*4       message(IPC_SHM_SIZE)
C
	If (why .eq. 1) Then
C
C	Get the function strings
C
	Call texture_fun_io(IPC_READ,map1,map2)
	Call texfunc_to_num(map1,map1num,0,tstr)
	Call texfunc_to_num(map2,map2num,0,tstr)
C
C	get new tables
C
	Call eval_texture_num(map1num,rtable,pal_2d(1),texture_c(1),
     +				pal_2d(769),texture_o(1),i)
	If (i .ne. 0) goto 100
	Call eval_texture_num(map1num,gtable,pal_2d(257),texture_c(257),
     +				pal_2d(769),texture_o(1),i)
	If (i .ne. 0) goto 100
	Call eval_texture_num(map1num,btable,pal_2d(513),texture_c(513),
     +				pal_2d(769),texture_o(1),i)
	If (i .ne. 0) goto 100
	
	Do i=1,65536
		blend_map1(i)=rtable(i)+256*gtable(i)+256*256*btable(i)
	Enddo

100	Continue

	Call eval_texture_num(map2num,rtable,pal_2d(1),texture_c(1),
     +				pal_2d(769),texture_o(1),i)
	If (i .ne. 0) goto 200
	Call eval_texture_num(map2num,gtable,pal_2d(257),texture_c(257),
     +				pal_2d(769),texture_o(1),i)
	If (i .ne. 0) goto 200
	Call eval_texture_num(map2num,btable,pal_2d(513),texture_c(513),
     +				pal_2d(769),texture_o(1),i)
	If (i .ne. 0) goto 200
	
	Do i=1,65536
		blend_map2(i)=rtable(i)+256*gtable(i)+256*256*btable(i)
	Enddo

200	Continue

	Endif
C
C	Touch those 2D windows which need redrawing...
C
	Do i=1,MAX_WINDOWS
		If (wind2d(i).wid .ne. -1) Then
			If (wind2d(i).texture_mode .ne. 0) Then
				Call dirty_window(i,1)
				Call qenter(REDRAW,wind2d(i).wid)
			Endif
		Endif
	Enddo
C
C	Update the texture map shared memory block
C
	Call vt_write_tex_fun_map(1,blend_map1)
	Call vt_write_tex_fun_map(2,blend_map2)
C
C	Update the LUT map shared memory block
C
	Do i=1,256
		rtable(i)=pal_2d(i)+pal_2d(i+256)*256
     +			+pal_2d(i+512)*256*256+ishft(pal_2d(i+768),-24)
		gtable(i)=palette(i)+palette(i+256)*256
     +			+palette(i+512)*256*256+ishft(opacity(i),-24)
		btable(i)=texture_c(i)+texture_c(i+256)*256
     +			+texture_c(i+512)*256*256+ishft(texture_o(i),-24)
C		rtable(i)=pal_2d(i)+pal_2d(i+256)*256
C     +			+pal_2d(i+512)*256*256+lshift(pal_2d(i+768),24)
C		gtable(i)=palette(i)+palette(i+256)*256
C     +			+palette(i+512)*256*256+lshift(opacity(i),24)
C		btable(i)=texture_c(i)+texture_c(i+256)*256
C     +			+texture_c(i+512)*256*256+lshift(texture_o(i),24)
	Enddo
	Call vt_write_lut_map(1,rtable) ! Image
	Call vt_write_lut_map(2,gtable) ! Voxel
	Call vt_write_lut_map(3,btable) ! Texture
C
	Return

	End
C
C	Routine to compute new texture table
C
	Subroutine eval_texture_num(num,table,IL,TL,IO,TO,err)

	Implicit 	None

	Integer*4	num,table(65536),IL(256),TL(256),IO(256),TO(256)
	Integer*4	err

	Integer*4	TV,VV,j,i
C
C	Fiill in the table
C
	j = 1
	Do TV=1,256
		Do VV=1,256	
			i = 0
			If (num .eq. 1) then   ! image lut
				i = IL(VV)
			Else if (num .eq. 2) then  ! texture LUT
				i = TL(TV)
			Else if (num .eq. 3) then  ! modulate by image opac
				i = IL(VV)*IO(VV)+(TL(TV)*(255-IO(VV)))
				i = i / 255
			Else if (num .eq. 4) then  ! modulate by texture opac
				i = IL(VV)*IO(TV)+(TL(TV)*(255-IO(TV)))
				i = i / 255
			Else if (num .eq. 5) then  ! image*texture
				i = IL(VV)*TL(TV)/255
			Else
				err = 1
				return
			Endif
			table(j) = i
			j = j + 1
		Enddo
	Enddo

	err = 0

	Return
	End
