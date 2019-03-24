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
C    MODULE NAME    : outputdata
C    VERSION        : 1.0 
C    LOCATION       :  
C    LAST EDIT      :  
C    STATUS         :  (Development,Testing,Production,Obsolete)
C    LANGUAGE       : FORTRAN
C    PACKAGE        : voxbrain_calc
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
	Subroutine  outputdata(volume,tstr)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'  
	Include '../libs/vt_inc.f'  

	character*(*)	tstr
	Character*6	typname(4)
	Character*24	fdate
	real*4		volume(max_rgns,max_rgns),lesvol,ratio

	integer*4	strlen,I,J

	logical	found
	
	external 	strlen,fdate

	data	typname/"Region","Region","Lesion","Lesion"/
C
	inquire(exist=found,file=tstr)
        If (found) then
                Open(66,file=tstr,status="old",
     +                  form="formatted",err = 500)
        Else
                Open(66,file=tstr,status="new",
     +                  form="formatted",err = 500)
        Endif      
C
C	do it...
C
	write(66,100,err=499) fdate(),tstr(1:strlen(tstr))
100	Format("Brainvox_calc output:",A24,/,"file:",A)
	write(66,104,err=499)
	write(66,101,err=499) "Region name    ","Type",
     +		"Volume         ","% Lesion"
	write(66,103,err=499)
101	Format(A15," ",A6," ",A15," mm3 ",A8) 
102	Format(A15," ",A6," ",F15.1," mm3 ",F8.4) 
103	Format("--------------- ------ --------------- --- --------")
104	Format("---------------------------------------------------")
	Do I=1,numrgns
		lesvol = 0.0
		Do J=1,numrgns
			if(i.ne.j)lesvol=lesvol+volume(I,J)
		Enddo
		if (volume(I,I) .ne. 0) then
			ratio = (lesvol/volume(i,i)) * 100
		Else
			ratio = 0.0
		Endif
		if (rgntypes(I) .ge. 3) ratio = 100.0
		write(66,102,err=499) rgnnames(I),typname(rgntypes(I)),
     +			volume(I,I),ratio
	Enddo
	write(66,104,err=499)
C
C	Detailed lesion output
C
	write(66,205,err=499)
	write(66,201,err=499) "Region name    ","Type",
     +		"Lesion Volume","Total Volume"
	write(66,206,err=499) "Lesion name    ","Intersect Vol",
     +		"% Inter","% Total"
	write(66,204,err=499)
201	Format(A15," ",A6," ",A15," mm3 ",A15," mm3") 
202	Format(A15," ",A6," ",F15.1," mm3 ",F15.1," mm3") 
204	Format("---------------------------------------",
     +		"-----------------------")
205	Format("-------------------- Lesion Details ---",
     +		"-----------------------")
206	Format("       ",A15," ",A15,"     ",A7,"   ",A7) 
207	Format("       ",A15," ",F15.1," mm3",F8.3," %",F8.3," %") 
	Do I=1,numrgns
		lesvol = 0.0
		Do J=1,numrgns
			if(i.ne.j)lesvol=lesvol+volume(I,J)
		Enddo
C
C	If (lesvol != 0.0) sub report on intersecting rois
C
		If (lesvol .ne. 0.0) then
			write(66,202,err=499) rgnnames(I),
     +				typname(rgntypes(I)),lesvol,volume(I,I)
			Do J=1,numrgns
				If ((i.ne.j).and.(volume(I,J).ne.0.0)) then
C
C	Lesion J intersects the ROI I with volume(I,J)
C
			write(66,207,err=499) rgnnames(J),
     +				volume(I,J),(volume(I,J)/lesvol)*100.0,
     +				(volume(I,J)/volume(I,I))*100.0
				Endif
			Enddo
		Endif
	Enddo
	write(66,204,err=499)
C
	close(66)
	Return
C
499	close(66)
500	Continue
	Call bv_error("Unable to write to output files|"//
     +		"Check file permissions or disk space}",1)
C
	Return
	End
