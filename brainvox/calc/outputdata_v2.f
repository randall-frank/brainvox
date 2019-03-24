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
C    MODULE NAME    : outputdata_v2
C    VERSION        : 2.0 
C    LOCATION       :  
C    LAST EDIT      :  
C    STATUS         :  (Development,Testing,Production,Obsolete)
C    LANGUAGE       : FORTRAN
C    PACKAGE        : brainvox_calc
C
C
C    -- CREATION --------------                                                 
C                                                                               
C    PROGRAMMER     : rjf
C    DATE           : 6 May 94       
C    LINE-BY-LINER  :    
C    APPROVAL       : 
C
C    DESCRIPTION    :  	Routine to output ASCII from the version 2.0 of
C				calc stats.
C                                                                               
C    -- REVISION HISTORY --------------                                         
C
C		6 May 94 RJF
C			This is revision 2.0 of the output data routine.
C			The main change is that the entire VOLUME matrix
C			is strictly valid intersections and that the
C			LES_VOL is the lesion volume of any non-lesion
C			volume.
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
	Subroutine  outputdata_v2(volume,les_vol,tstr)

	Implicit None

	Include 'fgl.h'
	Include 'fdevice.h'
	Include 'sgiuif.h'  
	Include '../libs/vt_inc.f'  

	Character*(*)	tstr
	Character*6	typname(4)
	Character*24	fdate
	Real*4		volume(max_rgns,max_rgns),ratio
	Real*4		les_vol(max_rgns)

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
100	Format("Brainvox_calc Ver 2.0 output:",A24,/,"File:",A)
	write(66,104,err=499)
	write(66,101,err=499) "Region name    ","Type  ",
     +		"Total Volume ","Lesion Volume","% Lesion"
	write(66,103,err=499)
101	Format(A15," ",A6," ",A13," mm3 ",A13," ",A8) 
102	Format(A15," ",A6," ",F13.1," mm3 ",F13.1," ",F8.4) 
103	Format("--------------- ------ --------------- ---",
     +		" ------------- --------")
104	Format("------------------------------------------",
     +		"-----------------------")
	Do I=1,numrgns
		if (volume(I,I) .ne. 0) then
			ratio = (les_vol(i)/volume(i,i)) * 100
		Else
			ratio = 0.0
		Endif
		if (rgntypes(I) .ge. 3) ratio = 100.0
		write(66,102,err=499) rgnnames(I),typname(rgntypes(I)),
     +			volume(I,I),volume(I,I)*ratio/100.0,ratio
	Enddo
	write(66,104,err=499)
C
C	Detailed intersection output
C
	write(66,209,err=499)
	write(66,205,err=499)
	write(66,208,err=499)
	write(66,204,err=499)
	write(66,201,err=499) "Region name    ","Type  ",
     +		"Total Volume ","% of Total Vol"
	write(66,206,err=499) "Intersect Rgn  ","Type  ","Intersect Vol",
     +		"% of Total Vol"
	write(66,204,err=499)
201	Format(A15," ",A6,"       ",A15,"     ",A14) 
202	Format(A15," ",A6,"       ",F15.1," mm3",F11.3," %") 
204	Format("---------------------------------------",
     +		"-----------------------")
205	Format("--------- Region Intersection Details -",
     +		"-----------------------")
208	Format("- Non-Zero Intersection Values Only ---",
     +		"-----------------------")
206	Format("      ",A15," ",A6," ",A15,"     ",A14) 
207	Format("      ",A15," ",A6," ",F15.1," mm3",F11.3," %") 
209	Format(" ")
	Do I=1,numrgns
		ratio = 100
		write(66,202,err=499) rgnnames(I),
     +			typname(rgntypes(I)),volume(I,I),100.0
		Do J=1,numrgns
			If (i.ne.j) then
			if (volume(i,j) .ne. 0) Then
C
C	Lesion J intersects the ROI I with volume(I,J)
C
			ratio = 0.0
			If (volume(i,i) .ne. 0.0) Then
				ratio = (volume(I,J)/volume(I,I))*100.0
			Endif
			write(66,207,err=499) rgnnames(J),
     +				typname(rgntypes(J)),
     +				volume(I,J),ratio
			Endif
			Endif
		Enddo
		If (i .ne. numrgns) write(66,209,err=499)
	Enddo
	write(66,204,err=499)

	close(66,err=500)
	Return
C
C
499	close(66,err=500)
500     Continue
        Call bv_error("Unable to write to output files|"//
     +          "Check file permissions or disk space}",1)
C
	Return
	End
