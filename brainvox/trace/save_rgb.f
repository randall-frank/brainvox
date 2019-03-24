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
C    MODULE NAME    : save_rgb	
C    VERSION        : 1.0 
C    LOCATION       :  
C    LAST EDIT      :  
C    STATUS         :  (Development,Testing,Production,Obsolete)
C    LANGUAGE       : FORTRAN
C    PACKAGE        : brainvox_voxel
C
C
C    -- CREATION --------------                                                 
C                                                                               
C    PROGRAMMER     : rjf
C    DATE           : 20 Sep 89       
C    LINE-BY-LINER  :    
C    APPROVAL       : 
C
C    DESCRIPTION    :  	
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
	Subroutine save_rgb(temp,istmp)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'fget.h'
C
	Integer*4	array4(1024*1024)
	Integer*2	array2(1024*1024),r,g,b
	Integer*4	dx,dy,istmp
C
	Character*(*)	temp
	Character*256	filename	
	Character*255	cmd
C
	Integer*4	I,j,h,strlen
	logical		error,found
	External strlen
C
C	look for valid filename
C
	if (istmp .ne. 0) then
		I = 0
		found = .true.
		Do while(found)
			i = i + 1
			Call name_changer(temp,i,filename,error)
			if (error) then
	write(*, *) "Unable to find valid filename to save image."
				return
			Endif
			inquire(exist=found,file=filename)
		Enddo
	else
		filename = temp
	endif
C
C	zero terminate
C
	filename(strlen(filename)+1:strlen(filename)+1) = char(0)
C
C	get window extents
C
	Call getsiz(dx,dy)
	if (dx*dy .gt. 1024*1024) then
		write(*, *)"Sorry maximum image size is 768x768"
		return
	Endif
C
C	set readsource
C
	call readso(SRCFRO)
C
C	get the pixels
C
	If ((getdis() .eq. DMSING).or.(getdis() .eq. DMDOUB)) then
C
C	read color map mode
C
		i = rectre(0,0,dx-1,dy-1,array2)
		Do i = 1,dx*dy
			h = '0fff'X
			j = iand(int(array2(i)),h)
			call getmco(j,r,g,b)
			array4(i) = r + (256 * g) + (65536 * b)
		Enddo
	Else
C
C	read RGB mode
C
		i = lrectr(0,0,dx-1,dy-1,array4)
	Endif
C
C	replace readsource
C
	call readso(SRCAUT)
C
C	save the image to disk
C
	call svimg(filename,array4,%val(dx),%val(dy),%val(3),
     +		%val(1),%val(0),%val(0),%val(dx),%val(dy),i)
C
	write(*, 11)filename(1:index(filename,char(0))-1)
11	Format("Image saved:",A)
C
	Return
	End
