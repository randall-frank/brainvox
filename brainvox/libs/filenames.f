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
C    MODULE NAME    : filenames
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
C    DATE           : 13 Nov 91       
C    LINE-BY-LINER  :    
C    APPROVAL       : 
C
C    DESCRIPTION    :  	 routines to convert an roiname  to a directory name
C			and to an _rgns filename.  Generally included to
C			improve compatability with previous versions of the sw
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
C	returns name of images subdir from an roiname
C
	Subroutine roi_to_dir(roi,dir)

	Implicit None

	Character*(*)	roi,dir
	Integer*4	I,J,strlen
	External	strlen
C
C	special compatability cases
C
	If (roi .eq. "*lhemi.roi") then
		dir = "lefthemi "
	Else if (roi .eq. "*rhemi.roi") then
		dir = "righthemi "
	Else
C
C	The proper rule:
C		Given: *xxx.roi  ->  xxx.dir
C
		i = index(roi,"*")
		j = index(roi,".roi")
		if (j .eq. 0) j = strlen(roi)
C
C	Check for errors
C
		If (i+1 .gt. j-1) then
			dir = "error.dir "
		Else
			dir = roi(i+1:j-1)//".dir "
		Endif
C
	Endif

	Return
	End
C
C	returns name of _rgns file from an roiname
C
	Subroutine roi_to_rgn(roi,rgn)

	Implicit None

	Character*(*)	roi,rgn
	Integer*4	I,J,strlen
	External	strlen
C
C	special compatability cases
C
	If (roi .eq. "*lhemi.roi") then
		rgn = "_lrgns"
	Else if (roi .eq. "*rhemi.roi") then
		rgn = "_rrgns"
	Else
C
C	The proper rule:
C		Given: *xxx.roi  ->  _xxx.rlst
C
		i = index(roi,"*")
		j = index(roi,".roi")
		if (j .eq. 0) j = strlen(roi)
C
C	Check for errors
C
		If (i+1 .gt. j-1) then
			rgn = "_error.rlst"
		Else
			rgn = "_"//roi(i+1:j-1)//".rlst"
		Endif
C
	Endif
	
	Return
	End
C
C	Clean up an ROIname for use in a popup menu
C
	Subroutine	clean_roi_name(roi)

	Implicit None

	Character*(*)	roi
	Integer*4	i,num,j,strlen
	External strlen
	Character*200	tt

	num = strlen(roi)
	j = 1

	Do i=1,num
		If (roi(i:i) .ne. '%') Then
			tt(j:j) = roi(i:i)
		Else
			tt(j:j) = "X"
		Endif
		j = j + 1
	Enddo
	tt(j:) = " "

	roi = tt

	Return
	End
C
C	Return the roiname in the _hemis file which matches the
C	given index
C
	Subroutine indexnum_to_roi(idx,roi,type)

	Implicit None

	Integer*4	idx,num,rgntype,i,type
	Character*(*)	roi
	Character*40	rgnname

	roi = ""
	type = -1
	If (idx .eq. 0) Return
C
C	open the file
C
	Open(unit=66,file='_hemis',form='formatted',
     +             readonly,status='old',err=999)
C
C	read the number of entries
C
	read(66,*,err=997) num
	If (idx .gt. num) Then
		close(66)
		return
	Endif
C
C	Read the entry
C
	Do i=1,idx
		read(66,11,err=997) rgnname,rgntype
11      Format(A40,I9)
	Enddo

	roi = rgnname
	type = rgntype

	close(66)

	return
C
C	Errors
C
999	type 998
998	Format("Brainvox error - Unable to open _hemis",
     +		" file in indexnum_to_roi")

	Return

997	type 996
996	Format("Brainvox error - error in read _hemis",
     +		" file in indexnum_to_roi")
	Close(66)
	Return

	End
C
C       Copy null terminated string from data into tstr returning length
C
        Subroutine get_filename(tstr,data,len)

        Implicit None

        Character*(*)   tstr
        Integer*4       i,len
        Byte            data(*)

        tstr = ""
        i = 0
        Do while (data(i+1) .ne. 0)
                i = i + 1
                tstr(i:i) = char(data(i))
        Enddo
C
C       Space terminate
C
        tstr(i+1:i+1) = " "

        len = i

        Return
        End

