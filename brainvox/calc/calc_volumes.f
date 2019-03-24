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
C    MODULE NAME    : calc_volumes
C    VERSION        : 1.0 
C    LOCATION       :  
C    LAST EDIT      :  6 May 94
C    STATUS         :  (Development,Testing,Production,Obsolete)
C    LANGUAGE       : FORTRAN
C    PACKAGE        : brainvox
C
C
C    -- CREATION --------------                                                 
C                                                                               
C    PROGRAMMER     : rjf
C    DATE           : 6 May 94       
C    LINE-BY-LINER  :    
C    APPROVAL       : 
C
C    DESCRIPTION    :  	Routine to compute the volume matrix and the
C			lesion volumes
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
	Subroutine calc_volumes(volume,les_vol,dialog)

	Implicit None

	Include 'fdevice.h'
	Include 'fgl.h'
	Include 'sgiuif.h'
	Include '../libs/vt_inc.f'  
	Include 'calcuif.inc'  
C
	Real*4		volume(max_rgns,max_rgns)
	Real*4		les_vol(max_rgns)
C
	logical		error,quitflag,found
C
	real*4		area,fixedIPS
	Integer*4	counts(127)
	Integer*4	trace(4096,2),num
C
	Integer*4	status,i,j,c,x,y
	Character*100	tstr,fname
	Record /ditem/	dialog(*)
C
	Logical		done
	Character*200	roi,rgn,dir
C
	Integer*4	slice,side
	byte		image(512,512),image2(512,512)
C
	integer*4	strlen

	external 	strlen
C
	quitflag = .false.
C
	Call doevents(dialog,quitflag)
	if (quitflag) goto 999
C
C	fix the interpixel for the image size
C	ie if images are less than 512x512 then interpixel spacing
C	must be reduced by the ratio
C
	fixedIPS = (interpixel*float(imagesize))/512.0
C
	done = .false.
	side = 0
	Do while (.not. done)
C
C	For all sub volumes
C
		side = side + 1
		Call indexnum_to_roi(side,roi,i)
		If (i .eq. -1)  Then
			done = .true.
			goto 500
		Endif
C
C	read the region def files
C
		Call roi_to_rgn(roi,rgn)
		Call read_rgns(rgn,error)
		write(unit=dialog(worktext2).text,fmt=11) roi(1:strlen(roi))
		if (error) goto 500
11	Format("Working on ",A)
C
C	init volume space
C
	Do I=1,numrgns
		Do J=1,numrgns
			volume(I,J) = 0.0
		Enddo
		les_vol(i) = 0.0
	Enddo
C
C	for all slices
C
	do slice = 1, realimages
		write(unit=dialog(worktext).text,fmt=10) slice,realimages
10	Format("Working on slice ",I3.3," of ",I3.3)
		Call qenter(REDRAW,0)
		Call doevents(dialog,quitflag)
		if (quitflag) goto 999
C
C	compute a slice worth
C
C	Clear the buffer Image
C
		Do y=1,512 
			Do x=1,512
				image(x,y) = 0
			Enddo
		Enddo
C
C	First for each lesion paint the image buffer and calc all total volumes
C
		Do I=1,numrgns
C
C	read each ROI
C
			error = .false.
			Call name_changer(rgnnames(I),slice,tstr,error)
			if (error) goto 300
			Call roi_to_dir(roi,dir)
			fname = dir(1:strlen(dir))//"/"
     +				//tstr(1:strlen(tstr))
C
C       special case for hemispheres
C
        		If (i .eq. 1) Then
				fname=tstr(1:strlen(tstr))
			Endif
			Call read_roi_file(fname,trace,num,error)
			if ((error).or.(num .lt. 4)) goto 300
			Call fingers_cursor
C
			Call CalcArea(trace,num,area)
C
C	Add area on
C
		   	volume(i,i)=volume(i,i) +
     +				(area*(fixedIPS**2))*interslice
C
C	If lesion, paint into the image
C
		   	If (rgntypes(i) .ge. 3) then
				Call PaintArea(trace,num,I,image)
		   	Endif
C
300		   Continue
C
		   Call doevents(dialog,quitflag)
		   if (quitflag) goto 999
C
		Enddo
C
C	Now for all ROIs
C		If the ROI is not a lesion, get its % lesion volume
C		Intersect the ROI with ALL other ROIs
C
		Do I=1,numrgns
C
C	read each ROI
C
			error = .false.
			Call name_changer(rgnnames(I),slice,tstr,error)
			if (error) goto 200
			Call roi_to_dir(roi,dir)
			fname = dir(1:strlen(dir))//"/"
     +				//tstr(1:strlen(tstr))
C
C       special case for hemispheres
C
        		If (i .eq. 1) Then
				fname=tstr(1:strlen(tstr))
			Endif
			Call read_roi_file(fname,trace,num,error)
			if ((error).or.(num .lt. 4)) goto 200
			Call fingers_cursor
C
C	If non-lesion, how much is lesion?
C
		   	If (rgntypes(i) .le. 2) then
				Call IntersectArea(trace,num,counts,image)
				c = 0
				Do j=1,numrgns
					If (counts(j) .ge. 0) Then
						c = c + counts(j)
					Endif
				Enddo
C
C	add it on
C
				les_vol(i) = les_vol(i) + (float(c)*
     +					(fixedIPS**2))*interslice
			Endif
C
C	Paint the current ROI into another image
C
			Do x=1,512 
				Do y=1,512
					image2(x,y) = 0
				Enddo
			Enddo
			Call PaintArea(trace,num,1,image2)
C
C	Loop through all other ROIs
C
			Do j=i+1,numrgns
C
C	read each ROI
C
				error = .false.
				Call name_changer(rgnnames(j),slice,tstr,
     +					error)
				if (error) goto 400
				Call roi_to_dir(roi,dir)
				fname = dir(1:strlen(dir))//"/"
     +					//tstr(1:strlen(tstr))
C
C       special case for hemispheres
C
        			If (j .eq. 1) Then
					fname=tstr(1:strlen(tstr))
				Endif
				Call read_roi_file(fname,trace,num,error)
				if ((error).or.(num .lt. 4)) goto 400
				Call fingers_cursor
C
C	Compute intersections
C
				Call IntersectArea(trace,num,counts,image2)
				If (counts(1) .ge. 0) Then
					volume(i,j) = volume(i,j) + 
     +						(float(counts(1))*
     +						(fixedIPS**2))*interslice
					volume(j,i) = volume(i,j)
				Endif

C
C	Next J ROI
C
400			Continue
C
		   	Call doevents(dialog,quitflag)
		   	if (quitflag) goto 999

			Enddo
C
C	Next I ROI
C
200		   Continue
C
		   Call doevents(dialog,quitflag)
		   if (quitflag) goto 999
C
		Enddo
C
C	next slice
C
	Enddo
C
C	output ASCII files
C
		Call roi_to_rgn(roi,rgn)
		tstr = rgn(1:strlen(rgn))//".data "
		Call outputdata_v2(volume,les_vol,tstr)
C
C	next sub volume
C
500	continue
	Enddo
C
C	exit the routine  
C
999	continue
C
	Call arrow_cursor
C
	Return
C
	End
