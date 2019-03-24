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
C    MODULE NAME    : virt    
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
C    DATE           : 20 Sep 89       
C    LINE-BY-LINER  :    
C    APPROVAL       : 
C
C    DESCRIPTION    :  	This file contains all the dynamic memory allocation
C 			and copy routines.. It uses MALLOC & FREE from the
C			sgiuif package...
C                                                                               
C    -- REVISION HISTORY --------------                                         
C
C                                                                               
C    -- GENERAL --------------                                                  
C                                                                               
C    SYNTAX         :     See below...
C    INPUTS         :     ...     
C    OUTPUTS        :     ...
C
C    LIMITATIONS    :      
C    NOTES          :      
C                 
C                                                                               
C    ====================================================================== 
c
c**************************************************************
c
c
c**************************************************************
c
	Subroutine  savetrace(inum,tnum,trace)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'  
	Include '../libs/vt_inc.f'
	Include 'tr_inc.f'  

	Integer*4	tptrs(max_images,max_rgns)

	integer*4	inum,tnum,size,num
	integer*4	trace(4096,2)

	common	/virt/tptrs
c
C	remember that the image number coming in is ODD... so +1
C
	num=inum
C	num=(inum/2)+1
C
C	bytes=((# of pairs)*2+(one pair counter))*(4 bytes/I*4)
C
	size = ((trace(1,1)*2)+1)*4
c
c	deallocate mem if needed
c
	If (tptrs(num,tnum) .ne. 0) call ffree(tptrs(num,tnum))
c
c	allocate mem 
c
	call fmalloc(tptrs(num,tnum),size)
	If (tptrs(num,tnum) .eq. 0) then
		write(*, 1)
		call trace_exit(-999,-999)
1	format('Unable to allocate memory to save a trace.')
	endif
c
c	get the proper pointer and call the routine
c
	call svtr(%val(tptrs(num,tnum)),trace)
C	
	Return
	End
c
c**************************************************************
c
	Subroutine  gettrace(inum,tnum,trace)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'  
	Include '../libs/vt_inc.f'
	Include 'tr_inc.f'  

	Integer*4	tptrs(max_images,max_rgns)

	integer*4	inum,tnum,num
	integer*4	trace(4096,2)

	common	/virt/tptrs
c
C	remember that the image number coming in is ODD... so +1
c
	num=inum
C	num=(inum/2)+1
c
c	if no saved then return null
c
	If (tptrs(num,tnum) .eq. 0) then
		trace(1,1)=0
		return
	endif
c
c	get the proper pointer and call the routine
c
	call gttr(%val(tptrs(num,tnum)),trace)
C	
	Return
	End
c
c**************************************************************
c
	subroutine	svtr(pttr,trace)

	implicit none
	
	integer*4	pttr(*),trace(4096,2),i,j

	pttr(1)=trace(1,1)
	j=2
	do i=2,trace(1,1)+1
		pttr(j)=trace(I,1)
		pttr(j+1)=trace(I,2)
		j=j+2
	enddo

	return
	end
c
c**************************************************************
c
	subroutine	gttr(pttr,trace)

	implicit none
	
	integer*4	pttr(*),trace(4096,2),i,j

	trace(1,1)=pttr(1)
	j=2
	Do i=2,trace(1,1)+1
		trace(i,1)=pttr(j)
		trace(i,2)=pttr(j+1)
		j=j+2
	enddo

	return
	end
