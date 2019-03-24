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
C    MODULE NAME    : getroipnt
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
C    DESCRIPTION    :  	This routine reads an ROI from disk...
C                                                                               
C    -- REVISION HISTORY --------------                                         
C
C                                                                               
C    -- GENERAL --------------                                                  
C                                                                               
C    SYNTAX         :     
C	Call getroipnt(num,point)
C    INPUTS         :          num : index into the rgnname list for template
C    OUTPUTS        : 		point : coordinates of the centroid
C
C    LIMITATIONS    :      
C    NOTES          :      
C                 
C                                                                               
C    ====================================================================== 
	Subroutine getroipnt(num,point)

	Implicit None

        Include 'fdevice.h'
        Include 'fgl.h'
        Include 'sgiuif.h'
        Include '../libs/vt_inc.f'

	Integer*4	num,theslice
	Real*4		scalef,point(3) !x,y,z in the volume
	Real*4		numimg
	Integer*4	I,J
	Character*200	filename,temp,dir,roi
	logical		found,error
	Integer*4	rois(4096,2)
	Integer*4	SliceToZ,strlen
	External	SliceToZ,strlen
C
C	Init vars
C
	point(1) = 0.0
	point(2) = 0.0
	point(3) = 0.0
C
	scalef = (float(imagesize)/512.0)
C
C	Get the base filename stuff
C
	Call indexnum_to_roi(cut_dir,roi,i)
	Call roi_to_dir(roi,dir)
C
C	Loop through all the images
C
	Do theslice = 1,realimages
C
C	build the filename
C
        	temp=''
        	Call Name_Changer(rgnnames(num),theslice,temp,Error)
        	if (error) goto 100
C
C	prefix the filename with appropriate directory
C
C		if (cut_dir .eq. 0) then
C			filename=temp(1:index(temp," ")-1)
C		Else if (cut_dir  .eq. 1) then
C			filename="righthemi/"//temp(1:index(temp," ")-1)
C		Else if (cut_dir  .eq. 2) then
C			filename="lefthemi/"//temp(1:index(temp," ")-1)
C		Endif
		If (cut_dir .eq. 0) then
			filename=temp(1:strlen(temp))
		Else
			filename=dir(1:strlen(dir))//"/"//
     +				temp(1:strlen(temp))   
C
C	special case for hemispheres
C
			If (num .eq. 1) then
				filename=temp(1:strlen(temp))
			Endif
		Endif
C
C	is it there??? (assume the worst...)
C
		inquire(file=filename,exist=found)
		If (.not. found) goto 100
C
		open(66,file=filename,form='formatted',status='old',
     +			err=100,readonly)
C
C	read number of points..
C
		read(66,*,err=100,end=100) rois(1,1)
C
C	Init vars
C
		point(1) = 0.0
		point(2) = 0.0
		point(3) = 0.0
C
C	half the number of points is the number of coords...
C
		rois(1,1)=rois(1,1)/2
C
		Do I=2,rois(1,1)+1
			read(66,*,err=100,end=100) rois(I,1)
			read(66,*,err=100,end=100) rois(I,2)
			point(1) = point(1) + rois(I,1)
			point(2) = point(2) + rois(I,2)
		Enddo
C
C	Compute point position in PIXELS (scale firstr)
C
		point(1) = (point(1)/float(rois(1,1)))*scalef
		point(2) = (point(2)/float(rois(1,1)))*scalef
		point(3) = SlicetoZ(theslice)
C
C	Now subtract out the center of the cube
C
		numimg = realimages+((realimages - 1) * interpimages)
		point(1) = point(1) - (imagesize/2.0)
		point(2) = point(2) - (imagesize/2.0)
		point(3) = (point(3) - (numimg/2.0))*squeezefactor
C		Type *,"Point found on slice ",theslice," x,y,z:",
C     +			point(1),point(2),point(3)
C
100		close(66)
C
	Enddo
C
	Return
C
	End
