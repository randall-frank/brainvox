C    ======================================================================     
C    THIS SOFTWARE IS THE PROPERTY OF:                                          
C         The Image Analysis Facility                                     
C         71 HBRF     
C         The University of Iowa, Iowa City, Iowa                               C	  Copyright (C) 1989 Image Analysis Facility, University of Iowa
C    ======================================================================     
C                                                                               
C    -- OVERVIEW --------------                                                 
C                                                                               
C    MODULE NAME    : _Main   
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
C    DATE           : 18 Sep 89       
C    LINE-BY-LINER  :    
C    APPROVAL       : 
C
C    DESCRIPTION    :  	This program creates the BRW coordinate system
C			to be used as an external conversion system.
C			(sort of a desktop BRW coordinate system calculator)
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

	Implicit None

	Include "mri_brw_inc.f"

	Real*4		bv_pt(3),brw_pt(3)
	Real*4		points(9,3)
	Character*1	yn
	Integer*4	I,done,brainvox_coords,prompt_len
	Character*8	prompt

	Integer*4	iargc
	External	iargc
C
C	check for streaming
C
	If (iargc() .ne. 0) Then
		Call streaming
		Call exit(0)
	Endif
C
C	read any existing file...
C
	brainvox_coords = 1
	prompt = "Brainvox"
	prompt_len = 8
	Call bv_calib_brw()
C
C	Enter data if needed
C
	If (BRW_valid .eq. 1) Then
99	Format("Enter new calibration points? (y/n) ",$)
98	Format(A1)
97		Type 99
		Read(*,98,err=97) yn
		If ((yn .eq. 'Y') .or. (yn .eq. 'y')) Then
			BRW_valid = 0
		Endif
		
	Endif
	If (BRW_valid .eq. 0) Then
200	Format("Is Brainvox the source for the points? (y/n) ",$)
201		Type 200
		Read(*,98,err=201) yn
		If ((yn .eq. 'n') .or. (yn .eq. 'N')) Then
			brainvox_coords = 0
			prompt = "Mtrace"
			prompt_len = 6
		Endif
		
C
C	Collect the data for the (MM) points on an axial section
C
100	Format(/,/,/,
     +	"This software assumes you are looking at an axial section",/
     +  "of the dataset and are entering numbers in MM.",/,
     +  "The scan should contain 9 fiducials (minimally) arranged and",/,
     +	"numbered as follows:")
101     Format(
     +  "         Right        ^          Left",/,
     +  "                1     |     9        ",/,
     +  "                      |     8        ",/,
     +  "                      +-------->",/,
     +  "                2                    ",/,
     +  "                3           7        ",/,
     +  "                  4     5 6          ",/,/,
     +  "Please enter the coordinates for each of these points",/,
     +  "in order (1-9) shown here.")
102	Format("Enter the coordinates for point:",I2)
		Type 100
		Type 101

		Do i=1,9
103			Type 102,i
			Type 303,"Brainvox","X"
			Read(*,*,err=103) points(i,1)
			Type 303,"Brainvox","Y"
			Read(*,*,err=103) points(i,2)
			Type 303,"Brainvox","Z"
			Read(*,*,err=103) points(i,3)
C
C	Brainvox records Z backward...
C
			If (Brainvox_coords .eq. 1) Then
				points(i,3) = -points(i,3)
			Endif
C
		Enddo

		Call bv_calibrate_brw(points)
	Endif
C
	Do i=1,3
		brw_pt(i) = 0
		bv_pt(i) = 0
	Enddo
C
	done = 0
	Do while (done .eq. 0)
299		continue
		Type 300,prompt(1:prompt_len)
		Type 301,brw_pt(1),brw_pt(2),brw_pt(3),prompt,
     +			bv_pt(1),bv_pt(2),bv_pt(3)
		Type 302,prompt(1:prompt_len)
		Read(*,*,err=299) i
		if (i .eq. 0) then
			done = 1
		Else if (i .eq. 1) then
			Type 303,"BRW","X"
			Read(*,*,err=299) brw_pt(1)
			Type 303,"BRW","Y"
			Read(*,*,err=299) brw_pt(2)
			Type 303,"BRW","Z"
			Read(*,*,err=299) brw_pt(3)
			Call BRW_to_MRI(brw_pt,bv_pt)
C
C	Brainvox records Z backward...
C
			If (Brainvox_coords .eq. 1) bv_pt(3) = -bv_pt(3)
C
		Else if (i .eq. 2) then
			Type 303,prompt(1:prompt_len),"X"
			Read(*,*,err=299) bv_pt(1)
			Type 303,prompt(1:prompt_len),"Y"
			Read(*,*,err=299) bv_pt(2)
			Type 303,prompt(1:prompt_len),"Z"
			Read(*,*,err=299) bv_pt(3)
C
C	Brainvox records Z backward...
C
			If (Brainvox_coords .eq. 1) bv_pt(3) = -bv_pt(3)
C
			Call MRI_to_BRW(bv_pt,brw_pt)
		Endif
	Enddo
C
	call exit(0)
C
300	Format(/,/,/,/,/,
     +	       "------------------------------------------------",/,
     +         "BRW->",A," coordinate conversion",/,
     +	       "Version 1.1  13 Sep 94 rjf",/,
     +         "------------------------------------------------")
301	Format("     BRW point: X=",F8.3," Y=",F8.3," Z=",F8.3,/,
     +	       A8," point: X=",F8.3," Y=",F8.3," Z=",F8.3)
302	Format("------------------------------------------------",/,
     +         "Options:  (0) Exit",/,
     +         "          (1) Enter BRW point",/,
     +         "          (2) Enter ",A," point",/,
     +	       "Select (0-2):",$)
303	Format("Enter ",A," coordinate: ",A,"=",$)
C
	End
C
C	Command line version
C
	Subroutine streaming

	Implicit None

	Include "mri_brw_inc.f"

	Real*4		bv_pt(3),brw_pt(3)
	Real*4		points(9,3)
	Integer*4	i,dir

	Do i=1,9
		Read(*,*,err=299) points(i,1),points(i,2),points(i,3)
	Enddo
	Call bv_calibrate_brw(points)

	Do while (.true.)

	Read(*,*,err=299) dir,bv_pt(1),bv_pt(2),bv_pt(3)
	Do i=1,3
		brw_pt(i) = bv_pt(i)
	Enddo
	If (dir .eq. 1) Then
		Call MRI_to_BRW(bv_pt,brw_pt)
		type *,brw_pt(1),brw_pt(2),brw_pt(3)
	Else
		Call BRW_to_MRI(brw_pt,bv_pt)
		type *,bv_pt(1),bv_pt(2),bv_pt(3)
	Endif

	Enddo

299	continue

	Return
	End
