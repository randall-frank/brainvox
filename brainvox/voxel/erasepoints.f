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
C    MODULE NAME    : erasepoints
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
C    DESCRIPTION    :  	Routine to erase points from lists
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
	Subroutine  erasepoints(num3,list3,num2,list2,slice,zoom,offset,
     +		x,y,col,change,myset)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'windows.inc'

	Integer*4	num2,list2(4096,4),num3,list3(4096,4)
	Integer*4	slice,zoom,offset(2)
	Integer*4	x,y,col,change
	Integer*4       myset(*)

	Integer*4	I,p(8),j,k,slicetoZ,error,out(8)
	Integer*4	tp2(4096,4),tp3(4096,4)
	Integer*4	etp2(4096,4),etp3(4096,4)
	Integer*4	rmove

	External	Slicetoz
	Real*4		temp
C
	Integer*4	FAT,clippingon,tal_over,clipshade
C
        common  /fat_com/FAT,clippingon,tal_over,clipshade
C
C	check error space
C
	temp = float(slicetoz(2) - slicetoz(1))/2.0
	if (temp .gt. 1.7) temp = 1.7
C
	change = 0
	j = 0
C
C	Check the mode
C
	If (slice .eq. -1) then
C
C	if 3d points then RGBmode
C
C	run thru list
C
		Do I=1,num3
			p(1) = (list3(I,1)-offset(1))*zoom
			p(2) = (list3(I,2)-offset(2))*zoom
			p(3) = p(1) + (zoom - 1) +FAT
			p(4) = p(2) + (zoom - 1) +FAT
C
C	Ok to save this point ?
C
			rmove = 0
			If ((x .ge. p(1)-1) .and. (x .le. p(3)+1)
     +				.and. (y .ge. p(2)-1) .and.
     +				(y .le. p(4)+1)) Then
				rmove = 1
C
C	Point could still pass if a specific color was requested
C
				If ((col .ne. -1).and.
     +					(list3(i,4).ne.col)) Then
					rmove = 0
				Endif
			Endif
			If (rmove .eq. 0) Then
				j = j + 1
				Do k = 1,4
					tp3(j,k) = list3(i,k)
					tp2(j,k) = list2(i,k)
				Enddo
			Else
				change = change + 1
				Do k = 1,4
					etp3(change,k) = list3(i,k)
					etp2(change,k) = list2(i,k)
				Enddo
				if (etp3(change,4) .ne. 0) Then
					etp3(change,4) = 0
					etp2(change,4) = 0
				Else
					etp3(change,4) = 7
					etp2(change,4) = 7
				Endif
			Endif
		Enddo
	Else
C
C	if 2d points then cmode
C
C	run thru list
C
		Do I=1,num2 
C
C	Are we in cuttingplane mode?
C
			error = 1
			If (wind2d(curwin).mode .eq. 0) then
C
C	Is the point on the slice?
C
				If (abs(slicetoz(slice)-list2(I,3)) 
     +						.le. temp) then
C
					p(1) = (list2(I,1)-offset(1))*zoom
					p(2) = (list2(I,2)-offset(2))*zoom
					p(3) = p(1) + (zoom - 1) +FAT
					p(4) = p(2) + (zoom - 1) +FAT
					error = 0
C
				Endif
			Else 
				p(1) = list2(I,1)
				p(2) = list2(I,2)
				p(3) = list2(I,3)
				Call vl_3dtocutplane(p(1),out(1),temp,error)
Cifdef DEBUG
Ctype *,"Erasepoints:in",p(1),p(2),p(3),"out:",out(1),out(2),out(3)
Cendif
				If (error .eq. 0) then
					p(1) = (out(1)-offset(1))*zoom
					p(2) = (out(2)-offset(2))*zoom
					p(3) = p(1) + (zoom - 1) +FAT
					p(4) = p(2) + (zoom - 1) +FAT
					error = 0
				Endif

			Endif
C
C	Ok to save this point ?
C
			rmove = 0
			If (error .eq. 0) Then
Cifdef DEBUG
Cwrite(*, *),"2x,y=",x,y," p =",p(1),p(2),p(3),p(4)				
Cendif
				If ((x .ge. p(1)-1) .and. (x .le. p(3)+1)
     +					.and. (y .ge. p(2)-1) .and.
     +					(y .le. p(4)+1)) Then
					rmove = 1
Cifdef DEBUG
Cwrite(*, *)"Removed2"
Cendif
C
C	Point could still pass if a specific color was requested
C
					If ((col .ne. -1).and.
     +						(list2(i,4).ne.col)) Then
						rmove = 0
					Endif
				Endif
			Endif
			If (rmove .eq. 0) Then
				j = j + 1
Cifdef DEBUG
Cwrite(*, *)"Saved2",j
Cendif
				Do k = 1,4
					tp3(j,k) = list3(i,k)
					tp2(j,k) = list2(i,k)
				Enddo
			Else
				change = change + 1
				Do k = 1,4
					etp3(change,k) = list3(i,k)
					etp2(change,k) = list2(i,k)
				Enddo
				if (etp2(change,4) .ne. 0) Then
					etp3(change,4) = 0
					etp2(change,4) = 0
				Else
					etp3(change,4) = 7
					etp2(change,4) = 7
				Endif
			Endif
		Enddo
	Endif
C
C	Copy the edited lists back into the original arrays
C
	If (change .ne. 0) Then

Cifdef DEBUG
Ctype *,"n2,n3,j=",num2,num3,j
Cendif
		change = 1
		num2 = j
		num3 = j
		Do i=1,j
			Do k = 1,4
				list3(i,k) = tp3(i,k)
				list2(i,k) = tp2(i,k)
			Enddo
		Enddo
		Do i=1,change
			Do k = 1,4
				list3(j+i,k) = etp3(i,k)
				list2(j+i,k) = etp2(i,k)
			Enddo
		Enddo
	Endif

	Return
	End
